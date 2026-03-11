@echo off
setlocal enabledelayedexpansion

set "ROOT=%~dp0"
set "SPCOMP=%ROOT%csgo\addons\sourcemod\scripting\spcomp.exe"
set "INCLUDE=%ROOT%csgo\addons\sourcemod\scripting\include"
set "SRC=%ROOT%csgo\addons\sourcemod\scripting\mode_vote.sp"
set "OUT=%ROOT%csgo\addons\sourcemod\plugins\mode_vote.smx"

if not exist "%SRC%" (
  echo [mode_vote build] source not found: %SRC%
  exit /b 1
)

if not exist "%SPCOMP%" (
  echo [mode_vote build] spcomp.exe not found: %SPCOMP%
  if exist "%OUT%" (
    echo [mode_vote build] using existing compiled plugin: %OUT%
    exit /b 0
  )
  echo [mode_vote build] install full SourceMod package (with scripting compiler) or provide mode_vote.smx.
  exit /b 2
)

echo [mode_vote build] compiling mode_vote.sp ...
"%SPCOMP%" "%SRC%" -i "%INCLUDE%" -o "%OUT%"
if errorlevel 1 (
  echo [mode_vote build] compile failed.
  if exist "%OUT%" (
    echo [mode_vote build] falling back to existing plugin binary: %OUT%
    exit /b 0
  )
  exit /b 3
)

echo [mode_vote build] compiled successfully: %OUT%
exit /b 0
