# Stage28 — original AALib.exe state rearm path, verified static analysis

**Date:** 2026-11-09. **Status:** verified original binary plus six passing local tests; **offline only**. No game hook, DLL build or new runtime capture.

## Source and provenance

- User explicitly reported their game executable in ChatGPT Library. Found Library archive `/AALib.zip`, containing `AALib.exe` (175,556,608 bytes), and materialized it to a **private local analysis container**. Decompressed bytes SHA-256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, exactly matching all Stage16/25/27 pinned evidence.
- x86 PE32, image base `0x00400000`, `.text` virtual RVA `0x1000` length `0x2BDEBE` (raw file offset `0x400`). Analysis reads only PE bytes/sections; never executes the game EXE.
- Complete 2.87MB raw .text section disassembled with local GNU `objdump -D -b binary -m i386 -M intel --adjust-vma=0x1000`. Targeted byte sequences were cross-checked with a stdlib Python PE-RVA verifier.
- Reproducibility: `Stage28_Analysis/tools/verify_stage28.py` (26 original code signatures, 16 rel32 CALL targets, 1 rel32 tail JMP, SHA256 guard), `Stage28_Analysis/tests/test_stage28.py`. Local equivalent verifier ran on the exact original binary: **PASS**, 26/16/1. Six unittest checks passed, including the real original PE. No original 175MB executable or zipped copy was committed to GitHub.

## Immediate function graph

```text
0x97609 CALL 0x97970
  |  state=[object+0xD4]  where object = player+0x30
  +-- state=0 ------------------------------> CALL 0x978B0
  +-- state=1, three guards pass
  |     0x979CC CALL 0x97870 (clear bit0, set state=2)
  |     0x979D2 CALL 0x978B0 (possible state1 re-arm)
  +-- state=2, same guards pass
        0x979AC JMP 0x97870 (tail jump, skips 0x978B0)

0x978B0:
  +-- if bit1 of [object+0xC8] set:
  |     clear bit1; check selected fighter's pair (type=0, codes0x13,0x14,0x15)
  |     if NONE match => 0x97958 CALL 0x97830 (state=1), return AL=1
  +-- otherwise / if matching pair:
        require object+0xC4 bit0=0 and object+0xCC bit1=0
        require state [object+0xD4]=0
        0x991E0 decrement countdown slot0, returns PRE-decrement value
        if previous counter<=0: CALL 0x97830 (state=1), return AL=1
        otherwise return AL=0
```

**Crucial:** `0x978B0` is an **action/state rearm predicate** as inferred from its actual writes and calls, not itself a queue producer. This is a **technical** description, *not* a known gameplay entity name.

## Exact 0x978B0 branch conditions (AALib RVAs)

1. Function entry at `0x978B0`, one pointer arg `object=[EBP+8]` saved in ESI. When non-null, reads `[object+0x108]` and `[object+0x10C]`, calls `0x9EEA0` at **`0x978CA`** to select a fighter object in EAX. Its `[EAX+0x3A2C]` is the pair-record header used below; the selected fighter may be the first or second player depending on source arguments.
2. `0x978D6 MOV ECX,[ESI+0xC8]` then `0x978DC TEST CL,0x2`. If bit1 is clear, bypass the special path and go straight to `0x97923`. If set:
    - `0x978E9 AND ECX,0xFFFFFFFD` and `0x978EF MOV [ESI+0xC8],ECX`: consume/clear bit1.
    - `0x978F5 CALL 0xF2ED0` with pointer to selected fighter `+0x3A2C`, type0, code **0x13**; if comparator true, go to common gates.
    - If false, `0x97906 CALL 0xF2ED0` for type0/code **0x14**; if true, go to common gates.
    - If false, `0x97917 CALL 0xF2ED0` for type0/code **0x15**; if true, go to common gates.
    - If **all three are false**, `0x97921 JE 0x97956` bypasses the common state0/countdown gates and goes directly to state1 rearm `0x97958 CALL 0x97830`.
3. **Common gates `0x97923–0x97954`:** require `[object+0xC4]&1==0`; `[object+0xCC]&2==0`; `[object+0xD4]==0`. Otherwise `0x97935` returns AL=0.
4. `0x97944 LEA EDX,[object+0x10]`; `0x9794A CALL 0x991E0(object+0x10, index0)`. It returns the **old** counter slot value and reduces the stored value by the slot's first DWORD (clamping negative result to zero). `0x97954 JG 0x97935`: if old counter **positive**, return AL=0; if old counter **zero or negative**, fall into state1 restart.
5. `0x97958 CALL 0x97830` with EAX=object. On return the function explicitly `MOV AL,1` at `0x9795E`, returns AL=1.

For object=player+0x30 in Stage26: `[object+0xC8]` is `player+0xF8`; `[object+0xC4]` is `player+0xF4`; `[object+0xCC]` is `player+0xFC`; `[object+0xD4]` is `player+0x104`. The pointer selection fields become player+0x138/+0x13C. The countdown data base at object+0x10 is player+0x40; slot0 is player+0x44.

### 0xF2ED0 is a comparison, NOT queue insertion

