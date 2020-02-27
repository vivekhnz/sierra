@echo off

set VCVARSALL="C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
set PLATFORM=x64
set SRC_FILE=src\main.cpp
set OBJ_DIR=obj
set BIN_DIR=bin
set EXE_FILENAME=terrain.exe

set INCLUDE_PATH=deps\nuget\glfw.3.3.2\build\native\include
set LIB_PATH=deps\nuget\glfw.3.3.2\build\native\lib\static\v142\x64
set LIBS=glfw3.lib user32.lib gdi32.lib shell32.lib

echo Initializing environment...
call %VCVARSALL% %PLATFORM%

echo Cleaning...
if exist %OBJ_DIR% (rmdir /s /q %OBJ_DIR%)
mkdir %OBJ_DIR%
if exist %BIN_DIR% (rmdir /s /q %BIN_DIR%)
mkdir %BIN_DIR%

echo Restore packages...
nuget restore deps\packages.config

echo Building...
cl /MD /EHsc %SRC_FILE% /Fo%OBJ_DIR%/ /I %INCLUDE_PATH% /link /out:%BIN_DIR%\%EXE_FILENAME% /LIBPATH:%LIB_PATH% %LIBS%

echo Done.