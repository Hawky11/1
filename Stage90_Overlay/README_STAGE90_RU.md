# Stage90 r1 — full native F57D0 bitmap clear audit (SOURCE ONLY)

**Status:** static verification **PASS** (30 PE/source checks, original AALib.exe SHA-pinned); offline correlator positive/negative tests **PASS**. **NOT built or runtime-tested on Windows**. The preceding Stage89 Windows x86 DLL (1,984,000 bytes) remains last tested build. No COM AI/gameplay change is claimed.

## Why Stage90 is needed

Stage88 saw 170 exact selector→native F57D0 outputs, including **27 events for selector=2 and 1 real bit0x800 0→1**. Stage89 installed four native F58E0 reader hooks but saw only four calls at AAB55 (AL=0), all with `P+3A0C & 0xE00 == 0`. Existing Stage88 only logged the native EE2F0 EDI=0x2C branch; **1,444 other F57D0 calls were filtered** in the last run. Those may explain how the special bits were cleared between samples.

Original native F57D0 disassembly (SHA-verified binary):

```asm
F57EC TEST BYTE PTR [table + selector*3], 1
F57F6 JE F5800
F57F8 XOR EAX, EAX
F57FA MOV [EDI+4],EAX  ; P+3A0C=0 when EDI=P+3A08
F57FD MOV [EDI+8],EAX  ; P+3A10=0
F5800 MOV [EDI],ESI   ; P+3A08=selector command code
...    F5856/F58AC can OR a new bit into bitmap
```

The existing Stage24 **EE306→F57D0** native wrapper already executes the native helper once and invokes Stage88's pre/post bridge. Stage90 extends **that same C++ bridge**, not the native hook, and samples all F57D0 P1/P2 calls. Thus `clearMask` and `setMask` are *net* special bit transitions, not claims that the internal zero instruction never ran when a bit was re-added.

## What the package contains

- `src/Stage90AllF57D0.inl`: read-only Stage24 Pre/Post observer for both players, 16-level TLS depth guard, worker-only ring log with 8192 records, full output triples, table descriptors, dispatcher metadata, QPC and optional same-thread `stage87Seq` (does NOT consume Stage87's handoff).
- `src/Stage88Stage24Join.inl`: **only** adds Stage90 Pre before existing Stage88 Pre, Stage90 Post after existing Stage88 Post. Original Stage88 code is preserved.
- `src/Stage81SelectorWriter.inl`: adds Stage90 start/flush/health and includes the new file; retains Stage89 and older instrumentation.
- `ApplyStage90.ps1` and `RollbackStage90.ps1`: SHA-check exact Stage89 r1 source, back up both existing `.inl` files and restore safely; refuse unexpected changes. Runtime PE signature still validated by Stage81/89.
- `tools/verify_stage90.py`: SHA pinned original AALib.exe (also accepts AALib.zip) and 30 exact signature/bridge checks.
- `tools/correlate_stage90.py`: offline strict Stage88 inside Stage90 containment, net bit set/clear audit and nearest earlier Stage90 output for Stage89 (only temporal context).
- `tools/test_correlate_stage90.py`: synthetic positive and negative tests.
- `analysis/STATIC_ANALYSIS.md`: explain native clear/reset and caveats.

## Install and test on Windows

1. **Close the game**. Preserve known-good Stage89 `build\dinput8.dll` (**1,984,000 bytes**) and copy full source folder as a backup. Ensure Stage89 r1 overlay is installed, not merely compiled; exact source hashes are verified by the script.
2. In PowerShell, execute `powershell -ExecutionPolicy Bypass -File .\ApplyStage90.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'` from this package.
3. In `ArcanaAI.ini`, section `[AIResearch]`, keep `EnableStage24ResolverConsumerTrace=1`, `EnableStage88=1`, `EnableStage89F58E0ReaderTrace=1`, `EnableStage90AllF57D0Trace=1`, `Stage90LogAllF57D0=1`. **Keep `EnableStage36ControlToCommandTrace=0`** (Stage82 callsite collision).
4. Build with existing `Build.bat` **MSVC Windows x86**. Before running, verify successful build and that the previous DLL backup exists. Use 3–5 COM-vs-COM rounds; no synthetic inputs and no manual changes to bitmap.
5. Upload the same-session `Build.log`, `ArcanaAI_Stage90AllF57D0Trace.log`, `ArcanaAI_Stage88Stage24JoinTrace.log`, `ArcanaAI_Stage87SelectorConsumerTrace.log`, `ArcanaAI_Stage89F58E0ReaderTrace.log`, `ArcanaAI_Stage24ResolverConsumerTrace.log`, `ArcanaAI_FinalCommandTrace.log` and `ArcanaAI_EventDispatchTrace.log` to GitHub.
6. Offline verification: `python tools/correlate_stage90.py ArcanaAI_Stage90AllF57D0Trace.log --stage88 ArcanaAI_Stage88Stage24JoinTrace.log --stage89 ArcanaAI_Stage89F58E0ReaderTrace.log --output stage90_results.json`.
7. If the game hangs/crashes, build fails or any HEALTH count (`mismatch`, `readFail`, `depthOverflow`, `dropped`) is nonzero, stop the test. Run `RollbackStage90.ps1`, rebuild x86 and restore known-good DLL as required.

**Expected evidence, not assumed outcome:** Stage90 may show an ordinary native F57D0 call that clears a Stage88 special bit, or may find none (meaning an outside writer needs localization). An AAB55 `AL=0` does not prove an actual E8560 call or command execution. Stage90 does not instrument the generic F57A0 initializer or every other possible bitmap writer.
