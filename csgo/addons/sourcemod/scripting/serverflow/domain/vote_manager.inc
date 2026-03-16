#if defined _SERVERFLOW_VOTE_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_VOTE_MANAGER_INC_

float VoteManager_GetDuration()
{
    return g_VoteDuration;
}

void Vote_Reset()
{
    Session_StartVote(VoteType_None, 0, 0.0, 0);
    Session_FinishVote();
}

bool VoteManager_CanStartVote(int caller, char[] reason, int reasonLen)
{
    return State_CanClientStartVote(caller, reason, reasonLen);
}

bool VoteManager_RequestVoteStart(int caller, char[] reason, int reasonLen)
{
    if (!VoteManager_CanStartVote(caller, reason, reasonLen))
    {
        return false;
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

    Notify_VoteStarted(caller);
    ModeDebug("vote started by %N", caller);
    return true;
}

bool VoteManager_RequestVoteCast(int client, int modeIndex, char[] reason, int reasonLen)
{
    if (!Session_IsVoteInProgress())
    {
        strcopy(reason, reasonLen, "vote_not_running");
        return false;
    }

    if (!Scenario_IsValidIndex(modeIndex))
    {
        strcopy(reason, reasonLen, "invalid_scenario");
        return false;
    }

    if (Session_HasClientVoted(client))
    {
        strcopy(reason, reasonLen, "already_voted");
        return false;
    }

    Session_RecordVote(client, modeIndex);
    reason[0] = '\0';
    return true;
}

public Action Timer_FinishVote(Handle timer)
{
    Session_FinishVote();

    int winner = 0;
    for (int i = 1; i < Scenario_GetCount(); i++)
    {
        if (Session_GetVoteCount(i) > Session_GetVoteCount(winner))
        {
            winner = i;
        }
    }

    char winnerScenario[16];
    Scenario_GetIdByIndex(winner, winnerScenario, sizeof(winnerScenario));

    ServerState state = Session_GetState();
    PendingConfirmPolicy confirmPolicy = (state == ServerState_PostMatch) ? PendingConfirm_Auto : PendingConfirm_Manual;
    char voteReason[96];
    Format(voteReason, sizeof(voteReason), "vote winner (%d votes)", Session_GetVoteCount(winner));
    Pending_Create(winnerScenario, "", "vote_winner", 0, voteReason, ServerState_PostMatch, confirmPolicy);

    if (confirmPolicy == PendingConfirm_Auto)
    {
        Pending_Select();
        Pending_Confirm(0, "vote_auto_confirm");
        Transition_ApplyPending("vote_auto_apply");
    }

    Call_StartForward(g_FwdOnVoteCompleted);
    Call_PushString(winnerScenario);
    Call_PushCell(Session_GetVoteCount(winner));
    Call_Finish();

    Notify_VoteFinished(winnerScenario);
    return Plugin_Stop;
}
