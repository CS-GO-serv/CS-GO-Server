#if defined _SERVERFLOW_STATUS_PRESENTER_INC_
 #endinput
#endif
#define _SERVERFLOW_STATUS_PRESENTER_INC_

static void Status_GetNextActionText(char[] buffer, int maxlen)
{
    PendingChange pending;
    Session_GetPendingChange(pending);

    PendingChangeState pendingState = Session_GetPendingChangeState();
    if (pendingState == Pending_Proposed || pendingState == Pending_Selected)
    {
        if (pending.confirmPolicy == PendingConfirm_Auto)
        {
            char allowedStateText[24];
            ServerStateToString(pending.allowedApplyState, allowedStateText, sizeof(allowedStateText));
            Format(buffer, maxlen, "auto-apply pending change when state is %s", allowedStateText);
        }
        else
        {
            strcopy(buffer, maxlen, "await admin confirm (sm_confirmpending) or cancel");
        }
        return;
    }

    if (pendingState == Pending_Confirmed)
    {
        strcopy(buffer, maxlen, "apply confirmed pending change on next transition check");
        return;
    }

    if (pendingState == Pending_Applying)
    {
        strcopy(buffer, maxlen, "finish transition and apply queued scenario/map");
        return;
    }

    ServerState state = Session_GetState();
    if (state == ServerState_PreMatch)
    {
        PreMatchPhase phase = Session_GetPreMatchPhase();
        switch (phase)
        {
            case PreMatch_Setup: strcopy(buffer, maxlen, "teams setup, then ready-check");
            case PreMatch_Teams: strcopy(buffer, maxlen, "team assignment, then ready-check");
            case PreMatch_Ready: strcopy(buffer, maxlen, "wait for players ready and countdown");
            case PreMatch_Lock: strcopy(buffer, maxlen, "countdown lock, then enter Match");
            default: strcopy(buffer, maxlen, "continue pre-match flow");
        }
        return;
    }

    switch (state)
    {
        case ServerState_Lobby: strcopy(buffer, maxlen, "wait for scenario selection/vote");
        case ServerState_Match: strcopy(buffer, maxlen, "wait for match end -> PostMatch");
        case ServerState_PostMatch: strcopy(buffer, maxlen, "return to Lobby or apply pending scenario");
        case ServerState_Transition: strcopy(buffer, maxlen, "finish transition and enter target state");
        default: strcopy(buffer, maxlen, "no action available");
    }
}

void Status_ShowToClient(int client)
{
    char line[256];
    Status_GetCompactText(line, sizeof(line));
    PrintToChat(client, "%s", line);
}

void Status_ShowVerboseToClient(int client)
{
    char line[256];
    Status_GetVerboseText(line, sizeof(line));
    PrintToChat(client, "%s", line);
}

void Status_GetCompactText(char[] buffer, int maxlen)
{
    char currentScenario[16];
    char queuedScenario[16];
    char stateText[24];
    char nextAction[96];

    Session_GetCurrentScenario(currentScenario, sizeof(currentScenario));
    Session_GetQueuedScenario(queuedScenario, sizeof(queuedScenario));

    ServerStateToString(Session_GetState(), stateText, sizeof(stateText));
    Status_GetNextActionText(nextAction, sizeof(nextAction));

    Format(
        buffer,
        maxlen,
        "[ServerFlow] current=%s next=%s state=%s next_action=%s",
        currentScenario,
        queuedScenario,
        stateText,
        nextAction
    );
}

void Status_GetVerboseText(char[] buffer, int maxlen)
{
    char currentScenario[16];
    char queuedScenario[16];
    char stateText[24];
    char preMatchText[24];
    char pendingStateText[24];
    char allowedStateText[24];
    char confirmText[16];
    char nextAction[96];

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

    int pendingLeft = pending.expiresAt > 0 ? pending.expiresAt - GetTime() : 0;
    if (pendingLeft < 0)
    {
        pendingLeft = 0;
    }

    ServerStateToString(Session_GetState(), stateText, sizeof(stateText));
    PreMatchPhaseToString(Session_GetPreMatchPhase(), preMatchText, sizeof(preMatchText));
    PendingChangeStateToString(pending.state, pendingStateText, sizeof(pendingStateText));
    ServerStateToString(pending.allowedApplyState, allowedStateText, sizeof(allowedStateText));
    strcopy(confirmText, sizeof(confirmText), pending.confirmPolicy == PendingConfirm_Auto ? "auto" : "manual");
    Status_GetNextActionText(nextAction, sizeof(nextAction));

    Format(
        buffer,
        maxlen,
        "[ServerFlow] current=%s next=%s state=%s prematch=%s ready=%d/%d cd=%d pending=%s apply_state=%s apply_in=%d confirm=%s next_action=%s",
        currentScenario,
        queuedScenario,
        stateText,
        preMatchText,
        readyCount,
        readyPool,
        countdownLeft,
        pendingStateText,
        allowedStateText,
        pendingLeft,
        confirmText,
        nextAction
    );
}

