@echo off
setlocal
rem Requires the Windows SDK fxc.exe on PATH. These are first-party capture shaders, not model weights.
where fxc.exe >nul 2>nul
if errorlevel 1 exit /b 2
set "ROOT=%~dp0"
set "DEST=%ROOT%"
if not "%~1"=="" set "DEST=%~1\"
if not exist "%DEST%optimization-20261002\build" mkdir "%DEST%optimization-20261002\build"
if not exist "%DEST%intrinsic-20261006\fp16-next537\candidate994\scaling992" mkdir "%DEST%intrinsic-20261006\fp16-next537\candidate994\scaling992"
fxc /nologo /O3 /T cs_5_0 /E main /Vn captureGpuShader /Fh "%DEST%optimization-20261002\build\capture_gpu_shader.h" "%ROOT%optimization-20261002\src\capture_gpu.hlsl"
if errorlevel 1 exit /b 1
fxc /nologo /O1 /T vs_5_0 /E VS /Vn windowNormalizeVS795 /Fh "%DEST%intrinsic-20261006\fp16-next537\candidate994\scaling992\window_normalize795_vs.h" "%ROOT%intrinsic-20261006\window_normalize795.hlsl"
if errorlevel 1 exit /b 1
fxc /nologo /O1 /T ps_5_0 /E PS /Vn windowNormalizePS795 /Fh "%DEST%intrinsic-20261006\fp16-next537\candidate994\scaling992\window_normalize795_ps.h" "%ROOT%intrinsic-20261006\window_normalize795.hlsl"
exit /b %errorlevel%
