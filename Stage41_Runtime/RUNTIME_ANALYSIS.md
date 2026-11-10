# Stage41 Windows x86 runtime — native player-object action request state confirmed

User upload [main commit c39c966](https://github.com/Hawky11/1/commit/c39c9663eeba7e0a91b24c43f83661a54bedf598), timestamp 2026-11-10 06:09:11 UTC. This evidence branch was created at the exact user-upload HEAD, before adding this report, so ALL original Build.log and runtime log blob content is preserved at the branch root unmodified. Stage41 source branch is stage41-direct-object-commit-window; older stage branches and evidence are untouched.

## Build and installation

Build.log started 10.11.2026 09:07:12 local (Windows x86 MSVC), ends BUILD SUCCEEDED, dinput8.dll size 805376 bytes. Only preexisting C4754 warning in AICommandSiteTrace.cpp, no C++ errors. Stage40 log starts STAGE40_ACTION_CONSUMER_BUILD=2026-11-10-r2, STAGE40_INSTALLED 3/3. New Stage41 log starts STAGE41_DIRECT_OBJECT_BUILD=2026-11-10-r1 and STAGE41_ENABLED piggyback=Stage40 3/3. No additional native patches were installed by Stage41; its snapshots reuse Stage40's validated hooks.

## Exact 306-record cross-stream analysis

- ArcanaAI_Stage40ActionConsumerTrace.log: exactly 306 consecutive seq=1..306 consumer windows, all valid P+3F4C before and FFFFFFFF after, full preMask=FF and postMask=FF, all selectorSeen=1/selector=FFFFFFFF/actorSeen=0. No explicit Stage40 errors/dropped/overflow/mismatch in retrieved log.
- ArcanaAI_Stage41ObjectCommitTrace.log: exactly 306 consecutive STAGE41_OBJECT_WINDOW records stage40Seq=1..306; every one matches its Stage40 row on side, player pointer, thread, raw request, player mode, both QPC boundaries and native player object pointer, with no gaps/mismatched pairs. All 306 have preMask=postMask=1F (all five DWORDs readable) and objectBefore==objectAfter.
- Side distribution: P1 133 observations, P2 173, with 98 unique raw IDs total. Stage41 object P+4 remained consistent for all.
- In 257 cases, branch predicate (bit24Before!=0 OR object+60 pre != raw request) predicts the native 1E9F00 **new/requeue** path. All 257 satisfy object+64 POST==raw request, object+08 POST bit0==1, and object+30 POST low byte==0. Distribution new/requeue P1 133, P2 124. 254/257 have an observable actual change in object+64; 3/257 already held the requested ID. 255/257 have an observed bit0 0->1 transition; in two it was already 1. These are runtime confirmations of native command state, not a proof of visual attack or successful hit.
- In 49 cases (all P2), predicate (bit24Before==0 AND object+60 pre==raw request) predicts native 1E9F00 **duplicate/current** shortcut. In all 49, object+68 POST==FFFFFFFF as expected. Important limitation: object+68 PRE was ALREADY FFFFFFFF in those windows, so there is no observed new write to object+68, only strong branch-condition consistency with static disassembly. All 49 had object+64 PRE==POST equal raw request. Do not claim observing a change in this field for duplicates.
- Across all 306 observations, object+08 DWORD changed in 255, object+30 DWORD changed in 107, object+60 (current action) changed in ZERO, object+64 (pending action) changed in 254, and object+68 DWORD changed in ZERO. Thus Stage41 proves commands are QUEUED into object+64 and the pending bit; it does NOT by itself prove the transition into the current action object+60.

The exact machine-readable row-level classification with QPC, side, masks and before/after fields is [stage41_object_commit_classified.csv](stage41_object_commit_classified.csv).

## Other diagnostic streams

Stage39ActionOutputTrace: 50 windows, all 50 match a subsequent same-side Stage40 window with exactly the same ID before any next Stage39 request for that side. No unobserved tail in this run. Stage37Type1CommitTrace: 55 effects, all 55 exactly join EventDispatch type1 rows; EventDispatch has 155 detail rows total. Stage36 r2 has 882 sequential candidate results, 13 with nativeEAX not FFFFFFFF; no explicit warnings/errors in this file. Build and these smaller files were retrieved completely. Huge Stage38CallbackWindowTrace and Stage35ControllerConsumerTrace were not fully retrievable via the current GitHub textual connector; their original uploaded blobs remain untouched in evidence branch. Do NOT claim integrity counters of unread large files.

## Next exact native activation boundary

Original SHA-pinned AALib.exe is already present in ChatGPT Library, SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba; original bytes and CALL checked locally with objdump and PE mapping.

- Native function 1E9F40 (no args; ECX actual object) tests byte [ECX+08]&1 at RVA 1E9F5C. If set it calls 1E9C70 at RVA **1E9F62**, original bytes E8 09 FD FF FF, then returns 1; otherwise it returns 0.
- Callee 1E9C70 starts with 56 8B F1 83 66 08 FE (clears object+08 bit0 at 1E9C73). Later at 1E9E0B it loads object+64 into ECX and at 1E9E10 writes it to object+60 (current/active ID). It also clears intermediate state, calls further native state handling. It runs native game behavior, not an injectable operation.
- Original per-frame function AD710 iterates players via AD72F->E8480 (Stage40) and then linked-list objects via AD75C/AD778/AD797/AD7BC -> 1E9F40. These four CALLs also verified against original AALib.exe. These are the probable next activation sites, but activation of P1/P2 has not been checked in runtime.

**Stage42 source-only proposal:** hook ONLY the original 1E9F62 CALL->1E9C70, replay original native thiscall once, snapshot O+08/+30/+60/+64/+68 before/after, and filter to exact objects [P1+4]/[P2+4]. This precisely tests when pending ID becomes current active ID. It must have strict original CALL/prologue signature checks, preserve FPU/GPR/flags and support ring worker health counters. Stage42 must not inject ID/control or modify native state beyond the CALL-site observation patch. Do not infer animation, damage, combat effectiveness or improved AI decisions merely from object state. Windows x86 Stage42 build/runtime is not yet verified.
