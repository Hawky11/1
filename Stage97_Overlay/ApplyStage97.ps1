# Stage97 r1 SHA-gated source-only install ON TOP OF confirmed Stage96 r1 sources.
# Close AALib.exe; source files only, no binary/game changes by this script.
param([Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
function Need([bool]$yes,[string]$msg){if(-not $yes){throw $msg}}
function Sha([string]$path){return (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()}
$rootFull=(Resolve-Path -LiteralPath $Root).Path
$src=Join-Path $rootFull 'src'
$stage81=Join-Path $src 'Stage81SelectorWriter.inl'
$stage96=Join-Path $src 'Stage96PreBypassEsiWriter.inl'
$stage97=Join-Path $src 'Stage97InheritedEsiTrace.inl'
$manifest96=Join-Path $rootFull '.stage96_source_backup/manifest.json'
$backup=Join-Path $rootFull '.stage97_source_backup'
$pack81=Join-Path $PSScriptRoot 'src/Stage81SelectorWriter.inl'
$pack97=Join-Path $PSScriptRoot 'src/Stage97InheritedEsiTrace.inl'
$old81='7ea9938883f8a7b5e3db6142d64750902d51a49f9ce6603ba617f9f7e1644f81'
$old96='d3defa5befc6cb8a1fc1b1d51ab72d3c8445e319ac40762a14679bc74e4f8516'
$new81='a7d8aef49b68d5b30090b313e3cb2f94cebf51ab6a23d7af40766f01a941fe43'
$new97='f368037433b3baaec4f322428875edbbb8f3c12e1c89486c5d9ba8ff49c2f78c'
Need (!(Get-Process AALib -ErrorAction SilentlyContinue)) 'Close AALib.exe before Stage97 install.'
Need ((Test-Path -LiteralPath $stage81 -PathType Leaf) -and
      (Test-Path -LiteralPath $stage96 -PathType Leaf) -and
      (Test-Path -LiteralPath $manifest96 -PathType Leaf) -and
      (Test-Path -LiteralPath $pack81 -PathType Leaf) -and
      (Test-Path -LiteralPath $pack97 -PathType Leaf)) 'Stage96 source or Stage97 package incomplete; no changes.'
Need (!(Test-Path -LiteralPath $stage97) -and !(Test-Path -LiteralPath $backup)) 'Stage97 already exists; refusing reapply.'
Need ((Sha $stage81) -eq $old81 -and (Sha $stage96) -eq $old96) 'Stage96 source SHA differs from verified r1; no changes.'
$m96=Get-Content -LiteralPath $manifest96 -Raw -Encoding UTF8|ConvertFrom-Json
Need (($m96.installed_stage81_sha -eq $old81) -and ($m96.installed_stage96_sha -eq $old96)) 'Stage96 rollback manifest differs; no changes.'
Need ((Sha $pack81) -eq $new81 -and (Sha $pack97) -eq $new97) 'Stage97 package source SHA mismatch; no changes.'
$wired=[System.IO.File]::ReadAllText($pack81)
Need ($wired.Contains('#include "Stage97InheritedEsiTrace.inl"') -and
      $wired.Contains('if (arcana_stage96::Install()) arcana_stage97::Install();') -and
      $wired.Contains('arcana_stage96::Flush(); arcana_stage97::Flush();') -and
      $wired.Contains('arcana_stage96::Health(); arcana_stage97::Health();')) 'Stage97 loader wiring missing.'
New-Item -ItemType Directory -Path $backup -ErrorAction Stop|Out-Null
$made97=$false;$wrote81=$false
try {
 [System.IO.File]::WriteAllBytes((Join-Path $backup 'Stage81SelectorWriter.inl'),[System.IO.File]::ReadAllBytes($stage81))
 $made97=$true
 [System.IO.File]::WriteAllBytes($stage97,[System.IO.File]::ReadAllBytes($pack97))
 $wrote81=$true
 [System.IO.File]::WriteAllBytes($stage81,[System.IO.File]::ReadAllBytes($pack81))
 Need ((Sha $stage81) -eq $new81 -and (Sha $stage97) -eq $new97) 'Stage97 post-write SHA mismatch.'
 $m=[ordered]@{stage='Stage97 r1';installed_stage81_sha=$new81;installed_stage97_sha=$new97;
  parent_stage81_sha=$old81;parent_stage96_sha=$old96}
 $utf8=New-Object System.Text.UTF8Encoding($false)
 [System.IO.File]::WriteAllText((Join-Path $backup 'manifest.json'),($m|ConvertTo-Json),$utf8)
 Write-Host 'STAGE97 SOURCE INSTALLED (verified Stage96 intact). Build MSVC x86; require STAGE97_INSTALLED 1/1.'
} catch {
 if($wrote81){[System.IO.File]::WriteAllBytes($stage81,[System.IO.File]::ReadAllBytes((Join-Path $backup 'Stage81SelectorWriter.inl')))}
 if($made97 -and (Test-Path -LiteralPath $stage97)){Remove-Item -LiteralPath $stage97 -Force}
 if(Test-Path -LiteralPath $backup){Remove-Item -LiteralPath $backup -Recurse -Force}
 throw
}
