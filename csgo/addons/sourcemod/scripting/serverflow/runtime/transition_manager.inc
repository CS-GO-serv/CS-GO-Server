#if defined _SERVERFLOW_TRANSITION_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_TRANSITION_MANAGER_INC_

void Transition_QueueScenario(const char[] scenarioId, int actor, const char[] source = "scenario_queue")
{
    Session_SetQueuedScenario(scenarioId);
    Pending_Create(scenarioId, "", source, actor);
}

void Transition_QueueMap(const char[] mapName, int actor, const char[] source = "map_queue")
{
    char currentScenario[16];
    Session_GetCurrentScenario(currentScenario, sizeof(currentScenario));
    Pending_Create(currentScenario, mapName, source, actor);
}

void Transition_CancelPending(const char[] reason = "cancel")
{
    Pending_Cancel(reason);
}

static void Transition_ApplyScenario(const char[] scenarioId)
{
    int scenarioIndex = FindModeById(scenarioId);
    if (scenarioIndex == -1)
    {
        ModeDebug("transition apply rejected: unknown scenario %s", scenarioId);
        return;
    }

    Session_SetCurrentScenario(scenarioId);
    Session_SetQueuedScenario(scenarioId);

    if (g_Scenarios[scenarioIndex].cfgFile[0] != '\0')
    {
        ServerCommand("exec %s", g_Scenarios[scenarioIndex].cfgFile);
    }

    if (g_Scenarios[scenarioIndex].mapgroup[0] != '\0')
    {
        ServerCommand("mapgroup %s", g_Scenarios[scenarioIndex].mapgroup);
    }
}

bool Transition_ApplyPending(const char[] source = "")
{
    PendingChange pending;
    Session_GetPendingChange(pending);

    if (pending.state != Pending_Confirmed)
    {
        return false;
    }

    Session_SetTransitionType(TransitionType_ApplyPending);
    Session_SetPendingChangeState(Pending_Applying);

    if (pending.scenarioId[0] != '\0')
    {
        Transition_ApplyScenario(pending.scenarioId);
    }

    if (pending.mapName[0] != '\0' || pending.scenarioId[0] != '\0')
    {
        char resolvedMap[64];
        Playlist_ResolveNextMap(pending.scenarioId, pending.mapName, resolvedMap, sizeof(resolvedMap));
        Session_SetQueuedMap(resolvedMap);
        ServerCommand("changelevel %s", resolvedMap);
    }

    ModeDebug("pending applied: scenario=%s map=%s source=%s requested_source=%s", pending.scenarioId, pending.mapName, source, pending.source);

    Session_ClearPendingChange();
    return true;
}
