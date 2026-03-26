#if defined _SERVERFLOW_ADMIN_BUILDER_MENU_INC_
 #endinput
#endif
#define _SERVERFLOW_ADMIN_BUILDER_MENU_INC_

enum struct AdminBuilderState
{
    char scenarioId[16];
    char mapName[64];
    ModifierString modifiers[MAX_SESSION_MODIFIERS];
    int modifierCount;
}

static AdminBuilderState g_BuilderStates[MAXPLAYERS + 1];

void AdminBuilder_Reset(int client)
{
    g_BuilderStates[client].scenarioId[0] = '\0';
    g_BuilderStates[client].mapName[0] = '\0';
    g_BuilderStates[client].modifierCount = 0;
}


void AdminBuilder_OpenMain(int client)
{
    if (!State_CanClientManageTransitions(client, "", 0))
    {
        PrintToChat(client, "[\x04ServerFlow\x01] Нет доступа.");
        return;
    }

    Menu menu = new Menu(AdminBuilderMain_Callback);
    menu.SetTitle("ServerFlow Builder Dashboard\n-------------------------");

    char titleScenario[64];
    if (g_BuilderStates[client].scenarioId[0] == '\0') {
        strcopy(titleScenario, sizeof(titleScenario), "НЕ ВЫБРАН");
    } else {
        strcopy(titleScenario, sizeof(titleScenario), g_BuilderStates[client].scenarioId);
    }
    
    char titleMap[64];
    if (g_BuilderStates[client].mapName[0] == '\0') {
        strcopy(titleMap, sizeof(titleMap), "АВТО (По умолчанию)");
    } else {
        strcopy(titleMap, sizeof(titleMap), g_BuilderStates[client].mapName);
    }

    char item1[128], item2[128], item3[128];
    Format(item1, sizeof(item1), "1. Сценарий: [%s]", titleScenario);
    menu.AddItem("scenario", item1);

    if (g_BuilderStates[client].scenarioId[0] == '\0') {
        menu.AddItem("map", "2. Карта (Сначала выберите сценарий!)", ITEMDRAW_DISABLED);
    } else {
        Format(item2, sizeof(item2), "2. Карта: [%s]", titleMap);
        menu.AddItem("map", item2);
    }

    Format(item3, sizeof(item3), "3. Модификаторы: [%d шт. активны]", g_BuilderStates[client].modifierCount);
    menu.AddItem("modifiers", item3);

    menu.AddItem("", "", ITEMDRAW_SPACER);

    if (g_BuilderStates[client].scenarioId[0] != '\0')
    {
        menu.AddItem("apply", "-> ПРИМЕНИТЬ КОНФИГУРАЦИЮ");
    }
    else
    {
        menu.AddItem("apply", "-> ПРИМЕНИТЬ КОНФИГУРАЦИЮ", ITEMDRAW_DISABLED);
    }

    menu.AddItem("reset", "-> СБРОСИТЬ ДАШБОРД");

    menu.Display(client, MENU_TIME_FOREVER);
}

public int AdminBuilderMain_Callback(Menu menu, MenuAction action, int client, int param2)
{
    if (action == MenuAction_End)
    {
        delete menu;
    }
    else if (action == MenuAction_Select)
    {
        char info[32];
        menu.GetItem(param2, info, sizeof(info));

        if (StrEqual(info, "scenario"))
        {
            AdminBuilder_OpenScenarioSelection(client);
        }
        else if (StrEqual(info, "map"))
        {
            AdminBuilder_OpenMapSelection(client);
        }
        else if (StrEqual(info, "modifiers"))
        {
            AdminBuilder_OpenModifierSelection(client);
        }
        else if (StrEqual(info, "reset"))
        {
            AdminBuilder_Reset(client);
            AdminBuilder_OpenMain(client);
        }
        else if (StrEqual(info, "apply"))
        {
            // Apply logic
            AdminBuilder_Apply(client);
        }
    }
    return 0;
}

