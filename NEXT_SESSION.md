# Next session handoff

**Read this file first in a new chat.** Inspect current GitHub branch HEADs and newly uploaded files before asking the user for anything.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- **Latest source candidate (Stage23 r2):** [stage23-resolver-result-trace](https://github.com/Hawky11/1/tree/stage23-resolver-result-trace), HEAD `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`.
- **Latest tested runtime evidence:** [stage23-resolver-result-runtime-evidence](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence), HEAD `8e2d025ca60def5103206250a71929fa5d9329bd`. This branch preserves the exact uploaded logs, the user-pasted Build.log, and `Stage23_Overlay/STAGE23_RUNTIME_ANALYSIS.md`.
- **Latest build:** 2026-11-09 00:39:38, Visual Studio 18 Community / MSVC 14.51.36231 x86. All eight C++ translation units compiled; `build\\dinput8.dll` linked successfully at **229,376 bytes**. The only diagnostic is the non-fatal C4754 warning in `AICommandSiteTrace.cpp:785`.
- The initial Stage23 r2 compile attempt failed at two pointer casts in `AIQueueTrace.cpp`; both were fixed using `reinterpret_cast<std::intptr_t>(resolver)`. The subsequent r2 build succeeded, and the successful DLL was exercised in-game.
- **Stage23 runtime installation:** the log contains `STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r2` and confirms the observer installed at resolver CALL RVA `0x000EE273` -> helper RVA `0x0012EAC0`; `suspended=40`.
- Three type-1 / payload-`0x1E` resolver records were captured, all `recordReadable=1`, all on thread 11360. The resolver args were P2 `(0x17,0x17,0x1E,0x38)` and P1 twice `(0x0A,0x0A,0x1E,0x26)`. All returned the same 12 bytes `60 FF 03 1F 12 00 00 05 13 24 03 00`; DWORDs `(0x1F03FF60,0x05000012,0x00032413)`; fields +2=`0x03`, +3=`0x1F`, +4=`0x12`, +0x0B=`0x00`.
- All three samples correlate one-to-one by side/player/thread/tick/QPC with Stage13 ALT_CALLSITE at `0xF6AD7`, accepted queue insertion at `0xF6ADC`, EventDispatch at `0xEE5CB`, Stage21 classifier/fallback and Stage22 post-AND. Classifier helper `0xE8CF0` returned AL=0; fallback helper `0xF7EB0` ran with index 3; post-AND observer ran.
- Slot 3 was readable and `0 -> 0` before/after all three fallback calls. Controller flags were 0 before the AND and bit `0x400` remained `0 -> 0` for all three. This confirms execution, not a nonzero-to-zero mutation.
- Stage18 and Stage20 each captured 27 records, paired 27/27 by side/player/thread/tick/parent return. Stage18 gate AL=1, key `0x19`, table byte `0xFA` for all 27. Stage20 object helper returned AL=1 in 25; two P1 samples at tick 823152734 had AL=0 with object DWORD `0x58` and bit2 clear. Stage17 then recorded two P1 calls to setter caller RVA `0xC6502`, index `0x71`, 76 and 73 QPC ticks later. First setter snapshot saw player+`0x3B84`=0, second `0x20000`; bit17 likely set between them, but exact write is not independently verified because Stage16 writer log was empty.
- `ArcanaAI_StateMapTrace.log`, `ArcanaAI_PlayerBit17WriteTrace.log`, and `ArcanaAI_ProducerCallTrace.log` were zero bytes. `ArcanaAI_FinalCommandTrace.log` had no payload-`0x1E` at its monitored callsite `0xF7C5D`; that does not contradict the separately observed alternate-callsite queue/dispatch path at `0xF6AD7`. No ERROR/WARNING/DROP/OVERFLOW/FAILED markers were found in nonempty uploaded logs.
- Stage21 `preCallEdi` continues to equal the controller-array pointer, not the resolver byte; retain it as raw register data only. Do not revive the old decoded-action interpretation.

## Static interpretation and next task — Stage24

The Stage23 resolver byte +3 is `0x1F` in all three runtime samples. Static disassembly confirms this byte is passed as argument 2 to helper `0x004F57D0` at CALL RVA `0x000EE306`. The helper treats it as a selector/index into a three-byte-per-index mapping table at base `0x04DDF188`, using address `0x04DDF188 + selector*3`:

- table byte +0: flags;
- table byte +1 and +2: mapped selectors used to set bits in the output structure;
- output pointer: `player+0x3A08`;
- arg2 / selector: resolver byte +3 (0x1F);
- arg3: resolver byte +2 (0x03);
- arg4: resolver byte +4 (0x12).

The helper stores the selector in the output structure's first DWORD and builds the following DWORDs using the mapped bytes. Some flag bits alter initialization and may trigger a secondary helper call. The table lives in writable `.data` and may be initialized at runtime. **Do not infer that selector 0x1F is an attack ID, command ID, or animation ID.** Its exact high-level meaning remains unknown.

### Stage24 implementation target

Create a new source branch from the Stage23 r2 source HEAD `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`; do not modify either Stage23 source or runtime-evidence branch. Add a passive observational wrapper at CALL RVA `0x000EE306` -> helper RVA `0x000F57D0`. Validate the original callsite bytes/context and helper prologue against the same exact AALib.exe fingerprint. The observer should:

1. Copy and replay the four original helper arguments exactly once, preserving caller-owned stack layout and original cleanup.
2. Capture the output pointer and selector/other input arguments; for the three target payload-`0x1E` events, verify selector=0x1F, arg3=3, arg4=0x12.
3. Safely capture the three table bytes at `0x04DDF188 + selector*3` immediately before the helper call.
4. Capture the three output DWORDs at `player+0x3A08` both before and after the original helper executes.
5. Record whether the flag byte selected any additional initialization / secondary helper path. Do not add an intrusive hook unless the static control flow proves it is necessary; first trace the primary helper.
6. Preserve the original returned registers/EFLAGS and caller stack. Use read-only diagnostics only; never write the table or player structure.

Correlate Stage24 output records with Stage23 resolver QPC, Stage21 fallback and Stage22 post-AND for the same side/player/thread/tick. The target is to understand how selector 0x1F populates the player structure, not to alter behavior.

## Preservation rules

- Keep Stage21 r1/r2, Stage22 source/runtime evidence and Stage23 source/runtime evidence immutable.
- A source branch must not be described as tested until both a successful Build.log and runtime marker/hook-installation log exist.
- A completed run should preserve the exact source/INI, Build.log, all uploaded logs and a concise runtime analysis in its stage evidence branch.
- Keep `main` as documentation index and append-only history; update README, NEXT_SESSION and PROJECT_HISTORY after each completed stage.
- Distinguish a confirmed read/call/branch from a semantic interpretation that is still a hypothesis.

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
