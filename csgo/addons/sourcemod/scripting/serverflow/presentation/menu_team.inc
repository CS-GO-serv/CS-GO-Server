#if defined _SERVERFLOW_MENU_TEAM_INC_
 #endinput
#endif
#define _SERVERFLOW_MENU_TEAM_INC_

void ShowTeamMenu(int client)
{
    Menu menu = new Menu(TeamMenuHandler);
    menu.SetTitle("ServerFlow Team/Ready");
    menu.AddItem("ready", "Mark READY");
    menu.AddItem("unready", "Mark UNREADY");
    menu.AddItem("status", "Show status");
    menu.Display(client, 20);
}

public int TeamMenuHandler(Menu menu, MenuAction action, int client, int item)
{
    if (action == MenuAction_End)
    {
        delete menu;
        return 0;
    }

    if (action != MenuAction_Select)
    {
        return 0;
    }

    char info[16];
    menu.GetItem(item, info, sizeof(info));

    if (StrEqual(info, "ready"))
    {
        CommandRouter_RequestReadyToggle(client, true, "menu_team");
    }
    else if (StrEqual(info, "unready"))
    {
        CommandRouter_RequestReadyToggle(client, false, "menu_team");
    }
    else
    {
        Status_ShowToClient(client);
    }

    return 0;
}
