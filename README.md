# ArcanaAI / COM vs COM diagnostic research

This repository tracks the staged investigation of the ArcanaAI COM-vs-COM event/queue path.

## Start here

1. Read **[NEXT_SESSION.md](NEXT_SESSION.md)** first. It is the current handoff for the next AI/chat and contains the exact next task.
2. Read **[PROJECT_HISTORY.md](PROJECT_HISTORY.md)** for the append-only stage history and the evidence behind the current conclusions.
3. Open the referenced stage branch and inspect its README, overlay, and runtime logs before making changes.

## Current checkpoint

- Latest runtime evidence branch: [stage17-bit113-setter-callsite-trace](https://github.com/Hawky11/1/tree/stage17-bit113-setter-callsite-trace), HEAD `0c2411187c47e56a6cb675b597eeb8a754f8f8a6`.
- Current diagnostic-source branch: [stage17-callsite-init-diagnostics](https://github.com/Hawky11/1/tree/stage17-callsite-init-diagnostics), HEAD `34fbf14975b3e5d1b9dd6bdec0c3d6c8756bc25f`. It adds an early Stage17 startup/build marker, INI-value logging, early-exit diagnostics, correct CRLF log formatting, and a forward declaration fixing the `ReadDwordAtAddress` C3861 compile error. It has **not** yet been successfully rebuilt or runtime-tested.
- Stage17 callsite implementation in that branch hooks the direct CALL at `AALib.exe+0xC6502`, expected to pass bit index `0x71` to the generic setter at `0xFAA0`. This is a different implementation snapshot from the earlier `stage17-bit113-setter-caller-trace` entry-hook branch; keep both branches intact.
- **Build succeeded** on 2026-11-08 with VS 2026 18.9.1 / MSVC 14.51.36231 x86. All eight C++ translation units compiled and `build\\dinput8.dll` linked (190,976 bytes). One non-fatal C4754 warning remains in `AICommandSiteTrace.cpp` lines 782–785, in the 64-bit stack-address bounds calculation.
- Runtime logs from about 15:40:27–15:41:09 contain 22,739 Stage16 write records, 60 game-thread watchpoint installations, and no warning/error/drop reports in the reviewed write/producer/queue/dispatch logs.
- Six matched Stage15 events again split exactly by bit17: three with bit17=1 did not take the F6A68 JZ and did not reach F6A9D/F6AD7; three with bit17=0 took the JZ, reached the alternate writer at F6AD7, were accepted by QueueTrace, and were observed by EventDispatchTrace. The three set/clear pairs again use the confirmed setter at RVA `0xFAD0` and clearer at RVA `0xED19F`.
- **Stage17 caller attribution is still pending.** `ArcanaAI_Bit113SetterTrace.log` was not created in the 15:40 runtime run, so the callsite hook's install/validation result and `BIT113_SET_CALL` records remain unknown. Source review found that several early-return paths did not emit any Stage17 log. The diagnostic-source branch now adds a startup marker before those paths. The first marker-verified build exposed a missing forward declaration for `ReadDwordAtAddress`; this has been fixed in the current branch HEAD. Rebuild and verify before another full match. The Build.log was supplied in chat but is not present in the branch tree.
- Historical Stage12–Stage16 evidence branches remain unchanged. Do not rewrite them.

## Repository workflow

- Treat stage branches as immutable historical snapshots after a test run is completed.
- Create each diagnostic stage as a new branch from the immediately preceding completed stage. Stage16 was branched from the exact Stage15 HEAD above; do not rewrite Stages12–15.
- Before analysis, check the target branch HEAD and compare its files with the last reviewed commit. A newly uploaded archive or README alone is not a new runtime test.
- A completed run should include `Build.log` and the relevant runtime logs. Confirm hook installation from the runtime log; do not infer it from source code alone.
- Compare new logs with the previous stage/run, correlate events by QPC/timestamp and arguments, and distinguish observed facts from hypotheses.
- After each stage, append its findings and commit/branch references to `PROJECT_HISTORY.md`, and update `NEXT_SESSION.md` with the latest checkpoint and next concrete task. Keep this root README current if the active stage changes.

The default `main` branch is the documentation index and aggregate log snapshot. Stage overlays and their exact test snapshots live on their named stage branches.
