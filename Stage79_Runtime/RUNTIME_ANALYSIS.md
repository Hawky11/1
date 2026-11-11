# Stage79 — common native selector-family boundary RUNTIME VERIFIED: three positive selector transitions

**User capture:** [1b2aadaeabeebf959fa0575cf59023060e481323](https://github.com/Hawky11/1/commit/1b2aadaeabeebf959fa0575cf59023060e481323), 2026-11-11, frozen in this evidence branch **at that exact SHA**. Five raw user log files unchanged. Observer source: [stage79-effe8-common-selector-family-observer-source](https://github.com/Hawky11/1/tree/stage79-effe8-common-selector-family-observer-source). **Windows x86 BUILD + same-run original-call boundary RUNTIME VERIFIED; exact internal selector writer EIP NOT YET CAPTURED.**

## Build, installation and same-run integrity

- Build.log: **2026-11-11 14:19:14.08**, x86 MSVC 14.51.36231, eight source files, **BUILD SUCCEEDED**, user DLL **1,918,976 bytes** (+7,168 from Stage78 1,911,808). Existing C4754 warning persists. Compiled binary was not itself uploaded, so provenance is user-supplied build log.
- Instrumentation init **14:19:38**, game image base **00280000**; user game execution TID **12708**, P1 **0DF21F78** / O1 **29DBA020**, P2 **0DF4CCA0** / O2 **29DBAC50**. This is **not** the previous Stage78 session TID19736/O29B2*, and cross-run absolute QPCs are never combined.
- Stage79 native observer: **GUARD x86=1 xmm=1 stage38=1 stage76=3 stage78=1 stage59Started=0 stage79=0 nativeOK=1**; **STAGE79_INSTALLED 1/1 callRVA=0EFFE8 targetRVA=0E89E0 originalsOnce=1 DRwrites=0**.
- **3,808** Stage79 `STAGE79_COMMON_WINDOW` records, exact seq 1..3808; **30** Stage79 HEALTH samples all zero `filtered/dropped/overflow/mismatch/readFail`. All Stage79 masks **1FFF/1FFF**, all pairings are same actor/object/thread and native target.
- **4,315** Stage38 callback windows, seq1..4315; **4,315** matched Stage77 before/after-156C00 rows, all full masks, **0** native selector changes; Stage77 health samples zero errors. **8,718** Stage76 phase records, seq1..8718; **4,359** each phase0 EE480 and phase1 E8DA0, **0** conditional phase2 EE5E0; all masks 3FF/3FF and health zero errors. **No Stage38 callback-local selector changes**. Stage78 literal-one site trace contains **install banners only; zero hits at 155D29**.
- Stage59/65/70 logs were not supplied in this new upload; do not claim new health/event counts for those observers.

## Main result: three native original-family windows changed selector

Original native **CALL at EFFE8 invokes E89E0 once per Stage79 window**. All three changes are valid `P+3F50` reads inside this **one native call interval**, QPC-bracketed and correlated to *the same player* in the Stage38/76/77 chronological streams.

| Stage79 seq | Side | Original `E89E0` QPC interval | Selector `f0` | State `f1` | Event current code DWORD `f6` | Stage38 previous→next |
|---|---|---|---|---|---|---|
| **2901** | P1 | **1929407257966–1929407258465** | **0→1** | 0→0 | **FFFF→0011** | **2763→2765**, old0/new1 |
| **3404** | P2 | **1929433903237–1929433903586** | **0→1** | 0→0 | **FFFF→0011** | **3253→3255**, old0/new1 |
| **3740** | P2 | **1929450719743–1929450720115** | **1→0** | 0→0 | **FFFF→0011** | **3581→3583**, old1/new0 |

All three Stage79 native EAX values `FFFFFF00`, return `0EFFED`, `changedMask=0041` (selector field bit0, event field bit6), full masks `1FFF/1FFF`. `P+3F54` remains zero. The first two windows set selector=1; the third resets to zero. **This is family-level original native execution proof, not direct `0F7D22`, `0F3F50`, or other exact MOV EIP evidence.** Especially, the third 1→0 **cannot** result from the ten 1/2 MOV alone; original additional literal-zero sites exist, including `0F7D43`, `0F3F71`, `0F48CA`, `0F4690`, etc.

### Independent Stage38/76 chronological check, per exact P1/P2 (never group all windows without player)

- **P1 seq 2901:** previous Stage38 **seq2763 POST** selector 0 (QPC 1929407161380), then same-player Stage76 phase0/phase1 **seq5525/5526** still selector0, then Stage79 original call selector **0→1**, then next P1 Stage38 **seq2765 PRE** selector1 (QPC1929407258527), callback RVA `158570` argument 0x11.
- **P2 seq3404:** Stage38 seq3253 POST selector0; Stage76 seq6505/6506 selector0; Stage79 **0→1**; next P2 Stage38 seq3255 PRE selector1, `158570` arg 0x11.
- **P2 seq3740:** Stage38 seq3581 POST selector1; Stage76 seq7161/7162 selector1; Stage79 **1→0**; next P2 Stage38 seq3583 PRE selector0, `158570` arg 0x11.

Thus **exactly three** Stage38 between-window transitions by same player/object/TID, which correspond one-to-one to Stage79 changed windows; no Stage38 callback-local selector changes. A naive comparison of consecutive global Stage38 rows without grouping by player gives spurious P1↔P2 alternations; these are **NOT** additional transitions.

## Static consequence: best next boundary for Stage80

The SHA-pinned original `AALib.exe` direct call tree under native `E89E0` contains the four functions with ten register-derived writer MOV sites and several zero sites. In all three changed windows:
- `f5=P+3B7C=0`: `E89E0` entry **bit15 is clear**, excluding early `E89EE→F6960` branch in these snapshots absent outside changes.
- `f11=P+3F7C=0`: excludes early `E8A03→F7150` branch.
- `f12=P+3B90` values P1 0x2, P2 0, P2 0x2: **bit4 clear**, excluding early `E8A28→F70A0` branch in these snapshots.
- These fields were identical before/after E89E0, and native `E89E0` can proceed along its **late decision path** toward a direct **`E8A8E CALL F7150`**, but several intervening helper calls/short circuits remain. The records do **not** prove that `E8A8E` executed.

The late `F7150` function has **two original register-derived 1/2 stores at `F7D22/F7D38` and one zero store `F7D43`**, chosen by descriptor bytes `+1A/+1C`. The captured **0→1**, **1→0**, and accompanying **event 0x11** make a single passive **E8A8E→F7150** native boundary a useful Stage80 **discriminator**; positive windows would narrow to that function, negative windows would favor intervening helpers or indirect sites. It would **not** directly identify an individual writer EIP or prove which descriptor byte was read.

Prefer **one** guarded original 5-byte CALL hook at E8A8E (with original player PUSH ESI argument preserved and original target called exactly once), preserving flags/GPR/x87/SSE, and no additional DR owners or gameplay writes. Build/runtime safety must be separately verified. Do not silently patch 10 sites or claim smarter AI/changed attacks.

**Rollback:** user-tested Stage79 **1,918,976-byte** DLL; Stage78 **1,911,808-byte** DLL and Stage77 **1,905,664**. Raw capture from [1b2aada](https://github.com/Hawky11/1/commit/1b2aadaeabeebf959fa0575cf59023060e481323) remains unchanged.
