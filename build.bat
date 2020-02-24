@echo off
echo Initializing environment...
set vcvarsall="C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
call %vcvarsall% x64

echo Cleaning...
if exist src\obj (rmdir /s /q src\obj)
mkdir src\obj
if exist src\bin (rmdir /s /q src\bin)
mkdir src\bin

echo Compiling...
cl /EHsc src\main.cpp /Fosrc/obj/ /link /out:src\bin\terrain.exe

echo Done.