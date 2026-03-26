#if defined _SERVERFLOW_COUNTDOWN_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_COUNTDOWN_MANAGER_INC_

static Handle g_PreMatchCountdownTimer = null;
static int g_PreMatchCountdownSeconds = 10;

static void Countdown_HideProgressBarAll()
{
    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsClientInGame(i) && !IsFakeClient(i))
        {
            SetEntPropFloat(i, Prop_Send, "m_flProgressBarStartTime", 0.0);
            SetEntProp(i, Prop_Send, "m_iProgressBarDuration", 0);
        }
    }
}

static void Countdown_StopTimer()
{
    if (g_PreMatchCountdownTimer != null)
    {
        delete g_PreMatchCountdownTimer;
        g_PreMatchCountdownTimer = null;
    }
    
    Countdown_HideProgressBarAll();
}

static void Countdown_OrchestrateAfterLock(const char[] source = "countdown_done")
{
    Transition_TryEnterMatchFromLock(source);
}

bool CountdownManager_Start(const char[] source = "countdown_start")
{
    if (Session_GetState() != ServerState_PreMatch || Session_IsLockPhase())
    {
        return false;
    }

    if (Session_IsCountdownActive())
    {
        return true;
    }

    Countdown_StopTimer();
    Session_SetPreMatchPhase(PreMatch_Ready);
    Session_SetCountdownState(true, GetTime() + g_PreMatchCountdownSeconds);
    g_PreMatchCountdownTimer = CreateTimer(float(g_PreMatchCountdownSeconds), Timer_PreMatchCountdownDone);
    
    // Show native CS:GO progress bar to all players
    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsClientInGame(i) && !IsFakeClient(i))
        {
            SetEntPropFloat(i, Prop_Send, "m_flProgressBarStartTime", GetGameTime());
            SetEntProp(i, Prop_Send, "m_iProgressBarDuration", g_PreMatchCountdownSeconds);
        }
    }
    
    Notify_CountdownStarted(g_PreMatchCountdownSeconds, source);
    return true;
}

void CountdownManager_Cancel(const char[] reason = "countdown_cancel")
{
    if (!Session_IsCountdownActive())
    {
        return;
    }

    Countdown_StopTimer();
    Session_SetCountdownState(false, 0);
    Notify_CountdownCancelled(reason);
}

void CountdownManager_ForceLock(const char[] source = "countdown_done")
{
    Countdown_StopTimer();
    Session_SetCountdownState(false, 0);
    Session_SetLockPhase(true);
    Session_SetPreMatchPhase(PreMatch_Lock);
    Notify_LockPhase(source);

    Countdown_OrchestrateAfterLock(source);
}

public Action Timer_PreMatchCountdownDone(Handle timer)
{
    g_PreMatchCountdownTimer = null;

    if (Session_GetState() != ServerState_PreMatch)
    {
        Session_SetCountdownState(false, 0);
        Countdown_HideProgressBarAll();
        return Plugin_Stop;
    }

    if (!ReadyManager_IsReadySatisfied())
    {
        Session_SetCountdownState(false, 0);
        Notify_CountdownCancelled("ready_changed");
        Countdown_HideProgressBarAll();
        return Plugin_Stop;
    }

    CountdownManager_ForceLock("countdown_done");
    return Plugin_Stop;
}
