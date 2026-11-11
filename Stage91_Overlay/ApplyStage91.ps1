param([string]$Root='D:\INCUBATOR\ArcanaAIOverhaul')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
function Sha([string]$File){return (Get-FileHash -LiteralPath $File -Algorithm SHA256).Hash.ToLowerInvariant()}

$src=Join-Path $Root 'src'
$original81=Join-Path $src 'Stage81SelectorWriter.inl'
$original90=Join-Path $src 'Stage90AllF57D0.inl'
$original89=Join-Path $src 'Stage89F58E0Reader.inl'
$original88=Join-Path $src 'Stage88Stage24Join.inl'
$target91=Join-Path $src 'Stage91DirectSpecialReader.inl'
$overlay81=Join-Path $PSScriptRoot 'src\Stage81SelectorWriter.inl'
$overlay91=Join-Path $PSScriptRoot 'src\Stage91DirectSpecialReader.inl'
$state=Join-Path $Root 'Stage91InstallState.json'
$backup81="$original81.stage90r1.bak"
$base81='04b1d2644caf5312a8d53e3ec34a70c522c1929dcb6b05e8c3deb2a8c76af4ec'
$base90='080374d0551dee87551c0ffea1bd9ddfa4a58bad708f71814b36cc3d260b421a'
$base89='07ec1e7e4ed94bbe2c3e43f32bffd7a113a41f85c7d27318571b6e3107a889f2'
$base88='986b77b2fd07f67c92296ffb57bea0817dd2bf6b1758547da513f4d32c34a4bd'
foreach($p in @($original81,$original90,$original89,$original88,$overlay81,$overlay91)){
 if(!(Test-Path -LiteralPath $p -PathType Leaf)){throw "Stage91 missing $p; no changes made"}
}
foreach($p in @($target91,$backup81,$state)){
 if(Test-Path -LiteralPath $p){throw "Stage91 install would overwrite existing $p; no changes made"}
}
if((Sha $original81) -ne $base81){throw 'Stage91 requires EXACT Stage90 r1 Stage81 adapter, refusing changes'}
if((Sha $original90) -ne $base90){throw 'Stage91 requires EXACT Stage90 full-call observer, refusing changes'}
if((Sha $original89) -ne $base89){throw 'Stage91 requires EXACT Stage89 native reader source, refusing changes'}
if((Sha $original88) -ne $base88){throw 'Stage91 requires EXACT Stage90 Stage88 bridge, refusing changes'}
$updated81=[IO.File]::ReadAllText($overlay81)
foreach($needle in @('arcana_stage91::Install();','arcana_stage91::Flush();','arcana_stage91::Health();','#include "Stage91DirectSpecialReader.inl"', 'arcana_stage90::Start();')){
 if(!$updated81.Contains($needle)){throw "Stage91 malformed adapter $needle"}
}
$updated91=[IO.File]::ReadAllText($overlay91)
foreach($needle in @('0xF5A4Du','0xF5A5Cu','0xF5A76u','EnableStage91DirectSpecialReaderTrace','Stage91ReaderSiteMask','call dword ptr [g_Stage91F58E0NativeTarget]')){
 if(!$updated91.Contains($needle)){throw "Stage91 malformed observer $needle"}
}
if(Get-Process -Name 'AALib' -ErrorAction SilentlyContinue){throw 'Close AALib.exe before installing Stage91'}
Copy-Item -LiteralPath $original81 -Destination $backup81 -ErrorAction Stop
try {
 Copy-Item -LiteralPath $overlay91 -Destination $target91 -ErrorAction Stop
 Copy-Item -LiteralPath $overlay81 -Destination $original81 -Force -ErrorAction Stop
 $receipt=[ordered]@{stage='Stage91 r1';created=(Get-Date).ToString('o'); before81=$base81;
  after81=(Sha $original81);after91=(Sha $target91); base90=$base90;base89=$base89;base88=$base88}
 [IO.File]::WriteAllText($state,($receipt|ConvertTo-Json -Depth 5),[Text.UTF8Encoding]::new($false))
 Write-Host 'Stage91 source applied: three new F58E0 CALL-site observers (9/10/11), prior Stage87-90 retained.'
 Write-Host 'No EE306 duplicate; keep EnableStage36ControlToCommandTrace=0 and back up Stage90 1,991,168-byte DLL.'
 Write-Host 'Build with Windows MSVC x86 before testing. If not successful use RollbackStage91.ps1.'
} catch {
 if(Test-Path -LiteralPath $backup81){Copy-Item -LiteralPath $backup81 -Destination $original81 -Force}
 foreach($p in @($target91,$state,$backup81)){
  if(Test-Path -LiteralPath $p){Remove-Item -LiteralPath $p -Force}
 }
 throw
}
