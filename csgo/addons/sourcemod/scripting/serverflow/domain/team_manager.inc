#if defined _SERVERFLOW_TEAM_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_TEAM_MANAGER_INC_

void TeamManager_EnterPreMatchSetup(const char[] source = "prematch_setup")
{
    Session_SetPreMatchPhase(PreMatch_Teams);
    Session_SetLockPhase(false);
    Notify_TeamPhaseStarted(source);
}

bool TeamManager_IsLockPhaseActive()
{
    return Session_IsLockPhase();
}
