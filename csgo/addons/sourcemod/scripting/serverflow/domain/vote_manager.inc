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
    
    // Check for Threshold auto-finish
    if (g_VotePolicy == VotePolicy_Threshold || g_VoteThreshold > 0)
    {
        if (Session_GetVoteCount(modeIndex) >= g_VoteThreshold)
        {
            ModeDebug("Vote Threshold reached for scenario %d (%d votes). Finishing vote early.", modeIndex, g_VoteThreshold);
            // Delete the existing timer and trigger finish immediately
            Handle timer = Session_GetVoteTimer();
            if (timer != null)
            {
                delete timer;
                Session_SetVoteTimer(null);
            }
            Timer_FinishVote(null);
        }
    }

    reason[0] = '\0';
    return true;
}

public Action Timer_FinishVote(Handle timer)
{
    Session_FinishVote();

    int winner = 0;
    int totalVotes = 0;
    int scenarioCount = Scenario_GetCount();

    for (int i = 0; i < scenarioCount; i++)
    {
        totalVotes += Session_GetVoteCount(i);
    }

    if (totalVotes > 0)
    {
        // 1. Quorum Check
        int humanCount = 0;
        for (int i = 1; i <= MaxClients; i++)
        {
            if (IsClientInGame(i) && !IsFakeClient(i)) humanCount++;
        }
        
        if (humanCount > 0 && (float(totalVotes) / float(humanCount)) < g_VoteQuorum)
        {
            ModeDebug("Vote Invalid: Quorum not met (%d/%d vs required %.1f%%)", totalVotes, humanCount, g_VoteQuorum * 100.0);
            totalVotes = 0; // Force fallback logic
        }
    }

    if (totalVotes > 0)
    {
        if (g_VotePolicy == VotePolicy_Roulette)
        {
            float totalWeightedVotes = 0.0;
            float weights[MAX_RUNTIME_MODES];
            
            for (int i = 0; i < scenarioCount; i++)
            {
                int votes = Session_GetVoteCount(i);
                if (votes == 0) { weights[i] = 0.0; continue; }
                
                weights[i] = float(votes);
                if (g_VoteHistoryBias)
                {
                    char id[16]; Scenario_GetIdByIndex(i, id, sizeof(id));
                    if (Session_IsInHistory(id))
                    {
                        if (g_VoteHistoryMode == 1) // Block
                        {
                            weights[i] = 0.0;
                        }
                        else // Weight
                        {
                            weights[i] *= g_VoteHistoryWeight;
                        }
                    }
                }
                totalWeightedVotes += weights[i];
            }
            
            float roll = GetRandomFloat(0.0, totalWeightedVotes);
            float currentSum = 0.0;
            for (int i = 0; i < scenarioCount; i++)
            {
                currentSum += weights[i];
                if (roll <= currentSum)
                {
                    winner = i;
                    break;
                }
            }
            ModeDebug("Vote Roulette: weighted total %.1f, rolled %.1f, winner index %d", totalWeightedVotes, roll, winner);
        }
        else if (g_VotePolicy == VotePolicy_EqualWeight)
        {
            float totalWeights = 0.0;
            float scenarioWeights[MAX_RUNTIME_MODES];
            int candidates = 0;
            
            for (int i = 0; i < scenarioCount; i++)
            {
                if (Session_GetVoteCount(i) > 0)
                {
                    scenarioWeights[i] = 1.0;
                    if (g_VoteHistoryBias)
                    {
                        char id[16]; Scenario_GetIdByIndex(i, id, sizeof(id));
                        if (Session_IsInHistory(id))
                        {
                            if (g_VoteHistoryMode == 1) // Block
                            {
                                scenarioWeights[i] = 0.0;
                            }
                            else // Weighting
                            {
                                scenarioWeights[i] *= g_VoteHistoryWeight;
                            }
                        }
                    }
                    if (scenarioWeights[i] > 0.0)
                    {
                        totalWeights += scenarioWeights[i];
                        candidates++;
                    }
                }
                else scenarioWeights[i] = 0.0;
            }
            
            float roll = GetRandomFloat(0.0, totalWeights);
            float curSum = 0.0;
            for (int i = 0; i < scenarioCount; i++)
            {
                curSum += scenarioWeights[i];
                if (roll <= curSum)
                {
                    winner = i;
                    break;
                }
            }
            ModeDebug("Vote EqualWeight: %d candidates, total weight %.1f, rolled %.1f, winner %d", candidates, totalWeights, roll, winner);
        }
        else if (g_VotePolicy == VotePolicy_RandomTop2 || g_VotePolicy == VotePolicy_Threshold)
        {
            int first = -1, second = -1;
            for (int i = 0; i < scenarioCount; i++)
            {
                int count = Session_GetVoteCount(i);
                if (count == 0) continue;
                
                if (first == -1 || count > Session_GetVoteCount(first))
                {
                    second = first;
                    first = i;
                }
                else if (second == -1 || count > Session_GetVoteCount(second))
                {
                    second = i;
                }
            }
            
            if (second == -1 || GetRandomInt(0, 1) == 0) winner = first;
            else winner = second;
        }
        else // VotePolicy_Majority
        {
            for (int i = 1; i < scenarioCount; i++)
            {
                if (Session_GetVoteCount(i) > Session_GetVoteCount(winner))
                {
                    winner = i;
                }
            }
            
            // Consensus Check for Majority
            if (g_VoteConsensus > 0.0 && totalVotes > 0)
            {
                if ((float(Session_GetVoteCount(winner)) / float(totalVotes)) < g_VoteConsensus)
                {
                    ModeDebug("Vote Consensus failed: winner has %d/%d votes (required %.1f%%)", Session_GetVoteCount(winner), totalVotes, g_VoteConsensus * 100.0);
                    totalVotes = 0; // Force fallback
                }
            }
        }
    }
    else
    {
        // No votescast - use default behavior (continue current)
        char currentScenario[32];
        Session_GetCurrentScenario(currentScenario, sizeof(currentScenario));
        winner = Scenario_GetIndexById(currentScenario);
        ModeDebug("No votes cast, sticking with current scenario: %s", currentScenario);
    }

    char winnerScenario[32];
    Scenario_GetIdByIndex(winner, winnerScenario, sizeof(winnerScenario));

    ServerState state = Session_GetState();
    PendingConfirmPolicy confirmPolicy = (state == ServerState_PostMatch) ? PendingConfirm_Auto : PendingConfirm_Manual;
    char voteReason[96];
    if (totalVotes > 0)
    {
        char policyName[16];
        switch (g_VotePolicy)
        {
            case VotePolicy_Majority: strcopy(policyName, sizeof(policyName), "majority");
            case VotePolicy_Roulette: strcopy(policyName, sizeof(policyName), "roulette");
            case VotePolicy_EqualWeight: strcopy(policyName, sizeof(policyName), "equal_weight");
            case VotePolicy_RandomTop2: strcopy(policyName, sizeof(policyName), "top_2_random");
            case VotePolicy_Threshold: strcopy(policyName, sizeof(policyName), "threshold");
        }
        Format(voteReason, sizeof(voteReason), "vote winner (%d/%d votes, policy: %s)", 
            Session_GetVoteCount(winner), totalVotes, policyName);
    }
    else
    {
        strcopy(voteReason, sizeof(voteReason), "no votes cast, fallback to current");
    }

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
