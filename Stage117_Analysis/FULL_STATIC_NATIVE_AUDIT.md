# Stage117 — original AALib.exe FA480 native global writers / xrefs (STATIC ONLY)

**2026-11-12**. No DLL, no source overlay, no Windows rebuild and no game run; original executable static audit only. Historical Stage116 Windows runtime evidence is separate.

## Provenance and independent checks
- Original user private Library `/AALib.zip` → `AALib.exe` 175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. PE32 x86 preferred imagebase `0x00400000`.
- Original `.text` five DWORD absolute-address operand occurrences: **371/371 PE HIGHLOW relocated**, across **212,051** native relocation entries. Address-literal refs and decoded instructions were independently cross-checked with GNU objdump. **34/34 exact direct-store sites** and **18/18 original opcode signatures** verified; Python stdlib offline suite **21/21 PASS**. No original EXE distributed.
- Delivered conversation archive `ArcanaAI_Stage117_FA480_GlobalWriter_StaticAudit.zip`: **22,541 bytes**, SHA256 `bf9cef7f76c877f20e150892134f27ff41043290d1165833798d891b561e8fd8`, 8 files (full Russian report, 371-row xrefs CSV, 34-store disassembly contexts, machine-verification JSON, stdlib verifier, xref exporter, tests, README). The ZIP is distributed via the assistant attachment, **not** committed to GitHub.

## Relocated address operands vs explicit instructions that STORE to that address
| ID | Original preferred VA | .text absolute refs | explicit direct stores |
|---|---|---:|---:|
| G1 | 0E3C3EFC | 29 | 13 |
| G2 | 0E3C4930 | 9 | 2 |
| G3 | 0E10B37C | 148 | 11 |
| G4 | 0E10B640 | 136 | 7 |
| G5 | 0E10B8B8 | 49 | 1 |
| **Total** | | **371** | **34** |

These numbers are bounded to absolute relocated operands in original `.text`, **not** all possible indirect/array/memset/memcpy writers. Pointer immediate references are *not* data reads. The 34 store RVAs:

- G1: `1F3EB6,1F3F4D,1F3F96,1F400B,1F44D9,1F44F3,1F450D,1F4527,1F4559,1F456E,1F459E,1F45B8,1F4615`.
- G2: `1F4142,1F41CB`.
- G3: `04CFA8,04D011,0F9805,0F9AE5,0FB472,19EDD0,19F973,19FA5A,1A09D0,1A0C79,1A2D18`.
- G4: `19F669,19FA6F,19FB7F,19FCBB,1A0414,1A05B8,1A09BE`.
- G5: `1A05ED`.

## Original native control-flow: three FA480 allow conditions

```text
P1 = (G1 == 8 or G1 == 14) AND G2 != 0  // original helper 1F4700
P2 = (G3 == 1) AND (G4 == 4)              // FA48C/FA496
P3 = (G3 == 2) AND (G5 == 4)              // FA4A2/FA4A7
FA480 returns (P1 || P2 || P3)
```

Stage116's **1,455/1,455 full sampled global snapshots** `(0,0,3,7,0)` gave all three false and matched actual `AL=0,ZF=1` on sampled original `9888D` gate. Stage116 all-hit counters also showed **16,760/16,760** parentA gate skips. Stage117 is purely static and **does not** establish which writer last set the runtime values.

## Important writers, inheritance and CFG details
- **G1**: `1F3EB0` takes an argument and stores it to G1 at `1F3EB6` (**49 direct CALL** sites in original). In `1F44D0` state remap, old G1=0x15 → G1=0x0E at `1F4527`; old G1=0x14 → G1=**8 if G2!=0, otherwise 2** at `1F4559` (NEG/SBB/AND 6/ADD 2).
- **G2**: `1F4140` resets G2 and adjacent DWORDs to zero (**5 direct CALL** sites); `1F41B0` sets G2=1 at `1F41CB` (**2 direct CALL** sites). `1F40F0` returns address of G2 rather than its value, so unexamined *indirect* stores may exist.
- **G4/G5 are heads of adjacent 0x278-byte records**: `G5-G4=0x278`. `F97E0` passes G4 and `0x4F0` (=two records) to original memset-style RVA `207780`, then sets G3=0 at `F9805`. `F9830` zeroes selected `idx*0x278+G4` record. `F99A0` copies records with `REP MOVSD, ECX=0x9E` at `F9AC6/F9ADC` and sets G3=1/2 via `F9AE5`; these are real **indirect writes** to both heads. `1A05B8` and `1A05ED` also preserve head DWORD around selected-record reset.
- G3 direct writers include explicit zero (`F9805,FB472,19EDD0,1A2D18`), 6 (`4CFA8,4D011`), copy from object `[EDI+0x10]` (`19F973,1A0C79`), 1 (`19FA5A,1A09D0`) and 1/2 through slot swap `F9AE5`. G3=3 in Stage116 is a *measured value*, **not** an identified last-writer site.
- G4 direct writers include 6 (`19F669,19FB7F,19FCBB,1A0414`), 4 (`19FA6F`), 2 (`1A09BE`), restore after reset (`1A05B8`). The Stage116 **G4=7** cannot be uniquely attributed from these direct stores; possible computed-pointer / bulk-copy sites require additional provenance. The single G5 direct store `1A05ED` restores a previously saved value after resetting its slot; *not* proof of G5 being immutable.

## Concrete native positive-state pathway

RVA `19FA42 CALL 1F4700; 19FA47 TEST EAX,EAX; 19FA49 JE 19FB29`. On the nonzero branch: `19FA4F MOV EAX,1; 19FA5A MOV [G3],EAX` and `19FA6F MOV [G4],4`. Thus there **is** an original code path forming `G3=1,G4=4`. This proves **static CFG capability**, not that branch executed in the observed COM-vs-COM state.

## Critical scope caveat
There are **229 original `E8 CALL FA480` direct sites**, matching byte-rel32 and GNU objdump. Stage113 only observed **one** such path (`98886 → 9888D`), so 16,760/16,760 bypasses may *not* be generalized to 228 other callers. No live helper enter/exit trace exists for other sites; no game mode semantics, accepted command/animation/hit or AI improvement is established.

## Stage118 handoff
Continue STATIC-first: trace native callgraph and object-state writers that set `G3=3` and the indirect sources of `G4=7`, including `F9A0` two-slot swap and caller provenance. Determine whether an additional *read-only* writer trace is necessary; avoid code or gameplay writes, synthetic commands, forcing mode values, or conflicting native hooks. Preserve Stage115 Windows DLL **2,082,304 B**, Stage113 **2,079,744 B**, Stage110 **2,073,088 B**; Stage36 remains OFF. **No new DLL for Stage117.**

[Stage116 verified runtime audit](https://github.com/Hawky11/1/blob/stage116-fa480-five-globals-runtime-verified/Stage116_Runtime/FULL_SAME_RUN_AUDIT.md).
