# Stage93 r3 installer: identify the exact FinalCommandRecord field triplet.
# Apply ONLY to unmodified, source-verified Stage92 r3 Windows project.
param([Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
function Require([bool]$ok,[string]$message){if(-not $ok){throw $message}}
function Sha([string]$path){return (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()}
function ReplaceOne([string]$text,[string]$find,[string]$replace){
 $a=$text.IndexOf($find,[StringComparison]::Ordinal)
 Require ($a -ge 0) "Stage93 anchor absent: $find"
 Require ($text.IndexOf($find,$a+$find.Length,[StringComparison]::Ordinal) -lt 0) "Stage93 ambiguous duplicate: $find"
 return $text.Substring(0,$a)+$replace+$text.Substring($a+$find.Length)
}
function ReplaceScopedOne([string]$text,[string]$startMarker,[string]$endMarker,
                          [string]$find,[string]$replace){
 $cmp=[StringComparison]::Ordinal
 $a=$text.IndexOf($startMarker,$cmp)
 Require ($a -ge 0) "Stage93 scope start absent: $startMarker"
 Require ($text.IndexOf($startMarker,$a+$startMarker.Length,$cmp) -lt 0) "Stage93 scope start ambiguous: $startMarker"
 $b=$text.IndexOf($endMarker,$a+$startMarker.Length,$cmp)
 Require ($b -gt $a) "Stage93 scope end absent: $endMarker"
 Require ($text.IndexOf($endMarker,$b+$endMarker.Length,$cmp) -lt 0) "Stage93 scope end ambiguous: $endMarker"
 $scope=$text.Substring($a,$b-$a)
 $patched=ReplaceOne $scope $find $replace
 return $text.Substring(0,$a)+$patched+$text.Substring($b)
}
function InsertStage93RecordField([string]$text){
 # Target the actual member triplet, not a generic TickCount identifier.
 # Additional TickCount fields elsewhere in this source/record are immaterial.
 $cmp=[StringComparison]::Ordinal
 $marker='struct FinalCommandRecord'
 $a=$text.IndexOf($marker,$cmp)
 Require ($a -ge 0) 'Stage93 FinalCommandRecord declaration absent.'
 Require ($text.IndexOf($marker,$a+$marker.Length,$cmp) -lt 0) 'Stage93 duplicate FinalCommandRecord declarations.'
 $open=$text.IndexOf('{',$a+$marker.Length)
 Require ($open -ge 0 -and $open -lt $a+180) 'Stage93 FinalCommandRecord opening brace absent.'
 $close=$text.IndexOf('};',$open+1,$cmp)
 Require ($close -gt $open) 'Stage93 FinalCommandRecord closing brace absent.'
 $section=$text.Substring($open,$close-$open)
 $pattern='(?m)^[ \t]*DWORD[ \t]+ThreadId[ \t]*;[ \t]*\r?\n(?<indent>[ \t]*)DWORD[ \t]+TickCount[ \t]*;(?=[ \t]*\r?\n[ \t]*SYSTEMTIME[ \t]+LocalTime[ \t]*;)'
 $matches=[System.Text.RegularExpressions.Regex]::Matches($section,$pattern)
 Require ($matches.Count -eq 1) "Stage93 FinalCommandRecord exact ThreadId/TickCount/LocalTime triplet count=$($matches.Count); no files modified."
 $m=$matches[0]
 $newline=if($m.Value.Contains("`r`n")){"`r`n"}else{"`n"}
 $addition=$newline+$m.Groups['indent'].Value+'ULONGLONG Stage93Qpc;'
 $at=$open+$m.Index+$m.Length
 return $text.Substring(0,$at)+$addition+$text.Substring($at)
}
$rootFull=(Resolve-Path -LiteralPath $Root).Path
$src=Join-Path $rootFull 'src'
$stage81=Join-Path $src 'Stage81SelectorWriter.inl'
$stage92=Join-Path $src 'Stage92F5990BranchTrace.inl'
$stage93=Join-Path $src 'Stage93F5A68DirectBit3.inl'
$final=Join-Path $src 'AICommandSiteTrace.cpp'
$package81=Join-Path $PSScriptRoot 'src/Stage81SelectorWriter.inl'
$package93=Join-Path $PSScriptRoot 'src/Stage93F5A68DirectBit3.inl'
$back=Join-Path $rootFull '.stage93_source_backup'
Require ((Test-Path -LiteralPath $stage81 -PathType Leaf) -and (Test-Path -LiteralPath $stage92 -PathType Leaf) -and (Test-Path -LiteralPath $final -PathType Leaf)) 'Required source missing.'
Require (!(Test-Path -LiteralPath $stage93) -and !(Test-Path -LiteralPath $back)) 'Stage93 already present or backup exists. Refuse reapply.'
$expect81='6d914c14533913f7c22fd0c90b1162901c4e2a47f3cc144912be6d0288342bc8'
$expect92='34d4e6845594e1e107e412614211eddfd2a1e882ffe0e43669d14848e97f40f7'
Require ((Sha $stage81) -eq $expect81) 'Stage81 is not exact Stage92 r3 source; stop without writes.'
Require ((Sha $stage92) -eq $expect92) 'Stage92 is not exact r3 source (including Stage35-compatible guards); stop.'
Require ((Test-Path -LiteralPath $package81) -and (Test-Path -LiteralPath $package93)) 'Stage93 package incomplete.'
$enc=New-Object System.Text.UTF8Encoding($false)
$text=[System.IO.File]::ReadAllText($final,[System.Text.Encoding]::UTF8)
Require (!$text.Contains('Stage93Qpc') -and !$text.Contains('STAGE93_FINAL_QPC')) 'Final-command source already patched.'
# Isolate all mutations to the existing FinalCommand record producer/worker.
$text=InsertStage93RecordField $text
$text=ReplaceScopedOne $text 'void AppendLogAtTime(' 'void AppendLog(const char* message)' 'void AppendLogAtTime(const SYSTEMTIME* timestamp, const char* message)' 'void AppendLogAtTime(const SYSTEMTIME* timestamp, const char* message, const wchar_t* filename = L"ArcanaAI_FinalCommandTrace.log")'
$text=ReplaceScopedOne $text 'void AppendLogAtTime(' 'void AppendLog(const char* message)' 'GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_FinalCommandTrace.log")' 'GetSiblingPath(logPath, MAX_PATH, filename)'
$text=ReplaceScopedOne $text 'void QueueFinalCommandRecord(' 'void FlushReadyRecords()' 'record.TickCount = GetTickCount();' @'
record.TickCount = GetTickCount();
        LARGE_INTEGER stage93Qpc{};
        QueryPerformanceCounter(&stage93Qpc);
        record.Stage93Qpc = static_cast<ULONGLONG>(stage93Qpc.QuadPart);
'@
$text=ReplaceScopedOne $text 'void FlushReadyRecords()' 'bool MakeRelativeCall(' 'AppendLogAtTime(&record.LocalTime, message);' @'
AppendLogAtTime(&record.LocalTime, message);
            // Same original FinalCommand callsite and same pre-queue record.
            // QPC captured on game thread; file I/O stays on the existing worker.
            char stage93Line[460]{};
            sprintf_s(stage93Line,
                "STAGE93_FINAL_QPC build=2026-11-12-r1 seq=%ld side=P%lu tid=%lu player=%08lX "
                "qpc=%I64u tick=%lu returnRVA=%06lX type=%lu "
                "payloadESI=%08lX payloadArg=%08lX flag3B70=%08lX flagBit3=%lu",
                static_cast<long>(readIndex+1), record.Side,record.ThreadId,record.PlayerObject,
                record.Stage93Qpc,record.TickCount,record.CallReturnRva,record.EventType,
                record.PayloadRegister,record.PayloadArgument,record.Player.field3B70,
                (record.Player.field3B70 & 8u) ? 1u : 0u);
            AppendLogAtTime(&record.LocalTime,stage93Line,
                L"ArcanaAI_Stage93FinalCommandQpcTrace.log");
'@
# Only source files change. Do not change INI defaults or any game state.
New-Item -ItemType Directory -Path $back -ErrorAction Stop | Out-Null
Copy-Item -LiteralPath $stage81 -Destination (Join-Path $back 'Stage81SelectorWriter.inl')
Copy-Item -LiteralPath $final -Destination (Join-Path $back 'AICommandSiteTrace.cpp')
try {
 [System.IO.File]::WriteAllText($final,$text,$enc)
 Copy-Item -LiteralPath $package81 -Destination $stage81
 Copy-Item -LiteralPath $package93 -Destination $stage93
 $manifest=[ordered]@{
  stage81_original_sha=$expect81;stage92_verified_sha=$expect92
  final_original_sha=(Sha (Join-Path $back 'AICommandSiteTrace.cpp'))
  final_stage93_sha=(Sha $final);stage81_stage93_sha=(Sha $stage81);stage93_sha=(Sha $stage93)
 }
 [System.IO.File]::WriteAllText((Join-Path $back 'manifest.json'),($manifest|ConvertTo-Json),$enc)
 Write-Host 'STAGE93 SOURCE INSTALLED. Build x86; verify Stage93 and FinalCommand markers at runtime.'
} catch {
 Copy-Item -LiteralPath (Join-Path $back 'Stage81SelectorWriter.inl') -Destination $stage81 -Force
 Copy-Item -LiteralPath (Join-Path $back 'AICommandSiteTrace.cpp') -Destination $final -Force
 if(Test-Path -LiteralPath $stage93){Remove-Item -LiteralPath $stage93 -Force}
 Remove-Item -LiteralPath $back -Recurse -Force
 throw
}
