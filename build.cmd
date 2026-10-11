@echo off
setlocal
cd /d "%~dp0"
if not defined VULKAN_HEADERS if defined VULKAN_SDK set "VULKAN_HEADERS=%VULKAN_SDK%\Include"
if not defined VULKAN_HEADERS (
 echo Set VULKAN_HEADERS to a Vulkan-Headers include directory. See docs\BUILD.zh-CN.md.
 exit /b 2
)
where cl >nul 2>nul
if errorlevel 1 (
 for /f "usebackq tokens=*" %%I in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VS_ROOT=%%I"
)
if defined VS_ROOT call "%VS_ROOT%\VC\Auxiliary\Build\vcvars64.bat"
where cl >nul 2>nul
if errorlevel 1 exit /b 2
call native\build.cmd "%VULKAN_HEADERS%" "%CD%\dist"
if errorlevel 1 exit /b 1
call tools\build-desktop.cmd
exit /b %ERRORLEVEL%
