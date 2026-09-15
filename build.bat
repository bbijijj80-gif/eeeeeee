@echo off
REM Скрипт сборки MiniDB на Windows (требуется CMake и MSVC/MinGW)

setlocal
set BUILD_DIR=build

if not exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
)

cd %BUILD_DIR%

cmake .. -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo Ошибка конфигурации CMake
    exit /b 1
)

cmake --build . --config Release
if errorlevel 1 (
    echo Ошибка сборки проекта
    exit /b 1
)

echo.
echo Сборка завершена успешно.

REM Ищем собранный minidb.exe (MSVC кладёт его в Release\, MinGW — прямо в build\)
set EXE_PATH=
if exist "Release\minidb.exe" set EXE_PATH=%CD%\Release\minidb.exe
if exist "minidb.exe" set EXE_PATH=%CD%\minidb.exe

cd ..

if defined EXE_PATH (
    echo Запуск MiniDB в новом окне командной строки...
    start "MiniDB" cmd /k "%EXE_PATH%"
) else (
    echo Не удалось найти minidb.exe после сборки.
)

endlocal
