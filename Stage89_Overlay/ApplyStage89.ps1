param([string]$Root = 'D:\INCUBATOR\ArcanaAIOverhaul')
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
function Hash([string]$File) { return (Get-FileHash -LiteralPath $File -Algorithm SHA256).Hash.ToLowerInvariant() }

$src=Join-Path $Root 'src'
$stage81=Join-Path $src 'Stage81SelectorWriter.inl'
$stage89=Join-Path $src 'Stage89F58E0Reader.inl'
$stage88=Join-Path $src 'Stage88Stage24Join.inl'
$stage87=Join-Path $src 'Stage87SelectorConsumer.inl'
$state=Join-Path $Root 'Stage89InstallState.json'
$backup="$stage81.stage88r1.bak"
$expectStage88='ede3d2dcc7218b82bc0d8d7f4967e1e2cd5e93eee985f9137d9c10d37fe25f3f'
$overlay81=Join-Path $PSScriptRoot 'src\Stage81SelectorWriter.inl'
$overlay89=Join-Path $PSScriptRoot 'src\Stage89F58E0Reader.inl'
foreach ($p in @($stage81,$stage88,$stage87,$overlay81,$overlay89)) {
 if (!(Test-Path -LiteralPath $p -PathType Leaf)) {throw "Stage89: missing $p; no files changed"}
}
if ((Hash $stage81) -ne $expectStage88){ throw 'Stage89: Stage81 is not exact Stage88 r1; NO changes' }
if (Test-Path -LiteralPath $state) { throw 'Stage89: previous install state exists; NO changes' }
if (Test-Path -LiteralPath $backup) { throw 'Stage89: backup already exists; NO changes' }
if (Test-Path -LiteralPath $stage89) { throw 'Stage89: reader source already exists; NO changes' }
$updated=[IO.File]::ReadAllText($overlay81)
foreach ($needle in @('#include "Stage89F58E0Reader.inl"','arcana_stage89::Install();','arcana_stage89::Health();')){
 if (!$updated.Contains($needle)){ throw "Stage89: package inconsistent ($needle)" }
}
Copy-Item -LiteralPath $stage81 -Destination $backup -ErrorAction Stop
try {
 Copy-Item -LiteralPath $overlay89 -Destination $stage89 -ErrorAction Stop
 Copy-Item -LiteralPath $overlay81 -Destination $stage81 -Force -ErrorAction Stop
 $entry=[ordered]@{
    stage='Stage89 r1'; created=(Get-Date).ToString('o');
    stage81Before=$expectStage88; stage81After=(Hash $stage81); stage89Sha=(Hash $stage89)
 }
 [IO.File]::WriteAllText($state,($entry|ConvertTo-Json -Depth 4),[Text.UTF8Encoding]::new($false))
 Write-Host 'Stage89 apply OK. Four guarded native CALL sites; no F58E0 global detour, no second EE306 patch.'
 Write-Host 'Keep EnableStage36ControlToCommandTrace=0; Stage88+Stage24 enabled; use x86 Build.bat.'
} catch {
 if (Test-Path -LiteralPath $backup) {Copy-Item -LiteralPath $backup -Destination $stage81 -Force}
 if (Test-Path -LiteralPath $stage89) {Remove-Item -LiteralPath $stage89 -Force}
 if (Test-Path -LiteralPath $state) {Remove-Item -LiteralPath $state -Force}
 if (Test-Path -LiteralPath $backup) {Remove-Item -LiteralPath $backup -Force}
 throw
}
