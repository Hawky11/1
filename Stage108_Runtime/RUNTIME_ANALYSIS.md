# Stage108 — Stage107 r1 Windows runtime analysis: CEA20 POST verified, CEBE0 CEC64 not observed

**2026-11-12. WINDOWS BUILD SUCCEEDED; Stage107 INSTALLED 2/2. 13/13 Stage104 CEA20 PRE ↔ Stage107 POST confirmed same-run. NATIVE READ CEC64 = ZERO. No proof of downstream consumption/accepted action/attack/hit/AI improvement.**

## Immutable evidence provenance
- User's complete fresh upload: [commit db7a9967](https://github.com/Hawky11/1/commit/db7a9967a8f3ac6c85d0fc2072c96638a2a46e65) (ArcanaAI.ini, general ArcanaAI.log, Stage104 trace, Stage107 trace, Build.log).
- GitHub blob SHAs: `ArcanaAI.ini` 832d957303e29bb4f3d4759b45c1c8753a97afa4; general ArcanaAI.log 08a6a8874e263badd8301e801090056286e1ade1; Stage104 cb5555faa3a27e26101e6b50f90e951e5308e116; Stage107 7ba4d78fd08acb19677b1bda764d54dae759065c; Build.log 827fcaba0d6a9278985a76bd7e9b08a499250ef1.
- `Build.log`: Windows x86 MSVC 14.51.36231, **BUILD SUCCEEDED**, DLL **2,065,920 bytes**, only familiar nonfatal C4754 `AICommandSiteTrace.cpp:786`.
- `[AIResearch] EnableStage107ConsumerTrace=1`; `EnableStage36ControlToCommandTrace=0`. General ArcanaAI.log contains only COMvsCOM active line, not a broad gameplay stability report.
- Stage107 install `STAGE107_INSTALLED 2/2 post=0F7DA8 read=0CEC64 ... originalCmpOnce=1 DRwrites=0 noGameplayWrites=1`; Stage104 install `STAGE104_INSTALLED 2/2`; preexisting F2DE0 E9+NOP target VA `78E40E30` preserved (owner unknown).

## Full event accounting and strict correlation

**Stage104**: 43 HEALTH snapshots sum to mode0=29, cea20=13; 29 real MODE0 events include 13 special27 and 16 other codes. Each special27 MODE0 uniquely precedes one CEA20. **Stage107**: 43 HEALTH snapshots sum to post=13, nativeRead=0, matched=0, filtered=0, dropped=0, mismatch=0, readFail=0. Stage107 13 POST_CEA20 rows all `valid=1`, no READ_CEC64 rows.

All **13/13 Stage104 CEA20 PRE** map uniquely to **13/13 Stage107 POST** in the same session with identical P1/P2, player pointer, thread **11052**, calling stack EBP **0E58FAB4**, ESI `00000027`, CEA20 `ecx` = Stage107 `state`, increasing QPC, no reused candidate or ambiguity. P1=5/P2=8. PRE→POST QPC lag **min4, median5, max58** raw ticks. No cross-run joins.

Runtime PRE args: 12 records `arg0=player+0x930`; 1 record **P1** `arg0=player+0x954` (Stage104 seq42, Stage107 POST seq13). This is **different** from Stage105 earlier-run **P2** class5 +0x954 and must not be attributed to any class5 producer without same-session Stage96/98 evidence.

All POST rows: `state(P1)=0403C138`, `state(P2)=040641A4`, index=0, `v10=v18`, `v20=0000000A`, `readMask=07`, `valid=1`. POST values `v10/v18` are 0 for two initial captures and nonzero afterward; no direct proof of ownership of each change or eventual action. Stage107 POST `record=00000000` is a **nonpopulated field in the POST Event struct** (new `Event e{}` and no `e.record=` assignment), NOT a null CEA20 argument. The actual Stage104 CEA20 PRE `arg0` is always nonzero.

## Meaning of nativeRead=0

Verified Stage107 source from the actual Stage107 source ZIP: `Stage107ConsumerReadTrace.inl` `ObserveRead()` begins with `InterlockedIncrement(&g_nativeReads)` **before** index filter, recent-POST lookup or 10-second window. Since all 43 HEALTH snapshots report zero nativeRead and filtered=0, **no CEC64 observer callback occurred in this captured run**. This supports absence of execution *at CEC64 in the captured interval* under installed hook/logging, **not** absence of CEBE0 entry, all possible downstream reads, dispatch, animations/hits or any gameplay change. A CEBE0 call may branch around CEC64; alternative reader paths must be audited statically before introducing new hooks.

## Limits, safety and next recommendation
- No crash is reported by uploaded logs, but general ArcanaAI.log has one COM-active line; cannot infer game-wide stability.
- No Stage108 code/DLL, hook or game/DR writes created by analyst; current Windows-tested Stage107 DLL is 2,065,920 B. Keep Stage104 r2 prior tested DLL 2,058,240 B, Stage98 r2 2,051,584 B, Stage97 2,045,440 B as rollback references. Stage36 OFF.
- **Stage109 candidate:** first static audit CEBE0 entry/branch predicates from `CEBE0` to `CEC64` and known callers (988EA/98948/CAD79/CADDA), check whether this reader is reachable during the observed mode0 path. Only then consider a minimal passive CEBE0-entry or earlier-branch observer with exact ownership, guarded patch + verified rollback. No forced AI status/commands, DR changes or gameplay data writes.
