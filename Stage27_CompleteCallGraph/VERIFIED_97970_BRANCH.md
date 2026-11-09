# Stage27 — verified complete conditional call graph from AALib +0x97970

**Evidence captured:** 2026-11-09, user's second static PE report `Stage27_CallFlow_97970.txt`.
**Status:** static x86 control flow **confirmed** for the exact pinned AALib.exe build; 34/20 observed runtime return-address groups both explained. No game process modification or new DLL compilation.

## Provenance and verification

- Original local executable reported SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, matching Stage27's previously pinned original.
- Uploaded `Stage27_CallFlow_97970.txt` contains 4,257 bytes with Windows CRLF; **exact** Git blob SHA `d5d8c9f90665baf1c625bb65515910c2984b7a64`. It is preserved verbatim under `Stage27_CompleteCallGraph/Stage27_CallFlow_97970.txt`.
- Original Stage26 runtime blob `eeb87824a4cd10e573ef84ca124f93c9cd9b684c` in parent evidence. Cross-rechecked 140 events; 54 primary clear records (post-EIP RVA 0x9787A), 34 with `stack[1]` RVA `0x979D1` (all have `0x9760E` deeper at stack[4]) and 20 with `stack[1]` RVA `0x9760E`.
- Parsed the contiguous user's 0x97970–0x979DF code window and decoded 32-bit x86 instructions using `objdump -D -b binary -m i386 -M intel --adjust-vma=0x97970` against byte-for-byte hex extracted from the report. No Capstone or full game binary needed inside the assistant environment.
- Previous Stage27 report correctly verified CALL `0x97609 → 0x97970` (return 0x9760E) and CALL `0x979CC → 0x97870` (return 0x979D1), but could not explain the other 20 runtime stack returns. **The previously unknown edge is now statically verified as a tail `JMP`.**

## Exact decision tree at RVA 0x97970

The function takes its object-base argument at `[EBP+8]`, saved in ESI. For the Stage26 captures, ESI equals `player+0x30`; the offsets below therefore map to fighter offsets by adding 0x30.

1. **State switch:** `mov eax,[esi+0xD4]` at 0x97977, then comparisons against 0,1,2 (using `sub eax,0` and successive `sub eax,1`):
   - `state=0`: jump from 0x97980 to `0x979D1`, then call `0x978B0` (without clearing bit0 here).
   - `state=1`: jump at 0x97989 to `0x979B1`, where guard checks begin.
   - `state=2`: fall through at 0x9798F into another guard block.
   - Other state values: branch at 0x9798D to `0x979DA` and return without calling either helper.
2. **Shared clearing guard, evaluated in both state1 and state2 branches**:
   - `test byte ptr [esi+0xC4], 1` → reject when bit0 **set**;
   - `test byte ptr [esi+0xCC], 7` → reject if any of bits0–2 **set**;
   - `test byte ptr [esi+0xC8], 1` → reject when bit0 **clear**.
   - For observed fighter objects these correspond to `[player+0xF4]&1==0`, `[player+0xFC]&7==0`, and `[player+0xF8]&1==1`.
3. **State 1, guards pass: direct nested CALL**. `0x979CA: mov eax,esi; 0x979CC: call 0x97870`. The helper's post-write hardware exception sees return address RVA **0x979D1**. On return to 0x979D1, `push esi; call 0x978B0` at 0x979D2, `add esp,4`, then normal epilogue. **34** captured clears: P1=16, P2=18.
4. **State 2, guards pass: tail jump**. `0x979A8: mov eax,esi; 0x979AA: pop esi; 0x979AB: pop ebp; 0x979AC: jmp 0x97870` (5 bytes `E9 BF FE FF FF`). Because stack frame is removed and `JMP` does not push a return address, the caller's original RVA **0x9760E** remains at the top of the helper's call stack when its `PUSH EDI` executes. **20** captured clears: P1=12, P2=8. No post-clear `0x978B0` call on this route: the clearing helper's RET returns directly to RVA0x9760E.
5. The original clearer at RVA `0x97870` (both paths): `PUSH EDI; MOV EDI,EAX; AND DWORD PTR [EDI+0xC8],0xFFFFFFF2` (instruction 0x97873, trap after 0x9787A). It clears bits0,2,3, sets `[EDI+0xD4]=2`, invokes `0xF7EB0`, `0x976D0`, `0x99150`, sets bit0x400 in `[EDI+0xC4]`, and returns.

