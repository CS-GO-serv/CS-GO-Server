#if defined _SERVERFLOW_READY_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_READY_MANAGER_INC_

static bool g_ReadyFlags[MAXPLAYERS + 1];

void ReadyManager_ResetAll(const char[] source = "reset")
{
    for (int i = 1; i <= MaxClients; i++)
    {
        g_ReadyFlags[i] = false;
    }

    if (Session_IsCountdownActive())
    {
        CountdownManager_Cancel("ready_reset");
    }

    Session_SetLockPhase(false);
    Session_SetPreMatchPhase(PreMatch_Ready);
    ModeDebug("ready reset: source=%s", source);
}

bool ReadyManager_IsClientInPool(int client)
{
    if (!IsValidClient(client) || Session_IsLockPhase())
    {
        return false;
    }

    PlayerLifecycleState lifecycle = PlayerState_GetLifecycle(client);
    return (lifecycle != PlayerLifecycle_Afk && lifecycle != PlayerLifecycle_LateJoin);
}

bool ReadyManager_IsClientReady(int client)
{
    if (client < 1 || client > MaxClients)
    {
        return false;
    }

    return g_ReadyFlags[client];
}

void ReadyManager_ForceUnready(int client, const char[] source = "force_unready")
{
    if (client < 1 || client > MaxClients)
    {
        return;
    }

    if (!g_ReadyFlags[client])
    {
        return;
    }

    g_ReadyFlags[client] = false;
    Notify_ReadyChanged(client, false, ReadyManager_GetReadyCount(), ReadyManager_GetPoolCount());
    ReadyManager_Reevaluate(source);
}

int ReadyManager_GetPoolCount()
{
    int total;
    for (int i = 1; i <= MaxClients; i++)
    {
        if (ReadyManager_IsClientInPool(i))
        {
            total++;
        }
    }

    return total;
}

int ReadyManager_GetReadyCount()
{
    int total;
    for (int i = 1; i <= MaxClients; i++)
    {
        if (ReadyManager_IsClientInPool(i) && g_ReadyFlags[i])
        {
            total++;
        }
    }

    return total;
}

bool ReadyManager_IsReadySatisfied()
{
    int poolCount = ReadyManager_GetPoolCount();
    if (poolCount <= 0)
    {
        return false;
    }

    return ReadyManager_GetReadyCount() >= poolCount;
}

void ReadyManager_Reevaluate(const char[] source = "reevaluate")
{
    if (Session_GetState() != ServerState_PreMatch || Session_IsLockPhase())
    {
        return;
    }

    if (ReadyManager_IsReadySatisfied())
    {
        CountdownManager_Start("ready_satisfied");
        return;
    }

    CountdownManager_Cancel(source);
}

bool ReadyManager_SetClientReady(int client, bool ready, const char[] source = "manual")
{
    if (!ReadyManager_IsClientInPool(client))
    {
        return false;
    }

    g_ReadyFlags[client] = ready;
    PlayerState_SetLifecycle(client, ready ? PlayerLifecycle_Ready : PlayerLifecycle_Unready);

    Notify_ReadyChanged(client, ready, ReadyManager_GetReadyCount(), ReadyManager_GetPoolCount());
    ReadyManager_Reevaluate(source);
    return true;
}
