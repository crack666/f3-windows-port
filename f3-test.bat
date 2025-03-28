@echo off
echo F3 - Fight Flash Fraud - Windows Edition
echo =========================================
echo.

if "%~1"=="" (
  echo Usage: f3-test.bat [DRIVE_LETTER]
  echo Example: f3-test.bat E
  exit /b 1
)

set DRIVE=%~1:

echo Testing flash drive %DRIVE%\
echo.
echo Step 1: Writing test files with f3write.exe...
echo.
f3write.exe %DRIVE%\
echo.
echo Step 2: Verifying test files with f3read.exe...
echo.
f3read.exe %DRIVE%\
echo.
echo Test completed.
pause