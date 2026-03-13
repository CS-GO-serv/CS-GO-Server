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
    if (!CommandRouter_RequestQueueScenario(client, id))
    {
        return Plugin_Handled;
    }

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
    if (!CommandRouter_RequestQueueMap(client, mapName))
    {
        return Plugin_Handled;
    }

    ReplyToCommand(client, "Map queued: %s", mapName);
    return Plugin_Handled;
}

public Action Command_ConfirmPending(int client, int args)
{
    if (!CommandRouter_RequestConfirmPending(client))
    {
        ReplyToCommand(client, "No pending change to confirm.");
        return Plugin_Handled;
    }

    ReplyToCommand(client, "Pending change confirmed.");
    return Plugin_Handled;
}

public Action Command_CancelPending(int client, int args)
{
    char why[96];
    why[0] = '\0';

    if (args >= 1)
    {
        GetCmdArgString(why, sizeof(why));
        TrimString(why);
    }

    if (CommandRouter_RequestCancelPending(client, why))
    {
        ReplyToCommand(client, "Pending change canceled.");
    }

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
    if (!CommandRouter_RequestOverrideScenario(client, id))
    {
        return Plugin_Handled;
    }

    ReplyToCommand(client, "Override queued via transition manager: %s", id);
    return Plugin_Handled;
}


public Action Command_RuntimeHealth(int client, int args)
{
    if (!CommandRouter_RequestRuntimeHealth(client))
    {
        return Plugin_Handled;
    }

    ReplyToCommand(client, "Runtime health check executed. See server logs for details.");
    return Plugin_Handled;
}

public Action Command_RuntimeDump(int client, int args)
{
    if (!CommandRouter_RequestRuntimeDump(client))
    {
        return Plugin_Handled;
    }

    ReplyToCommand(client, "Runtime state dump emitted to server console.");
    return Plugin_Handled;
}
