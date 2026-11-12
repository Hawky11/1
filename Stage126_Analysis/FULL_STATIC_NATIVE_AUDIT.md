# Stage126 — original native handler dispatch (236 entries), CEBE0 record activation, possible 0x27 fallback

**Date: 2026-11-13 · ORIGINAL-NATIVE STATIC ONLY.** No new hooks, DLL, Windows build, game run, DR edits or gameplay-state writes.

## Exact source and reproducible proof

Original private `AALib.exe`, PE32/i386, **175,556,608 bytes**, SHA256 **28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba**, preferred base `0x00400000`, independently reverified on the original bytes. Direct X86 disassembly and raw opcode checks produced:

| Proof | Confirmed |
|---|---:|
| Full handler table at preferred VA `0x007F8380` | **236/236** 32-bit entries |
| Unique original handler addresses | **59** |
| PE HIGHLOW relocations of all 236 table pointers | **236/236** |
| Original `.text` address-literal references to table | **31/31**, all relocated |
| Pinned original x86 instruction bytes | **146/146** |
| Original relative branches | **38/38** |
| Original direct E8 callsites for six selected functions | **41/41** |
| Related record+0x10 writer opcode sites (not exhaustive globally) | **11/11** |
| Native-independent, failure-case/offline stdlib tests | **532/532 PASS** |

Full downloadable ChatGPT artifact **`ArcanaAI_Stage126_DispatchTable_RecordActivation_StaticAudit.zip`**, **31,158 bytes**, SHA256 **4ad1dd44e7d4dedc2edd32d201210d13b711c0329b979507db25b19a9b9eee62**, **16 source/text/JSON/CSV files** with full **Russian** report, 236-row table, 59-handler histogram, 31 xrefs, 41 direct calls, 146 original instruction contexts, branch catalog, exact-original standalone verifier and 532 tests. ZIP CRC PASS. Original EXE and DLL **not distributed**.

## Handler table, not a gameplay commit table

Source preferred VA `0x007F8380` corresponds to PE RVA `0x3F8380`. Handler pointers include:
- IDs `00..03` → original RVA `0CC510`
- IDs `1E..26` → `0CC5A0`
- IDs **`27..2A` → `0CC5F0`** (four indices)
- IDs `64..83` → `0CCF10` (32 IDs)
- IDs `89..D6` → `0CCFB0` (78 IDs)
- ID `EB` → `0CDA50`

Original handler `CC5F0` uses source tick `[0E100DFC]`, record start, `context+0x18010` and masks `[0E100EB4]` + `[0E100FFC]` to return a boolean through `SETNE AL`, `RET 10h`. This is a **predicate** (bitmask/time compatibility), not a verified attack or animation executor. The numeric code `0x27` matches Stage104's MODE0 ESI=0x27, but no Stage123 trace proves that same event actually invoked `CC5F0` through the table.

Original `CEAD0` loops over **0xEC** index values. Its `CEB39` loads the pointer, `CEB4A CALL EAX`, `CEB4C` writes `AL` to a byte result array; under special conditions `CEBB0 CALL EDX` retries and `CEBB2 OR [result],AL`. This is **bulk predicate evaluation**. Original `CEE04 CALL EDX` and `CEE31 CALL EDX` in `CEBE0` check AL with *different branch polarities*, both in native internal-selection logic.

## CEBE0 and record[0x10] activity

Native `CF0BF` compares a record value against indexed context `+0x28020`; `CF0C8` computes a comparison against `+0x28010`. One branch leading to **`CF0FB MOV BYTE PTR [EBX+0x10], AL`** copies a previously computed local `[EBP-0x2C]` (loaded at `CF0F8`) into record+0x10; other fields `record+0x0C/0x1C/0x20` then update. **The value is not necessarily 1**, and this is not proof of actual gameplay action acceptance. Other `CEBE0` branches clear or decrement existing record+0x10; `CEA20` and `CBDE0` also clear related record flags.

Four confirmed native E8 direct calls to `CEBE0` — **`0988EA`, `098948`, `0CAD79`, `0CADDA`** — were independently reconciled from actual E8/rel32 bytes and original objdump. Other nearby original call counts: `CEAD0=3`, `CBE00=2`, `CEA20=18`, `CBDE0=12`, `CE8C0=2` — **41** native E8 sites in the scoped census. No indirect calls are excluded. Stage123's real Stage110 hook reported **`CEBE0 entry=0`** in its 51 HEARTBEAT/HEALTH windows, so **the internal writer CF0FB was not observed in that run**. Original Stage113 also reported 16,490 bypasses at one FA480 gate, not all possible game paths.

## Important static fallback producer path: F30B0 → code 0x27

Two original E8 callers enter `F30B0`: **`0F3654`** and **`0F3CBC`**, the second inside the previously analyzed `F3930` producer. Subject to native entry prerequisites in F30B0, it checks record activity with the exactly decoded `CBE00(record)` boolean at **`F3216`** and **`F3234`**:
- First active-record scan: **40 records**, 0x24 stride, early-exits if any record+0x10 is nonzero.
- Second active-record scan: **176 records**, 0x24 stride, also early-exits if any active record is found.
- If **all 216** scanned records lack activity and preceding gates passed, **`F325A MOV EAX,0x27`** returns `0x27` and the original code zeros the two output DWORDs at `F3254/F325F`.

Thus the original producer can **naturally return special numeric 0x27 when no record is active**; this is a verified *possible CFG path*, NOT evidence that Stage123's 25 runtime MODE0 0x27 events all came from `F325A`, or that 0x27 semantically means "idle/no-op/rejected attack". There was no runtime observer at `F30B0`, `CBE00`, `CC5F0` or `CF0FB`.

## Evidence boundary and next-stage recommendation

Stage123 did prove **25/25 native MODE0(0x27)→CEA20 PRE→POST**, one G4=7 writer, Stage115 tuple (0,0,3,7,0) with FA480 AL0 in 1,498 samples and Stage110 CEBE0=0. **It did not prove executed handler selection or a final accepted gameplay action**, nor that these 25 records were definitely all inactive. Stage126 has no new Windows compile or gameplay run.

**Stage127 STATIC FIRST:** follow the genuine producer call sequence `F3930 → F30B0 → CBE00` and identify where active record flags are consumed/changed outside the zero-entry CEBE0 path; then investigate specific action execution / animation / hit handoff. Only consider one SHA-guarded, read-only, positive-controlled runtime observation if essential. Never force FA480, mutate records, rehook F2DE0, insert broad 236 handlers, write DR, or enable Stage36. Keep `EnableStage36ControlToCommandTrace=0`.

**Preserve Windows DLL:** Stage122 **2,089,984 B** (known good), Stage115 **2,082,304 B**, Stage113 **2,079,744 B**, Stage110 **2,073,088 B**.

[Stage125 original CEA20 audit](https://github.com/Hawky11/1/blob/stage125-native-command-packet-cea20-consumer-static-audit/Stage125_Analysis/FULL_NATIVE_COMMAND_STATIC_AUDIT.md).
