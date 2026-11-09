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
| 18 | [stage18-bit113-gate-condition-trace](https://github.com/Hawky11/1/tree/stage18-bit113-gate-condition-trace) | `520eec4cd216795e8ffbaa8738b3602b5e40d39a` | Passive observer of the 0x4F2FD0 state-table gate result; source-only, build/runtime pending |
| 18r | [stage18-bit113-gate-runtime-evidence](https://github.com/Hawky11/1/tree/stage18-bit113-gate-runtime-evidence) | 0139152b9eeb778a23ec4f3197c57faff749a78a | Uploaded Stage18/17/16 runtime logs plus the matching Stage18 overlay and runtime analysis; Build.log not uploaded |
| 19 | [stage19-bit113-state-map-trace](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-trace) | `841142895a976419a9bda197be286e7434993067` | State-code conversion observer; runtime evidence preserved separately |
| 19r | [stage19-bit113-state-map-runtime-evidence](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-runtime-evidence) | `3e75c07c7bdbd0827112f6a488936587a41a1b4b` | Stage19/18/17/16 runtime evidence and matching overlay |
| 20 | [stage20-bit113-object-bit2-guard-trace](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-guard-trace) | `11dde281a08a3481517d0cb60d4752393fa0e578` | Build-verified Stage20 source; real object-bit-2 helper wrapper |
| 20r | [stage20-bit113-object-bit2-runtime-evidence](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-runtime-evidence) | `08125627bfbb588aff8e735168c51a1083cb06e6` | 33 positive object-guard samples and three payload-0x1E dispatch events |
| 21 | [stage21-payload-1e-consumer-trace](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-trace) | `d210c3540ddd29fff1795ef8da6d3605958d481a` | Exact r1 source snapshot used for the successful build and runtime test |
| 21r | [stage21-payload-1e-consumer-runtime-evidence](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-runtime-evidence) | `150a666f6f61e58aa67766010e6af4300a51d901` | Archived Build.log and runtime logs; P1 object-guard AL=0 → setter → bit17 set/clear chain |
| 21r2 | [stage21-payload-1e-consumer-trace-r2](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-trace-r2) | `28fd26b76c22b9070eabaa33a85c713876234aaf` | Source correction captures pre-call EDI; Stage22 runtime shows it is a controller-array pointer, not decoded action code |
| 22 | [stage22-payload-1e-post-and-trace](https://github.com/Hawky11/1/tree/stage22-payload-1e-post-and-trace) | `27b711d4b3ee8079060bfbab8f8ecc22713ac917` | Build-tested post-AND controller bit-0x400 observer; Stage21 r2-derived consumer overlay |
| 23 | [stage23-resolver-result-trace](https://github.com/Hawky11/1/tree/stage23-resolver-result-trace) | `abaf5bb48d0920632f7b9e1d7c0316eeda3c330d` | Source-only resolver-result observer at dispatcher CALL 0xEE273 -> 0x52EAC0; corrected EDI dataflow |
| 22r | [stage22-payload-1e-runtime-evidence](https://github.com/Hawky11/1/tree/stage22-payload-1e-runtime-evidence) | `5b9d1bd83d1324d899f4c5f7f17dcaca136e7453` | Stage22 Build.log and runtime logs; 3 post-AND samples and 11/11 Stage18→20→17 correlations |
| 23r | — | — | Pending: Stage23 build and runtime evidence not yet supplied |

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


## Stage17 diagnostic build error and source correction — 2026-11-08

The user verified the r1 source marker in `D:\\INCUBATOR\\ArcanaAIOverhaul\\src\\AITrace.cpp`, proving the correct diagnostic source had been copied into the assembled project. The subsequent MSVC x86 build stopped in `AITrace.cpp` with 14 C3861 errors: `ReadDwordAtAddress` was called at lines 466–485 before any declaration was visible.

The exact r1 source inspection confirmed:
- first uses were inside `QueueStage17Bit113SetterRecord` (around line 466);
- the actual definition was later in the same anonymous namespace (around line 603);
- the implementation signature is `bool ReadDwordAtAddress(DWORD address, DWORD* value)`.

A minimal source-only correction was committed to `stage17-callsite-init-diagnostics`:
- forward declaration `bool ReadDwordAtAddress(DWORD address, DWORD* value);` inserted immediately before `QueueStage17Bit113SetterRecord`, inside the existing anonymous namespace;
- existing implementation left unchanged;
- no hook logic, callsite bytes, logger behavior, or runtime tracing semantics changed.

Current branch HEAD: `34fbf14975b3e5d1b9dd6bdec0c3d6c8756bc25f` (source fix commit `1da019d543be29df58f158cf9aecc8ecfc7eb85e`; README clarification commit `34fbf14975b3e5d1b9dd6bdec0c3d6c8756bc25f`). Source inspection confirms declaration at line 422 and implementation at line 607, with all earlier calls now following the declaration.

The user's reported build failed before link. Do not install any DLL from this attempt and do not treat it as a Stage17 build. The local build script deletes its project-local `build\\dinput8.dll` before compilation; the previously installed game-folder DLL is not thereby replaced.

**Next:** copy the corrected `Stage17_Overlay/src/AITrace.cpp` from this branch into the assembled project, verify the existing marker again, rerun Build.bat, and provide the new Build.log. Do not run another game test until a successful link. The current corrected source has not yet been compiled in MSVC in this environment.


### Stage17 corrected-source marker revision r2 — 2026-11-08

After inserting the forward declaration, the diagnostic build marker was bumped from `STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r1` to the distinct `STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r2`. This prevents a stale r1 DLL from being mistaken for the corrected r2 source. The Stage17 README's PowerShell check and expected runtime marker were updated accordingly.

Current source branch: `stage17-callsite-init-diagnostics`; latest HEAD after README update: `ebabe7faa0521115425e56d9b6880b76e1a48806`. This revision is still source-only; no MSVC build result is available yet.


## Stage17 diagnostic r2 build succeeded — 2026-11-08 16:32

The user supplied the next MSVC x86 Build.log from `D:\\INCUBATOR\\ArcanaAIOverhaul`, started 2026-11-08 at 16:32:02. The local source marker check had already returned the r1 marker before the compile-fix update; the corrected r2 source has since been verified in GitHub at branch `stage17-callsite-init-diagnostics`, HEAD `ebabe7faa0521115425e56d9b6880b76e1a48806`.

Build results in the supplied log:
- Visual Studio 2026 18.9.1 / MSVC 14.51.36231, x86 environment initialized successfully.
- All eight C++ translation units compiled, including `AITrace.cpp` and the existing Stage14/15 `AICommandSiteTrace.cpp`.
- Link succeeded and created `build\\dinput8.dll`.
- DLL size: **198,144 bytes**.
- The previous 14 C3861 errors for `ReadDwordAtAddress` are gone.
- One non-fatal C4754 warning remains at `AICommandSiteTrace.cpp` lines 782–785, related to mixed-width arithmetic in the stack-snapshot address bound calculation. It did not prevent compile or link.

This is the first successful build of the corrected Stage17 callsite diagnostic revision. It proves compilation/linking, not that the DLL has been installed, loaded, or that the RVA `0xC6502` hook installed.

### Next runtime verification

1. Copy the new 198,144-byte `build\\dinput8.dll` and the matching Stage17 `ArcanaAI.ini` beside the actual game executable / `AALib.exe`, keeping a backup of the prior DLL.
2. Launch the game and inspect `ArcanaAI_Bit113SetterTrace.log` before running another full COM-vs-COM test.
3. Its first lines must contain `STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r2`, `STAGE17_INIT`, and `STAGE17_CONFIG` with `EnableStage17Bit113SetterCallTrace=1`.
4. Inspect the hook installation result. If it reports installation at RVA `0x000C6502`, then conduct a short COM-vs-COM session to obtain `BIT113_SET_CALL` entries and correlate their QPC/player pointers with Stage16 bit17 writes.
5. If other ArcanaAI logs appear but the Stage17 marker does not, stop and determine which DLL/INI pair the game actually loaded. Do not repeat a full test blindly.

The Stage17 callsite candidate remains runtime-unconfirmed until the log proves successful hook installation and matching `BIT113_SET_CALL` records.


## Stage 17 runtime result — direct bit-113 caller confirmed (2026-11-08)

Evidence branch: [stage17-callsite-init-diagnostics](https://github.com/Hawky11/1/tree/stage17-callsite-init-diagnostics), uploaded runtime-log HEAD `dcfd1efbe221438b1f2b3fae800ca76b1fa87e7d`. This contains the corrected r2 source plus the fresh 16:34 runtime logs. The earlier `stage17-bit113-setter-callsite-trace` branch remains an unchanged pre-fix snapshot.

### Build and initialization

The user supplied Build.log from 2026-11-08 16:32:02:
- Visual Studio 2026 18.9.1 / MSVC 14.51.36231, x86.
- All eight C++ translation units compiled.
- Link succeeded; `build\\dinput8.dll` size **198,144 bytes**.
- Non-fatal C4754 remains in `AICommandSiteTrace.cpp` lines 782–785; it did not prevent linking.

Runtime initialization at 16:34:05:
- `STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r2`
- `FieldTraceThread` entered.
- INI values: `EnableControlFieldTrace=0`, `EnablePlayerBit17WriteTrace=1`, `EnableStage17Bit113SetterCallTrace=1`.
- Installed the passive observer at RVA `0x000C6502`, return RVA `0x000C6507`, original helper RVA `0x0000FAA0`; 34 process threads were suspended during the patch.
- No Stage17 error, warning, or ring-drop line was present.

### Direct runtime attribution

`ArcanaAI_Bit113SetterTrace.log` contains nine `BIT113_SET_CALL` records. All report:
- `callRVA=0x000C6502`
- `returnRVA=0x000C6507`
- `index=0x00000071`
- The exact P1/P2 object pointer in both `player` and `ECX/ESI`.

The Stage16 player-field log contains 20,456 `PLAYER_BIT17_WRITE` records and eight actual bit17 transitions: four 0→1 sets and four 1→0 clears. The Stage16 worker installed 62 write-watchpoint instances across 10 installation batches. No warning/error/drop/skip report was present in the reviewed PlayerBit17WriteTrace.

Every one of the four observed 0→1 writes at RVA `0x0000FAD2` has a preceding Stage17 call from the candidate site with the same side and player object. The Stage17 call observed `player+0x3B84=0`; the Stage16 write immediately followed from the original helper and set that field to `0x00020000`.

| Player | Time | Stage17 call seq | Stage17 QPC | Stage16 setter QPC | Delta |
|---|---|---:|---:|---:|---:|
| P2 | 16:34:29.538 | 1 | 7940066152498 | 7940066152865 | 367 |
| P1 | 16:34:30.940 | 3 | 7940080167261 | 7940080167781 | 520 |
| P1 | 16:34:36.957 | 5 | 7940140346097 | 7940140346493 | 396 |
| P2 | 16:34:43.566 | 7 | 7940206436096 | 7940206436449 | 353 |

Thus the static candidate at `AALib.exe+0xC6502` is **runtime-confirmed as the caller that set bit17 in all four observed transitions in this run**. Five additional Stage17 calls (seq 2, 4, 6, 8, 9) saw `player+0x3B84=0x00020000` already set; these are repeated/idempotent calls and did not correspond to additional 0→1 transitions. The callsite is confirmed for the observed scenario, not proven to be the only possible setter path in every game state.

All four corresponding clears were again observed at RVA `0x000ED1A9` (post-EIP for instruction `0x000ED19F`), with the watched DWORD transitioning from `0x00030000` to `0x00010000`.

### Relationship to the event suppression

The new ProducerCallTrace has four Stage15 records at each of:
- `F6A5D_F2ED0_BRANCH`
- `F6A63_PAYLOAD_BRANCH`
- `F6A68_PLAYER_BIT17_BRANCH`

All four F6A68 records saw bit17=1 and recorded `branchTaken=0` (the original JZ was not taken). There were zero F6A9D local-guard records and zero F6AAC secondary-helper records. The alternate-callsite hook at F6AD7 installed successfully but recorded no actual calls. QueueTrace and EventDispatchTrace contain no type-1 / payload-`0x1E` insert/dispatch in this run. The sample therefore confirms the suppression side of the bit17 gate; it does not add a new successful type-1 / payload-`0x1E` insertion sample.

### Caller context and remaining question

At the Stage17 callsite, the captured `otherObject` pointer equals the player pointer, and `other3B70` was `0x650` or `0x640`; bit index 2 is clear in both values, which is consistent with the final `0x495B00` guard allowing the setter call.

The captured low words at player `+0x3A36` were `0x42`, `0x22`, `0x20`, and `0x5E`. These differ from the `0x1A/0x18/0x19` values in the documented straight-line comparison sequence preceding the call. This does not invalidate the runtime callsite match; the hook is installed at the exact validated CALL and the setter transitions follow it. It means the next static/dynamic review should reconstruct which earlier branch paths enter the final block, and whether any helper between that comparison and the call changes these fields. Do not infer the semantic meaning of the state fields yet.

### Conclusion

Stage17 has completed its primary goal: it proves that `0x004C6502` passes bit index 113 to the generic bitset setter and initiates all four captured bit17 set transitions. Together with Stage16's runtime-confirmed setter at `0x0040FAD0`, clearer at `0x004ED19F`, and Stage15's F6A68 branch, the origin and suppression chain are now linked by runtime evidence.

Next investigation should focus on the conditions and branch paths inside caller function RVA `0x000C63B0`, particularly the earlier helper/branch gates and the repeated calls while bit17 is already set. Preserve this runtime branch as an evidence snapshot; put any new instrumentation in a new Stage18 branch.


## Stage 18 source overlay — bit-113 state-table gate result observer (2026-11-08)

Source branch: [stage18-bit113-gate-condition-trace](https://github.com/Hawky11/1/tree/stage18-bit113-gate-condition-trace), latest HEAD \`520eec4cd216795e8ffbaa8738b3602b5e40d39a\`. Stage17 runtime evidence branch \`stage17-callsite-init-diagnostics\` remains unchanged and remains the latest completed runtime checkpoint.

### Static reconstruction

The uploaded AALib.exe was rechecked directly (PE32, ImageBase \`0x00400000\`, SizeOfImage \`0x0E0EB000\`, SHA-256 \`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba\`).

At caller RVA \`0x000C63B0\`, the branch relevant to Stage17's confirmed setter call is:

- RVA \`0x000C64A0\`: CALL to helper RVA \`0x000F2FD0\`.
- The call arguments are pushed as \`push 0x19; push EAX; push ESI\`, where EAX came from \`0x000F6ED0\).
- RVA \`0x000C64A5\`: \`83 C4 0C 84 C0\` = \`add esp,0x0c; test al,al\`.
- RVA \`0x000C64AA\`: \`75 41\` = \`jne 0x000C64ED\`.

The four Stage17 calls recorded low words at player \`+0x3A36\` of \`0x42\`, \`0x22\`, \`0x20\`, and \`0x5E\`, not the direct-branch accepted values \`0x1A\`, \`0x18\`, and \`0x19\`. The disassembly shows that the \`0x4F2FD0\` result is the alternate incoming gate: AL=1 takes the JNE to \`0x4C64ED\`, bypassing the direct state comparisons. Stage17 did not log AL, so this remained a strong branch inference rather than a direct runtime observation.

Helper \`0x004F2FD0\`:
- chooses its table through selector helper \`0x004E6E30\`, player field \`+0x3F20\), and optionally a secondary table;
- reads a byte at \`table + stateIndex*0x1A + 0x19\`;
- returns AL=1 iff that byte is not \`0xFF\`.

The special selector helper \`0x004E6E30\` returns true when all three conditions hold:
1. DWORD \`[player+0x3978] == 0x1D\`;
2. bit 0 of byte \`[player+0x3B86]\` is set (bit 16 of DWORD \`[player+0x3B84]\`);
3. bit 28 of DWORD \`[player+0x3B88]\` is set.

It then selects index \`0x16\); otherwise the selector is DWORD \`[player+0x3F20]\`. The primary pointer array is at VA \`0x004C82518\` / RVA \`0x04882518\`. When bit 25 of DWORD \`[player+0x3B80]\` is set, a nonzero matching secondary pointer from VA \`0x004C82C20\` / RVA \`0x04882C20\` overrides the primary pointer.

### Stage18 source changes

The new overlay is under \`Stage18_Overlay/\`; the previous \`Stage17_Overlay/\` snapshot is preserved unchanged in this branch.

- \`src/AITrace.cpp\` adds a passive observer at RVA \`0x000C64A5\`, immediately after the original \`0x4F2FD0\` helper returns.
- The installer validates the argument push sequence \`6A 19 50 56\`, helper CALL bytes/target \`E8 2B CB 02 00\` → \`0x4F2FD0\`, overwritten bytes \`83 C4 0C 84 C0\`, and following branch \`75 41\`.
- The hook captures helper AL/EAX, player/state-index/key arguments, parent frame, selector-driving fields, table pointers and the calculated table byte.
- It removes the observer CALL return plus the original three stack arguments (\`add esp,0x10\`), recreates \`test al,al\`, and resumes at the untouched original JNE. It does not replace the helper result or change the branch decision.
- Table reads use \`ReadProcessMemory\` and bounded selectors/state indices; the actual helper AL is retained as the authoritative result. The direct comparison route at \`0x4C64B1\` bypasses this observer and remains outside Stage18's coverage.
- INI keeps Stage16 and Stage17 active and enables \`EnableStage18Bit113GateTrace=1\`. The new output is \`ArcanaAI_Bit113GateTrace.log\`.

### Verification status and next action

This is a **source-only diagnostic stage**. The branch contains the source, matching INI, README and static disassembly notes. No MSVC build, DLL link, hook installation or runtime capture has been performed for Stage18. Do not treat source presence as proof of installation.

Next:
1. Apply \`Stage18_Overlay/src/AITrace.cpp\` and \`Stage18_Overlay/ArcanaAI.ini\` over the already assembled Stage17 project.
2. Verify marker \`STAGE18_BIT113_GATE_BUILD=2026-11-08-r1\` in the local source, then build \`Release | Win32\` with MSVC and keep Build.log.
3. Only after a successful link, back up the currently working game-folder DLL and install the new DLL with the matching INI.
4. First confirm the Stage18 marker/config/install line in \`ArcanaAI_Bit113GateTrace.log\`; then run a short COM-vs-COM scenario and collect the Stage18, Stage17 and Stage16 logs plus related ProducerCall/AlternateCall/Queue/Dispatch traces.
5. Correlate Stage18 \`resultAL\` with the derived table byte and Stage17 setter calls/QPCs. Distinguish the helper-gated route from the direct comparison path.

Keep the Stage17 runtime branch immutable. If the Stage18 build fails, fix the source before any runtime test; if the build succeeds but the hook does not install, stop and diagnose from the Stage18-specific log rather than repeating the scenario.


## Stage18 runtime result — state-table gate and payload-0x1E suppression (2026-11-08)

Runtime evidence branch: [stage18-bit113-gate-runtime-evidence](https://github.com/Hawky11/1/tree/stage18-bit113-gate-runtime-evidence), HEAD 0139152b9eeb778a23ec4f3197c57faff749a78a. The earlier source-only branch [stage18-bit113-gate-condition-trace](https://github.com/Hawky11/1/tree/stage18-bit113-gate-condition-trace), HEAD 520eec4cd216795e8ffbaa8738b3602b5e40d39a, is preserved as the source snapshot. The runtime branch contains the uploaded logs and the exact Stage18 overlay. The separate analysis is in Stage18_Overlay/STAGE18_RUNTIME_ANALYSIS.md.

### Startup and hook installation

Runtime started on 2026-11-08 at 18:00:15. The logs show:
- Stage18 marker STAGE18_BIT113_GATE_BUILD=2026-11-08-r1;
- EnableStage18Bit113GateTrace=1, EnableStage17Bit113SetterCallTrace=1, and EnablePlayerBit17WriteTrace=1;
- successful Stage18 hook installation at RVA 0x000C64A5, after the helper CALL at 0x000C64A0 to 0x000F2FD0, with 44 threads suspended;
- successful Stage17 hook installation at RVA 0x000C6502, targeting generic SetBit helper 0x0000FAA0, also with 44 threads suspended.

**Build.log was not included in the uploaded files.** The startup markers and install messages prove the loaded DLL ran both observers, but the exact MSVC compiler/linker output and DLL size are not available for this run.

### Gate result and correlation

Stage18 logged six BIT113_GATE_RESULT records. All six returned AL=1; all six post-return table readbacks were 0xFA (not sentinel 0xFF), with key 0x19. The three unique state/table cases were:

| Player | State index | Selector | Byte address | Readback |
|---|---:|---:|---:|---:|
| P1 | 0x0E | 0x0A | 0x0559DBDD | 0xFA |
| P2 | 0x19 | 0x17 | 0x055A073B | 0xFA |
| P1 | 0x18 | 0x0A | 0x0559DCE1 | 0xFA |

The table byte is reread after the helper returns, so it is corroborative; the helper's actual AL is authoritative. No AL=0 example was captured in this run.

Stage17 contains six setter calls, all from caller RVA 0x000C6502, return RVA 0x000C6507, bit index 0x71, and the matching player object. Stage16 contains 15,094 PLAYER_BIT17_WRITE records and six bit17 transitions: three 0→1 sets at RVA 0x0000FAD2 with EDX=0x00020000, and three clears at RVA 0x000ED1A9.

| Episode | Player | Gate→setter | Setter→bit17 set | Bit17 set→F6A68 | F6A68→clear |
|---:|---|---:|---:|---:|---:|
| 1 | P1 | 136 QPC ticks | 342 | 2,015 | 1,572 |
| 2 | P2 | 135 QPC ticks | 343 | 1,974 | 1,767 |
| 3 | P1 | 169 QPC ticks | 441 | 2,537 | 1,989 |

Each episode also contains a second helper/setter call after bit17 is already set. These later calls are idempotent OR operations, not additional 0→1 transitions.

### Why the setter is called twice per episode

The Stage18 caller-frame records split into two parent return addresses:
- 0x00DB6801 (RVA 0x00096801, static VA 0x00496801) follows a direct CALL to 0x004C63B0 at 0x004967FC.
- 0x00DEB9FA (RVA 0x000CB9FA, static VA 0x004CB9FA) follows an indirect CALL EDX at 0x004CB9F8 inside a dispatcher beginning at 0x004CB9C0.

The dispatcher reads a function pointer from table VA 0x0428AEF0 + index*4. Static data inspection confirms entry index 0x0A at VA 0x0428AF18 contains pointer 0x004C63B0. Therefore, both paths invoke the same caller: once directly and once through callback/function-table slot 10. This explains the paired Stage18 gate and Stage17 setter records.

### Target payload-0x1E suppression

The three Stage15 F6A68 records have EDI=0x0000001E, bit17=1, ZF=1, and branchTaken=0. The target attempts were observed at 18:00:35.343 (P1), 18:00:37.836 (P2), and 18:00:42.083 (P1).

There are no later F6A9D or F6AAC branch records, and the F6AD7 alternate queue callsite hook installed but recorded no invocation. QueueTrace, EventDispatchTrace, and FinalCommandTrace each contain 27 type-1 records for other payloads; none has payload 0x1E.

**Conclusion:** Stage18 confirms that the 0x4F2FD0 state-table gate allowed all three observed target episodes (AL=1, readback 0xFA). Stage17/16 then show bit17 being set, and Stage15 confirms that bit17 is the decisive suppression condition for the target payload-0x1E attempts in this run. No successful payload-0x1E insertion or AL=0 table-gate sample was captured. The semantic meaning of byte 0xFA remains unassigned.

All reviewed Stage18, Stage17 setter, Stage16 writer, Stage15 producer, queue, dispatch, and alternate-callsite logs contain no warning/error/drop report.


## Stage19 source overlay — bit-113 state-code mapping (2026-11-08)

Source branch: [stage19-bit113-state-map-trace](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-trace), current HEAD `841142895a976419a9bda197be286e7434993067`. It was branched from the original Stage18 source snapshot [stage18-bit113-gate-condition-trace](https://github.com/Hawky11/1/tree/stage18-bit113-gate-condition-trace), HEAD `520eec4cd216795e8ffbaa8738b3602b5e40d39a`. The Stage18 runtime-evidence branch remains separate and unchanged.

### New findings from matching AALib.exe

The original `AALib.exe` from the user's Library archive has SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, matching the binary identity used by the Stage18 static analysis.

The three Stage18 table-byte readbacks of `0xFA` already exist in the on-disk PE's initialized `.data` image:

| Selector | State index | Preferred VA | Stage18 runtime VA | PE byte |
|---:|---:|---:|---:|---:|
| `0x0A` | `0x0E` | `0x04C7DBDD` | `0x0559DBDD` | `0xFA` |
| `0x17` | `0x19` | `0x04C8073B` | `0x055A073B` | `0xFA` |
| `0x0A` | `0x18` | `0x04C7DCE1` | `0x0559DCE1` | `0xFA` |

The runtime module base inferred from Stage18's return address is `0x00D20000`, a relocation delta of `+0x00920000` from the preferred `0x00400000` base. The relocation reproduces all three observed runtime addresses exactly. This establishes that the observed initial `0xFA` values are embedded in the PE; it does **not** establish that runtime code can never mutate the writable table.

The primary table pointer array at preferred VA `0x04C82518` maps selector `0x0A` to `0x04C7DA58` and selector `0x17` to `0x04C80498`. Each state row has stride `0x1A`; the gate reads key offset `0x19`.

### State-index conversion path

The wrapper `0x4F6ED0` calls `0x4F6BA0`, which reads `WORD [player+0x3A34]` and the signed raw code `WORD [player+0x3A36]`.

- Raw values `<=0x17` are returned as the state index directly.
- Raw values `0x18..0x1F` are rejected.
- Other mapped values pass through `0x4F02C0`, which computes `rawIndex=rawCode-0x20`, checks it against `DWORD [player+0x1220]`, and reads a WORD from `player+0x2B9E+rawIndex*0x14`.
- Flag priority in the helper is `0x0400→0`, `0x0800→1`, `0x1000→2`, `0x0200→5`, `0x2000→3`, `0x4000→7`, `0x8000→4`, `0x0008→6`; otherwise it returns `0xFFFFFFFF`.
- `0x4F6BA0` maps helper results `0..4` to indexes `0x18..0x1C`, `5→0x1A`, `6→0x1B`, `7→0x1C`, and `0xFFFFFFFF→-1`. The later caller rejects indexes `>=0x1C`.

This explains the Stage18 observations at the conversion level: P1 raw code `0x20` / stateIndex `0x18` implies helper result 0; P2 raw code `0x5C` / stateIndex `0x19` implies helper result 1. Stage19 is designed to capture these values directly.

For key offset `0x19`, the on-disk primary table's valid `0xFF` rows are:

| Selector | `0xFF` state indexes below `0x1C` |
|---:|---|
| P1 selector `0x0A` | `01, 03, 09, 0F, 14, 15, 16, 17` |
| P2 selector `0x17` | `01, 03, 07, 09, 0D, 0F` |

The mapped rows `0x18..0x1B` are all `0xFA` for these selectors. Thus a naturally reached direct state in the listed rows is the clearest current candidate for Stage18 to record actual `AL=0`. Those direct returns bypass Stage19's converter hook, but they remain covered by the Stage18 gate observer. Stage19 must not force-edit table or player state to manufacture such a sample.

### Stage19 source implementation

The new source overlay adds:
- `Stage19_Overlay/src/AITrace.cpp`;
- `Stage19_Overlay/ArcanaAI.ini`;
- `Stage19_Overlay/README_STAGE19.md`;
- `Stage19_Overlay/STATIC_STATE_MAP_ANALYSIS.md`.

It installs a passive inline observer at RVA `0x000F6C08`, immediately after the `0x4F02C0` CALL at `0x000F6C03`. The observer replaces six bytes (`ADD ESP,8; CMP EAX,5`) with CALL+NOP, logs the original helper EAX/raw code/WORD flags/derived index/QPC, accounts for its synthetic CALL return address plus the two original cdecl arguments, replays `CMP EAX,5`, and resumes at the untouched `JNE` at `0x000F6C0E`. It does not use DR0–DR3 and does not change the helper result or branch condition. Output is `ArcanaAI_StateMapTrace.log`.

### Verification status and next action

This is a **source-only** stage. Git comparison confirms that the Stage19 branch is six commits ahead of the Stage18 source snapshot, contains only the four new Stage19 overlay files, and has no changes to the Stage18 source or evidence branch. Stage19 has **not** been compiled with MSVC, linked, installed, or runtime-tested.

Next:
1. Apply Stage19 `AITrace.cpp` and `ArcanaAI.ini` over the assembled Stage18 source.
2. Build `Release | Win32` with MSVC and upload the complete `Build.log`. Do not install if compilation or linking fails.
3. Back up the working DLL, install only after a successful build, and confirm the Stage19 build marker/config plus successful hook installation at RVA `0x000F6C08`.
4. Run a short natural COM-vs-COM session and upload `ArcanaAI_StateMapTrace.log`, `ArcanaAI_Bit113GateTrace.log`, `ArcanaAI_Bit113SetterTrace.log`, `ArcanaAI_PlayerBit17WriteTrace.log`, plus relevant queue/dispatch/producer logs.
5. Verify the known conversions `0x20→0→0x18` and `0x5C→1→0x19`, compare the helper result with the WORD flags, correlate by player pointer/QPC, and check whether Stage18 observes an actual `AL=0` on a valid `0xFF` row.

Preserve Stage18 runtime evidence and all earlier historical branches unchanged.


## Stage19 runtime result — state-code mapping confirmed (2026-11-08)

Runtime evidence branch: [stage19-bit113-state-map-runtime-evidence](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-runtime-evidence), HEAD `3e75c07c7bdbd0827112f6a488936587a41a1b4b`. The user-uploaded log files were initially placed at the root of `main`; this companion evidence branch preserves the complete log set, the exact Stage19 overlay, and `Stage19_Overlay/STAGE19_RUNTIME_ANALYSIS.md`. The Stage19 source branch [stage19-bit113-state-map-trace](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-trace), HEAD `841142895a976419a9bda197be286e7434993067`, remains separate and unchanged.

### Installation and limitations

Run initialization: 2026-11-08 19:05:37.

- Stage17 observer at RVA `0x000C6502): installed, 44 threads suspended.
- Stage18 gate observer at RVA `0x000C64A5): installed, 44 threads suspended.
- Stage19 state-map observer at RVA `0x000F6C08): installed with six-byte patch, 44 threads suspended.
- Stage19 marker/config: `STAGE19_BIT113_STATE_MAP_BUILD=2026-11-08-r1`, `EnableStage19StateMapTrace=1`.

**Build.log was not uploaded.** Runtime marker and hook-install line prove the Stage19 DLL code loaded and executed, but do not provide compiler/linker output or final DLL size.

### Stage19 results

`ArcanaAI_StateMapTrace.log` contains **1,235** `STATE_MAP_CALL` records, all with `readMask=0x0F`, and no Stage19 warning/error/drop record.

| Side | Raw code | Helper EAX | WORD flags | Derived index | Count |
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
| **Total** | | | | | **1,235** |

Helper-result totals: 532×0, 435×1, 268×3. The observed flag values agree with the helper's priority checks. Stage18 QPC correlation confirms:
- 31 P2 raw `0x59) / helper EAX 1 / derived index `0x19) mappings, QPC gaps 67–108 ticks;
- two P1 raw `0x22) / EAX 0 / index `0x18) mappings, gaps 64–70 ticks;
- two P1 raw `0x20) / EAX 0 / index `0x18) mappings, gaps 63–71 ticks.

Mapped index `0x1B) from raw P1 `0x4A` and P2 `0x5E` did not appear in Stage18 gate records in this capture; not all calls to the state mapper necessarily enter the bit-113 caller. Direct raw indexes `<=0x17) bypass the Stage19 conversion helper.

### Stage18–Stage15 correlation

- Stage18: 42 gate results, all `AL=1), all post-return byte reads `0xFA). Still no actual `AL=0) sample. Parent returns: 38 direct path (`0x00DB6801`) and four callback path (`0x00DEB9FA`).
- Stage17: eight setter calls, two in each of four episodes, all from `0x000C6502), index `0x71`.
- Stage16: 17,361 writes; exactly four bit17 sets at RVA `0x0000FAD2` and four clears at RVA `0x000ED1A9`.
- Stage15: four payload-`0x1E` attempts; all saw bit17=1 and `branchTaken=0` at F6A68. No F6A9D/F6AAC/F6AD7 call followed, and no payload-`0x1E` record appeared in Queue, EventDispatch or FinalCommand.
- No diagnostic warning/error/drop reports were found in the reviewed Stage19–Stage15 logs.

