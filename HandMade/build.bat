@echo off
mkdir ..\build
pushd ..\build
cl > nul
if %ERRORLEVEL% neq 0 (
    echo "cl command failed, executing setup_build.bat"
    call "..\set_debug.bat" > nul
)
cl -Zi ..\HandMade\NewFile.cpp User32.lib Gdi32.lib
if %ERRORLEVEL% neq 0 (
    echo "Build failed"
    Timeout /T 5
)
popd