# Stage54 — three sibling CALL boundaries immediately before AF0A0 (source-only)

Verified SHA-pinned original: AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, size 175,556,608 bytes. Original physical x86 disassembly and relative-call destinations:

| CALL RVA | Bytes | Target | Helper entry signature |
|---|---|---|---|
| A2B81 | E8 FA EF FF FF | A1B80 | 80 3D 5C 1B 0A 0E 00 |
| A2B86 | E8 F5 CF 02 00 | CFB80 | E8 5B 06 11 00 6A 01 E8 54 06 11 00 59 C3 |
| A2B8B | E8 D0 7F 01 00 | BAB60 | 55 8B EC 81 EC C0 00 00 00 53 56 33 DB 57 |
| A2B90 | existing Stage53 wrapper | AF0A0 | Stage53 preserved |
| A2B95 | E8 F6 CE 02 00 | CFA90 | untouched |

These three callers have no stacked arguments or caller cleanup between their direct CALLs. CFB80 visibly ends in plain RET. Other helper returns are reached without caller cleanup in the same original function. Runtime behavior/ABI remains unverified until user Windows test.

Stage53 verified: 6,024 windows, no P1/P2 X/Y writes within AF0A0; 5,904 same-side/object QPC pairs Stage47→Stage53, **1,091 pre-AF0A0 X differences** (403 P1, 688 P2), zero Y differences. Stage47 POST X equals Stage53 PRE anchor X 5,904/5,904. Stage53 PRE equals Stage52 AEF10 PRE 6,024/6,024. Current drift therefore precedes A2B90.

New Stage54 source redirects only the three original CALL instructions after exact in-memory byte/rel32/helper-signature checks, Stage53/52/47 installation checks, and 3-way patch installation under suspended threads plus shared mutex. Each naked wrapper snapshots P1/P2 before and after **its own original no-arg native exactly once**, preserving full GPR/EFLAGS and FXSAVE/FXRSTOR state. No intentional command/coordinate/AI writes. 13 raw DWORD fields, QPC, per-site CALL RVA, masks, 8192 ring/8 TLS depth and health counters. The earlier Stage47–53 source is not changed.

Analysis decision:
1. Stage47 POST→Stage54 A2B81 PRE: if X already differs, writer is even earlier.
2. Stage54 A2B81 PRE→POST; A2B86 PRE→POST; A2B8B PRE→POST: find first true within-helper X change.
3. If all three are unchanged, compare inter-call snapshots and last Stage54 A2B8B POST→Stage53 A2B90 PRE. Differences there suggest another intervening write/thread or capture/ordering assumption needing reevaluation.
4. Cross-check same-side/object QPC and anchor X; never infer AI decisions, visual animation, hits or damage from raw coordinate changes.

**Windows MSVC x86 BUILD and in-game runtime PENDING**. Do not describe this branch as tested.
