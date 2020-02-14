@echo off
set buf=%temp%/%random%-dnschat
:top
nslookup -type=txt poll-concat.dnschat dns.c3murk.dev > %buf%
cls
type %buf%
timeout /t 2 /nobreak>nul
goto top
