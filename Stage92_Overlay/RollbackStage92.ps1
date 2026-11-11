param([string]$Root='D:\INCUBATOR\ArcanaAIOverhaul')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
function Sha([string]$p){(Get-FileHash -LiteralPath $p -Algorithm SHA256).Hash.ToLowerInvariant()}
$src=Join-Path $Root 'src'
$adapter=Join-Path $src 'Stage81SelectorWriter.inl'
$observer=Join-Path $src 'Stage92F5990BranchTrace.inl'
$backup="$adapter.stage91r1.bak"
$receipt=Join-Path $Root 'Stage92InstallState.json'
foreach($p in @($adapter,$observer,$backup,$receipt)){
 if(!(Test-Path -LiteralPath $p -PathType Leaf)){throw "Stage92 rollback missing $p; refusing"}
}
if(Get-Process -Name 'AALib' -ErrorAction SilentlyContinue){throw 'Exit AALib.exe first'}
if((Sha $adapter) -ne '6d914c14533913f7c22fd0c90b1162901c4e2a47f3cc144912be6d0288342bc8' -or
   (Sha $observer) -ne '7ba05c10d3e476403385d80b9c7925a415d09f59bd3f73fdc88975585c5a7e33' -or
   (Sha $backup) -ne '3b4eb42c458f3419c2ad2c7f9433efb2ad012c88244c3b0e5349793e89da397e'){
 throw 'Stage92 rollback hash mismatch; refusing to overwrite modified sources'
}
Copy-Item -LiteralPath $backup -Destination $adapter -Force
Remove-Item -LiteralPath $observer,$receipt,$backup -Force
Write-Host 'Stage91 adapter restored. Rebuild x86 or restore Stage91 DLL 1,999,872 bytes.'
