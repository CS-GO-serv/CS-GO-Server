#if defined _SERVERFLOW_CONFIG_LOADER_INC_
 #endinput
#endif
#define _SERVERFLOW_CONFIG_LOADER_INC_

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
            SetFailState("[serverflow] config validation failed");
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
        LogError("[serverflow] config reload completed with validation errors");
    }
}
