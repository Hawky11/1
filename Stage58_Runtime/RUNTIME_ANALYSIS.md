# Stage58 runtime analysis — conditional helper is not the observed X writer

**Date:** 2026-11-10. **Status:** Windows x86 build and in-game COM-vs-COM capture VERIFIED; candidate helper attribution LIMITED to this run. **No gameplay/AI improvement is established.**

## Evidence and provenance

- Original five-file, single-session user upload: [commit 5c90de3](https://github.com/Hawky11/1/commit/5c90de3d792f366a6ffd910187859b8b0ce020b1), followed by [commit bd62b33](https://github.com/Hawky11/1/commit/bd62b33a751c679f7e9f0a9a2b773059b5b36590). Frozen raw main snapshot: [stage58-runtime-conditional-no-x-evidence](https://github.com/Hawky11/1/tree/stage58-runtime-conditional-no-x-evidence).
- Stage58 source is on the **unchanged** [stage58-acdf0-pre-tail-conditional-calls](https://github.com/Hawky11/1/tree/stage58-acdf0-pre-tail-conditional-calls) branch. Do not confuse source-ready or a successful build with verified behavioral equivalence.
- Log blob SHAs on inspected latest main: `Build.log` `4f92f7a86d0a0b1c380a62546fae0cd2047d9ada`; `ArcanaAI_Stage55EarlierMotionCoordinateTrace.log` `200d6e60991347c35da1218ab9986d43aa4b10b5`; Stage56 `ba24b4d00c729865338071999f5c72445414caaf`; Stage57 `c37904b058266f1534ef1e942773fe01b7e39271`; Stage58 `80510eb7fd3c9f63f9059b0b26e758db82e361ae`.
- Build started `10.11.2026 20:54:37` local: **BUILD SUCCEEDED**, `build/dinput8.dll` **1,145,344 bytes**; only the pre-existing MSVC C4754 warning. Runtime initialized 20:55:06; matched observation rows timestamped roughly 20:55:20–20:55:47.
- Startup markers: `STAGE55_INSTALLED 3/3`, `STAGE56_INSTALLED 2/2`, `STAGE57_INSTALLED 3/3`, `STAGE58_INSTALLED 3/3`. This does **not** prove all three Stage58 conditions executed.

## Capture integrity

| Evidence | Rows | Site distribution | HEALTH records |
|---|---:|---|---:|
| Stage55 | 16,614 unique seq 1..16614 | 5,538 each A119B/AE6E0, A11A0/ACDF0, A11A5/AC8A0 | 23 |
| Stage57 | 16,614 unique seq 1..16614 | 5,538 each ACE11/A2C40, ACFD6/15CEC0, ACFDB/15B8A0 | 23 |
| Stage56 | 380 unique seq 1..380 | 380 ACF27/ACF2A, zero ACFA0/ACFA3 | 18 |
| Stage58 | 822 unique seq 1..822 | **822 ACF0F/B86A0**, zero ACF7A/B86A0, zero ACF8A/E9A80 | 18 |

All Stage55/57/58 13-field PRE/POST masks are `1FFF`. Stage56 before/after read flags are 1. Across inspected HEALTH lines: missingObjects, otherObjects, dropped, overflow, mismatch, badCaller and readFail (where applicable) all **zero**. Stage58 822 per-side rows represent **411 native CALL invocations**, as both P1/P2 are sampled per call. `resultEax=0` on 760 rows (380 calls) and `resultEax=1` on 62 rows (31 calls), without a Stage58 field change.

## Strict same-run nested attribution

Compared every Stage55 `A11A0→ACDF0` P1/P2 native window with Stage57 `ACE11`, `ACFD6` and `ACFDB` observations by exact **side, player address, object address, thread ID** and complete **QPC interval containment**, requiring unique results. All **5,538/5,538** outer windows contain exactly one of each Stage57 site; the sampled X state is continuous from parent PRE through the early helper, tail, last helper and parent POST. Stage58 and Stage56 observations were correlated by the same key/QPC within each outer window, not by timestamps alone. No cross-capture counts were mixed.

| X change region | P1 | P2 | Windows |
|---|---:|---:|---:|
| Stage55 ACDF0 whole call | 668 | 511 | **1,179** |
| Stage57 ACE11 POST → ACFD6 PRE (pre-tail gap) | 470 | 306 | **776** |
| Inside ACFD6 → 15CEC0 | 208 | 226 | **434** |
| Both gap and 15CEC0 in the same window | 10 | 21 | **31** |
| Stage57 ACE11 native, ACFDB native, Stage55 AC8A0 native | 0 | 0 | 0 |

Thus **776 + 434 − 31 = 1,179** distinct changed ACDF0 windows. Every changed ACDF0 window has changedMask `0002` (f1 = original object + `0x7C`, raw X); the other 4,359 have `0000`. **No raw Y change** occurred in the ACDF0 windows in this capture. This is a new run: the previous Stage57 capture had 744 total changed windows, and its numbers must not be combined.

### What Stage58 adds

- All **822** Stage58 per-side observations come from `ACF0F→B86A0`. PRE/POST `changedMask=0000` for every Stage58 row, including all 62 rows with `EAX=1`. The two other installed Stage58 conditional sites have **zero observed invocations**, not proven failures.
- Among the **776** pre-tail-gap changed outer windows, **323** have one Stage58 observation and **453** have none. Within the 323 with Stage58: **168** have the X difference already present between Stage57 ACE11 POST and Stage58 PRE; the other **155** acquire the difference between Stage58 POST and Stage57 ACFD6 PRE. No difference occurs inside the Stage58 native observation. These groups are disjoint in this capture.
- All **380** Stage56 `ACF27+ACF2A` direct store observations nest uniquely within Stage55 ACDF0 and follow Stage58; **zero** changed the tracked target X in this run. No Stage56 `ACFA0+ACFA3` observation. This is different from the previous Stage57 capture, in which direct writes were observed to change X; do not extrapolate across runs.
- Neither the observed Stage58 B86A0 call nor the observed Stage56 direct-store snapshots account for a changed tracked X value here. No precise writer instruction, concurrent writer, data race, physics/collision semantic or gameplay effect is established merely by these intervals.

## Original binary static cross-check and next Stage59 scope

Locally rechecked *private* `AALib.exe` SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` (PE32 x86; 175,556,608 bytes). In original ACDF0 disassembly, the visible inline `FSTP DWORD PTR [EAX+7C]` X stores are `ACF2A` and `ACFA3`. `9EE10` and `9EEA0` are read/index helpers; `1CB8B0` / `1CB850` have simple read/test bodies. That static scope **does not rule out writes by indirect callee trees, other threads, hooks, or different pathways**, nor does it prove the observed address is exclusively controlled by ACDF0. The Stage16 system historically uses debug registers DR0/DR1; *do not blindly add another independent DR0/DR1 monitor*, which could conflict.

**Recommended Stage59:** prioritize **actual P1/P2 object+0x7C write attribution**: audit existing Stage16 watchpoint ownership/configuration, then design one compatible, short-lived, read-only writer observer with exact after-write EIP/RVA, thread/QPC, address, before/after raw bits and health counters. If debug-register ownership is unavailable, leave existing watchpoints untouched and consider a narrower per-loop boundary fallback. Verify ABI, thread safety and unmodified in-game behavior before interpreting traces. **Stage59 source/build/runtime are not ready and must not be labeled as tested.** Preserve the Stage58 DLL as rollback baseline; no proven smarter AI, hit/damage change or new animation.
