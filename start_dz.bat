@echo off
setlocal
pushd "%~dp0"
call "%~dp0build_mode_vote.bat"
if errorlevel 1 (
  echo [startup] mode_vote build failed. Server start aborted.
  exit /b %errorlevel%
)
srcds.exe -game csgo -console -tickrate 128 +game_type 6 +game_mode 0 +mapgroup mg_dz_blacksite +map dz_blacksite -port 27015 +sv_setsteamaccount YOUR_GSLT_TOKEN_HERE
