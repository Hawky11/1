# Stage 17 — trace the direct bit-113 setter callsite

## Purpose

Stage16 found the exact instruction that sets player bit17:

- VA `0x0040FAD0`, RVA `0x0000FAD0`: `or dword ptr [eax], edx`
- On every captured 0→1 transition, `EAX` was the exact P1/P2 `player+0x3B84` address and `EDX=0x00020000`.

The instruction belongs to a generic player-bitset setter at VA `0x0040FAA0` / RVA `0x0000FAA0`. Its stack parameter is a bit index. Index `0x71` (113) maps to DWORD `player+0x3B84`, bit 17, because the bitset base is `player+0x3B78`.

A full direct-call scan found one callsite that explicitly passes index `0x71`:
- caller function VA `0x004C63B0`, RVA `0x000C63B0`
- the argument push at VA `0x004C64FE`
- the CALL at VA `0x004C6502`, RVA `0x000C6502`
- the original CALL bytes are `E8 99 95 F4 FF`, and it resolves to `0x0040FAA0`
- original return continuation RVA: `0x000C6507`

Stage17 observes this caller to test whether it explains the five Stage16 bit17 set transitions. It does not assume the candidate is proven until a fresh runtime log confirms matching P1/P2 calls.

## Static context around the candidate call

The caller uses `ESI` as the player object and `EBX` as a pointer to a related context/object. The path to the setter has several gates. The final part is:

```asm
004C64B1  cmp word ptr [esi+3A34h], 1
004C64B9  jne 004C640B
004C64BF  movzx eax, word ptr [esi+3A36h]
004C64C6  cmp ax, 1Ah
004C64CA  je 004C64DC
004C64CC  cmp ax, 18h
004C64D0  je 004C64DC
004C64D2  cmp ax, 19h
004C64D6  jne 004C640B
004C64DC  push 0Fh
004C64DE  mov ecx, esi
004C64E0  call 00401F30
004C64E5  test al, al
004C64E7  je 004C640B
004C64ED  mov ecx, [ebx]
004C64EF  push 2
004C64F1  call 00495B00
004C64F6  test al, al
004C64F8  jne 004C640B
004C64FE  push 71h
004C6500  mov ecx, esi
004C6502  call 0040FAA0
```

The helper at `0x00495B00` tests a bit in the related object's bitset beginning at `[object+0x3B70]`; argument 2 is the tested bit index. If that helper returns nonzero, this candidate path exits. If it returns zero, the caller invokes the generic setter with index 113.

There are additional earlier checks in the same function (global/state checks, calls through `0x00401F30`, `0x004F6ED0`, and `0x004F2FD0`). The Stage17 record captures ESI/ECX/EBX/EDI/EAX/EDX/EBP/EFLAGS, the related object pointer and its `+0x3B70` value, the nearby player fields, the call stack and QPC so these gates can be correlated with ProducerCallTrace and Stage16 writer records.

## Instrumentation

Stage17 extends the already-built `src/AITrace.cpp`; no extra project or Build.bat source entry is required.

- Replaces only the five-byte CALL at RVA `0x000C6502` with a passive observer CALL.
- Before patching, it verifies the original five bytes and decodes their destination. It refuses to patch if they no longer match the verified call to RVA `0x0000FAA0`.
- Uses the existing shared patch-install mutex `Local\\ArcanaAIOverhaul_PatchInstall_Mutex`.
- Suspends process threads while patching. It refuses the patch if thread enumeration/context capture is incomplete or if a suspended thread's EIP is inside the five bytes being replaced.
- The hook saves/restores registers and EFLAGS, records the original argument `0x71`, and tail-jumps to the original SetBit helper. The original return address and stack argument remain unchanged; the helper's `RET 4` returns to the original `AALib.exe+0xC6507` continuation.
- Records are buffered and flushed by the existing worker thread into `ArcanaAI_Bit113SetterTrace.log`.
- Stage16 write-only DR0/DR1 watchpoints remain enabled by the supplied INI and continue logging actual P1/P2 field writes in `ArcanaAI_PlayerBit17WriteTrace.log`.

This is a diagnostic observer, not a gameplay modification. It does not set or clear bit 17 itself.

## Build and install

Use a clean extraction of `ArcanaAIOverhaul_Stage11_Source.zip`, then apply the overlays in order:

1. Stage12: replace the Stage12 files as described in its README.
2. Stage13: replace `src/AICommandSiteTrace.cpp` and `ArcanaAI.ini`.
3. Stage14: replace `src/AICommandSiteTrace.cpp` and `ArcanaAI.ini`.
4. Stage15: replace `src/AICommandSiteTrace.cpp` and `ArcanaAI.ini`.
5. Stage16: replace `src/AITrace.cpp` and `ArcanaAI.ini`.
6. Stage17: replace `src/AITrace.cpp` and `ArcanaAI.ini` from this overlay.

The existing Stage11 project already compiles `AITrace.cpp`, so no `.vcxproj` or `Build.bat` modification is required. Build with MSVC x86 and retain the complete Build.log. Do not overwrite the known-working DLL unless the build succeeds.

Start with clean runtime logs. Run a short COM-vs-COM session exercising the same type-1 / payload-0x1E path, then close normally so buffers flush.

## Required upload

- `Build.log`
- `ArcanaAI_Bit113SetterTrace.log`
- `ArcanaAI_PlayerBit17WriteTrace.log`
- `ArcanaAI_ProducerCallTrace.log`
- `ArcanaAI_AlternateCallTrace.log`
- `ArcanaAI_QueueTrace.log`
- `ArcanaAI_EventDispatchTrace.log`
- `ArcanaAI_FinalCommandTrace.log`

Keep `ArcanaAI_ParentFlowTrace.log` and `ArcanaAI_ProducerContextTrace.log` when available.

## Validation and analysis

1. Confirm Build.log shows a successful Win32/x86 build.
2. Confirm `ArcanaAI_Bit113SetterTrace.log` reports the verified callsite hook installed at RVA `0x000C6502`, original helper RVA `0x0000FAA0`, and the number of suspended threads.
3. Check for `BIT113_SET_CALL` entries with `index=0x00000071`, matching `player` P1/P2 pointers and `callRVA=0x000C6502`.
4. Compare QPC/timestamp, player `3B84` value, and caller registers/fields against the Stage16 `RVA 0x0000FAD2` 0→1 writes.
5. Determine whether every observed Stage16 set transition has a preceding Stage17 call from this site. If so, this callsite is confirmed as the runtime origin for those transitions in this tested scenario. If there are Stage16 sets without a matching Stage17 call, broaden the next trace to all invocations of the generic helper at RVA `0x0000FAA0` and inspect dynamic indices/other modules.
6. Use the other event logs to relate the caller's state gates to F2ED0/F6A68 and the later clear at RVA `0x000ED19F`.

## Status

Stage17 is source-only until the user builds and runs it. It has not been compiled with MSVC or runtime-tested in this environment. Do not claim the hook installed or the candidate caller was reached until the new runtime log confirms both.
