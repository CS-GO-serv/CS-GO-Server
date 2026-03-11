#include <sourcemod>

#pragma semicolon 1
#pragma newdecls required

#define MAX_MODES 3
#define MODE_ACTION_LOG "addons/sourcemod/logs/mode_actions.log"
#define MODE_ROUTER_CFG "mode_router.cfg"
#define SAFE_HUB_MAP "de_mirage"

#define MODE_VOTE_DURATION_DEFAULT 20.0
#define MODE_VOTE_DURATION_MIN 5.0
#define MODE_VOTE_DURATION_MAX 120.0

#define MODE_VOTE_COOLDOWN_DEFAULT 120
#define MODE_VOTE_COOLDOWN_MIN 0
#define MODE_VOTE_COOLDOWN_MAX 1800

#define MODE_VOTE_MIN_PLAYERS_DEFAULT 4
#define MODE_VOTE_MIN_PLAYERS_MIN 1
#define MODE_VOTE_MIN_PLAYERS_MAX 64

enum struct ModeInfo
{
    char id[16];
    char namePhrase[32];
    int gameType;
    int gameMode;
    char mapgroup[32];
    char startMap[64];
    char cfgFile[64];
    char mapListFile[96];
    char fallbackMap[64];
}

static const ModeInfo g_Modes[MAX_MODES] =
{
    {"dz", "Mode Name DZ", 6, 0, "mg_dz_blacksite", "dz_blacksite", "mode_dz.cfg", "cfg/maplist_dz.txt", "dz_sirocco"},
    {"comp", "Mode Name Comp", 0, 1, "mg_active", "de_mirage", "mode_comp.cfg", "cfg/maplist_comp.txt", "de_dust2"},
    {"casual", "Mode Name Casual", 0, 0, "mg_casualdelta", "de_mirage", "mode_casual.cfg", "cfg/maplist_casual.txt", "de_anubis"}
};

bool g_VoteInProgress;
int g_VoteCounts[MAX_MODES];
bool g_HasVoted[MAXPLAYERS + 1];
Handle g_VoteTimer = null;
int g_NextVoteAllowedAt;

ConVar g_CvarVoteDuration = null;
ConVar g_CvarVoteCooldown = null;
ConVar g_CvarVoteMinPlayers = null;

float g_VoteDuration = MODE_VOTE_DURATION_DEFAULT;
int g_VoteCooldown = MODE_VOTE_COOLDOWN_DEFAULT;
int g_VoteMinPlayers = MODE_VOTE_MIN_PLAYERS_DEFAULT;

ArrayList g_ModeMapCache[MAX_MODES];
bool g_ModeMapCacheLoaded[MAX_MODES];
char g_ModeMapCacheError[MAX_MODES][192];

int g_SelectedDzTeamCount[MAXPLAYERS + 1];
bool g_SelectedDzAutoAssign[MAXPLAYERS + 1];
bool g_DzAutoShuffleEnabled = true;
bool g_DzShuffledThisMap;
int g_SelectedModeIndex[MAXPLAYERS + 1];

public Plugin myinfo =
{
    name = "Mode Vote",
    author = "Codex",
    description = "Mode menu and mode voting with cooldown",
    version = "1.3.0"
};

public void OnPluginStart()
{
    LoadTranslations("mode_vote.phrases");

    g_CvarVoteDuration = CreateConVar("sm_mode_vote_duration", "20.0", "Mode vote duration in seconds.", FCVAR_NOTIFY, true, MODE_VOTE_DURATION_MIN, true, MODE_VOTE_DURATION_MAX);
    g_CvarVoteCooldown = CreateConVar("sm_mode_vote_cooldown", "120", "Cooldown between mode votes in seconds.", FCVAR_NOTIFY, true, float(MODE_VOTE_COOLDOWN_MIN), true, float(MODE_VOTE_COOLDOWN_MAX));
    g_CvarVoteMinPlayers = CreateConVar("sm_mode_vote_min_players", "4", "Minimum human players required to start a mode vote.", FCVAR_NOTIFY, true, float(MODE_VOTE_MIN_PLAYERS_MIN), true, float(MODE_VOTE_MIN_PLAYERS_MAX));
    AutoExecConfig(true, "mode_vote", "sourcemod");

    RefreshVoteSettings("OnPluginStart");

    ValidateModeProfilesOrFail();
    EnsureModeRouterLoaded();

    RegConsoleCmd("sm_mode", Command_ModeMenu);
    RegConsoleCmd("sm_dz", Command_DzAlias);
    RegConsoleCmd("sm_comp", Command_CompAlias);

    RegConsoleCmd("sm_votemode", Command_VoteMode);
    RegAdminCmd("sm_forcemode", Command_ForceMode, ADMFLAG_CHANGEMAP);
    RegAdminCmd("sm_dzsize", Command_DzSize, ADMFLAG_CHANGEMAP);
    RegAdminCmd("sm_dzteams", Command_DzTeams, ADMFLAG_CHANGEMAP);
    RegAdminCmd("sm_mode_reloadlists", Command_ReloadModeLists, ADMFLAG_CHANGEMAP);

    HookEvent("round_start", Event_RoundStart, EventHookMode_PostNoCopy);

    ReloadModeMapCaches("OnPluginStart", 0);

    for (int i = 1; i <= MaxClients; i++)
    {
        g_SelectedModeIndex[i] = -1;
    }
}

public void OnMapStart()
{
    g_DzShuffledThisMap = false;
    RefreshVoteSettings("OnMapStart");
    ReloadModeMapCaches("OnMapStart", 0);
}

public Action Command_ModeMenu(int client, int args)
{
    if (!IsValidClient(client))
    {
        return Plugin_Handled;
    }

    PrintToChat(client, "%t", "Hint Mode Main Flow");
    ShowModeMenu(client);
    return Plugin_Handled;
}

public Action Command_DzAlias(int client, int args)
{
    if (!IsValidClient(client))
    {
        return Plugin_Handled;
    }

    g_SelectedDzTeamCount[client] = 2;
    g_SelectedDzAutoAssign[client] = true;
    ShowDzTeamSizeMenu(client);
    return Plugin_Handled;
}

