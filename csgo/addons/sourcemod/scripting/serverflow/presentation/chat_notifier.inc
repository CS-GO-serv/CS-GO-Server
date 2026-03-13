#if defined _SERVERFLOW_CHAT_NOTIFIER_INC_
 #endinput
#endif
#define _SERVERFLOW_CHAT_NOTIFIER_INC_

void Notify_StateChanged(ServerState oldState, ServerState newState, const char[] reason)
{
    PrintToChatAll("[ServerFlow] state %d -> %d (%s)", oldState, newState, reason);
}

void Notify_VoteStarted(int caller)
{
    PrintToChatAll("[ServerFlow] Vote started by %N.", caller);
}

void Notify_VoteFinished(const char[] scenarioId)
{
    PrintToChatAll("[ServerFlow] Vote complete: selected mode %s.", scenarioId);
}

void Notify_PendingCreated(const char[] scenarioId, const char[] mapName, int seconds, const char[] why, ServerState allowedApplyState, PendingConfirmPolicy confirmPolicy)
{
    PrintToChatAll("[ServerFlow] Selected next mode: %s (map: %s).", scenarioId, mapName[0] == '\0' ? "auto" : mapName);
    PrintToChatAll("[ServerFlow] Reason: %s", why[0] == '\0' ? "no reason provided" : why);

    if (confirmPolicy == PendingConfirm_Manual)
    {
        PrintToChatAll("[ServerFlow] Needs admin confirmation in %d sec (sm_confirmpending).", seconds);
    }
    else
    {
        PrintToChatAll("[ServerFlow] Waiting to apply automatically when state=%d is reached.", allowedApplyState);
    }
}

void Notify_PendingSelected(const char[] scenarioId, const char[] mapName, const char[] why, int expiresAt)
{
    int left = expiresAt - GetTime();
    if (left < 0)
    {
        left = 0;
    }

    PrintToChatAll("[ServerFlow] Selected: %s (map: %s).", scenarioId, mapName[0] == '\0' ? "auto" : mapName);
    PrintToChatAll("[ServerFlow] Confirm to continue. Reason: %s. Deadline: %d sec.", why[0] == '\0' ? "no reason provided" : why, left);
}

void Notify_PendingConfirmed(int actor, const char[] scenarioId, const char[] mapName)
{
    PrintToChatAll("[ServerFlow] %N confirmed transition to %s (map: %s).", actor, scenarioId, mapName[0] == '\0' ? "auto" : mapName);
}

void Notify_PendingCanceled(const char[] reason, const char[] why, const char[] scenarioId, const char[] mapName)
{
    PrintToChatAll("[ServerFlow] Transition canceled for %s (map: %s).", scenarioId[0] == '\0' ? "current mode" : scenarioId, mapName[0] == '\0' ? "auto" : mapName);
    PrintToChatAll("[ServerFlow] Cancel reason: %s%s%s.", reason, why[0] == '\0' ? "" : " / ", why);
}

void Notify_ReadyChanged(int client, bool ready, int readyCount, int poolCount)
{
    PrintToChatAll("[ServerFlow] %N is now %s (%d/%d).", client, ready ? "READY" : "UNREADY", readyCount, poolCount);
}

void Notify_CountdownStarted(int seconds, const char[] source)
{
    PrintToChatAll("[ServerFlow] Countdown started: %d sec (%s).", seconds, source);
}

void Notify_CountdownCancelled(const char[] reason)
{
    PrintToChatAll("[ServerFlow] Countdown canceled (%s).", reason);
}

void Notify_LockPhase(const char[] source)
{
    PrintToChatAll("[ServerFlow] Lock phase entered (%s). Ready is locked.", source);
}

void Notify_TeamPhaseStarted(const char[] source)
{
    PrintToChatAll("[ServerFlow] Team phase started (%s).", source);
}

void Notify_AdminOverrideQueued(int actor, const char[] scenarioId)
{
    if (actor == 0)
    {
        PrintToChatAll("[ServerFlow] Console queued admin override: %s", scenarioId);
        return;
    }

    PrintToChatAll("[ServerFlow] %N queued admin override: %s", actor, scenarioId);
}

void Notify_RequestDenied(int client, const char[] reason)
{
    if (!IsValidClient(client))
    {
        return;
    }

    if (StrEqual(reason, "not_enough_players"))
    {
        PrintToChat(client, "[ServerFlow] Not enough players for vote.");
        return;
    }

    if (StrEqual(reason, "cooldown"))
    {
        PrintToChat(client, "[ServerFlow] Vote is on cooldown.");
        return;
    }

    if (StrEqual(reason, "blocked_by_state"))
    {
        PrintToChat(client, "[ServerFlow] Action is blocked in current state.");
        return;
    }

    if (StrEqual(reason, "already_voted"))
    {
        PrintToChat(client, "[ServerFlow] You already voted.");
        return;
    }

    if (StrEqual(reason, "invalid_scenario"))
    {
        PrintToChat(client, "[ServerFlow] Invalid scenario selected.");
        return;
    }

    if (StrEqual(reason, "vote_not_running"))
    {
        PrintToChat(client, "[ServerFlow] No active vote.");
        return;
    }

    if (StrEqual(reason, "vote_in_progress"))
    {
        PrintToChat(client, "[ServerFlow] Vote is already in progress.");
        return;
    }

    PrintToChat(client, "[ServerFlow] Action denied: %s", reason);
}
