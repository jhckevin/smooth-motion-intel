[CmdletBinding()]
param(
 [string]$ImportScript,
 [string]$TestRoot,
 [string]$ReportPath
)
$ErrorActionPreference='Stop'
if(-not $ImportScript){$ImportScript=Join-Path $PSScriptRoot 'import-model.ps1'}
if(-not $TestRoot){$TestRoot=Join-Path $PSScriptRoot '..\build\import-tests'}
$TestRoot=[IO.Path]::GetFullPath($TestRoot)
$runRoot=Join-Path $TestRoot ('case-'+[Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $runRoot -Force | Out-Null
$results=New-Object 'System.Collections.Generic.List[object]'
$utf8=New-Object Text.UTF8Encoding($false)
function Assert-True($condition,[string]$message){if(-not $condition){throw $message}}
function Put([string]$path,[string]$text){
 New-Item -ItemType Directory -Path (Split-Path -Parent $path) -Force | Out-Null
 [IO.File]::WriteAllText($path,$text,$utf8)
}
function Entry([string]$path,[string]$physical){
 return [ordered]@{path=$path;size=(Get-Item -LiteralPath $physical).Length;sha256=(Get-FileHash -LiteralPath $physical -Algorithm SHA256).Hash.ToLowerInvariant()}
}
function Fixture([string]$name,[switch]$Legacy){
 $root=Join-Path $runRoot $name
 $src=Join-Path $root 'source'
 $graph=$src;$shaders=Join-Path $src 'shaders'
 if($Legacy){$graph=Join-Path $src 'UserData\runtime-work';$shaders=Join-Path $src 'UserData\runtime-shaders'}
 Put (Join-Path $graph 'assets\unit.bin') 'public-test-fixture-not-a-model'
 Put (Join-Path $shaders 'unit.spv') 'public-test-fixture-not-a-shader'
 $m=[ordered]@{schema=1;files=@((Entry 'assets\unit.bin' (Join-Path $graph 'assets\unit.bin')));externalShaders=[ordered]@{files=@((Entry 'unit.spv' (Join-Path $shaders 'unit.spv')))}}
 $manifest=Join-Path $root 'manifest.json';Put $manifest ($m|ConvertTo-Json -Depth 8)
 return @{Root=$root;Source=$src;Graph=$graph;Shaders=$shaders;Destination=(Join-Path $root 'out');Manifest=$manifest;Data=$m}
}
function SaveManifest($f){Put $f.Manifest ($f.Data|ConvertTo-Json -Depth 8)}
function Import($f,[switch]$WhatIfOnly){
 if($WhatIfOnly){& $ImportScript -Source $f.Source -Destination $f.Destination -ManifestPath $f.Manifest -WhatIf -Confirm:$false | Out-Null}
 else{& $ImportScript -Source $f.Source -Destination $f.Destination -ManifestPath $f.Manifest -Confirm:$false | Out-Null}
}
function Rejected($f){
 $rejected=$false
 try{Import $f}catch{$rejected=$true}
 Assert-True $rejected 'Import unexpectedly accepted invalid input'
 Assert-True (-not(Test-Path -LiteralPath $f.Destination)) 'Validation rejection wrote destination'
}
function Test([string]$name,[scriptblock]$body){
 try{& $body;$results.Add([ordered]@{case=$name;passed=$true})}
 catch{$results.Add([ordered]@{case=$name;passed=$false;error=$_.Exception.Message})}
}
Test 'whatif-verifies-without-writing' {
 $f=Fixture 'whatif';Import $f -WhatIfOnly
 Assert-True (-not(Test-Path -LiteralPath $f.Destination)) 'WhatIf wrote destination'
}
Test 'default-path-parameters-resolve-at-script-location' {
 $f=Fixture 'defaults'
 $toolDir=Join-Path $f.Root 'tools';New-Item -ItemType Directory -Path $toolDir -Force | Out-Null
 $copy=Join-Path $toolDir 'import-model.ps1';Copy-Item -LiteralPath $ImportScript -Destination $copy
 Copy-Item -LiteralPath $f.Manifest -Destination (Join-Path $f.Root 'assets988.json')
 & $copy -Source $f.Source -WhatIf -Confirm:$false | Out-Null
 Assert-True (-not(Test-Path -LiteralPath (Join-Path $f.Root 'dist'))) 'Default WhatIf wrote output'
}
Test 'junction-source-rejected-without-output' {
 $f=Fixture 'junction'
 $real=Join-Path $f.Root 'plain-assets';New-Item -ItemType Directory -Path $real -Force | Out-Null
 $target=Join-Path $real 'unit.bin';Put $target 'junction-fixture'
 $link=Join-Path $f.Graph 'linked'
 try {
  New-Item -ItemType Junction -Path $link -Target $real | Out-Null
  $f.Data.files=@((Entry 'linked\unit.bin' $target));SaveManifest $f;Rejected $f
 } finally {if(Test-Path -LiteralPath $link){[IO.Directory]::Delete($link)}}
}
Test 'successful-import-verifies-final-files' {
 $f=Fixture 'valid';Import $f
 foreach($entry in $f.Data.files){$p=Join-Path (Join-Path $f.Destination 'model') $entry.path;Assert-True ((Get-FileHash -LiteralPath $p).Hash -eq $entry.sha256) 'Graph output hash differs'}
 $p=Join-Path $f.Destination 'model\shaders\unit.spv'
 Assert-True ((Get-FileHash -LiteralPath $p).Hash -eq $f.Data.externalShaders.files[0].sha256) 'Shader output hash differs'
 $receipt=Get-Content -LiteralPath (Join-Path $f.Destination 'model\import-result.json') -Raw|ConvertFrom-Json
 Assert-True ($receipt.complete -and $receipt.files -eq 2) 'Invalid import receipt'
}
Test 'legacy-private-layout-import' {
 $f=Fixture 'legacy' -Legacy;Import $f
 Assert-True (Test-Path -LiteralPath (Join-Path $f.Destination 'model\assets\unit.bin')) 'Legacy source layout not supported'
}
Test 'hash-mismatch-rejected-before-writes' {
 $f=Fixture 'hash';$f.Data.files[0].sha256=('0'*64);SaveManifest $f;Rejected $f
}
Test 'size-mismatch-rejected-before-writes' {
 $f=Fixture 'size';$f.Data.files[0].size++;SaveManifest $f;Rejected $f
}
Test 'missing-source-rejected-before-writes' {
 $f=Fixture 'missing';$f.Data.files[0].path='assets\absent.bin';SaveManifest $f;Rejected $f
}
Test 'parent-traversal-rejected' {
 $f=Fixture 'traversal';$f.Data.files[0].path='..\outside.bin';SaveManifest $f;Rejected $f
}
Test 'absolute-path-rejected' {
 $f=Fixture 'absolute';$f.Data.files[0].path=(Join-Path $f.Graph 'assets\unit.bin');SaveManifest $f;Rejected $f
}
Test 'shader-subdirectory-rejected' {
 $f=Fixture 'shaderpath';$f.Data.externalShaders.files[0].path='nested\unit.spv';SaveManifest $f;Rejected $f
}
Test 'empty-manifest-rejected' {
 $f=Fixture 'empty';$f.Data.files=@();SaveManifest $f;Rejected $f
}
Test 'file-budget-rejected-before-writes' {
 $f=Fixture 'filebudget';$f.Data.files[0].size=67108865;SaveManifest $f;Rejected $f
}
Test 'existing-output-preserved-on-bad-source' {
 $f=Fixture 'rollback';Put (Join-Path $f.Destination 'model\assets\unit.bin') 'old-valid-local-copy'
 $before=(Get-FileHash -LiteralPath (Join-Path $f.Destination 'model\assets\unit.bin')).Hash
 $f.Data.files[0].sha256=('0'*64);SaveManifest $f;$rejected=$false
 try{Import $f}catch{$rejected=$true}
 Assert-True $rejected 'Bad source unexpectedly imported'
 Assert-True ($before -eq (Get-FileHash -LiteralPath (Join-Path $f.Destination 'model\assets\unit.bin')).Hash) 'Rejected import changed previous destination'
}
Test 'source-receipt-name-collision-rejected' {
 $f=Fixture 'reserved';Put (Join-Path $f.Graph 'import-result.json') '{"fixture":"declared-asset"}'
 $f.Data.files+=@((Entry 'import-result.json' (Join-Path $f.Graph 'import-result.json')));SaveManifest $f
 Rejected $f
}
Test 'duplicate-destination-rejected' {
 $variants=@('assets\unit.bin','ASSETS\UNIT.BIN','assets\.\unit.bin')
 for($i=0;$i -lt $variants.Count;$i++){
  $f=Fixture ('duplicate-'+$i)
  $f.Data.files+=@((Entry $variants[$i] (Join-Path $f.Graph 'assets\unit.bin')))
  SaveManifest $f;Rejected $f
 }
}
$failed=@($results|Where-Object{-not $_.passed}).Count
$report=[ordered]@{schema=1;scope='Remote CPU only; synthetic text fixtures, no GPU, real model, network download or original asset read';import_script_sha256=(Get-FileHash -LiteralPath $ImportScript -Algorithm SHA256).Hash;cases=$results.Count;passed=($results.Count-$failed);failed=$failed;results=$results.ToArray()}
$json=$report|ConvertTo-Json -Depth 8
if($ReportPath){Put ([IO.Path]::GetFullPath($ReportPath)) $json}
$json
# Remove only this test's verified child directory, never the parent.
$resolved=[IO.Path]::GetFullPath($runRoot);$prefix=$TestRoot.TrimEnd('\')+'\'
if(-not $resolved.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase) -or (Split-Path -Leaf $resolved) -notmatch '^case-[0-9a-f]{32}$'){throw 'Test cleanup target is outside its verified workspace'}
Remove-Item -LiteralPath $resolved -Recurse -Force
if($failed){exit 1}
