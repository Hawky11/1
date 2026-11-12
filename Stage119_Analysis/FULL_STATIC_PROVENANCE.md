# Stage119 — original x86 native selector/map/object-flag writer provenance (STATIC ONLY)

**Date:** 2026-11-12. **Source:** original user's private AALib.exe extracted from Library AALib.zip, PE32 x86 **175,556,608 bytes**, preferred imagebase 0x00400000, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. The original executable is **not** committed or distributed.

## Independent native verification

- **45/45** exhaustive PE HIGHLOW-relocated original .text refs to five selected original absolute addresses: selector `0x0E10BEDC` **29**; map[0] `0x05146280` **7**; map[1] `0x05146284` **5**; map[2] **2**; map[3] **2**. A relocated reference is not necessarily a writer. These counts are **not** exhaustive of pointer-computed writers.
- **42/42 unique critical original opcode instructions**, **135/135 E8 rel32 helper callsites** to `F9900` (91), `F9920` (1), `F9940` (23), `F9980` (9), `165D20` (11), and **7/7 Jcc targets** verified from original machine bytes. **46/46 Python unittest PASS**, including negative catalog alterations and symbolic local-state tests.
- Reproducible conversation source/evidence ZIP `ArcanaAI_Stage119_SelectorMapFlag_StaticAudit_Clean.zip` **22,631 bytes** SHA256 **`024a1259c9f95195276b3a9d90a29a4c20b276106ce3280c87f819d0c28f7e37`**, with `Stage119_Analysis/README_RU.md`, full Russian audit, `analysis/site_catalog.json` containing exact instruction bytes and owned relocation operands, 45-row xref CSV, 135-row direct-E8 CSV, GNU objdump native contexts, machine result JSON, test output, portable stdlib verifier and 46-test suite. No EXE/DLL, no gameplay hooks.

## Selector `[0x0E10BEDC]` — 11 original direct stores

| RVA(s) | Value |
|---|---|
| `0FB487` | original ESI value; do not assume ESI=0 from this site alone |
| `19F542,19F676,19FB8B,19FCC6,1A041F` | constant 4 |
| `1A2D2E` | constant 1, behind original branch after F9920 reset |
| `1A34AF` | constant 4 through EAX |
| `1A354B` | from `[ESI+50]` after decrement at 1A352A (negative wraps to 9) |
| `1A3597` | from `[ESI+50]` after increment at 1A3572 (greater than 9 wraps to 0) |
| `1A35DC` | constant 0 |

The selector **can be 8** by natural original local arithmetic from 7 +1 or 9 -1, provided the caller's other gates allow the update. It is **not proven** to have been 8 in the user's previous COM-vs-COM session. Do not assign unverified gameplay semantics to values 0..9.

## Map[1] `[0x05146284]` — competing original writer paths

- `F9920` resets all four entries to `-1` (1 direct caller `1A2D13`).
- `F9940` stores caller-provided `value` into `map[index]` without a local index check (23 direct E8 callers). Example: `165F6F`, `165FFE`, `16BF52` explicitly pass index1/value=-1; `19F36E` passes index1/value=1; `0FC587` passes index1 with a computed object value. Thus reset state is **not permanent**.
- `F9980` swaps the DWORDs of map[0] and map[1] (9 direct E8 callers). An existing `map[0]=-1` can become `map[1]=-1` without writing index 1 through F9940.
- `F9960` copies map[0]/map[1] into map[2]/map[3], not back to map[1].
- `F9900` reads map index0/1, returning -1 for invalid inputs. At original `19F2A0`, index comes from boolean object flag and is necessarily 0 or 1; hence `F9900==-1` there means the actual map value is -1. The user has **not** logged the table's live values or last writer.

## Object bytes `[ESI+2A/2B]` — native on/off transitions

Within the original object updater `19EF30`, `19F139 CALL 165D20` searches for value 0 in a four-DWORD array related to `[FDF70()+50F0]`; if index != -1 then `19F143 MOV BYTE [ESI+2A],1`. If 0 is absent, `19F156 CALL 165D20` searches for 1; if found, `19F160 MOV BYTE [ESI+2B],1`. **F565D20** is a 4-element search helper returning index 0..3 or -1. Source array contents and parent-gate runtime values are unknown; the two set paths are prioritized, not independent. Other early exits of `19EF30` apply.

Both object bytes are cleared together through 16-bit stores at `19EDBA` (initialization), `19EF7E`, `19EFD3`, and `19F3D0`. These are native writes to that object layout, not proof of an exhaustive alias-aware writer inventory.

## Stage118 G4=7 write path and Stage116 contrast

The verified native path at `19F223..` reaches `19F295 [object+10]=3` and `19F2AD [EDI]=7` for selector=8, selected object flag, and `F9900(mapIndex)=-1`. For first slot **G4**, necessary local conditions are `[object+2A]!=0`, selector=8 and map[1]=-1; the overall function must also pass preceding gates. For the second slot **G5**, object+2A=0 and object+2B!=0 selects map[0]. Setting object state=3 does **not** directly write global G3; previous Stage118 identified separate object→G3 copies `19F973`, `1A0C79`.

Stage116 measured **1,455** full post-FA480 global snapshots `(G1,G2,G3,G4,G5)=(0,0,3,7,0)`, AL0/ZF1, and **16,760** native parentA gate skips. Those are **observed runtime measurements**; Stage119 static findings show possible upstream state transitions but **do not identify which path, writer, or ordering actually occurred** at runtime. No direct evidence of selector=8, map[1]=-1, object flag=1 during the G4=7 write.

## Handoff Stage120

Preserve Stage115 known-good Windows DLL **2,082,304 B**, Stage113 **2,079,744 B**, Stage110 **2,073,088 B**; Stage36 remains OFF. No new DLL, overlay, runtime or gameplay change in Stage119. Stage120 should first audit object lifetime and parent function CFG; add carefully scoped, guard-checked **passive** trace only if necessary to establish real last-writer provenance and positive site hits. Do not patch FA480, mutate selector/map/object flags, write DR registers or synthesize game actions.