**New important observation:** 31 P2 table-gate calls at index `0x19` returned `AL=1` but had no subsequent setter call. Therefore the Stage18 table gate is not a sufficient condition for reaching the bit-113 setter.

## Stage20 source overlay — trace the downstream object bit-2 guard (2026-11-08)

Source branch: [stage20-bit113-object-bit2-guard-trace](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-guard-trace), latest HEAD `c46738caf098fe187fdb2dae16310efc4da13390`. Git comparison with `stage19-bit113-state-map-trace` shows the Stage20 branch is ahead with exactly four new files under `Stage20_Overlay/`; it does not modify the Stage19 source or evidence branches.

### Static finding

The matching `AALib.exe` (SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`) has this post-table-gate code:

```asm
004C64ED  mov ecx,[ebx]
004C64EF  push 2
004C64F1  call 00495B00
004C64F6  test al,al
004C64F8  jne 004C640B
004C64FE  push 71h
004C6500  mov ecx,esi
004C6502  call 0040FAA0
```

Helper `0x00495B00` uses argument 2 to make mask `0x4`, reads `[object+0x3B70]), returns AL=1 if bit 2 is set and AL=0 if clear. The subsequent JNE skips the setter on AL=1. This is the evidence-backed next guard to test: 31 P2 state-index-`0x19` table-allowed calls had no setter, while the four setter episodes had Stage17 `OtherObject3B70` values `0x440`, `0x640`, `0x460`, `0x640` (bit 2 clear). The reason for the 31 gate-only calls remains a hypothesis until the helper's actual AL and checked object are captured.

