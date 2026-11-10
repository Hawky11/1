# Stage42 passive native activation — static analysis and risks

State as of 2026-11-10. Original source Stage41 runtime P1/P2 exact 306 events verified; see Stage41_Runtime/RUNTIME_ANALYSIS.md at branch stage41-runtime-object-commit-evidence. This stage prepares a new optional observer, not a tested runtime result.

## Original EXE grounding

SHA-pinned AALib.exe in ChatGPT Library, 175556608 bytes, SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba. Original PE32 x86, ImageBase 00400000. The original disassembly and exact RVA bytes were checked against this EXE.

- 1E9F40 begins test of byte [ECX+30], conditionally moves O+68 to O+64 and ORs O+08 bit0. At 1E9F5C it tests byte [ECX+08],bit0 (F6 41 08 01). On true it CALLs 1E9C70 at 1E9F62, bytes E8 09 FD FF FF, then returns AL=1. On false it returns AL=0 without calling the consumer.
- 1E9C70 starts 56 8B F1 83 66 08 FE. At RVA 1E9C73 it clears [O+08] bit0. It then resets/derives several other object state fields, at RVA 1E9E0B reads O+64 (pending ID) and RVA 1E9E10 writes O+60 (current/active ID). The original function has no stack arguments, uses ECX=O and returns via RET, making it appropriate for a no-stack-args CALL wrapper.
- Four calls inside the per-frame AD710 sweep to 1E9F40 (AD75C, AD778, AD797, AD7BC) are confirmed in the original binary. The AD710 sweep processes E8480 via AD72F before iterating native objects. There are other callsites to 1E9F40 outside this frame sweep; only the **specific** 1E9F62->1E9C70 CALL is intercepted by Stage42, so it is not intended to enumerate every possible native O+60 setter.
- Stage41 runtime: for 306 E8480 windows O+60 did not change there, while 257 new/requeue requests yielded O+64=ID and bit0 set; 49 duplicates match the no-new-queue predicate. Stage42 measures the subsequent *promotion*, not the initial pending write.

## Stage42 overlay and safety model

- New Stage42_Overlay/src/Stage42ActivationHook.inl, included in Stage42_Overlay/src/Stage38CallbackWindow.inl after the proven Stage41 Stage40RequestConsumer.inl. Only these **two** files need local installation; Stage41 source remains unchanged.
- Stage42 wrapper installed at original 1E9F62 after checking exact 5 CALL bytes plus native 1E9C70 7-byte prologue and decoded CALL destination. Existing MakeRelativeCall, native suspended-thread patching mutex/VirtualProtect/FlushInstructionCache/restore protection are reused. Failure is fail-closed, without changing Stage40/41.
- Wrapper original callee invoked once with original ECX/stack. Bookkeeping before/after uses paired PUSHFD/PUSHAD and FXSAVE/FXRSTOR, preserving native GPR/flags/x87/SSE. P1/P2 exact O=[P+4] filter uses read-only SafeReadDword. Other object calls are only counted by health.
- Five fields O+08/O+30/O+60/O+64/O+68 captured before and after via Stage41's proven S41SnapObject, validity masks kept, QPC and thread IDs captured, per-thread TLS depth=8. Separate 2048-entry SRW ring flushed by existing Stage38 worker into ArcanaAI_Stage42ActionActivationTrace.log with STAGE42_ACTIVATION_WINDOW and STAGE42_HEALTH counters. No new thread, no INI, no game input or game-state writes other than the CALL-site diagnostic trampoline.
- Verified local source anchors: one new MakeRelativeCall, exactly one native replay CALL, two matched FXSAVE/FXRSTOR pairs, include/install/flush all integrated; brace counts balanced. Verified native byte and original call target with objdump, not Windows x86 build.

## What the next runtime can (and cannot) establish

Accepted results: BUILD SUCCEEDED; Stage40 INSTALLED 3/3 and Stage41 ENABLED; Stage42_ACTION_ACTIVATION_BUILD r1 and STAGE42_INSTALLED 1/1; P1/P2 Stage42 activation rows with valid object/masks, QPC order, bit0=1>0 and O+60 post == O+64 pre; no dropped/overflow/mismatch in STAGE42_HEALTH. Correlate to earlier Stage41 records by object and monotonic QPC/ID; do not enforce one-to-one counts because duplicate requests can be skipped, activations can come from another producer, and the log's last tail can be incomplete.

Known risks to check in Windows runtime: callsite may be independently modified by another hook, so Stage42 may fail closed without affecting earlier Stage40/41. If selector/mapping or stage41 log remains healthy but Stage42 windows are absent, inspect allCalls and otherObjects, not assume commands disappeared. A new native current-ID field value is **still not evidence of actual animation, hitboxes, damage, winning or AI quality**. Do not force requests or edit the original EXE.

Stage42 remains source-only until user x86 build and COM-vs-COM log upload. Preserve earlier stage branches and permanent history.
