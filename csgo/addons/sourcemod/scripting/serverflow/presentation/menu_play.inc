#if defined _SERVERFLOW_MENU_PLAY_INC_
 #endinput
#endif
#define _SERVERFLOW_MENU_PLAY_INC_

void ShowModeMenu(int client)
{
    Menu menu = new Menu(ModeMenuHandler);
    menu.SetTitle("ServerFlow: Select mode");

    char info[8], display[64];
    int scenarioCount = Scenario_GetCount();
    for (int i = 0; i < scenarioCount; i++)
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
        CommandRouter_RequestScenarioSelect(client, StringToInt(info), "menu_play");
    }

    return 0;
}
