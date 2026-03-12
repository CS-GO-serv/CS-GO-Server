#if defined _SERVERFLOW_STATE_DUMP_INC_
 #endinput
#endif
#define _SERVERFLOW_STATE_DUMP_INC_

void StateDump_ToServer()
{
    PrintToServer("[ServerFlow Dump] state=%d current=%s queued=%s", g_Session.currentState, g_Session.currentScenario, g_Session.queuedScenario);
}
