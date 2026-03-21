#if defined _SERVERFLOW_CONFIG_CORE_INC_
 #endinput
#endif
#define _SERVERFLOW_CONFIG_CORE_INC_

void ConfigCore_InitConVars()
{
    g_CvarVoteDuration = CreateConVar("sm_serverflow_vote_duration", "20.0", "Mode vote duration in seconds.", FCVAR_NOTIFY, true, MODE_VOTE_DURATION_MIN, true, MODE_VOTE_DURATION_MAX);
    g_CvarVoteCooldown = CreateConVar("sm_serverflow_vote_cooldown", "120", "Cooldown between mode votes in seconds.", FCVAR_NOTIFY, true, float(MODE_VOTE_COOLDOWN_MIN), true, float(MODE_VOTE_COOLDOWN_MAX));
    g_CvarVoteMinPlayers = CreateConVar("sm_serverflow_vote_min_players", "4", "Minimum human players required to start a mode vote.", FCVAR_NOTIFY, true, float(MODE_VOTE_MIN_PLAYERS_MIN), true, float(MODE_VOTE_MIN_PLAYERS_MAX));
    g_CvarVerbose = CreateConVar("sm_serverflow_verbose", "1", "Verbose logging for serverflow (0/1).", FCVAR_NOTIFY, true, 0.0, true, 1.0);
    g_CvarPendingConfirmWindow = CreateConVar("sm_serverflow_pending_confirm_window", "20", "Pending change confirmation window in seconds.", FCVAR_NOTIFY, true, 5.0, true, 60.0);
    g_CvarLateJoinPolicy = CreateConVar("sm_serverflow_late_join_policy", "1", "Default late join policy (0=Disallow, 1=Spectator, 2=Allow).", FCVAR_NOTIFY, true, 0.0, true, 2.0);
    g_CvarAfkPolicy = CreateConVar("sm_serverflow_afk_policy", "2", "Default AFK policy (0=None, 1=Warn, 2=Spec, 3=Kick).", FCVAR_NOTIFY, true, 0.0, true, 3.0);
    g_CvarAfkSeconds = CreateConVar("sm_serverflow_afk_seconds", "120", "Seconds before a player is considered AFK.", FCVAR_NOTIFY, true, 30.0, true, 600.0);
    g_CvarVotePolicy = CreateConVar("sm_serverflow_vote_policy", "0", "Voting policy (0=Majority, 1=Roulette, 2=EqualWeight, 3=RandomTop2, 4=Threshold).", FCVAR_NOTIFY, true, 0.0, true, 4.0);
    g_CvarVoteQuorum = CreateConVar("sm_serverflow_vote_quorum", "0.0", "Required participation % (0.0-1.0).", FCVAR_NOTIFY, true, 0.0, true, 1.0);
    g_CvarVoteConsensus = CreateConVar("sm_serverflow_vote_consensus", "0.0", "Required win % for Majority (0.0-1.0).", FCVAR_NOTIFY, true, 0.0, true, 1.0);
    g_CvarVoteHistoryBias = CreateConVar("sm_serverflow_vote_history_bias", "0", "Enable history-based penalty (0/1).", FCVAR_NOTIFY, true, 0.0, true, 1.0);
    g_CvarVoteHistorySize = CreateConVar("sm_serverflow_vote_history_size", "5", "Number of past scenarios to remember (1-10).", FCVAR_NOTIFY, true, 1.0, true, 10.0);
    g_CvarVoteHistoryMode = CreateConVar("sm_serverflow_vote_history_mode", "0", "History mode (0=Weight, 1=Block).", FCVAR_NOTIFY, true, 0.0, true, 1.0);
    g_CvarVoteHistoryWeight = CreateConVar("sm_serverflow_vote_history_weight", "0.2", "Weight multiplier for recently played modes (0.0-1.0).", FCVAR_NOTIFY, true, 0.0, true, 1.0);
    g_CvarVoteThreshold = CreateConVar("sm_serverflow_vote_threshold", "5", "Votes required for Flash/Threshold win.", FCVAR_NOTIFY, true, 0.0, true, 64.0);
}

void ConfigCore_RefreshRuntime()
{
    g_VoteDuration = g_CvarVoteDuration.FloatValue;
    g_VoteCooldown = g_CvarVoteCooldown.IntValue;
    g_VoteMinPlayers = g_CvarVoteMinPlayers.IntValue;
    g_PendingConfirmWindow = g_CvarPendingConfirmWindow.IntValue;
    g_LateJoinPolicy = view_as<LateJoinPolicy>(g_CvarLateJoinPolicy.IntValue);
    g_AfkPolicy = view_as<AfkPolicy>(g_CvarAfkPolicy.IntValue);
    g_AfkSeconds = g_CvarAfkSeconds.IntValue;
    g_VotePolicy = view_as<VotePolicy>(g_CvarVotePolicy.IntValue);
    g_VoteQuorum = g_CvarVoteQuorum.FloatValue;
    g_VoteConsensus = g_CvarVoteConsensus.FloatValue;
    g_VoteHistoryBias = g_CvarVoteHistoryBias.BoolValue;
    g_VoteHistorySize = g_CvarVoteHistorySize.IntValue;
    g_VoteHistoryMode = g_CvarVoteHistoryMode.IntValue;
    g_VoteHistoryWeight = g_CvarVoteHistoryWeight.FloatValue;
    g_VoteThreshold = g_CvarVoteThreshold.IntValue;
}
