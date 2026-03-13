#if defined _SERVERFLOW_HEALTH_CHECK_INC_
 #endinput
#endif
#define _SERVERFLOW_HEALTH_CHECK_INC_

static int Health_FindPlaylistById(const char[] playlistId)
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

HealthStatus Health_CheckScenarios()
{
    if (Scenario_GetCount() <= 0)
    {
        ModeDebug("health fail: no scenarios loaded");
        return HealthStatus_Fail;
    }

    return HealthStatus_Ok;
}

HealthStatus Health_CheckScenarioPlaylistLinks()
{
    bool hasWarnings = false;

    if (g_PlaylistCount <= 0)
    {
        ModeDebug("health fail: no playlist definitions loaded");
        return HealthStatus_Fail;
    }

    for (int i = 0; i < Scenario_GetCount(); i++)
    {
        int playlistIndex = -1;
        if (g_Scenarios[i].maplistPolicy == MaplistPolicy_Scenario)
        {
            playlistIndex = Health_FindPlaylistById(g_Scenarios[i].id);
        }
        else
        {
            playlistIndex = Health_FindPlaylistById(g_Scenarios[i].playlistId);
        }

        if (playlistIndex == -1)
        {
            ModeDebug("health fail: scenario '%s' unresolved playlist link (policy=%d, playlist_id='%s')", g_Scenarios[i].id, g_Scenarios[i].maplistPolicy, g_Scenarios[i].playlistId);
            return HealthStatus_Fail;
        }

        if (g_Scenarios[i].maplistPolicy == MaplistPolicy_Scenario && !StrEqual(g_Scenarios[i].id, g_Scenarios[i].playlistId, false))
        {
            hasWarnings = true;
            ModeDebug("health warn: scenario '%s' uses scenario policy but playlist_id='%s'", g_Scenarios[i].id, g_Scenarios[i].playlistId);
        }
    }

    return hasWarnings ? HealthStatus_Warn : HealthStatus_Ok;
}

HealthStatus Health_CheckPlaylists()
{
    bool hasWarnings = false;

    for (int i = 0; i < Scenario_GetCount(); i++)
    {
        if (g_ModeMapCache[i] == null || g_ModeMapCache[i].Length <= 0)
        {
            ModeDebug("health fail: scenario '%s' playlist unavailable (%s)", g_Scenarios[i].id, g_ModeMapCacheError[i]);
            return HealthStatus_Fail;
        }

        if (!g_ModeMapCacheLoaded[i])
        {
            hasWarnings = true;
            ModeDebug("health warn: scenario '%s' playlist marked as not loaded (%s)", g_Scenarios[i].id, g_ModeMapCacheError[i]);
        }
    }

    return hasWarnings ? HealthStatus_Warn : HealthStatus_Ok;
}

void Health_RunFullCheck(const char[] source = "runtime")
{
    HealthStatus scenarios = Health_CheckScenarios();
    HealthStatus links = Health_CheckScenarioPlaylistLinks();
    HealthStatus playlists = Health_CheckPlaylists();

    HealthStatus finalStatus = HealthStatus_Ok;
    if (scenarios == HealthStatus_Fail || links == HealthStatus_Fail || playlists == HealthStatus_Fail)
    {
        finalStatus = HealthStatus_Fail;
    }
    else if (scenarios == HealthStatus_Warn || links == HealthStatus_Warn || playlists == HealthStatus_Warn)
    {
        finalStatus = HealthStatus_Warn;
    }

    ModeDebug(
        "health self-check [%s]: status=%d scenarios=%d links=%d playlists=%d state=%d", 
        source,
        finalStatus,
        scenarios,
        links,
        playlists,
        Session_GetState()
    );
}
