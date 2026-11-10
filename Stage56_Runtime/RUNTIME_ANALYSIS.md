# Stage56 runtime analysis — ACF2A confirmed, additional ACDF0 X writers remain (2026-11-10)

**STATUS: Windows x86 BUILD and runtime VERIFIED for this capture; instruction-level coverage INCOMPLETE.** Stage56 is passive diagnostic instrumentation, not an AI behavior or combat improvement.

## Immutable evidence / source

- Evidence snapshot: [main upload commit `0a60de5d576efedb763daef9d1b1be4932cb8384`](https://github.com/Hawky11/1/commit/0a60de5d576efedb763daef9d1b1be4932cb8384), which follows two same-session trace uploads [`23d451f`](https://github.com/Hawky11/1/commit/23d451f6234b89f184d8ec17612b07ff49c4f558) and [`9307fc2`](https://github.com/Hawky11/1/commit/9307fc20a2481a7e40b0374829ac52df9ed8cd78).
- Tested source: [`stage56-acdf0-two-fstp-writer-observer`](https://github.com/Hawky11/1/tree/stage56-acdf0-two-fstp-writer-observer), original SHA-pinned AALib.exe `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Original EXE remains private.
- Frozen uploaded Git blob SHA and byte lengths:

| File (root of evidence commit) | Git blob SHA | Decoded text chars |
|---|---|---:|
| `Build.log` | `0d8c1ca60046eba4a4740cbc554ea60a41e8d3f5` | 1,918 |
| `ArcanaAI_Stage47MotionConsumerTrace.log` | `f0e3e697da48fcba9e315639745d393dfd82f4d8` | 3,533,320 |
| `ArcanaAI_Stage54PreParentCoordinateTrace.log` | `2ca162e22575576b3217adb83391f15ecb09aee8` | 9,545,059 |
| `ArcanaAI_Stage55EarlierMotionCoordinateTrace.log` | `55da5df7c4ecdd451f9100a7301f6e58c722aa44` | 9,372,877 |
| `ArcanaAI_Stage56ACDF0WriterTrace.log` | `533fe8b685890a255ab2df735695891b80d3a9b3` | 141,018 |

The lengths above are **decoded text character counts returned by the GitHub connector**, not independently verified raw Git blob byte lengths. The Git blob SHA values identify exact original bytes. **Do not overwrite the pinned evidence branch or compare to prior session's 480 or 504 X differences.**

## Build and install

Windows x86 Build.log started **2026-11-10 19:55:55 local**, **BUILD SUCCEEDED**, `dinput8.dll` **1,127,936 bytes**, pre-existing C4754 warning only. Runtime 19:56:41 markers:

- `STAGE56_ACDF0_WRITER_BUILD=2026-11-10-r1`
- `STAGE56_INSTALLED 2/2 ACF27+ACF2A ACFA0+ACFA3`
- `STAGE55_INSTALLED 3/3`, `STAGE54_INSTALLED 3/3`, Stage47 original calls retained.

## Integrity

| Log | Rows | Call distribution | Full field mask | HEALTH |
|---|---:|---|---|---|
| Stage47 | **6,194** (3,097/side) | all AE70C; AE93E none this run | 1FFF | 28 |
| Stage54 | **18,990** | three sites 6,330 each | 1FFF | 28 |
| Stage55 | **18,648** | three sites 6,216 each; 9,324/side | 1FFF | 28 |
| Stage56 | **593** | all ACF27/ACF2A; none ACFA0/ACFA3 | rawX reads 1/1 | 21 |

Each log has unique contiguous `seq=1..N`. Stage55/54/47 reported health counters zero where applicable; Stage56 health totals `calls=593 rows=593 otherObjects=0 dropped=0 overflow=0 mismatch=0 readFail=0`. No reported ERROR/FAILED/FATAL in reviewed logs. These checks do not establish full gameplay correctness.

## Stage47 → Stage55 → Stage54: same-run coordinate attribution

All joins use **side, player, object, thread** and correctly bounded QPC. For each adjacent boundary, require first subsequent PRE within 5,000 QPC ticks and a unique matched target; nested Stage47 requires both timestamps inside AE6E0. Compare raw DWORD X/Y, not float display rounding.

- **6,194/6,194** Stage47 AE70C windows each nest in exactly one AE6E0 Stage55 A119B parent, one nested window per matched parent. Stage47 POST X/Y = enclosing AE6E0 POST X/Y in every matched row.
- **6,216/6,216** AE6E0 POST→ACDF0 PRE matched, with identical X/Y/anchor X/anchor Y; gap 3–226 ticks.
- **6,216/6,216** ACDF0 POST→AC8A0 PRE matched, identical four raw coordinate fields; gap 3–119 ticks.
- **6,210/6,216** AC8A0 POST→first Stage54 PRE matched, same raw fields; gap 134–774 ticks. **Six terminal AC8A0 windows** lack subsequent first-Stage54 samples.
- **6,194/6,194** Stage47 POST→first Stage54 PRE matched uniquely, gap 298–1,398 ticks; **1,337 X differences (P1 741, P2 596), 0 Y differences**. Stage47 X POST = Stage54 anchor X PRE in **6,194/6,194**.
- ACDF0 has **1,337/6,216** X-changing native windows (P1 741/P2 596), **0 Y**, and exactly f1 O+7C changed. AC8A0 has zero changes; all 18,990 Stage54 sibling windows remain unchanged in the 13 watched fields. Thus the same-run changed Stage47→Stage54 windows are attributed to the entire ACDF0 call, as in Stage55.

**Excluded:** 22 AE6E0 parent windows with no Stage47 nested partner, six Stage55 end-tail AC8A0 windows without Stage54 successor and 120 first-Stage54 rows without a matching Stage55 predecessor. No cross-session sampling.

## Stage56 writer observer ↔ ACDF0 native boundary

Compare each Stage56 `STAGE56_WRITER` record with Stage55 A11A0→ACDF0 window using exact side/player/object/thread and **both** writer QPC bounds contained in the parent. All 593 writer rows have **one and only one** parent, with no unmatched or ambiguous assignments. In this capture each matching parent contains a single Stage56 writer event.

| Classification | Count |
|---|---:|
| Stage55 ACDF0 native windows | **6,216** |
| ACDF0 windows with changed X | **1,337** |
| Writer events at **ACF27 + ACF2A** | **593** (P1 231, P2 362) |
| Events at **ACFA0 + ACFA3** | **0** |
| ACF2A events with different raw X before/after | **272** (P1 69, P2 203) |
| ACDF0 changed-X windows with any Stage56 writer | **289** |
| Changed-X ACDF0 windows **with no Stage56 writer** | **1,048** |
| Changed-X windows completely explained by Stage56 event's PRE/POST raw X | **225** |
| Changed-X windows with a Stage56 event but not explained | **64** |
| Unchanged-X ACDF0 windows with a Stage56 writer | **304** |

**Critical fine point:** In the **64** changed windows with a writer but incomplete explanation, **all 64** have *ACDF0 PRE X = writer PRE X*, but **ACDF0 POST X != writer POST X**. There are **zero** writer PRE mismatches. This directly shows that an additional X change occurs **after the observed pair** in these 64 complete native windows. It does **not** identify a specific following helper or store. In the 1,048 changed windows with no Stage56 writer, an unobserved X path is active somewhere in the ACDF0 call.

There are 304 unchanged native windows with writer events. Among 289 changed native windows with writer events, 225 are completely explained by raw pair values and 64 have extra later X changes. The 272 observed changing ACF2A events establish that ACF2A can write the tracked P1/P2 X, but they are **not sufficient to account for all net X changes**. The ACFA3 path was **unexecuted on tracked P1/P2** in this session; that does not prove it never executes in other scenarios.

## Conclusion / Stage57

**ACF2A is now a runtime-observed exact X writer for P1/P2; ACDF0 contains additional unobserved X-writing activity.** The native boundary explains all 1,337 Stage47→Stage54 X differences, but the two direct candidate stores alone account for only 225 native-window net changes without an unexplained residual.

Static original AALib.exe inspection shows a pair of **consecutive final no-stack-argument calls** inside ACDF0:

- `ACFD6: E8 E5 FE 0A 00` → native RVA **15CEC0** (original VA 55CEC0).
- `ACFDB: E8 C0 E8 0A 00` → native RVA **15B8A0** (original VA 55B8A0).

Stage57 should observe these two native calls passively *within* the ACDF0 window to test whether they account for the 64 confirmed **post-ACF2A** X changes and the 1,048 no-writer X changes. Also consider early `ACE11→A2C40` if final helpers do not explain changes. Do not modify AI logic yet.

**Last-known-working Stage56 DLL:** preserve it; Stage57 source/build/runtime are separate. No direct evidence of enhanced AI, actions, hits, damage, or collision logic.

## Reproduction

```powershell
py -3 Stage56_Analysis\tools\analyze_stage56.py --stage55 "ArcanaAI_Stage55EarlierMotionCoordinateTrace.log" --stage56 "ArcanaAI_Stage56ACDF0WriterTrace.log" --out Stage56_Runtime
```

This stdlib analyzer is included on the Stage56 source branch and generates `summary.json` and `stage56_stage55_writer_join.csv` covering **all 6,216** ACDF0 windows. It does **not** by itself verify Stage47→Stage54 complete-chain correlation; that was independently computed from the same immutable session in this report. Review health and event timestamps, never join logs from different runs.
