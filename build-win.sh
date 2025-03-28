#!/bin/bash

set -e

SCRIPTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRCDIR="$(dirname "$SCRIPTDIR")"

echo "Building F3 Windows executables..."
echo "Source directory: $SRCDIR"
echo "Build directory: $SCRIPTDIR"

# Ensure we have the MinGW-w64 compiler
if ! which x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
  echo "Error: x86_64-w64-mingw32-gcc not found. Please install MinGW-w64."
  exit 1
fi

cd "$SCRIPTDIR"

# Compile Windows versions
echo "Compiling Windows versions..."
x86_64-w64-mingw32-gcc -std=c99 -Wall -Wextra f3write-win.c -o f3write.exe
x86_64-w64-mingw32-gcc -std=c99 -Wall -Wextra f3read-win.c -o f3read.exe
x86_64-w64-mingw32-gcc -std=c99 -Wall -Wextra f3probe-win.c -o f3probe.exe

echo "Build completed successfully!"
echo "Windows executables are in: $SCRIPTDIR"