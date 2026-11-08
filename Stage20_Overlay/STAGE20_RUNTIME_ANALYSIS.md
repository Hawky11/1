# Stage20 runtime analysis — object bit-2 guard and alternate payload path

## Runtime snapshot

- Runtime upload commit on `main`: `3bd72139fa658bb60e0a3a3a30b63af7cd7519b8` (2026-11-08 run beginning 19:58:47). This evidence branch starts from that exact commit, preserving the uploaded logs.
- Source snapshot: `stage20-bit113-object-bit2-guard-trace`, HEAD `11dde281a08a3481517d0cb60d4752393fa0e578` before the evidence copy.
- The previous user-supplied Build.log (19:55:42,91) showed VS 18 Community / MSVC 14.51.36231 / Win32, all eight C++ files compiled, link success, `dinput8.dll` size 216,576 bytes, with one non-fatal C4754 warning in `AICommandSiteTrace.cpp` lines 782–785. Build.log itself was not uploaded with this runtime log set. The runtime marker below proves the Stage20 code was loaded; no runtime DLL hash was logged.
- Run start: `ArcanaAI.log` reports COMvsCOM active at 19:58:47.

Uploaded runtime log set preserved at repository root: `ArcanaAI.log`, `ArcanaAI_AlternateCallTrace.log`, `ArcanaAI_Bit113GateTrace.log`, `ArcanaAI_Bit113ObjectGuardTrace.log`, `ArcanaAI_Bit113SetterTrace.log`, `ArcanaAI_CommandTrace.log`, `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_FinalCommandTrace.log`, `ArcanaAI_ParentFlowTrace.log`, `ArcanaAI_PlayerBit17WriteTrace.log`, `ArcanaAI_ProducerCallTrace.log`, `ArcanaAI_ProducerContextTrace.log`, `ArcanaAI_QueueTrace.log`, `ArcanaAI_StateMapTrace.log`, and `ArcanaAI_Trace.log`. No Build.log file is present in this upload.

## 1. Stage20 startup and installation — confirmed

`ArcanaAI_Bit113ObjectGuardTrace.log` contains:

- `STAGE20_BIT113_OBJECT_BIT2_GUARD_BUILD=2026-11-08-r1` at 19:58:47.885.
- `STAGE20_CONFIG` confirms `EnableStage20Bit113ObjectGuardTrace=1`, `EnableStage19StateMapTrace=1`, and `EnableStage18Bit113GateTrace=1`.
- Successful installation at 19:58:48.575: CALL RVA `0x000C64F1`, original helper RVA `0x00095B00`, resume RVA `0x000C64F6`; `following TEST/JNE left untouched`; `suspended=40`.
- `Stage20 trace active` is logged immediately afterward.

This is runtime proof that the Stage20 wrapper installed and the trace was active. It is not merely a source/build inference.

## 2. Stage20 object guard — 33 records, all AL=1 / bit 2 set

`ArcanaAI_Bit113ObjectGuardTrace.log` contains 33 `BIT113_OBJECT_GUARD` records, from 19:59:06.349 through 19:59:25.236:

| Field | Observed result |
|---|---:|
| Records | 33 |
| Side | P2 only (33) |
| Player | `0x0E9ECCA0` |
| EBX slot pointer | `0x0EFAF8E4` |
| `[EBX]` / saved original ECX | `0x0E9ECCA0` in all records |
| Helper argument | `2` in all records |
| Original helper result | `AL=1`, `EAX=1` in all 33 |
| Checked DWORD `[object+0x3B70]` | `0x0000004C` in all 33 |
| Bit `0x4` | set in all 33 |
| Read mask | `0x3F` in all 33 (all six diagnostic reads succeeded) |
| Parent return | `0x00DB6801` in all 33 |
| Return to caller | `0x00DE64F6` in all 33 |

The exact helper return AL is authoritative. The readback of `[object+0x3B70]` corroborates it: both are consistent with mask `0x4` being set.

## 3. One-to-one correlation with Stage18 gate — confirmed

