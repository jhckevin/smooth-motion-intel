@echo off
setlocal
set "ROOT=%~dp0.."
set "OUT=%ROOT%\build\desktop-tests"
if not exist "%OUT%" mkdir "%OUT%"
set "CSC=%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\csc.exe"
set "WPF=%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\WPF"
pushd "%ROOT%\desktop"
"%CSC%" /nologo /target:exe /platform:x64 /optimize+ /out:"%OUT%\EnginePresetTests.exe" /r:System.Web.Extensions.dll src\EnginePreset988.cs src\ProductModel988.cs tests\EnginePreset988Tests.cs
if errorlevel 1 goto fail
"%OUT%\EnginePresetTests.exe"
if errorlevel 1 goto fail
"%CSC%" /nologo /target:exe /platform:x64 /optimize+ /out:"%OUT%\DiagnosticsTests.exe" /r:System.Web.Extensions.dll /r:System.IO.Compression.dll /r:System.IO.Compression.FileSystem.dll src\Diagnostics994.cs tests\Diagnostics994Tests.cs
if errorlevel 1 goto fail
"%OUT%\DiagnosticsTests.exe" "%OUT%\fixtures"
if errorlevel 1 goto fail
"%CSC%" /nologo /target:exe /platform:x64 /optimize+ /out:"%OUT%\EngineHostTests.exe" /r:System.Web.Extensions.dll /r:System.IO.Compression.dll /r:System.IO.Compression.FileSystem.dll src\EngineHost988.cs src\EnginePreset988.cs src\ProductModel988.cs src\Diagnostics994.cs tests\EngineHost988Tests.cs
if errorlevel 1 goto fail
"%OUT%\EngineHostTests.exe"
if errorlevel 1 goto fail
popd
exit /b 0
:fail
popd
exit /b 1
