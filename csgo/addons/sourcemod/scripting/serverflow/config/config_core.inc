#if defined _SERVERFLOW_CONFIG_CORE_INC_
 #endinput
#endif
#define _SERVERFLOW_CONFIG_CORE_INC_

void ConfigCore_InitConVars()
{
    g_CvarVoteDuration = CreateConVar("sm_mode_vote_duration", "20.0", "Mode vote duration in seconds.", FCVAR_NOTIFY, true, MODE_VOTE_DURATION_MIN, true, MODE_VOTE_DURATION_MAX);
    g_CvarVoteCooldown = CreateConVar("sm_mode_vote_cooldown", "120", "Cooldown between mode votes in seconds.", FCVAR_NOTIFY, true, float(MODE_VOTE_COOLDOWN_MIN), true, float(MODE_VOTE_COOLDOWN_MAX));
    g_CvarVoteMinPlayers = CreateConVar("sm_mode_vote_min_players", "4", "Minimum human players required to start a mode vote.", FCVAR_NOTIFY, true, float(MODE_VOTE_MIN_PLAYERS_MIN), true, float(MODE_VOTE_MIN_PLAYERS_MAX));
    g_CvarVerbose = CreateConVar("sm_mode_vote_verbose", "1", "Verbose logging for serverflow (0/1).", FCVAR_NOTIFY, true, 0.0, true, 1.0);
    g_CvarPendingConfirmWindow = CreateConVar("sm_mode_pending_confirm_window", "20", "Pending change confirmation window in seconds.", FCVAR_NOTIFY, true, float(MODE_PENDING_CONFIRM_WINDOW_MIN), true, float(MODE_PENDING_CONFIRM_WINDOW_MAX));
}

void ConfigCore_RefreshRuntime()
{
    g_VoteDuration = g_CvarVoteDuration.FloatValue;
    g_VoteCooldown = g_CvarVoteCooldown.IntValue;
    g_VoteMinPlayers = g_CvarVoteMinPlayers.IntValue;
    g_PendingConfirmWindow = g_CvarPendingConfirmWindow.IntValue;
}
