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

    char auditMessage[192];
    Format(auditMessage, sizeof(auditMessage), "pending_created actor=%d scenario=%s map=%s source=%s", actor, scenarioId, mapName, source);
    Audit_Log(auditMessage);
}

void Pending_Select()
{
    if (Session_GetPendingChangeState() != Pending_Proposed)
    {
        return;
    }

    Session_SetPendingChangeState(Pending_Selected);
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

    char auditMessage[192];
    Format(auditMessage, sizeof(auditMessage), "pending_confirmed actor=%d scenario=%s map=%s source=%s", actor, pending.scenarioId, pending.mapName, source);
    Audit_Log(auditMessage);

    return true;
}

void Pending_Cancel(const char[] reason = "cancel")
{
    Pending_StopConfirmTimer();
    Session_ClearPendingChange();
    ModeDebug("pending canceled: reason=%s", reason);

    char auditMessage[160];
    Format(auditMessage, sizeof(auditMessage), "pending_canceled reason=%s", reason);
    Audit_Log(auditMessage);
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
