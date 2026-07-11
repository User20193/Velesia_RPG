#!/bin/bash
set -e

echo "========================================="
echo "Updating C++ Engine via Git..."
echo "========================================="

if ! command -v git &> /dev/null
then
    echo "Error: Git is not installed or not in PATH!"
    exit 1
fi

echo ""
echo "Pulling latest changes..."
git pull

echo ""
echo "Update successful! Starting build process..."
./build_and_run.sh
