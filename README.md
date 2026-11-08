# ArcanaAI / COM vs COM diagnostic research

This repository tracks the staged investigation of the ArcanaAI COM-vs-COM event/queue path.

## Start here

1. Read **[NEXT_SESSION.md](NEXT_SESSION.md)** first. It is the current handoff for the next AI/chat and contains the exact next task.
2. Read **[PROJECT_HISTORY.md](PROJECT_HISTORY.md)** for the append-only stage history and the evidence behind the current conclusions.
3. Open the referenced stage branch and inspect its README, overlay, and runtime logs before making changes.

## Current checkpoint

- Latest runtime-evidenced stage: **Stage16 — bit17 setter and clearer confirmed**.
- Latest runtime evidence branch: [stage16-player-bit17-writer-trace](https://github.com/Hawky11/1/tree/stage16-player-bit17-writer-trace), HEAD `58313d8c0f6071bc8099fc5ec04ae04d2974dafd`.
- Current source/diagnostic implementation: [stage17-bit113-setter-caller-trace](https://github.com/Hawky11/1/tree/stage17-bit113-setter-caller-trace), HEAD `46a84d2286e94ba66fd46fb11454d1189b36b526`.
- Stage16 confirmed five bit17 0→1 transitions through the generic setter at RVA `0xFAD0`, and five 1→0 clears at RVA `0xED19F`. Five of six sampled type-1/`0x1E` events were stopped at F6A68 while bit17=1; the one event with bit17=0 reached the alternate queue and EventDispatch.
- Stage17 adds a filtered entry observer at setter RVA `0xFAA0` for bit index `0x71`, with caller return-address and CALL-target decoding. It is source-only and awaits MSVC x86 build/runtime validation.
- Stage16 `Build.log` was not uploaded, so its compiler transcript remains unavailable even though runtime logs confirmed the module and watchpoints were active.
- Historical Stage12–Stage16 branches are evidence snapshots. Do not rewrite them.

## Repository workflow

- Treat stage branches as immutable historical snapshots after a test run is completed.
- Create each diagnostic stage as a new branch from the immediately preceding completed stage. Stage16 was branched from the exact Stage15 HEAD above; do not rewrite Stages12–15.
- Before analysis, check the target branch HEAD and compare its files with the last reviewed commit. A newly uploaded archive or README alone is not a new runtime test.
- A completed run should include `Build.log` and the relevant runtime logs. Confirm hook installation from the runtime log; do not infer it from source code alone.
- Compare new logs with the previous stage/run, correlate events by QPC/timestamp and arguments, and distinguish observed facts from hypotheses.
- After each stage, append its findings and commit/branch references to `PROJECT_HISTORY.md`, and update `NEXT_SESSION.md` with the latest checkpoint and next concrete task. Keep this root README current if the active stage changes.

The default `main` branch is the documentation index and aggregate log snapshot. Stage overlays and their exact test snapshots live on their named stage branches.
