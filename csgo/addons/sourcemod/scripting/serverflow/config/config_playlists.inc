#if defined _SERVERFLOW_CONFIG_PLAYLISTS_INC_
 #endinput
#endif
#define _SERVERFLOW_CONFIG_PLAYLISTS_INC_

#define SERVERFLOW_PLAYLISTS_CONFIG "addons/sourcemod/configs/serverflow/playlists.cfg"

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

static bool Playlists_IsAbsolutePath(const char[] path)
{
    if (path[0] == '/' || path[0] == '\\')
    {
        return true;
    }

    return (IsCharAlpha(path[0]) && path[1] == ':' && (path[2] == '/' || path[2] == '\\'));
}

static int Playlists_FindPlaylistIndexById(const char[] playlistId)
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

static bool Playlists_ParseConfig()
{
    g_PlaylistCount = 0;

    char path[PLATFORM_MAX_PATH];
    strcopy(path, sizeof(path), SERVERFLOW_PLAYLISTS_CONFIG);

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

        PlaylistDefinition pl;
        kv.GetSectionName(pl.id, sizeof(pl.id));
        kv.GetString("maplist_file", pl.mapListFile, sizeof(pl.mapListFile), "");
        kv.GetString("fallback_map", pl.fallbackMap, sizeof(pl.fallbackMap), "");

        char rotRaw[24];
        kv.GetString("rotation_mode", rotRaw, sizeof(rotRaw), "sequential");
        if (StrEqual(rotRaw, "random", false))
        {
            pl.rotationMode = RotationMode_Random;
        }
        else
        {
            pl.rotationMode = RotationMode_Sequential;
        }

        g_Playlists[g_PlaylistCount] = pl;
        g_PlaylistCount++;
        LogMessage("[serverflow] playlist loaded: id=%s maplist=%s", pl.id, pl.mapListFile);
    }
    while (kv.GotoNextKey(false));

    delete kv;
    return (g_PlaylistCount > 0);
}

bool ConfigPlaylists_Reload()
{
    Playlists_LoadFromConfig();
    Playlists_ParseConfig();

    for (int scenarioIndex = 0; scenarioIndex < g_ScenarioCount; scenarioIndex++)
    {
        // Look up the map list file from the PlaylistDefinition, not ModeInfo
        int plIdx = Playlists_FindPlaylistIndexById(g_Scenarios[scenarioIndex].playlistId);
        if (plIdx == -1)
        {
            Format(g_ModeMapCacheError[scenarioIndex], sizeof(g_ModeMapCacheError[]), "no playlist definition for id '%s'", g_Scenarios[scenarioIndex].playlistId);
            continue;
        }

        char configuredMapListPath[PLATFORM_MAX_PATH];
        strcopy(configuredMapListPath, sizeof(configuredMapListPath), g_Playlists[plIdx].mapListFile);

        if (configuredMapListPath[0] == '\0')
        {
            Format(g_ModeMapCacheError[scenarioIndex], sizeof(g_ModeMapCacheError[]), "playlist '%s' has empty maplist_file", g_Playlists[plIdx].id);
            continue;
        }

        char mapListPath[PLATFORM_MAX_PATH];
        if (Playlists_IsAbsolutePath(configuredMapListPath))
        {
            strcopy(mapListPath, sizeof(mapListPath), configuredMapListPath);
        }
        else
        {
            BuildPath(Path_SM, mapListPath, sizeof(mapListPath), "../../%s", configuredMapListPath);
        }

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

