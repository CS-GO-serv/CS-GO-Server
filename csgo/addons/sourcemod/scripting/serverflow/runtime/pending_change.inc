#if defined _SERVERFLOW_PENDING_CHANGE_INC_
 #endinput
#endif
#define _SERVERFLOW_PENDING_CHANGE_INC_

static Handle g_PendingConfirmTimer = null;

static void Pending_StopConfirmTimer()
{
    if (g_PendingConfirmTimer != null)
    {
        delete g_PendingConfirmTimer;
        g_PendingConfirmTimer = null;
    }
}

static void Pending_StartConfirmTimer(int seconds)
{
    Pending_StopConfirmTimer();
    g_PendingConfirmTimer = CreateTimer(float(seconds), Timer_PendingConfirmDeadline);
}

void Pending_Create(const char[] scenarioId, const char[] mapName, const char[] source, int actor)
{
    Session_SetPendingChange(scenarioId, mapName, source, actor, g_PendingConfirmWindow);
    Pending_StartConfirmTimer(g_PendingConfirmWindow);
}

void Pending_Select()
{
    if (Session_GetPendingChangeState() != Pending_Proposed)
    {
        return;
    }

    Session_SetPendingChangeState(Pending_Selected);
}

bool Pending_Confirm(const char[] source = "")
{
    PendingChangeState state = Session_GetPendingChangeState();
    if (state != Pending_Selected && state != Pending_Proposed)
    {
        return false;
    }

    PendingChange pending;
    Session_GetPendingChange(pending);

    if (pending.confirmDeadline > 0 && GetTime() > pending.confirmDeadline)
    {
        Pending_Cancel("confirm_timeout");
        return false;
    }

    Session_SetPendingChangeState(Pending_Confirmed);
    Pending_StopConfirmTimer();

    Call_StartForward(g_FwdOnPendingChangeConfirmed);
    Call_PushString(pending.scenarioId);
    Call_PushString(pending.mapName);
    Call_Finish();

    Transition_ApplyPending(source);
    return true;
}

void Pending_Cancel(const char[] reason = "cancel")
{
    Pending_StopConfirmTimer();
    Session_ClearPendingChange();
    ModeDebug("pending canceled: reason=%s", reason);
}

public Action Timer_PendingConfirmDeadline(Handle timer)
{
    g_PendingConfirmTimer = null;

    PendingChange pending;
    Session_GetPendingChange(pending);

    if (pending.state != Pending_None && pending.confirmDeadline > 0 && GetTime() >= pending.confirmDeadline)
    {
        Pending_Cancel("timeout");
    }

    return Plugin_Stop;
}
