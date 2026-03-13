#if defined _SERVERFLOW_COMMAND_ROUTER_INC_
 #endinput
#endif
#define _SERVERFLOW_COMMAND_ROUTER_INC_

void CommandRouter_Register()
{
    RegConsoleCmd("sm_mode", Command_ModeMenu);
    RegConsoleCmd("sm_votemode", Command_VoteMode);
    RegConsoleCmd("sm_status", Command_Status);
    RegConsoleCmd("sm_ready", Command_Ready);
    RegConsoleCmd("sm_unready", Command_Unready);

    RegConsoleCmd("sm_queuescenario", Command_QueueScenario);
    RegConsoleCmd("sm_queuemap", Command_QueueMap);
    RegConsoleCmd("sm_confirmpending", Command_ConfirmPending);
    RegConsoleCmd("sm_cancelpending", Command_CancelPending);
    RegConsoleCmd("sm_forcescenario", Command_OverrideScenario);
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


bool CommandRouter_RequestReadyToggle(int client, bool ready, const char[] source = "player_ready")
{
    if (!IsValidClient(client))
    {
        return false;
    }

    if (!ReadyManager_SetClientReady(client, ready, source))
    {
        Notify_RequestDenied(client, "blocked_by_state");
        return false;
    }

    return true;
}

bool CommandRouter_RequestQueueScenario(int client, const char[] scenarioId)
{
    char reason[48];
    if (!State_CanClientManageTransitions(client, reason, sizeof(reason)))
    {
        Notify_RequestDenied(client, reason);
        return false;
    }

    if (!Scenario_Exists(scenarioId))
    {
        ReplyToCommand(client, "Unknown scenario: %s", scenarioId);
        return false;
    }

    Transition_QueueScenario(scenarioId, client, "admin_command");
    return true;
}

bool CommandRouter_RequestQueueMap(int client, const char[] mapName)
{
    char reason[48];
    if (!State_CanClientManageTransitions(client, reason, sizeof(reason)))
    {
        Notify_RequestDenied(client, reason);
        return false;
    }

    Transition_QueueMap(mapName, client, "admin_command");
    return true;
}

bool CommandRouter_RequestConfirmPending(int client)
{
    char reason[48];
    if (!State_CanClientManageTransitions(client, reason, sizeof(reason)))
    {
        Notify_RequestDenied(client, reason);
        return false;
    }

    return Transition_ConfirmAndApplyPending(client, "admin_confirm");
}

bool CommandRouter_RequestCancelPending(int client, const char[] why = "")
{
    char reason[48];
    if (!State_CanClientManageTransitions(client, reason, sizeof(reason)))
    {
        Notify_RequestDenied(client, reason);
        return false;
    }

    Transition_CancelPending("admin_cancel", why);
    return true;
}

bool CommandRouter_RequestOverrideScenario(int client, const char[] scenarioId)
{
    if (!AdminOverride_RequestScenario(client, scenarioId, "admin_override"))
    {
        ReplyToCommand(client, "Override denied or unknown scenario: %s", scenarioId);
        return false;
    }

    return true;
}
