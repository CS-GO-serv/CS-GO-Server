@echo off
setlocal
pushd "%~dp0"
echo [startup] start_dz.bat is deprecated. Forwarding to start.bat...
call "%~dp0start.bat"
exit /b %errorlevel%