`ArcanaAI_Bit113GateTrace.log` contains 33 `BIT113_GATE_RESULT` records, all for P2, all with table key `0x19`, all returning `AL=1`. They have the same player pointer `0x0E9ECCA0` and parent return `0x00DB6801` as the Stage20 records.

Each Stage20 record has exactly one preceding Stage18 gate record with matching side and player pointer. The Stage20 QPC is 129–173 ticks after the matched Stage18 QPC. All 33 pairs match; there are no unmatched Stage20 records or Stage18 gate records in this captured set.

Stage18 `stateIndex` distribution (separate from the table `key=0x19`):

| `stateIndex` | Calls |
|---:|---:|
| `0x0A` | 4 |
| `0x04` | 2 |
| `0x14` | 25 |
| `0x10` | 2 |

Every captured Stage18 gate result is AL=1; no AL=0 table-gate sample appears in this run.

**Interpretation:** all 33 observed calls pass the table gate and then return AL=1 from the object bit-2 helper. Per the statically verified caller sequence, that nonzero AL makes the original JNE skip the direct bit-113 setter call at RVA `0x000C6502`. This gives direct runtime support to the object-guard explanation for these 33 calls.

Important distinction: the `key=0x19` value is not the same field as Stage18's `stateIndex`. The 33 Stage18 records have stateIndex values `0x0A`, `0x04`, `0x14`, and `0x10`; do not describe them as 33 `stateIndex=0x19` samples.

## 4. Stage17 setter and Stage16 bit17 writes

`ArcanaAI_Bit113SetterTrace.log` confirms the passive callsite observer installed at RVA `0x000C6502`, return RVA `0x000C6507`, original setter helper `0x0000FAA0`, with 40 suspended threads. It contains **zero** `BIT113_SET_CALL` records.

`ArcanaAI_PlayerBit17WriteTrace.log` contains 28,043 write snapshots:

| Side | Write records |
|---|---:|
| P1 | 13,069 |
| P2 | 14,974 |
| Total | 28,043 |

All 28,043 records report `oldBit17=0` and `newBit17=0`; there are zero bit17 transitions. Sixteen DWORD snapshots changed in some other bits, but none changed the traced bit17. The Stage15 branch snapshots also report `bit17=0`.

This is consistent with the object guard being nonzero and the direct setter not being called in this run. It does **not** yet supply the opposite control sample (object bit 2 clear / helper AL=0 / setter reached). The four earlier Stage17 setter episodes were from the previous run and cannot be retroactively treated as Stage20 object-guard observations.

## 5. Stage19 state-map trace — active, but not a one-to-one mapping to these gate records

`ArcanaAI_StateMapTrace.log` contains 2,974 `STATE_MAP_CALL` records:

- P1: 1,866 calls, all with expected index `0x18`.
- P2: 240 calls map raw code `0x5C` to expected index `0x19` (map result `1`).
- P2: 140 calls map raw codes `0x41` / `0x21` to expected index `0x18` (map result `0`).
- P2: 158 calls map raw code `0x63` to expected index `0x1C` (map result `7`).
- P2: 570 calls map raw code `0x60` to expected index `0x1B` (map result `3`).

These map-helper records are useful context but do not directly identify the 33 Stage18 gate records: their `stateIndex` field differs from the table key, and the nearest earlier map-helper calls are separated by millions of QPC ticks and do not have one uniform expected index. Keep the mapper output and Stage18 table-gate index distinct unless a stronger same-event correlation is established.

## 6. A second observed path: alternate callsite queues and dispatches payload 0x1E while bit17 is zero

`ArcanaAI_ProducerCallTrace.log` contains 12 `STAGE15_BRANCH` records, forming three same-thread/same-parent-frame episodes (P2 once, P1 twice). In each episode:

