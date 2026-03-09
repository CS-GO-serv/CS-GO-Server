#include <sourcemod>

#pragma semicolon 1
#pragma newdecls required

#define MODE_VOTE_DURATION 20.0
#define MODE_VOTE_COOLDOWN 120
#define MAX_MODES 4

enum struct ModeInfo
{
    char id[16];
    char namePhrase[32];
    int gameType;
    int gameMode;
    char mapgroup[32];
    char startMap[64];
    char cfgFile[64];
}

static const ModeInfo g_Modes[MAX_MODES] =
{
    {"dz", "Mode Name DZ", 6, 0, "mg_dz", "dz_blacksite", "mode_dz.cfg"},
    {"comp", "Mode Name Comp", 0, 1, "mg_active", "de_mirage", "mode_comp.cfg"},
    {"casual", "Mode Name Casual", 0, 0, "mg_casual", "de_dust2", "mode_casual.cfg"},
    {"dm", "Mode Name DM", 1, 2, "mg_deathmatch", "de_dust2", "mode_dm.cfg"}
};

bool g_VoteInProgress;
int g_VoteCounts[MAX_MODES];
bool g_HasVoted[MAXPLAYERS + 1];
Handle g_VoteTimer = null;
int g_NextVoteAllowedAt;

int g_SelectedDzTeamCount[MAXPLAYERS + 1];
bool g_SelectedDzAutoAssign[MAXPLAYERS + 1];
bool g_DzAutoShuffleEnabled = true;
bool g_DzShuffledThisMap;

public Plugin myinfo =
{
    name = "Mode Vote",
    author = "Codex",
    description = "Mode menu and mode voting with cooldown",
    version = "1.0.0"
};

public void OnPluginStart()
{
    LoadTranslations("mode_vote.phrases");

    RegConsoleCmd("sm_mode", Command_ModeMenu);
    RegConsoleCmd("sm_votemode", Command_VoteMode);
    RegConsoleCmd("sm_dzteams", Command_DzTeams);
    RegAdminCmd("sm_forcemode", Command_ForceMode, ADMFLAG_CHANGEMAP);

    HookEvent("round_start", Event_RoundStart, EventHookMode_PostNoCopy);
}

public void OnMapStart()
{
    g_DzShuffledThisMap = false;
}

public Action Command_ModeMenu(int client, int args)
{
    if (!IsValidClient(client))
    {
        return Plugin_Handled;
    }

    ShowModeMenu(client);
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

    ApplyMode(modeIndex);

    char modeName[64];
    Format(modeName, sizeof(modeName), "%T", g_Modes[modeIndex].namePhrase, client);
    ReplyToCommand(client, "%t", "Force Mode Success", modeName);

    return Plugin_Handled;
}

public Action Command_DzTeams(int client, int args)
{
    if (args < 1)
    {
        ReplyToCommand(client, "Usage: sm_dzteams <open|auto>");
        return Plugin_Handled;
    }

    char teamMode[16];
    GetCmdArg(1, teamMode, sizeof(teamMode));

    if (StrEqual(teamMode, "auto", false))
    {
        SetDzTeamAssignMode(true);
        ReplyToCommand(client, "Danger Zone team assignment set to auto.");
        return Plugin_Handled;
    }

    if (StrEqual(teamMode, "open", false))
    {
        SetDzTeamAssignMode(false);
        ReplyToCommand(client, "Danger Zone team assignment set to manual (open).");
        return Plugin_Handled;
    }

    ReplyToCommand(client, "Unknown dzteams mode: %s. Use open|auto.", teamMode);
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

        char modeName[64];
        Format(modeName, sizeof(modeName), "%T", g_Modes[modeIndex].namePhrase, client);

        if (StrEqual(g_Modes[modeIndex].id, "dz", false))
        {
            g_SelectedDzTeamCount[client] = 2;
            g_SelectedDzAutoAssign[client] = true;
            ShowDzTeamSizeMenu(client);
            return 0;
        }

        PrintToChat(client, "%t", "Mode Details", modeName, g_Modes[modeIndex].gameType, g_Modes[modeIndex].gameMode, g_Modes[modeIndex].mapgroup, g_Modes[modeIndex].startMap, g_Modes[modeIndex].cfgFile);
    }

    return 0;
}

