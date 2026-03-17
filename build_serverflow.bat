@echo off
setlocal enabledelayedexpansion

set "ROOT=%~dp0"
set "SCRIPTING=%ROOT%csgo\addons\sourcemod\scripting"
set "INCLUDE=%SCRIPTING%\include"
set "SRC=%SCRIPTING%\serverflow.sp"
set "OUT_PLUGIN=%ROOT%csgo\addons\sourcemod\plugins\serverflow.smx"
set "OUT_COMPILED=%SCRIPTING%\compiled\serverflow.smx"
set "LOG=%ROOT%build_serverflow.log"

set "CFG_PLAYLISTS=%ROOT%csgo\addons\sourcemod\scripting\serverflow\config\config_playlists.inc"
set "RT_TRANSITION=%ROOT%csgo\addons\sourcemod\scripting\serverflow\runtime\transition_manager.inc"

set "SPCOMP="
if exist "%SCRIPTING%\spcomp.exe" set "SPCOMP=%SCRIPTING%\spcomp.exe"
if not defined SPCOMP if exist "%SCRIPTING%\spcomp64.exe" set "SPCOMP=%SCRIPTING%\spcomp64.exe"
if not defined SPCOMP if exist "%SCRIPTING%\spcomp" set "SPCOMP=%SCRIPTING%\spcomp"
if not defined SPCOMP if exist "%SCRIPTING%\spcomp64" set "SPCOMP=%SCRIPTING%\spcomp64"

> "%LOG%" echo [serverflow build] started at %DATE% %TIME%
>>"%LOG%" echo [serverflow build] ROOT=%ROOT%
>>"%LOG%" echo [serverflow build] SRC=%SRC%
>>"%LOG%" echo [serverflow build] OUT_PLUGIN=%OUT_PLUGIN%
>>"%LOG%" echo [serverflow build] OUT_COMPILED=%OUT_COMPILED%

if not exist "%SRC%" (
  echo [serverflow build] source not found: %SRC%
  >>"%LOG%" echo [serverflow build] ERROR source not found
  exit /b 1
)

set "NEED_FIX=0"
if exist "%CFG_PLAYLISTS%" (
  findstr /N /C:"Path_Game" "%CFG_PLAYLISTS%" >nul
  if not errorlevel 1 (
    set "NEED_FIX=1"
    echo [serverflow build] warning: stale marker found ^(Path_Game^) in %CFG_PLAYLISTS%
    >>"%LOG%" echo [serverflow build] WARN stale marker: Path_Game in config_playlists.inc
    >>"%LOG%" findstr /N /C:"Path_Game" "%CFG_PLAYLISTS%"
  )
)
if exist "%RT_TRANSITION%" (
  findstr /N /C:"PendingChange ^&" "%RT_TRANSITION%" >nul
  if not errorlevel 1 (
    set "NEED_FIX=1"
    echo [serverflow build] warning: stale marker found ^(PendingChange ^&^) in %RT_TRANSITION%
    >>"%LOG%" echo [serverflow build] WARN stale marker: PendingChange ^& in transition_manager.inc
    >>"%LOG%" findstr /N /C:"PendingChange ^&" "%RT_TRANSITION%"
  )
)

if "%NEED_FIX%"=="1" (
  echo [serverflow build] ERROR stale source markers detected. Build is read-only and will not patch files.
  echo [serverflow build] ACTION required: update tracked source files in git and retry build.
  echo [serverflow build] EXPECTED fixes:
  echo [serverflow build]   - remove Path_Game-based replacements in %CFG_PLAYLISTS%
  echo [serverflow build]   - remove PendingChange ^& marker in %RT_TRANSITION%
  echo [serverflow build] tip: run git status, restore/rebase stale files, then rerun build_serverflow.bat
  >>"%LOG%" echo [serverflow build] ERROR stale source markers detected; read-only mode abort
  >>"%LOG%" echo [serverflow build] ACTION update source files in git and retry build
  exit /b 5
)

if not defined SPCOMP (
  echo [serverflow build] compiler not found in %SCRIPTING% ^(spcomp.exe/spcomp64.exe/spcomp/spcomp64^)
  >>"%LOG%" echo [serverflow build] ERROR compiler not found
  if exist "%OUT_PLUGIN%" (
    echo [serverflow build] using existing compiled plugin: %OUT_PLUGIN%
    >>"%LOG%" echo [serverflow build] fallback existing OUT_PLUGIN
    exit /b 0
  )
  if exist "%OUT_COMPILED%" (
    echo [serverflow build] using existing compiled artifact: %OUT_COMPILED%
    >>"%LOG%" echo [serverflow build] fallback existing OUT_COMPILED
    exit /b 0
  )
  echo [serverflow build] no compiler and no existing binary. See %LOG%
  exit /b 2
)

echo [serverflow build] using compiler: %SPCOMP%
>>"%LOG%" echo [serverflow build] using compiler: %SPCOMP%

if not exist "%ROOT%csgo\addons\sourcemod\plugins" mkdir "%ROOT%csgo\addons\sourcemod\plugins" >nul 2>&1
if not exist "%SCRIPTING%\compiled" mkdir "%SCRIPTING%\compiled" >nul 2>&1

echo [serverflow build] compiling serverflow.sp ...
"%SPCOMP%" "%SRC%" -i "%INCLUDE%" -o "%OUT_PLUGIN%" >>"%LOG%" 2>&1
if errorlevel 1 (
  echo [serverflow build] compile failed. See %LOG%
  echo [serverflow build] hint: update server sources from repo if log shows Path_Game or PendingChange ^& markers.
  if exist "%OUT_PLUGIN%" (
    echo [serverflow build] falling back to existing plugin binary: %OUT_PLUGIN%
    exit /b 0
  )
  if exist "%OUT_COMPILED%" (
    echo [serverflow build] falling back to existing compiled artifact: %OUT_COMPILED%
    exit /b 0
  )
  exit /b 3
)

if not exist "%OUT_PLUGIN%" (
  echo [serverflow build] compile returned success but output missing: %OUT_PLUGIN%
  >>"%LOG%" echo [serverflow build] ERROR output missing after compile
  exit /b 4
)

copy /Y "%OUT_PLUGIN%" "%OUT_COMPILED%" >nul
if errorlevel 1 (
  echo [serverflow build] warning: failed to mirror artifact to %OUT_COMPILED%
  >>"%LOG%" echo [serverflow build] WARN mirror copy failed
)

echo [serverflow build] compiled successfully: %OUT_PLUGIN%
echo [serverflow build] mirror artifact: %OUT_COMPILED%
echo [serverflow build] log: %LOG%
>>"%LOG%" echo [serverflow build] success
exit /b 0
