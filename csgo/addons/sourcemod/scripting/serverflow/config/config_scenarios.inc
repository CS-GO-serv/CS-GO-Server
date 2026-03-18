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

// Must be by-reference so fallback values are persisted in the caller.
static void Scenarios_ApplyLegacyFieldFallbacks(KeyValues kv, ModeInfo scenario)
{
    if (scenario.routerAlias[0] == '\0')
    {
        kv.GetString("router", scenario.routerAlias, sizeof(scenario.routerAlias), "");
    }

    if (scenario.playlistId[0] == '\0')
    {
        kv.GetString("playlist", scenario.playlistId, sizeof(scenario.playlistId), "");
    }

    if (scenario.playlistId[0] == '\0')
    {
        strcopy(scenario.playlistId, sizeof(scenario.playlistId), scenario.id);
    }
}

static bool Scenarios_ParseTeamPolicy(const char[] value, TeamPolicy &policy)
{
    if (StrEqual(value, "auto", false))
    {
        policy = TeamPolicy_Auto;
        return true;
    }
    if (StrEqual(value, "manual", false) || StrEqual(value, "open", false))
    {
        policy = TeamPolicy_Manual;
        return true;
    }
    
    policy = TeamPolicy_None;
    return true; // Default fallback is None
}

static bool Scenarios_ParseReadyPolicy(const char[] value, ReadyPolicy &policy)
{
    if (StrEqual(value, "required", false) || StrEqual(value, "percent", false) || StrEqual(value, "all", false))
    {
        policy = ReadyPolicy_Required;
        return true;
    }
    if (StrEqual(value, "optional", false))
    {
        policy = ReadyPolicy_Optional;
        return true;
    }
    
    policy = ReadyPolicy_Disabled;
    return true;
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

void Scenarios_LoadFallbackDefaults()
{
    g_ScenarioCount = 0;

    ModeInfo dz;
    strcopy(dz.id, sizeof(dz.id), "dz");
    strcopy(dz.routerAlias, sizeof(dz.routerAlias), "mode_dz");
    strcopy(dz.namePhrase, sizeof(dz.namePhrase), "Mode Name DZ");
    dz.gameType = 6;
    dz.gameMode = 0;
    strcopy(dz.mapgroup, sizeof(dz.mapgroup), "mg_dz_blacksite");
    strcopy(dz.startMap, sizeof(dz.startMap), "dz_blacksite");
    strcopy(dz.cfgFile, sizeof(dz.cfgFile), "mode_dz.cfg");
    strcopy(dz.playlistId, sizeof(dz.playlistId), "dz");
    dz.maplistPolicy = MaplistPolicy_Playlist;
    Scenarios_Register(dz);

    ModeInfo comp;
    strcopy(comp.id, sizeof(comp.id), "comp");
    strcopy(comp.routerAlias, sizeof(comp.routerAlias), "mode_comp");
    strcopy(comp.namePhrase, sizeof(comp.namePhrase), "Mode Name Comp");
    comp.gameType = 0;
    comp.gameMode = 1;
    strcopy(comp.mapgroup, sizeof(comp.mapgroup), "mg_active");
    strcopy(comp.startMap, sizeof(comp.startMap), "de_mirage");
    strcopy(comp.cfgFile, sizeof(comp.cfgFile), "mode_comp.cfg");
    strcopy(comp.playlistId, sizeof(comp.playlistId), "comp");
    comp.maplistPolicy = MaplistPolicy_Playlist;
    Scenarios_Register(comp);

    ModeInfo casual;
    strcopy(casual.id, sizeof(casual.id), "casual");
    strcopy(casual.routerAlias, sizeof(casual.routerAlias), "mode_casual");
    strcopy(casual.namePhrase, sizeof(casual.namePhrase), "Mode Name Casual");
    casual.gameType = 0;
    casual.gameMode = 0;
    strcopy(casual.mapgroup, sizeof(casual.mapgroup), "mg_casualdelta");
    strcopy(casual.startMap, sizeof(casual.startMap), "de_mirage");
    strcopy(casual.cfgFile, sizeof(casual.cfgFile), "mode_casual.cfg");
    strcopy(casual.playlistId, sizeof(casual.playlistId), "casual");
    casual.maplistPolicy = MaplistPolicy_Playlist;
    Scenarios_Register(casual);

    LogError("[serverflow] loaded fallback in-memory scenarios: %d", g_ScenarioCount);
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
        kv.GetString("playlist_id", scenario.playlistId, sizeof(scenario.playlistId), "");

        char maplistPolicyRaw[24];
        kv.GetString("maplist_policy", maplistPolicyRaw, sizeof(maplistPolicyRaw), "playlist");
        if (!Scenarios_ParseMaplistPolicy(maplistPolicyRaw, scenario.maplistPolicy))
        {
            LogError("[serverflow] invalid maplist_policy '%s' for scenario '%s'; using playlist", maplistPolicyRaw, scenario.id);
            scenario.maplistPolicy = MaplistPolicy_Playlist;
        }

        Scenarios_ApplyLegacyFieldFallbacks(kv, scenario);

        scenario.requirePrematch = !!kv.GetNum("require_prematch", 1);

        char teamPolicyRaw[24];
        kv.GetString("team_policy", teamPolicyRaw, sizeof(teamPolicyRaw), "none");
        Scenarios_ParseTeamPolicy(teamPolicyRaw, scenario.teamPolicy);

        char readyPolicyRaw[24];
        kv.GetString("ready_policy", readyPolicyRaw, sizeof(readyPolicyRaw), "disabled");
        Scenarios_ParseReadyPolicy(readyPolicyRaw, scenario.readyPolicy);

        if (!Scenarios_ValidateEntry(scenario, g_ScenarioCount))
        {
            continue;
        }

        if (Scenarios_FindById(scenario.id) != -1)
        {
            LogError("[serverflow] duplicate scenario id '%s'", scenario.id);
            continue;
        }

        LogMessage("[serverflow] scenario loaded: id=%s playlist=%s mapgroup=%s", scenario.id, scenario.playlistId, scenario.mapgroup);
        Scenarios_Register(scenario);
    }
    while (kv.GotoNextKey(false));

    delete kv;
    return (g_ScenarioCount > 0);
}
