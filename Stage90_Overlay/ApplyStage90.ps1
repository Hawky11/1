param([string]$Root='D:\INCUBATOR\ArcanaAIOverhaul')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
function Sha([string]$File) {return (Get-FileHash -LiteralPath $File -Algorithm SHA256).Hash.ToLowerInvariant()}

$src=Join-Path $Root 'src'
$original81=Join-Path $src 'Stage81SelectorWriter.inl'
$original88=Join-Path $src 'Stage88Stage24Join.inl'
$original89=Join-Path $src 'Stage89F58E0Reader.inl'
$target90=Join-Path $src 'Stage90AllF57D0.inl'
$overlay81=Join-Path $PSScriptRoot 'src\Stage81SelectorWriter.inl'
$overlay88=Join-Path $PSScriptRoot 'src\Stage88Stage24Join.inl'
$overlay90=Join-Path $PSScriptRoot 'src\Stage90AllF57D0.inl'
$state=Join-Path $Root 'Stage90InstallState.json'
$backup81="$original81.stage89r1.bak"
$backup88="$original88.stage89r1.bak"
$baseline81='ace3b4d3a38fccf40813ea1d923ae5b933ef7d9a49c9c4f4e2c4598cf4404088'
$baseline88='f5d0e2ca436398575309b91fb87977e820609810f88d42a2a677ef6c528e3fbc'
$baseline89='07ec1e7e4ed94bbe2c3e43f32bffd7a113a41f85c7d27318571b6e3107a889f2'
foreach($p in @($original81,$original88,$original89,$overlay81,$overlay88,$overlay90)){
 if(!(Test-Path -LiteralPath $p -PathType Leaf)){throw "Stage90 missing $p; no changes made"}
}
foreach($p in @($state,$backup81,$backup88,$target90)){
 if(Test-Path -LiteralPath $p){throw "Stage90 install would overwrite existing $p; no changes made"}
}
if((Sha $original81) -ne $baseline81){throw 'Stage90 requires exact Stage89 r1 Stage81 source; nothing changed'}
if((Sha $original88) -ne $baseline88){throw 'Stage90 requires exact Stage89 r1 Stage88 bridge; nothing changed'}
if((Sha $original89) -ne $baseline89){throw 'Stage90 requires exact Stage89 r1 reader source; nothing changed'}
$updated81=[IO.File]::ReadAllText($overlay81)
$updated88=[IO.File]::ReadAllText($overlay88)
foreach($needle in @('arcana_stage90::Start();','arcana_stage90::Flush();','arcana_stage90::Health();','#include "Stage90AllF57D0.inl"')){
 if(!$updated81.Contains($needle)){throw "Stage90 malformed source: $needle"}
}
if(!$updated88.Contains('arcana_stage90::Pre(frame);arcana_stage88::Pre(frame);') -or
   !$updated88.Contains('arcana_stage88::Post(saved);arcana_stage90::Post(saved);')){
 throw 'Stage90 bridge missing exact before/after wiring'
}
if(Get-Process -Name 'AALib' -ErrorAction SilentlyContinue){throw 'Close the game before installing Stage90'}
Copy-Item -LiteralPath $original81 -Destination $backup81 -ErrorAction Stop
try{
 Copy-Item -LiteralPath $original88 -Destination $backup88 -ErrorAction Stop
 Copy-Item -LiteralPath $overlay90 -Destination $target90 -ErrorAction Stop
 Copy-Item -LiteralPath $overlay81 -Destination $original81 -Force -ErrorAction Stop
 Copy-Item -LiteralPath $overlay88 -Destination $original88 -Force -ErrorAction Stop
 $receipt=[ordered]@{
   stage='Stage90 r1';created=(Get-Date).ToString('o')
   before81=$baseline81;before88=$baseline88
   after81=(Sha $original81);after88=(Sha $original88);after90=(Sha $target90)
 }
 [IO.File]::WriteAllText($state,($receipt|ConvertTo-Json -Depth 5),[Text.UTF8Encoding]::new($false))
 Write-Host 'Stage90 applied: new source + reusing existing Stage24 EE306 hook. No new native patches.'
 Write-Host 'Build with Windows MSVC x86. Keep EnableStage36ControlToCommandTrace=0, Stage24/88 enabled.'
} catch {
 if(Test-Path -LiteralPath $backup81){Copy-Item -LiteralPath $backup81 -Destination $original81 -Force}
 if(Test-Path -LiteralPath $backup88){Copy-Item -LiteralPath $backup88 -Destination $original88 -Force}
 if(Test-Path -LiteralPath $target90){Remove-Item -LiteralPath $target90 -Force}
 if(Test-Path -LiteralPath $state){Remove-Item -LiteralPath $state -Force}
 if(Test-Path -LiteralPath $backup81){Remove-Item -LiteralPath $backup81 -Force}
 if(Test-Path -LiteralPath $backup88){Remove-Item -LiteralPath $backup88 -Force}
 throw
}
