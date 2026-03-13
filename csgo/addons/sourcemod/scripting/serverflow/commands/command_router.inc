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

bool CommandRouter_RequestScenarioSelect(int client, int modeIndex, const char[] source = "player_select")
{
    char reason[48];
    if (!Scenario_CanPlayerSelect(client, modeIndex, reason, sizeof(reason)))
    {
        Notify_RequestDenied(client, reason);
        return false;
    }

    char scenarioId[16];
    Scenario_GetIdByIndex(modeIndex, scenarioId, sizeof(scenarioId));
    Transition_QueueScenario(scenarioId, client, source);
    return true;
}

bool CommandRouter_RequestVoteStart(int client)
{
    char reason[48];
    if (!VoteManager_RequestVoteStart(client, reason, sizeof(reason)))
    {
        Notify_RequestDenied(client, reason);
        return false;
    }

    return true;
}

bool CommandRouter_RequestVoteCast(int client, int modeIndex)
{
    char reason[48];
    if (!VoteManager_RequestVoteCast(client, modeIndex, reason, sizeof(reason)))
    {
        Notify_RequestDenied(client, reason);
        return false;
    }

    return true;
}
