#!/bin/bash
# Script to create a release ZIP for F3 Windows port

# Check if zip is installed
if ! command -v zip &> /dev/null; then
    echo "Error: zip command not found. Please install zip."
    exit 1
fi

# Ensure we have MinGW-w64 compiler
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "Error: x86_64-w64-mingw32-gcc not found. Please install MinGW-w64."
    exit 1
fi

# Build the executables
echo "Building Windows executables..."
./build-win.sh

# Create a version file with date
VERSION=$(date +"%Y%m%d")
echo "Creating release for version $VERSION"

# Create release directory
RELEASE_DIR="f3-windows-$VERSION"
mkdir -p "$RELEASE_DIR"

# Copy files to release directory
echo "Copying files to release directory..."
cp f3write.exe f3read.exe f3probe.exe "$RELEASE_DIR/"
cp f3-test.bat f3probe-test.bat "$RELEASE_DIR/"
cp README.md README_WINDOWS.txt "$RELEASE_DIR/"

# Create ZIP file
echo "Creating ZIP file..."
zip -r "${RELEASE_DIR}.zip" "$RELEASE_DIR"

# Clean up
echo "Cleaning up..."
rm -rf "$RELEASE_DIR"

echo "Release created: ${RELEASE_DIR}.zip"
echo "You can now upload this file to GitHub Releases."