param([string]$Root='D:\INCUBATOR\ArcanaAIOverhaul')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
function Sha([string]$File){return (Get-FileHash -LiteralPath $File -Algorithm SHA256).Hash.ToLowerInvariant()}
$src=Join-Path $Root 'src'
$stage81=Join-Path $src 'Stage81SelectorWriter.inl'
$stage88=Join-Path $src 'Stage88Stage24Join.inl'
$stage90=Join-Path $src 'Stage90AllF57D0.inl'
$backup81="$stage81.stage89r1.bak"
$backup88="$stage88.stage89r1.bak"
$state=Join-Path $Root 'Stage90InstallState.json'
foreach($p in @($stage81,$stage88,$stage90,$backup81,$backup88,$state)){
 if(!(Test-Path -LiteralPath $p -PathType Leaf)){throw "Rollback refused, missing $p"}
}
if(Get-Process -Name 'AALib' -ErrorAction SilentlyContinue){throw 'Close the game before rollback'}
$receipt=Get-Content -LiteralPath $state -Raw | ConvertFrom-Json
if($receipt.stage -ne 'Stage90 r1'){throw 'Unexpected install receipt stage'}
if((Sha $stage81) -ne $receipt.after81 -or
   (Sha $stage88) -ne $receipt.after88 -or
   (Sha $stage90) -ne $receipt.after90){
 throw 'Stage90 files changed since install. Preserve edits; rollback refused.'
}
if((Sha $backup81) -ne $receipt.before81 -or
   (Sha $backup88) -ne $receipt.before88){throw 'Baseline backups changed; rollback refused'}
Copy-Item -LiteralPath $backup81 -Destination $stage81 -Force
Copy-Item -LiteralPath $backup88 -Destination $stage88 -Force
if((Sha $stage81) -ne $receipt.before81 -or (Sha $stage88) -ne $receipt.before88){
 throw 'Restored source hash mismatch; preserve all backups and examine state.'
}
Remove-Item -LiteralPath $stage90 -Force
Remove-Item -LiteralPath $state -Force
Remove-Item -LiteralPath $backup81 -Force
Remove-Item -LiteralPath $backup88 -Force
Write-Host 'Stage90 rollback complete. Source returned to the exact Stage89 r1 baseline. Rebuild x86.'
