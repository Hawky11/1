# Stage35 — downstream controller consumers in the original AALib.exe

**Date:** 2026-11-10. **Status:** static research verified on the original SHA-pinned binary. A Stage35 read-only runtime observer is source-only until a user performs Windows x86 Build.bat and natural COM-vs-COM verification. No successful attack/movement or AI defect is claimed.

## Input / provenance

Stage34's completed [runtime evidence](../Stage34_Runtime/RUNTIME_ANALYSIS.md) (branch `stage34-type0-variant-runtime-evidence` @ `ad801c2278f81cdb997d5773207efecb4211af65`) contains **109 valid EE200 pre/post records**, and **33/33** queue→EE200 correlations; payload 0/1/4 were observed, payload5 was **not**. For 8/12 payload4 calls, slot `P+3A6C` ended at `0x00000F00`. These data establish controller-state mutations, not moves.

Exact unmodified game binary was obtained **read-only from ChatGPT Library `/AALib.zip`**, not reuploaded: `AALib.exe`, size 175,556,608 bytes; SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, x86 PE32, ImageBase `0x400000`. All code addresses below are **RVA**, not file offsets; use PE section mapping. [Verifier](tools/verify_stage35.py) passed on the real EXE: **19 exact opcode signatures, 7 E8 direct CALL targets**, and expected slot index/offset. No executable or its disassembly is published.

## Downstream path, after native EE200 state updates

A real main/update path runs `A1106 CALL ACA90`; `ACA90` iterates per-side entities, and in this order:

```asm
ACA90  ; iterate per-side objects via 9EE10 / 9EEA0
ACAB2  call E6EC0
ACAB8  call F5990
ACABD  push esi                ; per-side player P
ACABE  call F60C0              ; process/decrement controller slots
ACAC3  push esi
ACAC4  call F5BD0              ; derive player + object flags
ACAC9  mov esi, [esi+4]        ; transition to O
ACACE  call B86A0              ; additional object-dependent gating
```

This establishes the **order of consumption**, not an exact temporal parent relation to a particular `EE200` event; correlate future runtime records using side, address, thread and QPC, not made-up event IDs.

### Slot P+3A6C is time/state storage — not an attack opcode

`F60C0`, reached by the first wrapper candidate `ACABE`, contains:

```asm
F610E  xor ebx, ebx
F6110  push ebx                ; zero
F6111  push 1
F6113  lea edi, [esi+3A3Ch]
F6119  push 0Bh                ; slot index 11
F611B  push edi
F611C  call F7ED0              ; writes adjusted slot value
F6124  test eax, eax
F6126  jle F613B
F6128  cmp byte ptr [esi+15Dh], bl
F612E  je F613B
F6130  push 0Bh
F6132  push edi
F6133  call F7EB0              ; clears slot on additional condition
```

`F7ED0` decrements a slot by the structure's `0x78` (120) step for argument 1 and clamps nonnegative; `F7EB0` zeroes the selected slot. Stage34's `0xF00` equals 3840 = 32×120, **but this is not proof of 32 completed gameplay frames or an action duration**: invocation rate, special clear and other transitions must be observed.

`F5BD0` later directly tests this slot:

```asm
F5F4C  cmp dword ptr [esi+3A6Ch], 0
F5F53  jle F5F5F
F5F55  or dword ptr [esi+3B78h], 80h
```

**Important ambiguity:** at `F5C0C` in the **same routine** an earlier unconditional `OR [esi+3B78h], 0x380` already sets bit7, and `F5C71` can clear bit7 later in the routine. Therefore, the conditional `F5F55` is a native read-to-flag branch, but a **before/after observation of P+3B78 alone cannot prove that F5F55 executed**, and does not identify an action.

### Controller bitmap P+3A0C/+3A10 is also read by native logic

`F58E0(output=P+3A08,index)` tests bitmaps at `[output+4]` (and indexed words), returning a boolean in AL; special index `0x26` checks masks `0x200/0x400/0x800`. Verified consumer:

```asm
EB7D0  lea eax, [edi+3A08h]
EB7D6  push 1Eh
EB7D8  push eax
EB7D9  call F58E0
EB7E1  test al, al
EB7E3  je EB7F0
EB7E5  call E5F00       ; further predicate, not validated as action success
```

Other static `F58E0` callers include `EB718`, `F3129`, `F313C`, `126DF9`, `126F01` and `15593A`. `F58E0` returning true means a bit set, not that an attack, guard or movement happened.

## Stage35 bounded passive observational experiment

A **two-callsite, same-frame** observer (if enabled) should wrap original `ACA90→F60C0` at `ACABE` and `ACA90→F5BD0` at `ACAC4` to log exact P1/P2 before/after snapshots: `P+3A08/0C/10/6C`, `P+3B78/3B70/3ABC/3F64/3A3C/3B84`, `O+08/10`, with 12-bit read masks and QPC on each side. This can show whether a freshly set slot was consumed, whether a flag-recompute routine ran, and any net object changes. It **cannot** by itself distinguish every internal branch or prove an on-screen action. Do not install a hook at unvalidated byte positions, modify native arguments, or synthesize payload5 / type1 payload0x1E.

Runtime correlation checklist:
1. Validate the **correct Stage35 marker, both callsite installations**, x86 build and no positive dropped/overflow.
2. For same side/P and TID, order `Stage34 EE200 exit QPC` → `Stage35 TIMER (ACABE)` → `Stage35 FLAGS (ACAC4)`; use strict time ordering and require P/address match. There is **no inherited cross-file sequence ID**.
3. Compare timer `P+3A6C` before/after, both-read mask required; distinguish decrement vs zeroing, and acknowledge possible intermediate writes.
4. Compare flags `P+3B78`, object `O+08/10`; **net changes alone are not branch evidence**, especially for bit7 due to the earlier OR.
5. If stronger proof is needed, next stage can target the **specific native reader/branch at F5F4C** or `F58E0` return, with exact byte/stack validation; only then seek a concrete action boundary and visual evidence.
6. Preserve Stage29 `AITrace.cpp`, tested Stage34 branch and all historical stages; source-only Stage35 must **not** be called runtime complete.

## What is and is not established

Established by original bytes: real per-side update pipeline; slot11 is decremented/conditionally cleared before controller flag recomputation; native `F5BD0` reads the slot; native `F58E0` decodes controller bits. Still unknown: game action semantics for `0xF00` or bit7, direct relation to visible animations/hits/movement, and whether any COM-vs-COM gameplay defect exists.
