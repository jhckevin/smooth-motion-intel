param([string]$Executable=(Join-Path $PSScriptRoot '..\dist\SmoothMotion-Engine.exe'))
$ErrorActionPreference='Stop'
$exe=[IO.Path]::GetFullPath($Executable)
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\build\native-tests'))
New-Item -ItemType Directory -Path $root -Force | Out-Null
function Run-OwnCpuTest([string]$File,[string]$Arguments,[string]$Name) {
 $dir=Join-Path $root $Name;New-Item -ItemType Directory -Path $dir -Force | Out-Null
 $info=New-Object Diagnostics.ProcessStartInfo
 $info.FileName=$File;$info.Arguments=$Arguments;$info.WorkingDirectory=$dir
 $info.UseShellExecute=$false;$info.CreateNoWindow=$true
 $info.RedirectStandardOutput=$true;$info.RedirectStandardError=$true
 $child=New-Object Diagnostics.Process;$child.StartInfo=$info
 $timer=[Diagnostics.Stopwatch]::StartNew()
 try {
  if(-not $child.Start()){throw ('Could not start '+$Name)}
  $out=$child.StandardOutput.ReadToEndAsync();$err=$child.StandardError.ReadToEndAsync()
  if(-not $child.WaitForExit(15000)){$child.Kill();$child.WaitForExit();throw ('Own CPU test exceeded 15 seconds: '+$Name)}
  [IO.File]::WriteAllText((Join-Path $dir 'stdout.txt'),$out.Result)
  [IO.File]::WriteAllText((Join-Path $dir 'stderr.txt'),$err.Result)
  $code=$child.ExitCode
  if($code -ne 0){$hex=[BitConverter]::ToUInt32([BitConverter]::GetBytes([int]$code),0).ToString('X8');Write-Host $err.Result;throw ('CPU test failed: '+$Name+', exit '+$code+' (0x'+$hex+')')}
  return @{case=$Name;exit_code=$code;elapsed_ms=$timer.Elapsed.TotalMilliseconds}
 } finally { $child.Dispose() }
}
$results=@()
$results+=Run-OwnCpuTest (Join-Path ([IO.Path]::GetDirectoryName($exe)) 'capability995-test.exe') '' 'capability995-test'
foreach($case in @('--diagnostics-selftest994','--diagnostic-integration-selftest994','--output-geometry-selftest993','--telemetry-lifetime-selftest993','--foreground-wait-selftest992','--window-state-selftest990','--capture-contract-selftest','--host-selftest','--hud-selftest')){
 $results+=Run-OwnCpuTest $exe $case $case.TrimStart('-')
}
@{pass=$true;gpu_used=$false;cases=$results} | ConvertTo-Json -Depth 5 | Tee-Object -FilePath (Join-Path $root 'results.json')
