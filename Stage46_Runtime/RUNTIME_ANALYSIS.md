# Stage46 runtime: definition-driven motion-state initialization — VERIFIED

Date: 2026-11-10. Original user upload: [main 3e668d7](https://github.com/Hawky11/1/commit/3e668d74ec1181ee6adf5952c7665e8948b767df).
Stage46 source: [stage46-definition-motion-state-trace](https://github.com/Hawky11/1/tree/stage46-definition-motion-state-trace), source commit 2365dec.
This evidence branch was cut from the **exact upload commit** before analytical files were added; original uploaded Build.log and all diagnostic log blobs in the branch root are retained unchanged.
See [297-row CSV](stage46_motion_binding_join.csv) for the full P1/P2 join, guard/byte mode, raw IEEE754 values, and field changes.

## Windows x86 build and probe health

Build.log at 2026-11-10 12:51:39 local: **BUILD SUCCEEDED**, dinput8.dll 1,056,256 bytes.
The only recorded compiler warning is preexisting AICommandSiteTrace.cpp C4754. STAGE46_DEFINITION_MOTION_BUILD=2026-11-10-r1, STAGE46_ENABLED, STAGE45_INSTALLED 1/1, STAGE44_INSTALLED 1/1, Stage42 INSTALLED 1/1, Stage43 enabled and Stage40 3/3.
Stage46 has 297 sequential 1..297 full-length records (P1=152, P2=145), all resolverCount=1, preMask=postMask=1FFF, metaMask=F, definition=definitionAfter. No truncation or malformed records in Stage36/37 and Stage39–46 readable files; lengths match Git blob sizes and each ends in newline. Full 4.9 MB Stage38 and Stage35 large logs were not text-verified by the connector. Do not treat them as analyzed.
Stage44 HEALTH: calls=9706, otherObjects=9409, P1/P2=297, 33 samples. Stage45 HEALTH: calls=52979, otherObjects=52682, P1/P2=297, 34 samples. Stage42 HEALTH: calls=2943, otherObjects=2674, P1/P2=269, 33 samples. Every health sample has dropped=overflow=mismatch=0.

## Exact cross-stage joins

- Stage46 ↔ Stage44: **297/297** on seq/side/player/object/tid/parent QPC/after QPC and result pointer.
- Stage46 ↔ Stage45: **297/297** on side/player/object/tid/bindingQpcBefore, ID, variant, native EAX=Stage46 definition and timing within parent. Stage46 resolvedQpc minus Stage45 qpcAfter: 0 ticks (221), 1 tick (75), 9 ticks (1, seq80). The 9-tick gap is still within the same binding window and is NOT a failed resolver join.
- Stage45: 297/297 path3, predictionMatches=1, resultIsFallback=0; no nonzero variant observed. No evidence of fallback use in this run.
- Stage42 and Stage43: **269/269** activations, all native caller RVA AD75C; all 269 have exactly one Stage44 binding nested in the activation interval. **28 Stage44 bindings outside Stage42** (including initial two ID=0) are real nested resolver/binding records; their upstream attribution is unknown. Do not label them lost commands.
- Stage40 ↔ Stage41: **294/294** windows; 266 Stage41 writes change O+08 bit0 from 0 to 1 and match 266 distinct later Stage42 activations by same P/O/ID and QPC. 28 Stage41 windows do not set bit0. Three Stage42 activations have no matching *new* Stage41 write in this run: seq3 (P2, 0506→0000), seq4 (P1, 0506→0000), seq209 (P1, 002F→0025), all with caller AD75C; investigate source separately instead of assuming dropped requests. No new Stage41 request is unmatched.

## Definition-command modes and object-state effects

Stage46 mode = low byte of little-endian DWORD definition+30:
mode 0: 208; mode 1: 9; mode 2: 2; mode 3: 51; mode 4: 19; mode 5: 8. Modes 6..9 are not observed.
Guard (signed O+70 < O+6C and O+08 bit9 clear) passes **290/297** windows.
Seven guard-false records (seq60,63,121,122,123,125,126) all have object+08=00004600, mode0, and show no float-field change. All 297 advance O+70 (f2; observed PRE→POST). Some other fields can change outside the dispatch; the snapshots bracket the entire native 1E8490 function, not individual instructions.

**68/297 windows change at least one of the eight raw IEEE754 fields f5..f12**, with per-field changed-record counts: O+7C 1; O+80 11; O+88 44; O+8C 23; O+90 32; O+94 4; O+150 0; O+154 3. These counts overlap and are NOT 118 separate commands. Some differences are sign-of-zero (e.g. 00000000→80000000); no movement distance or damage can be inferred from them.
Illustrative verified runtime examples:
- Stage44 seq7 / mode3 / P1: O+88 0→about 3.86, O+90 0→about 0.64.
- Stage44 seq3 / mode2 / P2: O+7C 890→982 and O+80 0→196.
- Mode3 seq103 / P1: O+88 0→-8, O+8C 0→3.45, O+90 0→0.36, O+94 0→0.4.
The 51 mode3 bindings are particularly useful targets for a later *consumer-side* trace. Static native dispatch at 1E8D30 and guards at 1E8A1F/1E8A2D are independently checked in Stage46 static evidence. This runtime establishes actual numerical object-state initialization by native action definition, **not visible animation, position movement, hitboxes, damage, intelligent decision-making or improved AI**.

## Next — Stage47 candidate (SOURCE NOT PREPARED)

Use the SHA-pinned original AALib.exe already in ChatGPT Library (do not ask user to upload again). Statistically identify the downstream **readers and consumers** of O+88/+8C/+90/+94 (and O+7C/+80) in the update/animation/position path. Choose a concrete verified native callsite/field read; design passive x86 instrumentation only after disassembly/signature and register/stack/FPU preservation checks. Correlate read-time state with Stage46 mode3 initialization by P/O, ID and QPC, then separately test whether the consumer writes physical position/visual-animation state. Do NOT edit or force game state, trigger inputs or infer damage. Preserve all old stage branches and Stage46 original logs. Stage47 Windows DLL is NOT built.
