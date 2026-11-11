# Stage77 — static source/ABI audit (2026-11-11)

**Status: code review and source-pattern checks only.** Windows x86 build, native runtime, no-regression and actual writer attribution pending.

## Baseline and permitted delta

Input source: `Stage76_Overlay/src/Stage38CallbackWindow.inl` at branch `stage76-after-callback-phase-snapshots-source`, blob `f439a0f35e08891590acdb6099afd04ed8d821a4`. Stage76's `Stage76AfterCallbackPhases.inl` remains unchanged; Stage59/65/67/70 native observers and hook installation sequence are identical. Only the Stage38 adapter is replaced by the Stage77 overlay.

Stage77 adds a read-only TLS snapshot (max 16 nested prepare calls) just before the original `156C00` invocation; `Stage38Prepared` consumes it immediately after native 156C00 under the existing protected post-call wrapper. Original 156C00 is still called once, with both native context/player arguments constructed from the original incoming stack as in the verified Stage76 adapter.

## Register/stack audit

Before native 156C00, the inserted block performs `PUSHFD; PUSHAD`, reserves 0x210 bytes for 16-byte-aligned `FXSAVE`, runs `FNINIT; CLD` while observing memory, then `FXRSTOR; MOV ESP,EBX; POPAD; POPFD`. This is the same register/x87/SSE preservation idiom already used by Stage38 and Stage76 wrappers. Original ESP and argument pushes resume after it. No field is intentionally written; no debug registers are read/assigned.

A nontrivial risk remains: extra protected helper execution can perturb timing, original return-address expectations, or nested/reentrant control-flow. TLS pairing records P, frame, context, TID and QPC. A context's *contents* are not inspected before native 156C00 because that is precisely the function which initializes it; Stage38's existing after-native validation of the context remains. Overflow, bad pairing and failed safe memory reads have explicit counters.

## Snapshot mapping

`n0..n4` correspond respectively to P+3F50, P+3F54, P+3ED0, P+3ED4, P+4. Matched after-native values are existing Stage38 PRE `f30,f31,f3,f4,f0`; Stage38 POST is *after callback*, so must not be used for this native comparison. `STAGE77_NATIVE_PREPARE` shares Stage38 sequence number and produces individual n-before→after values, masks, changedMask and both QPC boundaries.

No new detour or patch installer is needed, so all previous Stage76 native E8 opcode checks and source construction remain intact. No changes to the game's COM AI logic or X-motion fields are claimed.

## Checks and limitations

`Stage77_Analysis/tools/verify_stage77.py` statically validates the overlay marker structure, single original 156C00 call, Stage76 inclusion, FXSAVE/FXRSTOR pairs, field offsets and post-snapshot mapping. These are **not** compiler tests, live disassembly verification or guarantees that the running game has unchanged behavior.

After the user builds, require STAGE38/76/70 install markers, Stage59 watch armed, and `STAGE77_HEALTH` errors all zero. The key result is whether old/new selector was written before or during the original native 156C00 call. The exact writer EIP remains unknown until further native inspection.

**Rollback:** previously tested Stage76 DLL 1,901,568 bytes and adapter.