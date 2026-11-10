# Stage50 — native AE100 coordinate writer immediately before Stage49 AF13D (SOURCE READY)

## Why trace it

Stage49 r2 Windows COM-vs-COM runtime **verified hook installation** on both selected native 1E9170 callers (AF13D/AF1AA); 6,818 complete P1/P2 records (AF13D only), 5,512 linked pointer readable, **zero** changedMask, **zero** staticGateCandidate. 32 Stage49 HEALTH samples: dropped/overflow/mismatch/badCaller all zero. Original native code bit15 gate at 1E92BA cannot pass for observed P1/P2 O+10 values 0x0/0x10000/0x80; no observed linked copy. [Stage49 r2 full analysis](https://github.com/Hawky11/1/blob/stage49r2-runtime-linked-motion-evidence/Stage49_Runtime/RUNTIME_ANALYSIS.md).

Stage47 current run 6,756 native 1E66E0 windows, 4,243 with changed fields; Stage48 1,901 binds, and [6,818-row exact join CSV](https://github.com/Hawky11/1/blob/stage49r2-runtime-linked-motion-evidence/Stage49_Runtime/stage49_stage47_stage48_join.csv) shows Stage47 last O+7C POST differs from Stage49 O+7C PRE in **1,201** cases, while Stage48 last ID is equal in 6,818/6,818. There is an intervening writer. Its identity is not established from that correlation alone.

## Original PE32 (physical file from user's ChatGPT Library)

Zip `AALib.zip`, original `AALib.exe` **175,556,608 bytes**, SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, preferred ImageBase 0x00400000, x86 .text. Inspected with `objdump -D -Mintel` at *virtual address = 0x00400000+RVA*. Exact RVA static signatures and native CALL target checked; [verifier](tools/verify_stage50.py) records them.

Caller loop over original root list `E2E02C8`:
| RVA | Original x86 | Effect |
|---|---|---|
| AF130 | `83 7E 38 FF` | active object `[ESI+38]!=-1` |
| AF134 | `74 1A` | skip inactive item |
| **AF136** | **`E8 C5 EF FF FF`** | CALL native **AE100**, current object in **ESI** |
| AF13B | `8B CE` | MOV ECX,ESI |
| AF13D | `E8 2E A0 13 00` | CALL native 1E9170 (Stage49 already hooks this) |

Native AE100 uses **ESI**, not ECX=thiscall, has no stack arguments, `RET` at AE216. Snapshot important calls and exact data writes:

| RVA | Bytes / operation | Why |
|---|---|---|
| AE100 | `55 8B EC 83 EC 08 83 7E 3C 04` | prologue, compare `O+3C` with 4 |
| AE114 | `39 BE A4 01 00 00` | compare `O+1A4` with 0x32 (EDI loaded 0x32) |
| AE1A6 | `D9 56 7C` | FST O+7C |
| AE1A9 | `D9 9E AC 00 00 00` | FSTP O+AC |
| AE1B8 | `D9 96 80 00 00 00` | FST O+80 |
| AE1BE | `D9 9E B0 00 00 00` | FSTP O+B0 |
| AE216 | `C3` | ordinary RET |

The guarded writer is reached when `O+1A4==0x32` OR `O+3C==4`, subject to any other dynamic execution effects. Stage50 logs `gateCandidate` from these two raw fields, not an asserted executed branch. Its code guards all above bytes and original rel32 target. It does **not** assume all 1,201 differences came from AE100, or assume float changes appear visually.

## Observer design and preservation

[Stage50PreLinkedCoordinate.inl](../Stage50_Overlay/src/Stage50PreLinkedCoordinate.inl) adds **one** CALL redirection at AF136; native AE100 invoked once in original position. Naked x86 no-arg wrapper preserves complete GPR/EFLAGS and x87/SSE with aligned FXSAVE/FXRSTOR, as in Stage49. `S50Enter` uses **saved PUSHAD ESI = saved[1]**; P1/P2 filtering uses player+4 object pointer, same as Stage49; caller RVA = ret minus base and 5. Record 11 DWORD PRE/POST with masks, QPC, changedMask, gateCandidate; ring 4,096, TLS depth 8, existing Stage38 worker flush. Static protections, mutex and suspended other threads, rechecks machine bytes before writing. Failure keeps Stage49 and all prior hooks running.

[Stage50 Stage38 adapter](../Stage50_Overlay/src/Stage38CallbackWindow.inl) differs from working Stage49 adapter **only** by adding the Stage50 include, requiring Stage49 installed before Stage50 install, and flushing Stage50 with existing worker; all earlier .inl/.cpp and INI must be retained. Windows x86 compiler/game runtime not yet executed.

## Exact next runtime proof

Windows x86 Build.bat success + loaded `STAGE50_PRELINKED_COORD_BUILD=2026-11-10-r1` + `STAGE50_INSTALLED 1/1`, then complete P1/P2 snapshots and all loss counters zero. Use matching object/side and ordered QPC to compare Stage47 POST → Stage50 PRE → Stage50 POST → Stage49 PRE in one native-loop iteration. Detect how often AE100 changes O+7C/+80/+AC/+B0 and whether Stage50 POST equals the immediately following Stage49 PRE. If not, inspect other intervening engine stores; do not infer user-visible movement/hits/AI quality without direct evidence.
