@echo off
setlocal enabledelayedexpansion

echo =========================================
echo Building C++ Engine and Running Game...
echo =========================================

echo.
echo [0/3] Setting up Visual Studio Environment...
:: Find Visual Studio / Build Tools path using vswhere
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo Error: vswhere.exe not found! Make sure Visual Studio Build Tools are installed.
    pause
    exit /b 1
)

:: Get the path to vcvars64.bat
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)

if "%VS_PATH%"=="" (
    echo Error: Could not find Visual Studio C++ Compiler!
    pause
    exit /b 1
)

:: Run vcvars64.bat to set up the compiler environment
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"

:: Clean old broken cache if it exists
if exist "build\CMakeCache.txt" (
    echo Cleaning old CMake cache...
    del "build\CMakeCache.txt"
)

echo.
echo [1/3] Configuring CMake...
cmake -B build -G "Visual Studio 17 2022" -A x64
if %ERRORLEVEL% neq 0 (
    echo Attempting fallback to older Visual Studio generator...
    cmake -B build
)
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [2/3] Compiling C++ Engine (Release mode)...
cmake --build build --config Release
if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [3/3] Engine compiled successfully! Starting Game...
echo.
cd game
python main.py

echo.
echo Game closed.
pause
