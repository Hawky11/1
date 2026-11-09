# Next session handoff

## Current checkpoint — Stage25 prepared, Stage24 follow-up complete

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


**Read this file first in a new chat.** Inspect current GitHub branch HEADs and newly uploaded files before asking the user for anything.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- **Latest tested source:** [stage23-resolver-result-trace](https://github.com/Hawky11/1/tree/stage23-resolver-result-trace), HEAD `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`.
- **Latest runtime evidence:** [stage23-resolver-result-runtime-evidence-run2](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence-run2), HEAD `0e6ed619a4e812687e615b00535ef17e54118b5b`. This branch stores the second Stage23 r2 run's exact 18 runtime logs, pasted Build.log, and `Stage23_Overlay/STAGE23_RUNTIME_ANALYSIS_RUN2.md`. The first Stage23 r2 capture remains preserved at [stage23-resolver-result-runtime-evidence](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence), HEAD `306b006a2230c1226a49ab93b9b8b74722fd1683`.
- **Latest build:** 2026-11-09 09:04:32, Visual Studio 18 Community / MSVC 14.51.36231 x86. All eight C++ translation units compiled; `build\\dinput8.dll` linked successfully at **229,376 bytes**. Only diagnostic: non-fatal C4754 at `AICommandSiteTrace.cpp:785`.
- The first Stage23 r2 compile attempt failed at two pointer casts in `AIQueueTrace.cpp`; r2 fixed both with `reinterpret_cast<std::intptr_t>(resolver)`. The successful r2 binary has been built and tested in two separate captures.
- **Important stage identification:** the new 09:06 upload is still a Stage23 r2 run, **not Stage24**. It contains marker `STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r2`, Stage23 resolver installation at `0x000EE273 -> 0x0012EAC0`, and the header `Stage 12/21/22/23 tracing requested`. There is no Stage24 marker/log and no installation at CALL RVA `0x000EE306`.
- **Stage23 runtime:** three type-1 / payload-`0x1E` resolver records were captured, all readable and all on thread 16252:
  | Side | Tick | Resolver QPC | Args (arg1,arg2,arg3,arg4) | Record pointer |
  |---|---:|---:|---|---|
  | P2 | 1331046 | 13310813095 | `(0x17,0x17,0x1E,0x38)` | `0x05C8DCA0` |
  | P1 | 1332312 | 13323424234 | `(0x0A,0x0A,0x1E,0x26)` | `0x0586F490` |
  | P1 | 1350156 | 13501860595 | `(0x0A,0x0A,0x1E,0x26)` | `0x0586F490` |
- All three resolver records again contain `60 FF 03 1F 12 00 00 05 13 24 03 00`; byte +3 is selector `0x1F`. All correlate to ALT_CALLSITE RVA `0xF6AD7`, accepted insertion at `0xF6ADC`, EventDispatch at `0xEE5CB`, Stage21 classifier/fallback and Stage22 post-AND.
- For each of the three `0x1E` events, classifier helper `0xE8CF0` returned AL=0, fallback helper `0xF7EB0` ran with index 3, and the Stage22 post-AND observer ran. Slot 3 stayed `0 -> 0`; bit `0x400` stayed `0 -> 0`. No nonzero-to-zero fallback clear was captured.
- **New Stage18/20/17 sample:** eight Stage18 gate records and eight Stage20 object-guard records (all P2) correlate sequentially 8/8 by side/player/thread/tick/stack/QPC. Each object check followed its gate by 121–175 QPC ticks. All eight gate results were AL=1, key `0x19`, table byte `0xFA`. First six object helper results were AL=1 with object DWORD `0x4C`; last two were AL=0 with object DWORD `0x58` (bit2 clear), both at tick 1353484.
- Those last two AL=0 guards match two Stage17 setter calls at CALL RVA `0xC6502`, both index `0x71`, same P2 player and same EBP/stack frames. Guard #7 QPC `0x326C2BD8E` → setter #1 `0x326C2BDD6` = 72 ticks; guard #8 `0x326C2C03B` → setter #2 `0x326C2C081` = 70 ticks. The `player+0x3B84` snapshots show 0 at the first pair and `0x20000` at the second pair. This is strong correlation consistent with the first call setting bit17, but the exact store is not directly confirmed: `ArcanaAI_PlayerBit17WriteTrace.log` is empty.
- Empty logs: `ArcanaAI_PlayerBit17WriteTrace.log`, `ArcanaAI_StateMapTrace.log`, `ArcanaAI_ProducerCallTrace.log`. `FinalCommandTrace` has 45 records but no payload-`0x1E` at monitored RVA `0xF7C5D`; this does not contradict the separately confirmed `0xF6AD7` route. No ERROR/WARNING/DROP/OVERFLOW/FAILED markers were found in nonempty uploaded logs.
- Stage21 `preCallEdi` still equals the controller-array pointer, not resolver byte +3. Retain it as raw EDI only.

## Next task — Stage24 resolver consumer observer

Stage23 static analysis and both runtime runs establish resolver byte +3 = `0x1F`. Static disassembly shows this byte is passed as argument 2 to helper `0x004F57D0` at CALL RVA `0x000EE306`. The helper treats it as a selector into a three-byte mapping table at `0x04DDF188 + selector*3`; it builds a three-DWORD output structure at `player+0x3A08`. The three table bytes are one flags byte and two mapped selectors. High-level semantics such as attack/action/command/animation are not established.

### Stage24 implementation target

Create a new source branch from the unchanged Stage23 r2 source HEAD `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`. Do not modify either Stage23 source branch or either Stage23 runtime-evidence branch. Add a passive observational wrapper at CALL RVA `0x000EE306` -> helper RVA `0x000F57D0`, validating exact callsite bytes/context and helper prologue against the same AALib.exe fingerprint.

Capture:
1. The four original helper arguments; expected target values are output pointer `player+0x3A08`, selector `0x1F`, arg3 `0x03`, arg4 `0x12`.
2. The three runtime mapping bytes at `0x04DDF188 + selector*3` immediately before the helper call.
3. The three output DWORDs at `player+0x3A08` immediately before and after the original helper call.
4. Whether table flags select an initialization/secondary-helper path; first inspect the primary helper's control flow and do not add another intrusive hook unless necessary.

Replay the original args exactly once; preserve original returned registers/EFLAGS, caller-owned stack and cleanup. Diagnostics must be read-only; never write the table or output structure. Correlate Stage24 QPC with Stage23 resolver, Stage21 fallback and Stage22 post-AND for the same side/player/thread/tick.

**Make Stage24 unmistakable:** add `STAGE24_RESOLVER_CONSUMER_BUILD=...` and a dedicated `ArcanaAI_Stage24ResolverConsumerTrace.log`. Do not reuse Stage23's marker or filename. A successful compile alone is not runtime evidence; require both Build.log and the Stage24 runtime marker/installation line.

## Preservation rules

- Keep Stage21 r1/r2, Stage22 source/runtime evidence, Stage23 source, and both Stage23 runtime captures immutable.
- A source branch must not be described as tested until a successful Build.log and runtime marker/hook-installation log both exist.
- A completed run must preserve the exact source/INI, Build.log, all uploaded logs and concise runtime analysis in its evidence branch.
- Keep `main` as documentation index and append-only history; update README, NEXT_SESSION and PROJECT_HISTORY after each completed stage.
- Distinguish confirmed calls/reads/branches from semantic interpretations that remain hypotheses.

## Historical build/runtime details

The stage-specific records below are retained for chronology. The current Stage23 resolver-result build/runtime task is in the action list above.

## Latest build result — successful

The user supplied Build.log dated 2026-11-08 16:32:02. VS 2026 18.9.1 / MSVC 14.51.36231 x86 compiled all eight C++ translation units and linked `build\\dinput8.dll`, size **198,144 bytes**. One non-fatal C4754 warning remains in `AICommandSiteTrace.cpp` lines 782–785. Build.log was pasted in chat, not uploaded in the branch tree.

## Stage17 runtime result — caller attribution confirmed for this run

Runtime run: 2026-11-08, initialization at 16:34:05, relevant events 16:34:29–16:34:44.

- Stage17 r2 marker, worker entry, and INI settings are present.
- The passive observer installed successfully at callsite RVA `0x000C6502`, return RVA `0x000C6507`, original helper RVA `0x0000FAA0`; `suspended=34`.
- `ArcanaAI_Bit113SetterTrace.log` contains nine `BIT113_SET_CALL` records; every record uses `index=0x00000071`, the exact call/return RVAs, and the correct P1/P2 pointer.
- Stage16 PlayerBit17WriteTrace contains 20,456 records and eight actual bit17 transitions (four sets and four clears). There are 62 thread-watchpoint installations across 10 batches, with no warning/error/drop/skip lines.
- All four Stage16 0→1 writes at RVA `0x0000FAD2` have a preceding Stage17 call from `0x000C6502` with the same player pointer and bit index. QPC deltas are 367, 520, 396, and 353 ticks. The four corresponding clears again execute `0x000ED19F` (post-EIP `0x000ED1A9`).
- Five additional Stage17 calls occurred while the bit was already set; they did not create extra transitions.
- Four Stage15 F6A68 events all saw bit17=1 and the original JZ was not taken. No F6A9D/F6AAC/F6AD7 call was recorded; QueueTrace and EventDispatchTrace contain no type-1 / payload-`0x1E` insertion/dispatch in this run. This is a suppression-side sample, not a successful queue insertion sample.
- No warning/error/drop reports were found in the reviewed Stage17, Stage16, producer, queue, dispatch, or alternate-callsite logs.

**Conclusion:** the candidate direct caller at `AALib.exe+0xC6502` is runtime-confirmed as the origin of all four bit17 set transitions observed in this run. This does not prove it is the only possible setter path for every game state. Preserve the current evidence branch unchanged.

## Stage18 runtime result — complete for the tested scenario

- Latest runtime evidence branch: [stage18-bit113-gate-runtime-evidence](https://github.com/Hawky11/1/tree/stage18-bit113-gate-runtime-evidence), HEAD 0139152b9eeb778a23ec4f3197c57faff749a78a. This branch combines the user's 2026-11-08 18:00 uploaded runtime logs with the Stage18 overlay and Stage18_Overlay/STAGE18_RUNTIME_ANALYSIS.md.
- Source snapshot branch: [stage18-bit113-gate-condition-trace](https://github.com/Hawky11/1/tree/stage18-bit113-gate-condition-trace), HEAD 520eec4cd216795e8ffbaa8738b3602b5e40d39a. Keep it as the source-stage snapshot.
- Runtime start: 18:00:15. Stage18 marker/config and installation at RVA 0xC64A5 are present; Stage17 installation at RVA 0xC6502 is also confirmed. Both hooks report 44 suspended threads.
- **Build.log was not uploaded.** Runtime markers and installation messages prove the loaded DLL ran both hooks, but compiler/linker diagnostics and final DLL size are unavailable.

### Confirmed Stage18 findings

- Six Stage18 gate records; all returned AL=1 and the safely-read post-return byte was 0xFA. The unique state/table cases were P1 index 0x0E / selector 0x0A, P2 index 0x19 / selector 0x17, and P1 index 0x18 / selector 0x0A. No AL=0 sample was captured.
- Stage17 recorded six calls from 0xC6502, all passing bit index 0x71. Stage16 recorded 15,094 write records and three 0→1 bit17 transitions plus three clears.
- Each episode has two calls into caller function 0x4C63B0. One arrives directly from 0x4967FC (return 0x496801); the other is reached via indirect CALL EDX at 0x4CB9F8 (return 0x4CB9FA). Static table entry 0xA at 0x428AF18 points to 0x4C63B0, explaining the second invocation through callback/function-table dispatch.
- The first setter call in each episode sets bit17; the second repeats the OR when it is already set. QPC deltas are in PROJECT_HISTORY.md and STAGE18_RUNTIME_ANALYSIS.md.
- Three target Stage15 attempts had EDI=0x1E, bit17=1, ZF=1 and branchTaken=0. No F6A9D/F6AAC or F6AD7 invocation followed. QueueTrace/EventDispatchTrace/FinalCommandTrace each show 27 type-1 records for other payloads and none for 0x1E.

## Stage19 runtime result — mapping confirmed (2026-11-08)

Evidence branch: [stage19-bit113-state-map-runtime-evidence](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-runtime-evidence), HEAD `3e75c07c7bdbd0827112f6a488936587a41a1b4b`. Source branch remains [stage19-bit113-state-map-trace](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-trace), HEAD `841142895a976419a9bda197be286e7434993067`. The uploaded logs were placed in root `main`; the evidence branch preserves them with the exact Stage19 overlay and `Stage19_Overlay/STAGE19_RUNTIME_ANALYSIS.md`.

### Installation and limitations

- Run initialized 2026-11-08 at 19:05:37.
- Stage17 hook at RVA `0x000C6502`, Stage18 hook at RVA `0x000C64A5`, and Stage19 hook at RVA `0x000F6C08) all installed successfully; each reported 44 suspended threads.
- Stage19 marker/config show `STAGE19_BIT113_STATE_MAP_BUILD=2026-11-08-r1` and `EnableStage19StateMapTrace=1`.
- **Build.log was not uploaded.** Runtime proves a DLL containing the Stage19 instrumentation loaded and its hook installed, but compiler/linker output and final DLL size are unavailable.

### Confirmed Stage19 mapping results

There are 1,235 `STATE_MAP_CALL` records, all with `readMask=0x0F`, and no Stage19 drop/warning/error.

| Side | Raw code | Helper EAX | WORD flags | Derived state index | Calls |
|---|---:|---:|---:|---:|---:|
| P1 | `0x20` | 0 | `0x0400` | `0x18` | 200 |
| P1 | `0x22` | 0 | `0x0400` | `0x18` | 190 |
| P1 | `0x24` | 0 | `0x0400` | `0x18` | 130 |
| P1 | `0x29` | 0 | `0x0400` | `0x18` | 12 |
| P1 | `0x4A` | 3 | `0x2000` | `0x1B` | 152 |
| P2 | `0x58` | 1 | `0x0802` | `0x19` | 82 |
| P2 | `0x59` | 1 | `0x0982` | `0x19` | 297 |
| P2 | `0x5C` | 1 | `0x0802` | `0x19` | 56 |
| P2 | `0x5E` | 3 | `0x2002` | `0x1B` | 116 |

Helper EAX totals: 532×0, 435×1, 268×3. The observed WORD flags agree with the helper priority checks. QPC correlation with Stage18 confirms raw `0x59→EAX 1→index 0x19`, raw `0x22→0→0x18`, and raw `0x20→0→0x18` in the matching gate calls. Indexes generated by raw codes `0x4A` and `0x5E` did not appear in Stage18's bit-113 gate records in this run; do not assume every map-helper call proceeds through that caller.

### Stage18 / Stage17 / Stage16 / Stage15 correlation

- Stage18 logged 42 gate results: all `AL=1`, all post-return table-byte readbacks `0xFA`. No `AL=0) sample.
- Parent-return attribution: 38 direct-call path entries (`0x00DB6801`) and four callback path entries (`0x00DEB9FA`).
- Stage17 logged eight bit-113 setter calls (two per episode).
- Stage16 logged 17,361 writes, including exactly four bit17 `0→1` writes at RVA `0x0000FAD2) and four clears at RVA `0x000ED1A9).
- Stage15 captured four payload-`0x1E` attempts. Every `F6A68` saw bit17=1 and `branchTaken=0`; no `F6A9D`, `F6AAC), or `F6AD7` invocation followed. QueueTrace, EventDispatchTrace and FinalCommandTrace contain no payload-`0x1E) insertion/dispatch.
- Most important new observation: 31 P2 gate calls at state index `0x19) returned `AL=1) but were not followed by setter calls. Therefore the table gate is not sufficient to guarantee the setter is reached.

