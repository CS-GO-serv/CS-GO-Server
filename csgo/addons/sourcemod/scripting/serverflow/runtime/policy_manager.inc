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
