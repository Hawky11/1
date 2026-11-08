# Stage18 static analysis — bit-113 state-table gate

## Binary identity

All addresses below were rechecked against the uploaded AALib.exe used by Stages 16–17:

- PE32 / Intel 386
- ImageBase: 0x00400000
- SizeOfImage: 0x0E0EB000
- SHA-256: 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba

Exact raw bytes at the Stage18 observer points:

| RVA | Bytes | Decoded instruction |
|---|---|---|
| 0x000C64A0 | E8 2B CB 02 00 | CALL 0x004F2FD0 |
| 0x000C64A5 | 83 C4 0C 84 C0 | ADD ESP,0x0C; TEST AL,AL |
| 0x000C64AA | 75 41 | JNE 0x004C64ED |

The Stage18 observer replaces only the five bytes at 0x000C64A5; the call to 0x4F2FD0 remains intact and the original conditional branch at 0xC64AA remains in place.

## Why this is the relevant gate

The Stage17-confirmed setter call is at 0x004C6502. Its function contains this preceding path:

    004C649C  push 19h
    004C649E  push eax       ; state index returned by 0x004F6ED0
    004C649F  push esi       ; player
    004C64A0  call 004F2FD0
    004C64A5  add esp,0Ch
    004C64A8  test al,al
    004C64AA  jne 004C64ED

If AL=1, the branch to 0x4C64ED bypasses the direct state comparisons at 0x4C64B1–0x4C64D6 and reaches the final related-object bit-2 guard at 0x4C64F1. Stage17 captured player+0x3A36 low words 0x42, 0x22, 0x20 and 0x5E, none of which match the direct comparison set {0x1A, 0x18, 0x19}. This makes the 0x4F2FD0-accepted branch the explanation consistent with the Stage17 calls, but Stage17 did not capture the helper's AL.

If AL=0, this branch returns through the epilogue at 0x4C64AC without reaching the bit-113 setter from this path.

## Helper 0x004F2FD0

The function receives three stack parameters:

- [EBP+8]: player object
- [EBP+0x0C]: state index
- [EBP+0x10]: byte/key offset, passed as 0x19

It selects a state table, calculates table + stateIndex*0x1A + key (the caller pushes key 0x19), and returns AL=1 iff the byte at that location is not 0xFF:

    004F3041  mov ecx,[ebp+0Ch]
    004F3044  mov edx,[ebp+10h]
    004F3047  imul ecx,ecx,1Ah
    004F304A  add ecx,edi
    004F304C  cmp byte ptr [ecx+edx],0FFh
    004F3050  pop edi
    004F3051  setne al
    004F3054  pop esi
    004F3055  pop ebp
    004F3056  ret

Thus, for the call used by the 0x4C63B0 caller:

- AL=0 when table[stateIndex*0x1A + 0x19] == 0xFF.
- AL=1 when that byte is any value other than 0xFF.

This is a data-driven state-table gate. The binary does not name the gameplay meaning of the byte, so no semantic label is guessed here.

## Table selector

Helper 0x004E6E30 returns true only when all three conditions are satisfied:

1. DWORD [player+0x3978] == 0x1D;
2. bit 0 of byte [player+0x3B86] is set (equivalent to bit 16 / mask 0x00010000 in DWORD [player+0x3B84]);
3. bit 28 / mask 0x10000000 in DWORD [player+0x3B88] is set.

When true, selector 0x16 is used; otherwise selector DWORD [player+0x3F20] is used.

Primary table-pointer array:
- VA 0x004C82518
- RVA 0x04882518

If bit 25 / mask 0x02000000 in DWORD [player+0x3B80] is set, the helper also checks the secondary pointer array:
- VA 0x004C82C20
- RVA 0x04882C20

A nonzero secondary entry replaces the primary table pointer. A zero secondary entry leaves the primary pointer in use. The helper rechecks the 0x4E6E30 selector when resolving the secondary entry; it is the same predicate.

## State index source

The caller obtains EAX from 0x004F6ED0 (a wrapper around 0x004F6BA0), then rejects -1 and values >=0x1C before passing EAX to 0x4F2FD0. The observer records the exact stack argument instead of assigning semantic names to the index.

## Stage18 observer contract

The observer is placed at the return site 0x004C64A5, not inside 0x4F2FD0. It records:

- AL and full EAX returned by the helper;
- the exact player, state index and key arguments still on the caller stack;
- caller EBP and its parent-frame values;
- the selector-driving player fields;
- primary and secondary table pointers, selected pointer, calculated byte address and its safely-read byte value.

It independently reproduces the original stack cleanup (ADD ESP,0x0C) plus the observer CALL's extra return address (ADD ESP,0x10), then executes TEST AL,AL and jumps to the original JNE at 0x4C64AA. The hook does not synthesize or modify AL, the table byte, or the branch result.

The table-byte read is diagnostic only. It is bounded to selector values 0–255 and state indices 0–27, and uses ReadProcessMemory; a failed or out-of-range read is represented by byte=0xFFFFFFFF / an incomplete readMask. The helper's actual AL remains the authoritative runtime result.

## Coverage limitation

This observer covers only calls through 0x4C64A0 / 0x4F2FD0. The alternative direct comparison route beginning at 0x4C64B1 bypasses this helper and will not generate a Stage18 record. Keep Stage17 enabled to determine whether each bit-113 setter call came from the already-confirmed 0xC6502 site.

No semantic label is assigned to selector values, table entries, or states without separate evidence.
