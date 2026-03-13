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

float g_VoteDuration = MODE_VOTE_DURATION_DEFAULT;
int g_VoteCooldown = MODE_VOTE_COOLDOWN_DEFAULT;
int g_VoteMinPlayers = MODE_VOTE_MIN_PLAYERS_DEFAULT;
int g_PendingConfirmWindow = MODE_PENDING_CONFIRM_WINDOW_DEFAULT;

ArrayList g_ModeMapCache[MAX_RUNTIME_MODES];
bool g_ModeMapCacheLoaded[MAX_RUNTIME_MODES];
char g_ModeMapCacheError[MAX_RUNTIME_MODES][192];

int g_PlaylistCursor[MAX_RUNTIME_MODES];
RotationMode g_PlaylistRotationMode = RotationMode_Sequential;