### Stage20 instrumentation contract

The new overlay wraps only the five-byte CALL at RVA `0x000C64F1). It saves original ECX, duplicates argument 2 and executes the real helper. It logs the actual AL/EAX, original ECX object pointer, EBX slot pointer, `[EBX]), `[object+0x3B70]), bit-2 state, player fields, QPC and EBP parent return. It restores helper registers/EFLAGS, discards the saved ECX via flag-neutral `LEA ESP,[ESP+4]), then `RET 4` consumes the original caller argument and returns to `0x000C64F6`. The original `TEST/JNE` remains untouched. No hardware watchpoints are added. Output: `ArcanaAI_Bit113ObjectGuardTrace.log`.

### Verification status and next action

Stage20 is **source-only**; it has not been built with MSVC or runtime-tested. The source includes startup/config markers, signature checks for the callsite and following TEST/JNE, and a wrapper-specific installation log.

Next:
1. Apply Stage20 `AITrace.cpp` and `ArcanaAI.ini` over the assembled Stage19 source.
2. Build `Release | Win32` with MSVC and upload the complete `Build.log`. Do not install unless compilation and linking both succeed.
3. Back up the working DLL, install only after successful build, and verify Stage20 marker/config plus the wrapper installation at RVA `0x000C64F1).
4. Run a short natural COM-vs-COM test and upload `ArcanaAI_Bit113ObjectGuardTrace.log`, Stage18/17/16/19 logs and Stage15/Queue/Dispatch/FinalCommand logs.
5. Test whether the 31 P2 index-`0x19` gate-only calls show guard AL=1 and bit 2 set, while the four setter episodes show AL=0 and bit 2 clear. Continue to seek a natural Stage18 AL=0 table-sentinel sample.

Preserve Stage19 source/evidence and all Stage12–Stage19 historical branches unchanged.

## Stage20 MSVC build result — successful (2026-11-08 19:55:42)

Source branch: [stage20-bit113-object-bit2-guard-trace](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-guard-trace), build-verified source snapshot HEAD 11dde281a08a3481517d0cb60d4752393fa0e578. The Stage20 overlay is based directly on the immutable Stage19 source branch stage19-bit113-state-map-trace (HEAD 841142895a976419a9bda197be286e7434993067). Stage19 runtime evidence remains separate in stage19-bit113-state-map-runtime-evidence (HEAD 3e75c07c7bdbd0827112f6a488936587a41a1b4b).

The user supplied the build transcript in chat:
- Build started: 2026-11-08 19:55:42,91.
- Visual Studio: 18 Community.
- MSVC toolset: 14.51.36231.
- Architecture: x86 / Win32.
- All eight C++ sources compiled, including AITrace.cpp.
- Link step succeeded and produced build\\dinput8.dll.
- DLL size: **216,576 bytes**.
- One non-fatal C4754 warning remains in AICommandSiteTrace.cpp lines 782–785; the compiler notes an arithmetic comparison branch that cannot execute and suggests a 64-bit cast. The build succeeded despite this warning.

The Build.log was pasted in chat and is not present as a file in the repository. Do not claim to have an archived Build.log.

### Current status

**Stage20 is build-verified, runtime-pending.** The successful build does not prove that the Stage20 wrapper installed in the game process. The next run must first confirm the STAGE20_BIT113_OBJECT_BIT2_GUARD_BUILD=2026-11-08-r1 marker, config value EnableStage20Bit113ObjectGuardTrace=1, and the successful observational wrapper installation at RVA 0x000C64F1, targeting original helper RVA 0x00095B00. Stop before gameplay if validation or installation fails.

After installation is confirmed, run a short natural COM-vs-COM test and upload ArcanaAI_Bit113ObjectGuardTrace.log plus the Stage18/17/16/19 and relevant Stage15/queue/dispatch logs. The target correlation remains: do the 31 P2 state-index-0x19 gate-only calls show object-helper AL=1 and bit 2 set, while the four known setter episodes show AL=0 and bit 2 clear? This remains a hypothesis, not a runtime conclusion.

Preserve all Stage12–Stage19 source and evidence branches unchanged.
## Stage20 runtime result — object guard confirmed; alternate payload path observed (2026-11-08 19:58:47)

Runtime evidence branch: [stage20-bit113-object-bit2-runtime-evidence](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-runtime-evidence). It starts from uploaded runtime snapshot commit `3bd72139fa658bb60e0a3a3a30b63af7cd7519b8` and preserves all logs plus the exact Stage20 overlay. The Stage20 source branch remains [stage20-bit113-object-bit2-guard-trace](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-guard-trace), HEAD `11dde281a08a3481517d0cb60d4752393fa0e578`.

### Stage20 wrapper installation

Runtime log confirms marker `STAGE20_BIT113_OBJECT_BIT2_GUARD_BUILD=2026-11-08-r1`, config `EnableStage20Bit113ObjectGuardTrace=1`, and successful wrapper installation at CALL RVA `0x000C64F1` to original helper RVA `0x00095B00`, resume RVA `0x000C64F6`, with 40 suspended threads. The original TEST/JNE were left untouched.

### Direct object-guard result

Stage20 recorded 33 object-helper calls, all P2. All 33 returned actual helper `AL=1` / `EAX=1`; the checked DWORD `[object+0x3B70]` was `0x0000004C` and bit `0x4` was set in every record. All 33 had `readMask=0x3F`. All matched the 33 Stage18 gate records one-to-one by side, player pointer and QPC, with Stage20 following Stage18 by 129–173 QPC ticks. All Stage18 records returned AL=1 and used table key `0x19`; the separate `stateIndex` values were 0x0A (4), 0x04 (2), 0x14 (25), and 0x10 (2).

Per the statically verified caller, this nonzero helper result makes the original JNE skip the direct setter call at `0x000C6502`. The Stage17 passive observer at that site installed but logged zero setter calls in this run. Stage16 produced 28,043 writer snapshots; every one had `oldBit17=0` and `newBit17=0`, with zero bit17 transitions. This is consistent with the object-guard result. No AL=0 / bit-2-clear control sample was obtained, so the opposite case remains unverified.

Stage19 mapping recorded 2,974 helper calls, including 240 P2 mappings rawCode `0x5C` -> expected index `0x19` (mapResult 1), 570 rawCode `0x60` -> `0x1B` (result 3), 158 rawCode `0x63` -> `0x1C` (result 7), and P2 calls mapping to `0x18`. These are not a direct one-to-one match for the 33 Stage18 table-gate records; keep the mapper's `expectedStateIndex`, Stage18's `stateIndex`, and its table `key` distinct.

### Alternate payload path

Three alternate callsite events at RVA `0x000F6AD7` carried payload `0x1E` (P2 once, P1 twice). The matching queue details show type 1/payload `0x1E` accepted and stored `(0001,001E)`; the matching EventDispatch details show the same type/payload with `beforeMatches=1` and `afterMatches=1`. All three episodes have Stage15 bit17=0 and a taken F6A68 branch; F6AD7 follows the corresponding F6A9D probe by 311–426 QPC ticks and F6A68 by 620–851 ticks, in the same thread/parent frame. This demonstrates that the observed alternate queue/dispatch path runs while bit17=0; it is distinct from the direct setter branch at `0xC6502`.

FinalCommandTrace captured 60 records and no payload `0x1E` at its monitored callsite RVA `0x000F7C5D`. Treat this as a scope-limited no-hit, not proof the dispatched event was unhandled.

No explicit warning/error/drop/failure records were found in the reviewed runtime logs. The user's successful Build.log was pasted in chat before the test and is not present as a file in the runtime upload.

### Next research questions

1. Obtain a natural object-bit-2-clear Stage20 call and correlate actual AL=0 with Stage18 and the direct Stage17 setter/Stage16 bit17 transition.
2. Statically locate the consumer after dispatch of type 1 / payload `0x1E` at caller RVA `0x000EE5CB`; do not assume the `0x000F7C5D` FinalCommand hook observes this path.
3. Resolve why F6A68/F6A9D report taken branches with bit17 zero while F6AD7 is still reached. Avoid assigning branch semantics from probe labels alone.

See `Stage20_Overlay/STAGE20_RUNTIME_ANALYSIS.md` on the evidence branch for detailed counts and correlations. Preserve all Stage12–Stage19 historical branches unchanged.

Documentation sync on `main`: latest runtime evidence branch HEAD `08125627bfbb588aff8e735168c51a1083cb06e6`; current Stage20 source remains `11dde281a08a3481517d0cb60d4752393fa0e578`.


## Stage21 source stage — locate the payload-0x1E consumer (2026-11-08)

Source branch: [stage21-payload-1e-consumer-trace](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-trace), HEAD `d210c3540ddd29fff1795ef8da6d3605958d481a`. It branches from the immutable Stage20 source snapshot `stage20-bit113-object-bit2-guard-trace`, HEAD `11dde281a08a3481517d0cb60d4752393fa0e578`. Stage20 runtime evidence remains separately preserved in `stage20-bit113-object-bit2-runtime-evidence`, HEAD `08125627bfbb588aff8e735168c51a1083cb06e6`.

### Static consumer identification

- Corrected the Stage12 address interpretation: dispatcher entry is RVA `0x000EE200`; `callerRVA=0x000EE5CB` is the return address after the dispatcher CALL.
- In the type-1 branch at `0x004EE200`, the dispatcher calls `0x004E8CF0` at RVA `0x000EE426` with payload in EAX, player in ESI and a literal selector argument 1 on the stack. This selector is distinct from the original dispatch event type in `[dispatcher EBP+8]`.
- The resolver's byte at result-structure offset `+3` is decoded into EDI; `cmp edi,1` at `0xEE3E1` therefore tests decoded action code 1, not original dispatch event type 1. For payload `0x1E`, helper `0x004E8CF0` is statically constrained to return `AL=0`: if the intermediate `0x00525860` lookup returns nonzero, the helper's early exit is AL=0; otherwise `0x1E < 0x20`, but `0x1E <= 0x17` is false at the final `SETBE`.
- The subsequent `JE 0x004EE443` selects the fallback call `0x004F7EB0` at RVA `0x000EE446`. The helper writes zero to `DWORD [controllerArray + index*4 + 4]`; the caller supplies index 3 and pointer `player+0x3A3C`, so the exact slot is `[controllerArray+0x10]`. The next instruction clears bit `0x400` in `[controller+8]`.
- This locates a real consumer-side effect downstream of dispatcher and explains why the unrelated monitored FinalCommand callsite `0xF7C5D` had no payload-`0x1E` record.

### F6A68 / F6A9D / F6AAC control-flow clarification

- At `F6A68`, bit17=0 makes `JE F6A9D` taken; bit17=1 takes the other path and returns at `F6A9C` rather than entering the alternate queue insertion.
- On the bit17-clear path, `F6A9D` checks `[ebp-4] != 2`; its taken branch jumps to `F6AC8`. If it falls through, the secondary helper at `F6AA5` runs.
- At `F6AAC`, the secondary helper's nonzero result also jumps to `F6AC8`; zero allows the extra `0x72` setter sequence before joining at the same `F6AC8`.
- `F6AC8` is a common continuation, not a return. The queue insertion call at `F6AD7` follows it. Therefore the F6A9D and F6AAC taken branches do not prevent queueing payload `0x1E`; they skip only the extra side-effect sequence. This resolves the previously reported “branch taken but still reaches F6AD7” observation by static disassembly.
- The Stage20 evidence bundle did not contain the Stage15 per-probe runtime records needed for a fresh QPC-level reconstruction of each of the three individual branch episodes. The conclusion above is control-flow proof from the matching static binary, not a newly claimed per-event runtime trace.

### Stage21 source overlay

- `Stage21_Overlay/src/AIQueueTrace.cpp` adds two observational CALL wrappers:
  - `0xEE426 -> 0x4E8CF0`: logs real post-call EAX/AL plus original dispatch type/payload, decoded action code and helper selector argument.
  - `0xEE446 -> 0x4F7EB0`: logs array pointer, index, and slot-3 DWORD before/after the original clear, plus controller flags immediately before the following AND instruction.
- Both wrappers duplicate the original cdecl arguments, invoke the original helper exactly once, preserve its result/register effects for the original caller, and leave original caller stack cleanup instructions intact. Install-time validation checks the surrounding instruction signatures and decoded helper destinations; installation uses the shared patch mutex and suspends other process threads.
- `Stage21_Overlay/ArcanaAI.ini` enables `EnableStage21PayloadConsumerTrace=1`. New log: `ArcanaAI_Stage21PayloadConsumerTrace.log`.
- `Stage21_Overlay/STATIC_PAYLOAD_1E_CONSUMER_ANALYSIS.md` contains the full disassembly and trace contract. `Stage21_Overlay/README_STAGE21.md` contains application/build steps.

### Status and next action

Stage21 is **source-only**. No MSVC build, DLL, runtime hook installation or Stage21 runtime evidence is claimed in this commit.

1. Apply the Stage21 `AIQueueTrace.cpp` and INI overlays to the assembled Stage20 project, keeping Stage20 `AITrace.cpp` and all earlier source units intact.
2. Build `Release | Win32` and preserve the complete Build.log. Do not install if compilation/linking fails.
3. After a successful build, confirm the Stage21 marker and both CALL-wrapper installation entries; then capture `ArcanaAI_Stage21PayloadConsumerTrace.log` with the matching Stage12 EventDispatch and Stage20 object-guard logs.
4. Confirm runtime classifier `AL=0` and slot-`3` pre/post clear. Separately continue the natural Stage20 object-bit-2-clear experiment to obtain the missing object-guard `AL=0) sample. Never force the object bit or helper return value.

