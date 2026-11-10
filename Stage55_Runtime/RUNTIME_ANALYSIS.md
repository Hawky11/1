# Stage55 runtime — ACDF0 whole-function X attribution (2026-11-10)

**Status: VERIFIED for the observed capture as reported in the Stage55 same-run analysis.** This is **function-boundary** attribution, **not instruction-level writer attribution** and not a claim about better AI, collisions, animations, hits, or damage.

- Immutable evidence commit: [`c27b6b516edcea4551de9e52e6a151acdc115279`](https://github.com/Hawky11/1/commit/c27b6b516edcea4551de9e52e6a151acdc115279).
- Tested source: Stage55 [`00614967dbc386822d75b4bb37954d4f6600a439`](https://github.com/Hawky11/1/commit/00614967dbc386822d75b4bb37954d4f6600a439).
- Raw uploaded root logs and source/history branches remain unchanged. The evidence for this run is pinned by the commit above, not by mutable `main`.
- The detailed numerical same-frame correlation below is carried forward from the completed Stage55 analyst report. The Build.log was separately re-read from the pinned GitHub commit. Do not treat this prose as an independent re-execution of `analyze_stage55.py` without that script and original log bytes.

## Build and installation

Windows x86 Build.log began **2026-11-10 18:51:03** (log-local time), reports **BUILD SUCCEEDED** and **dinput8.dll 1,122,816 bytes**, with pre-existing nonfatal C4754 warning. Stage55 r1 marker and **STAGE55_INSTALLED 3/3** at A119B→AE6E0, A11A0→ACDF0, A11A5→AC8A0 were reported in the same-session runtime log. Stage47/49/50/51/52/53/54 hooks remained installed.

## Capture integrity

| Stage | Window rows | Capture notes |
|---|---:|---|
| 55 | **19,764** | Seq 1..19764 contiguous/unique; P1 9,882, P2 9,882; A119B, A11A0, A11A5 each 6,588 (3,294/side) |
| 47 | 6,576 | All observed events AE70C; no AE93E in this run |
| 49 | 6,638 | pre/post masks 07FF |
| 50 | 6,640 | pre/post masks 07FF |
| 51 | 6,642 | pre/post masks 1FFF |
| 52 | 13,292 | pre/post masks 1FFF |
| 53 | 6,646 | pre/post masks 1FFF |
| 54 | 19,944 | pre/post masks 1FFF |

All listed logs have unique, contiguous sequence IDs and 29 HEALTH samples with zero reported missingObjects/dropped/overflow/mismatch/badCaller. Stage55 has complete pre/post 1FFF masks on all windows. Broad root-log scan for ERROR/FAILED/FATAL was negative, but does not certify every legacy log or gameplay correctness.

## Stage55 within-call before/after field changes

| Native boundary | Rows | X O+7C differences | Y O+80 differences | Notes |
|---|---:|---:|---:|---|
| A119B→AE6E0 | 6,588 | 2,722 | 2,816 | Contains normal Stage47 motion; nested Stage47 POST must be compared |
| **A11A0→ACDF0** | **6,588** | **480** | **0** | Only f1 / O+7C changed, X anchor O+AC unchanged |
| A11A5→AC8A0 | 6,588 | 0 | 0 | All 13 watched fields unchanged |

The ACDF0 X changes split **P1 248 / P2 232**. Stage54 has **19,944 unchanged** windows. Stage53 and Stage52 AEF10 each show the same four O+18 0→4 transitions (two per side, initialization/reinitialization), no X/Y changes; Stage49–51 and Stage52 AED90 remain unchanged in watched fields.

## Same-run correlation — exactly matched objects and intervals

Matching uses the **same side, player, object and thread**. Nested Stage47 interval must lie within both QPC bounds of its parent AE6E0 window. Adjacent joins require the **first subsequent PRE within 5,000 QPC ticks**, unique target assignments, and exclusion of unmatched stale tails.

- **6,576/6,576** Stage47 AE70C windows are contained in exactly one A119B→AE6E0 parent window; one Stage47 window per matched parent.
- **6,576/6,576** Stage47 POST X/Y = enclosing AE6E0 POST X/Y; no extra X/Y drift after the observed nested Stage47 window.
- **6,588/6,588** AE6E0 POST→ACDF0 PRE unique matches, QPC gaps 3–77; X/Y/anchor X/anchor Y all equal.
- **6,588/6,588** ACDF0 POST→AC8A0 PRE unique matches, QPC gaps 3–231; X/Y/anchors all equal.
- **6,588/6,588** AC8A0 POST→first Stage54 PRE unique matches, QPC gaps 65–880; X/Y/anchors all equal.
- **6,576/6,576** Stage47 POST→first Stage54 PRE unique matches, QPC gaps 258–1,185; **480 raw X differences, zero Y differences**. Stage47 POST X equals Stage54 anchor X in all 6,576.
- **6,576 complete chains** pass every boundary. **All 480 changed chains isolate the raw X difference to ACDF0**, not merely a coincidence of separate aggregate counts.

**Excluded explicitly:** 12 Stage55 parent chains lack Stage47 counterparts; another 60 first-Stage54 rows lack Stage55 predecessors. The extra 12 Stage55 chains have no additional ACDF0 X changes. They must not be compared with stale prior frames. The **504** X differences from the earlier Stage54-only run are a different session and must never be added to the **480** here.

## Established result and limitations

**ACDF0 is the complete *native function boundary* accounting for all 480 Stage47→Stage54 raw X differences in the 6,576 fully matched chains of this capture.** This does *not* prove which instruction made the change. Static candidates **ACF2A** and **ACFA3** both contain `D9 58 7C` (`FSTP dword ptr [EAX+7C]`), but ACDF0 invokes other helpers. Any instruction-level claim requires a passive per-writer probe and exact object/write-value correlation.

No new AI commands are implemented in this stage; the observation is numerical. Hardware/binary instrumentation can itself influence timing. No controlled gameplay A/B conclusion follows from these counters.

## Stage56 handoff — pending source work

1. Inspect ACDF0 internal paths and the two native candidate store instructions **against the original SHA-pinned AALib.exe** (SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`; private binary must not be committed).
2. Check instruction lengths, source/target address conventions, PE relocation overlap, surrounding branches and calling convention. Confirm how to observe exactly one original store, including FPU operand semantics and x87 state.
3. Design **passive per-writer PRE/POST** logging with exact target object, before/after raw X, side/thread/QPC, instruction site and loss/error counters. Preserve GPR, EFLAGS, x87/SSE, stack and execution frequency; abort safely if signatures disagree.
4. Join Stage56 writer events to the same Stage55 ACDF0 windows by object/thread and nested QPC. Count both matched and unmatched changes and investigate helper stores if ACF2A/ACFA3 fail to explain all changes.
5. **Current Stage55 DLL remains the verified executable.** Stage56 source/build/runtime are **not implied by this report**; no Stage55 rebuild is needed.

## Reproduction notes

Analysis specification: stdlib Python `Stage55_Runtime/analyze_stage55.py`, expected to emit `summary.json` and `sameframe_join.csv` (all **6,576** complete chains, including unchanged rows). This report records the original analyst's reproduction command; publication/verification of that script and output CSV are separate artifacts and must be checked before claiming independently rerunnable results. Evidence checkout must be at the commit above. Retain original log bytes and avoid re-uploading over them.
