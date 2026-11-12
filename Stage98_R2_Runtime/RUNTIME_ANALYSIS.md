# Stage98 r2 — Windows live verification: TID fixed, native F7A00 route decisions (2026-11-12)

## Run provenance and boundaries

**User fresh same-session upload:** [main commit `8919f8e`](https://github.com/Hawky11/1/commit/8919f8ec242f1137c447d2eb4afc7ba0ed0c0e1e). `Build.log`, `ArcanaAI_Stage98CommandRouteTrace.log`, `ArcanaAI_Stage97InheritedEsiTrace.log`, `ArcanaAI_Stage96EsiWriterTrace.log`, `ArcanaAI_Stage95AlternateEsiTrace.log`, `ArcanaAI_Stage94ProducerGateTrace.log`, and `ArcanaAI_Stage93FinalCommandQpcTrace.log` were all refreshed for this new run (TID 11260 and coherent QPCs). Do **not** join Stage98 r1 or older Stage97/96 logs to this session.

[**Stage98 r2 source-only TID hotfix**](https://github.com/Hawky11/1/tree/stage98-r2-f7a00-tid-logger-fix-source-only/Stage98_R2_TID_Fix) only reactivates `e.tid=GetCurrentThreadId();` after a comment swallowed the assignment in Stage98 r1; Stage98 F7A00 native hook and original MOV/Jcc route code were not intentionally changed. The original Stage98 source-only zip still logs `STAGE98_BUILD=...-r1` at runtime, which is a **static marker**: do not mistake it for evidence the logger fix failed.

## Windows x86 build and live installation

- `Build.log` started **2026-11-12 14:36:39 local**. MSVC x86 **BUILD SUCCEEDED**, `dinput8.dll` **2,051,584 bytes**, same size as Stage98 r1. Only existing nonfatal warning C4754 in `src/AICommandSiteTrace.cpp:786`.
- `STAGE98_INSTALLED 1/1 rva=0F7A00 resume=0F7A06 Stage97Owner=1 originalOnce=1 nativeBranch=1 gameWritesExtra=0 DRwrites=0`.
- Existing hooks coexist: Stage97 **1/1**, Stage96 **5/5**, Stage95 **2/2**, Stage94 **2/2**; Stage94's Stage82 call redirect reports exact ownership.
- **49** HEALTH snapshots for **each** Stage94/95/96/97/98, all with zero `filtered`, `dropped`, `mismatch`, `readFail`. Every Stage98 ROUTE snapshot `valid=1`. These are observer health counters, **not** proof of AI improvement or comprehensive process safety.

## Fixed TID — no longer requires workaround

**14,107/14,107** Stage98 ROUTE records carry real `tid=11260`. All same-run Stage94, Stage95, Stage96 and Stage97 native events have exactly this TID. The Stage98 r1 `tid=0` bug is resolved in the Windows runtime. Snapshot context is now strictly `player+tid+frame`, with the same session's QPC.

## Strict native route join

Join each Stage98 `ROUTE` to the first later Stage94 `PRODUCER` or Stage95 `BYPASS` in exact (player,TID,frame) context, no later than the next same-context Stage98 snapshot and no more than 10,000 raw QPC ticks. Source events are in QPC order. **14,107/14,107** unique route outcomes; **0 missing, 0 duplicate assignments, 0 deterministic prediction mismatches**. Observed lag **1–322 raw QPC ticks**. All observed native outcomes come from **1,633 F3930 producers + 12,474 F7A94 bypasses**, exactly one downstream route per Stage98 snapshot in this run.

| Stage98 class | Earliest deterministic prediction | Observed outcome | Total | Result |
|---|---|---|---:|---|
| **1** | Early bit8/bit18 route to producer F7A99→F3930 | PRODUCER | **1,633** | **1,633/1,633 predicted correctly** |
| **3** | Earlier branches false; bit15 clear→bypass | BYPASS | **12,013** | **12,013/12,013 predicted correctly** |
| **5** | Dynamic later conditions, including mode/helper; NO early prediction | BYPASS | **461** | Observed path, causal predicate **unresolved** |
| 2 / 4 | Early bypass alternatives | Not seen | **0** | No evidence for this run |
| **Total** | | | **14,107** | **13,646/13,646 deterministic predictions correct** |

Class5 sample, still an open question: **461 events** (P1=139, P2=322) with mode `0000` **208** and mode `0001` **253**. Frequencies of logged `state60`: `0000001A` 124; `00000302` 108; `00000303` 83; `00000012` 64; `00000304` 62; `0000001B` 14; `00000131` 6. All class5 outcomes in this run were bypasses. Stage98's pre-branch snapshot does not witness the later helper decision at `525CD0`; **do not claim those status values or mode alone cause bypass**.

## Command provenance and final command consistency — fresh same session

- Stage94 `GATE`: **201** (167 forward, 34 reject).
- Stage94 `PRODUCER`: 1,633 native calls, of which **151 finite**.
- Stage95 `BYPASS`: 12,474, of which **50 finite**.
- All **201/201** gates have unique, value-supported, disjoint immediate source: **151 finite F3930 producer +50 finite F7A94 bypass**.
- All **50/50** finite bypasses uniquely match Stage94 gate, lag **2–51 QPC ticks**.
- Same-session Stage93 FinalCommand QPC log has **167** type1 events, all **167/167** match forwarded Stage94 gates by player/TID, actual ESI/payload arg/type1 and subsequent QPC, lag **1–85 QPC ticks**. No ambiguous or missing final records.

This is a good cross-stage consistency result. It does not establish improved AI, causality of hit/attack outcome, or a safe mechanism to force any flag or command.

## Checkpoint and next research

**Stage98 r2 is verified Windows-built and live-installed.** Preserve the working Stage98 r2 DLL **2,051,584 B** and Stage97 r1 fallback **2,045,440 B**. Keep Stage36 OFF and no added game/DR writes.

**Stage99 candidate:** analyze the exact native conditions beyond F7A49/F7A5E, especially `mode3A34`, status/flags and helper `525CD0` on class5. Start with static disassembly / existing logs to identify a uniquely guarded, low-noise observation site; do not instrument broadly or infer helper return values from the Stage98 class5 bypass outcome. Any new source-only intervention needs exact signature owner checks, rollback, and offline tests; Windows build/runtime would still be pending.

