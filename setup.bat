

@echo off
setlocal enabledelayedexpansion

:: Get script directory
set SCRIPT_DIR=%~dp0
set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

:: Check if architecture was provided
if "%~1"=="" (
    echo Usage: %~nx0 ^<architecture^>
    echo - architectures:
    echo       - x86_64
    echo       - arm64
    exit /b 1
)

set ARCH=%~1

:: ========== HANDLE PARAMETER ==========
if "%ARCH%"=="arm64" (
    set "BASE_PATH_64BITS=%SCRIPT_DIR%\Liho\src\main\libs\arm64-v8a"
    set "BASE_PATH_32BITS=%SCRIPT_DIR%\Liho\src\main\libs\armeabi-v7a"

    set "SOURCE_LIHO_64BITS=arm64-v8a.so"
    set "SOURCE_LIHO_32BITS=armeabi-v7a.so"
) else if "%ARCH%"=="x86_64" (
    set "BASE_PATH_64BITS=%SCRIPT_DIR%\Liho\src\main\libs\x86_64"
    set "BASE_PATH_32BITS=%SCRIPT_DIR%\Liho\src\main\libs\x86"

    set "SOURCE_LIHO_64BITS=x86_64.so"
    set "SOURCE_LIHO_32BITS=x86.so"
) else (
    echo Invalid architecture: %ARCH%
    exit /b 1
)

:: ========== BUILD .so ==========
echo Running ndk-build in: %SCRIPT_DIR%\Liho\src\main
pushd "%SCRIPT_DIR%\Liho\src\main" >nul
call ndk-build
popd >nul

:: ========== COPY .SO TO MODULE ==========
set "DESTINATION_MODULE_PATH=%SCRIPT_DIR%\LihoMagiskModule\zygisk"

call :copy_if_exists "%BASE_PATH_64BITS%" "%SOURCE_LIHO_64BITS%" "%DESTINATION_MODULE_PATH%"
call :copy_if_exists "%BASE_PATH_32BITS%" "%SOURCE_LIHO_32BITS%" "%DESTINATION_MODULE_PATH%"

:: ========== ZIP MODULE ==========
echo.
echo Zipping Liho Magisk Module...

set "MODULE_DIR=%SCRIPT_DIR%\LihoMagiskModule"
set "ZIP_PATH=%SCRIPT_DIR%\LihoMagiskModule.zip"

if exist "%ZIP_PATH%" del /f "%ZIP_PATH%"
pushd "%MODULE_DIR%" >nul
powershell -command "Compress-Archive -Path * -DestinationPath '%ZIP_PATH%'"
popd >nul

echo Zip created at: %ZIP_PATH%
exit /b 0

:copy_if_exists
set "SOURCE_DIR=%~1"
set "FILENAME=%~2"
set "DEST_DIR=%~3"

echo Copying %FILENAME% to %DEST_DIR%...
if exist "%SOURCE_DIR%\%FILENAME%" (
    if not exist "%DEST_DIR%" mkdir "%DEST_DIR%"
    copy /Y "%SOURCE_DIR%\%FILENAME%" "%DEST_DIR%\"
) else (
    echo Source file %SOURCE_DIR%\%FILENAME% not found!
)
exit /b