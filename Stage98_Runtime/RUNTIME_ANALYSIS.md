# Stage98 r1 — live native route-predicate analysis + Stage98 r2 TID logger correction (2026-11-12)

## Provenance and testing boundaries

Source-only Stage98 r1 branch: [F7A00 original overlay](https://github.com/Hawky11/1/tree/stage98-f7a00-command-route-source-only/Stage98_Overlay). Original static AALib.exe SHA256 pinned and reverified: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. User's **fresh Stage98 r1 Windows runtime logs**: [upload aace9cd](https://github.com/Hawky11/1/commit/aace9cd5b4eb001ddddfdb338fd8416471e3b0a9) (`Build.log` and complete same-run Stage98/97/96/95/94 logs). This run is distinct from earlier Stage97 and Stage96 sessions; never join QPC across different runs.

`Build.log` started **2026-11-12 14:02:47 local**, **BUILD SUCCEEDED**, MSVC x86, `build/dinput8.dll` size **2,051,584 bytes**. Existing nonfatal C4754 warning in `src/AICommandSiteTrace.cpp:786`.

Live exact installation:
- `STAGE98_INSTALLED 1/1 rva=0F7A00 resume=0F7A06 Stage97Owner=1 originalOnce=1 nativeBranch=1 gameWritesExtra=0 DRwrites=0`.
- Stage97 1/1 (F77D4); Stage96 5/5; Stage95 2/2; Stage94 2/2 (Stage82 original call chain compatibility verified).
- **49 HEALTH snapshots for each Stage94/95/96/97/98**, all report zero `filtered/dropped/mismatch/readFail`. All logged Stage98 snapshots report `valid=1`. This validates observer-reported counters, not independent proof of game memory safety or improved combat AI.
- Existing `ArcanaAI_Stage93FinalCommandQpcTrace.log` on main retains old SHA `27f2f4d9ca080216a4e846d39d5b8eb2ceb777cb` from the preceding Stage97 run: it is **NOT** a same-session Stage98 final log. **Do NOT** join this to Stage98 on QPC.

## Complete same-session event census

| Log event | Count |
|---|---:|
| Stage98 `STAGE98_ROUTE` (F7A00) | **13,926** |
| Stage97 `STAGE97_INHERITED` | **13,926** |
| Stage96 `STAGE96_WRITER` | **17,946** |
| Stage95 `STAGE95_BYPASS` | **12,032** |
| Stage94 `STAGE94_PRODUCER` (F3930 native entry route) | **1,894** |
| Stage94 `STAGE94_GATE` | **224** |

Stage98 snapshots split exactly into 13,926 outcomes: **1,894 PROCUCER events** and **12,032 BYPASS events**. These counts are entry/path observations, not proof of 13,926 actual completed attack commands: the majority of F3930/bypass events carry ESI = FFFFFFFF, and only 224 gate events were logged.

## Critical Stage98 r1 logging defect — why the stock correlator falsely fails

In Stage98 r1's `src/Stage98CommandRouteDecision.inl`, the intended statement was concatenated with a C++ comment on the same line:

```cpp
e.preEax=saved[7]; // pushad saved EAX is slot 7; slot 3 is ESPe.tid=GetCurrentThreadId();
```

Thus **all 13,926 Stage98 ROUTE rows have `tid=0`**, whereas Stage94/95/96/97 all record the **single actual TID `11956`** for the entire current native sample. All player IDs, frame pointers and QPC fields remain coherent. The strict original `tools/correlate_stage98.py` requiring exact `player/TID/frame` would incorrectly report outcomes missing; **its `tid=0` input is invalid**. Do not ignore this flaw or claim that the r1 strict tool passed.

For this one session, the Stage94/95/96/97 traces all show one unique TID and match the P1/P2 `player,frame` keys. Recomputed a temporary offline join **only** across these single-threaded logs: for each Stage98 route in actor/frame QPC order, select the first later Stage94 PRODUCER or Stage95 BYPASS event within 10,000 QPC ticks, before the next same-actor/frame Stage98 route. This yielded unique, complete 1:1 outcomes with lag **1–538 raw QPC ticks**, no duplicated assignments, no missing rows and no conflicting class-1/class-3 predictions. Do not generalize TID omission to any multithreaded run.

| Stage98 `branchClass` | Native interpretation | Observed next route | Count | Prediction |
|---|---|---|---:|---|
| 1 | Early bit8 / bit18 condition, predicted producer F7A99/F3930 | PRODUCER | **1,894** | **1,894/1,894 correct** |
| 3 | Early bit15 clear, predicted bypass F7A94 | BYPASS | **11,335** | **11,335/11,335 correct** |
| 5 | Later mode/helper (`525CD0`) conditions unresolved | BYPASS | **697** | **No early prediction** |
| **Total** | | | **13,926** | **13,229/13,229 predicted routes correct** |

Classes 2 and 4 were **not observed** in this particular session. All 697 class5 events reached BYPASS; which individual later predicate selected it is NOT directly observed. Therefore no 697/697 prediction accuracy claim for class5.

## Stage98 r2 SOURCE-ONLY logger hotfix (Windows test pending)

[**Stage98 r2 fix source, updater, and rollback**](https://github.com/Hawky11/1/tree/stage98-r2-f7a00-tid-logger-fix-source-only/Stage98_R2_TID_Fix) · [source commit](https://github.com/Hawky11/1/commit/1299c79324621ab1c286ce91e244ef36daa8c56f). A one-region source delta restores an executable `e.tid=GetCurrentThreadId();` assignment **after** the `//` comment; old file SHA256 `a2548073f73fd7967ecece70cb278b32c162c0dfd07d9f63be7f132f628d5ba1`, new SHA256 `a56fe5692b3284fae37822e6f08438e3a32a800accbad5ff4c72d36ea97a1591`.

The updater accepts only exact Stage98 r1 manifest/source and Stage81/97 hashes, makes its own r2 rollback snapshot, updates the installed SHA in the original Stage98 rollback manifest, **preserving `stage='Stage98 r1'` required by the old RollbackStage98.ps1**. Original detour F7A00, native MOV replay, branch conditions, Stage81 loader, Stage94–97 hooks, game/DR data writes are unchanged.

Offline checks: original Stage98 exact-PE/static suite **86/86 PASS** (original AALib.exe SHA rechecked), r2 exact-delta/preflight/rollback/synthetic-TID suite **72/72 PASS**. R2 **has NOT been built on Windows or runtime-validated**; last Windows-tested DLL remains Stage98 **2,051,584 bytes**. Keep Stage97 DLL **2,045,440 bytes** for rollback; Stage36 OFF.

**Next:** close game, preserve working Stage98 r1 DLL, run `UpdateStage98r2.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'` from the new ZIP, MSVC x86 Build.bat, short game test. Require Stage98 1/1, upstream installed, new Stage98 ROUTE `tid` real and matching Stage94/95/97, clean health; then run original strict `tools/correlate_stage98.py` on **fresh same-run** Stage98/94/95 logs. If r2 SHA guard fails, stop; use included `RollbackStage98r2.ps1` or restore tested DLL. Do not claim AI gameplay changes, or plan new native hook merely to repeat already-proven early branches. The class5 downstream decision remains a useful Stage99 research candidate, but warrants targeted static logic before installing anything else.

