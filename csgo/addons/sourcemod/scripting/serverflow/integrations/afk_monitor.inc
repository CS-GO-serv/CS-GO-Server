#if defined _SERVERFLOW_AFK_MONITOR_INC_
 #endinput
#endif
#define _SERVERFLOW_AFK_MONITOR_INC_

static void AfkMonitor_MoveToSpectator(int client)
{
    if (GetClientTeam(client) != 1)
    {
        ChangeClientTeam(client, 1);
    }
}

static void AfkMonitor_EnterAfk(int client, int idleSeconds)
{
    ReadyManager_ForceUnready(client, "afk_enter");
    PlayerState_SetLifecycle(client, PlayerLifecycle_Afk);

    switch (g_AfkPolicy)
    {
        case AfkPolicy_Warn:
        {
            PrintToChat(client, "[ServerFlow] Вы отмечены как AFK. Проявите активность, чтобы вернуться.");
            Audit_LogEvent("afk", "enter", client, "warn", "policy_warn");
            ModeDebug("afk enter: client=%N idle=%d action=warn", client, idleSeconds);
        }
        case AfkPolicy_MoveToSpec:
        {
            AfkMonitor_MoveToSpectator(client);
            PrintToChat(client, "[ServerFlow] Вы переведены в наблюдатели за AFK.");
            Audit_LogEvent("afk", "enter", client, "spectator", "policy_move_to_spec");
            ModeDebug("afk enter: client=%N idle=%d action=move_to_spec", client, idleSeconds);
        }
        case AfkPolicy_Kick:
        {
            Audit_LogEvent("afk", "enter", client, "kick", "policy_kick");
            ModeDebug("afk enter: client=%N idle=%d action=kick", client, idleSeconds);
            KickClient(client, "AFK timeout");
        }
        default:
        {
            Audit_LogEvent("afk", "enter", client, "none", "policy_none");
            ModeDebug("afk enter: client=%N idle=%d action=none", client, idleSeconds);
        }
    }
}

public Action Timer_AfkMonitor(Handle timer)
{
    if (g_AfkPolicy == AfkPolicy_None)
    {
        return Plugin_Continue;
    }

    int now = GetTime();
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!IsValidClient(client))
        {
            continue;
        }

        PlayerLifecycleState lifecycle = PlayerState_GetLifecycle(client);
        if (lifecycle == PlayerLifecycle_Afk || lifecycle == PlayerLifecycle_LateJoin)
        {
            continue;
        }

        int idleSeconds = now - PlayerState_GetLastActionAt(client);
        if (idleSeconds < g_AfkSeconds)
        {
            continue;
        }

        AfkMonitor_EnterAfk(client, idleSeconds);
    }

    return Plugin_Continue;
}

void AfkMonitor_Init()
{
    if (g_AfkMonitorTimer != null)
    {
        delete g_AfkMonitorTimer;
        g_AfkMonitorTimer = null;
    }

    g_AfkMonitorTimer = CreateTimer(1.0, Timer_AfkMonitor, _, TIMER_REPEAT);
}

void AfkMonitor_RecordClientActivity(int client, const char[] source)
{
    if (!IsValidClient(client))
    {
        return;
    }

    PlayerState_RecordActivity(client);

    PlayerLifecycleState lifecycle = PlayerState_GetLifecycle(client);
    if (lifecycle != PlayerLifecycle_Afk && lifecycle != PlayerLifecycle_LateJoin)
    {
        return;
    }

    if (Session_GetState() == ServerState_PreMatch)
    {
        PlayerState_SetLifecycle(client, PlayerLifecycle_Unready);
        ReadyManager_ForceUnready(client, "afk_back_to_unready");
        Audit_LogEvent("afk", "return", client, "unready", source);
        ModeDebug("afk return: client=%N source=%s lifecycle=unready", client, source);
        return;
    }

    PlayerState_SetLifecycle(client, PlayerLifecycle_Eligible);
    Audit_LogEvent("afk", "return", client, "eligible", source);
    ModeDebug("afk return: client=%N source=%s lifecycle=eligible", client, source);
}
