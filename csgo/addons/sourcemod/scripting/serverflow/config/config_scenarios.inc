#if defined _SERVERFLOW_CONFIG_SCENARIOS_INC_
 #endinput
#endif
#define _SERVERFLOW_CONFIG_SCENARIOS_INC_

#define SERVERFLOW_SCENARIOS_CONFIG "addons/sourcemod/configs/serverflow/scenarios.cfg"

static bool Scenarios_ParseMaplistPolicy(const char[] value, MaplistPolicy &policy)
{
    if (StrEqual(value, "playlist", false) || StrEqual(value, "", false))
    {
        policy = MaplistPolicy_Playlist;
        return true;
    }

    if (StrEqual(value, "scenario", false))
    {
        policy = MaplistPolicy_Scenario;
        return true;
    }

    return false;
}

bool Scenarios_ValidateEntry(const ModeInfo scenario, int index)
{
    bool valid = true;

    if (scenario.id[0] == '\0' || scenario.routerAlias[0] == '\0')
    {
        LogError("[serverflow] invalid scenario at index %d: id/alias cannot be empty", index);
        valid = false;
    }

    if (scenario.playlistId[0] == '\0')
    {
        LogError("[serverflow] invalid scenario '%s': playlist_id cannot be empty", scenario.id);
        valid = false;
    }

    return valid;
}

bool Scenarios_Register(const ModeInfo scenario)
{
    if (g_ScenarioCount >= MAX_RUNTIME_MODES)
    {
        LogError("[serverflow] scenario limit reached (%d)", MAX_RUNTIME_MODES);
        return false;
    }

    g_Scenarios[g_ScenarioCount] = scenario;
    g_ScenarioCount++;
    return true;
}

int Scenarios_FindById(const char[] id)
{
    return FindModeById(id);
}

bool Scenarios_LoadFromConfig()
{
    g_ScenarioCount = 0;

    char path[PLATFORM_MAX_PATH];
    strcopy(path, sizeof(path), SERVERFLOW_SCENARIOS_CONFIG);

    KeyValues kv = new KeyValues("ServerFlowScenarios");
    if (!kv.ImportFromFile(path))
    {
        LogError("[serverflow] failed to load scenarios config: %s", path);
        delete kv;
        return false;
    }

    if (!kv.GotoFirstSubKey(false))
    {
        LogError("[serverflow] scenarios config is empty: %s", path);
        delete kv;
        return false;
    }

    do
    {
        ModeInfo scenario;
        kv.GetSectionName(scenario.id, sizeof(scenario.id));
        kv.GetString("router_alias", scenario.routerAlias, sizeof(scenario.routerAlias), "");
        kv.GetString("name_phrase", scenario.namePhrase, sizeof(scenario.namePhrase), "mode.name.default");
        scenario.gameType = kv.GetNum("game_type", 0);
        scenario.gameMode = kv.GetNum("game_mode", 0);
        kv.GetString("mapgroup", scenario.mapgroup, sizeof(scenario.mapgroup), "");
        kv.GetString("start_map", scenario.startMap, sizeof(scenario.startMap), "");
        kv.GetString("cfg_file", scenario.cfgFile, sizeof(scenario.cfgFile), "");

        kv.GetString("playlist_id", scenario.playlistId, sizeof(scenario.playlistId), scenario.id);

        char maplistPolicyRaw[24];
        kv.GetString("maplist_policy", maplistPolicyRaw, sizeof(maplistPolicyRaw), "playlist");
        if (!Scenarios_ParseMaplistPolicy(maplistPolicyRaw, scenario.maplistPolicy))
        {
            LogError("[serverflow] invalid maplist_policy '%s' for scenario '%s'; using playlist", maplistPolicyRaw, scenario.id);
            scenario.maplistPolicy = MaplistPolicy_Playlist;
        }

        if (!Scenarios_ValidateEntry(scenario, g_ScenarioCount))
        {
            continue;
        }

        if (Scenarios_FindById(scenario.id) != -1)
        {
            LogError("[serverflow] duplicate scenario id '%s'", scenario.id);
            continue;
        }

        Scenarios_Register(scenario);
    }
    while (kv.GotoNextKey(false));

    delete kv;
    return (g_ScenarioCount > 0);
}
