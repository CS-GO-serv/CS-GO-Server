@echo off
setlocal
pushd "%~dp0"
call "%~dp0build_serverflow.bat"
if errorlevel 1 (
  echo [startup] serverflow build failed. Server start aborted.
  exit /b %errorlevel%
)
srcds.exe -game csgo -console -tickrate 128 +map de_mirage +exec mode_lobby.cfg -port 27015 +sv_setsteamaccount YOUR_GSLT_TOKEN_HERE
