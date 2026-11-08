# Stage21 runtime analysis — payload 0x1E consumer and direct setter control

Date: 2026-11-08  
Run window: 21:44:33–21:45:03 local time  
Runtime upload commit: `e7f3faf33c5e37f49896ed4f0375b12c2b853daa`  
Build transcript: `Stage21_Overlay/Build.log` (pasted by the user in chat and archived verbatim here)  
Source under test: the Stage21 r1 overlay in this branch, copied byte-for-byte from `stage21-payload-1e-consumer-trace`.

## 1. Build result

The supplied Build.log reports:
- Build start: 2026-11-08 21:43:30,49.
- Visual Studio 18 Community; MSVC 14.51.36231, x86/Win32.
- All eight translation units compiled: Proxy.cpp, ComVsCom.cpp, AITrace.cpp, AICommandTrace.cpp, AIQueueTrace.cpp, AICommandSiteTrace.cpp, AIProducerContextTrace.cpp and AIParentFlowTrace.cpp.
- Link succeeded; `build\\dinput8.dll` size is **222,208 bytes**.
- One non-fatal C4754 warning remains in `AICommandSiteTrace.cpp` lines 782–785 (reported location line 785).

This is a successful compile/link, not by itself runtime evidence. Runtime evidence below independently confirms the new Stage21 hook installation.

## 2. Stage21 installation

`ArcanaAI_Stage21PayloadConsumerTrace.log` contains marker:
`STAGE21_PAYLOAD_1E_CONSUMER_BUILD=2026-11-08-r1`.

The runtime log confirms both observational wrappers were installed:
- CALL RVA `0x000EE426` -> original classifier helper RVA `0x000E8CF0`.
- CALL RVA `0x000EE446` -> original fallback slot-clear helper RVA `0x000F7EB0`.
- `suspended=40` process threads during the patch operation.

Stage20 was also active in this run: the wrapper at `0x000C64F1` targeting `0x00095B00` installed successfully (`suspended=40`). Stage17's passive setter observer at `0x000C6502` installed successfully, also with 40 threads suspended. Stage18's passive observer at `0x000C64A5` installed successfully.

## 3. Payload-0x1E path: P1 and P2 both reached fallback

There are two matched type-1 / payload-`0x1E` queue, dispatch and Stage21 consumer episodes, one for each side. Both ran on thread ID 23660.

| Side | Alternate queue call QPC | Queue detail QPC | Dispatch QPC before | Stage21 classifier QPC | Stage21 slot-clear QPC | Dispatch QPC after |
|---|---:|---:|---:|---:|---:|---:|
| P1 | 8126274129023 | 8126274129067 | 8126274129108 | 8126274130091 | 8126274132254 | 8126274132761 |
| P2 | 8126274135408 | 8126274135433 | 8126274135454 | 8126274136430 | 8126274136908 | 8126274137265 |

For both sides:
- Alternate writer CALL at RVA `0x000F6AD7` carried type 1 / payload `0x1E`.
- Queue trace recorded `ACCEPTED_BY_QUEUE`, changing queue active from 0 to 1 and storing type/payload `(0001,001E)`.
- Event dispatch at RVA `0x000EE200` was called by caller return RVA `0x000EE5CB`; detailed before/after snapshots both had `beforeMatches=1` and `afterMatches=1`.
- Stage21 classifier hook at `0x000EE426` recorded real `EAX=0`, `AL=0`.
- Stage21 fallback hook at `0x000EE446` ran for the same side/player/thread and logged index 3.

The Stage21 classifier's actual AL is confirmed for both samples. This validates the static branch: `TEST AL,AL; JE 0xEE443` selects the fallback for payload `0x1E`.

### Slot and controller-state limitation

Both slot-clear records showed:
- P1 array `0x0E9C59B4`, slot address `0x0E9C59C4`.
- P2 array `0x0E9F06DC`, slot address `0x0E9F06EC`.
- `slotValueBefore=0`, `slotValueAfter=0`, `afterReadable=1`.
- `controllerFlagsBeforeFollowingAND=0`.

Therefore the fallback helper definitely executed, but neither sample demonstrates a nonzero slot changing to zero; both slots were already zero. The bit-`0x400` flag was also already clear before the next AND. This run confirms the fallback path is entered, not a non-idempotent state transition.

## 4. Stage20 negative control: natural object-bit-2-clear sample found

This same run finally captured the missing Stage20 negative control.

`ArcanaAI_Bit113ObjectGuardTrace.log` contains one record:

- Side P1; thread 23660.
- Player / checked object: `0x0E9C1F78`.
- Helper argument: `2` (mask `0x4`).
- Actual helper `EAX=0`, `AL=0`.
- Checked DWORD `[object+0x3B70]=0x00010440`.
- `bit2Set=0`, `readMask=0x3F`.

