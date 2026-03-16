#if defined _SERVERFLOW_PLAYER_STATE_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_PLAYER_STATE_MANAGER_INC_

void PlayerState_ResetAll()
{
    for (int i = 1; i <= MaxClients; i++)
    {
        g_PlayerState[i].lifecycle = PlayerLifecycle_Connected;
        g_PlayerState[i].selectedDzAutoAssign = false;
        g_PlayerState[i].selectedDzTeamCount = 0;
        g_PlayerState[i].selectedModeIndex = -1;
        g_PlayerState[i].lastActionAt = 0;
    }
}

void PlayerState_OnClientAuthorized(int client)
{
    if (client < 1 || client > MaxClients)
    {
        return;
    }

    g_PlayerState[client].lifecycle = PlayerLifecycle_Eligible;
    g_PlayerState[client].lastActionAt = GetTime();
}

void PlayerState_OnClientDisconnected(int client)
{
    if (client < 1 || client > MaxClients)
    {
        return;
    }

    g_PlayerState[client].lifecycle = PlayerLifecycle_Connected;
    g_PlayerState[client].lastActionAt = GetTime();
    ReadyManager_SetClientReady(client, false, "disconnect");
}

void PlayerState_SetLifecycle(int client, PlayerLifecycleState state)
{
    if (client < 1 || client > MaxClients)
    {
        return;
    }

    g_PlayerState[client].lifecycle = state;
    g_PlayerState[client].lastActionAt = GetTime();
}

PlayerLifecycleState PlayerState_GetLifecycle(int client)
{
    if (client < 1 || client > MaxClients)
    {
        return PlayerLifecycle_Connected;
    }

    return g_PlayerState[client].lifecycle;
}
