@echo off
setlocal enabledelayedexpansion

set "ROOT=%~dp0"
set "SPCOMP=%ROOT%csgo\addons\sourcemod\scripting\spcomp.exe"
set "INCLUDE=%ROOT%csgo\addons\sourcemod\scripting\include"
set "SRC=%ROOT%csgo\addons\sourcemod\scripting\serverflow.sp"
set "OUT=%ROOT%csgo\addons\sourcemod\plugins\serverflow.smx"

if not exist "%SRC%" (
  echo [serverflow build] source not found: %SRC%
  exit /b 1
)

if not exist "%SPCOMP%" (
  echo [serverflow build] spcomp.exe not found: %SPCOMP%
  if exist "%OUT%" (
    echo [serverflow build] using existing compiled plugin: %OUT%
    exit /b 0
  )
  echo [serverflow build] install full SourceMod package (with scripting compiler) or provide serverflow.smx.
  exit /b 2
)

echo [serverflow build] compiling serverflow.sp ...
"%SPCOMP%" "%SRC%" -i "%INCLUDE%" -o "%OUT%"
if errorlevel 1 (
  echo [serverflow build] compile failed.
  if exist "%OUT%" (
    echo [serverflow build] falling back to existing plugin binary: %OUT%
    exit /b 0
  )
  exit /b 3
)

echo [serverflow build] compiled successfully: %OUT%
exit /b 0
