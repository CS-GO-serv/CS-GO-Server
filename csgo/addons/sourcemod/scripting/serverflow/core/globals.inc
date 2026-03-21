#if defined _SERVERFLOW_GLOBALS_INC_
 #endinput
#endif
#define _SERVERFLOW_GLOBALS_INC_

ModeInfo g_Scenarios[MAX_RUNTIME_MODES];
int g_ScenarioCount;

PlaylistDefinition g_Playlists[MAX_RUNTIME_MODES];
int g_PlaylistCount;

RuntimeSession g_Session;
PlayerRuntimeState g_PlayerState[MAXPLAYERS + 1];

ConVar g_CvarVoteDuration = null;
ConVar g_CvarVoteCooldown = null;
ConVar g_CvarVoteMinPlayers = null;
ConVar g_CvarVerbose = null;
ConVar g_CvarPendingConfirmWindow = null;
ConVar g_CvarLateJoinPolicy = null;
ConVar g_CvarAfkPolicy = null;
ConVar g_CvarAfkSeconds = null;
ConVar g_CvarVotePolicy = null;
ConVar g_CvarVoteQuorum = null;
ConVar g_CvarVoteConsensus = null;
ConVar g_CvarVoteHistoryBias = null;
ConVar g_CvarVoteThreshold = null;
ConVar g_CvarVoteHistorySize = null;
ConVar g_CvarVoteHistoryMode = null;
ConVar g_CvarVoteHistoryWeight = null;

float g_VoteDuration = MODE_VOTE_DURATION_DEFAULT;
int g_VoteCooldown = MODE_VOTE_COOLDOWN_DEFAULT;
int g_VoteMinPlayers = MODE_VOTE_MIN_PLAYERS_DEFAULT;
int g_PendingConfirmWindow = MODE_PENDING_CONFIRM_WINDOW_DEFAULT;
LateJoinPolicy g_LateJoinPolicy = LateJoinPolicy_Spectator;
AfkPolicy g_AfkPolicy = AfkPolicy_MoveToSpec;
int g_AfkSeconds = 120;

VotePolicy g_VotePolicy = VotePolicy_Majority;
float g_VoteQuorum = 0.0;
float g_VoteConsensus = 0.0;
bool g_VoteHistoryBias = false;
int g_VoteHistorySize = 5;
int g_VoteHistoryMode = 0; // 0=Weight, 1=Block
float g_VoteHistoryWeight = 0.2;
int g_VoteThreshold = 5;
int g_PostMatchDecisionTime = 30;

ArrayList g_ModeMapCache[MAX_RUNTIME_MODES];
bool g_ModeMapCacheLoaded[MAX_RUNTIME_MODES];
char g_ModeMapCacheError[MAX_RUNTIME_MODES][192];
bool g_ConfigValidationDegraded;

int g_PlaylistCursor[MAX_RUNTIME_MODES];
RotationMode g_PlaylistRotationMode = RotationMode_Sequential;

Handle g_AfkMonitorTimer = null;
