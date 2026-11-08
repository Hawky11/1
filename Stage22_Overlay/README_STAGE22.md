# Stage22 — payload 0x1E post-AND trace

Stage22 is a narrow follow-up to Stage21. It is based on the corrected Stage21 r2 source, preserving the valid pre-call EDI capture and adding one observer at the next instruction in the real payload-0x1E fallback path.

## What this stage answers

Stage21 confirmed the fallback helper at CALL RVA \`0x000EE446\`, but its controller flag snapshot was taken before the next instruction. Stage22 measures \`[controller+8]\` immediately before and after the actual bit-\`0x400\` AND at RVA \`0x000EE451\`. It can distinguish:
- a true \`1 -> 0\` clear;
- an executed but idempotent \`0 -> 0\` AND;
- an unreadable post-state.

## Files

- \`src/AIQueueTrace.cpp\`: complete replacement overlay derived from Stage21 r2. Keeps the r2 classifier wrapper and Stage21 slot-3 pre/post trace; adds a validated post-AND wrapper at \`0x000EE451\`.
- \`ArcanaAI.ini\`: enables both Stage21 consumer trace and Stage22 post-AND trace.
- \`STATIC_POST_AND_ANALYSIS.md\`: exact instruction, patch contract, runtime fields and interpretation.

New runtime file: \`ArcanaAI_Stage22PayloadPostAndTrace.log\`.

## Apply

1. Start from the same assembled Stage20/Stage21 project used for the last Stage21 r1 run. Keep all inherited source files and project settings.
2. Replace the assembled project's \`src/AIQueueTrace.cpp\` with \`Stage22_Overlay/src/AIQueueTrace.cpp\`.
3. Replace the assembled \`ArcanaAI.ini\` with \`Stage22_Overlay/ArcanaAI.ini\`.
4. Build \`Release | Win32\` and preserve the complete build transcript and resulting DLL size.
5. Start a natural COM-vs-COM session. Do not edit the controller flag or slot value in Cheat Engine and do not force the helper result.
6. Confirm the Stage21 r2 marker/wrappers and Stage22 post-AND installation marker before interpreting any event data.

## Required uploads

- \`Build.log\`
- \`ArcanaAI_Stage22PayloadPostAndTrace.log\`
- \`ArcanaAI_Stage21PayloadConsumerTrace.log\`
- \`ArcanaAI_EventDispatchTrace.log\`
- \`ArcanaAI_QueueTrace.log\`
- Same-run Stage20 object-guard, Stage17 setter, Stage16 writer, Stage18 gate and Stage19 state-map logs when available.

A \`STAGE22_POST_AND\` record with \`bit400Before=1 bit400After=0\` confirms the actual flag transition. A \`0 -> 0\` record confirms execution only. Stage21's slot-3 before/after fields continue to decide whether that separate DWORD changed.

## Safety / scope

Stage22 does not write a forced value. The wrapper replaces only the validated original AND instruction and executes that exact AND once, then restores the original post-AND EFLAGS and general registers. Instrumentation adds timing overhead. If the PE fingerprint or byte signature differs, the hook refuses installation. This overlay is source-only and has not yet been compiled or runtime-tested.

Preserve Stage21 r1 runtime evidence and Stage21 r1/r2 source branches unchanged.
