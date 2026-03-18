#if defined _SERVERFLOW_GAME_EVENTS_INC_
 #endinput
#endif
#define _SERVERFLOW_GAME_EVENTS_INC_

static void GameEvents_OrchestrateEnterMatch(const char[] source)
{
    Transition_TryEnterMatchFromLock(source);
}

static void GameEvents_OrchestrateEnterPostMatch(const char[] source)
{
    Transition_TryEnterPostMatchFromMatch(source);
}

void Integrations_RegisterEvents()
{
    HookEvent("round_start", Event_RoundStart, EventHookMode_PostNoCopy);
<<<<<<< Updated upstream
    HookEvent("round_end", Event_RoundEnd, EventHookMode_PostNoCopy);
    HookEvent("cs_win_panel_match", Event_MatchCompleted, EventHookMode_PostNoCopy);
    HookEvent("game_end", Event_MatchCompleted, EventHookMode_PostNoCopy);
=======
    HookEvent("round_announce_match_start", Event_MatchStart, EventHookMode_PostNoCopy);
    HookEvent("cs_win_panel_match", Event_MatchEnd, EventHookMode_PostNoCopy);
>>>>>>> Stashed changes
}

public void Event_RoundStart(Event event, const char[] name, bool dontBroadcast)
{
    ConfigCore_RefreshRuntime();
    GameEvents_OrchestrateEnterMatch("event_round_start");
}

public void Event_RoundEnd(Event event, const char[] name, bool dontBroadcast)
{
    GameEvents_OrchestrateEnterPostMatch("event_round_end");
}

public void Event_MatchCompleted(Event event, const char[] name, bool dontBroadcast)
{
    GameEvents_OrchestrateEnterPostMatch("event_match_completed");
}

public void Event_MatchStart(Event event, const char[] name, bool dontBroadcast)
{
    if (Session_GetState() != ServerState_Match)
    {
        State_EnterMatch("game_event_match_start");
    }
}

public void Event_MatchEnd(Event event, const char[] name, bool dontBroadcast)
{
    if (Session_GetState() != ServerState_PostMatch)
    {
        State_EnterPostMatch("game_event_match_end");
    }
}

