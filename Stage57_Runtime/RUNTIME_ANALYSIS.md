# Stage57 runtime analysis — ACDF0 X changes localized to pre-tail gap and 15CEC0 (2026-11-10)

**Status: Windows x86 BUILD + runtime VERIFIED. Exact Stage55/56/57 same-session correlation VERIFIED. Residual writer identification PENDING. Stage57 has no intentional gameplay/AI writes.**

## Immutable provenance

- Same-session raw upload: [commit `31f83c31677e9a3f1ff891d24ed8dd223c7f2a54`](https://github.com/Hawky11/1/commit/31f83c31677e9a3f1ff891d24ed8dd223c7f2a54), four files: `Build.log`, `ArcanaAI_Stage55EarlierMotionCoordinateTrace.log`, `ArcanaAI_Stage56ACDF0WriterTrace.log`, `ArcanaAI_Stage57ACDF0HelperBoundaryTrace.log`.
- This evidence is retained on [`stage57-runtime-acdf0-verified-evidence`](https://github.com/Hawky11/1/tree/stage57-runtime-acdf0-verified-evidence) branched from the **raw upload commit**, without modifying any log file.
- Tested source: [`stage57-acdf0-tail-native-boundaries`](https://github.com/Hawky11/1/tree/stage57-acdf0-tail-native-boundaries); original private AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Do not publish proprietary binary.

## Build and instrumentation health

- Build started 10 November 2026 **20:23:06.63 local log time**, **BUILD SUCCEEDED**, `build/dinput8.dll` **1,137,664 bytes**. Existing MSVC C4754 warning in AICommandSiteTrace.cpp; no compilation/link failure.
- Runtime at 20:23:38: **Stage55 INSTALLED 3/3**, **Stage56 INSTALLED 2/2**, **Stage57 INSTALLED 3/3**. Stage57 hooks: `ACE11→A2C40`, `ACFD6→15CEC0`, `ACFDB→15B8A0`.
- All reviewed health records show **0 missingObjects, dropped, overflow, mismatch, badCaller**, and for Stage56 **0 readFail/otherObjects**. Stage55 23 health lines, Stage56 14, Stage57 23. No `ERROR`/`FAILED`/`FATAL` trace lines.
- Stage55: 16,494 unique consecutive rows `1..16494`, exactly 5,498 each at `A119B→AE6E0`, `A11A0→ACDF0`, `A11A5→AC8A0`; 2,749 P1 and 2,749 P2 per site.
- Stage56: 359 unique consecutive rows `1..359`, every event at `ACF27/ACF2A` (P1 247, P2 112); 0 at `ACFA0/ACFA3`. Both pre/post reads present for all rows.
- Stage57: 16,512 unique consecutive rows `1..16512`, exactly 5,504 per site (P1 2,752/P2 2,752 per site). All Stage55/57 snapshot field masks `1FFF`; all QPC start ≤ end.
- The **last 18 Stage57 rows** (`seq=16495..16512`; 6 per helper callsite) occur after the last logged Stage55 ACDF0 outer window. They have **no Stage55 parent** and are **excluded** from strict nesting calculations. This is a logging-coverage tail mismatch (possible flush timing), **not** a proven dropped-hook event; counters remain zero.

## Strict same-run nesting

Join key: **side + player + object + thread ID**, with **full QPC interval containment**, not wall-clock matching. Every one of the **5,498 Stage55 ACDF0 outer windows** has exactly one nested Stage57 `ACE11`, one `ACFD6`, and one `ACFDB` call window, ordered and non-overlapping:

- **16,494/16,494** matched Stage57 helper rows; 18 extra tail rows excluded; **0 ambiguous, 0 duplicate helper slots**.
- **359/359** Stage56 direct-writer events are uniquely nested in Stage55 ACDF0, **all between Stage57 early `ACE11` POST and final `ACFD6` PRE**. No ambiguous/unmatched direct writer.
- Side P1/P2 object and raw-X values are read from the *same* outer and nested snapshots; no mixing different runs.

## Exact X decomposition inside ACDF0

Stage55 `A11A0→ACDF0` contains **744 net raw X changes** (P1 389 / P2 355) and **zero Y changes** among 5,498 windows. The nested boundary analysis resolves **all 744**:

| Ordered phase / transition | P1 X changes | P2 X changes | Total X changes |
|---|---:|---:|---:|
| ACDF0 PRE → ACE11 PRE | 0 | 0 | 0 |
| ACE11 native PRE → POST | 0 | 0 | 0 |
| **ACE11 POST → ACFD6 PRE (inner loop/gap)** | **121** | **206** | **327** |
| **ACFD6 native `15CEC0` PRE → POST** | **271** | **149** | **420** |
| ACFD6 POST → ACFDB PRE | 0 | 0 | 0 |
| ACFDB native `15B8A0` PRE → POST | 0 | 0 | 0 |
| ACFDB POST → ACDF0 POST | 0 | 0 | 0 |

- Exactly **324** outer windows changed X **only in the pre-ACFD6 gap**; **417** changed X **only inside the ACFD6 call**; **3** (all P1) changed in **both** phases.
- So **327 + 420 − 3 = 744 unique changed windows**. Other 4,754 ACDF0 windows were X-unchanged, with **zero observed phase changes**. There is **no unexplained post-ACFDB or inter-tail net X delta** in this same-run capture.
- **ACFD6→15CEC0 contributes an observed X before/after difference in 420 calls.** This does **not** identify an individual store instruction in that callee or prove which nested function writes X; it is a *call-boundary* localization.

## Breakdown of 327 pre-ACFD6-gap changes

The **359 Stage56 ACF2A store observations** all happened inside this gap, and **112 actually changed raw X** (all **P1**, 0 P2).

| Explanation within gap | P1 windows | P2 windows | Total |
|---|---:|---:|---:|
| Stage56 ACF2A observed raw-X-changing store | **112** | **0** | **112** |
| **Gap X change with no Stage56 store event at all** | **9** | **206** | **215** |
| **Total** | **121** | **206** | **327** |

- All 112 changing Stage56 writer events satisfy **writer PRE raw X = ACE11 POST raw X** and **writer POST raw X = ACFD6 PRE raw X** for the same side/object. **0 state-chain mismatches**.
- 247 Stage56 events targeted P1 (112 changed X); 112 targeted P2 (all raw-X-unchanged). The other **215 gap changes have no instrumented Stage56 writer event** for that side/object.
- Of the 420 `15CEC0`-changed windows, **394** had no Stage56 event, **23** had an unchanged Stage56 store, and **3** also had a changing Stage56 store.

This capture is different from the previous Stage56 run with 1,337 changed ACDF0 windows and 272 direct-store changes. **Do not combine row counts from distinct runs.**

## Static pointer to Stage58

The original SHA-pinned ACDF0 native code, as independently disassembled from the private PE32, contains several **conditional CALL sites between ACE11 and ACFD6**, besides already instrumented ACF2A/ACFA3:

- `ACED5→1CB8B0` and `ACF53→1CB8B0` (validator function; short original native disassembly appears read-only);
- `ACEE8→1CB850` and `ACF60→1CB850` (another short read-only validator);
- **`ACF0F→B86A0`, `ACF7A→B86A0`**, more complex boolean check/call trees;
- **`ACF8A→E9A80`**, `thiscall` with one stack parameter and `RET 4` (nontrivial native that writes internal object fields).
- Loop enumeration `9EE10`/`9EEA0` appears read-only in native disassembly; this does **not** prove absence of concurrent modifications or subtle call-tree effects elsewhere.

**Proposed Stage58:** instrument *only* carefully validated conditional call sites/call trees in this **pre-tail gap**, starting from the complex `B86A0` and `E9A80` sites; confirm exact original CALL bytes, argument forwarding/callee stack cleanup, and PE relocation non-overlap before any patch. Snapshot raw P1/P2 X before/after each helper and match Stage55/56/57 by side, object, thread, QPC. Target explaining **215 unmatched gap changes** (P1 9 / P2 206). Preserve Stage57 DLL as rollback. Avoid changing AI policy or game state until the origin is understood.

**Not established:** AI improvement, hit/collision changes, animation causality, exhaustive executed instructions inside 15CEC0, or byte-for-byte ABI equivalence of instrumentation under every x87 exceptional state.

## Reproduction expectations

Use the four pinned same-session raw logs. Require 5,498 complete triple helper joins and 359 direct-store joins, verify raw-X phase equality and counts; exclude 18 unparented Stage57 tail records rather than inventing parents. Use the Stage57 source branch static verifier for original callsite/prologue identity; it does not establish actual native ABI equivalence. No full programmatic CSV is claimed as published in this report.
