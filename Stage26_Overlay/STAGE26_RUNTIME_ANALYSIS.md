# Stage26 runtime analysis — 2026-11-09 17:28

Source snapshot: Stage26 branch `a249f452353c340fe8f1dab23375a8a932b079b4`. Uploaded root logs commit `8c6edc82d3146c11b3dd23d964471c31ba3fe816`; preserve the exact original blob SHA for each uploaded log. The normalized transcript is copied from the user's chat Build.log dated 17:27:39, not independently uploaded as a Build.log file.

## Compilation and installation

- MSVC 14.51.36231 x86; eight C++ translation units compiled; BUILD SUCCEEDED; `dinput8.dll` 240,640 bytes.
- Two nonfatal warnings: **C4456** in `src/AITrace.cpp:1671`, local `reported` shadows `reported` at line1656; simple variable rename will silence it in a future source revision, but the build and observed instrumentation worked. Preexisting C4754 in `AICommandSiteTrace.cpp:785` persists, independent of the new watchpoint mode.
- Stage26 marker `STAGE26_OBJECT_BIT0_WRITE_BUILD=2026-11-09-r1` at 17:28:21.575, config `EnableStage26ObjectBit0WriteTrace=1 Stage26TransitionsOnly=1 EnablePlayerBit17WriteTrace=0 EnableControlFieldTrace=0`.
- Stage26 active header 17:28:22.277: P1 `0x0E252070` (RVA `0x0DCA2070`), P2 `0x0E27CD98` (RVA `0x0DCCCD98`), initial values **both zero**, DR7=`0x00DD0005`, 100ms scan, excluded COM writer TID13952. Six installation batches 38+1+16+1+1+2 = **59** game threads. A successful batch does not mean all later/new threads were covered.
- `ArcanaAI_Stage26ObjectBit0WriteTrace.log`: 89,929 bytes, 150 newline-terminated lines, 140 records with seq1..140 all present/unique, no ERROR/WARNING/DROPPED/OVERFLOW; 140 records all in thread18044, all shadowInitialized=1, all real bit0 transitions, 70 0→1 and 70 1→0. By target: P1=82 transitions (41 sets+41 clears); P2=58 (29 sets+29 clears).
- `ArcanaAI_ProducerCallTrace.log` complete 7,563,025 bytes, 9,730 unique continuous seqIDs, 14 Stage15 branch records; `ArcanaAI_StateMapTrace.log` 1,268,525 bytes, 2,266 unique continuous seqIDs. Both end in newline and contain no explicit warning/error/drop/overflow. All other newly uploaded moderate logs inspected for markers and end newlines; no explicit failures.
- **Important historical leftover:** `ArcanaAI_PlayerBit17WriteTrace.log` has the **unchanged Git blob** `befd639d44731fd9cb5b35de44d49878406d716c` from Stage25 (12,948,453 bytes). Stage16 is disabled in this run. This file must NOT be attributed to the Stage26 runtime capture, despite appearing in the user's upload. Retained in the snapshot for exact provenance.

## Setters: Stage25 original OR (all 70 transitions)

Every 0→1 bit0 transition is a post-instruction trap at **DLL EIP 0x50B0C267**, where the Stage25 wrapper executes the original `OR DWORD PTR [EAX+0xF8],1` from AALib RVA `0xF58C9` (this is not an unmodified in-game EIP). P1 41 sets; P2 29 sets. The code window at the trapped EIP contains `83 88 F8 00 00 00 01` immediately before the pushfd/pushad logger sequence. This establishes a direct observed origin for all bit0 0→1 events during this monitored window, NOT exclusivity across every possible game state.

## Clearers: four post-instruction EIPs, 70 transitions

