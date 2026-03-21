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

enum PreMatchPhase
{
    PreMatch_Setup = 0,
    PreMatch_Teams,
    PreMatch_Ready,
    PreMatch_Lock
};

enum PostMatchPhase
{
    PostMatch_Results = 0,
    PostMatch_Decision,
    PostMatch_Confirm
};

enum PendingChangeState
{
    Pending_None = 0,
    Pending_Proposed,
    Pending_Selected,
    Pending_Confirmed,
    Pending_Applying
};

enum PendingConfirmPolicy
{
    PendingConfirm_Manual = 0,
    PendingConfirm_Auto
};

enum PlayerLifecycleState
{
    PlayerLifecycle_Connected = 0,
    PlayerLifecycle_Spectator,
    PlayerLifecycle_Eligible,
    PlayerLifecycle_Unready,
    PlayerLifecycle_Ready,
    PlayerLifecycle_Assigned,
    PlayerLifecycle_Afk,
    PlayerLifecycle_LateJoin,
    PlayerLifecycle_LockedIn
};

enum ReadyPolicy
{
    ReadyPolicy_Disabled = 0,
    ReadyPolicy_Optional,
    ReadyPolicy_Required
};

enum TeamPolicy
{
    TeamPolicy_None = 0,
    TeamPolicy_Auto,
    TeamPolicy_Manual
};

enum TeamValidationMode
{
    TeamValidation_None = 0,
    TeamValidation_Balance,
    TeamValidation_Strict
};

enum VoteType
{
    VoteType_None = 0,
    VoteType_Scenario,
    VoteType_Map,
    VoteType_Teams,
    VoteType_Ready
};

enum VoteState
{
    VoteState_Idle = 0,
    VoteState_Running,
    VoteState_Finished
};

enum OverrideLevel
{
    OverrideLevel_None = 0,
    OverrideLevel_Moderator,
    OverrideLevel_Admin,
    OverrideLevel_Console
};

enum LateJoinPolicy
{
    LateJoinPolicy_Disallow = 0,
    LateJoinPolicy_Spectator,
    LateJoinPolicy_Allow
};

enum AfkPolicy
{
    AfkPolicy_None = 0,
    AfkPolicy_Warn,
    AfkPolicy_MoveToSpec,
    AfkPolicy_Kick
};

enum PostMatchDefaultAction
{
    PostMatchAction_Stay = 0,
    PostMatchAction_Vote,
    PostMatchAction_Rotate,
    PostMatchAction_Lobby
};

enum RotationMode
{
    RotationMode_Sequential = 0,
    RotationMode_Random
};

enum MaplistPolicy
{
    MaplistPolicy_Playlist = 0,
    MaplistPolicy_Scenario
};

enum TransitionType
{
    TransitionType_None = 0,
    TransitionType_EnterLobby,
    TransitionType_EnterPreMatch,
    TransitionType_EnterMatch,
    TransitionType_EnterPostMatch,
    TransitionType_EnterTransition,
    TransitionType_ApplyPending,
    TransitionType_Fallback
};

enum HealthStatus
{
    HealthStatus_Ok = 0,
    HealthStatus_Warn,
    HealthStatus_Degraded,
    HealthStatus_Fail
};

enum VotePolicy
{
    VotePolicy_Majority = 0,
    VotePolicy_Roulette,
    VotePolicy_EqualWeight,
    VotePolicy_RandomTop2,
    VotePolicy_Threshold
};
