#if defined _SERVERFLOW_POSTMATCH_MANAGER_INC_
    #endinput
#endif
#define _SERVERFLOW_POSTMATCH_MANAGER_INC_

static Handle g_PostMatchTimer = null;

void PostMatchManager_Init()
{
    g_PostMatchTimer = null;
}

static void PostMatch_StopTimer()
{
    if (g_PostMatchTimer != null)
    {
        delete g_PostMatchTimer;
        g_PostMatchTimer = null;
    }
}

void PostMatchManager_OnEnterPostMatch(const char[] reason)
{
    PostMatch_StopTimer();
    
    Session_SetPostMatchPhase(PostMatch_Results);
    ModeDebug("PostMatch: entered Results phase (reason: %s)", reason);
    
    // Show results for 10 seconds, then move to decision
    g_PostMatchTimer = CreateTimer(10.0, Timer_PostMatchToDecision);
}

public Action Timer_PostMatchToDecision(Handle timer)
{
    g_PostMatchTimer = null;
    
    if (Session_GetState() != ServerState_PostMatch)
    {
        return Plugin_Stop;
    }
    
    Session_SetPostMatchPhase(PostMatch_Decision);
    
    // [NEW] Idle Protection Check
    if (CountHumanPlayers() < g_VoteMinPlayers)
    {
        ModeDebug("PostMatch Idle Protection: Less than %d players. Halting rotation to allow warmup.", g_VoteMinPlayers);
        
        char currentScenario[16];
        Session_GetCurrentScenario(currentScenario, sizeof(currentScenario));
        
        Pending_Create(currentScenario, "", "idle_protection", 0, "idle_protection", ServerState_PostMatch, PendingConfirm_Auto);
        Transition_ConfirmAndApplyPending(0, "idle_protection");
        
        return Plugin_Stop;
    }
    
    char currentScenario[16];
    Session_GetCurrentScenario(currentScenario, sizeof(currentScenario));
    int scenarioIndex = FindModeById(currentScenario);
    
    int decisionTime = g_PostMatchDecisionTime;
    if (scenarioIndex != -1 && g_Scenarios[scenarioIndex].postMatchDecisionTime > 0)
    {
        decisionTime = g_Scenarios[scenarioIndex].postMatchDecisionTime;
    }
    
    ModeDebug("PostMatch: entered Decision phase (time: %d)", decisionTime);
    
    // Start voting if policy says so? 
    // Usually a vote should have been started during Match or early PostMatch.
    // If no vote is running, and policy is Vote, start it now.
    
    if (scenarioIndex != -1 && g_Scenarios[scenarioIndex].postMatchDefaultAction == PostMatchAction_Vote)
    {
        if (!Session_IsVoteInProgress())
        {
             char voteReason[64];
             VoteManager_RequestVoteStart(0, voteReason, sizeof(voteReason));
        }
    }

    g_PostMatchTimer = CreateTimer(float(decisionTime), Timer_PostMatchDecisionTimeout);
    return Plugin_Stop;
}

public Action Timer_PostMatchDecisionTimeout(Handle timer)
{
    g_PostMatchTimer = null;
    
    if (Session_GetState() != ServerState_PostMatch)
    {
        return Plugin_Stop;
    }
    
    ModeDebug("PostMatch: decision timeout reached");
    
    if (Session_GetPendingChangeState() != Pending_None)
    {
        ModeDebug("PostMatch: Pending change exists, confirming...");
        Transition_ConfirmAndApplyPending(0, "postmatch_timeout_confirm");
    }
    else
    {
        PostMatchManager_ExecuteDefaultAction();
    }
    
    return Plugin_Stop;
}

void PostMatchManager_ExecuteDefaultAction()
{
    char currentScenarioId[16];
    Session_GetCurrentScenario(currentScenarioId, sizeof(currentScenarioId));
    int scenarioIndex = FindModeById(currentScenarioId);
    
    PostMatchDefaultAction action = PostMatchAction_Stay;
    if (scenarioIndex != -1)
    {
        action = g_Scenarios[scenarioIndex].postMatchDefaultAction;
    }
    
    ModeDebug("PostMatch: executing default action %d for scenario %s", action, currentScenarioId);
    
    if (action == PostMatchAction_Lobby)
    {
        Pending_Create("lobby", "", "postmatch_default_lobby", 0, "default_action_lobby", ServerState_PostMatch, PendingConfirm_Auto);
    }
    else if (action == PostMatchAction_Rotate)
    {
        // Resolve next map in current scenario
        char nextMap[64];
        if (Playlist_ResolveNextMap(currentScenarioId, "", nextMap, sizeof(nextMap)))
        {
             Pending_Create(currentScenarioId, nextMap, "postmatch_default_rotate", 0, "default_action_rotate", ServerState_PostMatch, PendingConfirm_Auto);
        }
        else
        {
             Pending_Create("lobby", "", "postmatch_fallback_lobby", 0, "map_resolution_failed", ServerState_PostMatch, PendingConfirm_Auto);
        }
    }
    else if (action == PostMatchAction_Stay)
    {
         // Just restart current map or do nothing? 
         // We'll treat it as "Rotate" to next map in same mode by default if it's a playlist.
         char nextMap[64];
         if (Playlist_ResolveNextMap(currentScenarioId, "", nextMap, sizeof(nextMap)))
         {
              Pending_Create(currentScenarioId, nextMap, "postmatch_default_stay", 0, "default_action_stay", ServerState_PostMatch, PendingConfirm_Auto);
         }
         else
         {
              ServerCommand("mp_restartgame 5"); // Just restart the match
              Notify_StateChanged(ServerState_PostMatch, ServerState_PreMatch, "stay_restart");
              State_EnterPreMatch("stay_restart");
         }
    }
    else if (action == PostMatchAction_Vote)
    {
        // If we are here, a vote should have finished or we fallback to Lobby
        Pending_Create("lobby", "", "postmatch_vote_fallback_lobby", 0, "vote_failed_no_result", ServerState_PostMatch, PendingConfirm_Auto);
    }
    
    Transition_ApplyPending("postmatch_default_action");
}
