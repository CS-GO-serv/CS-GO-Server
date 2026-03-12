#if defined _SERVERFLOW_PENDING_CHANGE_INC_
 #endinput
#endif
#define _SERVERFLOW_PENDING_CHANGE_INC_

void Pending_Create(const char[] scenarioId, const char[] mapName, int actor)
{
    Session_SetPendingChange(scenarioId, mapName, actor);
}

void Pending_Confirm()
{
    if (g_PendingChange.state != Pending_Queued)
    {
        return;
    }

    g_PendingChange.state = Pending_Confirmed;
    Call_StartForward(g_FwdOnPendingChangeConfirmed);
    Call_PushString(g_PendingChange.scenarioId);
    Call_PushString(g_PendingChange.mapName);
    Call_Finish();
}

void Pending_Cancel()
{
    Session_ClearPendingChange();
}