public Action Command_CompAlias(int client, int args)
{
    if (!IsValidClient(client))
    {
        return Plugin_Handled;
    }

    int modeIndex = FindModeById("comp");
    if (modeIndex != -1)
    {
        char modeName[64];
        Format(modeName, sizeof(modeName), "%T", g_Modes[modeIndex].namePhrase, client);
        PrintToChat(client, "%t", "Mode Details", modeName, g_Modes[modeIndex].gameType, g_Modes[modeIndex].gameMode, g_Modes[modeIndex].mapgroup, g_Modes[modeIndex].startMap, g_Modes[modeIndex].cfgFile);
    }

    return Plugin_Handled;
}

public Action Command_VoteMode(int client, int args)
{
    if (!IsValidClient(client))
    {
        return Plugin_Handled;
    }

    TryStartVote(client);
    return Plugin_Handled;
}

public Action Command_ForceMode(int client, int args)
{
    if (args < 1)
    {
        ReplyToCommand(client, "%t", "Usage ForceMode");
        return Plugin_Handled;
    }

    char modeId[16];
    GetCmdArg(1, modeId, sizeof(modeId));

    int modeIndex = FindModeById(modeId);
    if (modeIndex == -1)
    {
        ReplyToCommand(client, "%t", "Unknown Mode", modeId);
        return Plugin_Handled;
    }

    char applyError[192];
    if (!ApplyMode(modeIndex, client, "sm_forcemode", applyError, sizeof(applyError)))
    {
        ReplyToCommand(client, "%t", "Error Mode Apply Failed", applyError);
        return Plugin_Handled;
    }

    char modeName[64];
    Format(modeName, sizeof(modeName), "%T", g_Modes[modeIndex].namePhrase, client);
    ReplyToCommand(client, "%t", "Force Mode Success", modeName);

    return Plugin_Handled;
}

public Action Command_DzSize(int client, int args)
{
    if (args < 1)
    {
        ReplyToCommand(client, "%t", "Cmd Usage DzSize");
        return Plugin_Handled;
    }

    char dzSize[16];
    GetCmdArg(1, dzSize, sizeof(dzSize));

    int teamCount = 2;
    if (StrEqual(dzSize, "solo", false) || StrEqual(dzSize, "1", false))
    {
        teamCount = 1;
    }
    else if (StrEqual(dzSize, "trio", false) || StrEqual(dzSize, "3", false))
    {
        teamCount = 3;
    }
    else if (!StrEqual(dzSize, "duo", false) && !StrEqual(dzSize, "2", false))
    {
        ReplyToCommand(client, "%t", "Error Unknown DzSize", dzSize);
        return Plugin_Handled;
    }

    char teamCfg[64];
    GetDzTeamCountCfg(teamCount, teamCfg, sizeof(teamCfg));
    RunModeRouterAlias(teamCfg);

    LogModeAction(client, "sm_dzsize", "dz team size set to %d", teamCount);
    ReplyToCommand(client, "%t", "Success DzSize Set", teamCount);
    return Plugin_Handled;
}

public Action Command_DzTeams(int client, int args)
{
    if (args < 1)
    {
        ReplyToCommand(client, "%t", "Cmd Usage DzTeams");
        return Plugin_Handled;
    }

    char teamMode[16];
    GetCmdArg(1, teamMode, sizeof(teamMode));

    if (StrEqual(teamMode, "auto", false))
    {
        SetDzTeamAssignMode(true);
        LogModeAction(client, "sm_dzteams", "dz teams set to auto");
        ReplyToCommand(client, "%t", "Success DzTeams Auto");
        return Plugin_Handled;
    }

    if (StrEqual(teamMode, "open", false) || StrEqual(teamMode, "manual", false))
    {
        SetDzTeamAssignMode(false);
        LogModeAction(client, "sm_dzteams", "dz teams set to manual/open");
        ReplyToCommand(client, "%t", "Success DzTeams Manual");
        return Plugin_Handled;
    }

    ReplyToCommand(client, "%t", "Error Unknown DzTeams", teamMode);
    return Plugin_Handled;
}

public Action Command_ReloadModeLists(int client, int args)
{
    bool allLoaded = ReloadModeMapCaches("sm_mode_reloadlists", client);

    if (allLoaded)
    {
        ReplyToCommand(client, "[mode_vote] map lists cache reloaded successfully for all modes.");
    }
    else
    {
        ReplyToCommand(client, "[mode_vote] map lists cache reloaded with errors. See logs for failed modes.");
    }

    return Plugin_Handled;
}

void ShowModeMenu(int client)
{
    Menu menu = new Menu(ModeMenuHandler);
    menu.SetTitle("%T", "Mode Menu Title", client);

    for (int i = 0; i < MAX_MODES; i++)
    {
        char modeName[64];
        Format(modeName, sizeof(modeName), "%T", g_Modes[i].namePhrase, client);
        menu.AddItem(g_Modes[i].id, modeName);
    }

    menu.ExitButton = true;
    menu.Display(client, 20);
}

public int ModeMenuHandler(Menu menu, MenuAction action, int client, int item)
{
    if (action == MenuAction_End)
    {
        delete menu;
    }
    else if (action == MenuAction_Select)
    {
        char modeId[16];
        menu.GetItem(item, modeId, sizeof(modeId));

        int modeIndex = FindModeById(modeId);
        if (modeIndex == -1)
        {
            return 0;
        }

        if (StrEqual(g_Modes[modeIndex].id, "dz", false))
        {
            g_SelectedModeIndex[client] = modeIndex;
            g_SelectedDzTeamCount[client] = 2;
            g_SelectedDzAutoAssign[client] = true;
            ShowDzTeamSizeMenu(client);
            return 0;
        }

        g_SelectedModeIndex[client] = modeIndex;
        ShowModeActionMenu(client, modeIndex);
    }

    return 0;
}