static Handle g_HudTimer = null;

void StatusPresenter_Init()
{
    if (g_HudTimer != null)
    {
        delete g_HudTimer;
    }
    
    g_HudTimer = CreateTimer(1.0, Timer_DrawHUD, _, TIMER_REPEAT);
}

public Action Timer_DrawHUD(Handle timer)
{
    ServerState state = Session_GetState();
    
    // Do not spam the HUD during active matches or transitions where it might distract.
    // It's most critical in Lobby, PreMatch, and PostMatch decision phases.
    if (state == ServerState_Match || state == ServerState_Transition)
    {
        return Plugin_Continue;
    }
    
    char hudLine[512];
    char currentScenario[32];
    char queuedScenario[32];
    char stateText[32];
    char countText[128];
    char pendingText[128];
    
    Session_GetCurrentScenario(currentScenario, sizeof(currentScenario));
    Session_GetQueuedScenario(queuedScenario, sizeof(queuedScenario));
    ServerStateToString(state, stateText, sizeof(stateText));
    
    char stateColor[16];
    if (state == ServerState_Lobby) strcopy(stateColor, sizeof(stateColor), "#00c3ff");
    else if (state == ServerState_PreMatch) strcopy(stateColor, sizeof(stateColor), "#ffcc00");
    else if (state == ServerState_PostMatch) strcopy(stateColor, sizeof(stateColor), "#ff3366");
    else strcopy(stateColor, sizeof(stateColor), "#ffffff");

    char scenarioLine[128];
    if (queuedScenario[0] == '\0' || StrEqual(currentScenario, queuedScenario))
    {
        Format(scenarioLine, sizeof(scenarioLine), "PLAYING: <font color='#00ff00'>%s</font>", currentScenario);
    }
    else
    {
        Format(scenarioLine, sizeof(scenarioLine), "PLAYING: %s  |  NEXT: <font color='#ffaa00'>%s</font>", currentScenario, queuedScenario);
    }

    Format(hudLine, sizeof(hudLine), "<font color='%s'><b>[ %s ]</b></font>   %s\n", stateColor, stateText, scenarioLine);
    
    int readyCount = ReadyManager_GetReadyCount();
    int readyPool = ReadyManager_GetPoolCount();
    
    if (Session_IsCountdownActive())
    {
        int countdownLeft = Session_GetCountdownEndsAt() - GetTime();
        if (countdownLeft < 0) countdownLeft = 0;
        Format(countText, sizeof(countText), "\n<font color='#ff0000'><b>STARTING IN: %d SEC</b></font>\n", countdownLeft);
    }
    else
    {
        Format(countText, sizeof(countText), "\n<font color='#cccccc'>Waiting for players (%d / %d Ready)</font>\n", readyCount, readyPool);
    }
    StrCat(hudLine, sizeof(hudLine), countText);
    
    PendingChange pending;
    Session_GetPendingChange(pending);
    if (pending.state == Pending_Proposed || pending.state == Pending_Selected)
    {
        int pendingLeft = pending.expiresAt > 0 ? pending.expiresAt - GetTime() : 0;
        if (pendingLeft < 0) pendingLeft = 0;
        Format(pendingText, sizeof(pendingText), "<font color='#ff5500'>Pending Vote Confirmed. Auto-Cancel in: %ds</font>", pendingLeft);
        StrCat(hudLine, sizeof(hudLine), pendingText);
    }
    
    PrintHintTextToAll("%s", hudLine);
    
    return Plugin_Continue;
}