- `F6A5D_F2ED0_BRANCH`: ZF=1, branch not taken, bit17=0.
- `F6A63_PAYLOAD_BRANCH`: ZF=1, branch not taken, bit17=0.
- `F6A68_PLAYER_BIT17_BRANCH`: ZF=1, branch taken, bit17=0.
- `F6A9D_LOCAL_GUARD_BRANCH`: ZF=1, branch taken, bit17=0.

The alternate callsite observer at RVA `0x000F6AD7` captured three calls, all with payload `0x0000001E`:

| Side | ALT call QPC | Previous `F6A9D` QPC | Delta |
|---|---:|---:|---:|
| P2 | 8062843391704 | 8062843391392 | 312 |
| P1 | 8062907268493 | 8062907268067 | 426 |
| P1 | 8062987182831 | 8062987182520 | 311 |

The earlier `F6A68` branch-to-ALT-call deltas are 621, 851, and 620 QPC ticks respectively. All three records use the same game thread and parent EBP frame. Thus, in these observed episodes, the bit17 branch is taken while bit17 is zero, yet execution still reaches the alternate callsite with payload `0x1E`.

Each alternate call has a matching queue insertion and dispatch:

| Side | Queue detail QPC | Event-dispatch QPC before/after | Result |
|---|---:|---:|---|
| P2 | 8062843391759 | 8062843391784 / 8062843392790 | Accepted, then dispatched |
| P1 | 8062907268528 | 8062907268552 / 8062907269941 | Accepted, then dispatched |
| P1 | 8062987182921 | 8062987182939 / 8062987183958 | Accepted, then dispatched |

All three queue details report event type `1`, payload `0x1E`, `activeAfter=1`, and stored pair `(0001,001E)`. The three dispatch details report event type `1`, payload `0x1E`, `playerMode=1`, and `beforeMatches=1` / `afterMatches=1`.

**Interpretation:** in this run, the alternate `F6AD7` queue/dispatch path is demonstrably active while bit17 is zero. Therefore, do not treat bit17 as a necessary precondition for this specific observed alternate payload-`0x1E` path. This does not establish what the event ultimately does after dispatch.

## 7. FinalCommandTrace boundary

`ArcanaAI_FinalCommandTrace.log` contains 60 `FINAL_COMMAND` records at the monitored type-1 callsite (`0x000F7C5D` / return `0x000F7C62`). None has payload `0x1E`.

This is a **scope-limited no-hit** at that particular final-command callsite. It does not negate the three separately observed queue insertions or dispatches, and it does not prove that the dispatched `0x1E` event was unhandled. The alternate path may reach a different consumer.

## 8. Errors, coverage limits and next investigation

- The reviewed Stage20, Stage18, Stage17, Stage19, Stage16, Stage15, alternate-call, queue, dispatch, final-command, parent-flow and producer-context logs contain no explicit warning/error/drop/failure records.
- No Stage20 `AL=0` / bit-2-clear sample was captured.
- No Stage20 object-guard record for P1 was captured.
- No Stage17 direct setter call or Stage16 bit17 transition was captured in this runtime.
- No Stage18 table-gate `AL=0` sample was captured.
- The exact Build.log was pasted in chat, not uploaded as a file; no DLL hash is present in the runtime logs.

Next work should be staged around two independent unknowns:

1. **Direct setter positive control:** obtain a natural call where `[object+0x3B70] & 4` is clear, then correlate actual Stage20 AL=0 with the Stage17 call at `0x000C6502` and Stage16 bit17 transition. Do not force-edit object/player memory.
2. **Alternate payload consumer:** statically identify the event handler/consumer after the observed type-1 / payload-`0x1E` dispatch at caller RVA `0x000EE5CB`. The `0x000F7C5D` FinalCommand callsite did not observe this payload in the current window, so Stage21 should trace the alternate consumer rather than assume that this one callsite is universal.

Also investigate why `F6A68` and `F6A9D` report taken branches with bit17 zero while `F6AD7` is still reached shortly afterward in the same parent frame. Treat branch names as probe labels, not semantic proof of what the branch suppresses.

Preserve the Stage19 source/evidence branches and all Stage12–Stage19 historical branches unchanged.