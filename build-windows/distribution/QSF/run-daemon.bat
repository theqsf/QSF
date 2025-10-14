@echo off
cd /d "%~dp0"
echo Starting QSF Daemon...
qsf.exe --rpc-bind-ip=127.0.0.1 --rpc-bind-port=18072
pause
