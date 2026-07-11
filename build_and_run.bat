@echo off
echo =========================================
echo Building C++ Engine and Running Game...
echo =========================================

echo.
echo [1/3] Configuring CMake...
cmake -B build
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
