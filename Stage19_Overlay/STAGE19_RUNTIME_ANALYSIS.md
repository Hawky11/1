# Stage19 runtime analysis — state-code mapping and bit-17 suppression (2026-11-08)

## Runtime identity and installation

The uploaded run started at **2026-11-08 19:05:37**. The log set is in the root of this evidence branch. The Stage19 runtime marker is:

`STAGE19_BIT113_STATE_MAP_BUILD=2026-11-08-r1`

The startup logs confirm:

| Component | Runtime result |
|---|---|
| Stage16 P1/P2 player+0x3B84 write watchpoints | Active |
| Stage17 observer at RVA `0x000C6502` | Installed |
| Stage18 gate-result observer at RVA `0x000C64A5` | Installed |
| Stage19 state-map observer at RVA `0x000F6C08` | Installed |
| Stage19 suspended threads during install | 44 |
| Stage19 INI switch | `EnableStage19StateMapTrace=1` |

The Stage19 installer reports a six-byte patch and resumes at the original `0x000F6C0E` branch. Stage17 and Stage18 each report successful installation with 44 suspended threads.

**Build.log was not uploaded.** The runtime marker and the hook-installation message prove that a DLL containing and executing the Stage19 code was loaded. The standalone MSVC compiler/linker transcript and final DLL size cannot be independently verified from this upload.

## Stage19 — the conversion helper is confirmed

`ArcanaAI_StateMapTrace.log` contains **1,235** `STATE_MAP_CALL` records. All 1,235 have `readMask=0x0F`, meaning the four diagnostic reads (player WORDs, entry count, and the associated WORD flags for the mapped state) succeeded. No Stage19 ring-drop, warning or error record was found.

The logger captures the actual EAX returned by helper `0x004F02C0`. `expectedStateIndex` is then derived using the statically reconstructed wrapper mapping in `0x004F6BA0`; it is not a separate direct capture of that wrapper's final EAX.

| Side | Raw code | Helper EAX | WORD flags readback | Derived state index | Calls |
|---|---:|---:|---:|---:|---:|
| P1 | `0x20` | `0` | `0x0400` | `0x18` | 200 |
| P1 | `0x22` | `0` | `0x0400` | `0x18` | 190 |
| P1 | `0x24` | `0` | `0x0400` | `0x18` | 130 |
| P1 | `0x29` | `0` | `0x0400` | `0x18` | 12 |
| P1 | `0x4A` | `3` | `0x2000` | `0x1B` | 152 |
| P2 | `0x58` | `1` | `0x0802` | `0x19` | 82 |
| P2 | `0x59` | `1` | `0x0982` | `0x19` | 297 |
| P2 | `0x5C` | `1` | `0x0802` | `0x19` | 56 |
| P2 | `0x5E` | `3` | `0x2002` | `0x1B` | 116 |
| **Total** | | | | | **1,235** |

Helper return totals: **532 × 0**, **435 × 1**, **268 × 3**. The observed flag values agree with the helper's tested masks and priority: `0x0400 → 0`, `0x0800 → 1`, and `0x2000 → 3`. The extra bits in `0x0982` and `0x2002` do not change those results because the respective higher-priority matching masks are present.

The Stage18 gate log independently confirms the derived indexes for the matching calls:
- P2 raw code `0x59` / helper result 1 / derived index `0x19` correlates with 31 Stage18 gate records at index `0x19`; the Stage19-to-Stage18 QPC gaps are 67–108 ticks.
- P1 raw code `0x22` / helper result 0 / derived index `0x18` correlates with two Stage18 records; gaps are 64–70 ticks.
- P1 raw code `0x20` / helper result 0 / derived index `0x18` correlates with two further Stage18 records; gaps are 63–71 ticks.

Direct raw indexes at or below `0x17` do not call `0x4F02C0`, so Stage19 correctly has no conversion record for Stage18's direct states `0x0C`, `0x0E` and `0x10`.

The Stage19 log also observed mapped indexes `0x1B` from raw codes `0x4A` and `0x5E`; there was no matching Stage18 gate record at index `0x1B` in this capture. Do not assume every use of the state mapper proceeds through the specific bit-113 caller.

## Stage18 — table gate allowed every observed call

The new `ArcanaAI_Bit113GateTrace.log` contains **42** `BIT113_GATE_RESULT` records:

| Side | State index | Selector | Results | Table-byte readback |
|---|---:|---:|---:|---:|
| P1 | `0x0C` | `0x0A` | 2 | `0xFA` |
| P2 | `0x10` | `0x17` | 3 | `0xFA` |
| P2 | `0x19` | `0x17` | 31 | `0xFA` |
| P1 | `0x18` | `0x0A` | 4 | `0xFA` |
| P2 | `0x0E` | `0x17` | 2 | `0xFA` |

All 42 helper calls returned **AL=1** and all 42 post-return table readbacks were `0xFA`. There is still **no AL=0 sample**. The two confirmed parent invocation routes are both present: 38 records return to parent address `0x00DB6801` (the direct-call path) and four to `0x00DEB9FA` (the callback/function-table path).

