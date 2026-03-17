#if defined _SERVERFLOW_POLICY_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_POLICY_MANAGER_INC_


OverrideLevel Policy_GetActorOverrideLevel(int actor)
{
    if (actor == 0)
    {
        return OverrideLevel_Console;
    }

    if (!IsValidClient(actor))
    {
        return OverrideLevel_None;
    }

    if (CheckCommandAccess(actor, "serverflow_transition_console", ADMFLAG_ROOT, true))
    {
        return OverrideLevel_Console;
    }

    if (CheckCommandAccess(actor, "serverflow_transition_admin", ADMFLAG_CHANGEMAP, true))
    {
        return OverrideLevel_Admin;
    }

    return OverrideLevel_Moderator;
}

bool Policy_IsScenarioSelectionAllowed(ServerState state)
{
    return (state != ServerState_Transition);
}

bool Policy_IsVoteAllowed(ServerState state)
{
    return (state == ServerState_Lobby || state == ServerState_PostMatch);
}

bool Policy_ApplyLateJoinGate(int client, const char[] source)
{
    if (!IsValidClient(client))
    {
        return false;
    }

    if (Session_GetState() != ServerState_PreMatch && Session_GetState() != ServerState_Match)
    {
        PlayerState_SetLifecycle(client, PlayerLifecycle_Eligible);
        Audit_LogEvent("policy", "late_join", client, "bypass", "state_not_live");
        ModeDebug("late join gate bypass: client=%N source=%s state=%d", client, source, Session_GetState());
        return true;
    }

    switch (g_LateJoinPolicy)
    {
        case LateJoinPolicy_Disallow:
        {
            PlayerState_SetLifecycle(client, PlayerLifecycle_LateJoin);
            ReadyManager_ForceUnready(client, "late_join_disallow");
            ChangeClientTeam(client, 1);
            PrintToChat(client, "[ServerFlow] Позднее подключение запрещено. Вы переведены в наблюдатели.");
            Audit_LogEvent("policy", "late_join", client, "denied", "policy_disallow");
            ModeDebug("late join denied: client=%N source=%s policy=%d", client, source, g_LateJoinPolicy);
            return false;
        }
        case LateJoinPolicy_Spectator:
        {
            PlayerState_SetLifecycle(client, PlayerLifecycle_LateJoin);
            ReadyManager_ForceUnready(client, "late_join_spectator");
            ChangeClientTeam(client, 1);
            PrintToChat(client, "[ServerFlow] Позднее подключение в матч запрещено: наблюдайте до следующей стадии.");
            Audit_LogEvent("policy", "late_join", client, "spectator", "policy_spectator");
            ModeDebug("late join spectator: client=%N source=%s policy=%d", client, source, g_LateJoinPolicy);
            return true;
        }
    }

    PlayerState_SetLifecycle(client, PlayerLifecycle_Eligible);
    Audit_LogEvent("policy", "late_join", client, "allow", "policy_allow");
    ModeDebug("late join allowed: client=%N source=%s policy=%d", client, source, g_LateJoinPolicy);
    return true;
}

bool State_CanClientSelectScenario(int client, int scenarioIndex, char[] reason, int reasonLen)
{
    if (!IsValidClient(client))
    {
        strcopy(reason, reasonLen, "invalid_client");
        return false;
    }

    if (!Scenario_IsValidIndex(scenarioIndex))
    {
        strcopy(reason, reasonLen, "invalid_scenario");
        return false;
    }

    if (!Policy_IsScenarioSelectionAllowed(Session_GetState()))
    {
        strcopy(reason, reasonLen, "blocked_by_state");
        return false;
    }

    reason[0] = '\0';
    return true;
}

bool State_CanClientStartVote(int client, char[] reason, int reasonLen)
{
    if (!IsValidClient(client))
    {
        strcopy(reason, reasonLen, "invalid_client");
        return false;
    }

    if (!Policy_IsVoteAllowed(Session_GetState()))
    {
        strcopy(reason, reasonLen, "blocked_by_state");
        return false;
    }

    if (Session_IsVoteInProgress())
    {
        strcopy(reason, reasonLen, "vote_in_progress");
        return false;
    }

    if (CountHumanPlayers() < g_VoteMinPlayers)
    {
        strcopy(reason, reasonLen, "not_enough_players");
        return false;
    }

    if (GetTime() < Session_GetVoteNextAllowedAt())
    {
        strcopy(reason, reasonLen, "cooldown");
        return false;
    }

    reason[0] = '\0';
    return true;
}

bool State_CanClientManageTransitions(int client, char[] reason, int reasonLen)
{
    if (client == 0)
    {
        reason[0] = '\0';
        return true;
    }

    if (!IsValidClient(client))
    {
        strcopy(reason, reasonLen, "invalid_client");
        return false;
    }

    if (!CheckCommandAccess(client, "serverflow_transition_admin", ADMFLAG_CHANGEMAP, true))
    {
        strcopy(reason, reasonLen, "missing_permission");
        return false;
    }

    reason[0] = '\0';
    return true;
}
