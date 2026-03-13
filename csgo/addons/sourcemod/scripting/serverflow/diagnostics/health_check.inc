#if defined _SERVERFLOW_HEALTH_CHECK_INC_
 #endinput
#endif
#define _SERVERFLOW_HEALTH_CHECK_INC_

HealthStatus Health_CheckScenarios()
{
    if (Scenario_GetCount() <= 0)
    {
        ModeDebug("health fail: no scenarios loaded");
        return HealthStatus_Fail;
    }

    return HealthStatus_Ok;
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
    HealthStatus playlists = Health_CheckPlaylists();

    HealthStatus finalStatus = HealthStatus_Ok;
    if (scenarios == HealthStatus_Fail || playlists == HealthStatus_Fail)
    {
        finalStatus = HealthStatus_Fail;
    }
    else if (scenarios == HealthStatus_Warn || playlists == HealthStatus_Warn)
    {
        finalStatus = HealthStatus_Warn;
    }

    ModeDebug(
        "health self-check [%s]: status=%d scenarios=%d playlists=%d state=%d", 
        source,
        finalStatus,
        scenarios,
        playlists,
        Session_GetState()
    );
}
