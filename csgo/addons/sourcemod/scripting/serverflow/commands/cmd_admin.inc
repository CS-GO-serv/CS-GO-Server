#if defined _SERVERFLOW_CMD_ADMIN_INC_
 #endinput
#endif
#define _SERVERFLOW_CMD_ADMIN_INC_

public Action Command_QueueScenario(int client, int args)
{
    if (args < 1)
    {
        ReplyToCommand(client, "Usage: sm_queuescenario <id>");
        return Plugin_Handled;
    }

    char id[16];
    GetCmdArg(1, id, sizeof(id));
    if (!Scenario_Exists(id))
    {
        ReplyToCommand(client, "Unknown scenario: %s", id);
        return Plugin_Handled;
    }

    Transition_QueueScenario(id, client, "admin_command");
    ReplyToCommand(client, "Scenario queued: %s", id);
    return Plugin_Handled;
}

public Action Command_QueueMap(int client, int args)
{
    if (args < 1)
    {
        ReplyToCommand(client, "Usage: sm_queuemap <map>");
        return Plugin_Handled;
    }

    char mapName[64];
    GetCmdArg(1, mapName, sizeof(mapName));
    Transition_QueueMap(mapName, client, "admin_command");
    ReplyToCommand(client, "Map queued: %s", mapName);
    return Plugin_Handled;
}

public Action Command_ConfirmPending(int client, int args)
{
    if (!Pending_Confirm("admin_confirm"))
    {
        ReplyToCommand(client, "No pending change to confirm.");
        return Plugin_Handled;
    }

    ReplyToCommand(client, "Pending change confirmed.");
    return Plugin_Handled;
}

public Action Command_CancelPending(int client, int args)
{
    Transition_CancelPending("admin_cancel");
    ReplyToCommand(client, "Pending change canceled.");
    return Plugin_Handled;
}

public Action Command_OverrideScenario(int client, int args)
{
    if (args < 1)
    {
        ReplyToCommand(client, "Usage: sm_forcescenario <id>");
        return Plugin_Handled;
    }

    char id[16];
    GetCmdArg(1, id, sizeof(id));
    if (!Scenario_Exists(id))
    {
        ReplyToCommand(client, "Unknown scenario: %s", id);
        return Plugin_Handled;
    }

    Transition_CancelPending("override_replace");
    Transition_QueueScenario(id, client, "admin_override");
    Pending_Confirm("admin_override");

    ReplyToCommand(client, "Override applied via transition manager: %s", id);
    return Plugin_Handled;
}