The 31 repeated P2 index-`0x19` calls are especially useful: Stage19 correlates them to raw code `0x59` and helper result 1, but none of those 31 is followed by a Stage17 bit-113 setter call. Therefore, AL=1 from the table helper is an allowed result, not proof that the caller must execute the setter.

## Stage17 and Stage16 — four real bit-17 episodes

Stage17 logged **eight** calls to the confirmed setter callsite `0x000C6502`, all passing index `0x71`. They form two calls per episode:

| Time | Player | Stage18 state index | Stage17 calls | Stage16 bit-17 transition |
|---|---|---:|---:|---|
| 19:05:57.308 | P1 | `0x0C` | 2 | `0→1`, then later `1→0` |
| 19:06:02.625 | P1 | `0x18` | 2 | `0→1`, then later `1→0` |
| 19:06:04.227 | P2 | `0x0E` | 2 | `0→1`, then later `1→0` |
| 19:06:10.145 | P1 | `0x18` | 2 | `0→1`, then later `1→0` |

The Stage16 `ArcanaAI_PlayerBit17WriteTrace.log` contains **17,361** write records, including exactly eight bit-17 transitions:
- Four sets at RVA `0x0000FAD2`, each changing `player+0x3B84` from bit17=0 to bit17=1.
- Four clears at RVA `0x000ED1A9`, each changing bit17=1 to bit17=0.

The first Stage17 call in each pair precedes the corresponding set by 341, 345, 501 and 341 QPC ticks. The second setter call arrives after the bit is already set and is idempotent. Stage16's watchpoint worker installed DR0/DR1 on 61 game threads across three batches (43, 16 and 2); no watchpoint error or drop record was present.

## Stage15 and queue/dispatch — payload 0x1E remains suppressed

There are four Stage15 observations at each of:
- `F6A5D_F2ED0_BRANCH`;
- `F6A63_PAYLOAD_BRANCH`;
- `F6A68_PLAYER_BIT17_BRANCH`.

All four `F6A68` records have:
- `EDI=0x1E`;
- bit17=1;
- ZF=1;
- `branchTaken=0` (the original JZ is not taken).

The four `F6A68` observations occur at the same four episode timestamps listed above. There are no Stage15 branch records at `F6A9D` or `F6AAC`; the Stage13 hook at `F6AD7` installed but recorded no invocation.

The ordinary traffic logs continue to contain events (203 queue records, 197 dispatch records and 42 final-command records), but none contains payload `0x1E`. Thus the target event still does not reach the alternate queue insertion or dispatch path in this run.

No warning/error/drop record was found in the Stage19, Stage18, Stage17, Stage16, Stage15 ProducerCall, Queue, EventDispatch, FinalCommand or AlternateCall logs. The Stage10 ParentFlow/ProducerContext startup lines that contain the phrase `failedResultsSampled` are configuration text, not warnings.

## New interpretation: the downstream bit-2 object guard is the next target

Static disassembly of the matching AALib.exe identifies the additional guard immediately after the state-table gate:

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

Helper `0x00495B00` treats ECX as an object pointer and the stack argument as a bit index. With argument `2`, it forms mask `1 << 2 = 0x4`, reads DWORD `[object+0x3B70]`, and returns AL=1 iff that bit is set. The following `JNE` skips the setter when AL=1; the setter at `0x4C6502` is reached when AL=0.

This makes the observations consistent:
- The four Stage17 setter episodes that did proceed have `OtherObject3B70` values `0x440`, `0x640`, `0x460` and `0x640`; bit `0x4` is clear in every value.
- The 31 P2 index-`0x19` table-gate calls returned AL=1 but produced no setter call. A set bit `0x4` in the object checked by `0x495B00` is a strong candidate for the downstream rejection, but the current logs do not capture that helper's return or the full checked object value for those rejected calls.

**Next diagnostic should observe the result of the call at RVA `0x000C64F1`, without changing the helper, EAX/AL, stack cleanup, or the original `TEST/JNE`.** The cleanest placement is a call-wrapper at `0x4C64F1`: it calls the original `0x495B00`, records returned AL plus the pointer in `[EBX]` and its `+0x3B70` DWORD, then returns to the untouched `TEST AL,AL` at `0x4C64F6`. This will directly test whether the bit-2 guard explains the 31 AL=1/no-setter cases.

The table-gate question remains open too: no state index containing the on-disk `0xFF` sentinel was reached, so a real Stage18 AL=0 has not yet been observed.

## Stage19 completion status

Stage19's primary runtime objective is complete for this tested scenario: the new observer installed, recorded 1,235 helper results, and confirmed the known mapping patterns against the live Stage18 gate records. The Stage18–Stage16 suppression chain was also reproduced and correlated.

Limitations:
- Build.log is absent, so MSVC compiler/linker details and final DLL size are unavailable.
- No Stage18 AL=0 sample was captured.
- The new Stage19 logger's WORD flag readback occurs immediately after the helper returns; the helper's actual EAX remains authoritative if the entry mutates concurrently.
- No semantic labels are assigned to raw codes or WORD masks beyond the observed bit tests.

Preserve the Stage19 source branch and this evidence branch as immutable snapshots. Start the next diagnostic source stage from `stage19-bit113-state-map-trace`, not from this evidence branch.
