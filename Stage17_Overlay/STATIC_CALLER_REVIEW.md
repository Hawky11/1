# Stage17 static caller candidate for bit index 0x71

## Generic setter

The setter entry is VA `0x0040FAA0` / RVA `0x0000FAA0`. Its first seven bytes are:

```text
55 8B EC 56 8B 75 08
```

The setter uses ECX as the player object and the DWORD argument at entry `[ESP+4]` as the bit index. It divides the index by 32 to select a DWORD relative to `player+0x3B78), then sets one bit within that DWORD. For index `0x71`:
- DWORD index = `0x71 / 32 = 3`.
- Bit position = `0x71 % 32 = 17`.
- Destination = `player + 0x3B78 + 3*4 = player+0x3B84`.
- Mask = `1 << 17 = 0x00020000`.

The final setter instruction at VA `0x0040FAD0` is:
```asm
or dword ptr [eax], edx
```

## Candidate caller at VA 0x004C6502

```asm
004C64ED  mov ecx,[ebx]
004C64EF  push 2
004C64F1  call 00495B00
004C64F6  test al,al
004C64F8  jne 004C640B
004C64FE  push 0x71
004C6500  mov ecx,esi
004C6502  call 0040FAA0
```

The helper at `0x00495B00` tests bit 2 in the DWORD at `[object+0x3B70]`. The setter call occurs only when AL=0. The intervening PUSH/MOV/CALL instructions do not modify EFLAGS, so the expected setter-entry ZF is 1.

Expected runtime call metadata:
- caller CALL VA: `0x004C6502`; RVA: `0x000C6502`.
- return VA: `0x004C6507`; RVA: `0x000C6507`.
- call opcode: `E8 rel32`; decoded destination must be `0x0040FAA0`.
- index argument: `0x71`.
- ECX: player object; expected to match ESI at this call site.
- entry ZF: expected 1 when the preceding helper returns AL=0.

The Stage17 runtime trace must confirm or falsify this candidate for each actual bit-113 setter call. Other callers may use a register/computed bit index and are not ruled out by the static literal-push search.
