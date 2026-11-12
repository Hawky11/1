# Stage112 — CBDE0 original native CFG / CEBE0 caller reachability

**2026-11-12 · STATIC ONLY · 43/43 original-byte and branch/call checks PASS · 19/19 offline tests PASS.**

## Native provenance

Original user `AALib.exe`, PE32 x86, preferred base `0x00400000`; decompressed size **175,556,608 B**; SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Verified against original copy in user's ChatGPT Library. No original binary is committed here.

## New findings

**CBDE0 is not a command consumer.** Original RVA `CBDE0..CBDFC` is a one-argument leaf initializer: reads global `[0E100DFC]` twice, writes to `record+0` and `record+8`, clears `byte [record+0x10]`, then `RET`. No inner CALL and no indexed shared-state reader.

Exhaustive raw-`.text` E8/rel32 candidate scan produces **12 direct CBDE0 calls** (all individually verified):
- `CA52A`, `CA56F`: `CA500` initializes 64 records at `player+2CC` and up to 0xB0 records at `player+1184`.
- `F7DB5`, `F7DC1`: after `F7DA3 CALL CEA20`, original `F7DA8 CMP byte [EBX+55],0; F7DAC JE F7D8D` skips CBDE0 if zero; otherwise resets `player+7DC` and `+800`.
- `F7E0C`, `F7E18`, `F7E24`, `F7E30`: other `F7150` branch (`[EBP-8]==0 && ESI==0x27`) resets `player+7DC/+800/+824/+848`.
- `153408`, `153414`, `153420`, `15342C`: `153270` initializes `player+930/+954/+978/+99C`. The first two offsets also occur in Stage111 CEA20 PRE; same address offset is **not** a same-time consumption proof.

**CEBE0 has four direct E8/rel32 callsites in separate callers**: `988EA`, `98948` (function `98780`), and `CAD79`, `CADDA` (function `CACE0`). All four use record banks `player+2CC` or `player+1184`, stride `0x24`, and index the side state `0x0433C138 + side*0x2806C`. The actual CEBE0 ID is a signed metadata WORD from an array with stride `0x14`, **not** automatically the Stage104 MODE0 ESI=0x27.

**Reachability gates before CEBE0**:
- `95CD6 CALL 98780` is an original direct parent call. `98886 CALL FA480`, followed by `9888B TEST AL,AL; 9888D JE 98965`, can bypass both CEBE0 loops. An additional `player+10CC==-1` guard also redirects before the first loop.
- `EFDD8 CALL CACE0` is another original direct parent call. `CACEA CMP [player+10CC],-1; CACF1 JE CADF9` exits before both reader loops.

This is a *static* call graph, not evidence those two parent functions ran in the Stage111 match. Raw E8 scans do not rule out indirect calls or dynamic routes.

## Stage111 boundary

Previously confirmed Stage111 same-run: **171/171** Type1 gate-to-FinalCommand, **43/43** MODE0 gate-to-Stage104, **20/20** special `0x27` CEA20 PRE-to-Stage107 POST; **47 Stage110 HEALTH snapshots entry=0/guard=0**, **47 Stage107 HEALTH nativeRead=0**, error counters clean. Stage110 hook is installed but has **no positive entry hit**. These zeros cannot be explained as failed CEBE0 guards when the entry itself was never observed.

## Next Stage113, only if needed

First audit ABI/ownership and then consider guarded **read-only positive controls** at parent callsites `95CD6` and `EFDD8` or their callee entries `98780` and `CACE0`. Distinguish never-called parents from `FA480=0`, `count==-1`, and actual inner CEBE0 calls. Correlate only same-run QPC/TID/side/player/record/state. Preserve original instructions and stack/flags once; transactional install and rollback. Do not redetour `F2DE0`, force actions, use DR, or write gameplay data.

**Preservation:** Stage36 OFF. Existing Windows-built Stage110 DLL **2,073,088 B**; Stage107 fallback **2,065,920 B**; Stage104 r2 **2,058,240 B**. No Stage112 DLL, native hook, new runtime experiment, hit/animation or AI improvement claim.

The complete reproducible Stage112 ZIP (standalone verifier, JSON with 43 checks, 19 tests and detailed RU report) was delivered as a ChatGPT artifact, not committed to GitHub. `main` and historical stage branches remain unchanged.