| post-write EIP RVA | instruction start RVA | observed operation | events | P1 | P2 |
| --- | --- | --- | ---: | ---: | ---: |
| 0x9787A | **0x97873** | `AND DWORD PTR [EDI+0xC8],0xFFFFFFF2` (clear bits 0,2,3) | 54 | 28 | 26 |
| 0x975AC | **0x975A6** | `MOV DWORD PTR [ESI+0xC8],EAX`, preceded by `AND EAX,0xFFFFFFFE` (ensures bit0 clear) | 7 | 7 | 0 |
| 0x95F5B | **0x95F55** | `MOV DWORD PTR [EDI+0xC8],EAX` with observed EAX=0 | 8 | 5 | 3 |
| 0x97037 | **0x97031** | `MOV DWORD PTR [ESI+0xC8],EBX` with observed EBX=0 | 1 | 1 | 0 |

In all clear records the relevant EDI/ESI equals `player+0x30`; therefore `[base+0xC8]` addresses `player+0xF8`. The instructions were decoded from the 24-byte window ending after the writer. At 0x97873 the imm8 F2 is sign-extended to 0xFFFFFFF2. Note that some paths overwrite the entire DWORD, not solely bit0.

## Exact Stage25/Stage26 same-event QPC correlation

There are **two** P2 type1/payload0x1E dispatch episodes. P2 is the source and P1 is the selected target (arg3=1, arg4=0, selector31). Each Stage25 post-OR record shows target P1 +0xF8 `0x0→0x1`:

| event | Stage23 resolver QPC | Stage26 trap for P1 set | Stage25 post-OR QPC | Stage24 completion QPC | Stage26 subsequent P1 clear |
| --- | ---: | ---: | ---: | ---: | ---: |
| #1 | 314503267114 | seq1 at 314503270177 | 314503270399 (+222) | 314503272207 | seq2 at 314503367634 (post-EIP 0x9787A) |
| #2 | 314608005325 | seq87 at 314608006662 | 314608006859 (+197) | 314608007660 | seq88 at 314608103823 (post-EIP 0x9787A) |

At the same events queue insertion occurred at QPC314503267061 and 314608005305; EventDispatch begin at314503267110/314608005318; Stage21 classifier AL0; fallback slot3 stayed0; Stage22 controller bit400 stayed0; EventDispatch finish at314503275423/314608010859. Stage24 selected P1 as helperResultEAX and wrote `(0x1F,0x10001,0)` in both episodes. No corruption or contradictory order is visible.

The Stage26 trap precedes Stage25 logger by 222 and197 QPC ticks, because the exception triggers immediately after OR and Stage25 writes its log afterward. The Stage26 follow-up clear happens later via AALib writer 0x97873, not as part of the Stage25 observer.

## Stage14/15, other probes, caveats

ProducerCallTrace: 14 Stage15 branch records, including two P2 accepted bit17=0/F6A68 branch-taken attempts and P1 suppressed bit17=1 attempts. Stage17 contains 4 setter index0x71 records, but Stage16 bit17 hardware-write logging is disabled in this run and the old Stage16 log must not be used as fresh evidence.
QueueTrace 40 QUEUE_EVENT_DETAIL records including 2 type1/payload0x1E; EventDispatchTrace 42 EVENT_DISPATCH_DETAIL records including both. FinalCommandTrace contains no type1/payload0x1E at its separately monitored final-command callsite, as in earlier runs.
All 140 Stage26 samples share game thread18044, and both player field addresses have correct relative offsets. The watcher shadow is still not an atomic pre-instruction read, and writes on uninstrumented threads may not be recorded; the balanced 70/70 is an observation, not a universal invariant.

## Next investigation

**Stage26 satisfies the immediate setter/clearer attribution question.** A natural Stage27 task is targeted static/control-flow analysis of clear paths (0x97873, 0x975A6, 0x95F55, 0x97031), their parent callers and P1/P2 object+0x30 alias, with the goal of identifying whether flag bit0 represents an ephemeral object/action state. Use existing Stage26 evidence first; do not add new intrusive hooks without a precise unproven question. A clean-up source revision may rename the inner C4456 `reported` variable, but do not rewrite the already built/tested Stage26 source branch. Preserve exact source, uploaded logs, chat Build transcript and this analysis in an immutable evidence branch.
