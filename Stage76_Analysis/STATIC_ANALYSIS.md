# Stage76 — bounded after-callback phase probes for selector P+3F50 (SOURCE READY)

**2026-11-11 — STATIC SOURCE VERIFIED, WINDOWS x86 BUILD AND GAME RUNTIME PENDING.** New branch based on Stage75. Original private `AALib.exe` SHA-256 **`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`** rechecked locally. Exact original **three native E8 callsites**, seven surrounding opcode ranges and all three relative call destinations verified in this original PE. [Static verifier](tools/verify_stage76.py). **No DLL built, no user runtime tested, no new game logs, no gameplay/AI field writes or DR access.** Call wrapper changes the callee-visible return address (and instrumented timing); safety/behavioral equivalence remains an untested assumption until Windows runtime.

## 1. Why Stage74 cannot attribute writer EIP

Stage74 [runtime evidence](../Stage74_Runtime/RUNTIME_ANALYSIS.md) recorded **3,708** valid Stage38 callback-window snapshots, **zero** selector f30 changes within the original callback boundaries, and **five** between-window P+3F50 transitions: P2 `0→1,1→0`; P1 `0→1,1→2,2→1`. Every following PRE saw callback `158570` with parameter `0x11`, but the value was **already set before the callback**. Stage75 [static analysis](../Stage75_Analysis/STATIC_ANALYSIS.md) traced native event-code handoff `P+3A36→158E90→P+3ED4`; it is distinct from the numeric selector origin.

Original Stage38 POST runs **before** native `FC60`. There is **more original code** before next Stage38 PRE that can change selector or event state. Without more boundaries, an attribution to `155D29` or any register-derived MOV source is conjecture.

## 2. Exact original native checkpoint targets

Original PE preferred base `00400000`, runtime base relocates; all addresses below are **RVAs**. Native code has:

```asm
EE7BE  CALL EDI                 ; native callback, Stage38 PRE/POST around this
EE7C7  E8 94 14 F2 FF          ; CALL FC60  (already replaced by existing Stage38 hook)
EE7CC  ADD ESP,4
EE7CF  MOV ECX,ESI             ; ECX=P
EE7D1  E8 AA FC FF FF          ; CALL EE480: event-record update
EE7D6  E8 C5 A5 FF FF          ; CALL E8DA0: further per-player update
EE7DB  MOV ECX,[ESI+3B7C]
EE7E1  SHR ECX,17h
EE7E4  TEST CL,1
EE7E7  JE EE7F2               ; skip conditional event processing
EE7E9  PUSH ESI                ; native EE5E0 argument P
EE7EA  E8 F1 FD FF FF          ; CALL EE5E0 (conditional)
EE7EF  ADD ESP,4               ; native caller cleans P argument
EE7F2  AND [ESI+3B7C],DFFFFFFF
```

**Stage76 replaces exactly the three original five-byte CALLs** at `EE7D1`, `EE7D6`, `EE7EA` with same-size E8 to three passive wrappers; it does not touch `EE7C7` (already patched by Stage38), `EE7BE`, the conditional gate, or selector writes. Each wrapper snapshots original fields **immediately before and after the corresponding original callee**, calls original target exactly once and returns to the original continuation RVA (`EE7D6/EE7DB/EE7EF`). The `EE7EA` wrapper re-pushes the *existing* original `[ESP+4]` actor argument for the real EE5E0 call, cleans only the copy, then returns so the native `EE7EF ADD ESP,4` cleans the original argument normally.

| Phase | Original CALL RVA | Actual target | PRE/POST brackets | Why helpful |
|---|---|---|---|---|
| **0** | `EE7D1` | `EE480` | native event record update | PRE also brackets the end of native FC60 relative to Stage38 POST |
| **1** | `EE7D6` | `E8DA0` | native player state update | Tests a second non-callback update stage |
| **2** | `EE7EA` | `EE5E0` | gated event processing/callback registration | Could show selector or callback registration under P+3B7C bit23 gate |

**No new DR0..DR3 claim or VEH**; existing Stage59 DR0/DR1 remains the only relevant X watch owner. Stage76 is a separate software CALL-site observer, not a selector write detector. Native ABI/register preservation is the intent, not yet experimentally established.

## 3. Precisely what is changed in source

The source overlay has **two files**, to copy side by side into the local Stage74 project `src` folder:

1. [Stage76_Overlay/src/Stage76AfterCallbackPhases.inl](../Stage76_Overlay/src/Stage76AfterCallbackPhases.inl): **new** passive phase logger/wrappers and all-three-site transactional patch installer. Captures P1/P2 only, records native TID, original return RVA, object identity, two QPC boundaries and read masks. Uses TLS depth, SRWLOCK ring (8,192 records), and health counters for drop, overflow, mismatch and read failures. Source blob **`4d1fad129a6269c764d6950fd8a8269bb6b18189`**.
2. [Stage76_Overlay/src/Stage38CallbackWindow.inl](../Stage76_Overlay/src/Stage38CallbackWindow.inl): exact copy of **Stage74 tested source** with only **three** integration edits: add one include after Stage70, conditionally install Stage76 after a successful Stage70 install and before Stage59 worker starts, and flush Stage76 from the existing periodic Stage38 flush. Stage74 original blob **`106dca56d662fe1c8e0b7083013a535b67cde3c8`**, new adapter blob **`f439a0f35e08891590acdb6099afd04ed8d821a4`**. Stage38's original 32 fields and f30/f31 layout remain intact.

