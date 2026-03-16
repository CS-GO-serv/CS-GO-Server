#if defined _SERVERFLOW_ADMIN_OVERRIDE_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_ADMIN_OVERRIDE_MANAGER_INC_

bool AdminOverride_RequestScenario(int actor, const char[] scenarioId, const char[] source = "admin_override")
{
    char reason[48];
    if (!State_CanClientManageTransitions(actor, reason, sizeof(reason)))
    {
        return false;
    }

    if (!Scenario_Exists(scenarioId))
    {
        return false;
    }

    Session_SetTransitionType(TransitionType_Fallback);
    Transition_CancelPending("override_replace");
    Transition_QueueScenario(scenarioId, actor, source);
    Notify_AdminOverrideQueued(actor, scenarioId);
    return true;
}
