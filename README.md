# ArcanaAI / COM vs COM diagnostic research

This repository tracks the staged investigation of the ArcanaAI COM-vs-COM event/queue path.

## Start here

1. Read **[NEXT_SESSION.md](NEXT_SESSION.md)** first. It is the current handoff for the next AI/chat and contains the exact next task.
2. Read **[PROJECT_HISTORY.md](PROJECT_HISTORY.md)** for the append-only stage history and the evidence behind the current conclusions.
3. Open the referenced stage branch and inspect its README, overlay, and runtime logs before making changes.

## Current checkpoint

- Latest runtime evidence branch: [stage18-bit113-gate-runtime-evidence](https://github.com/Hawky11/1/tree/stage18-bit113-gate-runtime-evidence), HEAD 0139152b9eeb778a23ec4f3197c57faff749a78a. It contains the 2026-11-08 18:00 Stage18/17/16 runtime logs, the exact Stage18 overlay, and STAGE18_RUNTIME_ANALYSIS.md. Build.log was not included in the upload; runtime markers nevertheless confirm that the DLL loaded and both hooks installed. The Stage17 evidence branch remains unchanged.
- Current diagnostic-source branch: [stage17-callsite-init-diagnostics](https://github.com/Hawky11/1/tree/stage17-callsite-init-diagnostics), HEAD `dcfd1efbe221438b1f2b3fae800ca76b1fa87e7d`. It adds an early Stage17 startup/build marker, INI-value logging, early-exit diagnostics, correct CRLF log formatting, and the `ReadDwordAtAddress` forward declaration. The corrected r2 source **built successfully** on 2026-11-08 at 16:32:02 (198,144-byte DLL) and was runtime-validated at 16:34.
- Stage17 callsite implementation in that branch hooks the direct CALL at `AALib.exe+0xC6502`, expected to pass bit index `0x71` to the generic setter at `0xFAA0`. This is a different implementation snapshot from the earlier `stage17-bit113-setter-caller-trace` entry-hook branch; keep both branches intact.
- Stage18 source branch: [stage18-bit113-gate-condition-trace](https://github.com/Hawky11/1/tree/stage18-bit113-gate-condition-trace), HEAD 520eec4cd216795e8ffbaa8738b3602b5e40d39a. Its passive observer after helper 0x4F2FD0 is now runtime-validated in the companion evidence branch. Six gate results returned AL=1 with table-byte readback 0xFA; these correlated with six Stage17 setter calls and three Stage16 bit17 0→1 transitions. Three target payload-0x1E attempts then reached F6A68 with bit17=1 and were suppressed. See STAGE18_RUNTIME_ANALYSIS.md in the evidence branch.
- **Builds succeeded** twice on 2026-11-08. The earlier runtime-test build linked `build\\dinput8.dll` at 190,976 bytes. The corrected r2 diagnostic source then built at 16:32:02 with VS 2026 18.9.1 / MSVC 14.51.36231 x86; all eight translation units compiled and the new DLL linked at 198,144 bytes. Both builds emitted the same non-fatal C4754 warning in `AICommandSiteTrace.cpp` lines 782–785, in the stack-address bounds calculation.
- The earlier 15:40 run remains the Stage16 baseline with 22,739 write records and six bit17 transitions. The new 16:34 Stage17 run contains 20,456 Stage16 write records, 62 watchpoint installations across 10 batches, four bit17 set transitions and four clears, with no warning/error/drop reports in the reviewed writer/producer/queue/dispatch logs.
- In the new 16:34 run, four Stage15 F6A68 events all saw bit17=1 and did not take the JZ; none reached F6A9D or F6AD7. QueueTrace and EventDispatchTrace contain no type-1 / payload-`0x1E` event for this run. This is consistent with the bit17 suppression path; it is not a successful insertion sample.
- **Stage17 caller attribution is now runtime-confirmed for this tested scenario.** The r2 log reports the build marker, all three INI values, and successful installation of the passive hook at RVA `0x000C6502` (return `0x000C6507`, original helper `0x0000FAA0`, 34 suspended threads). It records nine `BIT113_SET_CALL` entries. The four actual Stage16 0→1 changes to P1/P2 `player+0x3B84` each have a preceding call from this exact site with index `0x71`, matching player pointer, and QPC gaps of 367, 520, 396, and 353 ticks. Five further calls were observed while bit17 was already set. This confirms the candidate callsite for all four transitions in this run, not every possible game path. See the 2026-11-08 Stage17 runtime section in `PROJECT_HISTORY.md`.
- Historical Stage12–Stage17 evidence branches remain unchanged. Do not rewrite them.

## Repository workflow

- Treat stage branches as immutable historical snapshots after a test run is completed.
- Create each diagnostic stage as a new branch from the immediately preceding completed stage. Stage16 was branched from the exact Stage15 HEAD above; do not rewrite Stages12–15.
- Before analysis, check the target branch HEAD and compare its files with the last reviewed commit. A newly uploaded archive or README alone is not a new runtime test.
- A completed run should include `Build.log` and the relevant runtime logs. Confirm hook installation from the runtime log; do not infer it from source code alone.
- Compare new logs with the previous stage/run, correlate events by QPC/timestamp and arguments, and distinguish observed facts from hypotheses.
- After each stage, append its findings and commit/branch references to `PROJECT_HISTORY.md`, and update `NEXT_SESSION.md` with the latest checkpoint and next concrete task. Keep this root README current if the active stage changes.

The default `main` branch is the documentation index and aggregate log snapshot. Stage overlays and their exact test snapshots live on their named stage branches.
