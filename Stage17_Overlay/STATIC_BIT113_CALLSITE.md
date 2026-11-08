# Static analysis: bitset setter and direct bit-113 caller

## Generic setter at VA 0x0040FAA0 / RVA 0x0000FAA0

The function takes:
- ECX = player object
- `[EBP+8]` = zero-based bit index

It computes:
- DWORD address = `player + 0x3B78 + ((index >> 5) * 4)`
- mask = `1 << (index & 31)`
- performs `OR [DWORD address], mask`

For index `0x71` (113):
- `index >> 5 = 3`, so DWORD address = `player + 0x3B78 + 12 = player+0x3B84`
- `index & 31 = 17`, so mask = `0x00020000`

The final setter instruction is VA `0x0040FAD0` / RVA `0x0000FAD0`:

```asm
or dword ptr [eax], edx
```

This exactly explains the Stage16 runtime registers on all five 0→1 transitions: EAX was the observed P1/P2 field address and EDX was `0x00020000`.

## Direct callsite scan

An objdump scan of the verified AALib.exe .text section found 129 direct calls to the generic setter at `0x0040FAA0`. Only one direct callsite in that scan was preceded within the local instruction window by an immediate `push 0x71`:

VA `0x004C6502` / RVA `0x000C6502`, in a function beginning at VA `0x004C63B0` / RVA `0x000C63B0`.

```asm
004C64ED  mov ecx, [ebx]
004C64EF  push 2
004C64F1  call 00495B00
004C64F6  test al, al
004C64F8  jne 004C640B
004C64FE  push 71h
004C6500  mov ecx, esi
004C6502  call 0040FAA0
004C6507  mov ecx, [ebx]
```

The original CALL bytes at RVA `0x000C6502` are `E8 99 95 F4 FF`, which resolve to VA `0x0040FAA0`. Stage17 validates both the exact bytes and decoded target before patching.

## Surrounding gates

The caller takes its player object in ESI. Earlier instructions in the function check several conditions, including:
- the player fields at `+0x3A34` and `+0x3A36`;
- helper calls to `0x00401F30` with small state indices;
- helper `0x004F6ED0`, followed by range checks on its EAX result;
- helper `0x004F2FD0` with the player, that result, and `0x19`, whose AL result determines whether the later path continues;
- the final `0x00495B00` call with ECX=`[EBX]` and bit index 2. That helper reads a bitset beginning at `[relatedObject+0x3B70]`; if it reports the bit set, the caller exits. If it reports clear, the caller invokes SetBit for player bit 113.

The function has additional earlier branches; these notes do not assign semantic names to its state fields or helper return values.

## Stage17 purpose

Static disassembly makes `0x004C6502` the best direct callsite candidate for the Stage16 bit-17 set operations, but the Stage16 log captures the generic setter instruction rather than its caller. Stage17 records the caller stack/registers and player/context fields before tail-calling the original helper.

A runtime match between Stage17 `BIT113_SET_CALL` records and Stage16 `RVA 0x0000FAD2` writes is required before declaring this caller confirmed. If no matching call appears, other call paths may pass a dynamically computed bit index.
