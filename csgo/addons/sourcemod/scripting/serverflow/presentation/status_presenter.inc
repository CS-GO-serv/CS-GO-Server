#if defined _SERVERFLOW_STATUS_PRESENTER_INC_
 #endinput
#endif
#define _SERVERFLOW_STATUS_PRESENTER_INC_

void Status_ShowToClient(int client)
{
    PrintToChat(client, "[ServerFlow] state=%d scenario=%s queued=%s", g_Session.currentState, g_Session.currentScenario, g_Session.queuedScenario);
}
