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
  echo [mode_vote build] install full SourceMod package (with scripting compiler) and retry.
  exit /b 2
)

echo [mode_vote build] compiling mode_vote.sp ...
"%SPCOMP%" "%SRC%" -i "%INCLUDE%" -o "%OUT%"
if errorlevel 1 (
  echo [mode_vote build] compile failed.
  exit /b 3
)

echo [mode_vote build] compiled successfully: %OUT%
exit /b 0
