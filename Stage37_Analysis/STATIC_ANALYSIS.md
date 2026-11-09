# Stage37 — native type1 controller bitmap and object-state commit (STATIC/SOURCE ONLY)

**Date:** 2026-11-10. Base: verified [Stage36 r2 runtime evidence](../Stage36_R2_Runtime/RUNTIME_ANALYSIS.md). Original SHA-pinned `AALib.exe` from ChatGPT Library `/AALib.zip`, PE32/i386, preferred ImageBase `0x400000`, actual 175,556,608 bytes, SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.

**Status:** Original EXE static reverse-engineering **PASS**, Stage37 passive source prepared. **Stage37 Windows x86 compilation/runtime not yet performed**. Exactly what Stage36 proved: seven accepted `F4360 EAX` results matched seven `F7C5D→F2DE0 type1` queue writes, then seven type1 dispatcher `EE200` calls. Stage36 did **not** prove an animation, combat movement, attack, guard, hit or AI malfunction. Stage37 therefore investigates the *next native controller/object-state handoff*, not a guessed move ID.

## 1. Exact original type1 execution path inside EE200

All addresses below are **RVA**. Verified from full disassembly of the SHA-matched PE32 `.text` section with objdump, and independently checked with [verify_stage37.py](tools/verify_stage37.py) (**22 precise byte signatures + 7 exact E8 CALL target checks**, standard library only).

```asm
EE200   push ebp                  ; EE200(__thiscall ECX=player, type,payload)
EE23B   cmp eax,1                ; eventType == 1
EE23E   jne EE296                ; alternate path for type2
EE240   mov ecx,[esi+3978h]      ; native selector
EE263   mov eax,[esi+3F20h]      ; effective state selector, with prior special case
EE269   mov edx,[esi+1224h]
EE26F   push edx
EE270   push ebx                 ; type1 payload
EE271   push ecx
EE272   push eax
EE273   call 12EAC0              ; returns native description/table pointer
EE278   movzx ecx,byte ptr [eax+0Bh]
EE27C   mov edx,[esi+4]
EE27F   mov [edx+184h],ecx       ; **object-relative state write**
EE285   movzx ecx,byte ptr [eax+4]
EE289   movzx edx,byte ptr [eax+2]
EE28D   movzx edi,byte ptr [eax+3]
...
EE2C0   mov [esi+3F64h],ecx      ; **player-relative output write**
EE2C6   call 158EC0              ; updates P+3B70 flags from table
...
EE306   call F57D0               ; controller bitmap P+3A08/+04/+08 state write
EE30B   mov edi,[ebp+8]
EE315   call EC4F0               ; native player+object lifecycle/reset routine
```

`EE200` first chooses the description via `12EAC0`, then writes **`[O+184]`**, **`P+3F64`**, delegates state flags to `158EC0`, delegates the controller bit mapping to `F57D0`, then calls `EC4F0` which touches player and object state. Thus the command queue is **not** the end of the native processing chain. This is an observed binary instruction path, not yet verified per-event at runtime by Stage37.

### F57D0 — controller-bit writes, not automatic attacks

```asm
F57D0    push ebp / mov ebp,esp
F57DC    push eax
F57DD    push ecx
F57DE    call 9EEA0
F57F8    xor eax,eax
F57FA    mov [edi+4],eax
F57FD    mov [edi+8],eax
F5800    mov [edi],esi           ; edi is P+3A08 passed by EE306
F5802    movzx ecx,byte ptr [esi+esi*2+4DDF189h]
...
F5854    shl edx,cl
F5856    or [eax],edx           ; map selected input index into bitmap DWORD
...
F58AA    shl edx,cl
F58AC    or [eax],edx           ; second indexed controller bit, if present
```

The three-byte static descriptor table at `RVA 4DDF188` controls which fields get bits (may contain `0xFF` marker meaning no mapping). `F57D0` can reset bitmap(s) based on descriptor flags; do not interpret any single 1-bit as a successful move without observing the downstream engine. Its indirect internal helper `9EEA0` resolves a player-based state, but Stage37 snapshots do **not** hook F57D0 separately, so a net EE200 change cannot be assigned uniquely to one inner helper.

### EC4F0 — native player/object lifecycle state writes

