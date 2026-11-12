## Stage93 r2 installer correction (2026-11-12)

r1 installer failed safely before touching source: `Stage93 ambiguous duplicate: DWORD TickCount;`. The working `AICommandSiteTrace.cpp` has more than one globally matching field. The r2 `ApplyStage93.ps1` restricts edits to `FinalCommandRecord`, `AppendLogAtTime`, `QueueFinalCommandRecord` and `FlushReadyRecords`, requiring uniqueness **within** each target scope. Stage92 r3 exact SHA256 gates and reversible backup are unchanged. No native Stage93 hook implementation was modified. Archived FinalCommand structural test with injected unrelated duplicate members/statements and a fail-closed in-scope duplicate passed 3/3; Windows PowerShell/MSVC/game testing remains pending.

---

# Stage93 r1 — F5A68 native bit3 OR + FinalCommand QPC bridge (SOURCE ONLY)

**Status:** static/source verification only. Windows x86 MSVC build and runtime installation have NOT yet been tested. Preserve the working **Stage92 r3 DLL (2,011,648 bytes)** and last tested Stage91 DLL (1,999,872 bytes). **Stage36 disabled.**

## Motivation
Stage92 r3 confirmed 17,788 F5990 ticks and 9,786 helper events. P+3B70 bit3 rose 127 times, but only 8 rises coincided with captured SET_9ED20; the other 119 occurred without a SET helper. This does **not** imply an unknown out-of-function writer. Original AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` has an explicit native writer at **RVA F5A68**:

```asm
F5A42: jne F5A68                      ; +3B70 bit2 set
F5A4D: call F58E0                     ; bit index 9
F5A57: jne F5A68                      ; index9 AL1
F5A5C: call F58E0                     ; bit index 10
F5A66: je F5A73                       ; index10 AL0
F5A68: or dword ptr [esi+3B70h], 8    ; 83 8E 70 3B 00 00 08
F5A6F: pop edi; pop esi; pop ebp; ret
F5A76: call F58E0                     ; bit index 11
F5A86: call 9ED20                      ; Stage92 helper SET
F5A8F: call 9ED60                      ; Stage92 helper CLEAR
```

Stage93 instruments **only F5A68**, retaining the exact original OR once. It records actual pre/post P+3B70, actor/P1-P2, thread, QPC, parent Stage92 tick, flags and bit3 0→1 transitions. Installation checks original bytes and neighboring branches/return, fingerprint, completed Stage92 installation, patch mutex and peer EIP. New records go to `ArcanaAI_Stage93DirectBit3Trace.log` as `STAGE93_OR`, with periodic `STAGE93_HEALTH`. No synthetic input, game state writes beyond the replayed original OR, DR writes or other new detours.

The accompanying source-only ZIP distributed in the Stage93 ChatGPT conversation contains a SHA-gated `ApplyStage93.ps1` and `RollbackStage93.ps1`; it patches the existing `AICommandSiteTrace.cpp` final-command hook to capture **game-thread QPC** and output companion `ArcanaAI_Stage93FinalCommandQpcTrace.log` worker records (`STAGE93_FINAL_QPC`). It does **not** add a second F7C5D hook or change historic FinalCommand logs.

## Build/run procedure

1. Close AALib.exe, back up the working Stage92 r3 project and 2,011,648-byte DLL.
2. Extract the Stage93 source-only ZIP provided in the conversation. In the ZIP directory:
   `powershell -ExecutionPolicy Bypass -File .\ApplyStage93.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'`
3. Build with MSVC x86 using existing Build.bat; inspect Build.log. Keep `EnableStage36ControlToCommandTrace=0`, Stage91 and Stage92 enabled. Stage93 INI `EnableStage93DirectBit3Trace` defaults to 1.
4. Short COM-vs-COM run first: require `STAGE92_INSTALLED 3/3`, `STAGE93_INSTALLED 1/1`, clean `STAGE93_HEALTH`, and `STAGE93_FINAL_QPC`. Only then do 3–5 natural rounds.
5. Upload Build.log; both Stage93 logs; Stage92, Stage91, Stage90, FinalCommand, EventDispatch same-session logs. Join by same player, TID and raw QPC. Nearest command is a *temporal correlation*, not proof that bit3 caused it.
6. On failure/health errors use `RollbackStage93.ps1`, rebuild Stage92 r3 or restore tested DLL.

**Verification:** offline original-EXE/static signature checks **32/32 PASS**, independently assembled x86 OR bytes match, synthetic correlator **2/2 PASS**. MSVC compile and runtime remain **unverified**.

See the ZIP README and analysis for the complete source installer and rollback, and [Stage92 live analysis](https://github.com/Hawky11/1/blob/main/Stage92_Runtime/RUNTIME_ANALYSIS.md) for the preceding evidence.
