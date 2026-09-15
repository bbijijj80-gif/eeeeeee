@echo off
REM Скрипт сборки MiniDB на Windows (требуется CMake и MSVC/MinGW)
REM Окно не закрывается автоматически — в конце всегда стоит pause,
REM чтобы можно было прочитать сообщения об ошибках при запуске двойным щелчком.

setlocal enabledelayedexpansion
set BUILD_DIR=build

where cmake >nul 2>nul
if errorlevel 1 (
    echo [ОШИБКА] Программа cmake не найдена в PATH.
    echo Установите CMake (https://cmake.org/download/) и убедитесь, что он добавлен в PATH.
    goto :end
)

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

cd "%BUILD_DIR%"

echo Конфигурация проекта через CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo [ОШИБКА] Конфигурация CMake завершилась с ошибкой. См. сообщения выше.
    cd ..
    goto :end
)

echo.
echo Сборка проекта...
cmake --build . --config Release
if errorlevel 1 (
    echo [ОШИБКА] Сборка проекта завершилась с ошибкой. См. сообщения выше.
    cd ..
    goto :end
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
    start "MiniDB" cmd /k "!EXE_PATH!"
) else (
    echo [ОШИБКА] Не удалось найти minidb.exe после сборки.
)

:end
echo.
pause
endlocal
