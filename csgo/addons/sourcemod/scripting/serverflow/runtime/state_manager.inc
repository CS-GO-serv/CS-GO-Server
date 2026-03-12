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

    if (from == ServerState_Transition)
    {
        return (to == ServerState_Lobby || to == ServerState_PreMatch || to == ServerState_Match);
    }

    return true;
}

bool State_TransitionTo(ServerState newState, const char[] reason)
{
    ServerState oldState = Session_GetState();
    if (!State_CanTransition(oldState, newState))
    {
        return false;
    }

    Session_SetState(newState);

    Call_StartForward(g_FwdOnServerStateChanged);
    Call_PushCell(oldState);
    Call_PushCell(newState);
    Call_PushString(reason);
    Call_Finish();
    return true;
}
