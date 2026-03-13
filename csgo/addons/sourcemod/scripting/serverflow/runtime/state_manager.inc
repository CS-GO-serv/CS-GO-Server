#if defined _SERVERFLOW_STATE_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_STATE_MANAGER_INC_

bool State_CanTransition(ServerState from, ServerState to)
{
    if (from == to)
    {
        return true;
    }

    switch (from)
    {
        case ServerState_Lobby:
        {
            return (to == ServerState_PreMatch || to == ServerState_Transition);
        }
        case ServerState_PreMatch:
        {
            return (to == ServerState_Match || to == ServerState_Lobby || to == ServerState_Transition);
        }
        case ServerState_Match:
        {
            return (to == ServerState_PostMatch || to == ServerState_Transition);
        }
        case ServerState_PostMatch:
        {
            return (to == ServerState_Lobby || to == ServerState_PreMatch || to == ServerState_Transition);
        }
        case ServerState_Transition:
        {
            return (to != ServerState_Transition);
        }
    }

    return false;
}

bool State_IsScenarioSelectionAllowed(ServerState state)
{
    return (state != ServerState_Transition);
}

bool State_IsVoteAllowed(ServerState state)
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

    if (!State_IsScenarioSelectionAllowed(Session_GetState()))
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

    if (!State_IsVoteAllowed(Session_GetState()))
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

bool State_TransitionTo(ServerState newState, const char[] reason)
{
    ServerState oldState = Session_GetState();
    if (!State_CanTransition(oldState, newState))
    {
        Audit_Log("state_transition_denied");
        ModeDebug("state transition denied: %d -> %d reason=%s", oldState, newState, reason);
        return false;
    }

    Session_SetState(newState);

    Call_StartForward(g_FwdOnServerStateChanged);
    Call_PushCell(oldState);
    Call_PushCell(newState);
    Call_PushString(reason);
    Call_Finish();

    char auditMessage[160];
    Format(auditMessage, sizeof(auditMessage), "state_transition_applied old=%d new=%d reason=%s", oldState, newState, reason);
    Audit_Log(auditMessage);

    return true;
}

bool State_EnterLobby(const char[] reason = "enter_lobby")
{
    Session_SetTransitionType(TransitionType_EnterLobby);
    return State_TransitionTo(ServerState_Lobby, reason);
}

bool State_EnterPreMatch(const char[] reason = "enter_prematch")
{
    Session_SetTransitionType(TransitionType_EnterPreMatch);
    return State_TransitionTo(ServerState_PreMatch, reason);
}

bool State_EnterMatch(const char[] reason = "enter_match")
{
    Session_SetTransitionType(TransitionType_EnterMatch);
    return State_TransitionTo(ServerState_Match, reason);
}

bool State_EnterPostMatch(const char[] reason = "enter_postmatch")
{
    Session_SetTransitionType(TransitionType_EnterPostMatch);
    return State_TransitionTo(ServerState_PostMatch, reason);
}

bool State_EnterTransition(const char[] reason = "enter_transition")
{
    Session_SetTransitionType(TransitionType_EnterTransition);
    return State_TransitionTo(ServerState_Transition, reason);
}