Stage21 is a source snapshot only; Stage20 source/evidence and all earlier historical branches remain unchanged.


## Stage21 runtime result — consumer fallback confirmed; object-guard negative control captured (2026-11-08 21:43–21:45)

Runtime evidence branch: [stage21-payload-1e-consumer-runtime-evidence](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-runtime-evidence), HEAD `274a58e2b514766ea2ef067c83f3ae321a0908ce`. This branch starts from the uploaded runtime snapshot commit `e7f3faf33c5e37f49896ed4f0375b12c2b853daa` and preserves the runtime logs, the exact Stage21 r1 overlay and the user-supplied Build.log transcript. Full analysis: `Stage21_Overlay/STAGE21_RUNTIME_ANALYSIS.md`.

Tested source branch: [stage21-payload-1e-consumer-trace](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-trace), HEAD `d210c3540ddd29fff1795ef8da6d3605958d481a`. It is the exact source version built and loaded in this run.

### Build and installation

- Build started 2026-11-08 21:43:30,49 using Visual Studio 18 Community / MSVC 14.51.36231 x86.
- All eight C++ translation units compiled and `build\\dinput8.dll` linked successfully at **222,208 bytes**.
- One non-fatal C4754 warning remains in `AICommandSiteTrace.cpp` lines 782–785.
- Stage21 marker `STAGE21_PAYLOAD_1E_CONSUMER_BUILD=2026-11-08-r1` is present.
- Both Stage21 CALL wrappers installed: `0xEE426 -> 0x4E8CF0` and `0xEE446 -> 0x4F7EB0`, with 40 process threads suspended during installation.
- Stage20 object guard at `0xC64F1`, Stage17 setter observer at `0xC6502`, and Stage18 observer at `0xC64A5` also installed.

