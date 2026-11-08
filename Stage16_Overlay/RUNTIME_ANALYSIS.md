# Stage16 runtime analysis — 2026-11-08

## Run integrity

Runtime log window: `2026-11-08 13:02:27.719` through `13:03:18.651`.

The uploaded branch contains the full Stage16 player-field trace (30,114 `PLAYER_BIT17_WRITE` records), Stage15 producer-call trace, alternate-callsite trace, QueueTrace, EventDispatchTrace, FinalCommandTrace, ParentFlowTrace, and ProducerContextTrace.

**Build.log was not uploaded.** The compiler transcript, warnings, and exact build command therefore cannot be independently reviewed. However, the runtime log confirms that the Stage16 module was active in the running process and that hardware watchpoints were installed.

Stage16 startup record:
- P1 player: `0x0E9C1F78`; watched DWORD: `0x0E9C5AFC` (AALib RVA `0x0DCA5AFC`); initial value `0x00000000`.
- P2 player: `0x0E9ECCA0`; watched DWORD: `0x0E9F0824` (AALib RVA `0x0DCD0824`); initial value `0x00000000`.
- DR7 configuration: `0x00DD0005` (local, write-only, four-byte DR0/DR1 data breakpoints).
- Eight watchpoint installation batches installed 60 game-thread watchpoints in total.
- No skipped-thread, dropped-record, or error/warning records were found in the Stage16 log.

## Confirmed bit-17 setter

The write watchpoint caught the same instruction for the five observed 0→1 bit-17 transitions.

The log reports `eipAfter=0x00D2FAD2`, so the writing instruction ends at RVA `0x0000FAD2`; disassembly shows the instruction begins at RVA `0x0000FAD0`:

```asm
0040FAD0  or dword ptr [eax], edx
```

At the five bit-17 transitions:
- EAX was the exact watched field address (`0x0E9C5AFC` for P1 or `0x0E9F0824` for P2).
- EDX was `0x00020000`.
- The result was a real 0→1 transition of bit 17.

This instruction belongs to a generic player bitfield setter at VA `0x0040FAA0` (RVA `0x0000FAA0`). Its input is a bit index:
- Base of the relevant bitfield array: `player+0x3B78`.
- Index `0x71` (113) selects DWORD number 3: `player+0x3B78+3*4 = player+0x3B84`.
- The index remainder is 17, producing mask `1 << 17 = 0x00020000`.
- The helper then executes `OR [EAX],EDX`.

The Stage16 trace also caught other calls through this helper that used EDX=`0x00400000` (bit 22 in the same watched DWORD). Those are not bit-17 transitions. For RVA-after `0xFAD2`, 85 writes were observed: 10 with EDX=`0x00020000` (five 0→1 transitions and five repeated 1→1 writes) and 75 with EDX=`0x00400000`.

### Static caller candidate for bit index 113

Among 129 direct call references to the generic setter at `0x0040FAA0`, one call site passes the literal bit index `0x71`:

```asm
004C64ED  mov ecx, dword ptr [ebx]
004C64EF  push 2
004C64F1  call 00495B00
004C64F6  test al, al
004C64F8  jne 004C640B
004C64FE  push 0x71
004C6500  mov ecx, esi
004C6502  call 0040FAA0
```

The helper at `0x00495B00` is itself a generic bit-test helper. Here it tests bit index 2 in the `+0x3B70` bitfield of the object passed in ECX. If that helper returns `AL=0`, the caller sets bit index 113 on the player object in ESI.

This is the only direct call reference found with a literal `push 0x71`. It is a strong static candidate for the runtime setter, but **the Stage16 record did not capture the setter's caller return address**. Do not claim that the runtime write was definitively issued by `0x004C6502` until a follow-up callsite/entry trace verifies it.

## Confirmed bit-17 clearer

The five 1→0 transitions all came from the instruction beginning at RVA `0x000ED19F`. The logged EIP-after is `0x00E0D1A9`, exactly ten bytes after the instruction start:

```asm
004ED188  push 0x1E
004ED18A  lea eax, [esi+0x3A2C]
004ED190  push 1
004ED192  push eax
004ED193  call 004F2ED0
004ED198  add esp, 0x0C
004ED19B  test al, al
004ED19D  jne 004ED1A9
004ED19F  and dword ptr [esi+0x3B84], 0xFFF9FFFF
004ED1A9  mov eax, [esi+0x3978]
```

For every transition:
- ESI matched the exact P1/P2 player object.
- The old/new DWORD changed from a value with bit17=1 to a value with bit17=0.
- The preceding `F2ED0` helper returned `AL=0`.
- The mask `0xFFF9FFFF` clears bits 17 and 18. In these five transitions, bit17 was the relevant bit that changed.

There were 597 executions observed at this clear instruction, but only five changed bit17 from 1 to 0; the other executions did not change bit17.

## Correlation with the Stage15 decision path

The Stage15 branch trace in this run contains six matching parent events. For all six:
- `F2F50` returned `EAX=0x11E`.
- `F2ED0` returned `AL=0`.
- `EDI=0x1E` passed the payload comparison at F6A63.

Five events had bit17 set at F6A68. In each case:
1. The generic setter wrote bit17=1 before the branch check.
2. F6A68 observed bit17=1; the original `JZ` was not taken (`branchTaken=0`).
3. The event did not reach F6A9D or the alternate queue call at F6AD7.
4. The clear instruction at RVA `0xED19F` then reset bit17 to 0.

The sixth event (P1 at `13:03:12.258`) arrived with bit17 already 0:
1. F6A68 took its `JZ` branch (`branchTaken=1`).
2. F6A9D was reached.
3. The call at F6AD7 inserted `type=1, payload=0x1E`.
4. QueueTrace reported `ACCEPTED_BY_QUEUE`.
5. EventDispatchTrace confirmed the same `type=1, payload=0x1E`.

No matching `payload=0x1E` event appears in FinalCommandTrace because this successful event used the separate alternate writer at F6AD7, not the F7C5D final-command callsite.

## Transition timeline

| Time | Side | Bit17 setter | F6A68 result | Bit17 clear |
|---|---|---|---|---|
| 13:02:52.873 | P1 | 0→1 | Set; JZ not taken | 1→0 |
| 13:02:58.590 | P1 | 0→1 | Set; JZ not taken | 1→0 |
| 13:02:58.730 | P2 | 0→1 | Set; JZ not taken | 1→0 |
| 13:03:04.799 | P1 | 0→1 | Set; JZ not taken | 1→0 |
| 13:03:08.724 | P2 | 0→1 | Set; JZ not taken | 1→0 |
| 13:03:12.258 | P1 | 0 | Clear; JZ taken | No transition |

## Conclusion and next research target

Stage16 closes the core writer-identification question for this run:
- The bit-17 setter is the generic bitfield OR at RVA `0xFAD0`, reached through the helper at RVA `0xFAA0`.
- The bit-17 clearer is the direct AND at RVA `0xED19F`.
- Their timing explains why five of six type-1/`0x1E` parent events were stopped at F6A68, while the one event without bit17 reached the alternate queue and dispatcher.

What remains unknown is the full semantic meaning of bit index 113 and which runtime callsite actually issued each setter invocation. The literal-`0x71` caller at `0x4C6502` is the strongest static candidate. The next diagnostic step should capture the return address, ECX player pointer, stack argument, and surrounding state when the generic setter at `0x40FAA0` is called with index `0x71`. This will confirm or reject the `0x4C6502` caller and expose the state condition responsible for setting the flag.

Stage16 is runtime-evidenced, but the uploaded branch does not contain Build.log; compiler output remains unreviewed.
