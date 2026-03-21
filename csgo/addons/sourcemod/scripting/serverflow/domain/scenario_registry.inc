#if defined _SERVERFLOW_SCENARIO_REGISTRY_INC_
 #endinput
#endif
#define _SERVERFLOW_SCENARIO_REGISTRY_INC_

bool Scenario_Register(const ModeInfo scenario)
{
    return Scenarios_Register(scenario);
}

bool Scenario_Exists(const char[] id)
{
    return (Scenarios_FindById(id) != -1);
}

int Scenario_GetIndexById(const char[] id)
{
    return Scenarios_FindById(id);
}

bool Scenario_Get(int scenarioIndex, ModeInfo scenario)
{
    if (!Scenario_IsValidIndex(scenarioIndex))
    {
        return false;
    }

    scenario = g_Scenarios[scenarioIndex];
    return true;
}

int Scenario_GetAllVisible(int client, int indexes[MAX_RUNTIME_MODES])
{
    int count = 0;
    for (int i = 0; i < g_ScenarioCount; i++)
    {
        indexes[count++] = i;
    }

    return count;
}

int Scenario_GetCount()
{
    return g_ScenarioCount;
}

bool Scenario_IsValidIndex(int scenarioIndex)
{
    return (scenarioIndex >= 0 && scenarioIndex < g_ScenarioCount);
}

void Scenario_GetNameByIndex(int scenarioIndex, int client, char[] buffer, int maxlen)
{
    if (!Scenario_IsValidIndex(scenarioIndex))
    {
        buffer[0] = '\0';
        return;
    }

    Format(buffer, maxlen, "%T", g_Scenarios[scenarioIndex].namePhrase, client);
}

bool Scenario_GetIdByIndex(int scenarioIndex, char[] buffer, int maxlen)
{
    if (!Scenario_IsValidIndex(scenarioIndex))
    {
        buffer[0] = '\0';
        return false;
    }

    strcopy(buffer, maxlen, g_Scenarios[scenarioIndex].id);
    return true;
}

bool Scenario_IsPlayableForClients(int scenarioIndex, int players)
{
    return Scenario_IsValidIndex(scenarioIndex);
}

bool Scenario_IsAdminOnly(int scenarioIndex)
{
    return false;
}

bool Scenario_CanPlayerSelect(int client, int scenarioIndex, char[] reason, int reasonLen)
{
    return State_CanClientSelectScenario(client, scenarioIndex, reason, reasonLen);
}
