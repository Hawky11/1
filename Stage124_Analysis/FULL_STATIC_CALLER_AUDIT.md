# Stage124 — FA480 229 original direct callers: byte-verified classification (STATIC ONLY)

**Analysis date: 2026-11-13.** The user's newly provided `ArcanaAI.log` line `[2026-11-12 23:48:01] COMvsCOM active: P1 and P2 control DWORDs are being forced to 1 (CPU).` resolves the previous Stage123 COMvsCOM confirmation gap. This does not by itself establish move acceptance.

## Primary static evidence

Private user-supplied original `AALib.exe`: **175,556,608 bytes**, PE32 x86 preferred imagebase `0x00400000`, SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` verified **again from bytes**. Every `E8 rel32 CALL` that resolves to **RVA FA480** was found using two independent techniques: (1) GNU objdump instruction decoding with its bytes rechecked against the original .text PE section, and (2) unassisted scan of original .text bytes for signed E8 rel32 target. **Both methods return the same exact 229 RVA addresses, with no surplus or missing matches.** These are direct original callsites; not all indirect FF/2 calls or runtime detours.

## Immediately following instruction-pattern classification

| Original instruction suffix | Number |
|---|---:|
| `CALL FA480; TEST AL,AL; JE` | **164** |
| `CALL FA480; TEST AL,AL; JNE` | **56** |
| Other / deferred use of AL | **9** |
| **Total independent verified direct sites** | **229** |

The remaining nine include 8 with no immediately adjacent `TEST AL,AL` and one with `TEST AL,AL` followed by a non-branch instruction. This is **linear byte-level classification, not function/gameplay semantics**; branch polarity and intended effect cannot be inferred from JE/JNE alone.

All **229 calls** are distributed among **20** 0x10000-RVA blocks: 07:1,08:1,09:16,0A:54,0B:2,0C:3,0D:1,0E:28,0F:7,12:7,13:4,15:2,16:22,17:6,18:41,19:4,1A:1,1B:4,1D:2,1F:23.

### Concrete original examples

- **RVA 098886** is the Stage113 instrumented native callsite. It calls `FA480` and then `TEST AL,AL; JE 098965`, avoiding subsequent CEBE0 calls on AL=0. Stage123 recorded **16,490/16,490 original parentA/gateA bypass** hits in the 51 reported HEALTH windows; this proves only that specific observed path was bypassed.
- **RVA 0A6B33**: `TEST AL,AL; JE 0A6C0B; CMP DWORD [G3],2; JNE 0A6C0B`. An additional global G3 condition is required.
- **RVA 0A6C45**: `TEST AL,AL; JE 0A6C85; CMP DWORD [G3],1; JNE 0A6C85`. A different additional global G3 condition is required.
- **RVA 0A6CA6**: `TEST AL,AL; JNE 0A6CBE` — inverse branch polarity.
- **RVA 0A2146**: CALL result followed by POP/POP/POP and `MOV AL,1` before return; not a trivial `TEST AL,AL; JE` caller.

### Relationship to existing AI command trace

The Stage123 **same-run** Stage104/107 proof remains **25/25 unique `MODE0(0x27) → CEA20 PRE`** and **25/25 unique `CEA20 PRE → Stage107 POST`**, P1=8 / P2=17. There is **no original direct `CALL FA480` within +/-0x400 bytes of `RVA 0F7CC0`**. Nearest direct call in the .text is `0F6484`, approximately 6204 bytes away from 0F7CC0. That is strictly an address-window statement: these functions might invoke other helpers that ultimately reach FA480, and FA480 state may influence upstream paths.

The original native `FA480` predicate is satisfied if `(G1 in {8,14} and G2!=0) || (G3==1 and G4==4) || (G3==2 and G5==4)`. Stage123's **1498/1498** sampled FA480 results at the Stage113 gate were consistent with tuple `(0,0,3,7,0)` and `AL=0`, while native G4=7 was positively written by RVA19F2AD on one object in the *same* fresh run. This **does not prove** all 229 sites were reached or took the same branch, or that the original AI rejected any specific move.

## Verification, deliverable, and limits

- Portable Python stdlib unit suite **25/25 PASS**, including SHA/size/base, 229 uniqueness, byte and displacement verification, branch-pattern counts, known-site contexts, and byte-independent full scan of original PE `.text` target E8 CALLs.
- User-facing reproducible ZIP `ArcanaAI_Stage124_FA480_229Caller_OriginalNative_StaticAudit.zip`, **29,473 bytes**, SHA256 **`9508fb0f7799ef468c982fea8a93832e1b96373db8e00d49b44b6afa3e623c23`**; **7 text/CSV/JSON/Python files** including a **229-row CSV with bytes and source context**. No original executable, Windows DLL, source patch or runtime hooks inside.
- The original EXE bytes were inspected locally but not shared. Disassembly and CSV are reproducible on the SHA-verified private user binary.

**Conclusion:** FA480 is a repeatedly reused native condition across a large set of code locations, and Stage113's one caller should not be conflated with the *entire* AI system. Stage124 does not claim genuine game-command semantics for all 229 callers.

**Stage125 handoff:** static-first investigation of concrete *action acceptance / commit* path and downstream data structures in the original command flow (Stage94→Stage104→CEA20→Stage107). Trace actual downstream branches and callers of those original functions, then choose a targeted positive-controlled read-only probe only if necessary. Do NOT introduce a bulk 229-hook system, patch FA480, force globals, or mutate gameplay/DR state. Stage36 OFF.

Preserve Windows-tested Stage122 DLL **2,089,984 B**; older rollbacks Stage115 **2,082,304 B**, Stage113 **2,079,744 B**, Stage110 **2,073,088 B**. **STATIC ONLY**: no new Windows build or game run in Stage124.

[Prior Stage123 positive G4 writer Windows audit](https://github.com/Hawky11/1/blob/stage123-positive-g4-writer-windows-runtime-audit/Stage123_Runtime/FULL_G4_WRITER_SAME_SESSION_AUDIT.md).
