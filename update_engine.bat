@echo off
setlocal enabledelayedexpansion

echo =========================================
echo Updating C++ Engine via Git...
echo =========================================

:: Check if git is installed
where git >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Error: Git is not installed or not in PATH!
    pause
    goto end
)

echo.
echo Pulling latest changes...
git pull

if %ERRORLEVEL% neq 0 (
    echo Error: Failed to pull latest changes from Git!
    pause
    goto end
)

echo.
echo Update successful! Starting build process...
call build_and_run.bat

:end
