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
    PrintToChatAll("[ServerFlow] Vote finished, pending scenario queued: %s", scenarioId);
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
