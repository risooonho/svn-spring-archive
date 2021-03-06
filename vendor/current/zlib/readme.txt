dll32\zlibwapi.dll :    The 32 bits DLL of ZLib 1.22
dll32\zlibwapi.lib :    The 32 bits import library for the DLL of ZLib 1.22
static32\zlibstat.lib : The 32 bits statis library of zLib 1.22 for Visual C++
dll32\zlib_bor.lib :    The 32 bits import library for the DLL of ZLib 1.22 for Borland C++

The zlib.h must be the version from zlib122.zip / zlib-1.2.2.tar.gz

Note :
  The DLL, named zlibwapi.dll, is compatible to the old zlib.dll built
  by Gilles Vollant from the zlib 1.2.x sources, and distributed at
    http://www.winimage.com/zLibDll
  It uses the WINAPI calling convention for the exported functions, and
  includes the minizip functionality. If your application needs that
  particular build of zlib.dll, you can rename zlibwapi.dll to zlib.dll.



If you want rebuild these file, for your information :

Building instructions for the DLL versions of Zlib 1.22
=======================================================

This directory contains projects that build zlib and minizip using
Microsoft Visual C++ 7.0/7.1.

You don't need to build these projects yourself. You can download the
binaries from:
  http://www.winimage.com/zLibDll

More information can be found at this site.


Build instructions
------------------
- Unzip zlib*.zip and copy the files from contrib\vstudio\vc7,
  from contrib\masmx86 and from contrib\minizip into the same
  directory.
- Download the crtdll library from
    http://www.winimage.com/zLibDll/crtdll.zip
  Unzip crtdll.zip to extract crtdll.lib.
- If you are using x86, use the Release target.
- Open zlibvc.sln with Microsoft Visual C++ 7.0 or 7.1
  (Visual Studio .Net 2002 or 2003).


Important
---------
- To use zlibwapi.dll in your application, you must define the
  macro ZLIB_WINAPI when compiling your application's source files.


Additional notes
----------------
- This DLL, named zlibwapi.dll, is compatible to the old zlib.dll built
  by Gilles Vollant from the zlib 1.1.x sources, and distributed at
    http://www.winimage.com/zLibDll
  It uses the WINAPI calling convention for the exported functions, and
  includes the minizip functionality. If your application needs that
  particular build of zlib.dll, you can rename zlibwapi.dll to zlib.dll.

- The new DLL was renamed because there exist several incompatible
  versions of zlib.dll on the Internet.

- There is also an official DLL build of zlib, named zlib1.dll. This one
  is exporting the functions using the CDECL convention. See the file
  win32\DLL_FAQ.txt found in this zlib distribution.

- There used to be a ZLIB_DLL macro in zlib 1.1.x, but now this symbol
  has a slightly different effect. To avoid compatibility problems, do
  not define it here.


Gilles Vollant
info@winimage.com
