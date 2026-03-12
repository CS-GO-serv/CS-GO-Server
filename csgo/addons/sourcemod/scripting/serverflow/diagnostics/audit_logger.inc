#if defined _SERVERFLOW_AUDIT_LOGGER_INC_
 #endinput
#endif
#define _SERVERFLOW_AUDIT_LOGGER_INC_

void Audit_Log(const char[] message)
{
    LogToFileEx(MODE_ACTION_LOG, "%s", message);
}
