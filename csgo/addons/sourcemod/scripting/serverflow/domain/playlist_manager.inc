#if defined _SERVERFLOW_PLAYLIST_MANAGER_INC_
 #endinput
#endif
#define _SERVERFLOW_PLAYLIST_MANAGER_INC_

bool Playlists_LoadFromConfig()
{
    for (int i = 0; i < MAX_MODES; i++)
    {
        if (g_ModeMapCache[i] != null)
        {
            delete g_ModeMapCache[i];
        }

        g_ModeMapCache[i] = new ArrayList(ByteCountToCells(64));
        g_ModeMapCacheLoaded[i] = false;
        g_ModeMapCacheError[i][0] = '\0';
    }

    return true;
}

bool Playlist_IsMapAllowed(int modeIndex, const char[] mapName)
{
    if (modeIndex < 0 || modeIndex >= MAX_MODES || g_ModeMapCache[modeIndex] == null)
    {
        return false;
    }

    char entry[64];
    for (int i = 0; i < g_ModeMapCache[modeIndex].Length; i++)
    {
        g_ModeMapCache[modeIndex].GetString(i, entry, sizeof(entry));
        if (StrEqual(entry, mapName, false))
        {
            return true;
        }
    }

    return false;
}
