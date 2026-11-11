param([Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
function Sha([string]$path){return (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()}
$rootFull=(Resolve-Path -LiteralPath $Root).Path
$src=Join-Path $rootFull 'src'
$back=Join-Path $rootFull '.stage93_source_backup'
$manifestPath=Join-Path $back 'manifest.json'
if(!(Test-Path -LiteralPath $manifestPath -PathType Leaf)){throw 'No Stage93 backup manifest; refusing rollback.'}
$m=Get-Content -LiteralPath $manifestPath -Raw -Encoding UTF8|ConvertFrom-Json
$a=Join-Path $src 'Stage81SelectorWriter.inl';$b=Join-Path $src 'AICommandSiteTrace.cpp'
$c=Join-Path $src 'Stage93F5A68DirectBit3.inl';$s92=Join-Path $src 'Stage92F5990BranchTrace.inl'
if(!(Test-Path -LiteralPath $a) -or !(Test-Path -LiteralPath $b) -or !(Test-Path -LiteralPath $c) -or
   (Sha $a) -ne $m.stage81_stage93_sha -or (Sha $b) -ne $m.final_stage93_sha -or
   (Sha $c) -ne $m.stage93_sha -or (Sha $s92) -ne $m.stage92_verified_sha){
 throw 'Stage93 files have changed; refusing to overwrite newer work.'
}
Copy-Item -LiteralPath (Join-Path $back 'Stage81SelectorWriter.inl') -Destination $a -Force
Copy-Item -LiteralPath (Join-Path $back 'AICommandSiteTrace.cpp') -Destination $b -Force
Remove-Item -LiteralPath $c -Force
if((Sha $a) -ne $m.stage81_original_sha -or (Sha $b) -ne $m.final_original_sha){throw 'Rollback verification failed; backup retained.'}
Remove-Item -LiteralPath $back -Recurse -Force
Write-Host 'Stage93 source rollback complete; Stage92 r3 source restored.'
