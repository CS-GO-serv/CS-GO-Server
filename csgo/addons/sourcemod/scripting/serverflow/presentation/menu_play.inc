#if defined _SERVERFLOW_MENU_PLAY_INC_
 #endinput
#endif
#define _SERVERFLOW_MENU_PLAY_INC_

void ShowModeMenu(int client)
{
    Menu menu = new Menu(ModeMenuHandler);
    menu.SetTitle("ServerFlow: Select mode");

    char info[8], display[64];
    for (int i = 0; i < MAX_MODES; i++)
    {
        IntToString(i, info, sizeof(info));
        Scenario_GetNameByIndex(i, client, display, sizeof(display));
        menu.AddItem(info, display);
    }

    menu.Display(client, 20);
}

public int ModeMenuHandler(Menu menu, MenuAction action, int client, int item)
{
    if (action == MenuAction_End)
    {
        delete menu;
        return 0;
    }

    if (action == MenuAction_Select)
    {
        char info[8];
        menu.GetItem(item, info, sizeof(info));
        int index = StringToInt(info);
        Transition_QueueScenario(g_Modes[index].id, client);
        PrintToChat(client, "[ServerFlow] Queued scenario: %s", g_Modes[index].id);
    }

    return 0;
}
