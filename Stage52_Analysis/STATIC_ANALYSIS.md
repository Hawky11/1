# Stage52 — passive pre-global native coordinate boundary (SOURCE READY)

## Motivation

Stage51 Windows x86 runtime has 6,560 P1/P2 windows with 13/13 readable fields and no intra-call changes. Stage47→Stage51 same-frame QPC comparison has 6,494 pairs, 435 X changes and zero Y changes; Stage47 POST X equals Stage51 PRE anchor O+AC 6,494/6,494. Frozen evidence: https://github.com/Hawky11/1/tree/stage51-runtime-global-coordinate-evidence .

The next task is to bracket the **two native calls preceding AF119→AE220**, not alter gameplay.

## Exact original binary

User-owned original AALib.exe from Library AALib.zip: PE32, ImageBase 0x400000, 175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.

| RVA | Physical bytes | Operation |
|---|---|---|
| AF10D | E8 FE FD FF FF | CALL AEF10, no pushed args |
| AF112 | E8 79 FC FF FF | CALL AED90, no pushed args |
| AF117 | 6A 01 | PUSH 1 for following AE220 call |
| AF119 | E8 02 F1 FF FF | CALL AE220, already Stage51-hooked in runtime |
| AEF10 | 55 8B EC B8 54 14 00 00 E8 B3 C5 15 00 | native prologue |
| AF09A | C3 | AEF10 plain RET |
| AED90 | 55 8B EC 83 EC 08 53 56 33 F6 57 89 75 FC 8B FF | native prologue |
| AEE5E | C3 | AED90 plain RET |

**Important original writer instructions inside AED90:**

- AEE01: `D9 56 7C` — FST [ESI+7C]
- AEE04: `D9 9E AC 00 00 00` — FSTP [ESI+AC]
- AEE16: `D9 96 80 00 00 00` — FST [ESI+80]
- AEE1C: `D9 9E B0 00 00 00` — FSTP [ESI+B0]

This proves **some** AED90 execution paths write these fields of an object, **not** that the observed P1/P2 X differences were caused by AED90. AEF10 clears object +18 bits and invokes native helpers; it also needs observing.

## Source design

Source: [Stage52PreGlobalCoordinate.inl](../Stage52_Overlay/src/Stage52PreGlobalCoordinate.inl). Two passive original-once, no-arg callsite wrappers at AF10D and AF112. Each preserves original GPR/EFLAGS/x87/SSE, snapshots P1/P2 raw 13 fields before and after the original native call, then returns with plain RET (no argument cleanup).

The installer validates exact physical byte signatures, native call displacements, prologues, two RET instructions and AED90 writer opcodes. Both adjacent 5-byte CALL patches are installed together after suspending other process threads, under the existing mutex. On mismatch it fails closed.

The [Stage38 adapter](../Stage52_Overlay/src/Stage38CallbackWindow.inl) adds one include, calls Stage52 install only after Stage51 succeeds, and flushes Stage52 through the existing Stage38 worker. No other source is changed. The ring has 8,192 slots, health counters and strict P1/P2 tracking. The 13 fields are the same as Stage51: O+60/+7C/+80/+AC/+B0/+3C/+1A4/+18/+24/+2F0/+88/+8C/+10.

[Verifier](tools/verify_stage52.py) is stdlib-only; it checks SHA-pinned original EXE and source/adapter structural guards. **Runtime and Windows x86 compile PENDING**, so no tested Stage52 DLL exists yet. Structural checks are not equivalent to real-game proof.

## Runtime analysis plan

After a successful Windows build and natural COM-vs-COM, correlate Stage47 POST → Stage52 AEF10 PRE/POST → Stage52 AED90 PRE/POST → Stage51 PRE → Stage50 → Stage49 by side, object and QPC. Compare exact 32-bit X/Y/anchor values. Exclude stale cross-frame tail matches. If either native call writes P1/P2 X, isolate its internal writer; otherwise inspect the remaining pre-AF10D gap.

The reported Scharlachrot hover dash followed by a kick cancel is an *uncontrolled player observation*, not evidence of an AI enhancement. Diagnostics can affect frame timing even without explicit gameplay writes. An A/B baseline is required before attributing behavior.
