#if defined _SERVERFLOW_HEALTH_CHECK_INC_
 #endinput
#endif
#define _SERVERFLOW_HEALTH_CHECK_INC_

void Health_RunFullCheck()
{
    ModeDebug("health check: modes=%d state=%d", Scenario_GetCount(), g_Session.currentState);
}
