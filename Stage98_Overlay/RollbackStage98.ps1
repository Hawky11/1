# Stage98 r1 — restore exact tested Stage97 sources. Game must be closed.
param([Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
function Need([bool]$ok,[string]$msg){if(-not $ok){throw $msg}}
function Sha([string]$f){(Get-FileHash -LiteralPath $f -Algorithm SHA256).Hash.ToLowerInvariant()}
$rootFull=(Resolve-Path -LiteralPath $Root).Path
$dir=Join-Path $rootFull '.stage98_source_backup'
$mPath=Join-Path $dir 'manifest.json'
$old81=Join-Path $dir 'Stage81SelectorWriter.inl'
$stage81=Join-Path $rootFull 'src/Stage81SelectorWriter.inl'
$stage98=Join-Path $rootFull 'src/Stage98CommandRouteDecision.inl'
$stage97=Join-Path $rootFull 'src/Stage97InheritedEsiTrace.inl'
Need (!(Get-Process AALib -ErrorAction SilentlyContinue)) 'Close AALib.exe before Stage98 rollback.'
Need ((Test-Path -LiteralPath $mPath -PathType Leaf) -and
      (Test-Path -LiteralPath $old81 -PathType Leaf) -and
      (Test-Path -LiteralPath $stage81 -PathType Leaf) -and
      (Test-Path -LiteralPath $stage97 -PathType Leaf) -and
      (Test-Path -LiteralPath $stage98 -PathType Leaf)) 'Stage98 rollback materials incomplete; refusing.'
$m=Get-Content -LiteralPath $mPath -Raw -Encoding UTF8|ConvertFrom-Json
Need (($m.stage -eq 'Stage98 r1') -and
      (Sha $stage81) -eq $m.installed_stage81_sha -and
      (Sha $stage98) -eq $m.installed_stage98_sha -and
      (Sha $old81) -eq $m.parent_stage81_sha -and
      (Sha $stage97) -eq $m.parent_stage97_sha) 'Stage98 source/backup SHA differs; manual audit necessary.'
$bytes81=[System.IO.File]::ReadAllBytes($stage81)
$bytes98=[System.IO.File]::ReadAllBytes($stage98)
try {
 [System.IO.File]::WriteAllBytes($stage81,[System.IO.File]::ReadAllBytes($old81))
 Need ((Sha $stage81) -eq $m.parent_stage81_sha) 'Stage97 restoration SHA verification failed.'
 Remove-Item -LiteralPath $stage98 -Force
 Remove-Item -LiteralPath $dir -Recurse -Force
 Write-Host 'STAGE98 ROLLED BACK TO VERIFIED STAGE97 SOURCE. Rebuild x86 or restore saved Stage97 DLL.'
} catch {
 [System.IO.File]::WriteAllBytes($stage81,$bytes81)
 if(!(Test-Path -LiteralPath $stage98)){[System.IO.File]::WriteAllBytes($stage98,$bytes98)}
 throw
}
