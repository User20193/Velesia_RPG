#!/bin/bash
set -e

echo "========================================="
echo "Building C++ Engine and Running Game..."
echo "========================================="

echo ""
echo "[1/3] Configuring CMake..."
PYTHON_EXE=$(python3 -c "import sys; print(sys.executable)")
echo "Using Python executable: $PYTHON_EXE"
cmake -B build -DPYTHON_EXECUTABLE="$PYTHON_EXE"

echo ""
echo "[2/3] Compiling C++ Engine..."
cmake --build build -j$(nproc)

echo ""
echo "[3/3] Engine compiled successfully! Starting Game..."
echo ""
cd game
xvfb-run python3 main.py

echo ""
echo "Game closed."