This is a natural runtime sample, not a forced memory edit.

### Exact correlation to the direct bit-113 setter

`ArcanaAI_Bit113SetterTrace.log` contains one record on the same side, player, thread and millisecond:

- Setter callsite RVA `0x000C6502`, return RVA `0x000C6507`.
- Original setter helper RVA `0x0000FAA0`.
- Index `0x71` (113).
- `ECX=this=ESI=0x0E9C1F78`.
- `other3B70=0x00010440`, matching the object guard record.

The QPC values are:
- Object guard: `0x00000764133B9CC6`.
- Setter call: `0x00000764133B9D15`.
- Delta: **79 QPC ticks**.

Thus, for this observed P1 call, the actual object-helper result `AL=0` is followed 79 QPC ticks later by the direct setter call passing index `0x71` for the same player. This is strong event-level runtime evidence that the original `JNE` after the object helper falls through to `0xC6502` when bit 2 is clear. It confirms the direct setter call was reached; it does not, by itself, prove the setter changed the player's bit17 DWORD.

### What remains unproven about bit17

`ArcanaAI_PlayerBit17WriteTrace.log` exists in the upload but is **zero bytes**. The general `ArcanaAI_Trace.log` shows five watchpoint-install batches (39, 16, 1, 1 and 2 newly discovered threads), but no bit17 write record is available.

The Stage17 setter record captures pre-call state, including `[player+0x3B84]=0x01000000`; the Stage20 object-guard record also reads that value. Bit17 (mask `0x00020000`) is clear in that pre-call DWORD. No post-call value or Stage16 write transition was captured, so the observed setter call must not be described as a confirmed bit17 0->1 transition.

## 5. Stage18 / Stage19 coverage

- Stage18 marker/config and observer installation at RVA `0x000C64A5` are present, but `ArcanaAI_Bit113GateTrace.log` contains no gate-call result records. This P1 negative-control episode therefore cannot be linked to an actual Stage18 gate AL/key/state-index record in this run.
- Stage19 state-map trace contains 1,320 `STATE_MAP_CALL` records. Its last observed P1 entries were around 21:45:01.68; no same-event Stage19 record was found for the Stage20 object guard / Stage17 setter at 21:45:03.002. Do not infer a same-event state-table conversion from the earlier records.
- `ArcanaAI_FinalCommandTrace.log` has no payload-`0x1E` record at its monitored callsite. This remains a scope-limited no-hit; the separate queue/dispatch/consumer path is directly observed.

## 6. Stage21 r1 diagnostic-field defect

The two Stage21 classifier lines correctly record:
- original dispatch type = 1,
- original dispatch payload = `0x1E`,
- helper selector argument = 1,
- original helper result `EAX=0`, `AL=0`.

However, r1 labels the post-helper saved EDI value as `decodedActionCode`. The observed values are:
- P1: `0x0E9C59B4`.
- P2: `0x0E9F06DC`.

Those values exactly equal the controller-array pointers captured by the following slot-clear records. This is not a plausible action code. The logger sampled EDI **after** calling `0x4E8CF0`, which repurposes/clobbers EDI. Therefore `decodedActionCode` in the r1 log is invalid and must be ignored. The actual classifier AL, dispatcher type/payload, player, helper argument and fallback slot data remain valid.

For a future run, preserve pre-call EDI (the decoder has already established the action-code branch when execution reaches CALL `0xEE426`) before invoking the original helper, then log that saved value. Do not rewrite this r1 source snapshot: it is the exact source version built and runtime-tested here.

## 7. Warnings, gaps and status

No Stage21 install failure is recorded. The Stage21 log contains the startup marker, successful dual-hook installation and exactly four consumer records (P1/P2 classifier + fallback). The Build.log shows one non-fatal C4754 warning. The Stage16 write log is empty; Stage18 gate log has no result records; Stage21 slot pre-values are zero.

### Stage21 result

**Stage21 r1 is build-verified and runtime-verified for hook installation and the type-1 / payload-0x1E fallback path.** It proves actual classifier `AL=0` for P1 and P2 and confirms both fallback calls execute. The same runtime also closes the Stage20 object-bit-2 negative-control gap: natural bit2-clear / real `AL=0` is followed by a same-player direct bit-113 setter call within 79 QPC ticks.

Not yet proven:
1. A nonzero slot-3 value being cleared by the fallback.
2. A set `0x400` controller flag being cleared by the following AND.
3. A post-setter bit17 0->1 transition; the Stage16 log is empty.
4. A same-event Stage18 table-gate result for the P1 negative-control case.
5. The real decoded action code in the Stage21 r1 log; its field is invalid because EDI was sampled after the classifier helper.
