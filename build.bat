@echo off
setlocal enableextensions enabledelayedexpansion

:: Set up the environment
cd /d "%~dp0"
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

:: Create the build directory and navigate to it
mkdir build
cd build

:: Run the cmake command
cmake -G "Ninja" -D CMAKE_BUILD_TYPE=Release ..
if errorlevel 1 goto error

:: Run the ninja build
ninja
if errorlevel 1 goto error


copy _deps\oidnzip-src\oidn-1.4.3.x64.vc14.windows\bin\OpenImageDenoise.dll .\bin\
if errorlevel 1 goto error

copy _deps\oidnzip-src\oidn-1.4.3.x64.vc14.windows\bin\tbb12.dll .\bin\
if errorlevel 1 goto error

:: Print success message
echo|set /p="SUCCESS" <nul
color 0A
goto End

:error
echo|set /p="ERROR" <nul
color 0C
goto End

:End
pause
