#if defined _SERVERFLOW_COMMAND_ROUTER_INC_
 #endinput
#endif
#define _SERVERFLOW_COMMAND_ROUTER_INC_

void CommandRouter_Register()
{
    RegConsoleCmd("sm_mode", Command_ModeMenu);
    RegConsoleCmd("sm_votemode", Command_VoteMode);
    RegConsoleCmd("sm_status", Command_Status);

    RegAdminCmd("sm_queuescenario", Command_QueueScenario, ADMFLAG_CHANGEMAP);
    RegAdminCmd("sm_queuemap", Command_QueueMap, ADMFLAG_CHANGEMAP);
    RegAdminCmd("sm_confirmpending", Command_ConfirmPending, ADMFLAG_CHANGEMAP);
    RegAdminCmd("sm_cancelpending", Command_CancelPending, ADMFLAG_CHANGEMAP);
    RegAdminCmd("sm_forcescenario", Command_OverrideScenario, ADMFLAG_CHANGEMAP);
}
