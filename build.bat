@echo off

set VCVARSALL="C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
set PLATFORM=x64
set SRC_FILE=src\main.cpp
set OBJ_DIR=src\obj
set BIN_DIR=src\bin
set EXE_FILENAME=terrain.exe

echo Initializing environment...
call %VCVARSALL% %PLATFORM%

echo Cleaning...
if exist %OBJ_DIR% (rmdir /s /q %OBJ_DIR%)
mkdir %OBJ_DIR%
if exist %BIN_DIR% (rmdir /s /q %BIN_DIR%)
mkdir %BIN_DIR%

echo Compiling...
cl /EHsc %SRC_FILE% /Fo%OBJ_DIR%/ /link /out:%BIN_DIR%\%EXE_FILENAME%

echo Done.