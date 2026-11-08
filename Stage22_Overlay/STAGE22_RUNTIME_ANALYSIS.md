# Stage22 runtime analysis — controller bit 0x400 remains zero; Stage18/20/17 chain correlated

Date: 2026-11-08  
Source branch: [stage22-payload-1e-post-and-trace](https://github.com/Hawky11/1/tree/stage22-payload-1e-post-and-trace)  
Runtime evidence branch: [stage22-payload-1e-runtime-evidence](https://github.com/Hawky11/1/tree/stage22-payload-1e-runtime-evidence)  
Build: Visual Studio 18 Community / MSVC 14.51.36231 / Win32 x86  
DLL: `build\\dinput8.dll`, **225,792 bytes**

## 1. Build and installation

The user supplied Build.log dated 2026-11-08 23:30:54. All eight C++ translation units compiled and the link succeeded. One non-fatal C4754 warning remains in `AICommandSiteTrace.cpp:785` (comparison promotion makes one branch unreachable). The transcript is archived as `Build.log`.

Runtime markers and installations:

- Stage21 r2 marker: `STAGE21_PAYLOAD_1E_CONSUMER_BUILD=2026-11-08-r2`.
- Stage21 classifier and fallback wrappers installed: `0xEE426 -> 0xE8CF0`, `0xEE446 -> 0xF7EB0`, suspended 38 threads.
- Stage22 marker: `STAGE22_PAYLOAD_POST_AND_BUILD=2026-11-08-r1`.
- Stage22 post-AND observer installed at `0xEE451`; log confirms validated seven-byte instruction was replaced with CALL + two NOPs.
- Event dispatch, queue, Stage20 object-guard, Stage17 setter and Stage18 gate observers also report successful installation.

Thus this is a build-verified and runtime-installed Stage22 run, not merely source evidence.

## 2. Three payload-0x1E events traced end-to-end through the consumer

The queue, dispatch, Stage21 and Stage22 logs provide three matching events, all on thread 10804:

| Event | Side | Queue QPC | Dispatch QPC before | Classifier QPC | Fallback QPC | Post-AND QPC |
|---|---|---:|---:|---:|---:|---:|
| 1 | P2 | 8190556018120 | 8190556018144 | 8190556019441 | 8190556022852 | 8190556023516 |
| 2 | P2 | 8190773497452 | 8190773497465 | 8190773498477 | 8190773500276 | 8190773500737 |
| 3 | P1 | 8190826674362 | 8190826674384 | 8190826675468 | 8190826677753 | 8190826678325 |

For all three:
- Alternate producer at `0xF6AD7` queued event type 1 / payload `0x1E`; QueueTrace reports `ACCEPTED_BY_QUEUE`.
- EventDispatch at `0xEE200` observed the same event and queue history matched the event arguments.
- The real helper `0xE8CF0` at `0xEE426` returned `EAX=0 / AL=0`.
- Fallback helper `0xF7EB0` at `0xEE446` ran with slot index 3.
- Stage22 captured the following AND at `0xEE451` and returned to the original flow via `0xEE456` then the two patched NOPs.

The QPC gaps from fallback to post-AND were 664, 461 and 572 ticks, respectively. The EventDispatch detail post-call QPC occurs after the Stage22 sample in all three events.

## 3. Stage22 controller flag result: no transition captured

There are exactly three `STAGE22_POST_AND` records:

| Seq | Side | Controller flags before | After | bit 0x400 before → after | Readable |
|---:|---|---:|---:|---:|---:|
| 1 | P2 | `0x00000000` | `0x00000000` | `0 → 0` | Yes |
| 2 | P2 | `0x00000000` | `0x00000000` | `0 → 0` | Yes |
| 3 | P1 | `0x00000004` | `0x00000004` | `0 → 0` | Yes |

All post-readbacks succeeded (`afterReadable=1`). Stage22 proves the original AND executed for all three payload-0x1E events and that the controller DWORD remained as shown. It **does not** capture a `1 → 0` transition because the targeted bit was already clear on entry. P1's unrelated lower bit `0x4` remains set; the mask only clears `0x400`.

Stage21 slot-3 records also show zero before and zero after for all three events. Therefore neither a nonzero-to-zero slot-3 mutation nor a nonzero-to-zero controller-bit mutation was observed in this run.

## 4. New same-event Stage18 → Stage20 → Stage17 correlation

This run resolves the previous missing Stage18 gate-result correlation for the object-bit-2-clear / direct-setter path.

- Stage18: **11** `BIT113_GATE_RESULT` records.
- Stage20: **11** `BIT113_OBJECT_GUARD` records.
- Stage17: **11** `BIT113_SET_CALL` records.

All 11 records match one-to-one by side, player pointer, thread 10804, game tick and the same parent return address (`0xDB6801` or `0xDEB9FA`). For every matched group:

1. Stage18 gate helper returned `AL=1`, table key `0x19`; state index was `0x18` or `0x19`. The selected table byte readback was `0xFA`.
2. 122–178 QPC ticks later, Stage20 object helper saw `[object+0x3B70]=0x00000640` (bit 2 clear) and returned actual `AL=0`.
3. 71–96 QPC ticks after the object guard, Stage17 observed the direct CALL at `0xC6502`, passing setter index `0x71` for the same player.

Exact QPC deltas (Stage18 gate → Stage20 object guard → Stage17 setter):

| Seq | Side | Tick | State index / key | Gate→guard | Guard→setter |
|---:|---|---:|---|---:|---:|
| 1 | P2 | 819047578 | `19 / 19` | 132 | 77 |
| 2 | P1 | 819055281 | `18 / 19` | 126 | 73 |
| 3 | P1 | 819055281 | `18 / 19` | 122 | 72 |
| 4 | P2 | 819056812 | `19 / 19` | 126 | 75 |
| 5 | P2 | 819056812 | `19 / 19` | 122 | 72 |
| 6 | P2 | 819063796 | `19 / 19` | 178 | 88 |
| 7 | P2 | 819063796 | `19 / 19` | 159 | 96 |
| 8 | P1 | 819069875 | `18 / 19` | 124 | 76 |
| 9 | P1 | 819069875 | `18 / 19` | 122 | 71 |
| 10 | P2 | 819078875 | `18 / 19` | 131 | 76 |
| 11 | P2 | 819078875 | `18 / 19` | 125 | 73 |

All 11 object DWORDs were `0x640`; all 11 object-helper returns were `AL=0`; all 11 setter calls passed `0x71`. This is a direct runtime correlation for the tested call chain. It does not establish that every possible game path uses this caller.

### What the setter snapshots imply — and what they do not

In five two-call episodes (P1 seq 2–3 and 8–9; P2 seq 4–5, 6–7 and 10–11), the first Stage17 setter entry saw `[player+0x3B84]=0`, while the second call in the same episode saw `[player+0x3B84]=0x00020000`. That is consistent with bit 17 becoming set between the first and second calls, after the index-`0x71` setter was invoked.

However, `ArcanaAI_PlayerBit17WriteTrace.log` is **zero bytes** in this upload. The exact writer instruction and QPC of the write were therefore not captured in this run. Do not present the Stage17 pre-call snapshots as a substitute for the missing Stage16 write-watchpoint record.

## 5. Stage21 r2 decoded-action field is still semantically invalid

The r2 wrapper corrected the sampling time by saving EDI before the original classifier helper. The resulting values now reveal a second issue: at the actual callsite `0xEE426`, EDI is **not** the decoded action code.

Observed values:

| Side | `decodedActionCodePreCall` decimal | Same value in hex | Stage21 fallback array pointer |
|---|---:|---:|---:|
| P2 | 245303004 | `0x0E9F06DC` | `0x0E9F06DC` |
| P1 | 245127604 | `0x0E9C59B4` | `0x0E9C59B4` |

The pre-call EDI values exactly equal the controller-array pointers subsequently passed to the slot-clear helper. Thus r2 successfully captures the pre-call EDI value, but the field name/interpretation `decodedActionCodePreCall` is wrong. The code must have repurposed EDI between the earlier `cmp edi,1` branch and CALL `0xEE426`, or the earlier register/dataflow interpretation needs correction. Do not use these values as an action code.

The actual classifier AL=0, dispatch type/payload and fallback slot data remain valid. If action-code identity is needed, next inspect the last write to EDI before `0xEE3E1` and the instructions between `0xEE3E1` and `0xEE426`; a future logger should capture the decoded byte at the actual decode/compare point, not assume EDI retains it at CALL entry.

## 6. Missing or incomplete evidence

- `ArcanaAI_PlayerBit17WriteTrace.log`: zero bytes; no exact bit17 writer attribution for this run.
- `ArcanaAI_StateMapTrace.log`: zero bytes; no Stage19 state-map records.
- `ArcanaAI_Bit113GateTrace.log`: 11 records, all useful and correlated as described above.
- No warning/error/drop/failure records were found in Stage21/22 consumer, queue, dispatch, Stage20 guard, Stage17 setter or Stage18 gate logs reviewed for this run.
- A natural precondition with slot 3 nonzero or controller bit `0x400` set remains unobserved. Stage22 is execution confirmation only for these three `0 → 0` cases.

## 7. Next research task

1. Preserve this evidence branch unchanged after review.
2. Correct the EDI dataflow analysis by disassembling the instructions from the resolver result through the compare and the callsite; determine where EDI becomes the controller-array pointer.
3. If action-code runtime confirmation is still needed, add a passive observer at the true decode/compare point while retaining Stage21/22 actual consumer records. Do not rename the r2 field in this archived runtime snapshot.
4. For further state-mutation confirmation, capture a natural event with slot 3 nonzero before helper `0xF7EB0`, or bit `0x400` set before the AND. Do not force either state.
5. Re-enable/diagnose the Stage16 writer and Stage19 state-map logs only if those exact correlations are still required; do not infer missing records.

Stage21 r1 and Stage21 r2 source snapshots, Stage21 r1 runtime evidence, and all earlier stage branches remain unchanged.
