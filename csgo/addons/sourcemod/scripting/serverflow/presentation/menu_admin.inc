#if defined _SERVERFLOW_MENU_ADMIN_INC_
 #endinput
#endif
#define _SERVERFLOW_MENU_ADMIN_INC_

void ShowAdminMenu(int client)
{
    Menu menu = new Menu(AdminMenuHandler);
    menu.SetTitle("ServerFlow Admin: Pending/Confirm");

    menu.AddItem("status", "Show status");
    menu.AddItem("confirm", "Confirm pending apply");
    menu.AddItem("cancel", "Cancel pending");

    menu.Display(client, 20);
}

public int AdminMenuHandler(Menu menu, MenuAction action, int client, int item)
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

    if (StrEqual(info, "status"))
    {
        Status_ShowToClient(client);
        return 0;
    }

    if (StrEqual(info, "confirm"))
    {
        CommandRouter_RequestConfirmPending(client);
        return 0;
    }

    if (StrEqual(info, "cancel"))
    {
        CommandRouter_RequestCancelPending(client);
        return 0;
    }

    return 0;
}
