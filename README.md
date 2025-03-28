# F3 - Fight Flash Fraud for Windows

This is a Windows port of the popular F3 (Fight Flash Fraud) tool, which helps detect counterfeit flash drives and memory cards.
![Windows Demo](https://github.com/user-attachments/assets/a2ac5203-10fc-4237-a413-c67b731efcab)

## Overview

Counterfeit flash drives are common in the market - these drives report a larger capacity than they actually have. F3 provides tools to test if your USB flash drive or memory card is genuine.

This Windows port includes three main tools:

- **f3write**: Writes test files to the flash drive
- **f3read**: Verifies test files written by f3write
- **f3probe**: Directly probes the device at a hardware level (requires admin privileges)

## Pre-compiled Binaries

For convenience, you can download pre-compiled Windows binaries from the [Releases](https://github.com/crack666/f3-windows-port/releases) section of this repository.

## Building from Source

### Requirements

- MinGW-w64 (for Windows builds)
- GCC (for Linux/WSL builds)
- Make

### Building on Linux/WSL for Windows

1. Clone the repository:
   ```
   git clone https://github.com/crack666/f3-windows-port.git
   cd f3-windows-port
   ```

2. Run the build script:
   ```
   chmod +x build-win.sh
   ./build-win.sh
   ```

3. The Windows executables (*.exe) will be created in the current directory.

### Building Directly on Windows

1. Install MinGW-w64:
   - Download from [MinGW-w64](https://www.mingw-w64.org/downloads/) or install using [MSYS2](https://www.msys2.org/)
   - Add MinGW bin directory to your PATH

2. Clone the repository or download the source files

3. Compile the source files:
   ```
   gcc -std=c99 -Wall -o f3write.exe f3write-win.c
   gcc -std=c99 -Wall -o f3read.exe f3read-win.c
   gcc -std=c99 -Wall -o f3probe.exe f3probe-win.c
   ```

## Usage

### Basic Flash Testing (f3write/f3read)

1. **Write test files**:
   ```
   f3write.exe J:
   ```
   This will fill drive J: with test pattern files. You can also specify how many MB to write:
   ```
   f3write.exe J: 1000
   ```
   
2. **Verify test files**:
   ```
   f3read.exe J:
   ```
   This reads back the test files and checks if they're intact. If any corruption is detected, the drive may be counterfeit or damaged.

### Advanced Hardware Testing (f3probe)

```
f3probe.exe J:
```

For more thorough testing (may overwrite data):
```
f3probe.exe --destructive J:
```

**Note**: f3probe requires administrator privileges and direct access to the drive. Some security software or write-protection mechanisms may interfere with its operation.

### Batch Testing

You can use the included batch files for easier testing:
- `f3-test.bat J` - Runs f3write and f3read on drive J:
- `f3probe-test.bat J` - Runs f3probe on drive J:

## Creating a Release

To create a release with pre-compiled binaries:

1. Build the executables following the instructions above
2. Create a ZIP archive containing:
   - f3write.exe
   - f3read.exe
   - f3probe.exe
   - f3-test.bat
   - f3probe-test.bat
   - README.md
   - README_WINDOWS.txt

3. Upload the ZIP file to the GitHub Releases section of this repository

## Limitations

This Windows port has some limitations compared to the Linux version:

1. f3probe may have limited functionality on some systems due to Windows' restrictions on direct disk access
2. f3fix and f3brew (Linux-only tools) are not available
3. Performance may vary compared to the Linux version

## Technical Details

The Windows port was created by:
1. Cross-compiling the f3write and f3read tools with MinGW
2. Reimplementing f3probe for Windows using direct disk access
3. Ensuring compatibility with Windows' file and path conventions

## Warning

- Always backup your data before testing drives with f3probe's destructive mode
- Be sure to run Command Prompt as Administrator when using f3probe

## Credits

- Original F3 tools by Michel Machado (https://github.com/AltraMayor/f3)
- Windows port created with Claude AI assistance

## License

The F3 tools are licensed under the GNU General Public License v3.0
