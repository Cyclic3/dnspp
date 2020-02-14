@echo off
if not exist "%userprofile%\.dnschat_counter" (
  echo %random%%random%  > "%userprofile%\.dnschat_counter"
  REM not cryptographic, but we need no leading 0
)

set /p dnschat_counter=<%userprofile%\.dnschat_counter
set /a dnschat_counter=%dnschat_counter% + 1

nslookup -type=txt %2.%1.%dnschat_counter%.dnschat dns.c3murk.dev

echo %dnschat_counter% > "%userprofile%\.dnschat_counter"
