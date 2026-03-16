#if defined _SERVERFLOW_CONFIG_VALIDATION_INC_
 #endinput
#endif
#define _SERVERFLOW_CONFIG_VALIDATION_INC_

static int Validation_FindPlaylistById(const char[] playlistId)
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

bool Config_ValidationIsCriticalFailure()
{
    if (g_ScenarioCount <= 0)
    {
        return true;
    }

    if (g_PlaylistCount <= 0)
    {
        return true;
    }

    for (int i = 0; i < g_ScenarioCount; i++)
    {
        for (int j = i + 1; j < g_ScenarioCount; j++)
        {
            if (StrEqual(g_Scenarios[i].id, g_Scenarios[j].id, false))
            {
                return true;
            }

            if (StrEqual(g_Scenarios[i].routerAlias, g_Scenarios[j].routerAlias, false))
            {
                return true;
            }
        }

        int playlistIndex = -1;
        if (g_Scenarios[i].maplistPolicy == MaplistPolicy_Scenario)
        {
            playlistIndex = Validation_FindPlaylistById(g_Scenarios[i].id);
        }
        else
        {
            playlistIndex = Validation_FindPlaylistById(g_Scenarios[i].playlistId);
        }

        if (playlistIndex == -1)
        {
            return true;
        }

        if (g_Scenarios[i].startMap[0] != '\0' && !Playlist_IsMapAllowed(i, g_Scenarios[i].startMap) && !IsMapValid(g_Scenarios[i].startMap))
        {
            return true;
        }
    }

    return false;
}

bool Config_ValidateAll()
{
    bool ok = true;
    bool degraded = false;
    g_ConfigValidationDegraded = false;

    if (g_ScenarioCount <= 0)
    {
        LogError("[serverflow] validation failed: no scenarios loaded");
        g_ConfigValidationDegraded = false;
        return false;
    }

    if (g_PlaylistCount <= 0)
    {
        LogError("[serverflow] validation failed: no playlists loaded");
        g_ConfigValidationDegraded = false;
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

        int playlistIndex = -1;
        if (g_Scenarios[i].maplistPolicy == MaplistPolicy_Scenario)
        {
            playlistIndex = Validation_FindPlaylistById(g_Scenarios[i].id);
        }
        else
        {
            playlistIndex = Validation_FindPlaylistById(g_Scenarios[i].playlistId);
        }

        if (playlistIndex == -1)
        {
            ok = false;
            LogError("[serverflow] scenario '%s' has unresolved playlist link (policy=%d, playlist_id='%s')", g_Scenarios[i].id, g_Scenarios[i].maplistPolicy, g_Scenarios[i].playlistId);
        }

        if (g_ModeMapCache[i] == null || g_ModeMapCache[i].Length <= 0)
        {
            degraded = true;
            LogMessage("[serverflow] validation degraded: scenario '%s' has empty playlist (%s)", g_Scenarios[i].id, g_ModeMapCacheError[i]);
        }

        if (g_Scenarios[i].startMap[0] != '\0' && !Playlist_IsMapAllowed(i, g_Scenarios[i].startMap) && !IsMapValid(g_Scenarios[i].startMap))
        {
            ok = false;
            LogError("[serverflow] invalid start_map '%s' for scenario '%s'", g_Scenarios[i].startMap, g_Scenarios[i].id);
        }
    }

    g_ConfigValidationDegraded = (ok && degraded);
    return ok;
}
