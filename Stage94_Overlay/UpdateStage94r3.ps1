# Stage94 r3: strictly source-only Stage82-callsite compatibility update.
# Updates only existing Stage94 observer and its rollback manifest checksum.
param([Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
function Need([bool]$ok,[string]$why) { if(-not $ok) { throw $why } }
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
$allowedParents=@(
 '80f5d540387c2e0df30898a0cb32d60050a8917104e816e4a63bdd135bec772e', # Stage94 r2 diagnostic
 '43e5fb93446b1da9868f5e0d354ce7bc1d83c1e185f078956ab6349cf2394606'  # Stage94 r1 original
)
$newSha='9b84a4f9c3d8b37b837d50dde252a76c02aa5d7acbc74d194f79fadf32eff662'
Need ((Test-Path -LiteralPath $manifest -PathType Leaf) -and
      (Test-Path -LiteralPath $obs -PathType Leaf) -and
      (Test-Path -LiteralPath $stage81 -PathType Leaf) -and
      (Test-Path -LiteralPath $stage92 -PathType Leaf) -and
      (Test-Path -LiteralPath $stage93 -PathType Leaf) -and
      (Test-Path -LiteralPath $package -PathType Leaf)) 'Expected Stage94 r1/r2 installation/backup or r3 package missing; no changes.'
$m=Get-Content -LiteralPath $manifest -Raw -Encoding UTF8|ConvertFrom-Json
$oldSha=Sha $obs
Need (($allowedParents -contains $oldSha) -and ($m.installed_stage94_sha -eq $oldSha)) 'Unexpected Stage94 source/manifest SHA. Refusing to modify.'
Need ((Sha $stage81) -eq '06602381692d903b8141cd7e735480c091ed8aa21f0adef5882afadc839aa4e6') 'Stage81 modified since verified Stage94 baseline; refusing.'
Need ((Sha $stage92) -eq '34d4e6845594e1e107e412614211eddfd2a1e882ffe0e43669d14848e97f40f7') 'Stage92 r3 source differs; refusing.'
Need ((Sha $stage93) -eq 'bb5653e40579fd22ba26b436040dc6403998b71b6837a70ebec3ae96940823b9') 'Stage93 source differs; refusing.'
Need ((Sha $package) -eq $newSha) 'Stage94 r3 package SHA invalid; refusing.'
$expectedStage82=Join-Path $src 'Stage82NestedSelector.inl'
Need (Test-Path -LiteralPath $expectedStage82 -PathType Leaf) 'Stage82 source missing; refusing Stage82-specific compatibility update.'
# Only static preflight. Exact runtime Stage82 handler/address/owner checked in code at installation.
$src82=[System.IO.File]::ReadAllText($expectedStage82)
Need ($src82.Contains('Stage82Call1') -and $src82.Contains('g_Stage82Target1') -and $src82.Contains('kRvas[2]={0xF79E4u,0xF7AD1u}')) 'Stage82 source lacks expected native CALL stub declarations; no changes.'
$oldObs=[System.IO.File]::ReadAllBytes($obs)
$oldManifest=[System.IO.File]::ReadAllBytes($manifest)
try {
 [System.IO.File]::WriteAllBytes($obs,[System.IO.File]::ReadAllBytes($package))
 Need ((Sha $obs) -eq $newSha) 'Stage94 r3 observer post-write SHA mismatch.'
 $m.installed_stage94_sha=$newSha
 $utf8=New-Object System.Text.UTF8Encoding($false)
 [System.IO.File]::WriteAllText($manifest,($m|ConvertTo-Json),$utf8)
 $m2=Get-Content -LiteralPath $manifest -Raw -Encoding UTF8 | ConvertFrom-Json
 Need ($m2.installed_stage94_sha -eq $newSha) 'Stage94 r3 rollback manifest update failed.'
 Write-Host 'STAGE94 r3 STAGE82-COMPAT SOURCE INSTALLED. Rebuild MSVC x86 and inspect STAGE94_PRODUCER_CHAIN_OK.'
} catch {
 [System.IO.File]::WriteAllBytes($obs,$oldObs)
 [System.IO.File]::WriteAllBytes($manifest,$oldManifest)
 throw
}
