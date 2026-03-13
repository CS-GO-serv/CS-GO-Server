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
