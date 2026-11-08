# ArcanaAI / COM vs COM diagnostic research

This repository tracks the staged investigation of the ArcanaAI COM-vs-COM event/queue path.

## Start here

1. Read **[NEXT_SESSION.md](NEXT_SESSION.md)** first. It is the current handoff for the next AI/chat and contains the exact next task.
2. Read **[PROJECT_HISTORY.md](PROJECT_HISTORY.md)** for the append-only stage history and the evidence behind the current conclusions.
3. Open the referenced stage branch and inspect its README, overlay, and runtime logs before making changes.

## Current checkpoint

- Latest completed runtime stage: **Stage15 — post-guard branch-flow diagnostics**.
- Current diagnostic implementation: [stage16-player-bit17-writer-trace](https://github.com/Hawky11/1/tree/stage16-player-bit17-writer-trace).
- Stage16 HEAD (source-only; awaiting Windows build/runtime test): `2ca74c4b3e9a815c0a3d2eb14cb98bb071a85cef`.
- Stage15 evidence HEAD: `4d1e2a049c835a991647cb04565d31d330aeac95`.
- Stage16 traces write-only hardware watchpoints on the exact P1/P2 `[player+0x3B84]` DWORDs. No Stage16 Build.log or runtime trace has been received yet.
- Historical Stage12–Stage15 branches are evidence snapshots. Do not rewrite them.

## Repository workflow

- Treat stage branches as immutable historical snapshots after a test run is completed.
- Create each diagnostic stage as a new branch from the immediately preceding completed stage. Stage16 was branched from the exact Stage15 HEAD above; do not rewrite Stages12–15.
- Before analysis, check the target branch HEAD and compare its files with the last reviewed commit. A newly uploaded archive or README alone is not a new runtime test.
- A completed run should include `Build.log` and the relevant runtime logs. Confirm hook installation from the runtime log; do not infer it from source code alone.
- Compare new logs with the previous stage/run, correlate events by QPC/timestamp and arguments, and distinguish observed facts from hypotheses.
- After each stage, append its findings and commit/branch references to `PROJECT_HISTORY.md`, and update `NEXT_SESSION.md` with the latest checkpoint and next concrete task. Keep this root README current if the active stage changes.

The default `main` branch is the documentation index and aggregate log snapshot. Stage overlays and their exact test snapshots live on their named stage branches.
