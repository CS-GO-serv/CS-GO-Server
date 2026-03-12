#if defined _SERVERFLOW_CHAT_NOTIFIER_INC_
 #endinput
#endif
#define _SERVERFLOW_CHAT_NOTIFIER_INC_

void Notify_StateChanged(ServerState oldState, ServerState newState, const char[] reason)
{
    PrintToChatAll("[ServerFlow] state %d -> %d (%s)", oldState, newState, reason);
}
