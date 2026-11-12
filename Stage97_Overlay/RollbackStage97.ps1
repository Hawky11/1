# Stage97 source-only rollback to SHA-verified Stage96 r1.
param([Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
function Need([bool]$yes,[string]$msg){if(-not $yes){throw $msg}}
function Sha([string]$path){return (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()}
$rootFull=(Resolve-Path -LiteralPath $Root).Path
$dir=Join-Path $rootFull '.stage97_source_backup'
$mPath=Join-Path $dir 'manifest.json'
$old81=Join-Path $dir 'Stage81SelectorWriter.inl'
$src=Join-Path $rootFull 'src'
$stage81=Join-Path $src 'Stage81SelectorWriter.inl'
$stage97=Join-Path $src 'Stage97InheritedEsiTrace.inl'
Need (!(Get-Process AALib -ErrorAction SilentlyContinue)) 'Close AALib.exe before rollback.'
Need ((Test-Path -LiteralPath $mPath -PathType Leaf) -and
      (Test-Path -LiteralPath $old81 -PathType Leaf) -and
      (Test-Path -LiteralPath $stage81 -PathType Leaf) -and
      (Test-Path -LiteralPath $stage97 -PathType Leaf)) 'Stage97 rollback files incomplete; refusing.'
$m=Get-Content -LiteralPath $mPath -Raw -Encoding UTF8|ConvertFrom-Json
Need ((Sha $stage81) -eq $m.installed_stage81_sha -and
      (Sha $stage97) -eq $m.installed_stage97_sha -and
      (Sha $old81) -eq $m.parent_stage81_sha) 'Source/backup modified. Manual audit required.'
$bytes81=[System.IO.File]::ReadAllBytes($stage81)
$bytes97=[System.IO.File]::ReadAllBytes($stage97)
try {
 [System.IO.File]::WriteAllBytes($stage81,[System.IO.File]::ReadAllBytes($old81))
 Need ((Sha $stage81) -eq $m.parent_stage81_sha) 'Stage96 restoration SHA failed.'
 Remove-Item -LiteralPath $stage97 -Force
 Remove-Item -LiteralPath $dir -Recurse -Force
 Write-Host 'STAGE97 ROLLED BACK TO TESTED STAGE96 SOURCE. Rebuild x86 before running AALib.'
} catch {
 [System.IO.File]::WriteAllBytes($stage81,$bytes81)
 if(!(Test-Path -LiteralPath $stage97)){[System.IO.File]::WriteAllBytes($stage97,$bytes97)}
 throw
}
