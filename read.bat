@echo off
:top
cls
nslookup -type=txt poll-concat.dnschat dns.c3murk.dev
timeout /t 2 /nobreak>nul
goto top
