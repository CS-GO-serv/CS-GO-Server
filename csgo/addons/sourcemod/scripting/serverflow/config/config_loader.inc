#if defined _SERVERFLOW_CONFIG_LOADER_INC_
 #endinput
#endif
#define _SERVERFLOW_CONFIG_LOADER_INC_

static bool Config_ApplySafeFallbackScenario()
{
    for (int i = 0; i < g_ScenarioCount; i++)
    {
        if (g_ModeMapCache[i] != null && g_ModeMapCache[i].Length > 0)
        {
            char fallbackMap[64];
            g_ModeMapCache[i].GetString(0, fallbackMap, sizeof(fallbackMap));
            if (!IsMapValid(fallbackMap))
            {
                strcopy(fallbackMap, sizeof(fallbackMap), SAFE_HUB_MAP);
            }

            for (int j = 0; j < g_ScenarioCount; j++)
            {
                if (j == i)
                {
                    g_ModeMapCacheLoaded[j] = true;
                    strcopy(g_ModeMapCacheError[j], sizeof(g_ModeMapCacheError[]), "ok");
                }
                else
                {
                    if (g_ModeMapCache[j] != null)
                    {
                        g_ModeMapCache[j].Clear();
                        g_ModeMapCache[j].PushString(fallbackMap);
                    }
                    g_ModeMapCacheLoaded[j] = false;
                    Format(g_ModeMapCacheError[j], sizeof(g_ModeMapCacheError[]), "degraded: using safe fallback scenario '%s'", g_Scenarios[i].id);
                }
            }

            LogError("[serverflow] safe fallback enabled: scenario='%s' map='%s'", g_Scenarios[i].id, fallbackMap);
            g_ConfigValidationDegraded = true;
            return true;
        }
    }

    for (int i = 0; i < g_ScenarioCount; i++)
    {
        if (g_ModeMapCache[i] == null)
        {
            continue;
        }

        g_ModeMapCache[i].Clear();
        g_ModeMapCache[i].PushString(SAFE_HUB_MAP);
        g_ModeMapCacheLoaded[i] = false;
        strcopy(g_ModeMapCacheError[i], sizeof(g_ModeMapCacheError[]), "degraded: forced SAFE_HUB_MAP");
    }

    if (g_ScenarioCount > 0)
    {
        LogError("[serverflow] safe fallback enabled: forced '%s' for all scenarios", SAFE_HUB_MAP);
        g_ConfigValidationDegraded = true;
        return true;
    }

    return false;
}

void Config_LoadAll()
{
    ConfigCore_InitConVars();
    AutoExecConfig(true, "serverflow", "sourcemod");
    ServerCommand("exec sourcemod/serverflow.cfg");
    ConfigCore_RefreshRuntime();

    bool scenariosOk = Scenarios_LoadFromConfig();
    ConfigPlaylists_Reload();

    if (!Config_ValidateAll())
    {
        LogError("[serverflow] config validation failed on primary config load; trying fallback scenarios");
        if (!scenariosOk || g_ScenarioCount <= 0)
        {
            Scenarios_LoadFallbackDefaults();
            ConfigPlaylists_Reload();
        }

        if (!Config_ValidateAll())
        {
            if (Config_ValidationIsCriticalFailure() || !Config_ApplySafeFallbackScenario())
            {
                SetFailState("[serverflow] config validation failed");
            }
        }
    }
}

void Config_ReloadAll()
{
    ConfigCore_RefreshRuntime();

    Scenarios_LoadFromConfig();
    ConfigPlaylists_Reload();

    if (!Config_ValidateAll())
    {
        if (Config_ValidationIsCriticalFailure())
        {
            LogError("[serverflow] config reload completed with critical validation errors");
        }
        else if (!Config_ApplySafeFallbackScenario())
        {
            LogError("[serverflow] config reload completed with validation errors (safe fallback unavailable)");
        }
    }
}
