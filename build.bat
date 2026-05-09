@echo off
cd /d "%~dp0"
"C:\msys64\mingw64\bin\gcc.exe" -Wall -O2 ^
  vice.c init.c bitboard.c hushkeys.c board.c data.c ^
  attack.c movegen.c makemove.c evaluate.c search.c misc.c uci.c ^
  -o vice.exe > build_log.txt 2>&1
echo Build exit code: %ERRORLEVEL%
