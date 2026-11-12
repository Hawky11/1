# Stage98 r2 SOURCE ONLY: restore GetCurrentThreadId() assignment accidentally commented out in r1.
# Safely updates Stage98 observer + its rollback SHA manifest, leaving Stage81/94-97 and native code unchanged.
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
$stage97backup = Join-Path $rootFull '.stage98_source_backup/Stage81SelectorWriter.inl'
$patch = Join-Path $PSScriptRoot 'src/Stage98CommandRouteDecision.inl'
$backup = Join-Path $rootFull '.stage98_r2_patch_backup'
$oldSha = 'a2548073f73fd7967ecece70cb278b32c162c0dfd07d9f63be7f132f628d5ba1'
$newSha = 'a56fe5692b3284fae37822e6f08438e3a32a800accbad5ff4c72d36ea97a1591'
$stage81Sha = '12f2afbee4cf427f2b13cce7e9baeb7d525fc4e4b23bc42cc1c3d0a86463a192'
$stage97Sha = 'f368037433b3baaec4f322428875edbbb8f3c12e1c89486c5d9ba8ff49c2f78c'
$backupStage81Sha = 'a7d8aef49b68d5b30090b313e3cb2f94cebf51ab6a23d7af40766f01a941fe43'
Need (!(Get-Process AALib -ErrorAction SilentlyContinue)) 'Close AALib.exe before updating Stage98 sources.'
foreach($p in @($source,$stage81,$stage97,$manifestPath,$stage97backup,$patch)) {
 Need (Test-Path -LiteralPath $p -PathType Leaf) ('Missing prerequisite: '+$p)
}
Need (!(Test-Path -LiteralPath $backup)) 'Stage98 r2 backup already exists; do not reapply.'
Need ((Sha $source) -eq $oldSha) 'Stage98 r1 source SHA mismatch. No changes.'
Need ((Sha $patch) -eq $newSha) 'Stage98 r2 patch SHA mismatch. No changes.'
Need ((Sha $stage81) -eq $stage81Sha -and (Sha $stage97) -eq $stage97Sha -and
      (Sha $stage97backup) -eq $backupStage81Sha) 'Stage81 / Stage97 source or previous backup changed. No changes.'
$m = Get-Content -LiteralPath $manifestPath -Raw -Encoding UTF8 | ConvertFrom-Json
Need (($m.stage -eq 'Stage98 r1') -and
      ($m.installed_stage98_sha -eq $oldSha) -and
      ($m.installed_stage81_sha -eq $stage81Sha) -and
      ($m.parent_stage81_sha -eq $backupStage81Sha) -and
      ($m.parent_stage97_sha -eq $stage97Sha)) 'Stage98 r1 rollback manifest invalid or already updated. No changes.'
$utf8 = New-Object System.Text.UTF8Encoding($false)
New-Item -ItemType Directory -Path $backup -ErrorAction Stop | Out-Null
try {
 [System.IO.File]::WriteAllBytes((Join-Path $backup 'Stage98CommandRouteDecision.inl'), [System.IO.File]::ReadAllBytes($source))
 [System.IO.File]::WriteAllBytes((Join-Path $backup 'manifest.json'), [System.IO.File]::ReadAllBytes($manifestPath))
 Need ((Sha (Join-Path $backup 'Stage98CommandRouteDecision.inl')) -eq $oldSha) 'Stage98 backup copy mismatch.'
 [System.IO.File]::WriteAllBytes($source, [System.IO.File]::ReadAllBytes($patch))
 Need ((Sha $source) -eq $newSha) 'Stage98 post-write SHA mismatch.'
 $m.installed_stage98_sha = $newSha
 $m | Add-Member -NotePropertyName source_revision -NotePropertyValue 'Stage98 r2 TID logging fix' -Force
 # Keep m.stage='Stage98 r1' because original RollbackStage98.ps1 explicitly guards this value.
 [System.IO.File]::WriteAllText($manifestPath, ($m | ConvertTo-Json -Depth 8), $utf8)
 $test = Get-Content -LiteralPath $manifestPath -Raw -Encoding UTF8 | ConvertFrom-Json
 Need (($test.stage -eq 'Stage98 r1') -and ($test.installed_stage98_sha -eq $newSha) -and
       ($test.source_revision -eq 'Stage98 r2 TID logging fix')) 'Stage98 patched manifest verification failed.'
 Write-Host 'STAGE98_R2_SOURCE_UPDATED tid=GetCurrentThreadId sourceOnly=1 nativeHookUnchanged=1'
 Write-Host 'Build x86, launch a short test, verify STAGE98_ROUTE tid != 0 and Stage98 HEALTH clean.'
} catch {
 if(Test-Path -LiteralPath (Join-Path $backup 'Stage98CommandRouteDecision.inl')) {
  [System.IO.File]::WriteAllBytes($source, [System.IO.File]::ReadAllBytes((Join-Path $backup 'Stage98CommandRouteDecision.inl')))
 }
 if(Test-Path -LiteralPath (Join-Path $backup 'manifest.json')) {
  [System.IO.File]::WriteAllBytes($manifestPath, [System.IO.File]::ReadAllBytes((Join-Path $backup 'manifest.json')))
 }
 if(Test-Path -LiteralPath $backup) { Remove-Item -LiteralPath $backup -Recurse -Force }
 throw
}
