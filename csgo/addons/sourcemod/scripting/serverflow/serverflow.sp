#include <sourcemod>

#pragma semicolon 1
#pragma newdecls required

#include "core/constants.inc"
#include "core/enums.inc"
#include "core/types.inc"
#include "core/globals.inc"
#include "core/forwards.inc"
#include "core/helpers.inc"
#include "core/debug.inc"

#include "diagnostics/audit_logger.inc"

#include "runtime/session_store.inc"
#include "runtime/state_manager.inc"
#include "runtime/policy_manager.inc"
#include "runtime/pending_change.inc"
#include "runtime/transition_manager.inc"

#include "config/config_core.inc"
#include "config/config_scenarios.inc"
#include "config/config_playlists.inc"
#include "config/config_validation.inc"
#include "config/config_loader.inc"

#include "domain/scenario_registry.inc"
#include "domain/playlist_manager.inc"
#include "domain/player_state_manager.inc"
#include "domain/ready_manager.inc"
#include "domain/countdown_manager.inc"
#include "domain/team_manager.inc"
#include "domain/vote_manager.inc"
#include "domain/admin_override_manager.inc"

#include "presentation/status_presenter.inc"
#include "presentation/chat_notifier.inc"
#include "presentation/menu_play.inc"
#include "presentation/menu_vote.inc"
#include "presentation/menu_team.inc"
#include "presentation/menu_admin.inc"

#include "commands/cmd_player.inc"
#include "commands/cmd_admin.inc"
#include "commands/command_router.inc"

#include "integrations/client_hooks.inc"
#include "integrations/game_events.inc"
#include "integrations/timers.inc"
#include "integrations/afk_monitor.inc"

#include "diagnostics/health_check.inc"

public Plugin myinfo =
{
    name = "ServerFlow",
    author = "Codex",
    description = "Server flow orchestration/bootstrap plugin",
    version = "2.0.0"
};

static void Bootstrap_Minimal()
{
    Forwards_Init();
    Session_Init();
    Config_LoadAll();
    PlayerState_ResetAll();

    CommandRouter_Register();
    Integrations_RegisterEvents();
    AfkMonitor_Init();
}

public void OnPluginStart()
{
<<<<<<< Updated upstream
    LoadTranslations("serverflow.phrases");
=======
    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "translations/serverflow.phrases.txt");
    if (FileExists(path))
    {
        LoadTranslations("serverflow.phrases");
    }
    else
    {
        LogError("[ServerFlow] Missing translations/serverflow.phrases.txt! Required plugin resource.");
    }
>>>>>>> Stashed changes

    Bootstrap_Minimal();
    Health_RunFullCheck("OnPluginStart");

    ModeDebug("serverflow boot complete");
}

public void OnMapStart()
{
    Config_ReloadAll();
    Health_RunFullCheck("OnMapStart");
}

public void OnClientPostAdminCheck(int client)
{
    PlayerState_OnClientAuthorized(client);
    Policy_ApplyLateJoinGate(client, "client_post_admin_check");
    AfkMonitor_RecordClientActivity(client, "client_post_admin_check");
}

public void OnClientDisconnect(int client)
{
    PlayerState_OnClientDisconnected(client);
}

public void OnServerStateChanged(ServerState oldState, ServerState newState, const char[] reason)
{
    Notify_StateChanged(oldState, newState, reason);
}

public Action OnPlayerRunCmd(int client, int &buttons, int &impulse, float vel[3], float angles[3], int &weapon)
{
    if (!IsValidClient(client))
    {
        return Plugin_Continue;
    }

    if (buttons != 0 || impulse != 0 || FloatAbs(vel[0]) > 0.0 || FloatAbs(vel[1]) > 0.0 || FloatAbs(vel[2]) > 0.0)
    {
        AfkMonitor_RecordClientActivity(client, "run_cmd");
    }

    return Plugin_Continue;
}
