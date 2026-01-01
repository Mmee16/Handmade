@echo off
mkdir build
pushd build
cl -Zi ..\NewFile.cpp User32.lib Gdi32.lib
if %ERRORLEVEL% neq 0 (
    echo "Build failed"
    Timeout /T 5
)
popd