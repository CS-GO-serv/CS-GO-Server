#if defined _SERVERFLOW_GAME_EVENTS_INC_
 #endinput
#endif
#define _SERVERFLOW_GAME_EVENTS_INC_

void Integrations_RegisterEvents()
{
    HookEvent("round_start", Event_RoundStart, EventHookMode_PostNoCopy);
}

public void Event_RoundStart(Event event, const char[] name, bool dontBroadcast)
{
    ConfigCore_RefreshRuntime();
}