The `[ESI+0xD4]` state field is `player+0x104` in the captured code. As the clearer writes value 2 to this field, the state-1 path transitions it to 2 before the `0x978B0` call. Whether state values 1 and 2 have meanings such as animation/action phases requires game-level evidence and is not established merely by this function.

## Full 32-bit x86 disassembly of state dispatcher

```asm
97970: 55                   push ebp
97971: 8B EC                mov ebp,esp
97973: 56                   push esi
97974: 8B 75 08             mov esi,[ebp+8]
97977: 8B 86 D4 00 00 00    mov eax,[esi+0xD4]
9797D: 83 E8 00             sub eax,0
97980: 74 4F                je 0x979D1         ; state=0
97982: B9 01 00 00 00       mov ecx,1
97987: 2B C1                sub eax,ecx
97989: 74 26                je 0x979B1         ; state=1
9798B: 2B C1                sub eax,ecx
9798D: 75 4B                jne 0x979DA        ; state other than 0,1,2
; state=2
9798F: 84 8E C4 00 00 00    test byte [esi+0xC4],cl
97995: 75 43                jne 0x979DA
97997: F6 86 CC 00 00 00 07 test byte [esi+0xCC],7
9799E: 75 3A                jne 0x979DA
979A0: 84 8E C8 00 00 00    test byte [esi+0xC8],cl
979A6: 74 32                je 0x979DA
979A8: 8B C6                mov eax,esi
979AA: 5E                   pop esi
979AB: 5D                   pop ebp
979AC: E9 BF FE FF FF       jmp 0x97870     ; TAIL JUMP, original caller return preserved
; state=1
979B1: 84 8E C4 00 00 00    test byte [esi+0xC4],cl
979B7: 75 21                jne 0x979DA
979B9: F6 86 CC 00 00 00 07 test byte [esi+0xCC],7
979C0: 75 18                jne 0x979DA
979C2: 84 8E C8 00 00 00    test byte [esi+0xC8],cl
979C8: 74 10                je 0x979DA
979CA: 8B C6                mov eax,esi
979CC: E8 9F FE FF FF       call 0x97870    ; NESTED CALL, return 0x979D1
979D1: 56                   push esi
979D2: E8 D9 FE FF FF       call 0x978B0
979D7: 83 C4 04             add esp,4
979DA: 5E                   pop esi
979DB: 5D                   pop ebp
979DC: C3                   ret
```

## Event-specific proof

The *two* Stage26 source-P2 / target-P1 `0x1E` events, set seq1→clear seq2 and set seq87→clear seq88, both have primary-clear stack return `0x979D1`, so both followed the **state=1 nested CALL** path in this captured run. This is a specific source-backed classification of two events; it is not evidence that a `0x1E` event cannot take the state2 route at another time.

## What remains

- Function `0x978B0` is definitely called after the state1 clearing path and on state0. The existing text report contains only its initial part; its full effects are not yet determined.
- The business meaning of `player+0x104` states 0,1,2 and flag `player+0xF8` remains hypothetical.
- **No causal link** between this object-bit0 lifecycle and the separate `player+0x3B84` bit17 producer gate or Stage21 dispatcher fallback has been demonstrated. Avoid turning the now verified technical call graph into a gameplay claim.
- Next optional offline analysis: export a window for `0x978B0` and its helpers; determine whether its state-dependent effects contribute to P1/P2 action state or affect command processing. No new DLL runtime hook is required without a specific missing game-state observation.