No warning/error/drop was found in the reviewed Stage19, Stage18, Stage17, Stage16, Stage15, Queue, Dispatch, FinalCommand or AlternateCall logs.

## Stage20 runtime result — confirmed

Evidence branch: [stage20-bit113-object-bit2-runtime-evidence](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-runtime-evidence). Full analysis: `Stage20_Overlay/STAGE20_RUNTIME_ANALYSIS.md`. Source: [stage20-bit113-object-bit2-guard-trace](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-guard-trace), HEAD `11dde281a08a3481517d0cb60d4752393fa0e578`.

Run began 2026-11-08 19:58:47. Stage20 marker/config were present. The observational wrapper installed at CALL RVA `0x000C64F1`, original helper `0x00095B00`, resume `0x000C64F6`, `suspended=40`; the original TEST/JNE remained untouched.

- 33 Stage20 object-guard records, all P2, matched all 33 Stage18 gate records by side/player/QPC; each Stage20 QPC followed its gate by 129–173 ticks.
- All 33 actual helper results were `AL=1`, with checked `[object+0x3B70]=0x4C` and bit `0x4` set; all read masks `0x3F`.
- Stage18 table gate records were all `AL=1`, key `0x19`; their separate `stateIndex` distribution was 0A (4), 04 (2), 14 (25), 10 (2). Do not conflate table key with `stateIndex`.
- Stage17 passive setter observer installed at `0xC6502` but logged zero `BIT113_SET_CALL` records. Stage16 captured 28,043 writes, all `oldBit17=0` / `newBit17=0`; no bit17 transitions.
- Stage19 map trace captured 2,974 records, but its map index must remain separate from Stage18's table key/stateIndex unless same-event correlation is proven.
- Three alternate callsite invocations at `0xF6AD7` carried payload `0x1E` (P2 once, P1 twice). Each was accepted into its CPU queue as type 1/payload `0x1E`, then observed by EventDispatchTrace.
- In all three alternate episodes bit17 was zero and the F6A68 branch was taken; F6AD7 still followed 620–851 QPC ticks later. Thus bit17=1 is not required for this observed alternate queue/dispatch path.
- FinalCommandTrace logged 60 records but no payload `0x1E` at its monitored callsite `0xF7C5D`; this does not negate the separate queue/dispatch evidence.
- No explicit error/warning/drop/failure records were found in the reviewed logs. The new runtime upload did not include a Build.log file; the successful build transcript was supplied in chat before the test.

### Current task — Stage21 follow-up

Stage21 r1 now confirms the payload-`0x1E` classifier fallback for P1/P2 and the complete P1 object-guard AL=0 → setter index 0x71 → bit17 0→1 write chain, followed by a later bit17 clear. Remaining runtime gaps are a nonzero slot-3 / set controller-bit-0x400 fallback sample and same-event Stage18 gate correlation. The r1 decoded-action field is invalid; use the corrected, unbuilt Stage21 r2 branch for a later capture if that field is required. Preserve r1 evidence and all Stage12–Stage20 historical branches unchanged.

## Historical branch rules

- Do not rewrite Stage12–Stage16 evidence branches.
- Keep `stage17-bit113-setter-caller-trace` and `stage17-bit113-setter-callsite-trace` distinct; they use different Stage17 hook placements.
- Append each completed analysis to `PROJECT_HISTORY.md`, update this handoff, and keep `README.md` as the entry point.
- Distinguish successful compilation from hook installation; only runtime log evidence proves the latter.
