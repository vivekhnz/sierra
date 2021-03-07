@echo off

set VCVARSALL="C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
set PLATFORM=x64
call %VCVARSALL% %PLATFORM%

set CONFIGURATION=Debug
set INTERMEDIATE_OUTPUT_PATH=..\..\obj\%CONFIGURATION%\%PLATFORM%\Game
set OUTPUT_PATH=..\..\bin\%CONFIGURATION%\%PLATFORM%

if not exist %INTERMEDIATE_OUTPUT_PATH% mkdir %INTERMEDIATE_OUTPUT_PATH%
if not exist %OUTPUT_PATH% mkdir %OUTPUT_PATH%

set COMMON_COMPILER_FLAGS=/nologo /std:c++17 /EHsc /Z7
set COMMON_LINKER_FLAGS=/INCREMENTAL:NO

@REM Compile game DLL

set COMPILER_FLAGS=%COMMON_COMPILER_FLAGS%
set COMPILER_FLAGS=%COMPILER_FLAGS% game.cpp
set COMPILER_FLAGS=%COMPILER_FLAGS% /I ..\..\deps
set COMPILER_FLAGS=%COMPILER_FLAGS% /I ..\..\deps\nuget\glm.0.9.9.700\build\native\include
set COMPILER_FLAGS=%COMPILER_FLAGS% /Fo%INTERMEDIATE_OUTPUT_PATH%/

set LINKER_FLAGS=%COMMON_LINKER_FLAGS%
set LINKER_FLAGS=%LINKER_FLAGS% /DLL
set LINKER_FLAGS=%LINKER_FLAGS% /NOIMPLIB
set LINKER_FLAGS=%LINKER_FLAGS% %OUTPUT_PATH%\Terrain.Engine.lib
set LINKER_FLAGS=%LINKER_FLAGS% /out:%OUTPUT_PATH%\terrain_game.dll
set LINKER_FLAGS=%LINKER_FLAGS% /PDB:%OUTPUT_PATH%\terrain_game.pdb

cl %COMPILER_FLAGS% /link %LINKER_FLAGS%

@REM Compile game executable

set COMPILER_FLAGS=%COMMON_COMPILER_FLAGS%
set COMPILER_FLAGS=%COMPILER_FLAGS% win32_game.cpp
set COMPILER_FLAGS=%COMPILER_FLAGS% /I ..\..\deps
set COMPILER_FLAGS=%COMPILER_FLAGS% /I ..\..\deps\nuget\glm.0.9.9.700\build\native\include
set COMPILER_FLAGS=%COMPILER_FLAGS% /I ..\..\deps\nuget\glfw.3.3.2\build\native\include
set COMPILER_FLAGS=%COMPILER_FLAGS% /Fo%INTERMEDIATE_OUTPUT_PATH%/

set LINKER_FLAGS=%COMMON_LINKER_FLAGS%
set LINKER_FLAGS=%LINKER_FLAGS% /NOIMPLIB
set LINKER_FLAGS=%LINKER_FLAGS% %OUTPUT_PATH%\Terrain.Engine.lib
set LINKER_FLAGS=%LINKER_FLAGS% /out:%OUTPUT_PATH%\win32_terrain.exe
set LINKER_FLAGS=%LINKER_FLAGS% /PDB:%OUTPUT_PATH%\win32_terrain.pdb

cl %COMPILER_FLAGS% /link %LINKER_FLAGS%

@REM Create a junction of the repo into 'C:\temp' so hardcoded executable paths in solution files can point there
set MIRROR_PATH=C:\temp\terrain_mirror
if not exist %MIRROR_PATH% mklink /J %MIRROR_PATH% ..\..\

echo Done