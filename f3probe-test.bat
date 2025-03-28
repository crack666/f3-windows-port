@echo off
echo F3 Probe - Test flash memory for counterfeit
echo ==========================================
echo.

if "%~1"=="" (
  echo Usage: f3probe-test.bat [DRIVE_LETTER] [--destructive]
  echo Example: f3probe-test.bat J
  echo Example: f3probe-test.bat J --destructive
  exit /b 1
)

set DRIVE=%~1:
set DESTRUCTIVE=
set TIME_OPS=--time-ops

if "%~2"=="--destructive" (
  set DESTRUCTIVE=--destructive
  echo WARNING: Destructive mode will OVERWRITE DATA on drive %DRIVE%
  echo.
  echo Press CTRL+C now to cancel, or...
  pause
  echo.
)

echo Testing flash drive %DRIVE% (requires Administrator privileges)
echo.
f3probe.exe %DESTRUCTIVE% %TIME_OPS% %DRIVE%
echo.
echo Test completed.
pause