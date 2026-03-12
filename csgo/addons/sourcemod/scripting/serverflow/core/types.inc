#if defined _SERVERFLOW_TYPES_INC_
 #endinput
#endif
#define _SERVERFLOW_TYPES_INC_

enum struct ModeInfo
{
    char id[16];
    char routerAlias[32];
    char namePhrase[32];
    int gameType;
    int gameMode;
    char mapgroup[32];
    char startMap[64];
    char cfgFile[64];
    char mapListFile[96];
    char fallbackMap[64];
}

enum struct PendingChange
{
    PendingChangeState state;
    char scenarioId[16];
    char mapName[64];
    char source[32];
    int requestedBy;
    int requestedAt;
    int confirmDeadline;
}

enum struct VoteRecord
{
    VoteType type;
    VoteState state;
    bool inProgress;
    int counts[MAX_RUNTIME_MODES];
    bool hasVoted[MAXPLAYERS + 1];
    Handle timer;
    int requestedBy;
    int startedAt;
    int endsAt;
    int nextAllowedAt;
}

enum struct RuntimeSession
{
    ServerState currentState;
    PreMatchPhase preMatchPhase;
    PostMatchPhase postMatchPhase;
    TransitionType lastTransitionType;

    char currentScenario[16];
    char queuedScenario[16];
    char previousScenario[16];

    char currentMap[64];
    char queuedMap[64];

    PendingChange pendingChange;
    VoteRecord vote;

    bool lockPhase;
    bool countdownActive;
    int countdownEndsAt;

    OverrideLevel overrideLevel;
    char fallbackReason[128];

    int startedAt;
    int lastStateChangeTs;
}

enum struct PlayerRuntimeState
{
    PlayerLifecycleState lifecycle;
    bool selectedDzAutoAssign;
    int selectedDzTeamCount;
    int selectedModeIndex;
    int lastActionAt;
}
