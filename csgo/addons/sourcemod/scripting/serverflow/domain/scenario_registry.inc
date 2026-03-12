#if defined _SERVERFLOW_SCENARIO_REGISTRY_INC_
 #endinput
#endif
#define _SERVERFLOW_SCENARIO_REGISTRY_INC_

bool Scenario_Exists(const char[] id)
{
    return (FindModeById(id) != -1);
}

void Scenario_GetNameByIndex(int modeIndex, int client, char[] buffer, int maxlen)
{
    Format(buffer, maxlen, "%T", g_Modes[modeIndex].namePhrase, client);
}
