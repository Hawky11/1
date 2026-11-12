# Stage98 r1 — source-only update on exactly verified Stage97 r1; preserve earlier stages.
# Requires AALib.exe closed. Fail closed on any SHA/manifest/wiring mismatch.
param([Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
function Need([bool]$ok,[string]$msg){if(-not $ok){throw $msg}}
function Sha([string]$f){(Get-FileHash -LiteralPath $f -Algorithm SHA256).Hash.ToLowerInvariant()}
$rootFull=(Resolve-Path -LiteralPath $Root).Path
$src=Join-Path $rootFull 'src'
$stage81=Join-Path $src 'Stage81SelectorWriter.inl'
$stage97=Join-Path $src 'Stage97InheritedEsiTrace.inl'
$stage98=Join-Path $src 'Stage98CommandRouteDecision.inl'
$manifest97=Join-Path $rootFull '.stage97_source_backup/manifest.json'
$backup=Join-Path $rootFull '.stage98_source_backup'
$pack81=Join-Path $PSScriptRoot 'src/Stage81SelectorWriter.inl'
$pack98=Join-Path $PSScriptRoot 'src/Stage98CommandRouteDecision.inl'
$parent81='a7d8aef49b68d5b30090b313e3cb2f94cebf51ab6a23d7af40766f01a941fe43'
$parent97='f368037433b3baaec4f322428875edbbb8f3c12e1c89486c5d9ba8ff49c2f78c'
$new81='12f2afbee4cf427f2b13cce7e9baeb7d525fc4e4b23bc42cc1c3d0a86463a192'
$new98='a2548073f73fd7967ecece70cb278b32c162c0dfd07d9f63be7f132f628d5ba1'
Need (!(Get-Process AALib -ErrorAction SilentlyContinue)) 'Close AALib.exe before Stage98 install.'
Need ((Test-Path -LiteralPath $stage81 -PathType Leaf) -and
      (Test-Path -LiteralPath $stage97 -PathType Leaf) -and
      (Test-Path -LiteralPath $manifest97 -PathType Leaf) -and
      (Test-Path -LiteralPath $pack81 -PathType Leaf) -and
      (Test-Path -LiteralPath $pack98 -PathType Leaf)) 'Stage97 source, manifest, or Stage98 package missing; no changes.'
Need (!(Test-Path -LiteralPath $stage98) -and !(Test-Path -LiteralPath $backup)) 'Stage98 already present; no reapply.'
Need ((Sha $stage81) -eq $parent81 -and (Sha $stage97) -eq $parent97) 'Stage97 source SHA mismatch; no changes.'
$m97=Get-Content -LiteralPath $manifest97 -Raw -Encoding UTF8|ConvertFrom-Json
Need (($m97.installed_stage81_sha -eq $parent81) -and
      ($m97.installed_stage97_sha -eq $parent97)) 'Stage97 backup manifest does not match installed source; no changes.'
Need ((Sha $pack81) -eq $new81 -and (Sha $pack98) -eq $new98) 'Stage98 package source SHA mismatch; no changes.'
$wired=[System.IO.File]::ReadAllText($pack81)
Need (($wired.Contains('namespace arcana_stage98 { bool Install(); void Flush(); void Health(); }')) -and
      ($wired.Contains('#include "Stage98CommandRouteDecision.inl"')) -and
      ($wired.Contains('if (arcana_stage97::Install()) arcana_stage98::Install();')) -and
      ($wired.Contains('arcana_stage97::Flush(); arcana_stage98::Flush();')) -and
      ($wired.Contains('arcana_stage97::Health(); arcana_stage98::Health();'))) 'Stage98 loader wiring invalid.'
New-Item -ItemType Directory -Path $backup -ErrorAction Stop|Out-Null
$made98=$false;$wrote81=$false
try {
 [System.IO.File]::WriteAllBytes((Join-Path $backup 'Stage81SelectorWriter.inl'),[System.IO.File]::ReadAllBytes($stage81))
 $made98=$true
 [System.IO.File]::WriteAllBytes($stage98,[System.IO.File]::ReadAllBytes($pack98))
 $wrote81=$true
 [System.IO.File]::WriteAllBytes($stage81,[System.IO.File]::ReadAllBytes($pack81))
 Need ((Sha $stage81) -eq $new81 -and (Sha $stage98) -eq $new98) 'Stage98 post-write source SHA mismatch.'
 $m=[ordered]@{stage='Stage98 r1';installed_stage81_sha=$new81;installed_stage98_sha=$new98;
  parent_stage81_sha=$parent81;parent_stage97_sha=$parent97}
 $utf8=New-Object System.Text.UTF8Encoding($false)
 [System.IO.File]::WriteAllText((Join-Path $backup 'manifest.json'),($m|ConvertTo-Json),$utf8)
 Write-Host 'STAGE98 SOURCE INSTALLED (Stage97 retained). Build MSVC x86; require STAGE98_INSTALLED 1/1.'
} catch {
 if($wrote81){[System.IO.File]::WriteAllBytes($stage81,[System.IO.File]::ReadAllBytes((Join-Path $backup 'Stage81SelectorWriter.inl')))}
 if($made98 -and (Test-Path -LiteralPath $stage98)){Remove-Item -LiteralPath $stage98 -Force}
 if(Test-Path -LiteralPath $backup){Remove-Item -LiteralPath $backup -Recurse -Force}
 throw
}
