@echo off

set VCVARSALL="C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
set PLATFORM=x64
set CONFIGURATION=Debug

set INTERMEDIATE_OUTPUT_PATH=..\..\obj\%CONFIGURATION%\%PLATFORM%\Game
set OUTPUT_PATH=..\..\bin\%CONFIGURATION%\%PLATFORM%

set INCLUDES=
set INCLUDES=%INCLUDES% /I ..\..\deps
set INCLUDES=%INCLUDES% /I ..\..\deps\nuget\glm.0.9.9.700\build\native\include
set INCLUDES=%INCLUDES% /I ..\..\deps\nuget\glfw.3.3.2\build\native\include

set COMPILER_FLAGS=/nologo /std:c++17 /EHsc /Z7 /Fo%INTERMEDIATE_OUTPUT_PATH%/
set LINKER_FLAGS=/NOIMPLIB /INCREMENTAL:NO

if not exist %INTERMEDIATE_OUTPUT_PATH% mkdir %INTERMEDIATE_OUTPUT_PATH%
if not exist %OUTPUT_PATH% mkdir %OUTPUT_PATH%

call %VCVARSALL% %PLATFORM%
cl %COMPILER_FLAGS% %INCLUDES% win32_game.cpp game.cpp /link %LINKER_FLAGS% %OUTPUT_PATH%\Terrain.Engine.lib /out:%OUTPUT_PATH%\win32_terrain.exe /PDB:%OUTPUT_PATH%\win32_terrain.pdb

@REM Create a junction of the repo into 'C:\temp' so hardcoded executable paths in solution files can point there
set MIRROR_PATH=C:\temp\terrain_mirror
if not exist %MIRROR_PATH% mklink /J %MIRROR_PATH% ..\..\

echo Done