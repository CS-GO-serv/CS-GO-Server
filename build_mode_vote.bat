@echo off
setlocal

REM Legacy compatibility shim.
REM Canonical build entrypoint is build_serverflow.bat.
call "%~dp0build_serverflow.bat"
exit /b %errorlevel%
