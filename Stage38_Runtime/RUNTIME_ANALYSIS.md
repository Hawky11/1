# Stage38 — Windows x86 runtime evidence review

Date: 2026-11-10. Upload: [def14ec](https://github.com/Hawky11/1/commit/def14ecc8a3354b96a2564d0280a088684a68ae6), Stage38 source: [4963dcd](https://github.com/Hawky11/1/commit/4963dcdccfaaa190d5ce2438510ff24c0ba2a51d). This branch archives the exact uploaded Git blobs without re-encoding. Original AALib.exe SHA256 remains `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. No need to re-request the EXE.

## Build and installation

`Build.log`: MSVC x86 2026-11-10 07:28:56; `BUILD SUCCEEDED`, `dinput8.dll` 278,528 bytes. No compile/link error; only historical C4754 at `AICommandSiteTrace.cpp` line 785. Stage38 build marker `STAGE38_CALLBACK_WINDOW_BUILD=2026-11-10-r1`, `STAGE38_INSTALLED 2/2` (EE7AE->156C00 AFTER, EE7C7->FC60 BEFORE), original guarded EE7BE `CALL EDI` unchanged. Stage36 r2 own-Stage10 detour recognition and 3/3 installation, Stage37 enabled, EventDispatch EE200 installed.

## Stage38 full-log validation

Read the complete 4,639,945-byte `ArcanaAI_Stage38CallbackWindowTrace.log` (newline terminated), 4,916 records, sequential seq=1..4916, no gaps, bad 30-field rows, mask inconsistencies, QPC inversions, invalid side/pointer pairs, or callback!=callbackAfter. All `preMask`/`postMask=0x3FFFFFFF`; all 30 raw f0..f29 before/after pairs present; recomputed `changedMask` matches 4,916/4,916. No explicit ERROR/WARNING/dropped/overflow/pairMismatch messages. Per-window qpcAfter-qpcBefore min=1, median=3, max=587 ticks (not a CPU benchmark).

P1 windows=2,369; P2=2,547. `guardedCall=1` for 4,195 (P1 2,119, P2 2,076), consistent with nonzero saved EDI and successful paired return. `guardedCall=0` for 721 (P1 250, P2 471), with zero target; **not executed callbacks**. 367/4916 windows have a nonzero net change among the selected 30 DWORD fields, 4,549 have zero net change. These measurements do not prove that an unchanged callback had no native side effects. Field-change counts: f3 (P+3ED0) 289, f4 (P+3ED4) 200, f24 (O+80) 53; f28 (O+278 generation) **0** within these isolated callback windows. Some callbacks replace the *stored next callback* while actual EDI remains the same before/after. For grouped native `P+3A34` WORDs, f1 low16 is type and high16 is payload. Windows by type: type0=1,628; type1=1,838; type2=1,450. A window is not a new command.

## Stage37 -> Stage38 correlation

In the same run, 176 contiguous `EVENT_DISPATCH_DETAIL` records (120 type0, 56 type1). All 56 `STAGE37_TYPE1_EFFECT` records exactly join distinct type1 EventDispatch records by dispatchSeq, side, player, TID, payload, qpcBefore and qpcAfter. All 56 Stage37 read masks full.

For all 56 Stage37 type1 events, the next Stage38 window on the same side matches object pointer `Stage37.fields[18].post == Stage38.f0.pre` and exact f1 type1/payload. **55/56** also match lifecycle generation `Stage37.fields[25].post == Stage38.f28.pre` and occur before the next same-side dispatch. Their per-window pre-start is 81..189 QPC ticks after the corresponding Stage37 post-end in the matched subset (separate exact per-record timestamps available in original logs). All 55 callbacks have nonzero EDI (`guardedCall=1`).

Exception: Stage37 `dispatchSeq=112`, P2, payload 0x19, qpcAfter=819293050041, object generation 0x97. No Stage38 callback before another P2 **type1 payload 0x19** dispatch (`dispatchSeq=113`, qpcBefore=819293051457, qpcAfter=819293052129), which increments object generation to 0x98. The next matching Stage38 is seq2169, qpcBefore=819293052224, generation=0x98 and real callback 0x0092EB10; it therefore belongs to the later same-payload dispatch by generation and temporal order. Do not assert the first duplicate dispatch was a missed action, or falsely attribute the later callback to both. There is no explicit overflow/pairing warning.

## Stage36 -> queue -> dispatch -> callback chain

Stage36 contains 367 contiguous `STAGE36_RESULT` CANDIDATE records (361 sentinel FFFFFFFF, six positives). All six positives uniquely align with subsequent same-player/TID/payload `QUEUE_EVENT_DETAIL callerRVA=F7C62`, type1 `EVENT_DISPATCH_DETAIL`, matching Stage37 state commit and matching nonzero Stage38 callback (in order):
- ID33 P1: Stage36 seq109 -> queue13 -> dispatch60 -> callback38 seq934 at 0x00923AF0;
- ID6 P2: seq150 -> queue24 -> dispatch93 -> callback seq1606 at 0x0092EB10;
- ID12 P2: seq159 -> queue25 -> dispatch94 -> callback seq1633 at 0x0092EB10;
- ID92 P2: seq169 -> queue26 -> dispatch95 -> callback seq1672 at 0x009DA490;
- ID10 P2: seq205 -> queue33 -> dispatch115 -> callback seq2366 at 0x0092EB10;
- ID37 P1: seq321 -> queue48 -> dispatch157 -> callback seq3359 at 0x00923AF0.
These establish a concrete native call path for six naturally selected command candidates. No INPUT_1E/INPUT_1F samples; do not claim their native gates are tested.

## Other observers / integrity

`Stage35ControllerConsumerTrace.log`: 10,592 contiguous full-mask (FFF) records, exact TIMER/FLAGS alternation for both players, 2,648 pairs per side, no observed mismatch or warning. `ArcanaAI_Stage36ControlToCommandTrace.log`: 367 contiguous records, no warning. `ArcanaAI_EventDispatchTrace.log`, Stage37, Queue, FinalCommand, Stage29 and Stage31: no explicit ERROR/WARNING/dropped/overflow in checked files. Full Stage38, Stage35, ProducerCall (9,001,773 bytes), StateMap (1,099,405 bytes) were read via original Git blobs and are nonempty/newline-terminated. ProducerCall contains 11,563 STAGE14_PROBE records; no explicit warning identified. Historical logger headers in unrelated modules are not themselves evidence of stale Stage38. Windows near exit may lack a completed pair; newline termination does not establish complete tail flush.

## Interpretation and next research

Runtime now demonstrates (1) native candidate selection, (2) type1 queueing/dispatch, (3) state commit, and (4) original guarded callback invocation with paired return and real target, for six positive candidates. **Not yet established:** animation request, action timing, active collision/hitbox, contact, damage, or AI quality improvement. Don't conflate repeated callback windows in same O+278 generation with new command generation. Stage39 should use the real callback targets (esp. RVA 12EB10, 123AF0, 1DA490) and original already-held SHA-verified AALib.exe for static reverse trace to the next verifiable action/animation/output handoff; prepare narrowly scoped passive instrumentation only if needed. Do not force AI inputs, payload, bit17, or gameplay flags. Keep Stage29 AITrace.cpp and Stage35/36 inl versions unchanged; preserve all historical branches. Update NEXT_SESSION.md and README as new source becomes ready.
