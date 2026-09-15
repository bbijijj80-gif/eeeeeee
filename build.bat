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
echo Сборка завершена успешно. Исполняемый файл minidb находится в %BUILD_DIR%\Release (или %BUILD_DIR%).
cd ..
endlocal
