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
    HookEvent("round_end", Event_RoundEnd, EventHookMode_PostNoCopy);
    HookEvent("cs_win_panel_match", Event_MatchCompleted, EventHookMode_PostNoCopy);
    HookEvent("game_end", Event_MatchCompleted, EventHookMode_PostNoCopy);
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