### Payload-0x1E consumer runtime

Two events were traced, one P1 and one P2, on thread 23660:
- The alternate producer at `0xF6AD7` queued type 1 / payload `0x1E`; QueueTrace reported `ACCEPTED_BY_QUEUE` and stored `(0001,001E)`.
- EventDispatch at `0xEE200` recorded the same arguments with `beforeMatches=1` and `afterMatches=1`.
- Stage21 classifier CALL at `0xEE426` returned real `EAX=0 / AL=0` for both P1 and P2.
- Both then reached fallback CALL `0xEE446` to helper `0x4F7EB0`, index 3.
- The slot DWORD was zero before and after both fallback helper calls. The controller flags read before the subsequent AND were also zero. Thus fallback execution is confirmed, but neither sample proves an actual nonzero→zero slot change or a set bit-`0x400` clear.

### Stage20 object-bit-2 negative control and Stage17 setter

This same run finally recorded a natural P1 object guard with bit 2 clear:
- `[object+0x3B70]=0x00010440), `bit2Set=0), helper argument 2, real `AL=0`, `readMask=0x3F`.
- 79 QPC ticks later, Stage17 recorded the direct setter CALL at `0xC6502`, index `0x71`, same P1 pointer `0x0E9C1F78), same thread 23660, and the same `other3B70=0x00010440`.

This confirms the object helper's negative result is followed by the direct bit-113 setter call in this sample; the original JNE is not taken and execution falls through to `0xC6502`.

### Remaining limits and r1 logger defect

- `ArcanaAI_PlayerBit17WriteTrace.log` is 7,565,612 bytes and contains 14,081 writer records. The blob reinspection correction appended below finds exactly two bit17 transitions: one P1 set at RVA `0xFAD2` and one later P1 clear at RVA `0xED1A9`. The P1 set follows the Stage17 setter by 361 QPC ticks; the full transition data and correlation are recorded in the correction section below.
- Stage18 observer installed but `ArcanaAI_Bit113GateTrace.log` contains no gate result entries. No same-event Stage18 state-table correlation is available for this P1 object-guard sample.
- Stage19 had 1,320 `STATE_MAP_CALL` entries, but none was correlated to the object guard/setter QPC at 21:45:03.002.
- The Stage21 r1 field `decodedActionCode` is invalid: it sampled EDI after helper `0x4E8CF0`; the observed values (`0x0E9C59B4` / `0x0E9F06DC`) equal the controller-array pointers in the following slot-clear records. The actual AL and dispatch type/payload fields are valid.

### Corrected Stage21 r2 source candidate

A separate source branch preserves this correction without rewriting r1: [stage21-payload-1e-consumer-trace-r2](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-trace-r2), HEAD `28fd26b76c22b9070eabaa33a85c713876234aaf`. It saves EDI on the stack before calling the original classifier helper and logs `decodedActionCodePreCall`; it uses marker `STAGE21_PAYLOAD_1E_CONSUMER_BUILD=2026-11-08-r2`. This branch is **not yet built or runtime-tested**. The Stage21 r1 source/evidence branches remain immutable.

### Next research questions

1. Capture a natural fallback where slot 3 is nonzero before `0x4F7EB0`, or controller bit `0x400` is set before the following AND, to confirm an actual state transition.
2. The direct setter → bit17 write chain is confirmed for this P1 episode. Remaining gaps are a natural fallback with nonzero slot 3 or bit `0x400` set before the helper/AND, and same-event Stage18 gate correlation.
3. If another Stage21 payload capture is needed, build from r2 to obtain a valid pre-call decoded action code. Do not rewrite or overwrite the r1 evidence branch.


## Stage21 Stage16 blob reinspection correction — bit17 transitions confirmed (2026-11-08)

This correction supersedes the Stage21 runtime paragraph above that described `ArcanaAI_PlayerBit17WriteTrace.log` as zero bytes. That statement came from a normal file-read result that returned no text for an oversized blob. The file is present in the upload and its Git blob was successfully retrieved and parsed.

- Blob size: **7,565,612 bytes**.
- Lines: **14,089**.
- `PLAYER_BIT17_WRITE` records: **14,081**.
- Exactly **two** bit17 transitions were found in the entire run; both are P1: one set and one later clear.

### Set transition correlated to the object guard and setter

All three records use P1 player `0x0E9C1F78` and thread 23660:
- Stage20 object-guard record: qpc `0x00000764133B9CC6`, `[object+0x3B70]=0x00010440`, bit2 clear, real helper `AL=0`.
- Stage17 setter record: qpc `0x00000764133B9D15`, CALL RVA `0x000C6502`, index `0x71`; gap from guard is **79 QPC ticks**.
- Stage16 set record: sequence 11981, qpc `0x00000764133B9E7E`, RVA `0x0000FAD2`, watched field `player+0x3B84`; old DWORD `0x01000000`, new DWORD `0x01020000`, `oldBit17=0`, `newBit17=1`, `shadowBit17Changed=1`. It occurs **361 QPC ticks after the setter** and **440 ticks after the object guard**.

This confirms the tested P1 path end-to-end: object bit2 clear causes the original helper to return AL=0; the caller reaches the direct bit-113 setter; the player's bit17 is actually set by the write at RVA `0xFAD2`.

### Later clear

Stage16 sequence 12178 records qpc `0x000007641364D88E`, RVA `0x000ED1A9`, old DWORD `0x00020000`, new DWORD `0x00000000`, `oldBit17=1`, `newBit17=0`. This is **2,701,840 QPC ticks after the set** and confirms the later clear path in the same capture.

### Corrected Stage21 r1 conclusions

- The previous statement that Stage16 writer evidence was unavailable is withdrawn.
- The P1 object-bit-2-clear → actual AL=0 → index-0x71 setter → actual bit17 0→1 write chain is now confirmed with QPC-level evidence.
- Stage16 observed exactly one set and one clear transition in this run; the P2 side had no bit17 transition.
- Stage18 still has no gate-result entries, so no same-event Stage18 table-key/state-index correlation has been established.
- Stage21 r1 fallback slot 3 and controller flag 0x400 were already zero before the respective P1/P2 fallback calls; a nonzero-to-zero mutation remains unobserved.
- The r1 `decodedActionCode` value remains invalid because EDI was sampled after the classifier helper. The separately preserved r2 source saves EDI before that helper but is not built or runtime-tested.

Updated detailed analysis: [Stage21 runtime analysis](https://github.com/Hawky11/1/blob/stage21-payload-1e-consumer-runtime-evidence/Stage21_Overlay/STAGE21_RUNTIME_ANALYSIS.md). Updated evidence-branch HEAD: `150a666f6f61e58aa67766010e6af4300a51d901`.


## Stage22 source stage — payload-0x1E controller post-AND trace (2026-11-08)

Source branch: [stage22-payload-1e-post-and-trace](https://github.com/Hawky11/1/tree/stage22-payload-1e-post-and-trace), HEAD `27b711d4b3ee8079060bfbab8f8ecc22713ac917`. It branches from the corrected Stage21 r2 source branch `stage21-payload-1e-consumer-trace-r2`, HEAD `28fd26b76c22b9070eabaa33a85c713876234aaf`. No earlier source or evidence branch was modified.

### Scope

Stage21 r1 captured the controller flags immediately before the following AND, but both P1/P2 samples already had bit `0x400` clear. Stage22 adds a post-instruction observation at dispatcher RVA `0x000EE451`:

```asm
004EE44B mov esi, [esi+4]
004EE44E add esp, 8
004EE451 and dword ptr [esi+8], 0xFFFFFBFF
```

- Expected seven-byte AND encoding: `81 66 08 FF FB FF FF`; the six-byte preceding context is also validated.
- The seven-byte instruction is replaced with a five-byte relative CALL plus two NOPs.
- The wrapper captures the controller DWORD before the operation, executes the exact original AND once, captures the value after, and restores the post-AND EFLAGS and all general registers.
- Installation uses the shared patch mutex, thread suspension, byte revalidation, instruction-cache flush and original page-protection restoration. Any fingerprint/signature mismatch refuses the patch.
- The new log is `ArcanaAI_Stage22PayloadPostAndTrace.log`; records include dispatcher type/payload, controller pointer, side when resolvable, before/after DWORDs and bit-`0x400` values.
- The inherited Stage21 r2 classifier wrapper logs `decodedActionCodePreCall`, fixing the r1 post-helper EDI sampling defect. The Stage21 slot-3 before/after helper capture is retained.

### Follow-up source review

A post-commit manual assembly/stack review corrected two details in the same Stage22 branch (latest HEAD `27b711d4b3ee8079060bfbab8f8ecc22713ac917`):
- After restoring the EFLAGS generated by the original AND, stack cleanup uses flag-neutral `LEA ESP,[ESP+8]`, not `ADD ESP,8`, so the original EFLAGS survive to the original continuation.
- The CALL replacement returns first to `0x000EE456`; its two trailing NOPs at `0xEE456–0xEE457` then continue at `0xEE458`. The static analysis and code comments reflect this.

At the time of this source-stage checkpoint, build/runtime were still pending; the subsequent Stage22 runtime result is appended later in this history and supersedes that interim status.

A further stack/register review (same source branch, current HEAD `27b711d4b3ee8079060bfbab8f8ecc22713ac917`) also saves the incoming EAX before using it to read the pre-AND DWORD, restores that EAX after logging, and adjusts logger offsets for the additional local. The logger reads the pre-AND DWORD from `savedEsp+12` and CALL return address from `savedEsp+16`. This remains source inspection only; no build or runtime result is claimed.

### Status at source-stage commit

At the moment this source branch was first committed, it was source-only and had no build/runtime result. The later Stage22 runtime section below records the successful 225,792-byte build and confirmed runtime installation; it supersedes this interim status. The exact expected instruction bytes remain checked before patching.

### Next test

Apply `Stage22_Overlay/src/AIQueueTrace.cpp` and `Stage22_Overlay/ArcanaAI.ini` to the assembled Stage20/21 project, then build Release | Win32. After successful build, verify both Stage21 r2 and Stage22 markers plus installation at RVA `0x000EE451`. Capture the Stage22 post-AND log and matching Stage21, dispatch, queue, Stage20, Stage17, Stage16, Stage18 and Stage19 logs.

Interpretation: `bit400Before=1 bit400After=0` proves the controller flag actually changed. `0 -> 0` proves execution only. A natural nonzero slot-3 entry is still required to prove the independent slot clear; same-event Stage18 gate correlation also remains open. Preserve all Stage21 historical branches and the Stage21 r1 runtime evidence unchanged.


## Stage22 runtime result — controller AND executed; Stage18/20/17 chain correlated (2026-11-08 23:30–23:32)

Source branch: [stage22-payload-1e-post-and-trace](https://github.com/Hawky11/1/tree/stage22-payload-1e-post-and-trace), HEAD `27b711d4b3ee8079060bfbab8f8ecc22713ac917`.  
Runtime evidence branch: [stage22-payload-1e-runtime-evidence](https://github.com/Hawky11/1/tree/stage22-payload-1e-runtime-evidence), HEAD `5b9d1bd83d1324d899f4c5f7f17dcaca136e7453`. The evidence branch preserves the exact tested source/INI, the user-supplied Build.log, uploaded logs, and `Stage22_Overlay/STAGE22_RUNTIME_ANALYSIS.md`.

### Build and installation

- Build started 2026-11-08 23:30:54 using VS 18 Community / MSVC 14.51.36231 x86.
- All eight C++ units compiled; `build\\dinput8.dll` linked successfully at **225,792 bytes**.
- One non-fatal C4754 warning remains at `AICommandSiteTrace.cpp:785`.
- Stage21 r2 and Stage22 markers are present. Both Stage21 callsite wrappers and the Stage22 observer at RVA `0xEE451` installed successfully.
- Queue, EventDispatch, Stage20 object guard, Stage17 setter and Stage18 gate observers also installed.

### Payload-0x1E consumer path

Three events (P2, P2, P1), all thread 10804, are matched across QueueTrace, EventDispatchTrace, Stage21 consumer and Stage22 post-AND logs:

| Side | Queue QPC | Dispatch-before QPC | Classifier QPC | Fallback QPC | Post-AND QPC |
|---|---:|---:|---:|---:|---:|
| P2 | 8190556018120 | 8190556018144 | 8190556019441 | 8190556022852 | 8190556023516 |
| P2 | 8190773497452 | 8190773497465 | 8190773498477 | 8190773500276 | 8190773500737 |
| P1 | 8190826674362 | 8190826674384 | 8190826675468 | 8190826677753 | 8190826678325 |

Each queue event was accepted, dispatch history matched type 1/payload `0x1E`, classifier helper `0xE8CF0` returned actual `AL=0`, and fallback helper `0xF7EB0` ran with index 3. Stage22 post-AND records follow fallback by 664, 461 and 572 QPC ticks.

### Post-AND controller state

All three post-AND readbacks succeeded. Bit `0x400` was `0 → 0` for P2 twice and P1 once. P1's complete controller DWORD was `0x4` both before and after; P2's was `0`. Stage21 slot-3 DWORD was also `0` before and after all three fallback helper calls.

Conclusion: Stage22 proves the original AND executed and preserves its post-AND observation, but this run did not capture a nonzero-to-zero mutation of either slot 3 or controller bit `0x400`.

### 11/11 Stage18 → Stage20 → Stage17 runtime matches

The Stage18 log has 11 gate-result records; Stage20 has 11 object-guard records; Stage17 has 11 setter-call records. Every indexed triplet matches by side, player pointer, thread 10804, game tick and parent return address. In each triplet:
- Stage18 returned `AL=1`, key `0x19`, state index `0x18` or `0x19`, and table byte `0xFA`.
- Stage20 read object DWORD `0x00000640`, bit 2 clear, and the original object helper returned `AL=0`.
- Stage17 then observed setter CALL RVA `0xC6502`, bit index `0x71`, 71–96 QPC ticks after the object guard (gate-to-guard gaps were 122–178 QPC ticks).

This resolves the previous same-event Stage18 gate-result correlation gap for this guard/setter path. It is not a direct mapping of Stage19 raw codes: `ArcanaAI_StateMapTrace.log` is zero bytes in this run.

Five two-call episodes show Stage17's first entry observing `[player+0x3B84]=0` and the second observing `0x20000`. This is consistent with bit17 becoming set between the calls. However, `ArcanaAI_PlayerBit17WriteTrace.log` is zero bytes, so the exact write instruction/QPC is not independently observed in this run.

### Correction: Stage21 r2 pre-call EDI is not an action code

The Stage21 r2 wrapper successfully captures EDI before helper `0xE8CF0`, but the runtime values prove that EDI has already been repurposed by CALL `0xEE426`:
- P2: decimal 245303004 = `0x0E9F06DC`, identical to the P2 controller-array pointer recorded by the following fallback call.
- P1: decimal 245127604 = `0x0E9C59B4`, identical to the P1 controller-array pointer recorded by the following fallback call.

Therefore `decodedActionCodePreCall` is a misleading field name and must not be interpreted as the decoded action. The timing correction is real; the assumed register dataflow was not. Future action-code instrumentation must capture the resolver result at the actual decode/compare point, or correct the static disassembly from resolver output to CALL `0xEE426`.

### Missing evidence and next task

- `ArcanaAI_PlayerBit17WriteTrace.log`: zero bytes.
- `ArcanaAI_StateMapTrace.log`: zero bytes.
- No natural precondition with slot 3 nonzero or controller bit `0x400` set before the operation was observed.
- Next priority: audit the EDI dataflow from resolver output through `cmp edi,1` at `0xEE3E1` to CALL `0xEE426`; add a passive observer at the true decode/compare location only if action-code identity is still required. Preserve all Stage21/22 evidence branches.



## Stage23 source stage — capture the type-1 resolver result (2026-11-09)

Source branch: [stage23-resolver-result-trace](https://github.com/Hawky11/1/tree/stage23-resolver-result-trace), HEAD `abaf5bb48d0920632f7b9e1d7c0316eeda3c330d`. It branches from the completed Stage22 source. The exact Stage22 source snapshot remains preserved in `Stage22_Overlay/src/AIQueueTrace.cpp` on this branch; the new complete replacement is in `Stage23_Overlay/src/AIQueueTrace.cpp`.

### Corrected static dataflow

The archived `AALib.exe` was extracted from the previously uploaded `AALib.zip`; its SHA-256 exactly matches the analyzed executable. Direct disassembly confirms:

- Type-1 resolver CALL: RVA `0x000EE273` -> helper RVA `0x0012EAC0`.
- The helper returns a pointer to a 12-byte record in EAX. The dispatcher reads bytes `+0x0B`, `+0x04`, `+0x02`, and `+0x03`. Byte `+0x03` is loaded into EDI at `0xEE28D`, then used through the subsequent `MOV EAX,EDI` and helper call at `0xEE306`.
- At `0xEE30B`, `MOV EDI,[EBP+8]` overwrites EDI with the dispatcher's original event-type argument. The callee `0x4EC4F0` preserves EDI. Therefore `CMP EDI,1` at `0xEE3E1` compares dispatcher event type, not the earlier resolver byte.
- In the later type-1/payload-`0x1E` path, `LEA EDI,[ESI+0x3A3C]` at `0xEE3FC` repurposes EDI as the controller-array pointer. This explains why Stage21 r2's pre-call EDI sample was the controller-array pointer.
- The earlier Stage21 static assertion that EDI at `0xEE3E1` still carried the resolver byte is explicitly superseded by this Stage23 binary review.

### Instrumentation

Stage23 wraps the five-byte CALL at `0xEE273`. It makes private copies of the four existing cdecl stack arguments, calls the original resolver exactly once with identical argument values, then records the returned record pointer, all 12 raw bytes, selected byte fields and input arguments. It preserves the original returned EAX, post-call registers and EFLAGS; it restores the private stack area with flag-neutral LEA, returning to the original `0xEE278` continuation with the original caller-owned arguments still in place.

The installer validates:
- the four-byte prefix `52 53 51 50`;
- the exact CALL bytes `E8 48 08 04 00`;
- the seven-byte suffix `0F B6 48 0B 8B 56 04`;
- resolver prologue `55 8B EC 8B 45 10 83 F8`;
- the decoded target equals `0x0012EAC0`;
- PE image size/timestamp.

It uses the shared mutex, suspends other process threads, revalidates the bytes immediately before patching, flushes the instruction cache and restores page protection. The log filters to exact P1/P2 type-1/payload-`0x1E` traffic. Its new output field is deliberately named `resolverByte03`, not an assumed action/attack ID. The inherited Stage21 field has been renamed to raw `preCallEdi` in the Stage23 overlay to avoid repeating the prior semantic mistake.

### Status and next test

**Source-only; not yet compiled or runtime-tested.** No DLL size, hook installation or resolver result is claimed. The expected marker is `STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r1`; the new file is `ArcanaAI_Stage23ResolverResultTrace.log`.

Next: apply the complete Stage23 overlay over the assembled Stage22 project, build Release | Win32, review the full Build.log, then run normally and correlate Stage23 resolver records with Stage21 classifier/fallback and Stage22 post-AND logs. Do not interpret `resolverByte03` as an attack ID without runtime evidence.


## Stage23 r2 compiler correction — resolver pointer casts (2026-11-09 00:33)

The first user-side Stage23 build attempt stopped in `src/AIQueueTrace.cpp`. Visual Studio 2026 Developer Command Prompt 18.9.1 initialized MSVC 14.51.36231 for x86 successfully. Project checks and the compiler/linker path checks passed; `Proxy.cpp`, `ComVsCom.cpp`, `AITrace.cpp`, and `AICommandTrace.cpp` compiled before the build stopped.

MSVC reported two C2440 errors:
- line 1515: `static_cast` could not convert `BYTE*` to `intptr_t`;
- line 1567: the same invalid pointer-to-integer conversion.

No final link occurred, and no Stage23 DLL was produced by this attempt. The cause was the same expression in two resolver-target comparisons: `static_cast<std::intptr_t>(resolver)`. The code already treats the neighboring callsite address as an integer through `reinterpret_cast<std::intptr_t>`, so the two helper-pointer conversions have now been changed to `reinterpret_cast<std::intptr_t>(resolver)`. This changes only the pointer-to-address representation conversion; it does not change the target RVA, signature checks, resolver arguments, wrapper execution, or hook placement.

Corrected source branch: [stage23-resolver-result-trace](https://github.com/Hawky11/1/tree/stage23-resolver-result-trace), current HEAD `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`. The Stage23 build marker is now `STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r2`. The updated source is `Stage23_Overlay/src/AIQueueTrace.cpp`; the Stage23 README and static analysis record the same correction.

**Status:** corrected source committed, rebuild pending. Do not claim successful compilation, DLL size, hook installation, or runtime records until a fresh Release | Win32 Build.log and same-run installation log are reviewed. The next user action is to reapply the Stage23 r2 source and INI over the assembled Stage22 project, rebuild, and upload the complete Build.log.


## Stage23 runtime result — resolver mapping captured (2026-11-09 00:39–00:40)

Source branch: [stage23-resolver-result-trace](https://github.com/Hawky11/1/tree/stage23-resolver-result-trace), HEAD `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`.  
Runtime evidence branch: [stage23-resolver-result-runtime-evidence](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence), HEAD `8e2d025ca60def5103206250a71929fa5d9329bd`. It preserves the exact uploaded run logs, the user-pasted Build.log, and `Stage23_Overlay/STAGE23_RUNTIME_ANALYSIS.md`.

### Build and hook installation

- Build began 2026-11-09 00:39:38,25 with Visual Studio 18 Community / MSVC 14.51.36231 x86.
- All eight translation units compiled: Proxy.cpp, ComVsCom.cpp, AITrace.cpp, AICommandTrace.cpp, AIQueueTrace.cpp, AICommandSiteTrace.cpp, AIProducerContextTrace.cpp and AIParentFlowTrace.cpp.
- `build\\dinput8.dll` linked successfully at **229,376 bytes**.
- Only warning: non-fatal C4754 in `AICommandSiteTrace.cpp:785`, associated with the bounds calculation around line 782. No compile/link errors.
- Runtime initialization was 2026-11-09 00:40:04. The marker `STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r2` is present.
- Stage23 observer installed at resolver CALL RVA `0x000EE273` -> helper RVA `0x0012EAC0`; installation reported 40 suspended threads. Three readable resolver records were captured.

### Resolver output for payload 0x1E

All three records are thread 11360, dispatcher type 1 / payload `0x1E`:

| Side | Tick | Resolver QPC | Record pointer | Args (arg1,arg2,arg3,arg4) |
|---|---:|---:|---|---|
| P2 | 823151515 | 8231589249280 | `0x063FDCA0` | `(0x17,0x17,0x1E,0x38)` |
| P1 | 823158406 | 8231658248588 | `0x05FDF490` | `(0x0A,0x0A,0x1E,0x26)` |
| P1 | 823166531 | 8231739431884 | `0x05FDF490` | `(0x0A,0x0A,0x1E,0x26)` |

Every result was readable and all three contained the same twelve bytes:

`60 FF 03 1F 12 00 00 05 13 24 03 00`

DWORD view: `(0x1F03FF60, 0x05000012, 0x00032413)`. Selected bytes: +2=`0x03`, +3=`0x1F`, +4=`0x12`, +0x0B=`0x00`. The P2 args agree with player fields +0x3978 / +0x3F20 = `0x17`, +0x1224 = `0x38`; the two P1 records agree with corresponding values `0x0A`, `0x0A`, `0x26`. The P1 pointer is reused for both P1 events; P2 has a distinct pointer, but the returned bytes match.

### End-to-end event correlation

All three Stage23 resolver records match Stage13 ALT_CALLSITE, QueueTrace, EventDispatchTrace, Stage21 and Stage22 using side/player/thread/tick/QPC:

| Side | ALT_CALLSITE QPC | Resolver QPC | Classifier QPC | Fallback QPC | Post-AND QPC |
|---|---:|---:|---:|---:|---:|
| P2 | 8231589249194 | 8231589249280 | 8231589253347 | 8231589256114 | 8231589257163 |
| P1 | 8231658248530 | 8231658248588 | 8231658251710 | 8231658252447 | 8231658252837 |
| P1 | 8231739431782 | 8231739431884 | 8231739434850 | 8231739435766 | 8231739436176 |

- ALT_CALLSITE is the alternate producer call at RVA `0xF6AD7`, type 1 / payload `0x1E`.
- QueueTrace logged `ACCEPTED_BY_QUEUE` at return RVA `0xF6ADC`; all three stored type `0x0001` / payload `0x001E`.
- EventDispatch logged all three at caller RVA `0xEE5CB`; queue history matched the dispatch args.
- Stage21 classifier helper `0xE8CF0` returned `AL=0` for all three.
- Stage21 fallback helper `0xF7EB0` ran with index 3 for all three.
- Stage22 post-AND observer at RVA `0xEE451` ran for all three.

Slot 3 was readable and zero before and after all three fallback helper calls. Controller flags before the AND were zero; Stage22 post-AND confirmed bit `0x400` remained `0 -> 0` in all three. The run proves the fallback and AND execute, but does not capture an actual nonzero-to-zero clear.

Stage21's `preCallEdi` values still equal the controller-array pointer (P2 `0x0E9F06DC`, P1 `0x0E9C59B4`); keep that field as raw EDI, not an action-code interpretation.

### Stage18 / Stage20 / Stage17 cross-check

- Stage18 and Stage20 each logged 27 records. Pairing by side, player, thread, tick and parent return produced **27/27** matches; each matched object-guard QPC followed its gate result by 124–204 ticks.
- All 27 Stage18 results returned `AL=1`, key `0x19`, table byte `0xFA`.
- Stage20 object helper returned `AL=1` in 25 records. Two P1 records at tick 823152734 had `AL=0`, object DWORD `0x58`, and bit2 clear.
- Stage17 logged two P1 setter calls at RVA `0xC6502`, index `0x71`, 76 and 73 QPC ticks after the two corresponding AL=0 guard results. First snapshot saw `player+0x3B84=0`; second saw `0x20000`. This is consistent with the bit becoming set between calls, but the exact write is not independently confirmed because the Stage16 writer log is zero bytes.
- StateMapTrace and ProducerCallTrace are also zero bytes. FinalCommandTrace has no payload-`0x1E` record at its monitored RVA `0xF7C5D`; this does not negate the confirmed separate alternate queue/dispatch route.
- No ERROR/WARNING/DROP/OVERFLOW/FAILED markers were found in the nonempty uploaded runtime logs.

### Resolver byte +3: downstream role established, semantic name still open

The runtime establishes byte +3 = `0x1F` in both P1 and P2 samples. Static disassembly of the same AALib binary shows it is passed as arg2 to helper `0x004F57D0` at CALL RVA `0x000EE306`. At that callsite:
- arg1 = pointer to `player+0x3A08`;
- arg2 = resolver byte +3 (selector `0x1F`);
- arg3 = resolver byte +2 (`0x03`);
- arg4 = resolver byte +4 (`0x12`).

The helper uses arg2/ESI as an index into a three-byte-per-entry mapping table at `0x04DDF188 + selector*3`: one flag byte and two mapped selector bytes. It stores the index in the output structure's first DWORD and uses the mapped bytes to set bits in the following DWORDs; flag bits may change initialization and trigger a secondary helper. The table is in writable `.data` and may be initialized at runtime. Therefore the strongest supported current description is **resolver mapping selector/index 0x1F**. Its high-level semantic category (attack, command, animation, etc.) is not established.

### Remaining evidence gaps and next stage

- `ArcanaAI_PlayerBit17WriteTrace.log`, `ArcanaAI_StateMapTrace.log`, and `ArcanaAI_ProducerCallTrace.log` were zero bytes.
- No actual nonzero-to-zero transition of fallback slot 3 or controller bit `0x400` was captured.
- Exact instruction that changes player bit17 is still absent from this run.

Next: Stage24 should add a passive observer at CALL RVA `0x000EE306` -> helper RVA `0x000F57D0`, without changing Stage23. Capture the four incoming args, the three runtime table bytes at `0x04DDF188 + 0x1F*3`, and the three DWORDs at `player+0x3A08` before/after the helper. Confirm whether the flag byte selects a secondary helper path. Preserve original args, return registers/EFLAGS and caller cleanup; never write table/player state.


## Stage23 runtime result — second r2 run (2026-11-09 09:04–09:07)

This is a second run of the existing Stage23 r2 binary, not a Stage24 run. The uploaded marker remains `STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r2`, the installed resolver hook is still at CALL RVA `0x000EE273 -> 0x0012EAC0`, and the queue/dispatch trace header requests Stages 12/21/22/23 only. No Stage24 marker, hook at `0x000EE306`, or Stage24 log is present.

Evidence branch: [stage23-resolver-result-runtime-evidence-run2](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence-run2), HEAD `0e6ed619a4e812687e615b00535ef17e54118b5b`. It preserves all 18 current uploaded runtime logs, the Build.log supplied in chat, and `Stage23_Overlay/STAGE23_RUNTIME_ANALYSIS_RUN2.md`. The first Stage23 capture remains unchanged at [stage23-resolver-result-runtime-evidence](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence), HEAD `306b006a2230c1226a49ab93b9b8b74722fd1683`.

### Build and hook installation

- Build began 2026-11-09 09:04:32,60 with Visual Studio 18 Community / MSVC 14.51.36231 x86.
- All eight translation units compiled; `build\\dinput8.dll` linked successfully at **229,376 bytes**.
- Only diagnostic was the known non-fatal C4754 warning at `AICommandSiteTrace.cpp:785`, associated with the bounds calculation beginning at line 782.
- Runtime initialized at 09:06:20. Stage23 r2 hook installed at `0x000EE273 -> 0x0012EAC0`, reporting 39 suspended threads. Runtime image base was `0x005B0000`.

### Resolver and payload-0x1E path

Three type-1 / payload-`0x1E` resolver records were captured, all readable and all on thread 16252:

| Side | Tick | ALT QPC | Resolver QPC | Δ | Args (arg1,arg2,arg3,arg4) | Record pointer |
|---|---:|---:|---:|---:|---|---|
| P2 | 1331046 | 13310812900 | 13310813095 | 195 | `(0x17,0x17,0x1E,0x38)` | `0x05C8DCA0` |
| P1 | 1332312 | 13323424185 | 13323424234 | 49 | `(0x0A,0x0A,0x1E,0x26)` | `0x0586F490` |
| P1 | 1350156 | 13501860518 | 13501860595 | 77 | `(0x0A,0x0A,0x1E,0x26)` | `0x0586F490` |

All returned the same 12 bytes as the first Stage23 capture: `60 FF 03 1F 12 00 00 05 13 24 03 00` (DWORDs `0x1F03FF60, 0x05000012, 0x00032413`). Byte +2=`0x03`, +3=`0x1F`, +4=`0x12`, +0x0B=`0x00`.

Each record correlates to Stage13 ALT_CALLSITE at RVA `0xF6AD7`, QueueTrace accepted insertion at return RVA `0xF6ADC`, EventDispatch at RVA `0xEE5CB`, Stage21 classifier/fallback and Stage22 post-AND:

| Side | Resolver QPC | Classifier QPC | Fallback QPC | Post-AND QPC |
|---|---:|---:|---:|---:|
| P2 | 13310813095 | 13310818324 | 13310820264 | 13310820882 |
| P1 | 13323424234 | 13323426193 | 13323426682 | 13323427028 |
| P1 | 13501860595 | 13501862845 | 13501863381 | 13501863736 |

All three classifier calls to `0xE8CF0` returned AL=0; fallback `0xF7EB0` ran with index 3; the Stage22 post-AND observer then executed. Slot 3 remained `0 -> 0`, and controller bit `0x400` remained `0 -> 0` in every sample. No actual nonzero-to-zero clear was observed. Stage21 `preCallEdi` remains the controller-array pointer, not the resolver mapping byte.

### Stage18 / Stage20 / Stage17 bit17 setter correlation

- Stage18 and Stage20 each recorded eight P2 entries. Sequential pairing by side/player/thread/tick, stack frame and QPC gives **8/8 matches**; object-guard checks followed their gate entries by 121–175 QPC ticks.
- All eight Stage18 gate results returned AL=1, key `0x19`, table byte `0xFA`.
- First six Stage20 object-guard results returned AL=1 with `object+0x3B70=0x4C`.
- Last two object-guard results returned AL=0 with `object+0x3B70=0x58` (bit2 clear), both at tick `1353484`. Their parent returns are `0x00646801` and `0x0067B9FA`, respectively.
- Stage17 logged exactly two P2 setter calls at CALL RVA `0x000C6502`, index `0x71`, same player `0x0E27CCA0`, with matching EBP/stack frames:
  - guard #7 QPC `0x326C2BD8E` → setter #1 `0x326C2BDD6` (**72 ticks**); `player+0x3B84` was 0.
  - guard #8 QPC `0x326C2C03B` → setter #2 `0x326C2C081` (**70 ticks**); `player+0x3B84` was `0x00020000`.
- Therefore the field was zero at the first AL=0 guard/setter entry and already `0x20000` at the second. The first setter call lies between the two observations. This is strong temporal and stack attribution consistent with the first original setter call setting bit17, but the exact memory-store instruction is not directly captured because `ArcanaAI_PlayerBit17WriteTrace.log` is zero bytes.

### Empty logs, scope-limited no-hits and diagnostics

- `ArcanaAI_PlayerBit17WriteTrace.log`, `ArcanaAI_StateMapTrace.log`, and `ArcanaAI_ProducerCallTrace.log` were zero bytes.
- `ArcanaAI_FinalCommandTrace.log` contains 45 records but none at its monitored callsite carries payload `0x1E`; this does not contradict the confirmed alternate queue insertion/dispatch route at `0xF6AD7`.
- No ERROR/WARNING/DROP/OVERFLOW/FAILED markers were found in nonempty uploaded runtime logs.

### Next task remains Stage24

The new run strengthens the Stage23 resolver result and the Stage18/20/17 setter attribution, but does not inspect the downstream mapping consumer. Stage24 must add a separate passive observer at CALL RVA `0x000EE306 -> 0x000F57D0`, capture its four arguments, the three runtime table bytes at `0x04DDF188 + selector*3`, and the three DWORDs at `player+0x3A08` before/after the helper. Use a distinct Stage24 marker and log filename. Keep both Stage23 evidence branches immutable. Read/write diagnostics only; do not alter mapping-table or player state.


## Current checkpoint — Stage24 runtime, 2026-11-09 16:35

- Tested source: [stage24-resolver-consumer-trace](https://github.com/Hawky11/1/tree/stage24-resolver-consumer-trace), commit `0bca8a6d8e851990563b281c517f01bd1a0c81a2`.
- Evidence: [stage24-resolver-consumer-runtime-evidence](https://github.com/Hawky11/1/tree/stage24-resolver-consumer-runtime-evidence), commit `983a7f4f205c99cd1c8f44814c58cc09ed76f0c8`. See [full analysis](https://github.com/Hawky11/1/blob/stage24-resolver-consumer-runtime-evidence/Stage24_Overlay/STAGE24_RUNTIME_ANALYSIS.md).
- User uploaded logs to Stage23 branch at `96464289b7c41c4f62402b75ad1081d191ae999b`; this is a Stage24 r1 run despite the branch name. Do not treat that branch HEAD as the old Stage23 source snapshot; old Stage23 r2 source remains at `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`.
- Build 16:35:12: eight units compile, DLL233984 bytes, only C4754 at AICommandSiteTrace.cpp:785. Marker and hook EE306->F57D0 confirmed.
- Two P1 events: selector0x1F, actual arg3=0/arg4=0, table pre-entry bytes03,00,10; output (6,0x1002,0)->(0x1F,0x10001,0) and (4,0x11,0)->(0x1F,0x10001,0). All reads successful. Both correlate to Stage23 and queue/dispatch/classifier/fallback/AND. Slot3 and bit0x400 stay zero.
- **Correction:** previous predictions arg3=3/arg4=18 were not confirmed. Resolver bytes remain3/18; full intervening dispatcher dataflow needs review. Wrapper argument-copy/replay/log offsets were reviewed without finding an overwrite.
- Flags3 suggest both static flag paths; secondary-helper execution is not directly captured. Higher-level selector semantics remain unknown.
- All19 log blobs and pasted Build.log preserved. Read17 logs fully without explicit error/drop/overflow markers. Two large logs (PlayerBit17WriteTrace and ProducerCallTrace) could not be read with available connector transport; do not describe them as empty or claim their write transitions were verified.
- Next: retrieve those two large logs; inspect full EE278..EE306 disassembly to explain zero arguments before selecting Stage25 instrumentation. No Stage25 code has been created.



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

## Stage25 — runtime-verified secondary-object OR, 2026-11-09

- Original source snapshot: [stage25-secondary-object-flag-trace](https://github.com/Hawky11/1/tree/stage25-secondary-object-flag-trace), `3f034550f9d1254cc136452dde363657f913a91c`.
- New immutable runtime evidence: [stage25-secondary-object-flag-runtime-evidence](https://github.com/Hawky11/1/tree/stage25-secondary-object-flag-runtime-evidence), `b426a8f74764b7606ce3b5da075bb3a61d3698b`, preserving 20 uploaded logs from main upload commit `b9ba7472ccaeea110b8828098d04b23a264527df` by identical Git blob SHA, source/INI, hand-pasted chat Build.log normalized as `Stage25_Overlay/Build_log_from_chat.txt`, and `Stage25_Overlay/STAGE25_RUNTIME_ANALYSIS.md`. Original Build.log was not uploaded.
- Build from chat started 09.11.2026 16:50:57.87: VS 18 Community, MSVC 14.51.36231 x86, 8 C++ files compile, `dinput8.dll` successfully linked at 237,568 bytes. The existing single C4754 warning in `AICommandSiteTrace.cpp:785` persists; no compiler error.
- `STAGE25_SECONDARY_OBJECT_FLAG_BUILD=2026-11-09-r1` at 16:51:44; validated 7-byte OR replacement installed at RVA `0xF58C9`. One 16:52:07 runtime sample P1/tid6128/tick29260265 captures source player `0x0E251F78`, target `0x0E27CCA0` (P2) and target DWORD `0x0E27CD98` before and after the original `OR [EAX+0xF8],1`: both `0x00000001`. Both reads succeeded; bit0 was already 1.
- Same-event QPC: alternative call `292602968311`; queue insertion `292602968399` (type1/payload0x1E, accepted); dispatcher start `292602968416`; Stage23 resolver `292602968420` (selector31); Stage25 OR `292602971617`; Stage24 helper completion `292602973136` (mapping bytes03/00/10, output `(0,1,0)→(0x1F,0x10001,0)`, helper EAX same as P2); Stage21 classifier `292602975570` AL0; fallback slot3 `0→0` at `292602976845`; Stage22 controller bit0x400 `0→0` at `292602977325`; dispatcher end `292602977883`. The Stage25 event is precisely inside the Stage24 helper, not a separate action.
- Full large-log inspection: Stage16 write trace 12,948,453 bytes with seq1..24176 all present/unique; Stage14 producer trace 8,833,751 bytes with seq1..11383 all present/unique. Both newline-terminated, neither has explicit error/warning/drop/overflow markers. One bit17 set+clear cycle for P1 (seq3119/3128) and one for P2 (seq13050/13059); these touch player+`0x3B84`, not target+`0xF8`. Stage15 has 10 branch records, including an accepted P1 payload0x1E event with bit17=0 and later P2 suppressed bit17=1 event.
- **Limit:** only one filtered Stage25 OR was observed; a 1→1 write does not show the initial writer. No gameplay-semantic attribution of +0xF8 bit0. Proposed Stage26: static writer xrefs and then (if needed) protected, noninterfering P1/P2 +0xF8 write observation with unique marker/log, while preserving Stage16 DR0/DR1.


---

## Stage26 — source-only P1/P2 +0xF8 bit0 write trace prepared, 2026-11-09

- Source branch: [stage26-object-bit0-write-trace](https://github.com/Hawky11/1/tree/stage26-object-bit0-write-trace), HEAD `a249f452353c340fe8f1dab23375a8a932b079b4`. Parent source snapshot is Stage25 `3f034550f9d1254cc136452dde363657f913a91c`. All prior source/evidence branches preserved.
- Motivating Stage25 result: P1/type1/payload0x1E selected P2; original `OR DWORD PTR [EAX+0xF8],1` at F58C9 observed target P2+`0xF8` `1→1`. Previous setter and gameplay semantics unknown.
- Static approach: adapt version-controlled Stage20 `Stage20_Overlay/src/AITrace.cpp` hardware write-watchpoint pipeline for a new Stage26 mode. New field offset `0xF8`, mask0x1, derived watchpoint RVAs P1 `0x0DCA2070` / P2 `0x0DCCCD98`. Preserve known DR0/DR1 write-only configuration `0x00DD0005`, DR2/DR3, external DR owners and original game writes.
- New INI `EnableStage26ObjectBit0WriteTrace=1`, `Stage26TransitionsOnly=1`; legacy `EnableControlFieldTrace=0` and Stage16 `EnablePlayerBit17WriteTrace=0`, checked mutually exclusive. Existing Stage25 other tracing remains enabled. Stage26 marker `STAGE26_OBJECT_BIT0_WRITE_BUILD=2026-11-09-r1`, log `ArcanaAI_Stage26ObjectBit0WriteTrace.log`.
- Existing Stage16 record buffer is reused: capture side, thread, tick/QPC, post-write EIP/RVA, registers/DR6, code window and eight guarded stack DWORDs, old/new shadow bit0, plus baseline P1/P2 values. Default suppress unchanged bit0 writes after updating shadow; log filtered counts periodically and report dropped records. For unfiltered traffic use `Stage26TransitionsOnly=0`.
- Static source/INI invariants checked in the creation session; the bundled verifier performs equivalent source/INI address and toggle checks. **No successful MSVC x86 build or in-game runtime is claimed.** An entire AALib.exe binary was not part of the source repository, so a complete static search of writer sites remains unavailable. A P1/P2 value already 1 at startup would not identify the original setter.
- Next: user applies Stage26 `AITrace.cpp` and INI over working Stage25 project (retaining Stage25 `AIQueueTrace.cpp`), builds and uploads a new game run. Only then analyze write attribution and archive evidence.
