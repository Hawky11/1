# Stage43 Windows x86 runtime VERIFIED — native caller provenance

Fresh user upload [main commit 09526f3](https://github.com/Hawky11/1/commit/09526f3edebd3057bad4a524cbaa75509a7fbfd6), 2026-11-10 06:57:18 UTC. This branch starts at the exact user-uploaded commit so the original Build.log and ALL runtime log blobs at root are preserved byte-for-byte; historical Stage42/41/etc evidence branches are untouched. Exact 232-row provenance: [stage43_native_origin_join.csv](stage43_native_origin_join.csv).

## Build, install, health

Windows x86 Build.log began 10.11.2026 09:55:43 local; BUILD SUCCEEDED, dinput8.dll 1041408 bytes; one pre-existing C4754 in AICommandSiteTrace.cpp, no compiler/link errors. Stage43 marker STAGE43_ACTIVATION_ORIGIN_BUILD=2026-11-10-r1 and STAGE43_ENABLED piggyback=Stage42 1/1. Stage42 INSTALLED 1/1, Stage41 ENABLED, Stage40 INSTALLED 3/3.

232 consecutive Stage43 origin windows 1..232 join 232/232 to Stage42 on seq, side, player, thread, object pointer, both QPC, active/pending IDs and pre O+30/O+68. No gaps or errors. Every nativeReturnRVA=001E9F67, callerRead=1, knownCaller=1; **ALL 232** have callerReturnRVA=000AD761 and callerRVA=000AD75C, split P1=114/P2=118. Therefore in this natural COM-vs-COM run the native activations originate at AD75C in AD710's object sweep. This does not rule out other callers in other play sessions, and the caller of 1E9F40 is not necessarily the original writer of O+64.

232/232 Stage42 activation windows have full object masks 1F/1F, monotonic QPC, O+08 bit0 1->0 and O+60 POST equal O+64 PRE; O+60 changed in 230/232, already matched in 2/232. Stage42 30 HEALTH messages total calls 2090, otherObjects 1858 and P1/P2 232, dropped=overflow=mismatch=0 in every message.

Stage41 295 consecutive object windows exactly join Stage40 295/295, valid masks and stable object, 232 NEW/REQUEUE (all correct O+64 POST=raw ID, bit0 POST=1, low byte O+30=0) and 63 CURRENT/DUPLICATE (all O+68 POST=FFFFFFFF, not necessarily newly changed). **All 232 NEW/REQUEUE** Stage41 observations join 232 DISTINCT immediately subsequent Stage42 activations of same object and ID; no unmatched new commands, no extra activations in this run. Earlier six extra activations from Stage42 run did NOT recur, so their specific upstream writers remain unproven.

Stage39: 39/39 output requests join first same-side Stage40, exact ID. Stage37: 45/45 effects match EventDispatch type1 by seq, side, player, QPC; EventDispatch 161 details (116 type0, 45 type1). Stage36 276 candidates, 7 non-sentinel native results. No explicit ERROR/WARNING in checked smaller logs. Huge Stage38 and Stage35 logs were uploaded but GitHub fetch_file returned empty text, so full-file integrity of those two cannot be asserted; raw Git blobs are preserved in this evidence branch.

## Next original native boundary (STATIC ONLY)

The SHA-pinned original AALib.exe in ChatGPT Library (SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba) was independently disassembled. After native 1E9C70 promotes O+64 into O+60 at 1E9E10, it CALLs native **1E8490** at callsite **1E9E2D**, original bytes E8 5E E6 FF FF.

1E8490 receives ECX=O, calls 1E4800 and 1E0120 to obtain the action table/context, then at RVA 1E84B3 CALLs 1DF2D0 with ECX=context and stack args (O+60 current ID, O+6C variant). 1DF2D0 returns a pointer to action-definition data or fallback, cleaning args with RET 8. Native 1E8490 stores it into **O+34** at RVA 1E84B8, sets byte **O+26D=1** at RVA 1E84BB, reads metadata byte [O+34]+2C and stores it to **O+178** at RVA 1E84D5; further state changes follow. This is a stronger boundary: **active ID -> action-definition binding**. It is NOT yet runtime-proven and should not be conflated with actual animation, hit, damage, win or AI quality.

A potential Stage44 should strictly validate original CALL and native prologue and wrap ONLY 1E9E2D->1E8490, replay native once, and capture real P1/P2 O+60, O+34, O+26C (containing byte+26D), O+178, O+6C and safe pointer metadata before/after. Preserve existing Stage43/42/41/40 hooks, no injections, no game writes, no synthetic requests. Original EXE never needs re-upload; all historical branches retained.
