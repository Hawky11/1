# Stage98 r1 — native command-route predicates at F7A00 (static audit)

Original user-Library `AALib.exe` SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` (x86 PE32, `.text` RVA 0x1000, raw offset 0x400). RVA-to-raw conversion here is `RVA - 0xC00`. **Static disassembly alone is not a live install or evidence that any predicate improves AI.**

## Why Stage98 and what it observes

Stage97 established native `F77D8 MOV ESI,[EBP-2C]` → Stage95 F7A94 → Stage94 F7B0A → real type1 FinalCommand for two new `0x2C` cases; all 61 observed finite bypasses have attributable immediate ESI writers. Next valuable unknown is *native route choice* — why the engine enters F3930 producer versus the F7A94 bypass. F7A00 is the shared branch boundary and neither Stage94/95/96/97 overwrites it. One passive 6-byte hook there snapshots state but does **not** override ESI, result, memory flags, or the subsequent native branch.

## Original control flow (verified exact bytes)

- **F7A00**: `8B 87 78 3B 00 00` — `MOV EAX,[EDI+3B78]`, 6 full bytes; Stage98 patch/replay exact once; resume F7A06.
- F7A06: `MOV EDX,EAX; SHR EDX,8; TEST DL,1` (8 bytes). F7A0E: `JE F7A99` (6 bytes, rel32 +0x85). **bit8 of [EDI+3B78] = 0 → producer route F7A99**.
- F7A14: `MOV ECX,[EDI+3B70]; SHR ECX,12h; TEST CL,1` (12 bytes). F7A20: `JNE F7A99` (2 bytes, rel8 +0x77). **bit18 of [EDI+3B70] = 1 → producer route F7A99**.
- F7A22-F7A36 checks `[[EDI+4]+60]` (the actor's current action/status) in inclusive range **0x816…0x81A**; if true → bypass F7A94.
- F7A38-F7A47 checks bit15 and bit17 of `[EDI+3B78]`; bit15=0 or bit17=1 → bypass F7A94.
- Remaining F7A49-F7A8F checks `[EDI+3A34]` and a dynamic call `525CD0`, then may call `F34D0` which can change ESI; **no prediction** beyond class 5 until live trace.
- F7A94 is already owned by Stage95; F7A99 proceeds to F7AD1 CALL F3930 via Stage82 and Stage94's post-call observer. Neither is newly patched.

## Branch classes

| Class | Predicted early route | Grounds |
|---|---|---|
| 0 | Invalid/insufficient reads | Do not use in correlation |
| 1 | **F7A99 → F3930 producer** | bit8=0, or bit8=1 and bit18=1 |
| 2 | **F7A94 bypass** | bit8=1, bit18=0, status in [0x816,0x81A] |
| 3 | **F7A94 bypass** | earlier conditions false, bit15=0 |
| 4 | **F7A94 bypass** | earlier conditions false, bit17=1 |
| 5 | **Further conditions** | action mode and helper `525CD0` may select route; do not predict |

Snapshot must have a valid player, EDI, EBP and seven read-valid flags. All prediction classes are based on native *read-only* state immediately prior to the original `MOV`. This does not establish gameplay action semantics or show that manipulating any bit would be safe.

## Scope, exact signature, coexistence

- Stage98 hook **only F7A00..F7A05**, 6 bytes, with `E9 rel32 + NOP`; original `MOV` replay once with `_emit`, all GPR/EFLAGS/x87/XMM preserved by pushfd/pushad/fxsave + restores; original F7A06 and conditional instructions execute as built.
- Fail closed unless exactly the verified Stage97 r1 source/rollback manifest exists; byte-guard F7A00/06/0E/14/20 and both exact branch targets; Stage97/96/95/94 installed owner flags, Stage96 **five** exact native detour targets, Stage95 exact F7A94 hook owner and Stage97 F77D4 hook owner.
- Original full AALib.exe PE fingerprint; patch mutex, suspended-peer EIP exclusion, second signature check and rollback. No write to game data/DR, no synthetic attacks, no double hooks at existing sensitive points. Stage36 stays **OFF**.
- `ArcanaAI_Stage98CommandRouteTrace.log`: `STAGE98_ROUTE` with `branchClass`, native snapshots `[player+3B78/70/84]`, current action status, player mode, local M8, ESI/EAX, QPC, side/TID/EBP and read flags. Periodic `STAGE98_HEALTH` records counters.

## Next live test

Compile Windows MSVC x86, require `STAGE98_INSTALLED 1/1` and existing Stage97 `1/1`, Stage96 `5/5`, Stage95 `2/2`, Stage94 `2/2`. Upload same-session Build.log and full Stage98/97/96/95/94 (plus Stage93 Final QPC) after natural COM-vs-COM. `tools/correlate_stage98.py` maps each Stage98 snapshot to **the first downstream Stage95 bypass or Stage94 F3930 producer for the same player/TID/EBP**, bounded by the next snapshot in that context and QPC lag; checks predicted classes 1–4 against observed route while class 5 stays unknown. Do not mix logs between sessions, and do not equate F3930 or bypass with improved attacks/hits.