Verified `0xF2ED0` bytecode reads `WORD [record+8]` and `WORD [record+0xA]`, sign-extends each, compares against the two integer args, returns EAX=1 for exact match and EAX=0 otherwise. Stage28's three calls therefore ask whether selected player's `WORD [player+0x3A34], WORD [player+0x3A36]` match `(0,0x13)`, `(0,0x14)`, `(0,0x15)`.

The research project's queue **writer** is the separate `AALib+0xF2DE0`. **No direct queue insertion `F2DE0` occurs inside function `0x978B0` or `0x97830`.** Do not interpret `0xF2ED0` as producer or 0x13/14/15 as new queued events. These code values are currently unlabeled.

### 0x991E0 countdown mutation

```asm
991EA: MOV EAX,[EDX+ESI*4+4]    ; old slot value (ESI=0)
991EE: MOV ECX,EAX
991F0: SUB ECX,[EDX]             ; subtract current base/stride
991F2: MOV [EDX+ESI*4+4],ECX     ; store decremented
991F6: JNS 99200
991F8: MOV [EDX+ESI*4+4],0       ; clamp if negative
99200: POP ESI
99201: POP EBP
99202: RET                        ; EAX still OLD value
```

Thus, checking `EAX>0` at 0x97954 **does not** test the newly updated value. It tests the old slot0 value, and the next call may then start state1. No time units or gameplay label for the counter have been established.

### 0x97830 and 0x97870: related state setters

| Operation | State-1 helper `0x97830` | State-2 clearer `0x97870` |
| --- | --- | --- |
| `[object+0xD4]` | write `1` at 0x97839 | write `2` at 0x97880 |
| `[object+0xC8]` | leaves untouched | `AND 0xFFFFFFF2` at 0x97873 (clears bits0,2,3 but **preserves bit1**) |
| `0xF7EB0(object+0x10,0)` | CALL at 0x97843 | CALL at 0x9788A |
| `0x976D0(arg)` | arg `0`, CALL at 0x9784A | arg `1`, CALL at 0x97891 |
| `0x99150(object)` | CALL at 0x97850 | CALL at 0x97897 |
| `[object+0xC4]` | OR `0x400` at 0x97855 | OR `0x400` at 0x9789C |

`0x99150` itself checks the object state and calls `0x99040` when nonzero. The deeper effects of `0x99040` are not exhaustively classified here.

Because `0x97870` **preserves bit1**, an already-pending bit1 can survive state2 transition and later lead `0x978B0` to rearm state1 *if* none of the three pair-record comparisons succeeds. This is a code-verified conditional possibility, **not an observed Stage26 transition**.

## Stage26 runtime comparison (important negative evidence)

Re-read full Stage26 bit0 write trace, root Git blob `eeb87824a4cd10e573ef84ca124f93c9cd9b684c`. The 140 recorded transition entries have **only full DWORD** transitions: `0x00000000→0x00000001` 70 times and `0x00000001→0x00000000` 70 times. No bit1 appears in any trapped old/new value.

**Limitation:** Stage26 default `Stage26TransitionsOnly=1` suppresses *writes that do not change bit0*. Therefore this log **cannot prove** that bit1 was never set or cleared between the captured transitions, nor can it establish that 0x978B0's special path never ran. It only constrains the captured bit0 boundary observations. A future Stage29 could instrument the bit1 writer only if its dynamics materially affect the original COM-vs-COM question.

## Direct control-flow xrefs

A whole-.text GNU objdump linear-sweep pass found the following direct references around the examined state machine:

| Destination | Direct references observed |
| --- | --- |
| `0x97970` | `0x97609 CALL` |
| `0x97870` | `0x979CC CALL`, `0x979AC JMP` |
| `0x978B0` | `0x979D2 CALL` |
| `0x97830` | `0x97958 CALL` |
| `0xF2ED0` from `0x978B0` | `0x978F5`, `0x97906`, `0x97917` |
| `0x991E0` from `0x978B0` | `0x9794A CALL` |

These are verified x86 rel32 instruction bytes at actual function boundaries. The whole-.text scan is **not** a proof that no indirect calls or computed jumps exist.

## Conclusions and next checkpoint

1. **Stage28 technical question answered**: `0x978B0` is a conditional state1 activator and bit1-consumer with a three-code record guard, not itself an enqueue writer. `0x991E0` mutates a countdown.
2. The discovered state lifecycle is consistent with the previous Stage27: state0 may start state1, state1 may clear bit0 to state2 and then invoke 978B0, while state2 follows a tail-jump route that **skips 978B0**.
3. No direct evidence yet links these object state transitions to **bit17 on player+0x3B84**, producer gate `0xF6A68`, or Stage21's dispatcher fallback. The two observed P2 payload0x1E events reached the queue and fallback as before; they do not prove the absence/presence of a causal relationship.
4. Next potential **Stage29**: use proven bit17/queue datasets to focus on **why payload0x1E reaches classifier AL0 and fallback slot3=0**, rather than continue mapping unrelated object flags without a specific hypothesis. Alternatively, a no-change-write Stage26 variant for bit1 only if justified. Do not add new game hooks without a bounded question.
