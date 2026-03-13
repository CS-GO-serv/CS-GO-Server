#if defined _SERVERFLOW_SESSION_STORE_INC_
 #endinput
#endif
#define _SERVERFLOW_SESSION_STORE_INC_

static void Session_ResetVote()
{
    g_Session.vote.type = VoteType_None;
    g_Session.vote.state = VoteState_Idle;
    g_Session.vote.inProgress = false;
    g_Session.vote.timer = null;
    g_Session.vote.requestedBy = 0;
    g_Session.vote.startedAt = 0;
    g_Session.vote.endsAt = 0;

    for (int i = 0; i < MAX_RUNTIME_MODES; i++)
    {
        g_Session.vote.counts[i] = 0;
    }

    for (int i = 1; i <= MaxClients; i++)
    {
        g_Session.vote.hasVoted[i] = false;
    }
}

void Session_Init()
{
    g_Session.currentState = ServerState_Lobby;
    g_Session.preMatchPhase = PreMatch_Setup;
    g_Session.postMatchPhase = PostMatch_Results;
    g_Session.lastTransitionType = TransitionType_None;

    strcopy(g_Session.currentScenario, sizeof(g_Session.currentScenario), "lobby");
    g_Session.queuedScenario[0] = '\0';
    g_Session.previousScenario[0] = '\0';

    GetCurrentMap(g_Session.currentMap, sizeof(g_Session.currentMap));
    g_Session.queuedMap[0] = '\0';

    g_Session.pendingChange.state = Pending_None;
    g_Session.pendingChange.scenarioId[0] = '\0';
    g_Session.pendingChange.mapName[0] = '\0';
    g_Session.pendingChange.source[0] = '\0';
    g_Session.pendingChange.requestedBy = 0;
    g_Session.pendingChange.requestedAt = 0;
    g_Session.pendingChange.confirmDeadline = 0;

    Session_ResetVote();

    g_Session.lockPhase = false;
    g_Session.countdownActive = false;
    g_Session.countdownEndsAt = 0;
    g_Session.overrideLevel = OverrideLevel_None;
    g_Session.fallbackReason[0] = '\0';
    g_Session.startedAt = GetTime();
    g_Session.lastStateChangeTs = g_Session.startedAt;
}

void Session_SetState(ServerState state)
{
    g_Session.currentState = state;
    g_Session.lastStateChangeTs = GetTime();
}

ServerState Session_GetState()
{
    return g_Session.currentState;
}

void Session_SetTransitionType(TransitionType transitionType)
{
    g_Session.lastTransitionType = transitionType;
}

void Session_SetPreMatchPhase(PreMatchPhase phase)
{
    g_Session.preMatchPhase = phase;
}

PreMatchPhase Session_GetPreMatchPhase()
{
    return g_Session.preMatchPhase;
}

void Session_SetLockPhase(bool enabled)
{
    g_Session.lockPhase = enabled;
}

bool Session_IsLockPhase()
{
    return g_Session.lockPhase;
}

void Session_SetCountdownState(bool active, int endsAt)
{
    g_Session.countdownActive = active;
    g_Session.countdownEndsAt = endsAt;
}

bool Session_IsCountdownActive()
{
    return g_Session.countdownActive;
}

int Session_GetCountdownEndsAt()
{
    return g_Session.countdownEndsAt;
}


void Session_SetOverrideLevel(OverrideLevel level)
{
    g_Session.overrideLevel = level;
}

OverrideLevel Session_GetOverrideLevel()
{
    return g_Session.overrideLevel;
}

void Session_SetFallbackReason(const char[] reason)
{
    strcopy(g_Session.fallbackReason, sizeof(g_Session.fallbackReason), reason);
}

void Session_GetFallbackReason(char[] buffer, int maxlen)
{
    strcopy(buffer, maxlen, g_Session.fallbackReason);
}

void Session_SetCurrentScenario(const char[] scenarioId)
{
    strcopy(g_Session.previousScenario, sizeof(g_Session.previousScenario), g_Session.currentScenario);
    strcopy(g_Session.currentScenario, sizeof(g_Session.currentScenario), scenarioId);
}

