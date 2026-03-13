#if defined _SERVERFLOW_CONFIG_VALIDATION_INC_
 #endinput
#endif
#define _SERVERFLOW_CONFIG_VALIDATION_INC_

bool Config_ValidateAll()
{
    bool ok = true;

    if (g_ScenarioCount <= 0)
    {
        LogError("[serverflow] validation failed: no scenarios loaded");
        return false;
    }

    for (int i = 0; i < g_ScenarioCount; i++)
    {
        for (int j = i + 1; j < g_ScenarioCount; j++)
        {
            if (StrEqual(g_Scenarios[i].id, g_Scenarios[j].id, false))
            {
                ok = false;
                LogError("[serverflow] duplicate scenario id '%s'", g_Scenarios[i].id);
            }

            if (StrEqual(g_Scenarios[i].routerAlias, g_Scenarios[j].routerAlias, false))
            {
                ok = false;
                LogError("[serverflow] duplicate router alias '%s'", g_Scenarios[i].routerAlias);
            }
        }

        if (g_ModeMapCache[i] == null || g_ModeMapCache[i].Length <= 0)
        {
            ok = false;
            LogError("[serverflow] scenario '%s' has empty playlist (%s)", g_Scenarios[i].id, g_ModeMapCacheError[i]);
        }

        if (g_Scenarios[i].startMap[0] != '\0' && !Playlist_IsMapAllowed(i, g_Scenarios[i].startMap) && !IsMapValid(g_Scenarios[i].startMap))
        {
            ok = false;
            LogError("[serverflow] invalid start_map '%s' for scenario '%s'", g_Scenarios[i].startMap, g_Scenarios[i].id);
        }
    }

    return ok;
}
