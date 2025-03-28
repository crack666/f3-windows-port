# F3 - Fight Flash Fraud for Windows

This is a Windows port of the popular F3 (Fight Flash Fraud) tool, which helps detect counterfeit flash drives and memory cards.

## Overview

Counterfeit flash drives are common in the market - these drives report a larger capacity than they actually have. F3 provides tools to test if your USB flash drive or memory card is genuine.

This Windows port includes three main tools:

- **f3write**: Writes test files to the flash drive
- **f3read**: Verifies test files written by f3write
- **f3probe**: Directly probes the device at a hardware level (requires admin privileges)

## Requirements

- Windows 7/8/10/11
- Administrator privileges (for f3probe)

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