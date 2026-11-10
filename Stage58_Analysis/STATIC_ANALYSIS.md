# Stage58 static analysis — ACDF0 pre-tail conditional native call probes

**SOURCE READY; Windows MSVC x86 build and in-game runtime NOT TESTED.** Date: 2026-11-10.

## Motivation from verified same-run Stage57

[Stage57 runtime report](https://github.com/Hawky11/1/blob/main/Stage57_Runtime/RUNTIME_ANALYSIS.md) identifies 744 net raw X changes inside ACDF0, split into 327 before ACFD6 and 420 inside the final ACFD6→15CEC0 call (3 outer windows changed in both). Within the pre-tail gap 112 X changes passed through Stage56 original ACF2A; **215** (P1 9 / P2 206) had **no** Stage56 X-store event. Stage58 narrows these 215 by passively observing three conditional original native CALL sites. It does not presume the candidate callees are writers.

## Original exact PE evidence

Private original AALib.exe: **175,556,608 bytes**, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, PE32/x86 ImageBase `00400000`. The following bytes and rel32 targets were independently checked directly from the private binary:

| CALL-site RVA | Original bytes | Callee RVA | ABI |
|---|---|---|---|
| **ACF0F** | `E8 8C B7 00 00` | **B86A0** | cdecl, two stack dwords, callee plain RET, caller `ADD ESP,8` |
| **ACF7A** | `E8 21 B7 00 00` | **B86A0** | same cdecl two-argument function |
| **ACF8A** | `E8 F1 CA 03 00` | **E9A80** | thiscall `ECX=ESI`, one stack dword `0`, native `RET 4` |

- B86A0 prologue: `55 8B EC 56 8B 75 08 83 FE FF`. Three sampled native exit branches at RVA B8703, B8732, B876B use plain `C3`.
- E9A80 prologue: `55 8B EC 8B 45 08 56 8B 74 81 08`. Six sampled branches including E9A93, E9AB3, E9ACF, E9ADF, E9AFE, E9B5E use `C2 04 00` (RET 4).
- Scanned **212,051 HIGHLOW PE relocation entries**, with **zero overlap** with the three instruction-aligned CALL signatures or two checked native prologues. These checks do **not** prove all indirect call-tree effects are read-only.
- Other conditional calls within the pre-tail gap include **ACED5/ACF53→1CB8B0** and **ACEE8/ACF60→1CB850**. Their small native disassemblies perform reads/boolean tests and are not chosen for initial Stage58 probes. Original 9EE10/9EEA0 enumerators also appear read-only. A static read-only impression is not a proof of an entire call tree's lack of side effects.

## Instrumentation design

- Adds `Stage58_Overlay/src/Stage58ACDF0ConditionalCalls.inl` and adapter `Stage58_Overlay/src/Stage38CallbackWindow.inl` on branch `stage58-acdf0-pre-tail-conditional-calls`.
- **Three original-once five-byte CALL-site replacements**, with signature checks before and during a thread-suspended patch transaction; no loop branch or direct coordinate instruction changed.
- B86A0 hook copies both original stack arguments in reverse order onto the native stack, invokes original exactly once and discards **only duplicated arguments** with `LEA ESP,[ESP+8]` (preserves native EFLAGS), then returns with plain `RET`; original caller retains and cleans the original arguments.
- E9A80 hook preserves original ECX, pushes a duplicate original argument, invokes native once (native RET 4 removes copied argument) and returns with `RET 4` to clean the caller's original stack argument.
- Uses Stage57's already-tested 13-field `S57Snap` for raw P1/P2 objects before/after each callee, plus QPC, site, thread, per-row native EAX return value, field masks, health counters. Saves/restores GPR/EFLAGS and x87/SSE state around C++ observer callbacks via FXSAVE/FXRSTOR. Does not *intentionally* change commands, AI policy or coordinates.
- Requires `g_s55Installed=3`, `g_s56Installed=2`, `g_s57Installed=3`; no Stage58 patch if a prerequisite or signature fails. No claim of fault-proof atomic patch rollback in every exceptional condition.
- Stage58 log: `ArcanaAI_Stage58ACDF0ConditionalCallTrace.log`. Installation marker `STAGE58_INSTALLED 3/3`.

### Static verification

```powershell
py -3 Stage58_Analysis\tools\verify_stage58.py --aalib "D:\path\to\AALib.exe" --repo-root "D:\path\to\Stage58_branch_checkout"
```

The stdlib script verifies original binary size/SHA, PE32/x86, original 3 CALL bytes/decoded targets, two prologues, sampled native RET patterns, relocation non-overlap and **exact 3 adapter changes** relative to Stage57. These are **static** properties. They cannot establish Windows MSVC compilation, hook ABI correctness in-game, absence of reentrancy/exception issues, or any AI improvement.

## Required next runtime test

1. Back up the last known working Stage57 DLL (1,137,664 bytes). Install only two Stage58 files according to [RU guide](../Stage58_Overlay/README_STAGE58_RU.md).
2. Require Build.log `BUILD SUCCEEDED` and runtime markers Stage55 3/3, Stage56 2/2, Stage57 3/3, Stage58 3/3.
3. Same-run trace nesting: `Stage55 ACDF0 PRE → Stage57 ACE11 POST → Stage58 helper sites + Stage56 ACF2A → Stage57 ACFD6 PRE → Stage57 ACFD6 POST → Stage55 ACDF0 POST`.
4. Classify every Stage58 callee per site as X-changing / unchanged and intersect with pre-tail residual X changes. Watch for calls that happen multiple times within the same ACDF0 outer window. Require full masks, zero failure health counters and contiguous sequence numbers.
5. If the 215 residual changes are outside all three candidates, do **not** invent a writer; further split the loop or investigate other executed instruction/call trees.

No in-game result is claimed for Stage58 until a user-provided Windows build and COM-vs-COM capture is inspected.
