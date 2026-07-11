@echo off
setlocal enabledelayedexpansion

echo =========================================
echo Building C++ Engine and Running Game...
echo =========================================

:: Clean old broken cache if it exists
if exist "build\CMakeCache.txt" (
    echo Cleaning old CMake cache...
    del /f /q "build\CMakeCache.txt"
)

echo.
echo [0/3] Detecting Compiler Environment...

:: Check for GCC (WinLibs / MinGW) first
where gcc >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo GCC / MinGW ^(WinLibs^) detected! Using MinGW Makefiles.
    echo.
    echo [1/3] Configuring CMake...

    :: Get the exact path of the active Python executable
    for /f "delims=" %%I in ('python -c "import sys; print(sys.executable)"') do set "PYTHON_EXE=%%I"
    echo Using Python executable: !PYTHON_EXE!

    cmake -B build -G "MinGW Makefiles" -DPYTHON_EXECUTABLE="!PYTHON_EXE!"
) else (
    echo GCC not found. Attempting to find Visual Studio...

    :: Find Visual Studio / Build Tools path using vswhere
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if not exist "!VSWHERE!" (
        echo Error: Neither GCC ^(WinLibs^) nor Visual Studio was found!
        echo Please ensure your compiler is added to the system PATH.
        pause
        exit /b 1
    )

    :: Get the path to vcvars64.bat
    for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VS_PATH=%%i"
    )

    if "!VS_PATH!"=="" (
        echo Error: Could not find Visual Studio C++ Compiler!
        pause
        exit /b 1
    )

    :: Run vcvars64.bat to set up the compiler environment
    call "!VS_PATH!\VC\Auxiliary\Build\vcvars64.bat"

    echo.
    echo [1/3] Configuring CMake...

    :: Get the exact path of the active Python executable
    for /f "delims=" %%I in ('python -c "import sys; print(sys.executable)"') do set "PYTHON_EXE=%%I"
    echo Using Python executable: !PYTHON_EXE!

    cmake -B build -G "Visual Studio 17 2022" -A x64 -DPYTHON_EXECUTABLE="!PYTHON_EXE!"
    if !ERRORLEVEL! neq 0 (
        echo Attempting fallback to older Visual Studio generator...
        cmake -B build -DPYTHON_EXECUTABLE="!PYTHON_EXE!"
    )
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
