# Stage18 runtime analysis — bit-113 gate confirmed (2026-11-08)

## Evidence snapshot

This branch was created from the user's 2026-11-08 upload commit on main and combines the uploaded runtime logs with the exact Stage18 source overlay. The original source-only branch stage18-bit113-gate-condition-trace remains unchanged.

Runtime start: **2026-11-08 18:00:15**. Gameplay trace activity: **18:00:34–18:00:49**.

The following markers are present:

- STAGE18_BIT113_GATE_BUILD=2026-11-08-r1
- STAGE18_INIT: FieldTraceThread entered.
- STAGE18_CONFIG: EnableStage18Bit113GateTrace=1 EnableStage17Bit113SetterCallTrace=1 EnablePlayerBit17WriteTrace=1.
- Installed Stage18 passive gate-result observer at RVA=0x000C64A5, target helper RVA=0x000F2FD0, with 44 suspended threads.
- Stage17 r2 marker and successful Stage17 installation at RVA 0x000C6502, targeting the generic setter at RVA 0x0000FAA0, also with 44 suspended threads.

The runtime markers and both installation messages prove that the diagnostic DLL was loaded and both hooks installed. **A Build.log was not included in the upload**, so the exact MSVC compiler/linker output and DLL size cannot be independently recorded from this snapshot.

## Stage18 gate results

ArcanaAI_Bit113GateTrace.log contains six BIT113_GATE_RESULT records. Every record has resultAL=1 and the post-return diagnostic table readback is 0xFA, not 0xFF. All have key 0x19, special=0, and usedSecondary=0.

| Gate seq | Player | State index | Selector | Table byte address | Readback |
|---:|---|---:|---:|---:|---:|
| 1–2 | P1 | 0x0E | 0x0A | 0x0559DBDD | 0xFA |
| 3–4 | P2 | 0x19 | 0x17 | 0x055A073B | 0xFA |
| 5–6 | P1 | 0x18 | 0x0A | 0x0559DCE1 | 0xFA |

The actual helper return AL is authoritative. The byte is reread after the helper returns, so the byte is supporting corroboration and could theoretically differ if the game changed the table concurrently. In this run, all six readbacks agree with AL=1.

## Direct caller and actual bit17 writes

ArcanaAI_Bit113SetterTrace.log contains six calls. All six use:

- call RVA 0x000C6502;
- return RVA 0x000C6507;
- bit index 0x71 (113);
- the matching P1/P2 player object.

ArcanaAI_PlayerBit17WriteTrace.log contains **15,094** PLAYER_BIT17_WRITE records and six actual bit17 transitions: three sets and three clears. All three 0→1 writes occur at post-EIP RVA 0x0000FAD2, with EDX=0x00020000, and all three clears occur at post-EIP RVA 0x000ED1A9.

| Episode | Player | Gate→first setter | Setter→0→1 write | 0→1 write→F6A68 | F6A68→clear |
|---:|---|---:|---:|---:|---:|
| 1 | P1 | 136 QPC ticks | 342 | 2,015 | 1,572 |
| 2 | P2 | 135 QPC ticks | 343 | 1,974 | 1,767 |
| 3 | P1 | 169 QPC ticks | 441 | 2,537 | 1,989 |

For each episode, Stage17 records a second helper-gated call and a second setter call after bit17 is already set. The repeated calls occur 160, 162, and 207 QPC ticks after the corresponding 0→1 write; the repeated gate-to-setter intervals are 124, 123, and 158 ticks. The snapshots therefore show the first call setting bit17 and the second call repeating the idempotent OR.

## Why there are two calls per episode

Stage18's caller-frame records split consistently into two parent return addresses:

- Sequences 1, 3, and 5: parent return 0x00DB6801 (RVA 0x00096801; static AALib VA 0x00496801). The verified disassembly shows a direct CALL 0x004C63B0 at VA 0x004967FC, followed by return address 0x00496801.
- Sequences 2, 4, and 6: parent return 0x00DEB9FA (RVA 0x000CB9FA; static AALib VA 0x004CB9FA). The verified disassembly shows this is the return address after CALL EDX at VA 0x004CB9F8 inside a function beginning at 0x004CB9C0. That function dispatches through a function-pointer table at 0x0428AEF0.

Both call paths invoke the same caller function at 0x004C63B0, which runs the 0x4F2FD0 gate and reaches the 0xC6502 bit-113 setter. This explains the two calls in each episode: one direct invocation and one invocation through the function-pointer callback path. The second invocation is not a second 0→1 state transition.

## The targeted payload-0x1E suppression is reproduced

The Stage15 branch records contain three F6A68_PLAYER_BIT17_BRANCH events, one for each episode. At each event:

- EDI=0x0000001E (the targeted payload);
- bit17=1;
- ZF=1;
- branchTaken=0 for the original conditional branch.

There are exactly nine Stage15 records for the three target attempts: one each at F6A5D, F6A63, and F6A68. There are **zero** records at F6A9D or F6AAC, and the alternate callsite hook at F6AD7 installed but recorded no actual call.

QueueTrace, EventDispatchTrace and FinalCommandTrace each contain 27 type-1 records for other payloads. None has payload 0x1E. Thus the trace distinguishes ordinary queue traffic from the target event: the target payload-0x1E attempts reached the bit17 branch but were suppressed before the later guards and alternate queue writer.

## Conclusion

Stage18's first runtime objective is achieved for this tested scenario:

1. 0x4F2FD0 returned AL=1 in all six observed calls; its selected table byte read back as 0xFA.
2. Those calls all fed the already-confirmed Stage17 callsite 0xC6502 with bit index 0x71.
3. The first call in each of three episodes set P1/P2 bit17; a second invocation through a distinct parent call path repeated the setter while the bit was already set.
4. The same three episodes then reached Stage15 with payload 0x1E, where bit17 was already set and the original branch prevented further processing.

This run does **not** show an AL=0 table-gate case, a successful payload-0x1E insertion, or prove the table's semantic meaning. It does establish that the 0x4F2FD0 gate did not reject the three observed target attempts; the later bit17 condition is the decisive suppression point in this run.

## Uploaded runtime files

The root of this branch contains the uploaded ArcanaAI_*.log files. Build.log was not uploaded.
