param([string]$Root = 'D:\INCUBATOR\ArcanaAIOverhaul')
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
function Hash([string]$File) {return (Get-FileHash -LiteralPath $File -Algorithm SHA256).Hash.ToLowerInvariant()}
$src=Join-Path $Root 'src'
$stage81=Join-Path $src 'Stage81SelectorWriter.inl'
$stage89=Join-Path $src 'Stage89F58E0Reader.inl'
$state=Join-Path $Root 'Stage89InstallState.json'
$backup="$stage81.stage88r1.bak"
foreach($p in @($stage81,$stage89,$state,$backup)) {
 if (!(Test-Path -LiteralPath $p -PathType Leaf)) {throw "Stage89 rollback: missing $p"}
}
$record=Get-Content -LiteralPath $state -Raw | ConvertFrom-Json
if ($record.stage -ne 'Stage89 r1' -or (Hash $stage81) -ne $record.stage81After -or
    (Hash $stage89) -ne $record.stage89Sha -or (Hash $backup) -ne $record.stage81Before) {
 throw 'Stage89 rollback: source changed unexpectedly; refusing to overwrite. Restore manually from trusted backup.'
}
Copy-Item -LiteralPath $backup -Destination $stage81 -Force
Remove-Item -LiteralPath $stage89 -Force
Remove-Item -LiteralPath $state -Force
Remove-Item -LiteralPath $backup -Force
Write-Host 'Stage89 rollback OK. Stage88 source restored. Rebuild x86, or restore saved Stage88 DLL.'
