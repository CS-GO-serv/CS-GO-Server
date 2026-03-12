#if defined _SERVERFLOW_PENDING_CHANGE_INC_
 #endinput
#endif
#define _SERVERFLOW_PENDING_CHANGE_INC_

void Pending_Create(const char[] scenarioId, const char[] mapName, int actor)
{
    Session_SetPendingChange(scenarioId, mapName, actor);
}

void Pending_Select()
{
    if (Session_GetPendingChangeState() != Pending_Proposed)
    {
        return;
    }

    Session_SetPendingChangeState(Pending_Selected);
}

void Pending_Confirm()
{
    if (Session_GetPendingChangeState() != Pending_Selected && Session_GetPendingChangeState() != Pending_Proposed)
    {
        return;
    }

    Session_SetPendingChangeState(Pending_Confirmed);

    PendingChange pending;
    Session_GetPendingChange(pending);

    Call_StartForward(g_FwdOnPendingChangeConfirmed);
    Call_PushString(pending.scenarioId);
    Call_PushString(pending.mapName);
    Call_Finish();
}

void Pending_Cancel()
{
    Session_ClearPendingChange();
}
