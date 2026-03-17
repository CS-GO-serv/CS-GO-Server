#if defined _SERVERFLOW_CMD_PLAYER_INC_
 #endinput
#endif
#define _SERVERFLOW_CMD_PLAYER_INC_

public Action Command_ModeMenu(int client, int args)
{
    if (IsValidClient(client))
    {
        ShowModeMenu(client);
    }

    return Plugin_Handled;
}

public Action Command_VoteMode(int client, int args)
{
    if (!IsValidClient(client))
    {
        return Plugin_Handled;
    }

    CommandRouter_RequestVoteStart(client);
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

public Action Command_StatusVerbose(int client, int args)
{
    if (IsValidClient(client))
    {
        Status_ShowVerboseToClient(client);
    }

    return Plugin_Handled;
}


public Action Command_Ready(int client, int args)
{
    if (IsValidClient(client))
    {
        CommandRouter_RequestReadyToggle(client, true, "player_ready_cmd");
    }

    return Plugin_Handled;
}

public Action Command_Unready(int client, int args)
{
    if (IsValidClient(client))
    {
        CommandRouter_RequestReadyToggle(client, false, "player_unready_cmd");
    }

    return Plugin_Handled;
}


public Action Command_Team(int client, int args)
{
    if (!IsValidClient(client))
    {
        return Plugin_Handled;
    }

    if (Session_GetState() == ServerState_Transition)
    {
        PrintToChat(client, "[ServerFlow] Сейчас идёт Transition. Дождитесь завершения и повторите sm_team.");
        return Plugin_Handled;
    }

    ShowTeamMenu(client);

    return Plugin_Handled;
}

public Action Command_Lobby(int client, int args)
{
    if (!IsValidClient(client))
    {
        return Plugin_Handled;
    }

    int lobbyIndex = FindModeById("lobby");
    if (lobbyIndex == -1)
    {
        PrintToChat(client, "[ServerFlow] Сценарий 'lobby' не настроен в scenarios.cfg.");
        return Plugin_Handled;
    }

    CommandRouter_RequestScenarioSelect(client, lobbyIndex, "player_lobby_alias");
    return Plugin_Handled;
}

public Action Command_HelpServerFlow(int client, int args)
{
    if (!IsValidClient(client))
    {
        return Plugin_Handled;
    }

    PrintToChat(client, "[ServerFlow] Команды: sm_mode, sm_status, sm_status_verbose, sm_votemode, sm_ready, sm_unready, sm_lobby, sm_team");
    PrintToChat(client, "[ServerFlow] В чат пишите !mode, !status, !votemode, !ready, !unready, !lobby (без префикса sm_). ");
    return Plugin_Handled;
}
