@echo off
chcp 65001 >nul
setlocal

set BUILD_DIR=build

where cmake >nul 2>nul
if errorlevel 1 goto no_cmake

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

cd "%BUILD_DIR%"

REM Pick a generator that matches what is actually installed.
REM Without this, CMake can default to "NMake Makefiles" (Visual Studio
REM toolchain) even when only MinGW is present, which fails immediately.
set GENERATOR=

where g++ >nul 2>nul
if not errorlevel 1 set GENERATOR=MinGW Makefiles

if not defined GENERATOR (
    where cl >nul 2>nul
    if not errorlevel 1 set GENERATOR=NMake Makefiles
)

if not defined GENERATOR goto no_compiler

REM Wipe any cache left by a previous run with a different/failed generator
if exist CMakeCache.txt del /q CMakeCache.txt
if exist CMakeFiles rd /s /q CMakeFiles

echo Configuring project with CMake (generator: %GENERATOR%)...
cmake .. -G "%GENERATOR%" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto cmake_failed

echo.
echo Building project...
cmake --build . --config Release
if errorlevel 1 goto build_failed

echo.
echo Build finished successfully.

set EXE_PATH=
if exist "Release\minidb.exe" set EXE_PATH=%CD%\Release\minidb.exe
if exist "minidb.exe" set EXE_PATH=%CD%\minidb.exe

cd ..

if "%EXE_PATH%"=="" goto exe_not_found

echo Starting MiniDB...
start "MiniDB" "%EXE_PATH%"
goto end

:no_cmake
echo [ERROR] cmake was not found in PATH.
echo Install CMake from https://cmake.org/download/ and make sure it is added to PATH.
goto end

:no_compiler
echo [ERROR] No supported C++ compiler was found in PATH.
echo Install either MinGW-w64 (g++) or open this script from a
echo "Developer Command Prompt for VS" (which provides cl.exe).
cd ..
goto end

:cmake_failed
echo [ERROR] CMake configuration failed. See messages above.
cd ..
goto end

:build_failed
echo [ERROR] Build failed. See messages above.
cd ..
goto end

:exe_not_found
echo [ERROR] minidb.exe was not found after the build.
goto end

:end
echo.
pause
endlocal
