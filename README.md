# ArcanaAI / COM vs COM diagnostic research

## Latest — Stage29 bit17 producer correlation source prepared (2026-11-09)

- **Source-only Stage29 branch:** [stage29-bit17-producer-correlation](https://github.com/Hawky11/1/tree/stage29-bit17-producer-correlation), HEAD `ad78d4668bd4ca14fc471289037bd6ef9ed62f0b`. Contents: `Stage29_Overlay/src/AITrace.cpp`, `Stage29_Overlay/ArcanaAI.ini`, [Stage29 static baseline](https://github.com/Hawky11/1/blob/stage29-bit17-producer-correlation/Stage29_Overlay/STAGE29_STATIC_BASELINE.md), [apply/readme](https://github.com/Hawky11/1/blob/stage29-bit17-producer-correlation/Stage29_Overlay/README_STAGE29.md), static verifier and a copy of `BINARY_SOURCE_IN_LIBRARY.md`. **Not compiled with MSVC or in-game validated yet.**
- **Why:** same 2026-11-09 17:28 run had four payload0x1E producer attempts: two P1 with bit17=1 took original `0xF6A75` alternative path (not queue); two P2 with bit17=0 reached `0xF6AD7`, queue, dispatcher and classifier AL0/fallback. Original `0xE8CF0` classifier *must* return AL0 for payload0x1E due range checks. The P2 fallback slot3=0→0 and controller bit400=0→0 are expected observed no-ops, not hook errors. **The P1 bit17 writer's QPC in the same run remains unobserved** because Stage26 watched bit0 instead.
- **New Stage29 mode** reuses Stage16 write-only DR0/DR1 and captures only P1/P2 `player+0x3B84` **bit17 changes** while updating shadow on each observed write. Marker `STAGE29_BIT17_TRANSITIONS_BUILD=2026-11-09-r1`, log `ArcanaAI_Stage29Bit17Transitions.log`, event `STAGE29_BIT17_TRANSITION`. Default INI: bit17 watcher=1; Stage29 filtering=1; Stage26 bit0 watcher=0; control watcher=0. Existing Stage14/15, Stage17–25 traces stay enabled. Stack snapshot and QPC included for each retained transition. Stage26's C4456 shadowing warning also fixed without modifying its immutable branch.
- Previous Stage16 **different-session** 16:51 run: 24,176 raw writes, just four bit17 toggles (two sets at post-EIP0xFAD2; two clears at post-EIP0xED1A9). Do not correlate its QPC with Stage15 17:28 run. Stage29 removes full-write log noise but still cannot capture writes on uninstrumented threads or the excluded COMvsCOM thread.
- **Next user action:** copy **only** Stage29 `src/AITrace.cpp` and `ArcanaAI.ini` over current `D:\INCUBATOR\ArcanaAIOverhaul` equivalents; **keep working Stage25/26 `src/AIQueueTrace.cpp` and all other files**. Build with existing x86 `Build.bat`, run natural COM-vs-COM and upload Build.log + fresh Stage29/Stage15/Stage17/queue/Stage21 logs. Archive results as new Stage29 runtime evidence; do not rewrite prior branches.
- **Binary is already in persistent ChatGPT Library:** `/AALib.zip` includes pinned original `AALib.exe`; [BINARY_SOURCE_IN_LIBRARY.md](BINARY_SOURCE_IN_LIBRARY.md) explains retrieval without reupload.

---


**Critical resource for future assistant sessions:** The exact original `AALib.exe` is already stored in the user's persistent ChatGPT File Library as **`/AALib.zip`**. **Do not ask for a new upload.** Read **[BINARY_SOURCE_IN_LIBRARY.md](BINARY_SOURCE_IN_LIBRARY.md)** for exact files.list/files.materialize instructions, SHA256, and PE RVA mapping. The EXE/ZIP is NOT in GitHub.

## Latest checkpoint — Stage28 original game EXE analyzed, 2026-11-09

- **Immutable Stage28 analysis:** [stage28-original-pe-state-lifecycle](https://github.com/Hawky11/1/tree/stage28-original-pe-state-lifecycle), HEAD `6a210f4be510ab264ddefcadd8d740bfe923cfa4`. [Detailed static findings](https://github.com/Hawky11/1/blob/stage28-original-pe-state-lifecycle/Stage28_Analysis/STATIC_STAGE28_ANALYSIS.md), SHA256-pinned verifier `tools/verify_stage28.py`, 6 tests and machine-readable summary. Stage28 is **offline-only**; no new DLL/game hook, no user game run.
- Found the user's `AALib.zip` in ChatGPT Library (not GitHub); exact original `AALib.exe` size 175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` verified. Original EXE/zip **not committed or shared**. Disassembled complete ~2.87MB `.text` plus targeted call graph. Local verifier passed 26 signatures, 16 direct CALL targets and one tail JMP; 6 Python tests passed including the actual local PE.
- **New finding:** the `0x978B0` function invoked after Stage27's state1 clear and on state0 can start state1 using `0x97830`. A bit1-set route at object+0xC8 (player+0xF8) clears bit1 and compares selected fighter's `(type,code)` header against `(0,0x13)`, `(0,0x14)`, `(0,0x15)` via `0xF2ED0`; if none match, it directly calls 0x97830. Otherwise a state0/countdown route checks object+0xC4 bit0, object+0xCC bit1, state=0; `0x991E0` decrements a countdown and returns its **old** value; a nonpositive old value also allows 0x97830.
- `0x97830` writes object state1, `0x97870` writes state2 and clears bit0; the two helpers share calls but differ in mode 0 vs1. **Important:** `0xF2ED0` is a tuple-comparison predicate, **not** queue insertion `0xF2DE0`. No causal connection to COM-vs-COM producer bit17 or fallback proven.
- Rechecked Stage26 full-DWORD values: all 140 captured records are exactly 70 `0→1` and 70 `1→0`; bit1=0 **at those observation points**. Because Stage26 `Stage26TransitionsOnly=1` filters writes that leave bit0 unchanged, the runtime sample does **not** disprove transient bit1 activity. Do not claim the special bit1 path was executed or never executed.
- **Next handoff:** Stage29 should refocus on original COM-vs-COM problem: producer bit17 gate at `0xF6A68`, accepted P1/P2 alternate queue insertion, and why the `0x1E` dispatcher runs classifier AL0/fallback. Use existing captured Stage15–26 data to pose a bounded question before adding probes. Keep Stage25–28 branches immutable.

---


## Previous checkpoint — Stage27 0x97970 CALL/tail-JMP control flow verified (2026-11-09)

- **Complete original PE analysis:** [stage27-bit0-tailjump-verified](https://github.com/Hawky11/1/tree/stage27-bit0-tailjump-verified), commit `78e48cdcef7a67fcdf64a50dfa95228357eedbe6`, descended from previous static evidence branch `stage27-verified-static-callgraph`. It contains the *exact* original 4,257-byte user-uploaded `Stage27_CallFlow_97970.txt` (Git blob `d5d8c9f90665baf1c625bb65515910c2984b7a64`) and [verified annotated disassembly](https://github.com/Hawky11/1/blob/stage27-bit0-tailjump-verified/Stage27_CompleteCallGraph/VERIFIED_97970_BRANCH.md).
- **Both pathways to primary bit0 clearer RVA0x97870 confirmed** in the SHA256-pinned original AALib.exe. `0x97609 CALL 0x97970`; when `[object+0xD4]==1`, and guard passes, `0x979CC CALL 0x97870` (34 Stage26 clear records, stack return0x979D1), then calls `0x978B0`. When `[object+0xD4]==2`, same guard leads to `0x979AC JMP 0x97870` *after* POP ESI/EBP (20 records, original return0x9760E preserved). State0 calls `0x978B0` without this clear, others/failed guards return.
- **Guard:** `[object+0xC4]&1==0`, `[object+0xCC]&7==0`, `[object+0xC8]&1==1`. Here `object=player+0x30` for captured records. The clearer masks `[player+0xF8]` with `0xFFFFFFF2`, sets `[player+0x104]=2`, and ORs `[player+0xF4]` with 0x400, besides helper calls. Exactly 54 clears observed: P1 28/P2 26. Both captured source-P2/payload0x1E events followed the state1 nested CALL path.
- **Stage27 complete as static/control-flow attribution**: no new DLL, game run, hook or build required. Remaining game-level semantics and any causal connection to COM-vs-COM suppression are *not proven*. Suggested next investigation is the `0x978B0` helper and its state-dependent effects, initially via offline byte windows rather than another runtime patch. Historical branches remain unchanged.

---


## Previous — Stage27 exact-PE static CALL verification (2026-11-09)

- User supplied `Stage27_AALib_StaticReport.txt` from the exact original local AALib.exe. SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` confirmed; all four bit0 clearer signatures and `0x97870` entry matched. **Exact 2,389-byte uploaded TXT** preserved in [stage27-verified-static-callgraph](https://github.com/Hawky11/1/tree/stage27-verified-static-callgraph), commit `2e08b356fa9923e57b0c78c08c70adbf8e1015c1`, blob `7e3dc3a77f602fde1e82ca9fe70903d4d29dd9c7`.
- **Corrected CALL graph:** `AALib RVA 0x979CC → 0x97870` (return `0x979D1`) is a verified direct E8 CALL, while `0x97609 → 0x97970` (return `0x9760E`) is a verified CALL to another function. Stage27's older prediction that both call directly into `0x97870` was wrong; its 34+20 dynamic stack counts remain unchanged. The transfer from `0x97970` to `0x97870` for the remaining 20 captures has **not** been disassembled yet.
- Decoded original PE windows further: `0x97870` clears object+0xF8 bits0,2,3, writes object+0x104=2, invokes three helpers then sets bit0x400 at object+0xF4; `0x975A6` is gated on `WORD [player+0x3A34]==1` and bit0 set, then clears bit0; `0x95F55` and `0x97031` participate in multi-field reset sequences. Do not conflate player+0xF4 bit0x400 with Stage22 controller+8 bit0x400.
- **Next:** inspect pinned executable bytes around function `0x97970` using the new read-only [inspect_call_flow.py](https://github.com/Hawky11/1/blob/stage27-verified-static-callgraph/Stage27_VerifiedStatic/tools/inspect_call_flow.py), export `Stage27_CallFlow_97970.txt` and upload its text only. Five local synthetic Python regression tests passed; the new script has not yet been run against the user's original PE. No DLL recompilation or gameplay session required.
- Previous Stage26 complete runtime evidence and original Stage27 analysis remain immutable and linked below. Detailed verified correction: [VERIFIED_CALL_GRAPH.md](https://github.com/Hawky11/1/blob/stage27-verified-static-callgraph/Stage27_VerifiedStatic/VERIFIED_CALL_GRAPH.md).

---


## Previous checkpoint — Stage27 offline lifecycle/caller analysis complete (2026-11-09)

- **Stage27 analysis-only branch:** [stage27-bit0-lifecycle-callers](https://github.com/Hawky11/1/tree/stage27-bit0-lifecycle-callers), commit `b490f87892791d45f0b0841a47e70d60329ec457`, based on preserved Stage26 runtime evidence. [Full Stage27 findings](https://github.com/Hawky11/1/blob/stage27-bit0-lifecycle-callers/Stage27_Analysis/STAGE27_LIFECYCLE_ANALYSIS.md), `cycles_stage26.csv` (70 matched pairs), `stage27_summary.json`, reproducible standard-library Python parser and 5 unit tests. No new hook, no changed DLL, no new runtime.
- All Stage26 P1/P2 bit0 sets and clears pair 70/70; main clearing helper at game RVA `0x97870` accounts for 54 clears. Its first instructions are `PUSH EDI; MOV EDI,EAX; AND [EDI+0xC8],0xFFFFFFF2`. From the known stack layout, two dynamic caller-return candidates were observed: RVA `0x979D1` (34, P1 16/P2 18) and `0x9760E` (20, P1 12/P2 8). All 34 nested records additionally carry `0x9760E` deeper on the stack.
- Other clear writers: instruction RVAs `0x975A6` (7), `0x95F55` (8), `0x97031` (1). The only two bit0 setters with EBX=`0x1E` were Stage26 seq1/seq87 (P2→P1); both cleared by 0x97873 through the 0x979D1 caller context.
- **Pending exact static check:** full `AALib.exe` not present in Git. Optional Python `--binary` mode verifies pinned SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, writer bytes and possible E8 CALL instructions at `0x979CC`/`0x97609`. Only after such verification may the dynamic caller-return inference be called a confirmed static call graph.
- **Next:** run Stage27 offline script against exact original local game EXE, share/export its short `Stage27_AALib_StaticReport.txt`; do not upload the entire proprietary game binary to GitHub. Python tests are supplied but not executed in the assistant environment. No MSVC/game build necessary for Stage27; Stage26 source/evidence remain immutable.

---


## Previous checkpoint — Stage26 runtime complete (2026-11-09)

- **Stage26 built and tested.** User's 17:27:39 chat Build.log: MSVC 14.51.36231 x86, all eight C++ translation units compiled, `dinput8.dll` 240,640 bytes. Nonfatal C4456 in `AITrace.cpp:1671` (inner variable `reported` shadows another) and previously known C4754 in `AICommandSiteTrace.cpp:785`. No compiler errors.
- Immutable source: [stage26-object-bit0-write-trace](https://github.com/Hawky11/1/tree/stage26-object-bit0-write-trace) at `a249f452353c340fe8f1dab23375a8a932b079b4`. Complete runtime archive: [stage26-object-bit0-write-runtime-evidence](https://github.com/Hawky11/1/tree/stage26-object-bit0-write-runtime-evidence) at `4b7564ff71664fedd5745e3bcfc4210d74540075` with all 21 root-uploaded Git log blobs (one stale Stage16 artifact), Build.log copied from chat and [Stage26 analysis](https://github.com/Hawky11/1/blob/stage26-object-bit0-write-runtime-evidence/Stage26_Overlay/STAGE26_RUNTIME_ANALYSIS.md).
- **Setter confirmed:** Stage26 startup P1/P2+`0xF8` both 0. DR0/DR1 write-only installed on 59 game threads in six batches, Stage16 disabled. 140 Stage26 records, seq1..140 complete: 70 transitions bit0 `0→1` (P1 41, P2 29), all from Stage25's faithful DLL wrapper of game OR at RVA `0xF58C9`; 70 `1→0` clears.
- **Four clear sites by post-write RVA:** `0x9787A` (54; game instruction at `0x97873` AND +0xC8,0xFFFFFFF2), `0x975AC` (7; MOV +0xC8,EAX after AND EAX,0xFFFFFFFE), `0x95F5B` (8; MOV +0xC8,0), `0x97037` (1; MOV +0xC8,0). The clearing register base equals player+0x30, giving player+0xF8. Higher-level game meaning is unproven.
- **Same-event validation:** two P2/type1/payload0x1E events queued and dispatched. In each Stage26 records P1 bit0 `0→1` **before** Stage25 post-OR logging (+222/+197 QPC ticks), followed by a later clear at post-EIP RVA `0x9787A`. Stage23 resolver, Stage24 consumer and Stage21/22 fallback also correlate.
- **Integrity:** Stage26 140 unique seq IDs; full Stage14 ProducerCallTrace 9,730 unique IDs, Stage19 StateMapTrace 2,266 unique IDs, no gaps/duplicates or explicit warning/error/drop/overflow; 21 uploaded logs preserve original SHA. Uploaded `ArcanaAI_PlayerBit17WriteTrace.log` is the **unchanged old Stage25 blob**, not new Stage26 bit17 evidence (Stage16 mode disabled).
- **Next (Stage27 not implemented):** static/control-flow study of the four clearing instructions and their callers to establish why/when the flag is cleared. Do not add more hooks until a specific unresolved question demands it. Preserve Stage26 runtime source/evidence untouched; optionally silence harmless C4456 in a separate future branch.

---


## Historical preparation — Stage26 source prepared (later built/tested)

- Source-only Stage26: [stage26-object-bit0-write-trace](https://github.com/Hawky11/1/tree/stage26-object-bit0-write-trace), HEAD `a249f452353c340fe8f1dab23375a8a932b079b4`. Apply `Stage26_Overlay/src/AITrace.cpp` and `Stage26_Overlay/ArcanaAI.ini` over the working Stage25 project; **retain Stage25 `src/AIQueueTrace.cpp`**. No project-file changes.
- Stage26 reuses the Stage16 write-only DR0/DR1 watchpoint pipeline but targets **P1/P2 +0xF8**, not +0x3B84. INI disables Stage16 and legacy control watchers to avoid conflicts. Marker `STAGE26_OBJECT_BIT0_WRITE_BUILD=2026-11-09-r1`; log `ArcanaAI_Stage26ObjectBit0WriteTrace.log`. By default log only transitions of bit0 (enable `Stage26TransitionsOnly=0` for all write traffic).
- Static source/INI consistency checked, including exact watch RVAs P1 `0x0DCA2070` and P2 `0x0DCCCD98`. Source has not been compiled with MSVC x86, instruction-level emulation has not been run, and no in-game Stage26 evidence exists. Prior Stage25 runtime remains the latest tested stage.
- Next: user builds using `Build.bat`, tests a fresh game session and uploads Build.log plus logs to `main`. Check Stage26 startup/installation and bit0 0→1 transition, correlate with Stage25 OR and prior queue events. Archive evidence separately; do not rewrite Stage25 sources/evidence.

---


## Prior tested checkpoint — Stage25 runtime complete (2026-11-09)

- **Build/runtime verified:** MSVC x86 Build.log pasted in chat: compiled successfully, `dinput8.dll` 237,568 bytes; one pre-existing C4754 at `AICommandSiteTrace.cpp:785`. Stage25 build marker, observer installation at `0xF58C9`, and one runtime event are present.
- **Immutable evidence snapshot:** [stage25-secondary-object-flag-runtime-evidence](https://github.com/Hawky11/1/tree/stage25-secondary-object-flag-runtime-evidence) at `b426a8f74764b7606ce3b5da075bb3a61d3698b`, with all 20 uploaded runtime logs, chat Build.log text, and [complete analysis](https://github.com/Hawky11/1/blob/stage25-secondary-object-flag-runtime-evidence/Stage25_Overlay/STAGE25_RUNTIME_ANALYSIS.md).
- **Confirmed:** P1 payload `0x1E` is enqueued/dispatched. Stage25 OR at RVA `0xF58C9` executes inside Stage24 helper: P1 is source, P2 is target, `[P2+0xF8]=0x1→0x1`; it did **not** set bit0 in this sample. QPC sequence: Stage23 resolver `292602968420` → Stage25 OR `292602971617` → Stage24 helper completion `292602973136`.
- Large logs checked completely: Stage16 24,176 write records and Stage14 11,383 producer records, complete sequence ranges without gaps/duplicates; no explicit errors/drop/overflow. Stage16 shows one P1 and one P2 bit17 set/clear cycle, unrelated to `[player+0xF8]`.
- **Next investigation (Stage26 not implemented):** discover the initial setter and lifecycle of bit0 at P1/P2+`0xF8`, ideally via static writer xrefs first, then passive writes-only observation if necessary. Preserve Stage25 source/evidence branches and existing game-state semantics. Update `NEXT_SESSION.md` when a safe implementation is prepared.

---


## Previous checkpoint — Stage25 prepared, Stage24 follow-up complete

- Source-only Stage25: [stage25-secondary-object-flag-trace](https://github.com/Hawky11/1/tree/stage25-secondary-object-flag-trace), commit `3f034550f9d1254cc136452dde363657f913a91c`. Apply `Stage25_Overlay/src/AIQueueTrace.cpp` and `Stage25_Overlay/ArcanaAI.ini` to the working Stage24 project. No project-file change.
- [Follow-up analysis](https://github.com/Hawky11/1/blob/stage25-secondary-object-flag-trace/Stage25_Overlay/STAGE24_FOLLOWUP_AND_STAGE25_STATIC.md) completes the two previously unread large Stage24 logs and corrects the argument dataflow using the fingerprinted EXE.
- Both large files fully retrieved and exact Git blob hashes verified. Write trace:21886 records, no missing/duplicate sequence IDs, blocks can appear out of order. Producer trace:10463 ordered records without gaps. No explicit errors/drops/overflow markers.
- P2 setter->bit17 OR confirmed: Stage17 QPC0x41E9BA1075 -> write seq15206 QPC0x41E9BA11CE, delta345. Clear seq15215 at ED19F uses **0xFFF9FFFF**, not the previously stated0xFFFDFFFF; it clears bits17/18, with observed0x30000->0x10000.
- **Argument issue resolved:** EE2F0 reloads ECX from player+3F34 (arg4); EE2F6 reloads EDX from player+3F30 (arg3). Resolver bytes3/18 are not the final arguments. Zero/zero in Stage24 is correct for those fields.
- **Secondary-path correction:** F58C1 calls9EF00(primary arg3,-1), not arg4. With arg3=0 it selects slot1; Stage24 returned EAX matches P2. Stage25 observes the original OR[target+F8],1 at F58C9 directly.
- Stage25 log:`ArcanaAI_Stage25SecondaryObjectFlagTrace.log`; marker:`STAGE25_SECONDARY_OBJECT_FLAG_BUILD=2026-11-09-r1`. Records source/target player, selector/args, before/after field and QPC. Original OR executes once; no extra game-state writes.
- Validation: exact PE SHA256 and patch signatures; extracted x86 wrapper assembled and compared with original OR in128 randomized Unicorn cases (registers, ESP, EFLAGS, output, logger context). **Not compiled with MSVC and not tested in-game yet.**
- Next: inspect user's new Build.log, then Stage25 marker/hook installation and records. Correlate Stage25 QPC between Stage23 and Stage24 records. Establish whether target bit0 transitions0->1 or is already1; gameplay semantics remain unknown.
- Stage24 evidence remains immutable at `983a7f4f205c99cd1c8f44814c58cc09ed76f0c8`. Its initial transport limitation is superseded by this follow-up; no old logs or historical source branches were rewritten.


---

The checkpoint sections below predate this Stage25 handoff and are retained as historical context.


This repository tracks the staged investigation of the ArcanaAI COM-vs-COM event/queue path.

## Start here

1. Read **[NEXT_SESSION.md](NEXT_SESSION.md)** first. It is the current handoff for the next AI/chat and contains the exact next task.
2. Read **[PROJECT_HISTORY.md](PROJECT_HISTORY.md)** for the append-only stage history and the evidence behind the current conclusions.
3. Open the referenced stage branch and inspect its README, overlay, and runtime logs before making changes.

## Current checkpoint

- **Latest diagnostic source (Stage23 r2):** [stage23-resolver-result-trace](https://github.com/Hawky11/1/tree/stage23-resolver-result-trace), HEAD `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`. Passive wrapper at resolver CALL RVA `0xEE273` -> helper RVA `0x12EAC0` captures the 12-byte resolver result before downstream EDI reuse. Both pointer-cast compiler errors from the first attempt were corrected in r2. The Stage22 source snapshot remains preserved unchanged on this branch. Two separate Stage23 r2 runtime captures are preserved: [first run](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence), HEAD `306b006a2230c1226a49ab93b9b8b74722fd1683`, and [second run](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence-run2), HEAD `0e6ed619a4e812687e615b00535ef17e54118b5b`.
- **Latest build/runtime-tested source:** Stage23 r2, [stage23-resolver-result-trace](https://github.com/Hawky11/1/tree/stage23-resolver-result-trace), HEAD `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`. The resolver-result observer installed and produced three readable records.
- **Latest runtime evidence:** [stage23-resolver-result-runtime-evidence-run2](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence-run2), HEAD `0e6ed619a4e812687e615b00535ef17e54118b5b`. Contains all 18 newly uploaded runtime logs, the pasted 09:04 Build.log and `Stage23_Overlay/STAGE23_RUNTIME_ANALYSIS_RUN2.md`. The first Stage23 runtime evidence branch remains preserved separately.
- **Stage23 static correction:** the exact AALib binary confirms byte `+3` of the record returned by `0x52EAC0` is read at `0xEE28D`, consumed downstream, then EDI is overwritten at `0xEE30B` with dispatcher `[EBP+8]`. Thus `CMP EDI,1` at `0xEE3E1` tests dispatcher event type, not the resolver byte. Stage21's later EDI value becomes the controller-array pointer at `0xEE3FC`. Stage23 observes the resolver return directly and renames the inherited Stage21 log field to raw `preCallEdi`.
- **Stage21 r1 source used by the successful build:** [stage21-payload-1e-consumer-trace](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-trace), HEAD `d210c3540ddd29fff1795ef8da6d3605958d481a`. Both consumer CALL wrappers installed. P1 and P2 type-1/payload-`0x1E` events returned classifier AL=0 and both fallback helpers executed. Slot 3 and controller bit `0x400` were already zero before the respective fallback calls.
- **Latest build result:** 2026-11-09 09:04:32, VS 18 Community / MSVC 14.51.36231 x86; all eight C++ translation units compiled and `dinput8.dll` linked at **229,376 bytes**. One non-fatal C4754 warning remains at `AICommandSiteTrace.cpp:785`. The complete Build.log transcript is archived with the Stage23 second-run evidence.
- **Stage20 → Stage17 → Stage16 chain confirmed for P1:** natural object-bit-2-clear / helper AL=0 at `0xC64F1`; setter index `0x71` at `0xC6502` 79 QPC ticks later; real bit17 write `0→1` at RVA `0xFAD2` after another 361 QPC ticks. The Stage16 trace later records the clear at RVA `0xED1A9`, 2,701,840 QPC ticks after the set. The 7,565,612-byte Stage16 file contains 14,081 write records and exactly these two bit17 transitions.
- **Stage21 r1 diagnostic limitation:** its `decodedActionCode` field sampled EDI after the classifier helper; values match the controller-array pointers and are invalid as action codes. Slot 3 and controller bit `0x400` were both already zero in P1/P2 samples. Stage18 installed but recorded no gate-result entries.
- **Stage21 r2 EDI correction re-evaluated at runtime:** [stage21-payload-1e-consumer-trace-r2](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-trace-r2), HEAD `28fd26b76c22b9070eabaa33a85c713876234aaf`. It captures pre-call EDI, but Stage22 logs show the values equal the controller-array pointers (`0x0E9F06DC` / `0x0E9C59B4`), not the decoded action code. The sampling-time fix worked; the register interpretation did not.
- **F6A68 flow remains confirmed statically:** bit17=0 takes `JE F6A9D`; the `F6A9D` and `F6AAC` guards converge at `F6AC8`, so their taken branches do not block the later queue CALL at `F6AD7`. The previous Stage20 source branch remains preserved at [stage20-bit113-object-bit2-guard-trace](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-guard-trace), HEAD `11dde281a08a3481517d0cb60d4752393fa0e578`.
- **Stage23 r2 repeated runtime result (2026-11-09 09:06):** Build succeeded at 229,376 bytes; marker and hook at `0xEE273 -> 0x12EAC0` confirmed. Three readable P1/P2 resolver samples again returned `60 FF 03 1F 12 00 00 05 13 24 03 00`; byte +3 remained selector `0x1F`. All three queue insertions and dispatches were confirmed; classifier AL=0, fallback index 3 and post-AND followed. Slot 3 and controller bit `0x400` remained 0→0. This was a **repeat Stage23 run, not Stage24**: no hook at `0xEE306` was installed. In the separate Stage18/20/17 chain, 8/8 gate/guard records matched; two P2 AL=0 object guards matched setter caller `0xC6502` / index `0x71` at +72 and +70 QPC ticks. `player+0x3B84` was 0 at first entry and `0x20000` at second, but the Stage16 writer log was empty. Stage24 remains the next source task.
- **Stage22 runtime conclusion:** three type-1 / payload-`0x1E` events (two P2, one P1) were accepted and dispatched; classifier AL=0 and fallback ran in all three. Stage22 post-AND observer installed and read the controller flags after the real AND, but all three were `0 → 0`; slot 3 was also zero before and after all three clears. No nonzero-to-zero transition captured.
- **New Stage18/20/17 correlation:** all 11 Stage18 gate records match Stage20 object-guard and Stage17 setter records one-to-one by side, player, thread, tick and parent return. Gate AL=1/key 0x19; object DWORD 0x640 / bit2 clear / AL=0; direct setter index 0x71 followed 71–96 QPC ticks later. See Stage22 runtime analysis.
- Stage16 writer and Stage19 state-map files in the Stage22 upload are zero bytes. The repeated Stage17 entry snapshots show `player+0x3B84` moving from 0 at the first call to `0x20000` at the second in five episodes, but the exact write instruction is not captured in this run.
- Stage20 runtime conclusion (2026-11-08 19:58): 33/33 correlated Stage18→Stage20 P2 records had helper AL=1 and `[object+0x3B70] & 4 != 0`; Stage17 setter calls=0 and bit17 transitions=0. Separately, three F6AD7 calls with payload 0x1E were accepted by the CPU queues and observed in dispatch while bit17=0. The monitored FinalCommand callsite F7C5D recorded no payload 0x1E; this is a scope-limited no-hit, not proof the event was unhandled.
- Stage17 callsite implementation in that branch hooks the direct CALL at `AALib.exe+0xC6502`, expected to pass bit index `0x71` to the generic setter at `0xFAA0`. This is a different implementation snapshot from the earlier `stage17-bit113-setter-caller-trace` entry-hook branch; keep both branches intact.
- Stage19 source snapshot: [stage19-bit113-state-map-trace](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-trace), HEAD 841142895a976419a9bda197be286e7434993067. Stage19 evidence is in the companion runtime branch above. Stage20 branches directly from this Stage19 source snapshot, not from its evidence branch. Stage18 source and runtime branches remain preserved.
- **Builds succeeded** twice on 2026-11-08. The earlier runtime-test build linked `build\\dinput8.dll` at 190,976 bytes. The corrected r2 diagnostic source then built at 16:32:02 with VS 2026 18.9.1 / MSVC 14.51.36231 x86; all eight translation units compiled and the new DLL linked at 198,144 bytes. Both builds emitted the same non-fatal C4754 warning in `AICommandSiteTrace.cpp` lines 782–785, in the stack-address bounds calculation.
- The earlier 15:40 run remains the Stage16 baseline with 22,739 write records and six bit17 transitions. The new 16:34 Stage17 run contains 20,456 Stage16 write records, 62 watchpoint installations across 10 batches, four bit17 set transitions and four clears, with no warning/error/drop reports in the reviewed writer/producer/queue/dispatch logs.
- In the new 16:34 run, four Stage15 F6A68 events all saw bit17=1 and did not take the JZ; none reached F6A9D or F6AD7. QueueTrace and EventDispatchTrace contain no type-1 / payload-`0x1E` event for this run. This is consistent with the bit17 suppression path; it is not a successful insertion sample.
- **Stage17 caller attribution is now runtime-confirmed for this tested scenario.** The r2 log reports the build marker, all three INI values, and successful installation of the passive hook at RVA `0x000C6502` (return `0x000C6507`, original helper `0x0000FAA0`, 34 suspended threads). It records nine `BIT113_SET_CALL` entries. The four actual Stage16 0→1 changes to P1/P2 `player+0x3B84` each have a preceding call from this exact site with index `0x71`, matching player pointer, and QPC gaps of 367, 520, 396, and 353 ticks. Five further calls were observed while bit17 was already set. This confirms the candidate callsite for all four transitions in this run, not every possible game path. See the 2026-11-08 Stage17 runtime section in `PROJECT_HISTORY.md`.
- Historical Stage12–Stage17 evidence branches remain unchanged. Do not rewrite them.

## Repository workflow

- Treat stage branches as immutable historical snapshots after a test run is completed.
- Create each diagnostic stage as a new branch from the immediately preceding completed stage. Stage16 was branched from the exact Stage15 HEAD above; do not rewrite Stages12–15.
- Before analysis, check the target branch HEAD and compare its files with the last reviewed commit. A newly uploaded archive or README alone is not a new runtime test.
- A completed run should include `Build.log` and the relevant runtime logs. Confirm hook installation from the runtime log; do not infer it from source code alone.
- Compare new logs with the previous stage/run, correlate events by QPC/timestamp and arguments, and distinguish observed facts from hypotheses.
- After each stage, append its findings and commit/branch references to `PROJECT_HISTORY.md`, and update `NEXT_SESSION.md` with the latest checkpoint and next concrete task. Keep this root README current if the active stage changes.

The default `main` branch is the documentation index and aggregate log snapshot. Stage overlays and their exact test snapshots live on their named stage branches.
