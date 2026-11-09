# Stage25 runtime analysis — 2026-11-09 16:51–16:52

**Provenance:** runtime logs uploaded to root of `main` in commit `b9ba7472ccaeea110b8828098d04b23a264527df`, copied into this evidence branch as identical Git blob SHAs. Source/INI snapshot was prepared on Stage25 commit `3f034550f9d1254cc136452dde363657f913a91c`. The build transcript was pasted in chat and normalized into `Build_log_from_chat.txt`; the original Build.log file was not among the uploaded GitHub files. The running DLL cannot be binary-compared with source here.

## Compilation, installation, and integrity

- Chat Build.log: 2026-11-09 16:50:57, MSVC 14.51.36231 x86, 8 sources compiled, `BUILD SUCCEEDED`, `dinput8.dll` 237,568 bytes. One existing nonfatal C4754 in `AICommandSiteTrace.cpp:785`; not a Stage25 build error.
- Stage25 marker `STAGE25_SECONDARY_OBJECT_FLAG_BUILD=2026-11-09-r1`, installation at `0xF58C9` with validated 7-byte OR replacement, exactly one Stage25 record. Stage23/24/21/22 hooks also report installation.
- All 20 newly uploaded .log files were preserved. Stage25 dedicated log is 3 newline-terminated lines; one record, no truncated record. Nonempty moderate-sized logs were checked for ERROR/WARNING/DROPPED/OVERFLOW markers; none found.
- Fully fetched both large Git blobs: `ArcanaAI_PlayerBit17WriteTrace.log` 12,948,453 bytes, 24,176 seq1..24176 with no missing/duplicate IDs; `ArcanaAI_ProducerCallTrace.log` 8,833,751 bytes, 11,383 seq1..11383 with no missing/duplicate IDs. Both end with newline and contain no explicit ERROR/WARNING/DROPPED/OVERFLOW markers. Hardware watchpoints cannot prove absence of writes on unobserved threads.

## Same-event sequence: P1 type1 / payload 0x1E, thread 6128, tick 29260265

| Event | QPC | Finding |
| --- | ---: | --- |
| ALT_CALLSITE F6AD7 | 292602968311 | P1, queue pointer 0x0E2559A4, payload0x1E |
| QUEUE_EVENT_DETAIL seq15 | 292602968399 | accepted as (type=1,payload=0x1E), active 0→1 |
| EVENT_DISPATCH_DETAIL seq16 begins | 292602968416 | P1 event dispatcher |
| Stage23 resolver seq1 | 292602968420 | selector byte+3=31, record readable |
| **Stage25 post-OR seq1** | **292602971617** | source P1, target P2, `[P2+0xF8]` 1→1 |
| Stage24 helper completion seq1 | 292602973136 | selector31, args3/4=0/0, mapping bytes 03/00/10 |
| Stage21 classifier | 292602975570 | AL=0 |
| Stage21 fallback slot-clear | 292602976845 | slot3 0→0 |
| Stage22 post-AND | 292602977325 | controller bit0x400 0→0 |
| EVENT_DISPATCH_DETAIL seq16 ends | 292602977883 | accepted event remains matching |

Stage25 executes **3,197 QPC ticks after Stage23** and **1,519 ticks before Stage24 completion**. This is correct because the secondary OR occurs *inside* the original Stage24-observed helper.

Stage25 record: `player=0x0E251F78` (P1), `target=0x0E27CCA0` (P2), `field=0x0E27CD98`, `selector=31`, `arg3=0`, `arg4=0`, `before=after=0x00000001`; both read flags 1; `bit0Before=bit0After=1`; `observerReturnRVA=0xF58CE`; EFLAGS=0x202. `helperReturn=0x50B3BE21` is a wrapper return address while Stage24 is enabled, not a game-RVA.

Stage24 output moved from `(00000000,00000001,00000000)` to `(0000001F,00010001,00000000)`; helper EAX matched target P2. The mapping flags 0x03 demand secondary path, and Stage25 proves its original OR actually executed. **This sample did not perform a 0→1 transition** at P2+0xF8; the bit was already 1. The writer that established it and the gameplay semantics are not identified.

## Other observed bit17 episodes

Stage16 full write trace: P1 set seq3119 at `0xFAD2` followed by clear seq3128 at `0xED1A9`, both tick29254921; P2 set seq13050 at `0xFAD2` followed by clear seq13059 at `0xED1A9`, tick29265093. Both pairs correspond to Stage17 index0x71 setter calls in same-side/tick. The source OR at `0xFAD0` sets bit17; the clear instruction at `0xED19F` uses mask `0xFFF9FFFF`. These are **player+0x3B84 bit17** writes, not the newly observed `player+0xF8 bit0`.

Stage15 in the large ProducerCallTrace shows 10 branch records: an earlier P1 suppressed attempt (bit17=1), the accepted P1 event (bit17=0, F6A68 and F6A9D branches taken), and a later P2 suppressed attempt (bit17=1). The accepted P1 event proceeds through ALT_CALLSITE, queue and dispatcher. FinalCommandTrace monitors a different callsite, so no final-command payload0x1E there is not evidence of failure.

## Next investigation

Do **not** infer a transition from this 1→1 OR. Stage26 candidate: trace origin/lifecycle of bit0 at P1/P2+0xF8. Capture initial values and all observed write transitions with exact writer EIP/stack/QPC, without clobbering Stage16 DR0/DR1 watchpoints, or first locate all static writer sites to avoid expensive extra instrumentation. Distinguish game initialization from later action-state writes; obtain both P1/P2 and multiple events, and preserve original code/game behavior. A new source stage is not yet implemented.
