@echo off
mkdir build
pushd build
cl -Zi ..\NewFile.cpp User32.lib Gdi32.lib
popd