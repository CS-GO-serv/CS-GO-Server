#if defined _SERVERFLOW_STATUS_PRESENTER_INC_
 #endinput
#endif
#define _SERVERFLOW_STATUS_PRESENTER_INC_

void Status_ShowToClient(int client)
{
    char line[192];
    Status_GetCompactText(line, sizeof(line));
    PrintToChat(client, "%s", line);
}

void Status_GetCompactText(char[] buffer, int maxlen)
{
    char currentScenario[16];
    char queuedScenario[16];
    Session_GetCurrentScenario(currentScenario, sizeof(currentScenario));
    Session_GetQueuedScenario(queuedScenario, sizeof(queuedScenario));

    Format(buffer, maxlen, "[ServerFlow] state=%d scenario=%s queued=%s", Session_GetState(), currentScenario, queuedScenario);
}
