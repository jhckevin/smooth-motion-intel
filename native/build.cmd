@echo off
setlocal
rem Run from an x64 Visual Studio Developer Command Prompt. No installation or global environment changes.
set "ROOT=%~dp0"
if not "%~1"=="" set "VULKAN_HEADERS=%~1"
if not defined VULKAN_HEADERS (
 echo Supply Vulkan-Headers include directory as argument 1 or VULKAN_HEADERS.
 exit /b 2
)
if not exist "%VULKAN_HEADERS%\vulkan\vulkan.h" exit /b 2
if not defined OUT_DIR set "OUT_DIR=%ROOT%..\dist"
if not "%~2"=="" set "OUT_DIR=%~2"
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
set "OBJ=%ROOT%build"
if not exist "%OBJ%" mkdir "%OBJ%"
pushd "%ROOT%intrinsic-20261006"
set "C=fp16-next537\candidate994"
cl /nologo /O2 /MT /Z7 /I. /I"%VULKAN_HEADERS%" /I"%ROOT%optimization-20261002\src" /c /Fo:"%OBJ%\app_graph994.obj" app_graph994.c
if errorlevel 1 goto fail
for %%S in (resolution_budget989 bounded_stdio989 diagnostic994) do (
 cl /nologo /O2 /MT /Z7 /EHsc /std:c++20 /c /Fo:"%OBJ%\%%S.obj" "%C%\%%S.cpp"
 if errorlevel 1 goto fail
)
cl /nologo /O2 /MT /Z7 /EHsc /std:c++20 /utf-8 /I"%VULKAN_HEADERS%" /I"%ROOT%optimization-20261002\src" /Fo:"%OBJ%\app_ui994.obj" app_ui994.cpp "%OBJ%\bounded_stdio989.obj" "%OBJ%\resolution_budget989.obj" "%OBJ%\app_graph994.obj" "%OBJ%\diagnostic994.obj" /Fe:"%OUT_DIR%\SmoothMotion-Engine.exe" /link /DEBUG:FULL /PDBALTPATH:%%_PDB%% /PDB:"%OUT_DIR%\SmoothMotion-Engine.pdb" /OPT:REF /OPT:ICF /SUBSYSTEM:WINDOWS user32.lib gdi32.lib d3d11.lib dxgi.lib dcomp.lib windowsapp.lib avrt.lib dwmapi.lib advapi32.lib shell32.lib
if errorlevel 1 goto fail
cl /nologo /O2 /MT /Z7 /EHsc /std:c++20 "%C%\diagnostic994_cli.cpp" "%OBJ%\diagnostic994.obj" /Fo:"%OBJ%\diagnostic994_cli.obj" /Fe:"%OUT_DIR%\diag-export994.exe" /link /DEBUG:FULL /PDBALTPATH:%%_PDB%% /PDB:"%OUT_DIR%\diag-export994.pdb"
if errorlevel 1 goto fail
cl /nologo /O2 /MT /TC "%ROOT%tests\capability995_test.c" /Fo:"%OBJ%\capability995_test.obj" /Fe:"%OUT_DIR%\capability995-test.exe"
if errorlevel 1 goto fail
popd
exit /b 0
:fail
popd
exit /b 1
