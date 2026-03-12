#if defined _SERVERFLOW_SESSION_STORE_INC_
 #endinput
#endif
#define _SERVERFLOW_SESSION_STORE_INC_

void Session_Init()
{
    g_Session.currentState = ServerState_Lobby;
    strcopy(g_Session.currentScenario, sizeof(g_Session.currentScenario), "lobby");
    g_Session.queuedScenario[0] = '\0';
    GetCurrentMap(g_Session.currentMap, sizeof(g_Session.currentMap));
    g_Session.queuedMap[0] = '\0';
    g_Session.lockPhase = false;
    g_Session.lastStateChangeTs = GetTime();
}

void Session_SetState(ServerState state)
{
    g_Session.currentState = state;
    g_Session.lastStateChangeTs = GetTime();
}

ServerState Session_GetState()
{
    return g_Session.currentState;
}

void Session_SetCurrentScenario(const char[] scenarioId)
{
    strcopy(g_Session.currentScenario, sizeof(g_Session.currentScenario), scenarioId);
}

void Session_SetQueuedScenario(const char[] scenarioId)
{
    strcopy(g_Session.queuedScenario, sizeof(g_Session.queuedScenario), scenarioId);
}

void Session_SetPendingChange(const char[] scenarioId, const char[] mapName, int actor)
{
    g_PendingChange.state = Pending_Queued;
    strcopy(g_PendingChange.scenarioId, sizeof(g_PendingChange.scenarioId), scenarioId);
    strcopy(g_PendingChange.mapName, sizeof(g_PendingChange.mapName), mapName);
    g_PendingChange.requestedBy = actor;
    g_PendingChange.requestedAt = GetTime();
}

void Session_ClearPendingChange()
{
    g_PendingChange.state = Pending_None;
    g_PendingChange.scenarioId[0] = '\0';
    g_PendingChange.mapName[0] = '\0';
    g_PendingChange.requestedBy = 0;
    g_PendingChange.requestedAt = 0;
}
