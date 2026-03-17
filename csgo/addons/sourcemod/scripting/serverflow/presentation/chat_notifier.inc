#if defined _SERVERFLOW_CHAT_NOTIFIER_INC_
 #endinput
#endif
#define _SERVERFLOW_CHAT_NOTIFIER_INC_

void Notify_StateChanged(ServerState oldState, ServerState newState, const char[] reason)
{
    PrintToChatAll("%t", "state.changed", oldState, newState, reason);
}

void Notify_VoteStarted(int caller)
{
    PrintToChatAll("%t", "state.vote.started", caller);
}

void Notify_VoteFinished(const char[] scenarioId)
{
    PrintToChatAll("%t", "state.vote.finished", scenarioId);
}

void Notify_PendingCreated(const char[] scenarioId, const char[] mapName, int seconds, const char[] why, ServerState allowedApplyState, PendingConfirmPolicy confirmPolicy)
{
    PrintToChatAll("%t", "pending.created.selection", scenarioId, mapName[0] == '\0' ? "auto" : mapName);
    PrintToChatAll("%t", "pending.created.reason", why[0] == '\0' ? "no reason provided" : why);

    if (confirmPolicy == PendingConfirm_Manual)
    {
        PrintToChatAll("%t", "pending.created.confirm_required", seconds);
    }
    else
    {
        PrintToChatAll("%t", "pending.created.auto_wait", allowedApplyState);
    }
}

void Notify_PendingSelected(const char[] scenarioId, const char[] mapName, const char[] why, int expiresAt)
{
    int left = expiresAt - GetTime();
    if (left < 0)
    {
        left = 0;
    }

    PrintToChatAll("%t", "confirm.selection", scenarioId, mapName[0] == '\0' ? "auto" : mapName);
    PrintToChatAll("%t", "confirm.prompt", why[0] == '\0' ? "no reason provided" : why, left);
}

void Notify_PendingConfirmed(int actor, const char[] scenarioId, const char[] mapName)
{
    PrintToChatAll("%t", "confirm.accepted", actor, scenarioId, mapName[0] == '\0' ? "auto" : mapName);
}

void Notify_PendingCanceled(const char[] reason, const char[] why, const char[] scenarioId, const char[] mapName)
{
    PrintToChatAll("%t", "pending.canceled.selection", scenarioId[0] == '\0' ? "current mode" : scenarioId, mapName[0] == '\0' ? "auto" : mapName);
    PrintToChatAll("%t", "pending.canceled.reason", reason, why[0] == '\0' ? "" : " / ", why);
}

void Notify_ReadyChanged(int client, bool ready, int readyCount, int poolCount)
{
    PrintToChatAll("%t", "state.ready.changed", client, ready ? "READY" : "UNREADY", readyCount, poolCount);
}

void Notify_CountdownStarted(int seconds, const char[] source)
{
    PrintToChatAll("%t", "state.countdown.started", seconds, source);
}

void Notify_CountdownCancelled(const char[] reason)
{
    PrintToChatAll("%t", "state.countdown.canceled", reason);
}

void Notify_LockPhase(const char[] source)
{
    PrintToChatAll("%t", "state.lock.entered", source);
}

void Notify_TeamPhaseStarted(const char[] source)
{
    PrintToChatAll("%t", "state.team.started", source);
}

void Notify_AdminOverrideQueued(int actor, const char[] scenarioId)
{
    if (actor == 0)
    {
        PrintToChatAll("%t", "pending.override.console", scenarioId);
        return;
    }

    PrintToChatAll("%t", "pending.override.admin", actor, scenarioId);
}

void Notify_RequestDenied(int client, const char[] reason)
{
    if (!IsValidClient(client))
    {
        return;
    }

    if (StrEqual(reason, "not_enough_players"))
    {
        PrintToChat(client, "%t", "state.denied.not_enough_players");
        return;
    }

    if (StrEqual(reason, "cooldown"))
    {
        PrintToChat(client, "%t", "state.denied.cooldown");
        return;
    }

    if (StrEqual(reason, "blocked_by_state"))
    {
        PrintToChat(client, "%t", "state.denied.blocked_by_state");
        return;
    }

    if (StrEqual(reason, "already_voted"))
    {
        PrintToChat(client, "%t", "state.denied.already_voted");
        return;
    }

    if (StrEqual(reason, "invalid_scenario"))
    {
        PrintToChat(client, "%t", "state.denied.invalid_scenario");
        return;
    }

    if (StrEqual(reason, "vote_not_running"))
    {
        PrintToChat(client, "%t", "state.denied.vote_not_running");
        return;
    }

    if (StrEqual(reason, "vote_in_progress"))
    {
        PrintToChat(client, "%t", "state.denied.vote_in_progress");
        return;
    }

    PrintToChat(client, "%t", "state.denied.generic", reason);
}
