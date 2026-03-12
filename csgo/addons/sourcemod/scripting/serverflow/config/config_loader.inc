#if defined _SERVERFLOW_CONFIG_LOADER_INC_
 #endinput
#endif
#define _SERVERFLOW_CONFIG_LOADER_INC_

void Config_LoadAll()
{
    ConfigCore_InitConVars();
    AutoExecConfig(true, "mode_vote", "sourcemod");
    ConfigCore_RefreshRuntime();
    ConfigPlaylists_Reload();
}

void Config_ReloadAll()
{
    ConfigCore_RefreshRuntime();
    ConfigPlaylists_Reload();
}
