param([string]$Root='D:\INCUBATOR\ArcanaAIOverhaul')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
function Sha([string]$File){return (Get-FileHash -LiteralPath $File -Algorithm SHA256).Hash.ToLowerInvariant()}
$src=Join-Path $Root 'src'
$stage81=Join-Path $src 'Stage81SelectorWriter.inl'
$stage91=Join-Path $src 'Stage91DirectSpecialReader.inl'
$backup81="$stage81.stage90r1.bak"
$state=Join-Path $Root 'Stage91InstallState.json'
foreach($p in @($stage81,$stage91,$backup81,$state)){
 if(!(Test-Path -LiteralPath $p -PathType Leaf)){throw "Stage91 rollback refused: missing $p"}
}
if(Get-Process -Name 'AALib' -ErrorAction SilentlyContinue){throw 'Close AALib.exe before rollback'}
$receipt=Get-Content -LiteralPath $state -Raw | ConvertFrom-Json
if($receipt.stage -ne 'Stage91 r1'){throw 'Unexpected Stage91 receipt stage'}
if((Sha $stage81) -ne $receipt.after81 -or (Sha $stage91) -ne $receipt.after91){
 throw 'Stage91 files were edited after installation. Preserve modifications; rollback refused.'
}
if((Sha $backup81) -ne $receipt.before81){throw 'Stage90 adapter backup changed; rollback refused'}
Copy-Item -LiteralPath $backup81 -Destination $stage81 -Force
if((Sha $stage81) -ne $receipt.before81){throw 'Restored adapter SHA mismatch; preserve backups and stop'}
Remove-Item -LiteralPath $stage91 -Force
Remove-Item -LiteralPath $state -Force
Remove-Item -LiteralPath $backup81 -Force
Write-Host 'Stage91 removed. Stage90 source preserved. Rebuild Windows x86 or restore Stage90 DLL backup.'