void ShowModeActionMenu(int client, int modeIndex)
{
    Menu menu = new Menu(ModeActionMenuHandler);
    menu.SetTitle("%T", "Menu Mode Action Title", client);

    char voteLabel[64];
    char adminLabel[64];
    char infoLabel[64];
    Format(voteLabel, sizeof(voteLabel), "%T", "Menu Mode Action Vote", client);
    Format(adminLabel, sizeof(adminLabel), "%T", "Menu Mode Action Ask Admin", client);
    Format(infoLabel, sizeof(infoLabel), "%T", "Menu Mode Action Info", client);

    menu.AddItem("vote", voteLabel);
    menu.AddItem("admin", adminLabel);
    menu.AddItem("info", infoLabel);
    menu.ExitButton = true;
    menu.Display(client, 20);
}

public int ModeActionMenuHandler(Menu menu, MenuAction action, int client, int item)
{
    if (action == MenuAction_End)
    {
        delete menu;
    }
    else if (action == MenuAction_Select)
    {
        if (!IsValidClient(client))
        {
            return 0;
        }

        int modeIndex = g_SelectedModeIndex[client];
        if (modeIndex < 0 || modeIndex >= MAX_MODES)
        {
            return 0;
        }

        char actionId[16];
        menu.GetItem(item, actionId, sizeof(actionId));

        if (StrEqual(actionId, "vote", false))
        {
            TryStartVote(client);
            return 0;
        }

        if (StrEqual(actionId, "admin", false))
        {
            RequestAdminModeApply(client, modeIndex, false);
            return 0;
        }

        char modeName[64];
        Format(modeName, sizeof(modeName), "%T", g_Modes[modeIndex].namePhrase, client);
        PrintToChat(client, "%t", "Mode Details", modeName, g_Modes[modeIndex].gameType, g_Modes[modeIndex].gameMode, g_Modes[modeIndex].mapgroup, g_Modes[modeIndex].startMap, g_Modes[modeIndex].cfgFile);
    }

    return 0;
}

void ShowDzTeamSizeMenu(int client)
{
    Menu menu = new Menu(DzTeamSizeMenuHandler);
    menu.SetTitle("%T", "Menu Dz Team Size Title", client);

    char soloLabel[32];
    char duoLabel[32];
    char trioLabel[32];
    Format(soloLabel, sizeof(soloLabel), "%T", "Menu Dz Team Size Solo", client);
    Format(duoLabel, sizeof(duoLabel), "%T", "Menu Dz Team Size Duo", client);
    Format(trioLabel, sizeof(trioLabel), "%T", "Menu Dz Team Size Trio", client);

    menu.AddItem("1", soloLabel);
    menu.AddItem("2", duoLabel);
    menu.AddItem("3", trioLabel);
    menu.ExitButton = true;
    menu.Display(client, 20);
}

public int DzTeamSizeMenuHandler(Menu menu, MenuAction action, int client, int item)
{
    if (action == MenuAction_End)
    {
        delete menu;
    }
    else if (action == MenuAction_Select)
    {
        if (!IsValidClient(client))
        {
            return 0;
        }

        char teamCountStr[4];
        menu.GetItem(item, teamCountStr, sizeof(teamCountStr));

        int teamCount = StringToInt(teamCountStr);
        if (teamCount < 1 || teamCount > 3)
        {
            teamCount = 2;
        }

        g_SelectedDzTeamCount[client] = teamCount;
        ShowDzTeamAssignMenu(client);
    }

    return 0;
}

void ShowDzTeamAssignMenu(int client)
{
    Menu menu = new Menu(DzTeamAssignMenuHandler);
    menu.SetTitle("%T", "Menu Dz Team Assign Title", client);

    char autoLabel[48];
    char manualLabel[48];
    Format(autoLabel, sizeof(autoLabel), "%T", "Menu Dz Team Assign Auto", client);
    Format(manualLabel, sizeof(manualLabel), "%T", "Menu Dz Team Assign Manual", client);

    menu.AddItem("auto", autoLabel);
    menu.AddItem("open", manualLabel);
    menu.ExitButton = true;
    menu.Display(client, 20);
}

public int DzTeamAssignMenuHandler(Menu menu, MenuAction action, int client, int item)
{
    if (action == MenuAction_End)
    {
        delete menu;
    }
    else if (action == MenuAction_Select)
    {
        if (!IsValidClient(client))
        {
            return 0;
        }

        char assignMode[16];
        menu.GetItem(item, assignMode, sizeof(assignMode));
        g_SelectedDzAutoAssign[client] = StrEqual(assignMode, "auto", false);

        int dzModeIndex = FindModeById("dz");
        if (dzModeIndex == -1)
        {
            return 0;
        }

        g_SelectedModeIndex[client] = dzModeIndex;
        ShowDzFinalActionMenu(client);
    }

    return 0;
}

void ShowDzFinalActionMenu(int client)
{
    Menu menu = new Menu(DzFinalActionMenuHandler);
    menu.SetTitle("%T", "Menu Dz Final Action Title", client);

    char applyLabel[64];
    char voteLabel[64];
    Format(applyLabel, sizeof(applyLabel), "%T", "Menu Dz Final Action Apply", client);
    Format(voteLabel, sizeof(voteLabel), "%T", "Menu Dz Final Action Vote", client);

    menu.AddItem("apply", applyLabel);
    menu.AddItem("vote", voteLabel);
    menu.ExitButton = true;
    menu.Display(client, 20);
}

public int DzFinalActionMenuHandler(Menu menu, MenuAction action, int client, int item)
{
    if (action == MenuAction_End)
    {
        delete menu;
    }
    else if (action == MenuAction_Select)
    {
        if (!IsValidClient(client))
        {
            return 0;
        }

        char actionId[16];
        menu.GetItem(item, actionId, sizeof(actionId));

        if (StrEqual(actionId, "apply", false))
        {
            RequestAdminModeApply(client, FindModeById("dz"), true);
            return 0;
        }

        TryStartVote(client);
    }

    return 0;
}

