# Stage27 — CALL vs tail JMP on object-bit0 clear

**Verified for SHA256-pinned AALib.exe.** Complete detailed analysis in `VERIFIED_97970_BRANCH.md`.

```text
0x97609 CALL 0x97970   (argument pointer to player+0x30)
            |
            +-- state [object+0xD4] = 0 ---> 0x978B0; no 0x97870
            |
            +-- state = 1
            |   guard: [obj+0xC4]&1=0, [obj+0xCC]&7=0, [obj+0xC8]&1=1
            |       0x979CC CALL 0x97870 ---> return to 0x979D1 ---> 0x978B0
            |       Stage26: 34 clears (P1 16, P2 18)
            |
            +-- state = 2
            |   same guard
            |       0x979AC POP ESI; POP EBP; JMP 0x97870
            |       return goes directly back to original caller 0x9760E
            |       Stage26: 20 clears (P1 12, P2 8)
            |
            +-- state otherwise / guard failed ---> return without clear
```

In both cleared branches `0x97870` does `AND [obj+0xC8],0xFFFFFFF2`, writes `[obj+0xD4]=2`, calls three helpers, and sets `[obj+0xC4]|=0x400`. With obj=player+0x30, these fields correspond to player+0xF8, +0x104, +0xF4.

The user's 4,257-byte raw static report is preserved by its exact Git blob `d5d8c9f90665baf1c625bb65515910c2984b7a64`. Earlier Stage27 source/evidence branches are unchanged.

The 34/20 Stage26 stack groups are explained *without any added hook*, and both captured P2 payload0x1E events took the state1 nested-call route. Gameplay meaning and link to event suppression remain unverified.
