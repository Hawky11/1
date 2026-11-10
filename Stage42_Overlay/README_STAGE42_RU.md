# Stage42: read-only native activation consumer (source only, Windows x86 untested)

## Verified Stage41 context

Fresh [Stage41 upload c39c966](https://github.com/Hawky11/1/commit/c39c9663eeba7e0a91b24c43f83661a54bedf598): Windows x86 BUILD SUCCEEDED, DLL 805376 bytes; Stage40 installed 3/3 and Stage41 enabled. There are exactly 306 Stage41 rows, a 306/306 full-field 1:1 match to Stage40, masks 1F/1F, stable actual objects O=[P+4]. All 257 new/requeue requests left object+64 equal to raw request and object+08 bit0 set; 49 current/duplicate requests (P2 only) satisfied the original short-circuit predicate and had object+68 already FFFFFFFF before and after. O+60 did not change within any of the 306 E8480 calls; subsequent activation is still unobserved. Archived logs and analysis are in [stage41-runtime-object-commit-evidence](https://github.com/Hawky11/1/tree/stage41-runtime-object-commit-evidence).

## Native target (from SHA-verified AALib.exe)

Native 1E9F40 tests object+08 bit0 at RVA 1E9F5C; if set it CALLs native 1E9C70 from callsite **1E9F62**, bytes E8 09 FD FF FF. Native 1E9C70 begins with 56 8B F1 83 66 08 FE and clears object+08 bit0 at 1E9C73. Later 1E9E0B reads pending ID O+64 and 1E9E10 writes the value to current ID O+60. Native AD710 invokes 1E9F40 on objects after processing player E8480 requests. A native O+60 update is action-state activation, NOT proof of successful animation, hit, damage or better AI decisions.

Stage42 adds **one passive CALL-site wrapper** at 1E9F62->1E9C70, calling the original native routine once, saving/restoring all GPR/flags and x87/SSE across bookkeeping, filtering exact P1/P2 objects only and taking five masked DWORD snapshots (O+08/+30/+60/+64/+68) before/after. No synthetic IDs, memory patches other than the diagnostic CALL redirect, changes to flags, or additional worker. The existing Stage38 worker flushes the Stage42 ring. Install checks original CALL and native prologue bytes; failed install is fail-closed and Stage40/41 continue.

## Installation: exactly TWO files

In D:\INCUBATOR\ArcanaAIOverhaul\src:

1. Replace [Stage38CallbackWindow.inl](src/Stage38CallbackWindow.inl) from Stage42_Overlay.
2. Add [Stage42ActivationHook.inl](src/Stage42ActivationHook.inl) from Stage42_Overlay.

Keep the last working Stage41 src\Stage40RequestConsumer.inl, src\Stage39ActionOutput.inl, src\AIQueueTrace.cpp, Stage35/36 inls, Stage29 AITrace.cpp, ArcanaAI.ini, Build.bat and other sources UNCHANGED. Archive old logs. Run original x86 Build.bat and require BUILD SUCCEEDED. Install freshly built dinput8.dll only on successful build. Run natural COM-vs-COM (no forced IDs/flags/inputs). Allow at least two seconds for worker flush after the match. Upload fresh Build.log, ArcanaAI_Stage42ActionActivationTrace.log and Stage41/40/39/38/37/36, EventDispatch, QueueTrace, ParentFlow, FinalCommand, Stage35 logs to main.

## Expected new trace and checks

New log ArcanaAI_Stage42ActionActivationTrace.log begins STAGE42_ACTION_ACTIVATION_BUILD=2026-11-10-r1 and STAGE42_INSTALLED 1/1 callsite=1E9F62 native=1E9C70. Every STAGE42_ACTIVATION_WINDOW includes side, P1/P2, object, thread, QPC before/after, five fields (o0 to o4), preMask/postMask, changedMask, bit0 before/after. Valid full-mask case: preMask=1F postMask=1F. Expected native 1E9C70 operation: object+08 bit0 1>0 and object+60 post should equal object+64 pre. Some other object fields may change because the native function performs additional work.

STAGE42_HEALTH exposes native calls, filtered other objects, dropped, overflow, and mismatch. Check dropped=overflow=mismatch=0 and monotonic QPC/sequence. The number of activations NEED NOT equal the number of newly enqueued Stage41 requests; duplicates, frames, ordering and other native callsites can change counts. If no P1/P2 activation windows, check health and object mapping before claiming the game dropped actions.

Stage42 source and original EXE target signatures were checked statically, but Windows x86 Stage42 compilation and runtime are STILL PENDING. Original SHA-pinned EXE exists in ChatGPT Library and need not be re-uploaded. Historical stage branches must remain intact.
