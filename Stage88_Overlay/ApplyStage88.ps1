param([string]$Root = 'D:\INCUBATOR\ArcanaAIOverhaul')
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Sha256([string]$Path) { return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant() }
function AssertOnce([string]$Text, [string]$Needle, [string]$Description) {
    $a = $Text.IndexOf($Needle, [StringComparison]::Ordinal)
    if ($a -lt 0 -or $Text.IndexOf($Needle, $a + $Needle.Length, [StringComparison]::Ordinal) -ge 0) {
        throw "Stage88: missing or ambiguous $Description"
    }
    return $a
}
function AddAfterInFunction([string]$Text,[string]$Function,[string]$Guard,[string]$Added) {
    $nl = if ($Text.Contains("`r`n")) { "`r`n" } else { "`n" }
    $start = AssertOnce $Text ($Function+$nl+'{') $Function
    $pos = $Text.IndexOf($Guard,$start,[StringComparison]::Ordinal)
    if ($pos -lt 0 -or $pos - $start -gt 400) { throw "Stage88: unable to find guard inside $Function" }
    $nl = if ($Text.Contains("`r`n")) { "`r`n" } else { "`n" }
    return $Text.Insert($pos+$Guard.Length,$nl+'    '+$Added)
}

$src = Join-Path $Root 'src'
$names = @('Stage81SelectorWriter.inl', 'Stage87SelectorConsumer.inl', 'AIQueueTrace.cpp')
$baseline = @{
    'Stage81SelectorWriter.inl' = '29d5e7dfb53211a34e9edfffe365b2c6dd7c2d7b81f7c12a15cd482117a018ba'
    'Stage87SelectorConsumer.inl' = '5057ce5615e8d02df64209517b27fc1050d0c6b745a9457211ccc0cb60df84e7'
}
$statePath = Join-Path $Root 'Stage88InstallState.json'
if (Test-Path -LiteralPath $statePath) { throw 'Stage88 state exists; refusing duplicate apply' }
$original = @{}; $backups = @{}; $beforeHash = @{}
foreach ($name in $names) {
    $p = Join-Path $src $name
    if (!(Test-Path -LiteralPath $p)) { throw "Stage88: missing $p" }
    $beforeHash[$name] = Sha256 $p
    if ($baseline.ContainsKey($name) -and $beforeHash[$name] -ne $baseline[$name]) {
        throw "Stage88: $name not the exact Stage87 r1 source; no files modified"
    }
    $backups[$name] = "$p.stage87.bak"
    if (Test-Path -LiteralPath $backups[$name]) { throw "Stage88: backup already exists: $($backups[$name])" }
}
foreach ($name in @('Stage81SelectorWriter.inl','Stage87SelectorConsumer.inl','Stage88Stage24Join.inl')) {
    $p = Join-Path (Join-Path $PSScriptRoot 'src') $name
    if (!(Test-Path -LiteralPath $p)) { throw "Stage88: missing overlay $p" }
}
$newInl = Join-Path $src 'Stage88Stage24Join.inl'
if (Test-Path -LiteralPath $newInl) { throw 'Stage88 bridge already exists; no files modified' }
$aiPath = Join-Path $src 'AIQueueTrace.cpp'
$body = [IO.File]::ReadAllText($aiPath)
if ($body.Contains('Stage88Stage24Pre') -or $body.Contains('Stage88Stage24Post')) {
    throw 'Stage88 AIQueueTrace.cpp bridge already present; no files modified'
}
$decl = 'extern "C" void __cdecl ArcanaCaptureStage24PreState(DWORD* frame);'
$pos = AssertOnce $body $decl 'Stage24 prototype'
$nl = if ($body.Contains("`r`n")) { "`r`n" } else { "`n" }
$body = $body.Insert($pos+$decl.Length,$nl+'extern "C" void __cdecl Stage88Stage24Pre(const DWORD* frame);'+$nl+'extern "C" void __cdecl Stage88Stage24Post(const DWORD* savedContext);')
$body = AddAfterInFunction $body 'extern "C" void __cdecl ArcanaCaptureStage24PreState(DWORD* frame)' 'if (frame == nullptr) return;' 'Stage88Stage24Pre(frame);'
$body = AddAfterInFunction $body 'extern "C" void __cdecl ArcanaCaptureStage24PostState(const DWORD* savedContext)' 'if (savedContext == nullptr) return;' 'Stage88Stage24Post(savedContext);'
$raw=[IO.File]::ReadAllBytes($aiPath)
$bom=$raw.Length -ge 3 -and $raw[0] -eq 0xEF -and $raw[1] -eq 0xBB -and $raw[2] -eq 0xBF
$encoding=New-Object System.Text.UTF8Encoding($bom)
$wrote=@()
try {
    foreach ($name in $names) { [IO.File]::Copy((Join-Path $src $name),$backups[$name]); }
    foreach ($name in @('Stage81SelectorWriter.inl','Stage87SelectorConsumer.inl')) {
        [IO.File]::Copy((Join-Path (Join-Path $PSScriptRoot 'src') $name),(Join-Path $src $name),$true)
        $wrote += $name
    }
    [IO.File]::Copy((Join-Path (Join-Path $PSScriptRoot 'src') 'Stage88Stage24Join.inl'),$newInl)
    [IO.File]::WriteAllText($aiPath,$body,$encoding)
    $wrote += 'AIQueueTrace.cpp'
    $after = @{}
    foreach ($name in $names) { $after[$name] = Sha256 (Join-Path $src $name) }
    $state = @{ stage='Stage88 r1'; created=(Get-Date).ToString('o'); before=$beforeHash; after=$after; bridgeSha= (Sha256 $newInl) }
    [IO.File]::WriteAllText($statePath,($state | ConvertTo-Json -Depth 5),[Text.UTF8Encoding]::new($false))
    Write-Host 'STAGE88 APPLY OK - original Stage24 EE306 hook retained, no second hook.'
    Write-Host 'Check EnableStage24ResolverConsumerTrace=1 and EnableStage36ControlToCommandTrace=0 before Build.bat.'
} catch {
    foreach ($name in $names) {
        if (Test-Path -LiteralPath $backups[$name]) { [IO.File]::Copy($backups[$name],(Join-Path $src $name),$true) }
    }
    if (Test-Path -LiteralPath $newInl) { Remove-Item -LiteralPath $newInl -Force }
    throw
}