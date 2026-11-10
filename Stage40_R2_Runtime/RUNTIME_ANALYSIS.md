# Stage40 r2 — Windows x86 runtime verified, direct object action path identified (2026-11-10)

## Provenance and integrity

Original user upload in [main commit e4d29fa](https://github.com/Hawky11/1/commit/e4d29fa639c74ab3e5345cf91fec729d43e43f78) on 2026-11-10 05:49:41 UTC. This evidence branch was created at that exact commit. Its original root Build.log and newly uploaded runtime log Git blobs are retained byte-for-byte, without modification. The analysis and exact 56-row correlation CSV were added under Stage40_R2_Runtime. Preserve all older source and evidence branches, including Stage40 r1 failed installation evidence.

Original AALib.exe was re-extracted from already-held ChatGPT Library AALib.zip; its SHA-256 matches 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba (PE32/i386, 175556608 bytes). Original disassembly rechecked independently with objdump and PE section mapping. No need for another EXE upload.

## Real build and hook health

Build.log began 2026-11-10 08:47:58 local and reports genuine Windows x86 BUILD SUCCEEDED with 688128-byte dinput8.dll. No compiler/linker errors, only historical C4754 in AICommandSiteTrace.cpp. Stage40 file starts at 08:48:24 with STAGE40_ACTION_CONSUMER_BUILD=2026-11-10-r2, followed by STAGE40_INSTALLED 3/3 AD72F->E8480, E84E3->125920, E8505->1357F0. Thus r2 relocation-aware signature validation passed in the live game and its three hooks installed successfully.

ArcanaAI_Stage40ActionConsumerTrace.log has 489 consecutive STAGE40_CONSUMER_WINDOW seq=1..489, no sequence gaps; all per-player snapshots preMask=FF and postMask=FF; QPC before <= selector <= after, no explicit WARNING, ERROR, dropped, overflow or mismatch in that retrieved file. File is newline-terminated. Stage40 counts P1=271, P2=218, TID=19980, with 95 distinct non-sentinel incoming raw IDs. All 489 have p0 non-FFFFFFFF before, FFFFFFFF after, selectorSeen=1, selector=FFFFFFFF, no native actor helper invocation (actorSeen=0; actor masks 0). This is a real observed branch, not an installation failure.

Additional parsed logs: Stage39ActionOutput 57 contiguous valid output windows, Stage37Type1Commit 59 effect rows, Stage36 r2 406 sequential CANDIDATE results (395 nativeEAX=FFFFFFFF, 11 non-sentinel), EventDispatch 188 detailed rows (129 type0, 59 type1), QueueTrace 57 detailed queue records. All 59 Stage37 rows join exactly to the 59 EventDispatch type1 details by dispatchSeq, side, player, thread and both QPC timestamps. No explicit ERROR/WARNING in these checked smaller streams. Stage38CallbackWindowTrace and Stage35ControllerConsumerTrace are large, and the available GitHub text retrieval returned empty contents for them; the original full blobs are preserved in the evidence branch but complete line-by-line integrity/warning checks for those two files were NOT performed in this review. Do not claim all observers fully verified from these results.

## Strong Stage39 -> Stage40 linkage

For each Stage39 request, find the immediately following same-side Stage40 consumer before any later Stage39 request for that side. Exact joins by player, raw ID and QPC chronology: **56 of 57**; all 56 Stage40 sequence targets are distinct. QPC elapsed values range 34 to 1292 ticks, median 449 ticks. There were no different-ID first-consumer mismatches in the 56 matched instances. Stage39 stage38Seq=6020 / P2 raw ID=00000303 has qpcAfter=866849261427, which occurs after the last captured Stage40 consumer (qpcBefore=866844261139); its consumption was not observable in this log. Do NOT call this unobserved tail a dropped or failed action. See stage39_to_stage40_next_consumer.csv.

## Why 125920 returns FFFFFFFF — confirmed native lookup semantics

Original E8480 (image VA 004E8480, RVA E8480) loads P+3F4C; sentinel bypasses, otherwise CALL at RVA E84AB calls native **1E9F00** first. Next, native CALL RVA E84E3->125920 performs a specific mode/action-ID lookup. The 125920 function reads a six-byte-record static lookup table at original VA 05CF7260, with type byte, requested word and resulting word, terminated by type FE. PE mapping of RVA 058F7260 to on-disk .data revealed exactly **40** entries: type/mode 8:3, 9:28, 29:9. E8480 uses P+3F20 for mode except when P+3978==29 with guard, in which case it uses mode 22. In this runtime, for every one of the 489 windows both P+3978 and P+3F20 are 10 (P1, mode 0x0A) or 23 (P2, mode 0x17), so the native special lookup cannot find any mode match. All 489 sentinel selector results are therefore precisely consistent with original code/table without asserting any gameplay AI failure or ABI error.

When the special lookup fails, native E8535 clears bit24 P+3B88; both branches reset P+3F4C at E853F and clear bit24 P+3B7C at E8549. These effects were observed for the raw request reset, and Stage40 flags are consistent. Actor+280 at E8523 is in the OPTIONAL successful selector branch only; 489/489 miss this branch, so there are zero actor snapshots, expected.

## More important downstream output before lookup

Original E84AB -> native function **1E9F00**, called with ECX=[P+4] (actual player object) and three stack arguments (raw request P+3F4C, zero, bit24 of P+3B7C). Its actual disassembly:

- If third argument low byte is zero and [object+60]==raw request, it writes [object+68]=FFFFFFFF and returns AL=0 (current/duplicate request branch).
- Otherwise writes byte [object+30]=0, [object+64]=raw request, sets bit0 in [object+08], and returns AL=1 (direct object action request state). It returns with RET 0Ch (callee cleanup).
- This occurs **before** the optional 125920 lookup, so no actor+280 write is needed for the main direct object state path. Stage40 r2 does not read O+60/64/68 pre/post, thus this specific state mutation is STATIC ONLY until observed.

Next source-only [Stage41 direct object observer](https://github.com/Hawky11/1/tree/stage41-direct-object-commit-window) adds five masked object DWORDs to the existing tested Stage40 before/after windows, using the same TLS/ring/worker. It does not patch additional original instructions or synthesize inputs. The source has not yet been compiled/installed or observed at runtime. Use the one-file installation instructions in Stage41_Overlay/README_STAGE41_RU.md. Do not claim that a native request state write is an animation, hitbox, collision, damage, win, or an improvement of AI decision quality.
