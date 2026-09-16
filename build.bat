@echo off
chcp 65001 >nul
setlocal

set BUILD_DIR=build

where cmake >nul 2>nul
if errorlevel 1 goto no_cmake

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

cd "%BUILD_DIR%"

echo Configuring project with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release
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
