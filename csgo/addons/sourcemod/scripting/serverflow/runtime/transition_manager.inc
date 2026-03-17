#if defined _SERVERFLOW_TRANSITION_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_TRANSITION_MANAGER_INC_

void Transition_QueueScenario(const char[] scenarioId, int actor, const char[] source = "scenario_queue")
{
    Session_SetQueuedScenario(scenarioId);
    Pending_Create(scenarioId, "", source, actor);
}

void Transition_QueueMap(const char[] mapName, int actor, const char[] source = "map_queue")
{
    char currentScenario[16];
    Session_GetCurrentScenario(currentScenario, sizeof(currentScenario));
    Pending_Create(currentScenario, mapName, source, actor);
}

void Transition_CancelPending(const char[] reason = "cancel")
{
    Pending_Cancel(reason);
}

static bool Transition_ApplyScenario(const char[] scenarioId)
{
    int scenarioIndex = FindModeById(scenarioId);
    if (scenarioIndex == -1)
    {
        ModeDebug("transition apply rejected: unknown scenario %s", scenarioId);
        Audit_Log("transition_apply_rejected_unknown_scenario");
        return false;
    }

    Session_SetCurrentScenario(scenarioId);
    Session_SetQueuedScenario(scenarioId);

    ServerCommand("game_type %d", g_Scenarios[scenarioIndex].gameType);
    ServerCommand("game_mode %d", g_Scenarios[scenarioIndex].gameMode);
    ModeDebug(
        "scenario apply game settings: id=%s game_type=%d game_mode=%d",
        scenarioId,
        g_Scenarios[scenarioIndex].gameType,
        g_Scenarios[scenarioIndex].gameMode
    );

    if (g_Scenarios[scenarioIndex].cfgFile[0] != '\0')
    {
        ServerCommand("exec %s", g_Scenarios[scenarioIndex].cfgFile);
    }

    if (g_Scenarios[scenarioIndex].mapgroup[0] != '\0')
    {
        ServerCommand("mapgroup %s", g_Scenarios[scenarioIndex].mapgroup);
    }

    return true;
}

bool Transition_ApplyPending(const char[] source = "")
{
    PendingChange pending;
    Session_GetPendingChange(pending);

    if (pending.state != Pending_Confirmed)
    {
        return false;
    }

    Session_SetTransitionType(TransitionType_ApplyPending);
    Session_SetPendingChangeState(Pending_Applying);

    if (pending.scenarioId[0] != '\0' && !Transition_ApplyScenario(pending.scenarioId))
    {
        Pending_Cancel("apply_failed_unknown_scenario");
        Audit_Log("pending_apply_fallback_cancelled");
        return false;
    }

    if (pending.scenarioId[0] != '\0')
    {
        State_EnterPreMatch("pending_confirmed");
        TeamManager_EnterPreMatchSetup("pending_confirmed");
        ReadyManager_ResetAll("pending_confirmed");
    }

    if (pending.mapName[0] != '\0' || pending.scenarioId[0] != '\0')
    {
        char resolvedMap[64];
        Playlist_ResolveNextMap(pending.scenarioId, pending.mapName, resolvedMap, sizeof(resolvedMap));
        Session_SetQueuedMap(resolvedMap);
        ServerCommand("changelevel %s", resolvedMap);
    }

    char auditMessage[224];
    Format(auditMessage, sizeof(auditMessage), "pending_applied scenario=%s map=%s source=%s requested_source=%s", pending.scenarioId, pending.mapName, source, pending.source);
    Audit_Log(auditMessage);
    ModeDebug("pending applied: scenario=%s map=%s source=%s requested_source=%s", pending.scenarioId, pending.mapName, source, pending.source);

    Session_ClearPendingChange();
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
        Audit_Log("pending_apply_fallback_after_confirm");
        return false;
    }

    return true;
}
