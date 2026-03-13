#if defined _SERVERFLOW_STATE_DUMP_INC_
 #endinput
#endif
#define _SERVERFLOW_STATE_DUMP_INC_

void StateDump_ToServer()
{
    PendingChange pending;
    Session_GetPendingChange(pending);

    PrintToServer(
        "[ServerFlow Dump] state=%d pre_phase=%d post_phase=%d transition=%d",
        g_Session.currentState,
        g_Session.preMatchPhase,
        g_Session.postMatchPhase,
        g_Session.lastTransitionType
    );

    PrintToServer(
        "[ServerFlow Dump] scenario_current=%s scenario_prev=%s scenario_queued=%s map_current=%s map_queued=%s",
        g_Session.currentScenario,
        g_Session.previousScenario,
        g_Session.queuedScenario,
        g_Session.currentMap,
        g_Session.queuedMap
    );

    PrintToServer(
        "[ServerFlow Dump] pending_state=%d pending_scenario=%s pending_map=%s pending_source=%s pending_actor=%d pending_requested=%d pending_confirmed_by=%d pending_confirmed_at=%d pending_why=%s pending_cancel=%s pending_allowed_state=%d pending_expires=%d pending_policy=%d",
        pending.state,
        pending.scenarioId,
        pending.mapName,
        pending.source,
        pending.requestedBy,
        pending.requestedAt,
        pending.confirmedBy,
        pending.confirmedAt,
        pending.why,
        pending.cancelReason,
        pending.allowedApplyState,
        pending.expiresAt,
        pending.confirmPolicy
    );

    PrintToServer(
        "[ServerFlow Dump] vote_type=%d vote_state=%d vote_in_progress=%d vote_requested_by=%d vote_started=%d vote_ends=%d vote_next_allowed=%d vote_timer=%d",
        g_Session.vote.type,
        g_Session.vote.state,
        g_Session.vote.inProgress,
        g_Session.vote.requestedBy,
        g_Session.vote.startedAt,
        g_Session.vote.endsAt,
        g_Session.vote.nextAllowedAt,
        (g_Session.vote.timer != null)
    );

    PrintToServer(
        "[ServerFlow Dump] override=%d lock_phase=%d countdown_active=%d countdown_ends=%d started_at=%d last_state_change=%d fallback_reason=%s",
        g_Session.overrideLevel,
        g_Session.lockPhase,
        g_Session.countdownActive,
        g_Session.countdownEndsAt,
        g_Session.startedAt,
        g_Session.lastStateChangeTs,
        g_Session.fallbackReason
    );
}
