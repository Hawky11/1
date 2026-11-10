# Stage40 — E8480 request consumer (SOURCE ONLY, runtime pending)

Stage39 completed and verified: see Stage39_Runtime/RUNTIME_ANALYSIS.md on stage39-native-action-runtime-evidence (ae6074d). It confirmed 51 callback windows where P+3F4C changes from FFFFFFFF to a native request ID. Stage40 observes its consumer and the actual object receiving the request. There is NO Stage40 Windows build or live runtime evidence yet.

## Installation in D:\INCUBATOR\ArcanaAIOverhaul

1. Replace src\Stage38CallbackWindow.inl with Stage40_Overlay/src/Stage38CallbackWindow.inl.
2. Add src\Stage40RequestConsumer.inl from Stage40_Overlay/src/Stage40RequestConsumer.inl.
3. If not already present, add src\Stage39ActionOutput.inl from Stage40_Overlay/src/Stage39ActionOutput.inl (unchanged Stage39 file).
4. Do not replace or modify the **working Stage38 src\AIQueueTrace.cpp**, Stage35ConsumerTrace.inl, Stage36ControlPredicate.inl r2, Stage29 AITrace.cpp, ArcanaAI.ini, or Build.bat. Existing EnableStage38CallbackWindowTrace=1 activates Stage40 alongside Stage38/39; there is no new INI key.

Archive the old logs. Run the existing **Windows x86 Build.bat**; require BUILD SUCCEEDED before installing build\dinput8.dll. Run the normal COM-vs-COM fight with no forced commands, inputs, action IDs or bitflags, and allow at least 2 seconds to flush before closing. Upload to main: fresh Build.log, ArcanaAI_Stage40ActionConsumerTrace.log, Stage39ActionOutputTrace, Stage38CallbackWindowTrace, Stage37Type1CommitTrace, EventDispatchTrace, Stage36ControlToCommandTrace, QueueTrace, ParentFlow, FinalCommandTrace, Stage35 logs.

## New passive hooks

Three exact original five-byte CALL sites in SHA-verified AALib.exe:

- AD72F -> E8480: per-player request reader, thiscall ECX=P. Fast path for request FFFFFFFF jumps directly to the original E8480 without C++ tracing. Non-sentinel path captures before/after.
- E84E3 -> 125920: reads the actual EAX selector result, including FFFFFFFF rejection. Does not alter native arguments.
- E8505 -> 1357F0: reads the actual EAX actor pointer and EDI selector BEFORE the native E850A and E8523 actor writes, then captures the same actor's fields after E8480 returns.

Every original callee is invoked exactly once; GPR, EFLAGS and FPU/SSE are preserved via PUSHFD/PUSHAD and FXSAVE/FXRSTOR. No game-state writes, synthesized inputs, or action injection. All three patches validate original bytes, rel32 destination and function prologues, recheck under installation mutex while threads are suspended, fail closed on mismatch. Stage38's existing hook and logging remain intact.

New runtime log: ArcanaAI_Stage40ActionConsumerTrace.log. Expected lines:

    STAGE40_ACTION_CONSUMER_BUILD=2026-11-10-r1
    STAGE40_INSTALLED 3/3 AD72F->E8480 E84E3->125920 E8505->1357F0 original calls once; passive.
    STAGE40_CONSUMER_WINDOW seq=... side=P... player=... tid=... qpcBefore=... qpcSelector=... qpcActor=... qpcAfter=... path=... selector=... actor=... p0=...>... a1=...>...

The record only appears for non-FFFFFFFF entry requests, including selector-rejected cases. Thus it does NOT count total idle calls to E8480.

Fields p0..p7 (each read before/after): P+3F4C request; P+4 object; P+3B88 flags; P+3B7C flags; P+43B4 candidate actor (NOT used as authoritative target); P+3978 control state; P+3F20 selector input; P+3B84 control flags. Their independent 8-bit masks: preMask/postMask.

Fields a0..a2 (before E850A / after E8480): actual actor returned by native 1357F0, offset +10 flags; +280 raw request ID; +2D0 original [P+4] pointer. Independent 3-bit actorPreMask/actorPostMask. Actor fields are not available on SELECTOR_SENTINEL and missing masks must not be interpreted as zeros.

selectorSeen and selector show original 125920 result; actorSeen/actor/selectorAtActor show actual 1357F0 return and corresponding EDI. selectorRequestRead/requestAtSelector and actorRequestRead/requestAtActor capture any intervening changes of P+3F4C. QPC times allow chronology checks. path separates SELECTOR_SENTINEL, ACTOR_RETURNED, NO_ACTOR and NO_SELECTOR.

## Acceptance criteria

- Genuine x86 BUILD SUCCEEDED and runtime STAGE38_INSTALLED 2/2, STAGE39_ENABLED and STAGE40_INSTALLED 3/3. Check all errors/warnings including dropped, pendingOverflow, mismatch.
- Stage40 sequence is contiguous, timestamp order valid, P1/P2 pointers match existing observers and requested ID p0.pre is non-sentinel. Check field counts and masks.
- When selector=FFFFFFFF, native takes E8535 flag-clear path and must not execute E8505. When selector is valid, look for actorSeen=1 and exact actor+280 after being equal to requestAtActor, also actor+2D0 after matching P+4 from the same instance (when those reads are valid). Check p0.post=FFFFFFFF and bit24 of P+3B7C cleared.
- Correlate Stage39 a0.post with the later Stage40 p0.pre on the SAME player and chronologically via QPC. Do not declare a unique join from action ID alone; repeated IDs and intervening writes may exist.
- Actor+280 is only a native action binding, not proof of animation, hitboxes, collision, damage or AI improvement. No new Stage40 x86 build/runtime has yet been performed.

Original EXE from Library AALib.zip already exists; no re-upload needed. SHA-256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba. Preserve all historical stage branches.
