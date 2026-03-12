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

    Transition_QueueScenario(id, client);
    ReplyToCommand(client, "Scenario queued: %s", id);
    return Plugin_Handled;
}
