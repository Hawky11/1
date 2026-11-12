# Stage94 r2 diagnostic source-only update. Updates ONLY the Stage94 observer,
# preserves Stage81/92/93, existing Stage94 backup and rollback manifest.
param([Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
function Need([bool]$condition,[string]$reason) { if(-not $condition){ throw $reason } }
function Sha([string]$path) { return (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() }
$rootFull=(Resolve-Path -LiteralPath $Root).Path
$src=Join-Path $rootFull 'src'
$obs=Join-Path $src 'Stage94Type1ProducerGate.inl'
$stage81=Join-Path $src 'Stage81SelectorWriter.inl'
$stage92=Join-Path $src 'Stage92F5990BranchTrace.inl'
$stage93=Join-Path $src 'Stage93F5A68DirectBit3.inl'
$backup=Join-Path $rootFull '.stage94_source_backup'
$manifest=Join-Path $backup 'manifest.json'
$package=Join-Path $PSScriptRoot 'Stage94Type1ProducerGate.inl'
$oldSha='43e5fb93446b1da9868f5e0d354ce7bc1d83c1e185f078956ab6349cf2394606'
$newSha='80f5d540387c2e0df30898a0cb32d60050a8917104e816e4a63bdd135bec772e'
Need ((Test-Path -LiteralPath $manifest -PathType Leaf) -and
      (Test-Path -LiteralPath $obs -PathType Leaf) -and
      (Test-Path -LiteralPath $stage81 -PathType Leaf) -and
      (Test-Path -LiteralPath $stage92 -PathType Leaf) -and
      (Test-Path -LiteralPath $stage93 -PathType Leaf) -and
      (Test-Path -LiteralPath $package -PathType Leaf)) 'Stage94 r1 source/backup or diagnostic package missing; nothing modified.'
$m=Get-Content -LiteralPath $manifest -Raw -Encoding UTF8 | ConvertFrom-Json
Need ((Sha $obs) -eq $oldSha -and $m.installed_stage94_sha -eq $oldSha) 'Stage94 observer already changed; no modifications.'
Need ((Sha $stage81) -eq '06602381692d903b8141cd7e735480c091ed8aa21f0adef5882afadc839aa4e6') 'Stage94 Stage81 source differs; no modifications.'
Need ((Sha $stage92) -eq '34d4e6845594e1e107e412614211eddfd2a1e882ffe0e43669d14848e97f40f7') 'Stage92 is not exact r3; no modifications.'
Need ((Sha $stage93) -eq 'bb5653e40579fd22ba26b436040dc6403998b71b6837a70ebec3ae96940823b9') 'Stage93 source differs; no modifications.'
Need ((Sha $package) -eq $newSha) 'Diagnostic package checksum invalid; no modifications.'
$oldObs=[System.IO.File]::ReadAllBytes($obs)
$oldManifest=[System.IO.File]::ReadAllBytes($manifest)
try {
  [System.IO.File]::WriteAllBytes($obs,[System.IO.File]::ReadAllBytes($package))
  Need ((Sha $obs) -eq $newSha) 'Stage94 observer post-write checksum mismatch.'
  $m.installed_stage94_sha = $newSha
  $utf8=New-Object System.Text.UTF8Encoding($false)
  [System.IO.File]::WriteAllText($manifest,($m | ConvertTo-Json),$utf8)
  $m2=Get-Content -LiteralPath $manifest -Raw -Encoding UTF8 | ConvertFrom-Json
  Need ($m2.installed_stage94_sha -eq $newSha) 'Stage94 manifest post-write verification failed.'
  Write-Host 'STAGE94 r2 DIAGNOSTIC SOURCE INSTALLED. Rebuild x86; run once; inspect STAGE94_SIGFAIL entries.'
} catch {
  [System.IO.File]::WriteAllBytes($obs,$oldObs)
  [System.IO.File]::WriteAllBytes($manifest,$oldManifest)
  throw
}