The new installer requires Stage38 installed 1/1 aggregate, Stage70 installed 1, Stage59 not yet started, x86 CPU with XMM, and exact opcode guards at the three E8 sites **plus surrounding native instructions**. It takes the existing named patch-install mutex, suspends peer threads, rejects any EIP inside the three patch sites, makes one executable page writable, replaces all three same-size CALLs, flushes instruction cache, restores protection, resumes threads and logs `STAGE76_INSTALLED 3/3` only after an all-or-nothing success. If a cache/protection failure occurs it attempts rollback while peers remain suspended, logs ERROR and does not claim success. This implementation is **source-only**; test coverage of Windows API failures and live thread races is not established.

**Important integration constraint:** DO NOT copy the Stage76 `Stage38CallbackWindow.inl` on top of an older Stage70 r2 project without Stage74's 32-field extension, and DO NOT use Stage76 source without both files together. The Stage76 adapter fully contains the Stage74 adapter; its Git blob differs by exactly the three recorded integration edits.

## 4. Output format and timeline interpretation

`ArcanaAI_Stage76AfterCallbackTrace.log` is new. It emits `STAGE76_GUARD`, `STAGE76_INSTALLED 3/3`, `STAGE76_PHASE` and `STAGE76_HEALTH`. Each phase record has:

| Stage76 field | Original player address | Purpose |
|---|---|---|
| f0 | **P+3F50** | numeric selector — the primary value |
| f1 | P+3F54 | selector companion state |
| f2 | P+3A2C | event-record status/current-mode packed DWORD |
| f3 | P+3A30 | current event code packed DWORD |
| f4 | P+3A34 | prior mode and prior code packed WORD pair; high 16 bits = **P+3A36** |
| f5 | P+3ED0 | callback descriptor pointer |
| f6 | P+3ED4 | callback parameter |
| f7 | P+3B7C | event flags including native conditional EE5E0 gate |
| f8 | P+3F4C | immediate action request |
| f9 | P+4 | current object identity |

Require valid `preMask/postMask` bit 0 before concluding selector movement; prefer all ten fields mask `3FF`, check health values `dropped=overflow=mismatch=readFail=0`. Follow **same actor P/O/TID and QPC**; never join cross-session sequences or treat Stage59 log line order as chronological (it had a 14-line ring-flush reorder in Stage74).

Interpretation:
- If first Stage76 phase0 PRE selector already differs from previous Stage38 POST, change occurred **during FC60 or the narrow unsampled gap**, not necessarily inside EE480.
- If a phase record has f0 `PRE != POST`, change occurred somewhere within that *native callee execution* (does NOT directly identify the nested native writer IP).
- If phase0 POST equals phase1 PRE and phase1 POST equals phase2 PRE but next Stage38 PRE differs, seek another unsampled native step or another actor's code; `EE5E0` might not be invoked when the bit gate is false.
- `P+3ED0` / `P+3ED4` changes during phase2 can demonstrate event-to-callback registration within that call, but remain distinct from selector production.

## 5. Windows installation (not yet tested)

Preserve the tested **Stage74 `dinput8.dll` 1,893,376 bytes** and the old local `src/Stage38CallbackWindow.inl` as rollback. Copy the two Stage76 overlay `.inl` files into the existing working project's `D:\INCUBATOR\ArcanaAIOverhaul\src` directory, overwriting **only Stage38CallbackWindow.inl** and adding Stage76AfterCallbackPhases.inl. Leave `ArcanaAI.ini`, every other C++ file/Stage70 source and gameplay settings untouched.

From a checkout of this exact branch, optionally validate the original local EXE and two overlay source hashes:

```powershell
py -3 .\Stage76_Analysis\tools\verify_stage76.py "D:\PATH\TO\AALib.exe"
```

Then build with `D:\INCUBATOR\ArcanaAIOverhaul\Build.bat`. Require **BUILD SUCCEEDED**; expected DLL size **unknown** until the build log. Confirm `STAGE38_INSTALLED 2/2`, `STAGE70_INSTALLED 1/1`, `STAGE76_INSTALLED 3/3` and Stage59 `WATCH_ARMED`. Run one normal COM-vs-COM session only if the build and install markers succeed. Upload from **that same run**: `Build.log`, `ArcanaAI_Stage76AfterCallbackTrace.log`, `ArcanaAI_Stage38CallbackWindowTrace.log`, `ArcanaAI_Stage65RequestOriginTrace.log`, `ArcanaAI_Stage70DeferredWriterTrace.log` and `ArcanaAI_Stage59XWriterWatchTrace.log`. A Stage76 failure without a game run needs only Build.log and available initial trace. If crashes, performance regressions, bad health counters, or game behavior changes occur, **restore the Stage74 adapter and its tested DLL**; do not attempt to repair by enabling extra DR modes.

## 6. Limits and next stage

**Stage76 status SOURCE READY ONLY.** Static checks support the three native-call target/continuation addresses and an exact three-edit Stage74 integration delta. Nothing here proves Windows compilation, native runtime/ABI equivalence, actual transition phase, the selector writer EIP, AI skill improvement, or a reliable general-purpose mod. After a clean one-run Stage76 capture, Stage77 should correlate phase boundaries to the five known *kinds* of transitions and then, **only if a phase is narrowed**, consider one bounded exact-original-byte writer observer. Preserve the original private EXE out of the public repository and maintain separate rollback DLLs.
