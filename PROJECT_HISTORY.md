# Project history

Append-only record of the ArcanaAI COM-vs-COM diagnostic investigation. Add a new dated entry after each completed stage; do not rewrite earlier conclusions except to correct a documented factual error.

## Project objective

Trace how the COM-vs-COM AI path produces and queues the type-1 event with candidate payload `0x1E`, and identify why the P1/P2 event sometimes does not reach the alternate queue insertion at `AALib.exe+0xF6AD7`.

The staged instrumentation is intended to be observational: it should preserve original arguments, return values, queue state, branch decisions, and AI behavior.

## Stage branch index

| Stage | Branch | Commit at last recorded check | Scope |
|---|---|---|---|
| 10 | [stage10-diagnostics](https://github.com/Hawky11/1/tree/stage10-diagnostics) | `ee06d43ad63f9806e3fce5ce0c66cfac91210cf1` | Stage10 source archive and baseline traces |
| 11 | [stage11-diagnostics](https://github.com/Hawky11/1/tree/stage11-diagnostics) | `9ae08714655744ceb692494d07ab4eac13a8f8b7` | Stage11 source archive and baseline traces |
| 12 | [stage12-eventdispatch-diagnostics](https://github.com/Hawky11/1/tree/stage12-eventdispatch-diagnostics) | `4552ac45e5d7dd68b592bd82a312ae75e0139851` | Queue and event-dispatch diagnostics |
| 13 | [stage13-alternate-callsite-diagnostics](https://github.com/Hawky11/1/tree/stage13-alternate-callsite-diagnostics) | `19b0e6fc40d9b7626cb2557aa844123eb3419ec1` | Alternate queue callsite at F6AD7 |
| 14 | [stage14-producer-call-diagnostics](https://github.com/Hawky11/1/tree/stage14-producer-call-diagnostics) | `dcd036d1b3cead45f8400fd8545dff41d0d59131` | Parent/helper call context and return values |
| 15 | [stage15-postguard-flow-diagnostics](https://github.com/Hawky11/1/tree/stage15-postguard-flow-diagnostics) | `4d1e2a049c835a991647cb04565d31d330aeac95` | Branch-by-branch flow between F2ED0 and F6AD7 |
| 16 | [stage16-player-bit17-writer-trace](https://github.com/Hawky11/1/tree/stage16-player-bit17-writer-trace) | `58313d8c0f6071bc8099fc5ec04ae04d2974dafd` | Runtime-confirmed bit17 setter/clearer; Stage17 caller verification remains |
| 17 | [stage17-bit113-setter-caller-trace](https://github.com/Hawky11/1/tree/stage17-bit113-setter-caller-trace) | `6b04b0a9ad272aecf8dcb481b5c52edfe5acfe1e` | Source-only setter-entry observer for bit index 0x71; awaiting build/runtime test |
| 17b | [stage17-callsite-init-diagnostics](https://github.com/Hawky11/1/tree/stage17-callsite-init-diagnostics) | `c33317c9b2fa28c339c38f3aaebdcb9a378b73a1` | Diagnostic hardening for the Stage17 direct-callsite observer; build/runtime pending |

Commit dates for these snapshots are 2026-11-08 (GitHub commit metadata, UTC). Verify branch HEADs again before relying on the index.

## Stage 12 — queue and event-dispatch diagnostics

Overlay and rationale: `Stage12_Overlay/README_STAGE12.md` on the Stage12 branch.

- Added detailed type-1 event-dispatch records with QPC/tick and before/after queue snapshots.
- Added queue-side diagnostics for EBP/ESP, saved-frame chain, caller/frame checks, and stack context.
- The dispatcher is still called exactly once; the overlay is diagnostic-only.
- Source README explicitly says the overlay was source-only and required a successful Windows/MSVC x86 build plus runtime verification.

## Stage 13 — alternate queue callsite

Overlay and rationale: `Stage13_Overlay/README_STAGE13.md` on the Stage13/15 branches.

- Added a passive observer at `AALib.exe+0xF6AD7`, expected to be a near CALL to `AALib.exe+0xF2DE0`.
- Captures registers, EFLAGS, return address, original stack arguments, stack/frame memory, and a code window.
- The hook validates the original instruction bytes and decoded CALL target before installation; it restores registers and EFLAGS before tail-jumping.
- The Stage13 README explains that Stage12 runtime traces had shown type-1 insertions from caller return RVA `0xF6ADC`, P1/P2, payload `0x1E`.

## Stage 14 — producer and guard-call diagnostics

Overlay and rationale: `Stage14_Overlay/README_STAGE14.md`.

- Added probes around the parent routine at `AALib.exe+0xF69F0` and helper calls/returns:
  - `F6A14` / `F6A19`: call to / return from `F2F50`.
  - `F6A53` / `F6A58`: call to / return from `F2ED0`.
- The Stage14 README records four observed cases in which F2F50 returned `0x11E`, F2ED0 returned `AL=0`, but only one case proceeded to the alternate queue writer. Stage15 was created to identify the intervening branch.
- Probes validate original bytes/targets and use buffered logging. They are observational and do not alter arguments, payloads, queue state, or helper return values.

## Stage 15 — post-guard branch-flow diagnostics

Overlay and rationale: `Stage15_Overlay/README_STAGE15.md`.

Five probes were added between the F2ED0 result and the alternate insertion:

| Probe | RVA | Observed decision |
|---|---:|---|
| `F6A5D_F2ED0_BRANCH` | `0xF6A5D` | Original JNE after `test al,al` |
| `F6A63_PAYLOAD_BRANCH` | `0xF6A63` | `cmp edi,1Eh; jne F6AC8` |
| `F6A68_PLAYER_BIT17_BRANCH` | `0xF6A68` | Bit 17 of `[player+0x3B84]`; `je F6A9D` |
| `F6A9D_LOCAL_GUARD_BRANCH` | `0xF6A9D` | `cmp [ebp-4],2; jne F6AC8` |
| `F6AAC_SECONDARY_HELPER_BRANCH` | `0xF6AAC` | Secondary-helper return test |

### Stage15 runtime result (latest reviewed run)

Source for this result: the Stage15 runtime-log analysis completed in the previous chat; branch/commit reference above. The current Stage15 branch contains the corresponding `ArcanaAI_ProducerCallTrace.log`, queue, dispatch, alternate-call, final-command, and related logs.

- Installation: all five Stage15 probes reported installed (`5/5`). Stage14 also reported `5/5`.
- Stage15 branch trace: 13 Stage15 records; all had `fieldsMask=0x0FFF`; local variables were readable (`localRead=(1,1)`).
- Four relevant invocations returned `0x11E` from F2F50. In all four F2ED0 returned `AL=0`; `EDI=0x1E` passed the payload comparison.
- The differentiating observation was bit 17 at `[player+0x3B84]`:

| Time | Player | `[player+0x3B84]` | Bit 17 | Reached alternate writer F6AD7 |
|---|---|---:|---:|---|
| 12:16:34.696 | P1 | `0x00020000` | 1 | No |
| 12:16:36.871 | P2 | `0x00020000` | 1 | No |
| 12:16:45.020 | P2 | `0x00000000` | 0 | Yes |
| 12:16:47.433 | P1 | `0x00020000` | 1 | No |

- In the successful P2 case at 12:16:45.020, the ordered trace was F6A5D (AL=0; JNE not taken), F6A63 (EDI=0x1E; mismatch branch not taken), F6A68 (bit17=0; JE F6A9D taken), F6A9D (`localFC=0`; JNE F6AC8 taken), and then F6AD7 with `type=1`, `payload=0x1E`.
- QueueTrace then recorded `ACCEPTED_BY_QUEUE`; the saved queue pair was `(0001,001E)`. EventDispatchTrace received the same arguments and reported `beforeMatches=1`, `afterMatches=1`.
- `F6AAC_SECONDARY_HELPER_BRANCH` had no records in this run. That matches the observed successful route, which passed through the common F6AC8 path to F6AD7 and bypassed F6AAC.
- The absence of payload `0x1E` in `FinalCommandTrace` is not evidence of event loss: that trace watches a different callsite (F7C62), whereas this event was inserted via F6AD7.
- The reviewed sequence numbers were continuous from `seq=1` through `seq=10833`, with no gaps reported in the prior analysis.
- **Build.log was not present in the uploaded Stage15 set.** Runtime installation was observed, but compiler warnings/build output were not independently reviewed.

### Stage15 interpretation and limits

The observed samples strongly associate the alternate insertion with bit17 being clear. In this run, all three sampled invocations with bit17 set bypassed F6AD7, while the one with bit17 clear reached it. This is a four-event observation, not yet proof of the complete game-wide rule or of who controls the flag.

The Stage15 branch itself does not identify the writer(s) of `[player+0x3B84]`. That is the next research target.

## Current hypothesis and next investigation

**Primary lead:** bit 17 of `[player+0x3B84]` gates whether the type-1 / payload-`0x1E` event reaches the alternate queue callsite at `F6AD7`.

Next, identify every code path that sets, clears, or otherwise changes that bit. Start with static disassembly/references to the player field and enumerate candidate writers. Then add a Stage16 diagnostic observer at validated write sites, capturing at minimum:

- player pointer and effective target address;
- old and new `[player+0x3B84]` values (or the affected byte/word and reconstructed DWORD);
- whether bit 17 changed and its old/new state;
- write instruction RVA and caller/return RVA;
- thread ID, QPC/timestamp, registers, and enough call context to distinguish P1 from P2.

Do not guess writer addresses or install speculative hooks. Validate instruction bytes and write semantics first. Keep Stage12–Stage15 branches intact and correlate Stage16 writer events with the existing Stage15 branch and queue traces.


## Stage 16 — player bit-17 writer trace (implementation prepared; runtime pending)

Branch: [stage16-player-bit17-writer-trace](https://github.com/Hawky11/1/tree/stage16-player-bit17-writer-trace). Base: exact Stage15 HEAD `4d1e2a049c835a991647cb04565d31d330aeac95`. Stage16 HEAD at the last documentation update: `590675c3db91ea0b4def838bce91908b7b97c667`.

### Static review

The previously supplied `AALib.zip` was materialized from the Library and its executable fingerprint was verified:

- SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
- PE32 i386, ImageBase `0x00400000`, SizeOfImage `0x0E0EB000`, EntryPoint RVA `0x0020EA6C`.

An Intel-syntax objdump scan found 107 direct references with displacement `+0x3B84`, including 60 write-like RMW instructions. The most relevant explicit clear is at VA `0x004ED19F` / RVA `0x000ED19F`:

```asm
and dword ptr [esi+3B84h], 0xFFF9FFFF
```

It clears bits 17 and 18. The path immediately before it pushes payload `0x1E`, type `1`, and a player subobject pointer into `AALib.exe+0x4F2ED0`; it runs the clear only when the helper returns `AL=0`. This is a static candidate path; runtime evidence must establish that ESI points to the same P1/P2 object addresses and that this path executes during the Stage15 scenario.

No immediate `OR [reg+0x3B84],0x00020000` was found. Two dynamic OR sites were resolved in context: `0x004F7BD4` ORs EAX=1 (bit 0), while `0x005BF48C` and `0x005BF495` OR EAX=0x00800000 (bit 23). Several dynamic AND masks were also decoded as affecting other bits. Full review: `Stage16_Overlay/STATIC_XREF_REVIEW.md` and `Stage16_Overlay/WRITE_XREFS_3B84.txt`.

### Stage16 instrumentation prepared

`Stage16_Overlay/src/AITrace.cpp` extends the already-compiled Stage11 `AITrace.cpp` module; no new project source or Build.bat changes are required.

- Derives each player object from the previously confirmed P1/P2 control DWORD RVAs by subtracting `0x3F2C`, then watches player+ `0x3B84`.
- Uses DR0/DR1 write-only 4-byte data watchpoints, DR7 configuration `0x00DD0005`.
- Requires DR0 and DR1 address registers to be zero and all DR0/DR1 enable and RW/LEN bits to be clear (`DR7 & 0x00FF000F == 0`) before installing in a game thread. It preserves DR2/DR3, excludes the COMvsCOM worker thread, and does not overwrite configured DR0/DR1 slots.
- Refuses to run this mode together with the legacy `EnableControlFieldTrace` mode, since both share DR0/DR1. Supplied Stage16 INI sets the legacy key to 0 and `EnablePlayerBit17WriteTrace=1`.
- Records writes from any user-mode module in the target process, not only AALib.exe. Logs absolute post-write EIP, `inAALib=0/1`, AALib RVA when applicable, side, TID, local time, tick, QPC, old/new DWORD (old as a best-effort shadow), bit17 status, registers, EFLAGS, DR6, and 24 code bytes around EIP. A worker flushes a 4096-record buffer to `ArcanaAI_PlayerBit17WriteTrace.log`.
- Logs skipped threads and dropped records. If a thread is skipped or a write is missed before a watchpoint is installed, the old-value shadow may be stale; the new value and post-write code context remain the primary evidence.

### Current status / next operation

Stage16 is **source-only**. It has not been compiled with MSVC and has not been installed or runtime-tested here. No Stage16 `Build.log` or runtime log has been uploaded. Do not report Stage16 as successful until the user supplies the build and runtime evidence.

Next: follow `Stage16_Overlay/README_STAGE16.md` to assemble the Stage11 base plus Stage12–Stage16 overlays, build with MSVC x86, run a short COM-vs-COM session, close normally, and upload the fresh logs. Compare each write event to Stage15 F6A68/F6A9D/F6AD7, QueueTrace, and EventDispatchTrace. Do not infer the flag setter from the static candidate alone.


## Stage 16 runtime results — 2026-11-08

Run window: `13:02:27.719`–`13:03:18.651`. Runtime analysis: [Stage16_Overlay/RUNTIME_ANALYSIS.md](https://github.com/Hawky11/1/blob/stage16-player-bit17-writer-trace/Stage16_Overlay/RUNTIME_ANALYSIS.md).

The Stage16 player-field log contains 30,114 `PLAYER_BIT17_WRITE` records. It starts with exact P1/P2 player and field addresses, initial DWORD values, and `DR7=0x00DD0005`. Eight installation batches report 60 thread watchpoint installations; there are no skipped-thread, dropped-record, or warning/error entries in the Stage16 log. The other Stage15/12 queue, dispatcher, producer and alternate-callsite logs are also present.

### Confirmed setter

The five observed 0→1 transitions all execute the same generic bitfield setter:
- Instruction at VA `0x0040FAD0` / RVA `0x0000FAD0`: `or dword ptr [eax],edx`.
- In each bit17 transition, EAX equals the exact watched P1/P2 field address and EDX=`0x00020000`.
- The instruction belongs to helper VA `0x0040FAA0` / RVA `0x0000FAA0`, which accepts a bit index and uses player bitfield base `+0x3B78`.
- Index `0x71` (113) maps to DWORD offset `+0x3B84`, bit remainder 17, mask `0x00020000`.
- The helper log also records 75 writes with EDX=`0x00400000` (bit 22), which do not set bit17.

A static search of 129 direct references to the generic helper found one literal-index caller at VA `0x004C6502`. It performs a generic test of bit index 2 at `0x00495B00`; when AL=0 it calls the bitfield helper with index `0x71` and player in ESI. This is a strong static candidate, but Stage16 did not record the helper's caller return address, so this callsite is not yet runtime-proven.

### Confirmed clearer

All five observed 1→0 transitions came from VA `0x004ED19F` / RVA `0x000ED19F`:

```asm
and dword ptr [esi+3B84h], 0xFFF9FFFF
```

For each transition ESI was the exact watched P1/P2 player object, and the preceding `0x004F2ED0` helper returned AL=0. The mask clears bits 17 and 18. The write watchpoint saw 597 executions at the clearer, but only five changed bit17 from 1 to 0.

### Correlation with the event path

Six matching Stage15 parent events were observed in this run. For all six, F2F50 returned EAX=`0x11E`, F2ED0 returned AL=0, and EDI=`0x1E` passed the payload compare.

Five events had bit17 set when F6A68 ran. In each case the generic setter set bit17 before the branch, F6A68's original JZ was not taken, and the event did not reach F6A9D/F6AD7. The clearer subsequently reset bit17.

The sixth event (P1 at `13:03:12.258`) arrived at F6A68 with bit17=0; the JZ was taken, F6A9D was reached, and F6AD7 inserted `type=1,payload=0x1E`. QueueTrace reported `ACCEPTED_BY_QUEUE`; EventDispatchTrace confirmed the same pair. FinalCommandTrace correctly has no matching payload-`0x1E` record because that event used the alternate writer at F6AD7, not F7C5D.

This run therefore confirms the bit17 set/clear instructions and strongly correlates the temporary flag with suppression at F6A68. The sample is six parent events; do not overgeneralize it to all matches or all AI events.

### Build-log limitation and next target

No `Build.log` was uploaded. Runtime logs prove that the module was active and its watchpoints installed, but compiler output and build warnings were not independently reviewed.

Next diagnostic question: which runtime caller supplies bit index `0x71` to `0x0040FAA0`? Stage17 should observe entry to the generic setter, record its caller return address, ECX/player, stack bit-index argument and relevant surrounding state, and verify or reject the static candidate at `0x004C6502`. Preserve the Stage16 and Stage12–Stage15 branches as evidence snapshots.


## Stage 17 — bit-113 setter caller trace (source prepared; runtime pending)

Branch: [stage17-bit113-setter-caller-trace](https://github.com/Hawky11/1/tree/stage17-bit113-setter-caller-trace), created from the Stage16 runtime-evidence branch at HEAD `58313d8c0f6071bc8099fc5ec04ae04d2974dafd`. Stage17 HEAD at this documentation update: `6b04b0a9ad272aecf8dcb481b5c52edfe5acfe1e`.

### Why Stage17 is needed

Stage16 confirmed that all five observed 0→1 changes to player bit17 executed the final OR at VA `0x0040FAD0` with EAX equal to the exact watched P1/P2 `player+0x3B84` field and EDX=`0x00020000`. The instruction belongs to a generic bitfield setter at VA `0x0040FAA0` / RVA `0x0000FAA0`; its input bit index `0x71` (113) maps to player+0x3B84, bit 17.

The strongest static caller candidate is VA `0x004C6502` / RVA `0x000C6502`. It performs a helper test and, on AL=0, pushes `0x71`, moves the player into ECX, and calls `0x0040FAA0`. The expected return address is VA `0x004C6507` / RVA `0x000C6507`. The call-site evidence is documented in `Stage17_Overlay/STATIC_CALLER_REVIEW.md`. This candidate was not runtime-proven by Stage16 because Stage16 did not capture the setter's caller return address.

### Prepared instrumentation

`Stage17_Overlay/src/AICommandSiteTrace.cpp` adds a narrowly filtered entry hook to the existing Stage14/15 probe module:

- Validates the exact seven-byte prologue at RVA `0xFAA0`: `55 8B EC 56 8B 75 08`. Refuses to patch if bytes differ.
- At entry, PUSHFD occurs before checking the bit-index argument. Only calls with stack argument `0x71` invoke probe 11.
- Preserves EFLAGS/registers, replays the original seven-byte prologue and continues at RVA `0xFAA7`.
- Interprets ECX as the player object; records player fields, CPU control, registers, EFLAGS/ZF, caller frame, original return address and bit-index argument.
- Reads the preceding five bytes as the caller CALL, decodes the E8 rel32 destination and emits `callTargetVerified=1` only when the destination equals the verified setter entry.
- Installs the setter hook before enabling the Stage14/15 parent-path probes to reduce the chance of missing the first relevant call.
- Reuses the existing bounded Stage14 probe ring; Stage17 entries route to `ArcanaAI_Bit113SetterTrace.log`. The worker flushes this ring even when Stage14 itself is disabled and reports ring drops to the Stage17 log.

The candidate should produce call RVA `0x000C6502`, return RVA `0x000C6507`, decoded target `0x0040FAA0`, bit index `0x71`, and entry ZF=1 if the preceding AL=0 branch condition is the one that led to the setter. Do not assume that candidate is the only caller.

### Current status / next operation

Stage17 is **source-only**. It has not been compiled with MSVC, installed, or runtime-tested. The Stage16 runtime logs are still the latest evidence; no Stage17 `Build.log` or setter-caller trace exists yet.

Use `Stage17_Overlay/README_STAGE17.md` for the exact overlay order and install procedure. Build with the existing x86 toolchain, retain `Build.log`, then run a short COM-vs-COM session and upload the fresh `ArcanaAI_Bit113SetterTrace.log` together with Stage16/Stage15 correlation logs. Only claim the static candidate is confirmed if a runtime record reports the expected return RVA and `callTargetVerified=1`.


## Stage 17 build attempt — 2026-11-08 (compile blocked)

The first reported MSVC x86 build used Visual Studio 2026 18.9.1 / MSVC 14.51.36231 and stopped while compiling `src/AITrace.cpp`. It emitted repeated C3861 errors stating that `ReadDwordAtAddress` was not declared, at 14 call sites (reported around source lines 465–484). Compilation did not reach a successful link; no Stage17 DLL should be installed from this attempt.

### Source comparison

The current Stage17 branch's `Stage16_Overlay/src/AITrace.cpp` is byte-for-byte the same Git blob as the Stage16 evidence branch's corresponding file (blob `2a40bfabc2cfe6b3a39ba67ccbf5257deac389a2`). That tracked file contains both the `ReadDwordAtAddress(DWORD, DWORD*)` implementation and its call sites, with the implementation in the same anonymous namespace before the uses. Therefore the pasted diagnostics do not match compilation of the tracked Stage16 overlay as currently published; a stale, partially merged, or otherwise different local `src/AITrace.cpp` is the leading explanation. This is a source comparison, not proof of the exact local mismatch because the local file was not included with the log.

### Corrective source revision

Stage17 was updated without modifying Stage16 or any earlier evidence branch:

- Stage17 branch HEAD: `6b04b0a9ad272aecf8dcb481b5c52edfe5acfe1e`.
- Added an explicit forward declaration for `ReadDwordAtAddress(DWORD, DWORD*)` near the start of the anonymous namespace in `Stage16_Overlay/src/AITrace.cpp`; the existing implementation remains in place.
- Expanded `Stage17_Overlay/README_STAGE17.md` with the exact overlay-copy requirement and a PowerShell check that verifies the local `src/AITrace.cpp` contains both the declaration and implementation.

This is a defensive source fix and build-path clarification, **not a verified build fix**. Stage17 remains uncompiled and untested until a fresh MSVC x86 Build.log shows a successful compile/link. If the same C3861 errors remain after copying the current Stage16 overlay, inspect the exact local `src/AITrace.cpp` rather than changing hook logic blindly.


## Stage 17 callsite build and runtime follow-up — 2026-11-08 (caller log pending)

Evidence branch: [stage17-bit113-setter-callsite-trace](https://github.com/Hawky11/1/tree/stage17-bit113-setter-callsite-trace), HEAD `0c2411187c47e56a6cb675b597eeb8a754f8f8a6`.

This is a separate Stage17 implementation snapshot from the earlier `stage17-bit113-setter-caller-trace` branch. The callsite variant replaces the five-byte CALL at RVA `0x000C6502` with a passive observer that tail-jumps to the original generic bitset setter at RVA `0x0000FAA0`. Its supplied INI enables `EnableStage17Bit113SetterCallTrace=1`. The expected original CALL bytes are `E8 99 95 F4 FF`, resolving to the setter; expected return RVA is `0x000C6507`.

### Build result

The user supplied the complete Build.log in chat. It reports:
- Visual Studio 2026 18.9.1, MSVC 14.51.36231, x86 environment initialized successfully.
- All eight translation units compiled: Proxy.cpp, ComVsCom.cpp, AITrace.cpp, AICommandTrace.cpp, AIQueueTrace.cpp, AICommandSiteTrace.cpp, AIProducerContextTrace.cpp, AIParentFlowTrace.cpp.
- Linking succeeded: `build\\dinput8.dll`, 190,976 bytes.
- One non-fatal warning C4754 in `AICommandSiteTrace.cpp` lines 782–785. It concerns the mixed-width intermediate in the stack snapshot address calculation:
  `static_cast<std::uint64_t>(stackBase) + i * sizeof(DWORD)`, followed by a 32-bit address bound check. The compiler suggests widening the `i * sizeof(DWORD)` subexpression. This warning did not prevent compilation or linking and is not evidence of a Stage17 hook failure.
- The Build.log was shared in conversation but was not included in the uploaded branch tree.

### Runtime run: 2026-11-08, approximately 15:40:27–15:41:09

The new branch contains Stage16/15/14/12 correlation logs from this session:
- `ArcanaAI_PlayerBit17WriteTrace.log`: 22,739 `PLAYER_BIT17_WRITE` records; 60 game-thread watchpoint installations; no warning/error/drop lines in the file.
- `ArcanaAI_ProducerCallTrace.log`: Stage14 and Stage15 probes; Stage15 branch counts are six F6A5D, six F6A63, six F6A68, three F6A9D, and zero F6AAC records.
- `ArcanaAI_AlternateCallTrace.log`: three relevant alternate writer calls at F6AD7.
- `ArcanaAI_QueueTrace.log`: all three corresponding type-1 / payload-`0x1E` inserts report `ACCEPTED_BY_QUEUE`.
- `ArcanaAI_EventDispatchTrace.log`: all three matching type-1 / payload-`0x1E` events are seen by the dispatcher with `beforeMatches=1` and `afterMatches=1`.
- The reviewed PlayerBit17Write, ProducerCall, Queue and EventDispatch logs contain no warning/error/drop reports.

### Reconfirmed writer/clearer sequence

Three actual 0→1 bit17 changes were captured, each immediately associated by QPC with an F6A68 sample for the same player:
| Time | Player | Setter post-EIP / RVA | Setter EAX | Setter EDX | Clear post-EIP / RVA |
|---|---|---|---|---|---|
| 15:40:46.482 | P2 | `0x00D2FAD2 / 0xFAD2` | `0x0E9F0824` | `0x00020000` | `0x00E0D1A9 / 0xED1A9` |
| 15:40:53.020 | P2 | `0x00D2FAD2 / 0xFAD2` | `0x0E9F0824` | `0x00020000` | `0x00E0D1A9 / 0xED1A9` |
| 15:41:01.431 | P1 | `0x00D2FAD2 / 0xFAD2` | `0x0E9C5AFC` | `0x00020000` | `0x00E0D1A9 / 0xED1A9` |

The setter instruction itself remains VA `0x0040FAD0` (`or [eax],edx`); the logged EIP is after it. The clearer instruction itself remains VA `0x004ED19F` (`and [esi+3B84h],0xFFF9FFFF`); the logged EIP is after it. Each clear changed the watched DWORD from `0x00030000` to `0x00010000`, clearing bit17.

### Six event outcomes

All six matching parent events had F2F50 return EAX=`0x11E`, F2ED0 return AL=0, and EDI=`0x1E` at the relevant payload/branch check.

| Time | Player | bit17 at F6A68 | Original JZ | Result |
|---|---|---:|---|---|
| 15:40:44.850 | P1 | 0 | Taken | F6AD7; accepted and dispatched |
| 15:40:46.482 | P2 | 1 | Not taken | Suppressed before F6A9D/F6AD7 |
| 15:40:53.020 | P2 | 1 | Not taken | Suppressed before F6A9D/F6AD7 |
| 15:40:53.351 | P1 | 0 | Taken | F6AD7; accepted and dispatched |
| 15:41:00.061 | P2 | 0 | Taken | F6AD7; accepted and dispatched |
| 15:41:01.431 | P1 | 1 | Not taken | Suppressed before F6A9D/F6AD7 |

The three bit17=1 cases each have a matching 0→1 setter write before F6A68 and a 1→0 clear afterward. The three bit17=0 cases each reach the alternate queue and EventDispatch. This is a second independent six-event sample supporting the bit17/F6A68 relationship; it still does not establish the full semantic meaning of bit113.

### Important missing Stage17 evidence

The uploaded branch tree does **not** contain `ArcanaAI_Bit113SetterTrace.log`. The callsite variant writes its own log and should record either:
- an installation line for the passive observer at RVA `0x000C6502`, or
- a validation/patch failure explaining why it refused to install,
followed by `BIT113_SET_CALL` records if the callsite executes.

The expected records expose `callRVA=0x000C6502`, `returnRVA=0x000C6507`, `index=0x00000071`, P1/P2 player object, registers, related object fields, stack and QPC. Without this file, the newly tested runtime still does **not** confirm that the three setter calls came from the static candidate at `0x004C6502`; the candidate remains unproven.

Next operation: obtain/upload only `ArcanaAI_Bit113SetterTrace.log` from the same game/output folder as the other ArcanaAI logs. Do not request another build or match unless the file was never created. If it was not created, check the active game-folder `ArcanaAI.ini` for `EnableStage17Bit113SetterCallTrace=1` and confirm the just-built DLL was installed in the game directory before arranging a targeted rerun. Keep this evidence branch immutable.


## Stage17 callsite missing-log diagnosis — 2026-11-08

The 2026-11-08 15:40:27–15:41:09 run produced the Stage16 write/branch/queue/dispatch logs but did not produce `ArcanaAI_Bit113SetterTrace.log`. The successful Build.log was pasted in chat (eight translation units, linked DLL size 190,976 bytes), but it is not stored in the evidence branch, and the log does not prove which exact `AITrace.cpp` contents were compiled.

Review of the original callsite-variant source at `stage17-bit113-setter-callsite-trace` HEAD `0c2411187c47e56a6cb675b597eeb8a754f8f8a6` found:

- The Stage17 logger itself was present and the supplied Stage17 INI set `EnableStage17Bit113SetterCallTrace=1`.
- However, several early exits in `FieldTraceThread` (conflicting DR0/DR1 modes, missing `AALib.exe`, invalid image/ranges, or failed initial P1/P2 reads) returned before writing to the Stage17-specific log. Therefore, absence of the file did not distinguish a stale DLL/source from an initialization failure.
- `AppendStage17Bit113Log` used a double-escaped `\\r\\n` format sequence, which wrote literal backslash characters instead of normal CRLF separators. This did not explain the missing file, but it was corrected.

A new source-only diagnostic branch was created from that evidence snapshot; the original run branch remains unchanged:

- Branch: [stage17-callsite-init-diagnostics](https://github.com/Hawky11/1/tree/stage17-callsite-init-diagnostics)
- HEAD: `c33317c9b2fa28c339c38f3aaebdcb9a378b73a1`
- Source change commit: `fbf466a151ea327a498b19741cda92787ffff46d`
- README/checklist commit: `c33317c9b2fa28c339c38f3aaebdcb9a378b73a1`

The revised source writes `STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r1`, a worker-entry marker, and the three relevant INI values before validation. It also records Stage17-specific errors on the former early-return paths and fixes CRLF formatting. The README adds a PowerShell source-marker check before build and requires installing the new DLL and matching INI beside the actual game executable.

**No new MSVC build or runtime test has been performed for this revision.** This change is an observability fix, not evidence that the callsite hook installed. Next, apply the diagnostic overlay after Stage16, verify the marker in the assembled local `src\\AITrace.cpp`, rebuild x86, install the new DLL/INI, and launch the game only long enough to confirm the Stage17 marker/config/install result. If the marker appears and reports the hook installed, run a short COM-vs-COM session and compare `BIT113_SET_CALL` QPCs with Stage16 bit17 writes. If no marker file appears while other ArcanaAI logs are written, stop and verify which proxy DLL the game actually loaded; do not repeat a match blindly.
