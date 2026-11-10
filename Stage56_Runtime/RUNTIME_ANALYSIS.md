# Stage56 runtime — ACF2A executes; other ACDF0 X writes remain (2026-11-10)

**Status: Windows x86 build/runtime VERIFIED for installed Stage56 r1. Instruction-level explanation is PARTIAL.** No proven AI/gameplay improvement.

## Provenance

- Immutable fresh root-log capture through [upload commit `0a60de5d576efedb763daef9d1b1be4932cb8384`](https://github.com/Hawky11/1/commit/0a60de5d576efedb763daef9d1b1be4932cb8384) (Build.log). The preceding same-session uploads: [Stage53–56 logs `9307fc2`](https://github.com/Hawky11/1/commit/9307fc20a2481a7e40b0374829ac52df9ed8cd78), [Stage47–52 logs `23d451f`](https://github.com/Hawky11/1/commit/23d451f6234b89f184d8ec17612b07ff49c4f558).
- Tested Stage56 source: [`stage56-acdf0-two-fstp-writer-observer`](https://github.com/Hawky11/1/tree/stage56-acdf0-two-fstp-writer-observer). Original SHA-pinned private `AALib.exe` SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
- Evidence branch: [`stage56-runtime-acdf0-writers-evidence`](https://github.com/Hawky11/1/tree/stage56-runtime-acdf0-writers-evidence), pinned to the complete uploaded logs without modifying original bytes.

## Build, install, integrity

Windows x86 `Build.log` started **10.11.2026 19:55:55** local log time: **BUILD SUCCEEDED**, DLL **1,127,936 bytes**, existing compiler warning C4754 only.

- Stage56 marker `STAGE56_ACDF0_WRITER_BUILD=2026-11-10-r1` and `STAGE56_INSTALLED 2/2 ACF27+ACF2A ACFA0+ACFA3` confirmed at 19:56:41.
- Stage55 marker `STAGE55_INSTALLED 3/3` retained; Stage47/53/54 same-run logs are present and show no installation/runtime ERROR strings from sampled scans.
- Stage55: **18,648** unique contiguous rows (seq 1..18648), **6,216** each for A119B/AE6E0, A11A0/ACDF0, A11A5/AC8A0; P1 9,324 / P2 9,324. Masks all `1FFF`. **28 HEALTH** with missingObjects, dropped, overflow, mismatch, badCaller all zero.
- Stage56: **593** unique contiguous writer rows (seq 1..593), 21 HEALTH with dropped/overflow/mismatch/readFail all zero, otherObjects zero. **All 593** have both `preRead=1` and `postRead=1`.
- Stage47/53/54 inspected logs have 28 HEALTH each and reported loss/error counters zero. A broad string check found no ERROR/FAILED/FATAL entries in the reviewed fresh trace files. This is not an exhaustive guarantee of gameplay correctness.

## Direct Stage55 ACDF0 function-boundary observations

| Stage55 boundary | Rows | X O+7C changes | Y O+80 changes |
|---|---:|---:|---:|
| A119B→AE6E0 | 6,216 | 2,835 | 3,003 |
| **A11A0→ACDF0** | **6,216** | **1,337** | **0** |
| A11A5→AC8A0 | 6,216 | 0 | 0 |

ACDF0 changed X: **P1 741; P2 596**. All 6,216 ACDF0 PRE/POST windows had full watched-field masks. The previous Stage55-only run's **480** differences (P1 248/P2 232) are from a different session; do not combine counts.

## Stage56 actual instruction observations

| Site | P1 events | P2 events | Raw-X-changing events |
|---|---:|---:|---:|
| **ACF27 (FADD) / ACF2A (FSTP)** | 231 | 362 | **272** (P1 69; P2 203) |
| ACFA0 (FADD) / ACFA3 (FSTP) | 0 | 0 | 0 |
| **Total** | **231** | **362** | **272** |

Each writer row has its real target object address (EAX), side, player, thread, QPC, and raw X before/after. All 593 target a tracked P1/P2 object; no row from the second `ACFA3` site in this capture. A missing event from the second site does **not** prove it can never execute.

## Exact same-run writer → ACDF0 interval nesting

Strict match uses **same side + player + object + thread**, requires *both* `writer.qpcBefore >= ACDF0.qpcBefore` and `writer.qpcAfter <= ACDF0.qpcAfter`, and requires a unique target parent. All **593/593** events match exactly one ACDF0 Stage55 window. There are **0 unmatched**, **0 ambiguous**, and **0 duplicate target assignments**; exactly one writer event in each of the 593 matched parent windows.

- **6,216** ACDF0 parent windows = **1,337 changed X** + **4,879 unchanged X**.
- Of the **1,337 changed** parent windows: **289** contained a Stage56 writer event and **1,048 did not contain any**.
- Of the 289 changed windows with a writer event, **225** had identical Stage55 PRE X → Stage56 PRE X and Stage56 POST X → Stage55 POST X. In the other **64**, Stage55 PRE X equals writer PRE X, but writer POST X **does not equal** Stage55 POST X. Hence an additional X change was observed after that writer event and before the enclosing function returned (not necessarily by a specific identified instruction).
- Of the **4,879 unchanged** parent windows, **304** had one writer event and **4,575** had none; all 304 matched the unchanged parent net X. Some of these events execute the store without changing raw X.
- Every **593/593** writer PRE value equals its containing Stage55 ACDF0 PRE X, including all 64 POST-mismatch cases. The later X effect cannot be attributed to an earlier write inside these parent windows.
- Therefore **only 225 of 1,337 changed Stage55 ACDF0 windows are fully accounted for by the observed Stage56 writer alone** under raw PRE/POST equality. That is **not** the number of all actual store executions: **272** of the 593 instrumented stores changed X directly.

### What is proven, and what is not

**Proven:** Original site **ACF2A** does execute for the tracked P1/P2 objects and changes raw X in this capture. However, the installed instrumentation of **ACF2A and ACFA3 together cannot account for all ACDF0 function-boundary changes**. More than three quarters of changed windows (1,048/1,337) have no observed direct-site hit at all; a further 64 have a different final raw X than the directly observed store output.

**Not proven:** which individual other instruction or nested helper caused the residual X differences; that ACFA3 is unreachable; that the AI became smarter or that visible animation, collisions, hit/damage semantics changed. Timing or exceptional x87 behavior under detours is not established as byte-for-byte identical.

## Next Stage57: locate other changes within ACDF0

Static reinspection of SHA-pinned original ACDF0 reveals two *unobserved subsequent native CALL boundaries* immediately before function return:

```
ACFD6: E8 E5 FE 0A 00    CALL RVA 15CEC0 (VA 55CEC0)
ACFDB: E8 C0 E8 0A 00    CALL RVA 15B8A0 (VA 55B8A0)
ACFE0: 5F ...            original epilogue
```

These are **candidates**, not established writers. ACDF0 also makes earlier helper calls (e.g., A2C40, ACB80, 5CB8B0, 5CB850, B86A0, E9A80), and coordinate writes may occur in any of their executed call trees. First perform static entry/return/stack and relocations checks for tail helpers, then bracket them *passively* to test whether they account for residual X changes. If both are unchanged, move earlier in ACDF0. Preserve Stage55 and Stage56 provenance. **Stage57 source is not part of this report and remains a separate task.**

## Reproducibility

The Stage56 branch contains stdlib [`Stage56_Analysis/tools/analyze_stage56.py`](https://github.com/Hawky11/1/blob/stage56-acdf0-two-fstp-writer-observer/Stage56_Analysis/tools/analyze_stage56.py), which is designed to join writer windows using exact object/thread/QPC. The reported counts above were independently derived from the pinned raw GitHub blobs using in-session parsing of Stage55 and Stage56; the published Python tool was **not** executed against a local complete evidence checkout during this review. No generated CSV is claimed here.
