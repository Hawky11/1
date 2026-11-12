# Stage96 r1 — verified live runtime analysis (2026-11-12)

## Provenance and boundaries

Runtime upload to `main`: [commit 7922935697265f370148fa3fadf06b710e88e395](https://github.com/Hawky11/1/commit/7922935697265f370148fa3fadf06b710e88e395), 2026-11-12 07:29:37 UTC. Same-run root logs: `Build.log`, `ArcanaAI_Stage96EsiWriterTrace.log`, `ArcanaAI_Stage95AlternateEsiTrace.log`, `ArcanaAI_Stage94ProducerGateTrace.log`, `ArcanaAI_Stage93FinalCommandQpcTrace.log` (and `ArcanaAI_Stage93DirectBit3Trace.log`). This is a NEW run, not the preceding Stage95 run. The Stage96 source-only package was delivered in chat as `ArcanaAI_Stage96_PreBypass_EsiWriter_SourceOnly.zip`; the runtime evidence proves MSVC x86 compilation and hook installation, not improvement in game AI.

`Build.log`: **BUILD SUCCEEDED**, `build/dinput8.dll` **2,039,808 bytes**. One pre-existing nonfatal C4754 warning in `AICommandSiteTrace.cpp`.

Exact installation signatures, same runtime:
- `STAGE94_INSTALLED 2/2` and verified Stage82-backed F7AD1 call chain (`viaStage82=1`).
- `STAGE95_INSTALLED 2/2` (F7A94 bypass, F7AEE fallback).
- `STAGE96_INSTALLED 5/5 rvas=0F7924,0F79B6,0F79D4,0F79F0,0F7A8F originalOnce=1 Stage95Owner=1 nativeJNE=1 gameWritesExtra=0 DRwrites=0`.
- Stage94, 95, and 96: **51 HEALTH records each**; all reported `filtered=0 dropped=0 mismatch=0 readFail=0`. This is reported observer health, not independent gameplay testing. Stage36 remains OFF; no additional game-data or debug-register writes reported.

## Complete event counts

| Source | Event kind | Count |
|---|---|---:|
| Stage96 | `STAGE96_WRITER` | **17,558** |
| Stage95 | `STAGE95_BYPASS` | **12,484** |
| Stage95 | `STAGE95_FALLBACK` | **0** |
| Stage94 | `STAGE94_PRODUCER` | **1,590** |
| Stage94 | `STAGE94_GATE` | **208** |
| Stage93 | `STAGE93_FINAL_QPC` | **163** |

Stage96 event distribution:

| Site | Total observed | Finite `resultEax` |
|---|---:|---:|
| F7924 | 11,804 | 141 |
| F79B6 | 1,693 | 17 |
| F79D4 | 1,270 | 33 |
| F79F0 | 2,286 | 39 |
| F7A8F | 505 | 4 |

All 17,558 Stage96 event rows reported `valid=1`, `localRead=1`, and `flagRead=1`. **Note**: `f79f0Skip=1` means the original *JNE after MOV ESI,EAX* branches to F7A00; the original MOV still executes. It is not a skipped ESI assignment.

## Cross-stage provenance — two routes, one open writer gap

- Of 208 Stage94 gates, **163 forward**, **45 reject**. Every one of the 163 forward gates matches the 163 Stage93 real `FINAL_QPC` commands in order by player, side, TID, ESI payload and argument. Gate→Final QPC lag **2–62 raw ticks**. No mismatches.
- **152/208** gates have a Stage94 F3930 producer whose native EAX equals gate ESI; Stage94 observed exactly 152 finite F3930 results.
- Stage95 shows exactly **56 non-FFFFFFFF bypass ESI** records. **56/56** have unique Stage94 gate partners, same player/TID/EBP/ESI and within **2–58 raw QPC ticks**; no ambiguous or missing Stage95→gate matches.
- These are two disjoint, value-supported route classes: **152 F3930 producer**, **56 F7A94 bypass** = **208/208 gates**.
- The Stage94 candidate heuristic labeled 153 gates with `sourceCandidateSeq!=0`, but **one of those is a false positive**: Stage94 gate seq **1487**, ESI **0x23**, had candidate producer seq **1486** returning `FFFFFFFF`, `sourceValueMatches=0`. Stage95 bypass seq **9707** and Stage96 F7924 writer (4 QPC ticks earlier) both show **0x23**. Do not count this gate as originating in F3930.
- Of the 56 bypass→gate events, **55/56** match the latest measured Stage96 ESI writer in the same player/TID/EBP context with exact value, **1–16 raw QPC ticks** before Stage95. Family breakdown of the 55 matched: **F79F0: 39** (36 forward, 3 reject); **F7924: 12** (12 forward); **F7A8F: 4** (3 forward, 1 reject). F79B6 and F79D4 were active but were not the immediately preceding measured writer for these 55 bypass→gate cases.
- **Unresolved 1/56:** Stage95 bypass seq **9461** → Stage94 gate seq **1484**, P1, ESI **0x2C**, forwarded. The most recent Stage96 record for the same player/TID/EBP was F79F0 with ESI **0x2B**, approximately **2,606,909 QPC ticks** earlier; no Stage96 record with `nativeNextEsi=0x2C` occurs in this upload. Thus the previous measured writer is stale and VALUE-DIFFERENT. This is genuine missing provenance, **not** 56/56 native writer attribution. Do not assume the origin of 0x2C without a new trace. Possible uninstrumented path or inherited ESI, still a hypothesis.

## Interpretation, next checkpoint

Stage96 r1 passed the Windows MSVC build and live hook/health checks. Complete immediate branch attribution exists for 208/208 gate events via value-supported Stage94 producer or Stage95 bypass, and 163/163 final commands match. The native ESI writer is directly matched for 55/56 bypass gates; **one bypass source remains unresolved**. The next task (Stage97 candidate) should reconstruct entry-to-F7A94 control flow and ESI live value for the `0x2C` exceptional path using original `AALib.exe` static disassembly and minimally scoped passive evidence. In particular inspect how F79F0 and other source sites can be bypassed and how ESI may enter the function. No unverified hook or write should be added to the game; keep Stage36 OFF.

Do not retroactively replace the 56/56 *bypass→gate* success with 56/56 *writer→bypass* success; those are distinct joins. This analysis records the exact runtime findings and exceptions for future sessions.
