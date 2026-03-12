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

#include "runtime/session_store.inc"
#include "runtime/state_manager.inc"
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

#include "commands/command_router.inc"
#include "commands/cmd_player.inc"
#include "commands/cmd_admin.inc"

#include "diagnostics/audit_logger.inc"
#include "diagnostics/health_check.inc"
#include "diagnostics/state_dump.inc"

#include "integrations/game_events.inc"
#include "integrations/map_control.inc"
#include "integrations/timers.inc"
#include "integrations/client_hooks.inc"
#include "integrations/afk_monitor.inc"

public Plugin myinfo =
{
    name = "ServerFlow",
    author = "Codex",
    description = "Server flow orchestration/bootstrap plugin",
    version = "2.0.0"
};

public void OnPluginStart()
{
    LoadTranslations("mode_vote.phrases");

    Forwards_Init();
    Session_Init();
    Config_LoadAll();

    CommandRouter_Register();
    Integrations_RegisterEvents();
    Health_RunFullCheck();

    ModeDebug("serverflow boot complete");
}

public void OnMapStart()
{
    Config_ReloadAll();
}
