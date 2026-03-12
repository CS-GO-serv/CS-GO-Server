#if defined _SERVERFLOW_DEBUG_INC_
 #endinput
#endif
#define _SERVERFLOW_DEBUG_INC_

void ModeDebug(const char[] fmt, any ...)
{
    if (g_CvarVerbose == null || !g_CvarVerbose.BoolValue)
    {
        return;
    }

    char message[256];
    VFormat(message, sizeof(message), fmt, 2);
    LogMessage("[serverflow] %s", message);
}
