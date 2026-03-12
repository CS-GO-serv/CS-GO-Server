#if defined _SERVERFLOW_GLOBALS_INC_
 #endinput
#endif
#define _SERVERFLOW_GLOBALS_INC_

static const ModeInfo g_Modes[] =
{
    {"dz", "mode_dz", "Mode Name DZ", 6, 0, "mg_dz_blacksite", "dz_blacksite", "mode_dz.cfg", "cfg/maplist_dz.txt", "dz_sirocco"},
    {"comp", "mode_comp", "Mode Name Comp", 0, 1, "mg_active", "de_mirage", "mode_comp.cfg", "cfg/maplist_comp.txt", "de_dust2"},
    {"casual", "mode_casual", "Mode Name Casual", 0, 0, "mg_casualdelta", "de_mirage", "mode_casual.cfg", "cfg/maplist_casual.txt", "de_anubis"}
};

#define MAX_MODES (sizeof(g_Modes))

RuntimeSession g_Session;
PendingChange g_PendingChange;

bool g_VoteInProgress;
int g_VoteCounts[MAX_MODES];
bool g_HasVoted[MAXPLAYERS + 1];
Handle g_VoteTimer = null;
int g_NextVoteAllowedAt;

ConVar g_CvarVoteDuration = null;
ConVar g_CvarVoteCooldown = null;
ConVar g_CvarVoteMinPlayers = null;
ConVar g_CvarVerbose = null;

float g_VoteDuration = MODE_VOTE_DURATION_DEFAULT;
int g_VoteCooldown = MODE_VOTE_COOLDOWN_DEFAULT;
int g_VoteMinPlayers = MODE_VOTE_MIN_PLAYERS_DEFAULT;

ArrayList g_ModeMapCache[MAX_MODES];
bool g_ModeMapCacheLoaded[MAX_MODES];
char g_ModeMapCacheError[MAX_MODES][192];

int g_SelectedDzTeamCount[MAXPLAYERS + 1];
bool g_SelectedDzAutoAssign[MAXPLAYERS + 1];
int g_SelectedModeIndex[MAXPLAYERS + 1];
