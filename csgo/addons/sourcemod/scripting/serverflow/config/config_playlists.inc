#if defined _SERVERFLOW_CONFIG_PLAYLISTS_INC_
 #endinput
#endif
#define _SERVERFLOW_CONFIG_PLAYLISTS_INC_

#define SERVERFLOW_PLAYLISTS_CONFIG "addons/sourcemod/configs/serverflow/playlists.cfg"

static bool Playlists_ParseRotationMode(const char[] value, RotationMode &mode)
{
    if (StrEqual(value, "random", false))
    {
        mode = RotationMode_Random;
        return true;
    }

    mode = RotationMode_Sequential;
    return StrEqual(value, "sequential", false) || StrEqual(value, "", false);
}

static int PlaylistDefs_FindById(const char[] playlistId)
{
    for (int i = 0; i < g_PlaylistCount; i++)
    {
        if (StrEqual(g_Playlists[i].id, playlistId, false))
        {
            return i;
        }
    }

    return -1;
}

bool Playlists_ValidateEntry(int scenarioIndex, const char[] mapName)
{
    if (!Scenario_IsValidIndex(scenarioIndex) || mapName[0] == '\0')
    {
        return false;
    }

    if (!IsMapValid(mapName))
    {
        LogMessage("[serverflow] skipping invalid map '%s' for scenario '%s'", mapName, g_Scenarios[scenarioIndex].id);
        return false;
    }

    return true;
}

int Playlists_FindById(const char[] scenarioId)
{
    return FindModeById(scenarioId);
}

bool Playlists_FindMap(int scenarioIndex, const char[] mapName)
{
    return Playlist_IsMapAllowed(scenarioIndex, mapName);
}

bool ConfigPlaylists_LoadDefinitions()
{
    g_PlaylistCount = 0;

    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_Game, path, sizeof(path), "%s", SERVERFLOW_PLAYLISTS_CONFIG);

    KeyValues kv = new KeyValues("ServerFlowPlaylists");
    if (!kv.ImportFromFile(path))
    {
        LogError("[serverflow] failed to load playlists config: %s", path);
        delete kv;
        return false;
    }

    if (!kv.GotoFirstSubKey(false))
    {
        LogError("[serverflow] playlists config is empty: %s", path);
        delete kv;
        return false;
    }

    do
    {
        if (g_PlaylistCount >= MAX_RUNTIME_MODES)
        {
            LogError("[serverflow] playlist limit reached (%d)", MAX_RUNTIME_MODES);
            break;
        }

        PlaylistDefinition playlist;
        kv.GetSectionName(playlist.id, sizeof(playlist.id));
        kv.GetString("maplist_file", playlist.mapListFile, sizeof(playlist.mapListFile), "");
        kv.GetString("fallback_map", playlist.fallbackMap, sizeof(playlist.fallbackMap), "");

        char rotationModeRaw[24];
        kv.GetString("rotation_mode", rotationModeRaw, sizeof(rotationModeRaw), "sequential");
        if (!Playlists_ParseRotationMode(rotationModeRaw, playlist.rotationMode))
        {
            LogError("[serverflow] invalid playlist rotation_mode '%s' for playlist '%s'; using sequential", rotationModeRaw, playlist.id);
            playlist.rotationMode = RotationMode_Sequential;
        }

        if (playlist.id[0] == '\0' || playlist.mapListFile[0] == '\0')
        {
            LogError("[serverflow] invalid playlist entry: id/maplist_file cannot be empty");
            continue;
        }

        if (PlaylistDefs_FindById(playlist.id) != -1)
        {
            LogError("[serverflow] duplicate playlist id '%s'", playlist.id);
            continue;
        }

        g_Playlists[g_PlaylistCount] = playlist;
        g_PlaylistCount++;
    }
    while (kv.GotoNextKey(false));

    delete kv;
    return g_PlaylistCount > 0;
}

bool ConfigPlaylists_Reload()
{
    Playlists_LoadFromConfig();

    if (!ConfigPlaylists_LoadDefinitions())
    {
        return false;
    }

    for (int scenarioIndex = 0; scenarioIndex < g_ScenarioCount; scenarioIndex++)
    {
        int playlistIndex = -1;
        if (g_Scenarios[scenarioIndex].maplistPolicy == MaplistPolicy_Scenario)
        {
            playlistIndex = PlaylistDefs_FindById(g_Scenarios[scenarioIndex].id);
        }
        else
        {
            playlistIndex = PlaylistDefs_FindById(g_Scenarios[scenarioIndex].playlistId);
        }
        if (playlistIndex == -1)
        {
            Format(g_ModeMapCacheError[scenarioIndex], sizeof(g_ModeMapCacheError[]), "playlist not found: %s", g_Scenarios[scenarioIndex].playlistId);
            continue;
        }

        char mapListPath[PLATFORM_MAX_PATH];
        strcopy(mapListPath, sizeof(mapListPath), g_Scenarios[scenarioIndex].mapListFile);

        if (!FileExists(mapListPath))
        {
            Format(g_ModeMapCacheError[scenarioIndex], sizeof(g_ModeMapCacheError[]), "maplist not found: %s", mapListPath);
            continue;
        }

        File file = OpenFile(mapListPath, "r");
        if (file == null)
        {
            Format(g_ModeMapCacheError[scenarioIndex], sizeof(g_ModeMapCacheError[]), "cannot open %s", mapListPath);
            continue;
        }

        char line[128];
        while (!file.EndOfFile() && file.ReadLine(line, sizeof(line)))
        {
            TrimString(line);
            if (line[0] == '\0' || line[0] == ';' || (line[0] == '/' && line[1] == '/'))
            {
                continue;
            }

            if (!Playlists_ValidateEntry(scenarioIndex, line))
            {
                continue;
            }

            g_ModeMapCache[scenarioIndex].PushString(line);
        }

        delete file;

        if (g_ModeMapCache[scenarioIndex].Length <= 0)
        {
            Format(g_ModeMapCacheError[scenarioIndex], sizeof(g_ModeMapCacheError[]), "no valid maps in %s", mapListPath);
            continue;
        }

        g_ModeMapCacheLoaded[scenarioIndex] = true;
        strcopy(g_ModeMapCacheError[scenarioIndex], sizeof(g_ModeMapCacheError[]), "ok");
    }

    return true;
}
