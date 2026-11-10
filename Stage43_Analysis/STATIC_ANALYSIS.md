# Stage43 static analysis — original native 1E9F40 caller provenance

Date 2026-11-10. Stage42 live upload [a1096e9](https://github.com/Hawky11/1/commit/a1096e980b6d40c290741693b6e8bc9116af01c8) is archived in [stage42-runtime-activation-evidence](https://github.com/Hawky11/1/tree/stage42-runtime-activation-evidence) with full runtime analysis and [356 exact joins](https://github.com/Hawky11/1/blob/stage42-runtime-activation-evidence/Stage42_Runtime/stage41_to_stage42_activation_join.csv). Stage42 native promotion O+64 -> O+60 was verified 356/356, 350/350 Stage41 NEW/REQUEUE events each matched a distinct Stage42 promotion. Six other activations came from additional/intermediate native producers (three P1 0x25 matching earlier O+68, three P2 0x24C/0x24D/0x24C with unknown upstream).

## Grounded original PE and stack ABI

Original SHA-pinned AALib.exe extracted from already available ChatGPT Library /AALib.zip: 175556608 bytes, SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba. PE32 i386, preferred base 00400000. Exact native original RVA 1E9F40 six bytes 80 79 30 00 74 16 (CMP [ECX+30],0; JE 1E9F5C), no prologue/stack frame. At RVA 1E9F62 original E8 09 FD FF FF CALL 1E9C70, with return RVA 1E9F67. Native 1E9F40 has no PUSH/stack adjustment before its 1E9F62 CALL.

A binary scan of the entire original .text section (RVA 00001000, 0x2BE000 raw bytes) found **14 direct CALL rel32 to 1E9F40**, each verified to target the exact RVA:

- 10164, 1023A
- AD75C, AD778, AD797, AD7BC (the four frame-sweep linked-list calls)
- AEA69, AEAD9, AEB09
- 13A20B, 13DFB9
- 1917AB, 19C2E1, 1A8E48

The Stage42 naked wrapper, at entry from original 1E9F62, sees [ESP]=return to 1E9F67 and [ESP+4]=return to the original caller of 1E9F40. It performs PUSHFD then PUSHAD: saved[3] is ESP after PUSHFD, so the exact **read-only** stack addresses saved[3]+4 and saved[3]+8 hold those returns. Successful SafeReadDword values are adjusted by runtime g_gameBase, the caller site is original return RVA minus 5, and compared to the 14 verified original direct CALL RVAs. Unknown callsites are logged with knownCaller=0, not blocked or treated as failure. No additional stack frame or return-address modification occurs.

## Proposed Stage43 runtime addition (no extra hooks)

Source-only [Stage43_Overlay/src/Stage42ActivationHook.inl](../Stage43_Overlay/src/Stage42ActivationHook.inl) replaces ONLY the existing Stage42ActivationHook.inl in the local already working Stage42 project. Stage38CallbackWindow.inl continues to include the same filename; Stage40RequestConsumer.inl (Stage41), Stage39ActionOutput.inl, AIQueueTrace.cpp, Stage35/36, Stage29 AITrace.cpp, INI, Build.bat untouched.

Preserved exactly ONE MakeRelativeCall patch at 1E9F62, exactly ONE original native replay call and two FXSAVE/FXRSTOR pairs. Code still takes P1/P2 five-field snapshots into Stage42 ring, worker, log, and health. Adds five DWORDs to each Stage42 ring record for original return address provenance and a companion ArcanaAI_Stage43ActivationOriginTrace.log emitted by the same worker after each Stage42 activation row. Marker STAGE43_ACTIVATION_ORIGIN_BUILD=2026-11-10-r1 and STAGE43_ENABLED piggyback=Stage42 1/1. Includes exact same stage42Seq/side/object/QPC, nativeReturnRVA, callerReturnRVA, callerRVA, callerRead, knownCaller, pending ID, active ID, pre O+30 and pre O+68. Static source checks confirm 14 sites in list, one patched CALL, two matched FPU save/restore pairs and balanced braces. Original PE 14/14 CALL signatures verified manually with Python SHA-pinned PE section mapping. **No Windows x86 Stage43 compile/run yet.**

## Hypothesis and limits

The six Stage42 extra activations were not wrong: all 356 used pending ID correctly and cleared bit0. The three P1 events may be from 1E9F40 prelude (O+30 nonzero and O+68 non-sentinel moved to O+64); the three P2 events had O+30=0 at Stage42 entry and their writes to O+64 are not sourced by Stage41. Capturing original native caller could identify a different caller/path for these activations in a new live run. **It does not itself prove where O+64 was previously written**. If every P2 event comes from same caller, next targeted diagnostic must inspect the writer of O+64 rather than claiming game error.

Do not change original control/flags/payloads or infer animation, hitbox, damage, successful attack or AI improvement solely from activated IDs. Preserve all historical branches and runtime evidence. The EXE is in Library; never ask the user to upload it again.