void RequestAdminModeApply(int client, int modeIndex, bool dzProfile)
{
    if (modeIndex < 0 || modeIndex >= MAX_MODES)
    {
        return;
    }

    char requester[MAX_NAME_LENGTH];
    GetClientName(client, requester, sizeof(requester));

    char modeName[64];
    Format(modeName, sizeof(modeName), "%T", g_Modes[modeIndex].namePhrase, client);

    int adminCount = 0;
    for (int i = 1; i <= MaxClients; i++)
    {
        if (!IsValidClient(i) || !CheckCommandAccess(i, "mode_vote_admin_notify", ADMFLAG_CHANGEMAP, true))
        {
            continue;
        }

        if (dzProfile)
        {
            char assignLabel[32];
            if (g_SelectedDzAutoAssign[client])
            {
                Format(assignLabel, sizeof(assignLabel), "%T", "Menu Dz Team Assign Auto", i);
            }
            else
            {
                Format(assignLabel, sizeof(assignLabel), "%T", "Menu Dz Team Assign Manual", i);
            }

            PrintToChat(i, "%t", "Mode Admin Request Dz To Admin", requester, g_SelectedDzTeamCount[client], assignLabel);
        }
        else
        {
            PrintToChat(i, "%t", "Mode Admin Request To Admin", requester, modeName, g_Modes[modeIndex].id);
        }

        adminCount++;
    }

    if (adminCount > 0)
    {
        PrintToChat(client, "%t", "Mode Admin Request Sent", modeName);
    }
    else
    {
        PrintToChat(client, "%t", "Mode Admin Request No Admin", modeName);
    }
}

void TryStartVote(int caller)
{
    int now = GetTime();
    int playersOnline = CountHumanPlayers();

    if (g_VoteInProgress)
    {
        PrintToChat(caller, "%t", "Vote Already Running");
        return;
    }

    if (now < g_NextVoteAllowedAt)
    {
        PrintToChat(caller, "%t", "Vote Cooldown", g_NextVoteAllowedAt - now);
        return;
    }

    if (playersOnline < g_VoteMinPlayers)
    {
        PrintToChat(caller, "%t", "Vote Not Enough Players", g_VoteMinPlayers, playersOnline);
        return;
    }

    StartVote(caller);
}

void StartVote(int caller)
{
    g_VoteInProgress = true;
    g_NextVoteAllowedAt = GetTime() + g_VoteCooldown;

    for (int i = 0; i < MAX_MODES; i++)
    {
        g_VoteCounts[i] = 0;
    }

    for (int i = 1; i <= MaxClients; i++)
    {
        g_HasVoted[i] = false;
        if (!IsValidClient(i))
        {
            continue;
        }

        ShowVoteMenu(i);
    }

    char callerName[MAX_NAME_LENGTH];
    GetClientName(caller, callerName, sizeof(callerName));
    PrintToChatAll("%t", "Vote Started By", callerName);

    LogModeAction(caller, "sm_votemode", "started mode vote");

    if (g_VoteTimer != null)
    {
        delete g_VoteTimer;
    }

    g_VoteTimer = CreateTimer(g_VoteDuration, Timer_FinishVote);
}

void ShowVoteMenu(int client)
{
    Menu menu = new Menu(VoteMenuHandler);
    menu.SetTitle("%T", "Vote Menu Title", client);

    for (int i = 0; i < MAX_MODES; i++)
    {
        char modeName[64];
        Format(modeName, sizeof(modeName), "%T", g_Modes[i].namePhrase, client);
        menu.AddItem(g_Modes[i].id, modeName);
    }

    menu.ExitButton = true;
    menu.Display(client, RoundToCeil(g_VoteDuration));
}

void RefreshVoteSettings(const char[] source)
{
    if (g_CvarVoteDuration == null || g_CvarVoteCooldown == null || g_CvarVoteMinPlayers == null)
    {
        LogError("[mode_vote] Failed to refresh vote settings in %s: missing ConVar handle.", source);
        return;
    }

    g_VoteDuration = ClampVoteDuration(g_CvarVoteDuration.FloatValue, source);
    g_VoteCooldown = ClampVoteCooldown(g_CvarVoteCooldown.IntValue, source);
    g_VoteMinPlayers = ClampVoteMinPlayers(g_CvarVoteMinPlayers.IntValue, source);
}

float ClampVoteDuration(float value, const char[] source)
{
    float clamped = value;
    if (value < MODE_VOTE_DURATION_MIN)
    {
        clamped = MODE_VOTE_DURATION_MIN;
    }
    else if (value > MODE_VOTE_DURATION_MAX)
    {
        clamped = MODE_VOTE_DURATION_MAX;
    }

    if (clamped != value)
    {
        LogMessage("[mode_vote] WARNING: sm_mode_vote_duration=%.2f out of range [%.2f..%.2f] in %s. Clamped to %.2f.", value, MODE_VOTE_DURATION_MIN, MODE_VOTE_DURATION_MAX, source, clamped);
    }

    return clamped;
}

int ClampVoteCooldown(int value, const char[] source)
{
    int clamped = value;
    if (value < MODE_VOTE_COOLDOWN_MIN)
    {
        clamped = MODE_VOTE_COOLDOWN_MIN;
    }
    else if (value > MODE_VOTE_COOLDOWN_MAX)
    {
        clamped = MODE_VOTE_COOLDOWN_MAX;
    }

    if (clamped != value)
    {
        LogMessage("[mode_vote] WARNING: sm_mode_vote_cooldown=%d out of range [%d..%d] in %s. Clamped to %d.", value, MODE_VOTE_COOLDOWN_MIN, MODE_VOTE_COOLDOWN_MAX, source, clamped);
    }

    return clamped;
}

int ClampVoteMinPlayers(int value, const char[] source)
{
    int clamped = value;
    if (value < MODE_VOTE_MIN_PLAYERS_MIN)
    {
        clamped = MODE_VOTE_MIN_PLAYERS_MIN;
    }
    else if (value > MODE_VOTE_MIN_PLAYERS_MAX)
    {
        clamped = MODE_VOTE_MIN_PLAYERS_MAX;
    }

    if (clamped != value)
    {
        LogMessage("[mode_vote] WARNING: sm_mode_vote_min_players=%d out of range [%d..%d] in %s. Clamped to %d.", value, MODE_VOTE_MIN_PLAYERS_MIN, MODE_VOTE_MIN_PLAYERS_MAX, source, clamped);
    }

    return clamped;
}

