@echo off
setlocal enabledelayedexpansion

REM Set UTF-8 encoding for proper character rendering
chcp 65001 >nul

REM Ensure script runs from the repository root directory
cd /d "%~dp0"

echo ===============================================================================
echo            Tibia CipSoft 7.72 Server - Windows Build Script
echo ===============================================================================
echo.

REM Handle optional clean / rebuild command-line arguments
if /i "%~1"=="clean" goto do_clean
if /i "%~1"=="rebuild" (
    call :do_clean
    goto do_configure
)
goto find_cmake

:do_clean
echo [*] Cleaning build directory...
if exist "build" (
    rmdir /s /q "build"
    echo [OK] Build folder removed successfully.
) else (
    echo [*] No build folder found, nothing to clean.
)
if /i "%~1"=="clean" (
    echo.
    echo [*] Clean completed.
    pause
    exit /b 0
)
goto :eof

:find_cmake
echo [*] Locating CMake and build tools...
set "CMAKE_CMD="

REM 1. Try CMake in system PATH
where cmake >nul 2>nul
if %ERRORLEVEL% equ 0 (
    set "CMAKE_CMD=cmake"
    echo [OK] CMake detected in system PATH.
    goto do_configure
)

REM 2. Try locating CMake via vswhere (Visual Studio 2019 / 2022 / 2026)
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "!VSWHERE!" (
    for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VS_PATH=%%i"
    )
    if defined VS_PATH (
        if exist "!VS_PATH!\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
            set "CMAKE_CMD=!VS_PATH!\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
            echo [OK] CMake detected via Visual Studio: "!CMAKE_CMD!"
            goto do_configure
        )
    )
)

REM 3. Try standard standalone CMake installation paths
if exist "%ProgramFiles%\CMake\bin\cmake.exe" (
    set "CMAKE_CMD=%ProgramFiles%\CMake\bin\cmake.exe"
    echo [OK] CMake detected at: "%CMAKE_CMD%"
    goto do_configure
)
if exist "%ProgramFiles(x86)%\CMake\bin\cmake.exe" (
    set "CMAKE_CMD=%ProgramFiles(x86)%\CMake\bin\cmake.exe"
    echo [OK] CMake detected at: "%CMAKE_CMD%"
    goto do_configure
)

echo [ERROR] CMake was not found on your system!
echo Please install Visual Studio with "Desktop development with C++" or install CMake from: https://cmake.org/download/
echo.
pause
exit /b 1

:do_configure
if not exist "build\CMakeCache.txt" (
    echo.
    echo [*] Generating CMake project configuration [x64 Release]...
    "!CMAKE_CMD!" -B build -A x64
    if !ERRORLEVEL! neq 0 (
        echo.
        echo [ERROR] Failed to configure CMake project.
        pause
        exit /b !ERRORLEVEL!
    )
)

echo.
echo [*] Building executable in Release mode [x64]...
"!CMAKE_CMD!" --build build --config Release --parallel
if !ERRORLEVEL! neq 0 (
    echo.
    echo [ERROR] Build failed. Please check compiler errors above.
    pause
    exit /b !ERRORLEVEL!
)

echo.
echo [*] Copying binaries to repository root...

if exist "build\Release\tibia-server.exe" (
    copy /y "build\Release\tibia-server.exe" ".\tibia-server.exe" >nul
    if !ERRORLEVEL! equ 0 (
        echo [OK] Executable updated in root: tibia-server.exe
    ) else (
        echo [WARNING] Could not overwrite tibia-server.exe [server might be currently running].
    )
) else (
    echo [ERROR] build\Release\tibia-server.exe not found after build.
    pause
    exit /b 1
)

REM Ensure required dynamic runtime libraries (DLLs) are present in the root folder
for %%F in (libmariadb.dll mpir.dll z.dll) do (
    if not exist ".\%%F" (
        if exist "build\Release\%%F" (
            copy /y "build\Release\%%F" ".\%%F" >nul
            echo [OK] Copied runtime DLL: %%F
        )
    )
)

echo.
echo ===============================================================================
echo [SUCCESS] Build completed successfully!
echo Executable ready at: %~dp0tibia-server.exe
echo ===============================================================================
echo.

pause
exit /b 0
