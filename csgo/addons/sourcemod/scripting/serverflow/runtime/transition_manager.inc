#if defined _SERVERFLOW_TRANSITION_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_TRANSITION_MANAGER_INC_

void Transition_QueueScenario(const char[] scenarioId, int actor)
{
    Session_SetQueuedScenario(scenarioId);
    Session_SetPendingChange(scenarioId, "", actor);
}

void Transition_QueueMap(const char[] mapName, int actor)
{
    Session_SetPendingChange(g_Session.currentScenario, mapName, actor);
}

bool Transition_ApplyPending()
{
    if (g_PendingChange.state != Pending_Confirmed)
    {
        return false;
    }

    if (g_PendingChange.scenarioId[0] != '\0')
    {
        Session_SetCurrentScenario(g_PendingChange.scenarioId);
    }

    if (g_PendingChange.mapName[0] != '\0')
    {
        strcopy(g_Session.queuedMap, sizeof(g_Session.queuedMap), g_PendingChange.mapName);
    }

    Session_ClearPendingChange();
    return true;
}
