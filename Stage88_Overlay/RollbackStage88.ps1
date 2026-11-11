param([string]$Root = 'D:\INCUBATOR\ArcanaAIOverhaul')
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
function Sha256([string]$Path) { return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant() }
$src = Join-Path $Root 'src'
$statePath = Join-Path $Root 'Stage88InstallState.json'
if (!(Test-Path -LiteralPath $statePath)) { throw 'Stage88 state missing; refusing blind rollback' }
$state = Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json
$names = @('Stage81SelectorWriter.inl','Stage87SelectorConsumer.inl','AIQueueTrace.cpp')
$bridge=Join-Path $src 'Stage88Stage24Join.inl'
if (!(Test-Path -LiteralPath $bridge) -or (Sha256 $bridge) -ne $state.bridgeSha) {
    throw 'Stage88 bridge differs from installed version; refusing rollback'
}
foreach ($name in $names) {
    $p=Join-Path $src $name; $bak="$p.stage87.bak"
    if (!(Test-Path -LiteralPath $p) -or (Sha256 $p) -ne $state.after.$name) { throw "Stage88: modified $name; refusing rollback" }
    if (!(Test-Path -LiteralPath $bak) -or (Sha256 $bak) -ne $state.before.$name) { throw "Stage88: missing or modified $bak" }
}
foreach ($name in $names) {
    $p=Join-Path $src $name
    [IO.File]::Copy("$p.stage87.bak",$p,$true)
}
Remove-Item -LiteralPath $bridge -Force
Remove-Item -LiteralPath $statePath -Force
Write-Host 'STAGE88 ROLLBACK OK - Stage87 r1 sources restored. Rebuild and reinstall DLL.'