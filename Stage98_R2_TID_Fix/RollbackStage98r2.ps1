# Stage98 r2 -> original Stage98 r1 SOURCE rollback; do not run while AALib.exe is open.
param([Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
function Need([bool]$ok,[string]$why) { if (-not $ok) { throw $why } }
function Sha([string]$p) { (Get-FileHash -LiteralPath $p -Algorithm SHA256).Hash.ToLowerInvariant() }
$rootFull = (Resolve-Path -LiteralPath $Root).Path
$source = Join-Path $rootFull 'src/Stage98CommandRouteDecision.inl'
$stage81 = Join-Path $rootFull 'src/Stage81SelectorWriter.inl'
$stage97 = Join-Path $rootFull 'src/Stage97InheritedEsiTrace.inl'
$manifestPath = Join-Path $rootFull '.stage98_source_backup/manifest.json'
$backup = Join-Path $rootFull '.stage98_r2_patch_backup'
$oldSource = Join-Path $backup 'Stage98CommandRouteDecision.inl'
$oldManifest = Join-Path $backup 'manifest.json'
$oldSha = 'a2548073f73fd7967ecece70cb278b32c162c0dfd07d9f63be7f132f628d5ba1'
$newSha = 'a56fe5692b3284fae37822e6f08438e3a32a800accbad5ff4c72d36ea97a1591'
$stage81Sha = '12f2afbee4cf427f2b13cce7e9baeb7d525fc4e4b23bc42cc1c3d0a86463a192'
$stage97Sha = 'f368037433b3baaec4f322428875edbbb8f3c12e1c89486c5d9ba8ff49c2f78c'
Need (!(Get-Process AALib -ErrorAction SilentlyContinue)) 'Close AALib.exe before r2 rollback.'
foreach($p in @($source,$stage81,$stage97,$manifestPath,$oldSource,$oldManifest)) {
 Need (Test-Path -LiteralPath $p -PathType Leaf) ('Missing rollback prerequisite: '+$p)
}
Need ((Sha $stage81) -eq $stage81Sha -and (Sha $stage97) -eq $stage97Sha) 'Existing Stage81/97 source changed.'
Need ((Sha $source) -eq $newSha -and (Sha $oldSource) -eq $oldSha) 'Stage98 source/backup differs; no rollback.'
$m=Get-Content -LiteralPath $manifestPath -Raw -Encoding UTF8 | ConvertFrom-Json
$old=Get-Content -LiteralPath $oldManifest -Raw -Encoding UTF8 | ConvertFrom-Json
Need (($m.stage -eq 'Stage98 r1') -and ($m.installed_stage98_sha -eq $newSha) -and
      ($m.source_revision -eq 'Stage98 r2 TID logging fix') -and
      ($old.stage -eq 'Stage98 r1') -and ($old.installed_stage98_sha -eq $oldSha) -and
      ($old.installed_stage81_sha -eq $stage81Sha)) 'Stage98 manifest/backup revision differs.'
$bytesNew=[System.IO.File]::ReadAllBytes($source)
$bytesManifest=[System.IO.File]::ReadAllBytes($manifestPath)
try {
 [System.IO.File]::WriteAllBytes($source,[System.IO.File]::ReadAllBytes($oldSource))
 [System.IO.File]::WriteAllBytes($manifestPath,[System.IO.File]::ReadAllBytes($oldManifest))
 Need ((Sha $source) -eq $oldSha) 'Stage98 r1 restoration failed.'
 $m1=Get-Content -LiteralPath $manifestPath -Raw -Encoding UTF8 | ConvertFrom-Json
 Need ($m1.installed_stage98_sha -eq $oldSha) 'Stage98 r1 manifest restoration failed.'
 Remove-Item -LiteralPath $backup -Recurse -Force
 Write-Host 'STAGE98_R2_ROLLED_BACK_TO_R1. Rebuild x86 or restore saved Stage98 r1 DLL.'
} catch {
 [System.IO.File]::WriteAllBytes($source,$bytesNew)
 [System.IO.File]::WriteAllBytes($manifestPath,$bytesManifest)
 throw
}