void Session_SetQueuedScenario(const char[] scenarioId)
{
    strcopy(g_Session.queuedScenario, sizeof(g_Session.queuedScenario), scenarioId);
}

void Session_SetQueuedMap(const char[] mapName)
{
    strcopy(g_Session.queuedMap, sizeof(g_Session.queuedMap), mapName);
}



void Session_GetCurrentScenario(char[] buffer, int maxlen)
{
    strcopy(buffer, maxlen, g_Session.currentScenario);
}

void Session_GetQueuedScenario(char[] buffer, int maxlen)
{
    strcopy(buffer, maxlen, g_Session.queuedScenario);
}

void Session_SetPendingChange(const char[] scenarioId, const char[] mapName, const char[] source, int actor, int confirmWindow)
{
    g_Session.pendingChange.state = Pending_Proposed;
    strcopy(g_Session.pendingChange.scenarioId, sizeof(g_Session.pendingChange.scenarioId), scenarioId);
    strcopy(g_Session.pendingChange.mapName, sizeof(g_Session.pendingChange.mapName), mapName);
    strcopy(g_Session.pendingChange.source, sizeof(g_Session.pendingChange.source), source);
    g_Session.pendingChange.requestedBy = actor;
    g_Session.pendingChange.requestedAt = GetTime();
    g_Session.pendingChange.confirmDeadline = g_Session.pendingChange.requestedAt + confirmWindow;
}

void Session_SetPendingChangeState(PendingChangeState state)
{
    g_Session.pendingChange.state = state;
}

PendingChangeState Session_GetPendingChangeState()
{
    return g_Session.pendingChange.state;
}

void Session_GetPendingChange(PendingChange &change)
{
    change = g_Session.pendingChange;
}

void Session_ClearPendingChange()
{
    g_Session.pendingChange.state = Pending_None;
    g_Session.pendingChange.scenarioId[0] = '\0';
    g_Session.pendingChange.mapName[0] = '\0';
    g_Session.pendingChange.source[0] = '\0';
    g_Session.pendingChange.requestedBy = 0;
    g_Session.pendingChange.requestedAt = 0;
    g_Session.pendingChange.confirmDeadline = 0;
    g_Session.overrideLevel = OverrideLevel_None;
}

bool Session_IsVoteInProgress()
{
    return g_Session.vote.inProgress;
}

int Session_GetVoteNextAllowedAt()
{
    return g_Session.vote.nextAllowedAt;
}

void Session_StartVote(VoteType voteType, int caller, float duration, int cooldown)
{
    Session_ResetVote();
    g_Session.vote.type = voteType;
    g_Session.vote.state = VoteState_Running;
    g_Session.vote.inProgress = true;
    g_Session.vote.requestedBy = caller;
    g_Session.vote.startedAt = GetTime();
    g_Session.vote.endsAt = g_Session.vote.startedAt + RoundToCeil(duration);
    g_Session.vote.nextAllowedAt = g_Session.vote.startedAt + cooldown;
}

bool Session_HasClientVoted(int client)
{
    return g_Session.vote.hasVoted[client];
}

void Session_RecordVote(int client, int modeIndex)
{
    if (modeIndex < 0 || modeIndex >= MAX_RUNTIME_MODES)
    {
        return;
    }

    g_Session.vote.hasVoted[client] = true;
    g_Session.vote.counts[modeIndex]++;
}

int Session_GetVoteCount(int modeIndex)
{
    if (modeIndex < 0 || modeIndex >= MAX_RUNTIME_MODES)
    {
        return 0;
    }

    return g_Session.vote.counts[modeIndex];
}

void Session_FinishVote()
{
    g_Session.vote.inProgress = false;
    g_Session.vote.state = VoteState_Finished;
    g_Session.vote.timer = null;
}

void Session_SetVoteTimer(Handle timer)
{
    g_Session.vote.timer = timer;
}

Handle Session_GetVoteTimer()
{
    return g_Session.vote.timer;
}
