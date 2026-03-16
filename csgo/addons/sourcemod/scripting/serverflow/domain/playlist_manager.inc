#if defined _SERVERFLOW_PLAYLIST_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_PLAYLIST_MANAGER_INC_

static int PlaylistManager_FindPlaylistById(const char[] playlistId)
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

static int PlaylistManager_BuildCriticalFallbackPool(const char[] scenarioId, char maps[][64], int maxMaps)
{
    if (maxMaps <= 0)
    {
        return 0;
    }

    if (StrEqual(scenarioId, "dz", false))
    {
        strcopy(maps[0], 64, "dz_blacksite");
    }
    else if (StrEqual(scenarioId, "comp", false))
    {
        strcopy(maps[0], 64, "de_dust2");
    }
    else if (StrEqual(scenarioId, "casual", false))
    {
        strcopy(maps[0], 64, "de_mirage");
    }
    else
    {
        return 0;
    }

    if (!IsMapValid(maps[0]))
    {
        strcopy(maps[0], 64, SAFE_HUB_MAP);
    }

    return 1;
}

static int PlaylistManager_GetScenarioPlaylistIndex(int scenarioIndex)
{
    if (scenarioIndex < 0 || scenarioIndex >= g_ScenarioCount)
    {
        return -1;
    }

    if (g_Scenarios[scenarioIndex].maplistPolicy == MaplistPolicy_Scenario)
    {
        return PlaylistManager_FindPlaylistById(g_Scenarios[scenarioIndex].id);
    }

    return PlaylistManager_FindPlaylistById(g_Scenarios[scenarioIndex].playlistId);
}

bool Playlists_LoadFromConfig()
{
    for (int i = 0; i < MAX_RUNTIME_MODES; i++)
    {
        if (g_ModeMapCache[i] != null)
        {
            delete g_ModeMapCache[i];
        }

        g_ModeMapCache[i] = new ArrayList(ByteCountToCells(64));
        g_ModeMapCacheLoaded[i] = false;
        g_ModeMapCacheError[i][0] = '\0';
        g_PlaylistCursor[i] = 0;
    }

    return true;
}

bool Playlist_IsMapAllowed(int scenarioIndex, const char[] mapName)
{
    if (scenarioIndex < 0 || scenarioIndex >= g_ScenarioCount || g_ModeMapCache[scenarioIndex] == null)
    {
        return false;
    }

    char entry[64];
    for (int i = 0; i < g_ModeMapCache[scenarioIndex].Length; i++)
    {
        g_ModeMapCache[scenarioIndex].GetString(i, entry, sizeof(entry));
        if (StrEqual(entry, mapName, false))
        {
            return true;
        }
    }

    return false;
}

void Playlist_RecordMapPlayed(const char[] scenarioId, const char[] mapName)
{
    int scenarioIndex = FindModeById(scenarioId);
    if (scenarioIndex == -1 || g_ModeMapCache[scenarioIndex] == null || g_ModeMapCache[scenarioIndex].Length <= 0)
    {
        return;
    }

    int length = g_ModeMapCache[scenarioIndex].Length;
    char map[64];
    for (int i = 0; i < length; i++)
    {
        g_ModeMapCache[scenarioIndex].GetString(i, map, sizeof(map));
        if (StrEqual(map, mapName, false))
        {
            g_PlaylistCursor[scenarioIndex] = (i + 1) % length;
            return;
        }
    }
}

bool Playlist_GetNextMap(const char[] scenarioId, char[] mapName, int maxlen)
{
    int scenarioIndex = FindModeById(scenarioId);
    if (scenarioIndex == -1 || g_ModeMapCache[scenarioIndex] == null || g_ModeMapCache[scenarioIndex].Length <= 0)
    {
        mapName[0] = '\0';
        return false;
    }

    int playlistIndex = PlaylistManager_GetScenarioPlaylistIndex(scenarioIndex);
    if (playlistIndex == -1)
    {
        mapName[0] = '\0';
        return false;
    }

    int length = g_ModeMapCache[scenarioIndex].Length;
    int selected = g_PlaylistCursor[scenarioIndex] % length;

    if (g_Playlists[playlistIndex].rotationMode == RotationMode_Random)
    {
        selected = GetRandomInt(0, length - 1);
    }

    g_ModeMapCache[scenarioIndex].GetString(selected, mapName, maxlen);

    if (g_Playlists[playlistIndex].rotationMode == RotationMode_Sequential)
    {
        g_PlaylistCursor[scenarioIndex] = (selected + 1) % length;
    }

    return true;
}

int Playlist_GetVotePool(const char[] scenarioId, char maps[][64], int maxMaps)
{
    int scenarioIndex = FindModeById(scenarioId);
    if (scenarioIndex == -1 || g_ModeMapCache[scenarioIndex] == null)
    {
        return PlaylistManager_BuildCriticalFallbackPool(scenarioId, maps, maxMaps);
    }

    int count = g_ModeMapCache[scenarioIndex].Length;
    if (count <= 0)
    {
        return PlaylistManager_BuildCriticalFallbackPool(scenarioId, maps, maxMaps);
    }

    if (count > maxMaps)
    {
        count = maxMaps;
    }

    for (int i = 0; i < count; i++)
    {
        g_ModeMapCache[scenarioIndex].GetString(i, maps[i], 64);
    }

    return count;
}

bool Playlist_ResolveNextMap(const char[] scenarioId, const char[] requestedMap, char[] resolvedMap, int maxlen)
{
    int scenarioIndex = FindModeById(scenarioId);
    if (scenarioIndex == -1)
    {
        strcopy(resolvedMap, maxlen, SAFE_HUB_MAP);
        return false;
    }

    if (requestedMap[0] != '\0' && Playlist_IsMapAllowed(scenarioIndex, requestedMap))
    {
        strcopy(resolvedMap, maxlen, requestedMap);
        return true;
    }

    if (Playlist_GetNextMap(scenarioId, resolvedMap, maxlen))
    {
        return true;
    }

    int playlistIndex = PlaylistManager_GetScenarioPlaylistIndex(scenarioIndex);
    if (playlistIndex != -1 && g_Playlists[playlistIndex].fallbackMap[0] != '\0' && IsMapValid(g_Playlists[playlistIndex].fallbackMap))
    {
        strcopy(resolvedMap, maxlen, g_Playlists[playlistIndex].fallbackMap);
        return true;
    }

    strcopy(resolvedMap, maxlen, SAFE_HUB_MAP);
    return false;
}

int Playlist_FilterByPlayerCount(const char[] scenarioId, int players, char maps[][64], int maxMaps)
{
    return Playlist_GetVotePool(scenarioId, maps, maxMaps);
}
