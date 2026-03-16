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



static bool Health_IsKnownServerState(ServerState state)
{
    return (state >= ServerState_Lobby && state <= ServerState_Transition);
}

HealthStatus Health_CheckRuntimeInvariants()
{
    PendingChange pending;
    Session_GetPendingChange(pending);

    if (!Health_IsKnownServerState(g_Session.currentState))
    {
        ModeDebug("health fail: invalid state enum value=%d", g_Session.currentState);
        return HealthStatus_Fail;
    }

    if (pending.state == Pending_None)
    {
        if (g_Session.queuedScenario[0] != '\0' && !StrEqual(g_Session.queuedScenario, g_Session.currentScenario, false))
        {
            ModeDebug("health fail: queued scenario diverges without pending (current=%s queued=%s)", g_Session.currentScenario, g_Session.queuedScenario);
            return HealthStatus_Fail;
        }

        if (pending.scenarioId[0] != '\0' || pending.mapName[0] != '\0' || pending.requestedAt != 0)
        {
            ModeDebug("health fail: pending payload exists while state is none");
            return HealthStatus_Fail;
        }
    }
    else
    {
        if (!Health_IsKnownServerState(pending.allowedApplyState))
        {
            ModeDebug("health fail: pending has invalid allowed state=%d", pending.allowedApplyState);
            return HealthStatus_Fail;
        }

        if (pending.scenarioId[0] == '\0' && pending.mapName[0] == '\0')
        {
            ModeDebug("health fail: pending has no scenario and no map");
            return HealthStatus_Fail;
        }

        if (pending.requestedAt <= 0)
        {
            ModeDebug("health fail: pending has invalid requestedAt=%d", pending.requestedAt);
            return HealthStatus_Fail;
        }

        if (g_Session.queuedScenario[0] != '\0' && pending.scenarioId[0] != '\0' && !StrEqual(g_Session.queuedScenario, pending.scenarioId, false))
        {
            ModeDebug("health fail: queued scenario differs from pending scenario (queued=%s pending=%s)", g_Session.queuedScenario, pending.scenarioId);
            return HealthStatus_Fail;
        }

        if (pending.state == Pending_Confirmed || pending.state == Pending_Applying)
        {
            if (pending.confirmedAt <= 0)
            {
                ModeDebug("health fail: pending confirmed/applying but confirmedAt=%d", pending.confirmedAt);
                return HealthStatus_Fail;
            }
        }
        else if (pending.confirmedAt != 0)
        {
            ModeDebug("health fail: pending not confirmed but confirmedAt=%d", pending.confirmedAt);
            return HealthStatus_Fail;
        }
    }

    if (Session_IsVoteInProgress() && pending.state != Pending_None)
    {
        ModeDebug("health fail: vote and pending change are active concurrently");
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
    HealthStatus links = Health_CheckScenarioPlaylistLinks();
    HealthStatus playlists = Health_CheckPlaylists();
    HealthStatus runtime = Health_CheckRuntimeInvariants();

    HealthStatus finalStatus = HealthStatus_Ok;
    if (scenarios == HealthStatus_Fail || links == HealthStatus_Fail || playlists == HealthStatus_Fail || runtime == HealthStatus_Fail)
    {
        finalStatus = HealthStatus_Fail;
    }
    else if (scenarios == HealthStatus_Warn || links == HealthStatus_Warn || playlists == HealthStatus_Warn || runtime == HealthStatus_Warn)
    {
        finalStatus = HealthStatus_Warn;
    }

    ModeDebug(
        "health self-check [%s]: status=%d scenarios=%d links=%d playlists=%d runtime=%d state=%d", 
        source,
        finalStatus,
        scenarios,
        links,
        playlists,
        runtime,
        Session_GetState()
    );
}
