@echo off
setlocal
pushd "%~dp0"
call "%~dp0build_serverflow.bat"
if errorlevel 1 (
  echo [startup] serverflow build failed. Server start aborted.
  exit /b %errorlevel%
)
srcds.exe -game csgo -console -tickrate 128 +game_type 0 +game_mode 1 +mapgroup mg_active +map de_mirage -port 27015 +sv_setsteamaccount YOUR_GSLT_TOKEN_HERE
