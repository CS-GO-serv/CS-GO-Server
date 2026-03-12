#if defined _SERVERFLOW_TRANSITION_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_TRANSITION_MANAGER_INC_

void Transition_QueueScenario(const char[] scenarioId, int actor)
{
    Session_SetQueuedScenario(scenarioId);
    Pending_Create(scenarioId, "", actor);
}

void Transition_QueueMap(const char[] mapName, int actor)
{
    char currentScenario[16];
    Session_GetCurrentScenario(currentScenario, sizeof(currentScenario));
    Pending_Create(currentScenario, mapName, actor);
}

bool Transition_ApplyPending()
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
        Session_SetCurrentScenario(pending.scenarioId);
    }

    if (pending.mapName[0] != '\0')
    {
        Session_SetQueuedMap(pending.mapName);
    }

    Session_ClearPendingChange();
    return true;
}
