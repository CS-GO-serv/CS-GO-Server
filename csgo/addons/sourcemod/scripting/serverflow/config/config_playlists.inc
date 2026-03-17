#if defined _SERVERFLOW_CONFIG_PLAYLISTS_INC_
 #endinput
#endif
#define _SERVERFLOW_CONFIG_PLAYLISTS_INC_

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

bool ConfigPlaylists_Reload()
{
    Playlists_LoadFromConfig();

    for (int scenarioIndex = 0; scenarioIndex < g_ScenarioCount; scenarioIndex++)
    {
        char configuredMapListPath[PLATFORM_MAX_PATH];
        strcopy(configuredMapListPath, sizeof(configuredMapListPath), g_Scenarios[scenarioIndex].mapListFile);

        char mapListPath[PLATFORM_MAX_PATH];
        if (Playlists_IsAbsolutePath(configuredMapListPath))
        {
            strcopy(mapListPath, sizeof(mapListPath), configuredMapListPath);
        }
        else
        {
            BuildPath(Path_Game, mapListPath, sizeof(mapListPath), "%s", configuredMapListPath);
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
