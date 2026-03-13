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
#include "runtime/pending_change.inc"
#include "runtime/transition_manager.inc"

#include "config/config_core.inc"
#include "config/config_scenarios.inc"
#include "config/config_playlists.inc"
#include "config/config_validation.inc"
#include "config/config_loader.inc"

#include "domain/scenario_registry.inc"
#include "domain/playlist_manager.inc"

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
}

public void OnPluginStart()
{
    LoadTranslations("mode_vote.phrases");

    Bootstrap_Minimal();
    Health_RunFullCheck("OnPluginStart");

    ModeDebug("serverflow boot complete");
}

public void OnMapStart()
{
    Config_ReloadAll();
    Health_RunFullCheck("OnMapStart");
}