void AdminBuilder_OpenScenarioSelection(int client)
{
    Menu menu = new Menu(AdminBuilderScenario_Callback);
    menu.SetTitle("Выберите Сценарий:\n ");

    int count = Scenario_GetCount();
    for (int i = 0; i < count; i++)
    {
        char id[16], name[32];
        Scenario_GetIdByIndex(i, id, sizeof(id));
        Scenario_GetNameByIndex(i, client, name, sizeof(name));
        
        char display[64];
        Format(display, sizeof(display), "%s (%s)", name, id);
        menu.AddItem(id, display);
    }

    menu.ExitBackButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

public int AdminBuilderScenario_Callback(Menu menu, MenuAction action, int client, int param2)
{
    if (action == MenuAction_End)
    {
        delete menu;
    }
    else if (action == MenuAction_Cancel && param2 == MenuCancel_ExitBack)
    {
        AdminBuilder_OpenMain(client);
    }
    else if (action == MenuAction_Select)
    {
        char scenarioId[16];
        menu.GetItem(param2, scenarioId, sizeof(scenarioId));
        
        // Update state and clear map since it belongs to previous scenario
        if (!StrEqual(g_BuilderStates[client].scenarioId, scenarioId))
        {
            g_BuilderStates[client].mapName[0] = '\0';
        }
        strcopy(g_BuilderStates[client].scenarioId, sizeof(g_BuilderStates[client].scenarioId), scenarioId);
        
        AdminBuilder_OpenMain(client);
    }
    return 0;
}

void AdminBuilder_OpenMapSelection(int client)
{
    Menu menu = new Menu(AdminBuilderMap_Callback);
    menu.SetTitle("Выберите Карту для %s (или АВТО):\n ", g_BuilderStates[client].scenarioId);

    menu.AddItem("AUTO", "--- АВТОМАТИЧЕСКИ ---");

    char maps[32][64];
    int mapCount = Playlist_GetVotePool(g_BuilderStates[client].scenarioId, maps, 32);
    for (int i = 0; i < mapCount; i++)
    {
        menu.AddItem(maps[i], maps[i]);
    }

    menu.ExitBackButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

public int AdminBuilderMap_Callback(Menu menu, MenuAction action, int client, int param2)
{
    if (action == MenuAction_End)
    {
        delete menu;
    }
    else if (action == MenuAction_Cancel && param2 == MenuCancel_ExitBack)
    {
        AdminBuilder_OpenMain(client);
    }
    else if (action == MenuAction_Select)
    {
        char mapName[64];
        menu.GetItem(param2, mapName, sizeof(mapName));
        
        if (StrEqual(mapName, "AUTO"))
        {
            g_BuilderStates[client].mapName[0] = '\0';
        }
        else
        {
            strcopy(g_BuilderStates[client].mapName, sizeof(g_BuilderStates[client].mapName), mapName);
        }
        
        AdminBuilder_OpenMain(client);
    }
    return 0;
}

void AdminBuilder_OpenModifierSelection(int client)
{
    Menu menu = new Menu(AdminBuilderModifier_Callback);
    menu.SetTitle("Выберите Модификаторы (Нажмите для вкл/выкл):\n ");

    // We scan directory "addons/sourcemod/configs/serverflow/modifiers/"
    char dirPath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, dirPath, sizeof(dirPath), "configs/serverflow/modifiers/");
    
    DirectoryListing dir = OpenDirectory(dirPath);
    if (dir == null)
    {
        menu.AddItem("", "Ошибка: Папка modifiers не найдена!", ITEMDRAW_DISABLED);
    }
    else
    {
        char fileName[64];
        FileType type;
        int count = 0;
        
        while (dir.GetNext(fileName, sizeof(fileName), type))
        {
            if (type == FileType_File && StrContains(fileName, ".cfg", false) != -1)
            {
                char fullCfgPath[128];
                Format(fullCfgPath, sizeof(fullCfgPath), "serverflow/modifiers/%s", fileName);
                
                // Check if already active
                bool active = false;
                for (int i = 0; i < g_BuilderStates[client].modifierCount; i++)
                {
                    if (StrEqual(g_BuilderStates[client].modifiers[i].cfg, fullCfgPath, false))
                    {
                        active = true;
                        break;
                    }
                }
                
                char display[128];
                Format(display, sizeof(display), "[%s] %s", active ? "X" : " ", fileName);
                menu.AddItem(fullCfgPath, display);
                count++;
            }
        }
        
        if (count == 0)
        {
            menu.AddItem("", "Нет доступных .cfg модификаторов", ITEMDRAW_DISABLED);
        }
        
        delete dir;
    }

    menu.ExitBackButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

public int AdminBuilderModifier_Callback(Menu menu, MenuAction action, int client, int param2)
{
    if (action == MenuAction_End)
    {
        delete menu;
    }
    else if (action == MenuAction_Cancel && param2 == MenuCancel_ExitBack)
    {
        AdminBuilder_OpenMain(client);
    }
    else if (action == MenuAction_Select)
    {
        char cfgPath[128];
        menu.GetItem(param2, cfgPath, sizeof(cfgPath));
        
        // Toggle logic
        bool found = false;
        for (int i = 0; i < g_BuilderStates[client].modifierCount; i++)
        {
            if (StrEqual(g_BuilderStates[client].modifiers[i].cfg, cfgPath, false))
            {
                // Remove
                found = true;
                for (int j = i; j < g_BuilderStates[client].modifierCount - 1; j++)
                {
                    strcopy(g_BuilderStates[client].modifiers[j].cfg, sizeof(g_BuilderStates[client].modifiers[].cfg), g_BuilderStates[client].modifiers[j+1].cfg);
                }
                g_BuilderStates[client].modifierCount--;
                break;
            }
        }
        
        if (!found && g_BuilderStates[client].modifierCount < MAX_SESSION_MODIFIERS)
        {
            // Add
            strcopy(g_BuilderStates[client].modifiers[g_BuilderStates[client].modifierCount].cfg, sizeof(g_BuilderStates[client].modifiers[].cfg), cfgPath);
            g_BuilderStates[client].modifierCount++;
        }
        
        // Re-open modifier menu to show updated state eagerly
        AdminBuilder_OpenModifierSelection(client);
    }
    return 0;
}

void AdminBuilder_Apply(int client)
{
    if (g_BuilderStates[client].scenarioId[0] == '\0')
    {
        PrintToChat(client, "[\x04ServerFlow\x01] Сценарий не выбран!");
        AdminBuilder_OpenMain(client);
        return;
    }

    // Push into pending change
    Pending_Create(
        g_BuilderStates[client].scenarioId,
        g_BuilderStates[client].mapName,
        "admin_builder",
        client,
        "Admin built custom session",
        Session_GetState(), // allow apply from current state
        PendingConfirm_Auto
    );

    // Inject modifiers into PendingChange using our abstraction
    for (int i = 0; i < g_BuilderStates[client].modifierCount; i++)
    {
        Pending_AddModifier(g_BuilderStates[client].modifiers[i].cfg);
    }

    // Because confirm_policy is Auto, it will just pend. We need to auto-confirm it.
    Pending_Select();
    Pending_Confirm(client, "admin_builder");

    if (Transition_ApplyPending("admin_builder"))
    {
        PrintToChat(client, "[\x04ServerFlow\x01] Запуск пользовательской сессии!");
        AdminBuilder_Reset(client);
    }
    else
    {
        PrintToChat(client, "[\x04ServerFlow\x01] Ошибка применения. Проверьте условия (Lock Phase?).");
    }
}