```asm
EC4F0  push ebp / mov ebp,esp / sub esp,20h
EC50A  and DWORD PTR [esi+3B70h], 7FDF8FFFh
EC528  and DWORD PTR [esi+3B74h], edi
EC53C  mov DWORD PTR [esi+3F84h], 2
EC550  call F7EB0                 ; clears native timer slot
EC558  call F7EB0
EC560  call F7EB0
EC582  mov edx,FFFFFFDFh
EC587  and DWORD PTR [O+08h],edx
EC58A  mov eax,[esi+4]
EC592  and DWORD PTR [O+08h],ecx
EC59F  and DWORD PTR [O+0Ch],edx
EC5A5  and DWORD PTR [O+10h],ecx
EC5FA  inc DWORD PTR [O+278h]   ; guarded by [O+3Ch]==0
EC603  mov BYTE PTR [O+27Ch],0
```

**Caution:** `EC4F0` is a broad state/lifecycle reset path, not proven to be the animation renderer, command acceptance gate, attack resolution or hit handler. Its writes do demonstrate a native player/object state handoff. The `O+184` write in `EE200` and `EC4F0` native field writes motivate the Stage37 before/after sample.

## 2. Stage37 observational implementation: no new hook

Stage37 reuses the **same already tested** `EventDispatchHook` at `EE200` (the Stage32/33/34 hook, with its original function called **once**); it does not patch `F57D0`, `EC4F0`, `F4360`, `F58E0` or any additional code. This avoids another potential Stage10/Stage36 inter-observer conflict. Previous Stage36 r2 and Stage35 code paths, Stage29 `AITrace.cpp`, and all prior historical branches remain unchanged.

Configuration: `[AIResearch] EnableStage37Type1CommitTrace=1`, with `EnableCpuEventDispatchTrace=1` **and** `EnableCpuEventDispatchDetailTrace=1`.

At entry and return of the **existing native EE200 call** for the *exact known P1/P2 CPU players, eventType=1*, Stage37 obtains 30 read-only DWORDs with independent 30-bit `preMask/postMask` and `changedMask` (only counts differences where **both** reads succeeded). The fields are carried in the existing bounded event-dispatch detail ring and flushed only on the existing worker to **`ArcanaAI_Stage37Type1CommitTrace.log`**. No new runtime thread, no new hot-path file writes, no game-state changes.

| Field index | Address / interpretation |
|---|---|
| 0..4 | `P+3978`, `P+3A08`, `P+3A0C`, `P+3A10`, `P+3A6C` |
| 5..11 | `P+3B70`, `P+3B74`, `P+3B78`, `P+3B7C`, `P+3B80`, `P+3B84`, `P+3B88` |
| 12..17 | `P+3F20`, `P+3F30`, `P+3F34`, `P+3F50`, `P+3F64`, `P+3F84` |
| 18 | `O = [P+4]` (object pointer, raw DWORD, may be invalid) |
| 19..28 | `O+08`, `O+0C`, `O+10`, `O+18`, `O+60`, `O+184`, `O+278`, `O+27C`, `O+280`, `O+2A8` |
| 29 | `P+3F78` |

`full mask=0x3FFFFFFF`; an absent mask bit denotes *unreadable* data, not a zero value. `O+27C` is read as raw DWORD (four adjacent bytes), not interpreted as one boolean flag. Value `O+184` may be a command-derived byte but does **not** automatically name an animation.

New entries carry **the same `dispatchSeq`, side/P1/P2, TID and before/after QPC** as their existing `EVENT_DISPATCH_DETAIL` record. They can be matched without heuristic wall-clock timestamps. If expected original type1 commands from Stage36 correspond to Stage37 records, require exact P/TID, type1 payload and QPC match; produce separate counts for failure/sentinel candidates that never enqueue. Monitor `WARNING: event dispatch detail buffer full` in the existing EventDispatch log, since the ring is shared. No record appears when there was no type1 dispatch, which does not mean malfunction.

## 3. What would be a proven result?

- New Windows x86 `BUILD SUCCEEDED`, `STAGE37_TYPE1_COMMIT_BUILD=2026-11-10-r1` and `STAGE37_ENABLED` plus Stage32/35/36/Stage10 previous installed markers.
- For all observed `EE200` type1, exact unique join `Stage37_TYPE1_EFFECT.dispatchSeq/qpcBefore/qpcAfter/side` with existing `EVENT_DISPATCH_DETAIL`. Record 30 raw before/after fields and masks; check whether `O+184`, `P+3F64`, `P+3A0C/10`, `O+08/0C/10/278` changed.
- Join previously accepted command IDs to these state changes; inspect identical payload occurrences and different character/side. **No single field transition proves rendered action**, so next step may still require per-frame native animation/action executor instrumentation.

**Status of new Stage37: source/static only; needs user-side x86 build and natural COM-vs-COM runtime.** Reuse existing `AALib.exe` in Library, **do not request upload**.
