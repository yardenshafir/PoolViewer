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

## Build
There are 4 projects in the repository:
1. ManagedUIKitWpf - implementing the GUI for PoolViewer (required .NET 4.5 to be installed)
2. PoolData - implementing all pool analysis functionality
3. PoolViewer - using PoolData and ManagedUIKitWpf to implement GUI analysis tool
4. PoolViewExt - using PoolData to create a WinDbg extension (does not require ManagedUIKitWpf to be built)

## Usage
You can either clone the repository and build it yourself or use the binaries found in Binaries folder.
Notice that the app needs dbgeng.dll and dbghelp.dll to exist in the same directory.
The ones in System32 are often broken, so copy the dlls from the same folder windbg.exe is in.

The app parses a memory dmp of a Windows 10 RS5+ machine, which can be created with livekd:
`livekd.exe -ml -k <path to kd.exe> -o c:\temp\live.dmp`

Another option is analyzing the live machine - this is done by creating a temporary dmp file of the live machine and analyzing it. This option requires running PoolViewer as admin.

## PoolViewerExt
WinDbg extension to print information about a specific pool address or a pool tag.
Options:
1. !poolview [address]
```!poolview ffffcc8b7d8840c0
  Address              Size       (Status)      Tag    Type
  ---------------------------------------------------------
* 0xffffcc8b7d884050   0xe70      (Allocated)   Proc   Vs
  0xffffcc8b7d884ee0   0x100      (Free)               Vs
  ```
2. !poolview -tag [tag]
```!poolview -tag Even
  Address              Size       (Status)      Tag    Type
  ---------------------------------------------------------
  0xffffcc8b460f6580   0x80       (Allocated)   Even   Lfh
  0xffffcc8b460f6700   0x80       (Allocated)   Even   Lfh
  0xffffcc8b460f6c80   0x80       (Allocated)   Even   Lfh
  0xffffcc8b460f6d00   0x80       (Allocated)   Even   Lfh
  0xffffcc8b497fc190   0xa0       (Allocated)   Even   Lfh
  0xffffcc8b497fc9b0   0xa0       (Allocated)   Even   Lfh
  0xffffcc8b4aafedd0   0x60       (Allocated)   Even   Lfh
  0xffffcc8b4aafef50   0x60       (Allocated)   Even   Lfh
  ```
