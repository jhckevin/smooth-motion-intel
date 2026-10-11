@echo off
setlocal
set "ROOT=%~dp0.."
set "OUT=%ROOT%\dist"
if not exist "%OUT%" mkdir "%OUT%"
set "CSC=%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\csc.exe"
set "WPF=%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\WPF"
if not exist "%CSC%" exit /b 2
pushd "%ROOT%\desktop"
"%CSC%" /nologo /target:winexe /platform:x64 /optimize+ /debug:pdbonly /out:"%OUT%\SmoothMotion.exe" /win32manifest:src\app.manifest /win32icon:assets\AppIcon989.ico /resource:assets\AppIcon989.ico,AppIcon989.ico /resource:src\Theme988.xaml,Theme988.xaml /r:"%WPF%\PresentationFramework.dll" /r:"%WPF%\PresentationCore.dll" /r:"%WPF%\WindowsBase.dll" /r:System.Xaml.dll /r:System.Runtime.Serialization.dll /r:System.Web.Extensions.dll /r:System.Windows.Forms.dll /r:System.Drawing.dll /r:System.IO.Compression.dll /r:System.IO.Compression.FileSystem.dll src\Desktop988.cs src\EngineHost988.cs src\ProductModel988.cs src\EnginePreset988.cs src\VectorIcons989.cs src\Diagnostics994.cs
set "RESULT=%ERRORLEVEL%"
popd
if not "%RESULT%"=="0" exit /b %RESULT%
copy /y "%ROOT%\assets988.json" "%OUT%\assets988.json" >nul
copy /y "%ROOT%\docs\USER-GUIDE.zh-CN.md" "%OUT%\USER-GUIDE.zh-CN.md" >nul
copy /y "%ROOT%\docs\DIAGNOSTICS.zh-CN.md" "%OUT%\DIAGNOSTICS.zh-CN.md" >nul
copy /y "%ROOT%\docs\ISSUE-TEMPLATE.zh-CN.md" "%OUT%\ISSUE-TEMPLATE.zh-CN.md" >nul
exit /b 0
