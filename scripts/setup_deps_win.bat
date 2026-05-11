@echo off
REM Copyright (c) 2026 MeshForge Project
REM SPDX-License-Identifier: AGPL-3.0-or-later
REM
REM Build all vendored dependencies for MeshForge on Windows.
REM Run once on a fresh machine. Subsequent builds reuse build outputs.
REM
REM Prerequisites (must be installed before running this script):
REM   - Visual Studio 2022 Community with:
REM       "Desktop development with C++" workload
REM       Windows 10 SDK 10.0.22621
REM   - Strawberry Perl (https://strawberryperl.com) added to PATH
REM       Required for OpenSSL build.
REM   - Git for Windows (https://gitforwindows.org)
REM   - CMake >= 3.21 added to PATH
REM       Available via Visual Studio installer or https://cmake.org
REM
REM Usage:
REM   scripts\setup_deps_win.bat
REM
REM Output: deps\build_win\destdir\usr\local\
REM Expected build time: 90-120 min on first run. Disk: ~15 GB.

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set REPO_ROOT=%SCRIPT_DIR%..
set DEPS_BUILD_DIR=%REPO_ROOT%\deps\build_win

echo =^> Checking prerequisites...

where cmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: cmake not found on PATH.
    echo        Install CMake ^>= 3.21 and add it to PATH.
    exit /b 1
)

where perl >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: perl not found on PATH.
    echo        Install Strawberry Perl and add it to PATH.
    exit /b 1
)

echo =^> Configuring deps ^(output: %DEPS_BUILD_DIR%^)
cmake -S "%REPO_ROOT%\deps" ^
      -B "%DEPS_BUILD_DIR%" ^
      -G "Visual Studio 17 2022" ^
      -A x64

if %ERRORLEVEL% neq 0 (
    echo FAILED: CMake configure step.
    exit /b 1
)

echo =^> Building deps ^(this takes 90-120 min on first run^)...
cmake --build "%DEPS_BUILD_DIR%" --config Release -- /m

if %ERRORLEVEL% neq 0 (
    echo FAILED: Build step.
    exit /b 1
)

echo.
echo =^> Dependencies built successfully in %DEPS_BUILD_DIR%
echo =^> To build MeshForge, run:
echo       cmake -B build -G "Visual Studio 17 2022" -A x64 ^
echo             -DDEPS_DIR="%DEPS_BUILD_DIR%\destdir\usr\local" ^
echo             -DSLIC3R_NETWORK=OFF
echo       cmake --build build --config Release

endlocal
