@echo off
setlocal enabledelayedexpansion

set "ROOT=%~dp0"
set "SPCOMP=%ROOT%csgo\addons\sourcemod\scripting\spcomp.exe"
set "INCLUDE=%ROOT%csgo\addons\sourcemod\scripting\include"
set "SRC=%ROOT%csgo\addons\sourcemod\scripting\serverflow.sp"
set "OUT_PLUGIN=%ROOT%csgo\addons\sourcemod\plugins\serverflow.smx"
set "OUT_COMPILED=%ROOT%csgo\addons\sourcemod\scripting\compiled\serverflow.smx"

if not exist "%SRC%" (
  echo [serverflow build] source not found: %SRC%
  exit /b 1
)

if not exist "%SPCOMP%" (
  echo [serverflow build] spcomp.exe not found: %SPCOMP%
  if exist "%OUT_PLUGIN%" (
    echo [serverflow build] using existing compiled plugin: %OUT_PLUGIN%
    exit /b 0
  )
  if exist "%OUT_COMPILED%" (
    echo [serverflow build] using existing compiled artifact: %OUT_COMPILED%
    exit /b 0
  )
  echo [serverflow build] install full SourceMod package (with scripting compiler) or provide serverflow.smx.
  exit /b 2
)

echo [serverflow build] compiling serverflow.sp ...
"%SPCOMP%" "%SRC%" -i "%INCLUDE%" -o "%OUT_PLUGIN%"
if errorlevel 1 (
  echo [serverflow build] compile failed.
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

if not exist "%ROOT%csgo\addons\sourcemod\scripting\compiled" (
  mkdir "%ROOT%csgo\addons\sourcemod\scripting\compiled" >nul 2>&1
)
copy /Y "%OUT_PLUGIN%" "%OUT_COMPILED%" >nul
echo [serverflow build] compiled successfully: %OUT_PLUGIN%
echo [serverflow build] mirror artifact: %OUT_COMPILED%
exit /b 0
