# Stage53 — AF0A0 parent-call boundary, passive source (Windows build pending)

## Why

Fresh [Stage52 verified runtime](https://github.com/Hawky11/1/tree/stage52-runtime-dual-native-evidence): x86 BUILD SUCCEEDED, dual Stage52 hooks 2/2 installed, 12,748 tracked rows (6,374 AEF10 and 6,374 AED90), 29 HEALTH samples with zero losses; exactly two `AEF10` first-frame **O+18 0→4** flag writes; no P1/P2 X/Y/anchor writes in either native function. Stage47→Stage52 AEF10 PRE has **6,244** tightly matched same-frame pairs; **868 X-only differences** (0 Y), Stage47 POST X equals AEF10 PRE anchor O+AC in all 6,244. Stage52 AEF10 POST→AED90 PRE 6,374/6,374 and AED90 POST→Stage51 PRE 6,370/6,370 same X/Y/anchor. X writer is **before AF10D→AEF10**.

## Exact native parent from original AALib.exe

Original file: 175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, PE32 preferred base 0x400000.

| RVA | Original bytes | Instruction |
|---|---|---|
| A2B81 | E8 FA EF FF FF | CALL A1B80 |
| A2B86 | E8 F5 CF 02 00 | CALL CFB80 |
| A2B8B | E8 D0 7F 01 00 | CALL BAB60 |
| **A2B90** | **E8 0B C5 00 00** | **CALL AF0A0**, target verified by rel32 |
| A2B95 | E8 F6 CE 02 00 | CALL CFA90 |
| AF0A0 | 55 8B EC B8 6C 14 00 00 E8 23 C4 15 00 | native prologue |
| AF34C | 5F 5E 5B 8B E5 5D C3 | native epilogue/plain RET |

Native `AF0A0` is a no-argument, caller-no-clean function. Its body contains early helper `AF0D1 CALL 1B26A0` (two caller-clean arguments, apparently timer/status helper), **AF10D→AEF10 (Stage52)**, **AF112→AED90 (Stage52)**, AF119→AE220 (Stage51), AF136→AE100 (Stage50), AF13D→1E9170 (Stage49). The pre-AF10D part does not obviously directly store tracked O+7C; it has several nested calls and global updates, so do not assume writer location yet.

## Passive instrumentation

[Stage53ParentCoordinate.inl](../Stage53_Overlay/src/Stage53ParentCoordinate.inl) passively redirects **only** the 5-byte CALL `A2B90→AF0A0`, after strict original byte/signature/ABI checks, one game-thread suspension and mutex. The naked wrapper snapshots P1/P2 objects before and after the **original AF0A0 call once**, preserving registers/EFLAGS/x87/SSE, returns plain `RET`. No artificial commands/coordinate writes. Same 13 raw DWORD fields as Stage52/51, 4,096 ring, sequence/QPC/health. Stage53 surrounds the full native body, including already-installed Stage52/51/50/49 hooks. It does not patch native AF0A0 body or those prior callsites.

[Stage38 adapter](../Stage53_Overlay/src/Stage38CallbackWindow.inl) based directly on the installed Stage52 adapter. Only differences: include Stage53ParentCoordinate.inl, chain `InstallStage53Hooks()` after Stage52, flush via existing Stage38 worker. Preserve previous Stage49 r2, Stage50 r1, Stage51 r1, Stage52 r1.

[Static verifier](tools/verify_stage53.py) checks original SHA, all native signatures/rel32, and adapter/source shape. Physical original bytes verified. **Windows MSVC x86 compilation and actual hook runtime are NOT yet verified.**

## Decision tree for next run

Correlate Stage47 POST→Stage53 PRE by exact side/object QPC; Stage53 PRE→Stage52 AEF10 PRE; Stage52 AED90→Stage53 POST. If the X divergence is already present at Stage53 PRE, investigate earlier parent CALLs A2B81 (A1B80), A2B86 (CFB80), A2B8B (BAB60) in controlled isolated probes. If it emerges inside AF0A0 before AEF10, examine its AF0D1 helper/early loop and any upstream writes. The broad AF0A0 POST alone doesn't prove it wrote X: it includes nested state writes. No claim of rendered motion/damage/AI improvement.
