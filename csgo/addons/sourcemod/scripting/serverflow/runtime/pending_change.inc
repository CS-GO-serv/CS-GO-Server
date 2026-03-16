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
    if (seconds <= 0)
    {
        Pending_StopConfirmTimer();
        return;
    }

    Pending_StopConfirmTimer();
    g_PendingConfirmTimer = CreateTimer(float(seconds), Timer_PendingConfirmDeadline);
}

void Pending_Create(const char[] scenarioId, const char[] mapName, const char[] source, int actor, const char[] why = "", ServerState allowedApplyState = ServerState_Lobby, PendingConfirmPolicy confirmPolicy = PendingConfirm_Manual)
{
    int now = GetTime();
    int expiresAt = now + g_PendingConfirmWindow;
    Session_SetPendingChange(scenarioId, mapName, source, actor, why, allowedApplyState, expiresAt, confirmPolicy);
    Pending_StartConfirmTimer(g_PendingConfirmWindow);

    Audit_LogEvent("pending", "create", actor, "ok", why);
    Notify_PendingCreated(scenarioId, mapName, g_PendingConfirmWindow, why, allowedApplyState, confirmPolicy);
}

void Pending_Select()
{
    if (Session_GetPendingChangeState() != Pending_Proposed)
    {
        return;
    }

    Session_SetPendingChangeState(Pending_Selected);

    PendingChange pending;
    Session_GetPendingChange(pending);
    Notify_PendingSelected(pending.scenarioId, pending.mapName, pending.why, pending.expiresAt);
}

bool Pending_Confirm(int actor, const char[] source = "")
{
    char reason[48];
    if (!State_CanClientManageTransitions(actor, reason, sizeof(reason)))
    {
        return false;
    }

    PendingChangeState state = Session_GetPendingChangeState();
    if (state != Pending_Selected && state != Pending_Proposed)
    {
        return false;
    }

    PendingChange pending;
    Session_GetPendingChange(pending);

    if (pending.expiresAt > 0 && GetTime() > pending.expiresAt)
    {
        Pending_Cancel("deadline_expired", "pending confirmation deadline passed");
        return false;
    }

    Session_SetPendingChangeState(Pending_Confirmed);
    Session_SetPendingChangeConfirmed(actor, GetTime());
    Session_SetOverrideLevel(Policy_GetActorOverrideLevel(actor));
    Pending_StopConfirmTimer();

    Call_StartForward(g_FwdOnPendingChangeConfirmed);
    Call_PushString(pending.scenarioId);
    Call_PushString(pending.mapName);
    Call_Finish();

    Audit_LogEvent("pending", "confirm", actor, "ok", source);
    Notify_PendingConfirmed(actor, pending.scenarioId, pending.mapName);

    return true;
}

void Pending_Cancel(const char[] reason = "cancel", const char[] why = "")
{
    PendingChange pending;
    Session_GetPendingChange(pending);

    Pending_StopConfirmTimer();
    Session_SetPendingChangeCancellationReason(why);

    Audit_LogEvent("pending", "cancel", pending.requestedBy, "ok", reason);

    Notify_PendingCanceled(reason, why, pending.scenarioId, pending.mapName);
    Session_ClearPendingChange();
    ModeDebug("pending canceled: reason=%s why=%s", reason, why);
}

public Action Timer_PendingConfirmDeadline(Handle timer)
{
    g_PendingConfirmTimer = null;

    PendingChange pending;
    Session_GetPendingChange(pending);

    if (pending.state != Pending_None && pending.expiresAt > 0 && GetTime() >= pending.expiresAt)
    {
        Pending_Cancel("deadline_expired", "pending confirmation deadline reached");
    }

    return Plugin_Stop;
}
