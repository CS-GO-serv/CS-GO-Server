#if defined _SERVERFLOW_TRANSITION_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_TRANSITION_MANAGER_INC_

static void Transition_AuditDeniedApply(PendingChange &pending, const char[] reasonCode, const char[] context, const char[] fallbackDecision)
{
    Audit_LogEvent("transition", "apply_pending", pending.requestedBy, "denied", reasonCode);
    Audit_LogEvent("transition", "fallback", pending.requestedBy, fallbackDecision, context);
}

static void Transition_AuditFailedApply(PendingChange &pending, const char[] reasonCode, const char[] context, const char[] fallbackDecision)
{
    Audit_LogEvent("transition", "apply_pending", pending.requestedBy, "failed", reasonCode);
    Audit_LogEvent("transition", "fallback", pending.requestedBy, fallbackDecision, context);
}

static void Transition_ApplyScenario(const char[] scenarioId)
{
    int scenarioIndex = FindModeById(scenarioId);

    Session_SetCurrentScenario(scenarioId);
    Session_SetQueuedScenario(scenarioId);

    if (g_Scenarios[scenarioIndex].cfgFile[0] != '\0')
    {
        ServerCommand("exec %s", g_Scenarios[scenarioIndex].cfgFile);
    }

    if (g_Scenarios[scenarioIndex].mapgroup[0] != '\0')
    {
        ServerCommand("mapgroup %s", g_Scenarios[scenarioIndex].mapgroup);
    }
}

static bool Transition_ValidateAndApply(PendingChange &pending, const char[] context)
{
    char reasonCode[64];
    reasonCode[0] = '\0';

    ServerState state = Session_GetState();
    OverrideLevel overrideLevel = Session_GetOverrideLevel();

    if (pending.state != Pending_Confirmed)
    {
        strcopy(reasonCode, sizeof(reasonCode), "deny_pending_state_not_confirmed");
    }
    else if (state == ServerState_Transition && overrideLevel < OverrideLevel_Console)
    {
        strcopy(reasonCode, sizeof(reasonCode), "deny_state_transition_locked");
    }
    else if (Session_IsLockPhase() && overrideLevel < OverrideLevel_Admin)
    {
        strcopy(reasonCode, sizeof(reasonCode), "deny_lock_phase_active");
    }
    else if (pending.expiresAt > 0 && GetTime() > pending.expiresAt)
    {
        strcopy(reasonCode, sizeof(reasonCode), "deny_deadline_expired");
    }
    else if (pending.allowedApplyState != state)
    {
        strcopy(reasonCode, sizeof(reasonCode), "deny_apply_state_mismatch");
    }
    else if (pending.scenarioId[0] != '\0' && FindModeById(pending.scenarioId) == -1)
    {
        strcopy(reasonCode, sizeof(reasonCode), "deny_invalid_scenario");
    }
    else if (pending.scenarioId[0] != '\0' && !State_CanTransition(state, ServerState_PreMatch))
    {
        strcopy(reasonCode, sizeof(reasonCode), "deny_illegal_state_transition");
    }

    char mapScenarioId[16];
    mapScenarioId[0] = '\0';

    if (reasonCode[0] == '\0')
    {
        if (pending.scenarioId[0] != '\0')
        {
            strcopy(mapScenarioId, sizeof(mapScenarioId), pending.scenarioId);
        }
        else
        {
            Session_GetCurrentScenario(mapScenarioId, sizeof(mapScenarioId));
        }

        if (pending.mapName[0] != '\0')
        {
            int mapScenarioIndex = FindModeById(mapScenarioId);
            if (mapScenarioIndex == -1)
            {
                strcopy(reasonCode, sizeof(reasonCode), "deny_map_validation_missing_scenario");
            }
            else if (!Playlist_IsMapAllowed(mapScenarioIndex, pending.mapName) || !IsMapValid(pending.mapName))
            {
                strcopy(reasonCode, sizeof(reasonCode), "deny_map_not_allowed");
            }
        }
    }

    if (reasonCode[0] != '\0')
    {
        Transition_AuditDeniedApply(pending, reasonCode, context, "cancel_pending");
        Pending_Cancel(reasonCode);
        Session_SetFallbackReason(reasonCode);
        return false;
    }

    Session_SetTransitionType(TransitionType_ApplyPending);
    Session_SetPendingChangeState(Pending_Applying);

    if (pending.scenarioId[0] != '\0')
    {
        Transition_ApplyScenario(pending.scenarioId);

        if (!State_EnterPreMatch("pending_confirmed"))
        {
            Transition_AuditFailedApply(pending, "failed_prematch_transition", context, "cancel_pending");
            Pending_Cancel("failed_prematch_transition");
            Session_SetFallbackReason("failed_prematch_transition");
            return false;
        }

        TeamManager_EnterPreMatchSetup("pending_confirmed");
        ReadyManager_ResetAll("pending_confirmed");
    }

    if (pending.mapName[0] != '\0' || pending.scenarioId[0] != '\0')
    {
        char resolvedMap[64];
        if (!Playlist_ResolveNextMap(mapScenarioId, pending.mapName, resolvedMap, sizeof(resolvedMap)) || !IsMapValid(resolvedMap))
        {
            Transition_AuditFailedApply(pending, "failed_map_resolution", context, "cancel_pending");
            Pending_Cancel("failed_map_resolution");
            Session_SetFallbackReason("failed_map_resolution");
            return false;
        }

        Session_SetQueuedMap(resolvedMap);
        ServerCommand("changelevel %s", resolvedMap);
    }

    return true;
}

void Transition_QueueScenario(const char[] scenarioId, int actor, const char[] source = "scenario_queue")
{
    Session_SetQueuedScenario(scenarioId);
    Pending_Create(scenarioId, "", source, actor, source, Session_GetState(), PendingConfirm_Manual);
}

void Transition_QueueMap(const char[] mapName, int actor, const char[] source = "map_queue")
{
    char currentScenario[16];
    Session_GetCurrentScenario(currentScenario, sizeof(currentScenario));
    Pending_Create(currentScenario, mapName, source, actor, source, Session_GetState(), PendingConfirm_Manual);
}

void Transition_CancelPending(const char[] reason = "cancel", const char[] why = "")
{
    Pending_Cancel(reason, why);
}

bool Transition_ApplyPending(const char[] source = "")
{
    PendingChange pending;
    Session_GetPendingChange(pending);

    if (!Transition_ValidateAndApply(pending, source))
    {
        return false;
    }

    Audit_LogEvent("transition", "apply_pending", pending.requestedBy, "ok", source);
    ModeDebug("pending applied: scenario=%s map=%s source=%s requested_source=%s", pending.scenarioId, pending.mapName, source, pending.source);

    Session_ClearPendingChange();
    Session_SetFallbackReason("apply_success");
    return true;
}

bool Transition_ConfirmAndApplyPending(int actor, const char[] source = "")
{
    if (!Pending_Confirm(actor, source))
    {
        return false;
    }

    if (!Transition_ApplyPending(source))
    {
        return false;
    }

    return true;
}
