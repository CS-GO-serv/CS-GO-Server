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

enum struct RuntimeSession
{
    ServerState currentState;
    char currentScenario[16];
    char queuedScenario[16];
    char currentMap[64];
    char queuedMap[64];
    bool lockPhase;
    int lastStateChangeTs;
}

enum struct PendingChange
{
    PendingChangeState state;
    char scenarioId[16];
    char mapName[64];
    int requestedBy;
    int requestedAt;
}
