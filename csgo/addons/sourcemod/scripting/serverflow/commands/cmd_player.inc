#if defined _SERVERFLOW_CMD_PLAYER_INC_
 #endinput
#endif
#define _SERVERFLOW_CMD_PLAYER_INC_

public Action Command_ModeMenu(int client, int args)
{
    if (!IsValidClient(client))
    {
        return Plugin_Handled;
    }

    ShowModeMenu(client);
    return Plugin_Handled;
}

public Action Command_VoteMode(int client, int args)
{
    if (!IsValidClient(client))
    {
        return Plugin_Handled;
    }

    if (CountHumanPlayers() < g_VoteMinPlayers)
    {
        PrintToChat(client, "[ServerFlow] Not enough players for vote.");
        return Plugin_Handled;
    }

    if (GetTime() < g_NextVoteAllowedAt)
    {
        PrintToChat(client, "[ServerFlow] Vote is on cooldown.");
        return Plugin_Handled;
    }

    Vote_Start(client);
    return Plugin_Handled;
}

public Action Command_Status(int client, int args)
{
    if (IsValidClient(client))
    {
        Status_ShowToClient(client);
    }

    return Plugin_Handled;
}
