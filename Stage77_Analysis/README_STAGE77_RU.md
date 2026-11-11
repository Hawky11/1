# Stage77 — selector before/after native 156C00

**2026-11-11 — SOURCE READY, Windows x86 build/runtime NOT YET TESTED.**

Stage76 verified 11,478 phase observations, five changes to P+3F50 between Stage38 windows, and no within-call changes in EE480/E8DA0. The unobserved interval includes native 156C00 before Stage38 PRE. Stage77 adds an observational before-native boundary to the existing Stage38 prepare wrapper; it does **not** create any native patch site, DR watchpoint or player/AI field write.

## Source and installation

Use the single replacement [Stage38CallbackWindow.inl](../Stage77_Overlay/src/Stage38CallbackWindow.inl) with the already installed **Stage76** `Stage76AfterCallbackPhases.inl`. Copy just this new file to:

`D:\INCUBATOR\ArcanaAIOverhaul\src\Stage38CallbackWindow.inl`

Leave **Stage76AfterCallbackPhases.inl**, Stage70, Stage59, all other sources and `ArcanaAI.ini` unchanged. Back up the current Stage76 Stage38 source and the working **1,901,568-byte** Stage76 `dinput8.dll` before rebuilding. The fallback Stage74 DLL was **1,893,376 bytes**. Do not apply this adapter to a Stage70-only project.

For optional source-only sanity checks, run from this branch:

```powershell
py -3 .\Stage77_Analysis\tools\verify_stage77.py
```

Then `D:\INCUBATOR\ArcanaAIOverhaul\Build.bat`; require **BUILD SUCCEEDED**. A new DLL size cannot be predicted until the actual build.

## Precise observation

Existing Stage38 `Stage38PrepareHook` is already substituted for the original near CALL at `AALib+EE7AE` to `AALib+156C00`. Stage77 adds a **PUSHFD/PUSHAD/FXSAVE** protected call to `Stage77BeforeNativePrepare` immediately before the existing **single** invocation of original 156C00. It samples five DWORDs:

| n | Before-native address | Existing Stage38 PRE field after native |
|---|---|---|
| n0 | P+3F50 selector | f30 |
| n1 | P+3F54 companion state | f31 |
| n2 | P+3ED0 registered callback | f3 |
| n3 | P+3ED4 callback argument | f4 |
| n4 | P+4 object identity | f0 |

TLS depth 16 pairs the same actor/frame/context/TID across the native call. After 156C00, `Stage38Prepared` consumes the matching TLS entry and the existing Stage38 PRE snapshot captures the after values. Existing callback and FC60 coverage is preserved. The Stage38 log gets one `STAGE77_NATIVE_PREPARE` line per corresponding Stage38 window, sharing `seq`, P1/P2 and QPC. `beforeMask` and `afterMask` should both be `1F`; `changedMask` bit 0 is the key test.

**Caveat:** The existing Stage38 wrapper already changes the original callee-visible return address and Stage77 adds memory reads/time overhead. Preservation of EFLAGS/GPR/x87/SSE is intentional, but behavioral and ABI safety are **not** proven by static review. Do not claim tested equivalence until a clean Windows runtime.

## Expected outcome

- If all five kinds of transition have n0 **old → new**, 156C00 or a nested callee is the narrowed writer window (still not a direct EIP).
- If n0 is **new → new** while the preceding Stage76 phase1 POST is old, the change occurred **before native 156C00** in unsampled engine logic.
- If n0 **old → old** but Stage38 next PRE shows new, treat it as a mismatch/invalid capture and investigate the snapshot pairing; the two snapshots ought to represent the same after-native point.
- Compare only one process session, identical P/O/TID and QPC; do not combine old Stage76 seq numbers from different runs.

Inspect `STAGE77_HEALTH calls=... overflow=0 mismatch=0 readFail=0`, `STAGE38_INSTALLED 2/2`, `STAGE77_ENABLED`, `STAGE76_INSTALLED 3/3`, `STAGE70_INSTALLED 1/1` and Stage59 `WATCH_ARMED`.

Run one ordinary COM-vs-COM test after successful build/install, then upload from that **same** run: `Build.log`, `ArcanaAI_Stage38CallbackWindowTrace.log`, `ArcanaAI_Stage76AfterCallbackTrace.log`, `ArcanaAI_Stage65RequestOriginTrace.log`, `ArcanaAI_Stage70DeferredWriterTrace.log`. Stage59 X log is only needed if its health or watchpoint behavior regresses. If the build fails, only Build.log is needed.

If crash, hang, mismatches or performance regressions occur, restore the backed-up Stage76 adapter and known-good DLL; do not enable extra DR registers or change gameplay/AI code.

**Status: source patch only; no Windows build or game observations for Stage77 yet.**