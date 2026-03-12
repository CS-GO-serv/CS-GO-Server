#if defined _SERVERFLOW_VOTE_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_VOTE_MANAGER_INC_

void Vote_Reset()
{
    Session_StartVote(VoteType_None, 0, 0.0, 0);
    Session_FinishVote();
}

void Vote_Start(int caller)
{
    if (Session_IsVoteInProgress())
    {
        return;
    }

    Session_StartVote(VoteType_Scenario, caller, g_VoteDuration, g_VoteCooldown);
    Session_SetVoteTimer(CreateTimer(g_VoteDuration, Timer_FinishVote));

    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsValidClient(i))
        {
            ShowVoteMenu(i);
        }
    }

    ModeDebug("vote started by %N", caller);
}

void Vote_Cast(int client, int modeIndex)
{
    if (!Session_IsVoteInProgress() || modeIndex < 0 || modeIndex >= MAX_MODES)
    {
        return;
    }

    if (Session_HasClientVoted(client))
    {
        return;
    }

    Session_RecordVote(client, modeIndex);
}

public Action Timer_FinishVote(Handle timer)
{
    Session_FinishVote();

    int winner = 0;
    for (int i = 1; i < MAX_MODES; i++)
    {
        if (Session_GetVoteCount(i) > Session_GetVoteCount(winner))
        {
            winner = i;
        }
    }

    Transition_QueueScenario(g_Modes[winner].id, 0);
    Pending_Confirm();

    Call_StartForward(g_FwdOnVoteCompleted);
    Call_PushString(g_Modes[winner].id);
    Call_PushCell(Session_GetVoteCount(winner));
    Call_Finish();

    PrintToChatAll("[ServerFlow] Vote finished: %s", g_Modes[winner].id);
    return Plugin_Stop;
}
