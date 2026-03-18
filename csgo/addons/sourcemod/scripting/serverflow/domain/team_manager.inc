#if defined _SERVERFLOW_TEAM_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_TEAM_MANAGER_INC_

void TeamManager_EnterPreMatchSetup(const char[] source = "prematch_setup")
{
    ModeInfo scenarioInfo;
    char currentScenario[16];
    Session_GetCurrentScenario(currentScenario, sizeof(currentScenario));
    
    if (Scenario_Get(FindModeById(currentScenario), scenarioInfo))
    {
        if (scenarioInfo.teamPolicy == TeamPolicy_None || scenarioInfo.teamPolicy == TeamPolicy_Auto)
        {
            // Auto bypass team setup phase if not manual
            Session_SetPreMatchPhase(PreMatch_Ready);
            return;
        }
    }

    Session_SetPreMatchPhase(PreMatch_Teams);
    Session_SetLockPhase(false);
    Notify_TeamPhaseStarted(source);
}

bool TeamManager_IsLockPhaseActive()
{
    return Session_IsLockPhase();
}
