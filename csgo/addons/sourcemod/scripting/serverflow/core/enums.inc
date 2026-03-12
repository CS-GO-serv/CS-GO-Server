#if defined _SERVERFLOW_ENUMS_INC_
 #endinput
#endif
#define _SERVERFLOW_ENUMS_INC_

enum ServerState
{
    ServerState_Lobby = 0,
    ServerState_PreMatch,
    ServerState_Match,
    ServerState_PostMatch,
    ServerState_Transition
};

enum PendingChangeState
{
    Pending_None = 0,
    Pending_Queued,
    Pending_Confirmed,
    Pending_Applying
};

enum VoteState
{
    VoteState_Idle = 0,
    VoteState_Running,
    VoteState_Finished
};
