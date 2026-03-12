#if defined _SERVERFLOW_CONFIG_PLAYLISTS_INC_
 #endinput
#endif
#define _SERVERFLOW_CONFIG_PLAYLISTS_INC_

bool ConfigPlaylists_Reload()
{
    Playlists_LoadFromConfig();

    for (int modeIndex = 0; modeIndex < MAX_MODES; modeIndex++)
    {
        File file = OpenFile(g_Modes[modeIndex].mapListFile, "r");
        if (file == null)
        {
            Format(g_ModeMapCacheError[modeIndex], sizeof(g_ModeMapCacheError[]), "cannot open %s", g_Modes[modeIndex].mapListFile);
            continue;
        }

        char line[128];
        while (!file.EndOfFile() && file.ReadLine(line, sizeof(line)))
        {
            TrimString(line);
            if (line[0] == '\0' || line[0] == ';' || (line[0] == '/' && line[1] == '/'))
            {
                continue;
            }

            g_ModeMapCache[modeIndex].PushString(line);
        }

        g_ModeMapCacheLoaded[modeIndex] = true;
        delete file;
    }

    return true;
}
