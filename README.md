# PoolViewer
An application to view and filter pool allocations from a dmp file on Windows 10 RS5+.
Presents information about active heaps and all pool allocations in the system, both allocated and free, as well as some basic statistics.
The information is extracted from a memory dump using Windows' Debugger API.

Integration of c++ code with WPF GUI relies on DllExport package and is heavily based on the code from here: 
https://www.codeproject.com/Articles/5253279/Create-An-Awesome-WPF-UI-for-Your-Cplusplus-QT-App

## Features
- Presents information in easy-to-use format
- Read from memory dump or from a live machine
- Can filter based on all fields
- Right-click on any of the tables in the "General" tab to filter pool blocks based on chosen row
- Export all pool blocks to csv

## Usage
You can either clone the repository and build it yourself or use the binaries found in Binaries folder.
Notice that the app needs dbgeng.dll and dbghelp.dll to exist in the same directory.
The ones in System32 are often broken, so copy the dlls from the same folder windbg.exe is in.

The app parses a memory dmp of a Windows 10 RS5+ machine, which can be created with livekd:
`livekd.exe -ml -k <path to kd.exe> -o c:\temp\live.dmp`