public int VoteMenuHandler(Menu menu, MenuAction action, int client, int item)
{
    if (action == MenuAction_End)
    {
        delete menu;
    }
    else if (action == MenuAction_Select)
    {
        if (!g_VoteInProgress || !IsValidClient(client) || g_HasVoted[client])
        {
            return 0;
        }

        char modeId[16];
        menu.GetItem(item, modeId, sizeof(modeId));

        int modeIndex = FindModeById(modeId);
        if (modeIndex == -1)
        {
            return 0;
        }

        g_HasVoted[client] = true;
        g_VoteCounts[modeIndex]++;

        char modeName[64];
        Format(modeName, sizeof(modeName), "%T", g_Modes[modeIndex].namePhrase, client);
        PrintToChat(client, "%t", "Vote Accepted", modeName);
    }

    return 0;
}

public Action Timer_FinishVote(Handle timer)
{
    g_VoteTimer = null;

    if (!g_VoteInProgress)
    {
        return Plugin_Stop;
    }

    int winners[MAX_MODES];
    int winnersCount = 0;
    int bestVotes = -1;

    for (int i = 0; i < MAX_MODES; i++)
    {
        if (g_VoteCounts[i] > bestVotes)
        {
            bestVotes = g_VoteCounts[i];
            winnersCount = 0;
            winners[winnersCount++] = i;
        }
        else if (g_VoteCounts[i] == bestVotes)
        {
            winners[winnersCount++] = i;
        }
    }

    int winnerIndex;
    if (winnersCount <= 0)
    {
        winnerIndex = 0;
    }
    else
    {
        winnerIndex = winners[GetRandomInt(0, winnersCount - 1)];
    }

    g_VoteInProgress = false;

    char modeName[64];
    Format(modeName, sizeof(modeName), "%T", g_Modes[winnerIndex].namePhrase, LANG_SERVER);
    PrintToChatAll("%t", "Vote Finished", modeName, g_VoteCounts[winnerIndex]);

    char applyError[192];
    if (!ApplyMode(winnerIndex, 0, "vote winner", applyError, sizeof(applyError)))
    {
        PrintToChatAll("%t", "Vote Apply Failed Fallback", applyError);
        ApplyModeFallback(0, "vote", applyError);
    }

    return Plugin_Stop;
}

bool ApplyMode(int modeIndex, int actorClient, const char[] source, char[] failureReason, int failureReasonMaxlen)
{
    if (modeIndex < 0 || modeIndex >= MAX_MODES)
    {
        strcopy(failureReason, failureReasonMaxlen, "invalid mode index");
        LogModeApplyError(actorClient, source, "unknown", failureReason);
        return false;
    }

    ConVar gameType = null;
    ConVar gameMode = null;
    ConVar mapCycle = null;
    if (!PreflightModeSwitchCvars(source, g_Modes[modeIndex].id, actorClient, gameType, gameMode, mapCycle))
    {
        strcopy(failureReason, failureReasonMaxlen, "required cvars are missing");
        LogModeApplyError(actorClient, source, g_Modes[modeIndex].id, failureReason);
        return false;
    }

    gameType.SetInt(g_Modes[modeIndex].gameType);
    gameMode.SetInt(g_Modes[modeIndex].gameMode);

    SetModeMapList(modeIndex, mapCycle);

    char nextMap[64];
    bool usingFallback = false;
    if (!ResolveModeMap(modeIndex, nextMap, sizeof(nextMap), usingFallback))
    {
        strcopy(failureReason, failureReasonMaxlen, "no valid start/fallback map");
        LogError("[mode_vote] Mode '%s' has no valid map/start fallback pair. Skipping mode switch.", g_Modes[modeIndex].id);
        LogModeApplyError(actorClient, source, g_Modes[modeIndex].id, failureReason);
        return false;
    }

    char modeCfgPath[PLATFORM_MAX_PATH];
    BuildPath(Path_Game, modeCfgPath, sizeof(modeCfgPath), "%s", g_Modes[modeIndex].cfgFile);
    if (!FileExists(modeCfgPath))
    {
        Format(failureReason, failureReasonMaxlen, "mode cfg missing: %s", g_Modes[modeIndex].cfgFile);
        LogError("[mode_vote] Mode '%s' cfg is missing: '%s' (resolved '%s'). Skipping mode switch.", g_Modes[modeIndex].id, g_Modes[modeIndex].cfgFile, modeCfgPath);
        LogModeApplyError(actorClient, source, g_Modes[modeIndex].id, failureReason);
        return false;
    }

    ServerCommand("mapgroup %s", g_Modes[modeIndex].mapgroup);
    RunModeRouterAlias(g_Modes[modeIndex].cfgFile);

    if (StrEqual(g_Modes[modeIndex].id, "dz", false))
    {
        RunModeRouterAlias("mode_dz_duo.cfg");
        SetDzTeamAssignMode(true);
    }

    if (usingFallback)
    {
        LogMessage("[mode_vote] Start map '%s' unavailable for mode '%s'; switching to fallback '%s'.", g_Modes[modeIndex].startMap, g_Modes[modeIndex].id, nextMap);
    }

    LogModeAction(actorClient, source, "mode=%s game_type=%d game_mode=%d map=%s", g_Modes[modeIndex].id, g_Modes[modeIndex].gameType, g_Modes[modeIndex].gameMode, nextMap);
    ServerCommand("changelevel %s", nextMap);
    strcopy(failureReason, failureReasonMaxlen, "ok");
    return true;
}

