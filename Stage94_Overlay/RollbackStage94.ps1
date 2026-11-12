# SHA-verified Stage94-only rollback. Does not remove Stage93 or its backups.
param([Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
function Sha([string]$p) { return (Get-FileHash -LiteralPath $p -Algorithm SHA256).Hash.ToLowerInvariant() }
$rootFull = (Resolve-Path -LiteralPath $Root).Path
$src = Join-Path $rootFull 'src'
$a = Join-Path $src 'Stage81SelectorWriter.inl'
$b = Join-Path $src 'Stage94Type1ProducerGate.inl'
$c = Join-Path $src 'Stage93F5A68DirectBit3.inl'
$d = Join-Path $src 'Stage92F5990BranchTrace.inl'
$backup = Join-Path $rootFull '.stage94_source_backup'
$manifestPath = Join-Path $backup 'manifest.json'
if (!(Test-Path -LiteralPath $manifestPath -PathType Leaf)) { throw 'Stage94 backup manifest missing. No changes.' }
$m = Get-Content -LiteralPath $manifestPath -Raw -Encoding UTF8 | ConvertFrom-Json
if (!(Test-Path -LiteralPath $a -PathType Leaf) -or !(Test-Path -LiteralPath $b -PathType Leaf) -or
    !(Test-Path -LiteralPath $c -PathType Leaf) -or !(Test-Path -LiteralPath $d -PathType Leaf) -or
    (Sha $a) -ne $m.installed_stage81_sha -or (Sha $b) -ne $m.installed_stage94_sha -or
    (Sha $c) -ne $m.stage93_sha -or (Sha $d) -ne $m.stage92_sha) {
    throw 'Stage94 or prerequisite files changed. Refusing to overwrite newer work.'
}
$old = Join-Path $backup 'Stage81SelectorWriter.inl'
if (!(Test-Path -LiteralPath $old -PathType Leaf) -or (Sha $old) -ne $m.parent_stage93_stage81_sha) {
    throw 'Stage94 original Stage81 backup absent/corrupt; refusing rollback.'
}
Copy-Item -LiteralPath $old -Destination $a -Force
if ((Sha $a) -ne $m.parent_stage93_stage81_sha) { throw 'Restore SHA failed; preserving backup.' }
Remove-Item -LiteralPath $b -Force
Remove-Item -LiteralPath $backup -Recurse -Force
Write-Host 'STAGE94 ROLLBACK COMPLETE. Exact tested Stage93 r3 source restored. Rebuild x86.'
