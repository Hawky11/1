param([string]$Root='D:\INCUBATOR\ArcanaAIOverhaul')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
function Sha([string]$Path){(Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()}
$src=Join-Path $Root 'src'
$adapter=Join-Path $src 'Stage81SelectorWriter.inl'
$reader=Join-Path $src 'Stage91DirectSpecialReader.inl'
$observer=Join-Path $src 'Stage92F5990BranchTrace.inl'
$overlayAdapter=Join-Path $PSScriptRoot 'src\Stage81SelectorWriter.inl'
$overlayObserver=Join-Path $PSScriptRoot 'src\Stage92F5990BranchTrace.inl'
$backup="$adapter.stage91r1.bak"
$receipt=Join-Path $Root 'Stage92InstallState.json'
$expectedAdapter='3b4eb42c458f3419c2ad2c7f9433efb2ad012c88244c3b0e5349793e89da397e'
$expectedReader='cd9a9495eaa6ac905bb961b3997b60b2edb21329c583a82c767be4a097b9a673'
$expectedNewAdapter='6d914c14533913f7c22fd0c90b1162901c4e2a47f3cc144912be6d0288342bc8'
$expectedNewObserver='7ba05c10d3e476403385d80b9c7925a415d09f59bd3f73fdc88975585c5a7e33'
foreach($p in @($adapter,$reader,$overlayAdapter,$overlayObserver)){
 if(!(Test-Path -LiteralPath $p -PathType Leaf)){throw "Missing Stage91/92 file $p; no changes"}
}
foreach($p in @($observer,$backup,$receipt)){
 if(Test-Path -LiteralPath $p){throw "Refusing to overwrite existing $p"}
}
if((Sha $adapter) -ne $expectedAdapter){throw 'Stage92 requires EXACT installed Stage91 r1 adapter'}
if((Sha $reader) -ne $expectedReader){throw 'Stage92 requires EXACT installed Stage91 r1 F58E0 reader'}
if((Sha $overlayAdapter) -ne $expectedNewAdapter -or (Sha $overlayObserver) -ne $expectedNewObserver){
 throw 'Stage92 overlay SHA mismatch; no changes'
}
if(Get-Process -Name 'AALib' -ErrorAction SilentlyContinue){throw 'Exit AALib.exe before patching'}
Copy-Item -LiteralPath $adapter -Destination $backup -ErrorAction Stop
try {
 Copy-Item -LiteralPath $overlayObserver -Destination $observer -ErrorAction Stop
 Copy-Item -LiteralPath $overlayAdapter -Destination $adapter -Force -ErrorAction Stop
 $state=[ordered]@{stage='Stage92 r1 source-only';created=(Get-Date).ToString('o');
   beforeAdapter=$expectedAdapter;afterAdapter=(Sha $adapter);afterObserver=(Sha $observer);
   stage91Reader=$expectedReader}
 [IO.File]::WriteAllText($receipt,($state|ConvertTo-Json -Depth 3),[Text.UTF8Encoding]::new($false))
 Write-Host 'Stage92 source installed; Stage91 retained.'
 Write-Host 'Back up Stage91 tested DLL 1,999,872 bytes; build x86 and check STAGE92_INSTALLED 3/3.'
} catch {
 if(Test-Path -LiteralPath $backup){Copy-Item -LiteralPath $backup -Destination $adapter -Force}
 foreach($p in @($observer,$receipt,$backup)){if(Test-Path -LiteralPath $p){Remove-Item -LiteralPath $p -Force}}
 throw
}