bool ApplyDzSelection(int teamCount, bool autoAssign, int actorClient, const char[] source, char[] failureReason, int failureReasonMaxlen)
{
    ConVar gameType = null;
    ConVar gameMode = null;
    ConVar mapCycle = null;
    if (!PreflightModeSwitchCvars(source, "dz", actorClient, gameType, gameMode, mapCycle))
    {
        strcopy(failureReason, failureReasonMaxlen, "required cvars are missing");
        LogModeApplyError(actorClient, source, "dz", failureReason);
        return false;
    }

    gameType.SetInt(6);
    gameMode.SetInt(0);

    ServerCommand("mapgroup mg_dz_blacksite");
    RunModeRouterAlias("mode_dz.cfg");

    char teamCfg[64];
    GetDzTeamCountCfg(teamCount, teamCfg, sizeof(teamCfg));
    RunModeRouterAlias(teamCfg);

    SetDzTeamAssignMode(autoAssign);

    int dzModeIndex = FindModeById("dz");
    if (dzModeIndex == -1)
    {
        LogModeAction(actorClient, source, "mode=dz team_count=%d auto=%d map=dz_blacksite", teamCount, autoAssign ? 1 : 0);
        ServerCommand("changelevel dz_blacksite");
        strcopy(failureReason, failureReasonMaxlen, "ok");
        return true;
    }

    SetModeMapList(dzModeIndex, mapCycle);

    char nextMap[64];
    bool usingFallback = false;
    if (!ResolveModeMap(dzModeIndex, nextMap, sizeof(nextMap), usingFallback))
    {
        strcopy(failureReason, failureReasonMaxlen, "no valid start/fallback map");
        LogError("[mode_vote] DZ selection failed: both start/fallback maps are invalid.");
        LogModeApplyError(actorClient, source, "dz", failureReason);
        return false;
    }

    if (usingFallback)
    {
        LogMessage("[mode_vote] DZ start map unavailable, fallback selected: %s.", nextMap);
    }

    LogModeAction(actorClient, source, "mode=dz team_count=%d auto=%d map=%s", teamCount, autoAssign ? 1 : 0, nextMap);
    ServerCommand("changelevel %s", nextMap);
    strcopy(failureReason, failureReasonMaxlen, "ok");
    return true;
}

void ApplyModeFallback(int actorClient, const char[] sourceTag, const char[] reason)
{
    LogModeAction(actorClient, "mode_apply_fallback", "source=%s reason=%s action=exec mode_lobby.cfg + changelevel %s", sourceTag, reason, SAFE_HUB_MAP);
    ServerCommand("exec mode_lobby.cfg");
    ServerCommand("changelevel %s", SAFE_HUB_MAP);
    PrintToChatAll("%t", "Mode Fallback Applied", SAFE_HUB_MAP);
}

void ResolveModeApplyOrigin(const char[] source, char[] origin, int maxlen)
{
    if (StrContains(source, "vote", false) != -1)
    {
        strcopy(origin, maxlen, "vote");
        return;
    }

    if (StrContains(source, "chat", false) != -1)
    {
        strcopy(origin, maxlen, "chat");
        return;
    }

    if (StrContains(source, "admin", false) != -1 || StrContains(source, "sm_", false) != -1)
    {
        strcopy(origin, maxlen, "admin");
        return;
    }

    strcopy(origin, maxlen, source);
}

void LogModeApplyError(int actorClient, const char[] source, const char[] modeId, const char[] reason)
{
    char origin[16];
    ResolveModeApplyOrigin(source, origin, sizeof(origin));
    LogModeAction(actorClient, "mode_apply_error", "origin=%s source=%s mode=%s reason=%s", origin, source, modeId, reason);
}

void SetModeMapList(int modeIndex, ConVar mapCycle)
{
    if (modeIndex < 0 || modeIndex >= MAX_MODES)
    {
        LogError("[mode_vote] SetModeMapList called with invalid modeIndex=%d.", modeIndex);
        return;
    }

    mapCycle.SetString(g_Modes[modeIndex].mapListFile);

    LogMessage("[mode_vote] active maplist for mode '%s': '%s' via mapcyclefile (shared pool for mapchooser, nominations, sm_map menu, sm_votemap menu, randomcycle).", g_Modes[modeIndex].id, g_Modes[modeIndex].mapListFile);
}

bool TryGetGameplayConVars(ConVar &gameType, ConVar &gameMode, const char[] source, const char[] modeId)
{
    gameType = FindConVar("game_type");
    if (gameType == null)
    {
        LogError("[mode_vote] Missing required ConVar 'game_type'. source='%s' mode='%s'.", source, modeId);
        return false;
    }

    gameMode = FindConVar("game_mode");
    if (gameMode == null)
    {
        LogError("[mode_vote] Missing required ConVar 'game_mode'. source='%s' mode='%s'.", source, modeId);
        return false;
    }

    return true;
}

bool PreflightModeSwitchCvars(const char[] source, const char[] modeId, int actorClient, ConVar &gameType, ConVar &gameMode, ConVar &mapCycle)
{
    if (!TryGetGameplayConVars(gameType, gameMode, source, modeId))
    {
        NotifyModeSwitchCancelled(actorClient, source, modeId);
        return false;
    }

    mapCycle = FindConVar("mapcyclefile");
    if (mapCycle == null)
    {
        LogError("[mode_vote] Missing required ConVar 'mapcyclefile'. source='%s' mode='%s'.", source, modeId);
        NotifyModeSwitchCancelled(actorClient, source, modeId);
        return false;
    }

    return true;
}

void NotifyModeSwitchCancelled(int actorClient, const char[] source, const char[] modeId)
{
    if (IsValidClient(actorClient))
    {
        PrintToChat(actorClient, "%t", "Error Mode Switch Cancelled", modeId);
        PrintToConsole(actorClient, "%t", "Hint Missing Convars Console", source, modeId);
    }
}

bool ResolveModeMap(int modeIndex, char[] mapName, int maxlen, bool &usingFallback)
{
    usingFallback = false;

    if (IsModeMapAllowed(modeIndex, g_Modes[modeIndex].startMap) && IsMapValid(g_Modes[modeIndex].startMap))
    {
        strcopy(mapName, maxlen, g_Modes[modeIndex].startMap);
        return true;
    }

    usingFallback = true;
    if (IsModeMapAllowed(modeIndex, g_Modes[modeIndex].fallbackMap) && IsMapValid(g_Modes[modeIndex].fallbackMap))
    {
        strcopy(mapName, maxlen, g_Modes[modeIndex].fallbackMap);
        return true;
    }

    return false;
}

