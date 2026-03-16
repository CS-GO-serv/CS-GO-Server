#if defined _SERVERFLOW_HELPERS_INC_
 #endinput
#endif
#define _SERVERFLOW_HELPERS_INC_

bool IsValidClient(int client)
{
    return (client > 0 && client <= MaxClients && IsClientInGame(client) && !IsFakeClient(client));
}

int CountHumanPlayers()
{
    int total;
    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsValidClient(i))
        {
            total++;
        }
    }

    return total;
}

int FindModeById(const char[] modeId)
{
    for (int i = 0; i < g_ScenarioCount; i++)
    {
        if (StrEqual(g_Scenarios[i].id, modeId, false))
        {
            return i;
        }
    }

    return -1;
}

void ServerStateToString(ServerState state, char[] buffer, int maxlen)
{
    switch (state)
    {
        case ServerState_Lobby: strcopy(buffer, maxlen, "Lobby");
        case ServerState_PreMatch: strcopy(buffer, maxlen, "PreMatch");
        case ServerState_Match: strcopy(buffer, maxlen, "Match");
        case ServerState_PostMatch: strcopy(buffer, maxlen, "PostMatch");
        case ServerState_Transition: strcopy(buffer, maxlen, "Transition");
        default: Format(buffer, maxlen, "Unknown(%d)", state);
    }
}

void PreMatchPhaseToString(PreMatchPhase phase, char[] buffer, int maxlen)
{
    switch (phase)
    {
        case PreMatch_Setup: strcopy(buffer, maxlen, "Setup");
        case PreMatch_Teams: strcopy(buffer, maxlen, "Teams");
        case PreMatch_Ready: strcopy(buffer, maxlen, "Ready");
        case PreMatch_Lock: strcopy(buffer, maxlen, "Lock");
        default: Format(buffer, maxlen, "Unknown(%d)", phase);
    }
}

void PendingChangeStateToString(PendingChangeState state, char[] buffer, int maxlen)
{
    switch (state)
    {
        case Pending_None: strcopy(buffer, maxlen, "none");
        case Pending_Proposed: strcopy(buffer, maxlen, "proposed");
        case Pending_Selected: strcopy(buffer, maxlen, "selected");
        case Pending_Confirmed: strcopy(buffer, maxlen, "confirmed");
        case Pending_Applying: strcopy(buffer, maxlen, "applying");
        default: Format(buffer, maxlen, "unknown(%d)", state);
    }
}
