#if defined _SERVERFLOW_FORWARDS_INC_
 #endinput
#endif
#define _SERVERFLOW_FORWARDS_INC_

GlobalForward g_FwdOnServerStateChanged;
GlobalForward g_FwdOnPendingChangeConfirmed;
GlobalForward g_FwdOnVoteCompleted;

void Forwards_Init()
{
    g_FwdOnServerStateChanged = new GlobalForward("OnServerStateChanged", ET_Ignore, Param_Cell, Param_Cell, Param_String);
    g_FwdOnPendingChangeConfirmed = new GlobalForward("OnPendingChangeConfirmed", ET_Ignore, Param_String, Param_String);
    g_FwdOnVoteCompleted = new GlobalForward("OnVoteCompleted", ET_Ignore, Param_String, Param_Cell);
}
