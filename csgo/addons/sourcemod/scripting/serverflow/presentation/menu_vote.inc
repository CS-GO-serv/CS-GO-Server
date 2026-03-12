#if defined _SERVERFLOW_MENU_VOTE_INC_
 #endinput
#endif
#define _SERVERFLOW_MENU_VOTE_INC_

void ShowVoteMenu(int client)
{
    Menu menu = new Menu(VoteMenuHandler);
    menu.SetTitle("ServerFlow Vote");

    char info[8], display[64];
    for (int i = 0; i < MAX_MODES; i++)
    {
        IntToString(i, info, sizeof(info));
        Scenario_GetNameByIndex(i, client, display, sizeof(display));
        menu.AddItem(info, display);
    }

    menu.Display(client, RoundToNearest(g_VoteDuration));
}

public int VoteMenuHandler(Menu menu, MenuAction action, int client, int item)
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

    char info[8];
    menu.GetItem(item, info, sizeof(info));
    Vote_Cast(client, StringToInt(info));
    return 0;
}
