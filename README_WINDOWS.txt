F3 - Fight Flash Fraud - Windows Edition
=======================================

This directory contains Windows executables for the F3 tools:

- f3write.exe
- f3read.exe
- f3probe.exe (Windows-specific implementation)

These are simplified versions of the F3 tools compiled for Windows.

Usage
-----

1. Open Command Prompt (cmd.exe) as Administrator (right-click, "Run as administrator")
2. Navigate to the folder containing these executables
3. Run the tools as follows:

   f3write.exe E:\
   f3read.exe E:\
   
   f3probe.exe [options] J:
   
   Options for f3probe:
   --destructive    Perform destructive testing (will overwrite data)
   --time-ops       Time read and write operations
   --help           Show help message

Examples
--------

To test if your flash drive is counterfeit (non-destructive mode):
   f3probe.exe J:

To test with destructive mode (WARNING: will overwrite data!):
   f3probe.exe --destructive J:

Important Notes
--------------

1. f3probe.exe REQUIRES ADMINISTRATOR PRIVILEGES since it accesses the raw disk.

2. When using f3probe's destructive mode, ALL DATA ON THE DRIVE MAY BE LOST!
   Always backup your data before using this option.

3. These are simplified versions with limitations compared to the Linux versions:
   - f3write/f3read are placeholder implementations
   - f3probe is a custom Windows implementation with basic counterfeit detection

4. The original Linux-only tools f3fix and f3brew are not available as Windows executables.

5. If you need complete functionality with 100% feature parity:
   - Setup proper access to Windows drives from WSL
   - Use Docker with the F3 image

For more information, please refer to the full F3 documentation at:
https://github.com/AltraMayor/f3

License
-------

F3 is licensed under the GNU General Public License v3.0