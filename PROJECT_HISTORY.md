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
