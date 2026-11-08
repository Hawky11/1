# Stage21 — static map of the type-1 / payload 0x1E consumer

Date: 2026-11-08  
Binary: `AALib.exe`, PE32 x86, ImageBase `0x00400000`, SizeOfImage `0x0E0EB000`, timestamp `0x5FE5AA91`  
SHA-256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`

The binary identity matches the executable analysed in the earlier stages. This document is static analysis; Stage21 runtime confirmation still requires a build and a fresh in-game log.

## 1. Correct interpretation of the Stage12 caller address

The Stage12 detour is installed at the entry of the dispatcher:

- Dispatcher entry: RVA `0x000EE200` (VA `0x004EE200`).
- Stage12 records `_ReturnAddress()`; therefore `callerRVA=0x000EE5CB` is the caller's return site after a `CALL 0x004EE200`, not the dispatcher's entry and not a consumer-function entry.

The three Stage20 records for payload `0x1E` show `eventType=1`, `payload=0x1E`, and the dispatcher's queue-history fields matching those arguments. Their player objects are the known P1/P2 objects.

## 2. Type-1 dispatch branch inside 0x004EE200

At VA `0x004EE3E1`, the dispatcher checks the event type:

```asm
004EE3E1 cmp edi, 1
004EE3E4 jne 004EE34F
004EE3EA cmp ebx, 20h
004EE3ED jge 004EE40F
...
004EE422 push 1
004EE424 mov eax, ebx
004EE426 call 004E8CF0
004EE42B add esp, 4
004EE42E test al, al
004EE430 je 004EE443
```

At this point:
- `EDI` is the event type (1).
- `EBX` is the event payload (`0x1E` in the relevant samples).
- `ESI` is the player object.
- The helper receives the payload in `EAX`, player in `ESI`, and event type 1 on the stack.

### Helper 0x004E8CF0

For event type 1, the helper copies the payload to EDI, performs a state-dependent check through `0x00525860`, and then applies the payload range check:

```asm
004E8D35 test al, al
004E8D37 jne 004E8CFC       ; return AL=0
004E8D39 cmp edi, 20h
004E8D3C jl 004E8D8D
...
004E8D8D cmp edi, 17h
004E8D90 setbe al
004E8D93 ret
```

For payload `0x1E`:
- If `0x00525860` returns nonzero, the helper exits through `004E8CFC` with `AL=0`.
- Otherwise `0x1E < 0x20`, then `0x1E <= 0x17` is false, so `setbe al` also produces `AL=0`.

Therefore, for the observed `eventType=1, payload=0x1E` case, this helper's result is statically constrained to `AL=0`, independent of the result of the intermediate lookup. Stage21 records the actual return value rather than relying on this static deduction.

## 3. The actual fallback consumer

The conditional at `004EE430` is `JE 004EE443`; `AL=0` selects that fallback:

```asm
004EE443 push 3
004EE445 push edi
004EE446 call 004F7EB0
004EE44B mov esi, [esi+4]
004EE44E add esp, 8
004EE451 and dword ptr [esi+8], 0xFFFFFBFF
```

Helper `0x004F7EB0` is:

```asm
004F7EB0 push ebp
004F7EB1 mov ebp, esp
004F7EB3 mov eax, [ebp+0Ch]       ; slot index
004F7EB6 mov ecx, [ebp+08h]       ; pointer to controller array
004F7EB9 mov dword ptr [ecx+eax*4+4], 0
004F7EC1 pop ebp
004F7EC2 ret
```

The dispatcher passes index 3 and the pointer `player+0x3A3C`. The helper therefore clears:

`DWORD [controllerArray + 3*4 + 4] = DWORD [controllerArray + 0x10]`

Immediately afterwards, the dispatcher loads the controller pointer from `[player+4]` and clears bit `0x400` in `[controller+8]` using mask `0xFFFFFBFF`.

This is the Stage21 consumer target: it is downstream of event dispatch and is independent of the unrelated `0xF7C5D` final-command callsite.

## 4. Stage21 instrumentation

The new code wraps only these two CALL instructions inside the existing dispatcher:

| Callsite RVA | Original helper RVA | Captured evidence |
|---|---:|---|
| `0x000EE426` | `0x000E8CF0` | Real post-call EAX/AL for event type 1 / payload 0x1E |
| `0x000EE446` | `0x000F7EB0` | Exact array pointer, slot index, slot DWORD before and after the original clear, plus controller flags before the following AND |

Both original helpers are called exactly once. The wrappers duplicate the original cdecl arguments and leave the original caller-owned arguments on the stack, so the original `ADD ESP` instructions remain in charge of cleanup. The CALL-site bytes, helper destinations, and surrounding instructions are validated before patching. Both patches share the existing patch-install mutex and are installed while other process threads are suspended.

New option:

```ini
[AIResearch]
EnableStage21PayloadConsumerTrace=1
```

New log: `ArcanaAI_Stage21PayloadConsumerTrace.log`.

The log intentionally emits only known P1/P2 player objects with event type 1 and payload `0x1E`. The classifier record reports the actual AL. The fallback record reports slot-3 pre/post values. The next instruction after the fallback call is the statically verified bit-`0x400` clear; the Stage21 slot wrapper records controller flags immediately before that instruction, not after it.

Correlate Stage21 records with `ArcanaAI_EventDispatchTrace.log` by player, side, thread ID, and QPC. Stage12's `callerRVA=0xEE5CB` remains the dispatcher caller return site; Stage21's own hook return sites are `0xEE42B` and `0xEE44B`.

## 5. Why bit17=0 still reaches the queue at 0xF6AD7

The static code at the alternate producer explains the previously puzzling observations:

```asm
004F6A68 mov eax, [esi+3B84h]
004F6A6E shr eax, 11h
004F6A71 test al, 1
004F6A73 je 004F6A9D
```

When bit17 is zero, the conditional jump is taken to `004F6A9D`. When bit17 is one, execution takes the other path at `004F6A75`, which completes its own work and returns at `004F6A9C`; it does not continue to the alternate queue call.

On the bit17-clear path:

```asm
004F6A9D cmp dword ptr [ebp-4], 2
004F6AA1 jne 004F6AC8
004F6AA3 mov ecx, esi
004F6AA5 call 004E6A90
004F6AAA test al, al
004F6AAC jne 004F6AC8
004F6AAE mov eax, [esi+3F30h]
004F6AB4 push 3
004F6AB6 push eax
004F6AB7 call 0040FB50
004F6ABF push 72h
004F6AC1 mov ecx, esi
004F6AC3 call 0040FAA0
004F6AC8 push 0
004F6ACA mov ecx, esi
004F6ACC call 004E8560
004F6AD1 mov ecx, [ebp-8]
004F6AD4 push edi
004F6AD5 push ebx
004F6AD6 push ecx
004F6AD7 call 004F2DE0
```

The `F6A9D` JNE and `F6AAC` JNE are not gates that prevent the queue call. Their taken branches jump to the common continuation `F6AC8`; their fall-through paths perform extra helper/setter work and then also reach `F6AC8`. From that common continuation, both cases proceed to the queue insertion at `F6AD7`.

For the observed three payload-`0x1E` samples, the Stage20 runtime summary reports bit17=0 and queue insertion at `F6AD7`. The static control flow is consistent with that sequence. The actual per-sample Stage15 probe records were not present in the Stage20 evidence bundle, so this document does not claim a new QPC-level correlation of those individual branch probes.

## 6. Separate open item: direct setter AL=0

Stage20 observed 33 calls at the object-guard helper with bit2 set and real `AL=1`. No negative-control `AL=0` record has yet been captured.

Stage21 keeps the Stage20 source/INI configuration active. The existing Stage20 hook at callsite `0xC64F1` continues to record the original helper's real AL and checked object DWORD. A natural in-game case with object bit2 clear is still required; Stage21 does not force or modify the object bit.

## 7. Scope and status

- This is diagnostic-only source. It does not change the game's AI decisions or helper return values.
- The Stage21 source overlay replaces the assembled project's `src/AIQueueTrace.cpp`; the Stage21 INI overlay replaces the assembled `ArcanaAI.ini`.
- Preserve the Stage20 `AITrace.cpp` and all earlier source files from the Stage20 source branch.
- The Stage21 source has been committed, but it has not been compiled or run in this environment. Build and runtime confirmation remain pending.
