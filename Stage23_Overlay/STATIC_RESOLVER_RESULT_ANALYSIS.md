# Stage23 — capture the real type-1 mapping-resolver result

Date: 2026-11-09  
Binary: `AALib.exe`, PE32 x86, ImageBase `0x00400000`, SizeOfImage `0x0E0EB000`, timestamp `0x5FE5AA91`  
SHA-256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`

This analysis was rebuilt directly from the archived `AALib.zip` and its extracted `AALib.exe`. The executable hash exactly matches the fingerprint previously used for all RVA-based Stage work.

## 1. Corrected dispatcher dataflow

The type-1 resolver call is at VA `0x004EE273` (RVA `0x000EE273`) and calls VA `0x0052EAC0` (RVA `0x0012EAC0`).

Verified disassembly:

```asm
004EE240  mov ecx, [esi+3978h]
004EE246  cmp ecx, 1Dh
...
004EE26F  push edx                 ; resolver arg4 = [player+1224h]
004EE270  push ebx                 ; resolver arg3 = dispatcher payload
004EE271  push ecx                 ; resolver arg2 = state3978-derived value
004EE272  push eax                 ; resolver arg1 = state/mode-derived value
004EE273  call 0052EAC0            ; returns pointer in EAX
004EE278  movzx ecx, byte ptr [eax+0Bh]
004EE27C  mov edx, [esi+4]
004EE27F  mov [edx+184h], ecx
004EE285  movzx ecx, byte ptr [eax+04h]
004EE289  movzx edx, byte ptr [eax+02h]
004EE28D  movzx edi, byte ptr [eax+03h]
...
004EE2CE  mov eax, edi
004EE2D0  cmp edi, 2Ch
...
004EE2FC  push ecx
004EE2FD  push edx
004EE2FE  push eax                 ; derived from resolver record byte +3
004EE2FF  lea eax, [esi+3A08h]
004EE305  push eax
004EE306  call 004F57D0
004EE30B  mov edi, [ebp+08h]       ; EDI is overwritten with dispatcher arg1
004EE311  push ebx
004EE312  push edi
004EE313  mov ecx, esi
004EE315  call 004EC4F0            ; callee preserves EDI
004EE31A  test edi, edi
...
004EE3E1  cmp edi, 1                ; tests dispatcher event type, not resolver byte
```

The resolver's `EAX` return is a pointer to a 12-byte entry. The helper computes entries with a 12-byte stride (`payload * 3 * 4`). For the observed payload `0x1E` (< `0x20`), the helper selects the table through resolver arg1 and indexes entry `0x1E`. The dispatcher reads four fields from that entry: offsets `+0x0B`, `+0x04`, `+0x02`, and `+0x03`. Byte `+0x03` is moved to EDI and later influences the value passed to helper `0x4F57D0`.

**Important correction to the older Stage21 static note:** byte `+0x03` is not still in EDI at `0xEE3E1`. It is first used after the resolver call, then EDI is explicitly overwritten by `MOV EDI,[EBP+8]` at `0xEE30B`. The `CMP EDI,1` at `0xEE3E1` tests the original dispatcher event type. In the type-1/payload-`0x1E` branch, the instruction at `0xEE3FC` subsequently repurposes EDI again as `player+0x3A3C`, the controller-array pointer. That is why Stage21 r2's pre-call EDI value equals the controller-array pointer rather than a resolver byte.

The Stage23 observer captures the actual resolver return record immediately after `CALL 0x52EAC0` at `0xEE273`, before any of these downstream register reuses.

## 2. Resolver arguments and captured data

At the callsite the original cdecl stack arguments are:

| Resolver argument | Source at dispatcher callsite | Meaning in this path |
|---|---|---|
| arg1 | EAX | state/mode-derived table selector |
| arg2 | ECX | state value derived from `[player+0x3978]` |
| arg3 | EBX | original dispatcher payload (`0x1E`) |
| arg4 | EDX | `[player+0x1224]` |

Stage23 copies the four caller-owned stack arguments into private locals, rebuilds the same cdecl argument list in the original order, and calls the resolver **exactly once**. The original caller stack remains unchanged; the original `ADD ESP,0x10` at `0xEE291` still owns caller cleanup.

After the real helper returns, the wrapper saves the result registers and EFLAGS, then passes a context to the logger. The logger captures:
- P1/P2 player identity, thread, game tick, QPC, dispatcher type and payload;
- the exact four resolver stack arguments and original return RVA;
- returned record pointer and all three DWORDs (12 raw bytes);
- byte offsets `+2`, `+3`, `+4`, and `+0x0B` as separate values;
- selected player state fields for cross-checking.

The output field is named `resolverByte03`, not `decodedActionCode`. Its semantic role beyond being a mapped field consumed by the following helper remains to be confirmed by runtime patterns.

## 3. Hook safety and behavior preservation

The five-byte CALL at `0xEE273` is replaced by a relative CALL to the Stage23 wrapper. Installation is refused unless all these checks pass:
- four-byte prefix `52 53 51 50` (`push edx; push ebx; push ecx; push eax`);
- exact CALL bytes `E8 48 08 04 00`;
- seven-byte suffix `0F B6 48 0B 8B 56 04`;
- resolver entry prologue `55 8B EC 8B 45 10 83 F8`;
- decoded CALL destination equals `AALib.exe+0x12EAC0`;
- PE image size and timestamp match the known executable.

The wrapper preserves the original caller's stack and calls the resolver once with the same four arguments. It preserves the actual returned EAX pointer and all post-call registers/EFLAGS across logging. Stack cleanup within the wrapper uses flag-neutral LEA; the caller's original cleanup remains in place. Installation uses the shared patch mutex, suspends other process threads, revalidates bytes immediately before patching, flushes the instruction cache, restores original page protection, and resumes the threads.

The trace filters to known P1/P2 objects with dispatcher event type `1` and payload `0x1E`; it does not write to player fields, resolver tables, or game state.

## 4. Runtime verification status

**Source-only at this checkpoint.** Stage23 has not yet been compiled or run in the game. Do not assume the CALL wrapper installed just because the code is present.

Expected marker:
`STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r1`

Expected installation line:
`Installed Stage23 resolver-result observer at callsite RVA 0x000EE273 -> helper RVA 0x0012EAC0`

New log:
`ArcanaAI_Stage23ResolverResultTrace.log`

The decisive runtime fields are `resolverRecord`, `recordReadable`, `recordDwords`, and `resolverByte03`. Correlate them with Stage21 classifier/fallback, Stage22 post-AND, EventDispatch and Queue logs using side, player, thread, payload, and QPC.

## 5. Build and upload

Apply this Stage23 overlay **on top of the assembled Stage22 project**:
1. Replace `src/AIQueueTrace.cpp` with `Stage23_Overlay/src/AIQueueTrace.cpp`.
2. Replace `ArcanaAI.ini` with `Stage23_Overlay/ArcanaAI.ini`.
3. Build `Release | Win32` with the existing MSVC x86 toolchain.
4. Upload the complete `Build.log` before runtime interpretation.
5. After a successful build, run a normal COM-vs-COM session and close the game normally so all worker logs flush.

Required logs: Build.log, `ArcanaAI_Stage23ResolverResultTrace.log`, `ArcanaAI_Stage21PayloadConsumerTrace.log`, `ArcanaAI_Stage22PayloadPostAndTrace.log`, `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_QueueTrace.log`, plus same-run Stage18 gate, Stage20 object guard, and Stage17 setter logs. Include Stage16 writer and Stage19 state-map logs if they produce data.
