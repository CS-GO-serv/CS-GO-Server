#if defined _SERVERFLOW_LOBBY_MANAGER_INC_
    #endinput
#endif
#define _SERVERFLOW_LOBBY_MANAGER_INC_

void LobbyManager_OnEnterLobby(const char[] reason)
{
    ModeDebug("Lobby: entered Lobby state (reason: %s)", reason);
    
    // In Lobby, we usually want to reset everything and wait for a scenario selection.
    // If we want a 'warmup' or 'gathering' period, we can trigger it here.
    
    // Clear any previous queued scenarios to avoid accidental auto-start
    Session_SetQueuedScenario("");
    
    // Notify players that they can use !play or !vote
    // (This is usually handled by StatusPresenter putting !play on the screen)
}