bool IsModeMapAllowed(int modeIndex, const char[] mapName)
{
    if (modeIndex < 0 || modeIndex >= MAX_MODES)
    {
        LogError("[mode_vote] IsModeMapAllowed called with invalid modeIndex=%d for map '%s'.", modeIndex, mapName);
        return false;
    }

    if (!g_ModeMapCacheLoaded[modeIndex] || g_ModeMapCache[modeIndex] == null)
    {
        LogError("[mode_vote] map validation failed: cache unavailable for mode=%s map='%s' error='%s'", g_Modes[modeIndex].id, mapName, g_ModeMapCacheError[modeIndex]);
        return false;
    }

    bool allowed = IsMapInModeCache(modeIndex, mapName);
    if (!allowed)
    {
        LogMessage("[mode_vote] map validation failed: mode=%s map='%s' maplist='%s'", g_Modes[modeIndex].id, mapName, g_Modes[modeIndex].mapListFile);
    }

    return allowed;
}

bool IsMapInModeCache(int modeIndex, const char[] mapName)
{
    ArrayList cache = g_ModeMapCache[modeIndex];
    if (cache == null)
    {
        return false;
    }

    char cachedMap[64];
    int count = cache.Length;
    for (int i = 0; i < count; i++)
    {
        cache.GetString(i, cachedMap, sizeof(cachedMap));
        if (StrEqual(cachedMap, mapName, false))
        {
            return true;
        }
    }

    return false;
}


void FormatActorLabel(int actorClient, char[] actorLabel, int maxlen)
{
    if (actorClient > 0 && actorClient <= MaxClients)
    {
        if (!GetClientName(actorClient, actorLabel, maxlen))
        {
            strcopy(actorLabel, maxlen, "unknown");
        }
        return;
    }

    strcopy(actorLabel, maxlen, "server");
}

bool ReloadModeMapCaches(const char[] source, int actorClient)
{
    bool allLoaded = true;

    for (int modeIndex = 0; modeIndex < MAX_MODES; modeIndex++)
    {
        if (!LoadModeMapCache(modeIndex, source, actorClient))
        {
            allLoaded = false;
        }
    }

    return allLoaded;
}

bool LoadModeMapCache(int modeIndex, const char[] source, int actorClient)
{
    if (modeIndex < 0 || modeIndex >= MAX_MODES)
    {
        return false;
    }

    if (g_ModeMapCache[modeIndex] == null)
    {
        g_ModeMapCache[modeIndex] = new ArrayList(ByteCountToCells(64));
    }
    else
    {
        g_ModeMapCache[modeIndex].Clear();
    }

    g_ModeMapCacheLoaded[modeIndex] = false;
    g_ModeMapCacheError[modeIndex][0] = '\0';

    char mapListPath[PLATFORM_MAX_PATH];
    BuildPath(Path_Game, mapListPath, sizeof(mapListPath), "%s", g_Modes[modeIndex].mapListFile);

    if (!FileExists(mapListPath))
    {
        Format(g_ModeMapCacheError[modeIndex], sizeof(g_ModeMapCacheError[]), "file does not exist: '%s'", mapListPath);
        char actorLabel[64];
        FormatActorLabel(actorClient, actorLabel, sizeof(actorLabel));
        LogError("[mode_vote] map cache load failed: mode=%s source=%s actor=%s maplist='%s' resolved='%s' reason='%s'", g_Modes[modeIndex].id, source, actorLabel, g_Modes[modeIndex].mapListFile, mapListPath, g_ModeMapCacheError[modeIndex]);
        return false;
    }

    File file = OpenFile(mapListPath, "r");
    if (file == null)
    {
        Format(g_ModeMapCacheError[modeIndex], sizeof(g_ModeMapCacheError[]), "failed to open file: '%s'", mapListPath);
        char actorLabel[64];
        FormatActorLabel(actorClient, actorLabel, sizeof(actorLabel));
        LogError("[mode_vote] map cache load failed: mode=%s source=%s actor=%s maplist='%s' resolved='%s' reason='%s'", g_Modes[modeIndex].id, source, actorLabel, g_Modes[modeIndex].mapListFile, mapListPath, g_ModeMapCacheError[modeIndex]);
        return false;
    }

    char line[128];
    int loadedMaps = 0;
    while (!file.EndOfFile() && file.ReadLine(line, sizeof(line)))
    {
        TrimString(line);

        if (line[0] == '\0' || line[0] == ';' || (line[0] == '/' && line[1] == '/'))
        {
            continue;
        }

        if (!IsMapValid(line))
        {
            LogMessage("[mode_vote] map cache skip invalid entry: mode=%s map='%s' maplist='%s'", g_Modes[modeIndex].id, line, g_Modes[modeIndex].mapListFile);
            continue;
        }

        g_ModeMapCache[modeIndex].PushString(line);
        loadedMaps++;
    }

    delete file;

    if (loadedMaps <= 0)
    {
        Format(g_ModeMapCacheError[modeIndex], sizeof(g_ModeMapCacheError[]), "no valid maps loaded from '%s'", mapListPath);
        char actorLabel[64];
        FormatActorLabel(actorClient, actorLabel, sizeof(actorLabel));
        LogError("[mode_vote] map cache load failed: mode=%s source=%s actor=%s maplist='%s' resolved='%s' reason='%s'", g_Modes[modeIndex].id, source, actorLabel, g_Modes[modeIndex].mapListFile, mapListPath, g_ModeMapCacheError[modeIndex]);
        return false;
    }

    g_ModeMapCacheLoaded[modeIndex] = true;
    strcopy(g_ModeMapCacheError[modeIndex], sizeof(g_ModeMapCacheError[]), "ok");
    LogMessage("[mode_vote] map cache loaded: mode=%s source=%s maplist='%s' maps=%d", g_Modes[modeIndex].id, source, g_Modes[modeIndex].mapListFile, loadedMaps);
    return true;
}

