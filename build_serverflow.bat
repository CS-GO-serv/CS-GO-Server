@echo off
setlocal enabledelayedexpansion

set "ROOT=%~dp0"
set "SCRIPTING=%ROOT%csgo\addons\sourcemod\scripting"
set "INCLUDE=%SCRIPTING%\include"
set "SRC=%SCRIPTING%\serverflow.sp"
set "OUT_PLUGIN=%ROOT%csgo\addons\sourcemod\plugins\serverflow.smx"
set "OUT_COMPILED=%SCRIPTING%\compiled\serverflow.smx"
set "LOG=%ROOT%build_serverflow.log"

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
