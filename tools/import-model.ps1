[CmdletBinding(SupportsShouldProcess=$true)]
param(
 [Parameter(Mandatory=$true)][string]$Source,
 [string]$Destination = '',
 [string]$ManifestPath = ''
)
$ErrorActionPreference='Stop'
$importerDirectory=$PSScriptRoot
if([String]::IsNullOrWhiteSpace($importerDirectory)){$importerDirectory=[IO.Path]::GetDirectoryName($MyInvocation.MyCommand.Path)}
if([String]::IsNullOrWhiteSpace($Destination)){$Destination=Join-Path $importerDirectory '..\dist'}
if([String]::IsNullOrWhiteSpace($ManifestPath)){$ManifestPath=Join-Path $importerDirectory '..\assets988.json'}
function File-Sha256([string]$Path) {
 $stream=[IO.File]::OpenRead($Path);$sha=[Security.Cryptography.SHA256]::Create()
 try {return [BitConverter]::ToString($sha.ComputeHash($stream)).Replace('-','').ToLowerInvariant()}
 finally {$sha.Dispose();$stream.Dispose()}
}
function Assert-PlainPath([string]$Path) {
 $cursor=[IO.Path]::GetFullPath($Path)
 while($cursor) {
  if(Test-Path -LiteralPath $cursor) {
   if((Get-Item -LiteralPath $cursor -Force).Attributes -band [IO.FileAttributes]::ReparsePoint){throw "Links are not accepted: $cursor"}
  }
  $parent=Split-Path -Parent $cursor
  if($parent -eq $cursor){break};$cursor=$parent
 }
}
function Child-Path([string]$Root,[string]$Relative) {
 if([IO.Path]::IsPathRooted($Relative)){throw 'Absolute asset path rejected'}
 $base=[IO.Path]::GetFullPath($Root).TrimEnd('\')
 $candidate=[IO.Path]::GetFullPath((Join-Path $base $Relative))
 if(-not $candidate.StartsWith($base+'\',[StringComparison]::OrdinalIgnoreCase)){throw 'Asset path escapes root'}
 Assert-PlainPath $candidate
 return $candidate
}
$sourceRoot=[IO.Path]::GetFullPath($Source)
$destRoot=[IO.Path]::GetFullPath($Destination)
Assert-PlainPath $sourceRoot;Assert-PlainPath $destRoot;Assert-PlainPath $ManifestPath
$m=Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
if(@($m.files).Count -lt 1 -or @($m.files).Count -gt 2000){throw 'Invalid manifest file count'}
$legacy=Join-Path $sourceRoot 'UserData\runtime-work'
if(Test-Path -LiteralPath $legacy){$graphSource=$legacy;$shaderSource=Join-Path $sourceRoot 'UserData\runtime-shaders'}else{$graphSource=$sourceRoot;$shaderSource=Join-Path $sourceRoot 'shaders'}
$plan=@()
foreach($f in $m.files){$plan+=@{From=(Child-Path $graphSource $f.path);Relative=[string]$f.path;Hash=[string]$f.sha256;Size=[long]$f.size}}
foreach($f in $m.externalShaders.files){if($f.path -ne [IO.Path]::GetFileName($f.path)){throw 'Invalid shader basename'};$plan+=@{From=(Child-Path $shaderSource $f.path);Relative=('shaders\'+$f.path);Hash=[string]$f.sha256;Size=[long]$f.size}}
# Reserve the receipt path and reject aliases before ShouldProcess can write anything.
$modelRoot=Join-Path $destRoot 'model'
$receiptPath=Child-Path $modelRoot 'import-result.json'
$destinations=[System.Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
[long]$total=0
foreach($f in $plan){
 if($f.Size -lt 0 -or $f.Size -gt 64MB -or $f.Hash -notmatch '^[0-9a-fA-F]{64}$'){throw 'Invalid asset metadata'}
 $total+=$f.Size;if($total -gt 256MB){throw 'Asset package exceeds import budget'}
 if(-not(Test-Path -LiteralPath $f.From -PathType Leaf)){throw ('Missing asset: '+$f.Relative)}
 if((Get-Item -LiteralPath $f.From).Length -ne $f.Size -or (File-Sha256 $f.From) -ne $f.Hash){throw ('Asset verification failed: '+$f.Relative)}
 $f.To=Child-Path $modelRoot $f.Relative
 if([StringComparer]::OrdinalIgnoreCase.Equals($f.To,$receiptPath) -or $f.To.StartsWith($receiptPath+'\',[StringComparison]::OrdinalIgnoreCase)){throw 'Asset path conflicts with reserved import receipt'}
 if(-not $destinations.Add($f.To)){throw ('Duplicate canonical asset destination: '+$f.Relative)}
 if($f.To -eq $f.From){throw 'Import destination must differ from source'}
}
if($PSCmdlet.ShouldProcess($destRoot,('Import '+$plan.Count+' verified local model files'))){
 foreach($f in $plan){
  $parent=Split-Path -Parent $f.To;New-Item -ItemType Directory -Path $parent -Force | Out-Null
  $temp=$f.To+'.importing-'+[Guid]::NewGuid().ToString('N')
  try {
   Copy-Item -LiteralPath $f.From -Destination $temp
   if((Get-Item -LiteralPath $temp).Length -ne $f.Size -or (File-Sha256 $temp) -ne $f.Hash){throw 'Source changed during import'}
   Move-Item -LiteralPath $temp -Destination $f.To -Force
  } finally {if(Test-Path -LiteralPath $temp){Remove-Item -LiteralPath $temp -Force}}
 }
 [ordered]@{complete=$true;files=$plan.Count;bytes=$total;manifest_sha256=(File-Sha256 $ManifestPath);scope='Local import and integrity check; not an asset license grant'} | ConvertTo-Json | Set-Content -LiteralPath $receiptPath -Encoding UTF8
}
Write-Output ("Verified {0} files ({1} bytes). No download, DLL execution or system changes." -f $plan.Count,$total)
