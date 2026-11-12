# Stage94 r1: exact Stage93-r3 source-only upgrade. Never change live AALib.exe.
param([Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
function Require([bool]$condition,[string]$why) { if (-not $condition) { throw $why } }
function Sha([string]$path) {
    return (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
}
$rootFull = (Resolve-Path -LiteralPath $Root).Path
$source = Join-Path $rootFull 'src'
$stage81 = Join-Path $source 'Stage81SelectorWriter.inl'
$stage92 = Join-Path $source 'Stage92F5990BranchTrace.inl'
$stage93 = Join-Path $source 'Stage93F5A68DirectBit3.inl'
$stage94 = Join-Path $source 'Stage94Type1ProducerGate.inl'
$package81 = Join-Path $PSScriptRoot 'src/Stage81SelectorWriter.inl'
$package94 = Join-Path $PSScriptRoot 'src/Stage94Type1ProducerGate.inl'
$backup = Join-Path $rootFull '.stage94_source_backup'

$expected81 = 'be577c23c8f7b0bfeada6240d7d205fb7d18f28ae1f74ba377ea77334bb6d0ab'
$expected92 = '34d4e6845594e1e107e412614211eddfd2a1e882ffe0e43669d14848e97f40f7'
$expected93 = 'bb5653e40579fd22ba26b436040dc6403998b71b6837a70ebec3ae96940823b9'
$package81Sha = '06602381692d903b8141cd7e735480c091ed8aa21f0adef5882afadc839aa4e6'
$package94Sha = '43e5fb93446b1da9868f5e0d354ce7bc1d83c1e185f078956ab6349cf2394606'
Require ((Test-Path -LiteralPath $stage81 -PathType Leaf) -and
         (Test-Path -LiteralPath $stage92 -PathType Leaf) -and
         (Test-Path -LiteralPath $stage93 -PathType Leaf)) 'Stage93 r3 source files missing; no changes.'
Require (!(Test-Path -LiteralPath $stage94) -and !(Test-Path -LiteralPath $backup)) 'Stage94 already present/backup exists; refusing reapply.'
Require ((Test-Path -LiteralPath $package81 -PathType Leaf) -and
         (Test-Path -LiteralPath $package94 -PathType Leaf)) 'Stage94 ZIP is incomplete; no changes.'
Require ((Sha $stage81) -eq $expected81) 'Stage81 differs from tested Stage93 r3 source; no changes.'
Require ((Sha $stage92) -eq $expected92) 'Stage92 is not exact Stage35-compatible r3; no changes.'
Require ((Sha $stage93) -eq $expected93) 'Stage93 is not the known-working F5A68 observer; no changes.'
Require ((Sha $package81) -eq $package81Sha) 'Stage94 package Stage81 file hash mismatch; no changes.'
Require ((Sha $package94) -eq $package94Sha) 'Stage94 native observer file hash mismatch; no changes.'
# Explicitly prevent a stale/transplanted package from silently losing Stage93.
$text = [System.IO.File]::ReadAllText($package81)
Require ($text.Contains('if (arcana_stage93::Install()) arcana_stage94::Install();') -and
         $text.Contains('#include "Stage94Type1ProducerGate.inl"') -and
         $text.Contains('arcana_stage93::Flush(); arcana_stage94::Flush();') -and
         $text.Contains('arcana_stage93::Health(); arcana_stage94::Health();')) 'Stage94 Stage81 integration missing; no changes.'

New-Item -ItemType Directory -Path $backup -ErrorAction Stop | Out-Null
try {
    Copy-Item -LiteralPath $stage81 -Destination (Join-Path $backup 'Stage81SelectorWriter.inl') -ErrorAction Stop
    [System.IO.File]::WriteAllBytes($stage81,[System.IO.File]::ReadAllBytes($package81))
    [System.IO.File]::WriteAllBytes($stage94,[System.IO.File]::ReadAllBytes($package94))
    Require ((Sha $stage81) -eq $package81Sha -and (Sha $stage94) -eq $package94Sha) 'Post-write SHA mismatch.'
    $manifest = [ordered]@{
        parent_stage93_stage81_sha = $expected81
        stage92_sha = $expected92
        stage93_sha = $expected93
        installed_stage81_sha = $package81Sha
        installed_stage94_sha = $package94Sha
    }
    $utf8 = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText((Join-Path $backup 'manifest.json'),($manifest|ConvertTo-Json),$utf8)
    Write-Host 'STAGE94 SOURCE INSTALLED. Keep tested Stage93 DLL backup; build Windows MSVC x86.'
} catch {
    if (Test-Path -LiteralPath (Join-Path $backup 'Stage81SelectorWriter.inl')) {
        Copy-Item -LiteralPath (Join-Path $backup 'Stage81SelectorWriter.inl') -Destination $stage81 -Force
    }
    if (Test-Path -LiteralPath $stage94) { Remove-Item -LiteralPath $stage94 -Force }
    Remove-Item -LiteralPath $backup -Recurse -Force
    throw
}