void ShowDzTeamSizeMenu(int client)
{
    Menu menu = new Menu(DzTeamSizeMenuHandler);
    menu.SetTitle("Danger Zone: Solo / Duo / Trio");
    menu.AddItem("1", "Solo");
    menu.AddItem("2", "Duo");
    menu.AddItem("3", "Trio");
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
    menu.SetTitle("Danger Zone: Авто-распределение / Ручной выбор");
    menu.AddItem("auto", "Авто-распределение");
    menu.AddItem("open", "Ручной выбор");
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

        char cfgFile[64];
        GetDzTeamCountCfg(g_SelectedDzTeamCount[client], cfgFile, sizeof(cfgFile));

        char assignLabel[32];
        if (g_SelectedDzAutoAssign[client])
        {
            strcopy(assignLabel, sizeof(assignLabel), "auto");
        }
        else
        {
            strcopy(assignLabel, sizeof(assignLabel), "open");
        }

        char modeName[64];
        Format(modeName, sizeof(modeName), "%T", g_Modes[dzModeIndex].namePhrase, client);
        PrintToChat(client, "%t", "Mode Details", modeName, g_Modes[dzModeIndex].gameType, g_Modes[dzModeIndex].gameMode, g_Modes[dzModeIndex].mapgroup, g_Modes[dzModeIndex].startMap, cfgFile);

        ApplyDzSelection(g_SelectedDzTeamCount[client], g_SelectedDzAutoAssign[client]);
        PrintToChat(client, "DZ profile applied: team_count=%d, teams=%s", g_SelectedDzTeamCount[client], assignLabel);
    }

    return 0;
}

void TryStartVote(int caller)
{
    int now = GetTime();

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

    StartVote(caller);
}

void StartVote(int caller)
{
    g_VoteInProgress = true;
    g_NextVoteAllowedAt = GetTime() + MODE_VOTE_COOLDOWN;

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

    if (g_VoteTimer != null)
    {
        delete g_VoteTimer;
    }

    g_VoteTimer = CreateTimer(MODE_VOTE_DURATION, Timer_FinishVote);
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
    menu.Display(client, RoundToCeil(MODE_VOTE_DURATION));
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

    ApplyMode(winnerIndex);

    return Plugin_Stop;
}

void ApplyMode(int modeIndex)
{
    if (modeIndex < 0 || modeIndex >= MAX_MODES)
    {
        return;
    }

    SetConVarInt(FindConVar("game_type"), g_Modes[modeIndex].gameType);
    SetConVarInt(FindConVar("game_mode"), g_Modes[modeIndex].gameMode);

    ServerCommand("mapgroup %s", g_Modes[modeIndex].mapgroup);
    ServerCommand("exec %s", g_Modes[modeIndex].cfgFile);

    if (StrEqual(g_Modes[modeIndex].id, "dz", false))
    {
        ServerCommand("exec mode_dz_duo.cfg");
        SetDzTeamAssignMode(true);
    }

    ServerCommand("changelevel %s", g_Modes[modeIndex].startMap);
}

void ApplyDzSelection(int teamCount, bool autoAssign)
{
    SetConVarInt(FindConVar("game_type"), 6);
    SetConVarInt(FindConVar("game_mode"), 0);

    ServerCommand("mapgroup mg_dz");
    ServerCommand("exec mode_dz.cfg");

    char teamCfg[64];
    GetDzTeamCountCfg(teamCount, teamCfg, sizeof(teamCfg));
    ServerCommand("exec %s", teamCfg);

    SetDzTeamAssignMode(autoAssign);
    ServerCommand("changelevel dz_blacksite");
}

void SetDzTeamAssignMode(bool autoAssign)
{
    if (autoAssign)
    {
        ServerCommand("exec mode_dz_teams_auto.cfg");
    }
    else
    {
        ServerCommand("exec mode_dz_teams_open.cfg");
    }

    g_DzAutoShuffleEnabled = autoAssign;
    g_DzShuffledThisMap = false;
}

void GetDzTeamCountCfg(int teamCount, char[] cfgFile, int maxlen)
{
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

bool IsValidClient(int client)
{
    return (client > 0 && client <= MaxClients && IsClientInGame(client) && !IsFakeClient(client));
}
