#if defined _SERVERFLOW_AUDIT_LOGGER_INC_
 #endinput
#endif
#define _SERVERFLOW_AUDIT_LOGGER_INC_

void Audit_LogEvent(const char[] category, const char[] action, int actor, const char[] result, const char[] reason = "")
{
    int now = GetTime();
    LogToFileEx(
        MODE_ACTION_LOG,
        "category=%s action=%s actor=%d result=%s reason=%s ts=%d",
        category,
        action,
        actor,
        result,
        reason,
        now
    );
}

void Audit_Log(const char[] message)
{
    Audit_LogEvent("legacy", "raw", 0, "ok", message);
}
