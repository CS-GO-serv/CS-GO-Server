#if defined _SERVERFLOW_SCENARIO_REGISTRY_INC_
 #endinput
#endif
#define _SERVERFLOW_SCENARIO_REGISTRY_INC_

bool Scenario_Exists(const char[] id)
{
    return (FindModeById(id) != -1);
}

int Scenario_GetCount()
{
    return MAX_MODES;
}

bool Scenario_IsValidIndex(int modeIndex)
{
    return (modeIndex >= 0 && modeIndex < MAX_MODES);
}

void Scenario_GetNameByIndex(int modeIndex, int client, char[] buffer, int maxlen)
{
    if (!Scenario_IsValidIndex(modeIndex))
    {
        buffer[0] = '\0';
        return;
    }

    Format(buffer, maxlen, "%T", g_Modes[modeIndex].namePhrase, client);
}

bool Scenario_GetIdByIndex(int modeIndex, char[] buffer, int maxlen)
{
    if (!Scenario_IsValidIndex(modeIndex))
    {
        buffer[0] = '\0';
        return false;
    }

    strcopy(buffer, maxlen, g_Modes[modeIndex].id);
    return true;
}

bool Scenario_CanPlayerSelect(int client, int modeIndex, char[] reason, int reasonLen)
{
    if (!IsValidClient(client))
    {
        strcopy(reason, reasonLen, "invalid_client");
        return false;
    }

    if (!Scenario_IsValidIndex(modeIndex))
    {
        strcopy(reason, reasonLen, "invalid_scenario");
        return false;
    }

    if (!State_IsScenarioSelectionAllowed(Session_GetState()))
    {
        strcopy(reason, reasonLen, "blocked_by_state");
        return false;
    }

    reason[0] = '\0';
    return true;
}
