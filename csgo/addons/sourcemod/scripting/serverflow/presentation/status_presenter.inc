#if defined _SERVERFLOW_STATUS_PRESENTER_INC_
 #endinput
#endif
#define _SERVERFLOW_STATUS_PRESENTER_INC_

void Status_ShowToClient(int client)
{
    char line[256];
    Status_GetCompactText(line, sizeof(line));
    PrintToChat(client, "%s", line);
}

void Status_GetCompactText(char[] buffer, int maxlen)
{
    char currentScenario[16];
    char queuedScenario[16];
    Session_GetCurrentScenario(currentScenario, sizeof(currentScenario));
    Session_GetQueuedScenario(queuedScenario, sizeof(queuedScenario));

    PendingChange pending;
    Session_GetPendingChange(pending);

    int readyCount = ReadyManager_GetReadyCount();
    int readyPool = ReadyManager_GetPoolCount();
    int countdownLeft = Session_IsCountdownActive() ? Session_GetCountdownEndsAt() - GetTime() : 0;
    if (countdownLeft < 0)
    {
        countdownLeft = 0;
    }

    Format(
        buffer,
        maxlen,
        "[ServerFlow] state=%d prematch=%d lock=%d ready=%d/%d countdown=%d pending=%d scenario=%s queued=%s",
        Session_GetState(),
        Session_GetPreMatchPhase(),
        Session_IsLockPhase(),
        readyCount,
        readyPool,
        countdownLeft,
        pending.state,
        currentScenario,
        queuedScenario
    );
}
