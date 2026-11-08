# Next session handoff

**Read this file first in a new chat.** Inspect current GitHub branch HEADs and uploaded files before asking the user for anything.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- **Latest runtime evidence:** `stage21-payload-1e-consumer-runtime-evidence`, HEAD `fe96faadaf1cea2ab559a5f0e0c2346eca9cc802`. It contains the exact r1 source/INI overlay, user-supplied Build.log, runtime logs and updated `Stage21_Overlay/STAGE21_RUNTIME_ANALYSIS.md`.
- **Tested source:** `stage21-payload-1e-consumer-trace`, HEAD `d210c3540ddd29fff1795ef8da6d3605958d481a`. Build succeeded at 2026-11-08 21:43:30; eight C++ units compiled; DLL size 222,208 bytes; one non-fatal C4754 warning at AICommandSiteTrace.cpp:785. Both Stage21 wrappers installed.
- **Corrected but untested source:** `stage21-payload-1e-consumer-trace-r2`, HEAD `28fd26b76c22b9070eabaa33a85c713876234aaf`. It saves EDI before the classifier helper, logs `decodedActionCodePreCall`, and uses build marker `STAGE21_PAYLOAD_1E_CONSUMER_BUILD=2026-11-08-r2`. It has not been built; r1 remains the exact runtime-tested source.
- Stage21 r1 runtime: P1 and P2 dispatched event type 1 / payload `0x1E`, were accepted by queue and reached dispatch. At classifier CALL `0xEE426`, both real helper returns were `EAX=0 / AL=0`; both fallback calls at `0xEE446` executed. Slot 3 and controller bit `0x400` were already zero in both events, so the fallback's non-idempotent state mutation remains unobserved.
- **Full direct-setter chain confirmed for P1:** object `[player+0x3B70]=0x00010440`, bit2 clear, actual object-helper `AL=0`; Stage17 then recorded setter CALL `0xC6502`, index `0x71`, same player/thread, 79 QPC ticks later. Stage16 recorded actual `player+0x3B84` bit17 set `0→1` at RVA `0xFAD2`, 361 QPC ticks after the setter. A later clear `1→0` occurred at RVA `0xED1A9`, 2,701,840 QPC ticks after the set.
- Stage16 `ArcanaAI_PlayerBit17WriteTrace.log` is **7,565,612 bytes**, contains **14,081** writer records and exactly two bit17 transitions (one P1 set, one P1 clear). Earlier empty-content reads were a retrieval limitation, not an empty file.
- Stage18 observer installed but recorded no gate results; Stage19 had 1,320 state-map calls but none correlated with the object guard/setter at 21:45:03.002. Do not claim same-event Stage18/19 correlation.
- R1's `decodedActionCode` field is invalid because EDI was sampled after helper `0x4E8CF0`; the values equal the controller-array pointers. R2 fixes this by saving pre-call EDI. Preserve the r1 evidence unchanged.
- Stage19 source branch remains `stage19-bit113-state-map-trace`, HEAD `841142895a976419a9bda197be286e7434993067`; Stage18 source branch remains `stage18-bit113-gate-condition-trace`, HEAD `520eec4cd216795e8ffbaa8738b3602b5e40d39a`. Keep all historical branches unchanged.

## Next actions

1. The Stage21 r1 consumer path and full P1 object-guard → setter → bit17 set/clear chain are runtime-confirmed. Do not rebuild r1; use the r2 overlay only for a further run that needs a valid decoded-action field.
2. If testing fallback state mutation further, seek a natural event with nonzero controller-array slot 3 or controller bit `0x400` set before the fallback. Both current P1/P2 events had those values already zero.
3. If a same-event state-table connection is needed, capture a Stage18 gate-result record correlated to the object guard/setter event. The Stage18 observer installed in this run but emitted no gate results.
4. For another consumer capture, build from r2 and preserve the full Build.log. Do not alter the r1 source/evidence snapshots.


## Historical build/runtime details

The stage-specific records below are retained for chronology. The current Stage21 status and next tasks are in the checkpoint and action list at the top of this file.

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
