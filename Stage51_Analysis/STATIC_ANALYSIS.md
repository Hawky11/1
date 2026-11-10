# Stage51 — original AE220 native per-frame update, passive global bracket (SOURCE READY)

## Why this stage

The Windows x86 Stage50 r1 runtime is **verified** ([frozen evidence](https://github.com/Hawky11/1/tree/stage50-runtime-prelinked-coordinate-evidence)), `STAGE50_INSTALLED 1/1` with 5,792 P1/P2 events and no `changedMask` or gateCandidate. Stage49 r2 retains 5,790 P1/P2 events, and [5,790-row join](https://github.com/Hawky11/1/blob/stage50-runtime-prelinked-coordinate-evidence/Stage50_Runtime/stage47_stage50_stage49_join.csv) proves Stage50 POST = Stage49 PRE for raw O+7C/80/AC/B0 in every matched pair. Stage47 has 5,668 P1/P2 windows; 1,257/5,792 Stage47 last POST O+7C differs from Stage50 PRE O+7C, while Stage47 O+7C POST = Stage50 O+AC PRE for 5,792/5,792. Stage46 has no intervening binder window between matched Stage47 and Stage50 events. Thus something updates live O+7C and leaves O+AC unchanged before native AE100. Whether this is game-visible translation is unproven.

## Original physical AALib.exe (already in Library)

PE32 size 175,556,608; SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`; ImageBase 0x400000. Original disassembly around `AF0A0`:

| RVA | Original bytes | Operation |
|---|---|---|
| AF10D | E8 FE FD FF FF | CALL AEF10 |
| AF112 | E8 79 FC FF FF | CALL AED90 |
| AF117 | 6A 01 | PUSH 1 |
| **AF119** | **E8 02 F1 FF FF** | CALL **AE220** |
| AF11E | 8B 35 ... | MOV ESI,[E2E02C8] (absolute operand relocated by Windows) |
| AF124 | 83 C4 04 | ADD ESP,4 (original caller cleans AE220's argument) |
| AF136 | E8 C5 EF FF FF | Stage50 AE100 |
| AF13D | E8 2E A0 13 00 | Stage49 1E9170 |

Native AE220 prologue at RVA AE220 is `55 8B EC 51 53 56 57 E8 44 13 FF FF`; internal AE2B7 nested call `E8 C4 19 04 00`; has `RET` (C3) at AE6DD. Native cdecl consumes one DWORD `[EBP+8]` and returns without popping arg; the original caller maintains stack cleanup.

**Why AE220?** It is a broad native player/object update preceding AF136 in the same caller. Disassembly shows conditional loops and nested calls. This is a candidate **boundary localization**, not proof that AE220 writes O+7C: the gap also includes preceding functions AEF10 and AED90 and other engine updates.

## Passive wrapper

[New Stage51GlobalCoordinate.inl](../Stage51_Overlay/src/Stage51GlobalCoordinate.inl) replaces just original CALL AF119 with one native-once wrapper preserving registers/EFLAGS and x87/SSE. Wrapper copies the original argument from `[ESP+4]` before calling AE220, frees **only the temporary copied argument**, then returns with plain RET; original AF124 still frees original arg. This differs deliberately from Stage49/50 zero-arg wrappers. The 7 static native guard signatures also check argument push, original rel32 CALL destination, *only* the unchanged 8B35 opcode after CALL (not relocated address), caller cleanup, AE220 prologue, internal native call, and native RET. Checked against physical SHA-pinned EXE.

Before/after the single native AE220 call, snapshot **both P1 and P2** via player+4 object pointers with 13 raw DWORD offsets (mask 1FFF): O+60/7C/80/AC/B0/3C/1A4/18/24/2F0/88/8C/10. QPC ordered, per-side/object thread and original caller, pass-through original arg. Two rows per invocation in existing Stage38 worker ring. HEALTH includes dropped/overflow/mismatch/badCaller/badArg/missingObjects. No automatic player inputs, memory writes beyond necessary CALL redirection, or additional worker threads.

[Stage51 Stage38 adapter](../Stage51_Overlay/src/Stage38CallbackWindow.inl) based exactly on Stage50 adapter with only Stage51 include, chained install after successful Stage50, and FlushStage51. Keep Stage49 r2, Stage50 and all other working source unchanged.

## Next Windows x86 proof

**NOT YET BUILT/TESTED.** Install [two files as instructed](../Stage51_Overlay/README_STAGE51_RU.md), build x86, confirm `STAGE51_GLOBAL_COORD_BUILD=2026-11-10-r1` and `STAGE51_INSTALLED 1/1`, natural COM-vs-COM, upload Build.log/Stage51/Stage50/Stage49/Stage47/Stage48. Correlate Stage47 POST → Stage51 PRE/POST → Stage50 PRE. If AE220 POST explains 1,257 differences, examine native subcalls. If it does not, inspect earlier AEF10/AED90 and any intermediate writers. No claim about rendered motion/hit/damage or smarter AI until proven independently.
