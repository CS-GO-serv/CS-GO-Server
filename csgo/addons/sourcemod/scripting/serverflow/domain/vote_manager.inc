#if defined _SERVERFLOW_VOTE_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_VOTE_MANAGER_INC_

void Vote_Reset()
{
    g_VoteInProgress = false;
    for (int i = 0; i < MAX_MODES; i++)
    {
        g_VoteCounts[i] = 0;
    }

    for (int i = 1; i <= MaxClients; i++)
    {
        g_HasVoted[i] = false;
    }
}

void Vote_Start(int caller)
{
    if (g_VoteInProgress)
    {
        return;
    }

    Vote_Reset();
    g_VoteInProgress = true;
    g_VoteTimer = CreateTimer(g_VoteDuration, Timer_FinishVote);
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
    if (!g_VoteInProgress || modeIndex < 0 || modeIndex >= MAX_MODES)
    {
        return;
    }

    if (g_HasVoted[client])
    {
        return;
    }

    g_HasVoted[client] = true;
    g_VoteCounts[modeIndex]++;
}

public Action Timer_FinishVote(Handle timer)
{
    g_VoteTimer = null;
    g_VoteInProgress = false;
    g_NextVoteAllowedAt = GetTime() + g_VoteCooldown;

    int winner = 0;
    for (int i = 1; i < MAX_MODES; i++)
    {
        if (g_VoteCounts[i] > g_VoteCounts[winner])
        {
            winner = i;
        }
    }

    Transition_QueueScenario(g_Modes[winner].id, 0);
    Pending_Confirm();

    Call_StartForward(g_FwdOnVoteCompleted);
    Call_PushString(g_Modes[winner].id);
    Call_PushCell(g_VoteCounts[winner]);
    Call_Finish();

    PrintToChatAll("[ServerFlow] Vote finished: %s", g_Modes[winner].id);
    return Plugin_Stop;
}