void SetDzTeamAssignMode(bool autoAssign)
{
    if (autoAssign)
    {
        RunModeRouterAlias("mode_dz_teams_auto.cfg");
    }
    else
    {
        RunModeRouterAlias("mode_dz_teams_open.cfg");
    }

    g_DzAutoShuffleEnabled = autoAssign;
    g_DzShuffledThisMap = false;
}

void GetDzTeamCountCfg(int teamCount, char[] cfgFile, int maxlen)
{
    // Spectator policy guard: solo != squads.
    // Solo uses mode_dz_solo.cfg (mp_forcecamera 0), while duo/trio use forcecamera 1
    // in their own cfg files to reduce info leakage between alive squad members and spectators.
    switch (teamCount)
    {
        case 1:
        {
            strcopy(cfgFile, maxlen, "mode_dz_solo.cfg");
        }
        case 3:
        {
            strcopy(cfgFile, maxlen, "mode_dz_trio.cfg");
        }
        default:
        {
            strcopy(cfgFile, maxlen, "mode_dz_duo.cfg");
        }
    }
}

public void Event_RoundStart(Event event, const char[] name, bool dontBroadcast)
{
    if (!g_DzAutoShuffleEnabled || g_DzShuffledThisMap)
    {
        return;
    }

    ConVar gameType = FindConVar("game_type");
    if (gameType == null || gameType.IntValue != 6)
    {
        return;
    }

    ServerCommand("dz_shuffle_teams");
    g_DzShuffledThisMap = true;
}

int FindModeById(const char[] modeId)
{
    for (int i = 0; i < MAX_MODES; i++)
    {
        if (StrEqual(modeId, g_Modes[i].id, false))
        {
            return i;
        }
    }

    return -1;
}

void ValidateModeProfilesOrFail()
{
    static const char requiredProfiles[][] =
    {
        "mode_router.cfg",
        "mode_lobby.cfg",
        "mode_comp.cfg",
        "mode_casual.cfg",
        "mode_dz.cfg",
        "mode_dz_solo.cfg",
        "mode_dz_duo.cfg",
        "mode_dz_trio.cfg",
        "mode_dz_teams_auto.cfg",
        "mode_dz_teams_open.cfg"
    };

    bool missingProfile = false;

    for (int i = 0; i < sizeof(requiredProfiles); i++)
    {
        char profilePath[PLATFORM_MAX_PATH];
        BuildPath(Path_Game, profilePath, sizeof(profilePath), "%s", requiredProfiles[i]);

        if (!FileExists(profilePath))
        {
            missingProfile = true;
            LogError("[mode_vote] Missing mode profile '%s' (resolved '%s').", requiredProfiles[i], profilePath);
        }
    }

    if (missingProfile)
    {
        SetFailState("[mode_vote] Required mode profiles are missing. Check error logs.");
    }
}

void EnsureModeRouterLoaded()
{
    ServerCommand("exec %s", MODE_ROUTER_CFG);
}

void RunModeRouterAlias(const char[] cfgFile)
{
    if (StrEqual(cfgFile, "mode_lobby.cfg", false))
    {
        ServerCommand("exec %s; mode_lobby", MODE_ROUTER_CFG);
        return;
    }

    if (StrEqual(cfgFile, "mode_comp.cfg", false))
    {
        ServerCommand("exec %s; mode_comp", MODE_ROUTER_CFG);
        return;
    }

    if (StrEqual(cfgFile, "mode_casual.cfg", false))
    {
        ServerCommand("exec %s; mode_casual", MODE_ROUTER_CFG);
        return;
    }

    if (StrEqual(cfgFile, "mode_dz.cfg", false))
    {
        ServerCommand("exec %s; mode_dz", MODE_ROUTER_CFG);
        return;
    }

    if (StrEqual(cfgFile, "mode_dz_solo.cfg", false))
    {
        ServerCommand("exec %s; mode_dz_solo", MODE_ROUTER_CFG);
        return;
    }

    if (StrEqual(cfgFile, "mode_dz_duo.cfg", false))
    {
        ServerCommand("exec %s; mode_dz_duo", MODE_ROUTER_CFG);
        return;
    }

    if (StrEqual(cfgFile, "mode_dz_trio.cfg", false))
    {
        ServerCommand("exec %s; mode_dz_trio", MODE_ROUTER_CFG);
        return;
    }

    if (StrEqual(cfgFile, "mode_dz_teams_auto.cfg", false))
    {
        ServerCommand("exec %s; mode_dz_teams_auto", MODE_ROUTER_CFG);
        return;
    }

    if (StrEqual(cfgFile, "mode_dz_teams_open.cfg", false))
    {
        ServerCommand("exec %s; mode_dz_teams_open", MODE_ROUTER_CFG);
        return;
    }

    LogError("[mode_vote] Unknown mode profile '%s': no router alias configured.", cfgFile);
}

void LogModeAction(int client, const char[] action, const char[] fmt, any ...)
{
    char detail[256];
    VFormat(detail, sizeof(detail), fmt, 4);

    char timestamp[32];
    FormatTime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", GetTime());

    char actorName[MAX_NAME_LENGTH];
    char actorAuth[64];

    if (client > 0 && client <= MaxClients)
    {
        if (!GetClientName(client, actorName, sizeof(actorName)))
        {
            strcopy(actorName, sizeof(actorName), "unknown");
        }

        if (!GetClientAuthId(client, AuthId_Steam2, actorAuth, sizeof(actorAuth), true))
        {
            strcopy(actorAuth, sizeof(actorAuth), "unknown");
        }
    }
    else
    {
        strcopy(actorName, sizeof(actorName), "server");
        strcopy(actorAuth, sizeof(actorAuth), "N/A");
    }

    LogToFileEx(MODE_ACTION_LOG, "[%s] actor=%s (%s) action=%s %s", timestamp, actorName, actorAuth, action, detail);
}

bool IsValidClient(int client)
{
    return (client > 0 && client <= MaxClients && IsClientInGame(client) && !IsFakeClient(client));
}

int CountHumanPlayers()
{
    int count = 0;

    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsValidClient(i))
        {
            count++;
        }
    }

    return count;
}
