# Stage59 r1 Windows runtime — BUILD VERIFIED, watchpoints NOT ARMED (2026-11-10)

**Evidence:** [six-file upload commit 7eb07e6](https://github.com/Hawky11/1/commit/7eb07e6bff9392708720cd3ff76ac713703af1c9) and [frozen raw log branch](https://github.com/Hawky11/1/tree/stage59-r1-runtime-timeout-evidence). This is **one failed startup capture**, not a validated memory-writer trace.

## Build and installation

- `Build.log`: Windows x86 MSVC build started **2026-11-10 21:53:04.70**, **BUILD SUCCEEDED**, `build/dinput8.dll` **1,152,512 bytes**. Only old C4754 warning.
- Stage55/56/57/58 install markers present: 3/3, 2/2, 3/3, 3/3.
- `ArcanaAI_Stage59XWriterWatchTrace.log` contains **only 3 lines**:
  - **21:53:34** `STAGE59_X_WRITER_WATCH_BUILD=2026-11-10-r1`
  - **21:53:34** `STAGE59_WORKER_STARTED`
  - **21:53:39** `ERROR: Stage59 timed out waiting for two readable player+4 object pointers; no registers touched.`
- No `STAGE59_OBJECTS_READY`, `STAGE59_WATCH_ARMED`, `STAGE59_THREAD_INSTALLED`, `STAGE59_X_WRITE` or `STAGE59_HEALTH` markers. Stage59 never armed DR0/DR1. The absence of X_WRITE **does not indicate that there were no writes**.

## Independent post-timeout evidence that objects eventually became valid

- Stage55 first complete observed native call at **21:53:53**, first object addresses **P1=2A710020, P2=2A710C50** and player addresses **P1=0E4A1F78, P2=0E4CCCA0**; 13-field PRE/POST read masks `1FFF`.
- Stage57 first helper observation **21:53:54**. Stage58 first `ACF0F` observation **21:53:55**. Stage56 first `ACF2A` observation **21:53:55**. The live observation starts roughly **14–16 seconds after the r1 timeout**, about **19–21 seconds after the r1 worker start**.
- Stage55 12,606 contiguous per-side records (4,202 per site); 368 `ACDF0` outer X changes (P1 184/P2 184), no outer Y changes. Stage57 12,624 contiguous records (4,208 per site), 368 `ACFD6→15CEC0` X changes. The start/end windows differ by six per site, so **do not claim complete 1:1 nesting of the entire run** without an exact QPC join.
- Stage56 176 tracked `ACF2A` FADD/FSTP observations, all unchanged X; no `ACFA3` observed. Stage58 414 per-side `ACF0F→B86A0` observations (207 physical calls), all changedMask=`0000`; other candidate Stage58 sites unobserved.
- HEALTH rows (Stage55 18, Stage57 18, Stage56 12, Stage58 12): no nonzero observed missingObjects/otherObjects/dropped/overflow/mismatch/badCaller/readFail. Stage59 itself has no meaningful coverage counters, since it never armed.

## Root cause and r2 change

Stage59 r1 waits just **100 × Sleep(50ms) ≈ 5 seconds** for both readable `[player+4]` object pointers and X words. It was started during proxy/hook initialization before COM-vs-COM actors existed. This 5-second deadline was exhausted before the first verified game objects were available, so no register installation was attempted. This is a **startup timing issue**, not evidence of an incorrect object offset or native coordinate writer, and not a watchpoint conflict. The r1 error alone cannot prove that 5-second wait was the *only* runtime issue: hardware VEH correctness still needs testing.

**Stage59 r2 source prepared:** [stage59-r2-wait-for-combat-objects](https://github.com/Hawky11/1/tree/stage59-r2-wait-for-combat-objects). Changes **only** the Stage59 observer source's startup wait (from 5 to 600 seconds), adds `STAGE59_WAITING` periodic diagnostics and `STAGE59_OBJECTS_READY` when live actors become readable, changes build marker r1→r2. No native hook patch or coordinate write was changed. For existing r1 users, replace **only** `src/Stage59XWriterWatch.inl` with the [r2 file](https://github.com/Hawky11/1/blob/stage59-r2-wait-for-combat-objects/Stage59_Overlay/src/Stage59XWriterWatch.inl), keep existing Stage59 adapter, `ArcanaAI.ini`, all earlier stages, and Build.bat untouched. [RU one-file retry](https://github.com/Hawky11/1/blob/stage59-r2-wait-for-combat-objects/Stage59_Overlay/README_STAGE59_RU.md).

**Status after fix:** R2 source READY; **R2 Windows build, DR0/DR1 installation, X_WRITE events and AI behavior NOT YET TESTED**. Retain working Stage59 r1 (1,152,512 bytes) and Stage58 (1,145,344 bytes) DLL for rollback. Obtain new same-run Stage59/58/57/56/55 logs plus Build.log before any causal attribution.
