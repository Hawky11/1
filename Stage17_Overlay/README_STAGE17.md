# Stage 17 — identify the caller that sets player bit 113

## Goal

Stage16 runtime tracing found the actual setter for bit17 in `[player+0x3B84]`:

```asm
0040FAD0  or dword ptr [eax], edx
```

The instruction is inside the generic player bitfield setter at VA `0x0040FAA0` (RVA `0x0000FAA0`). The caller supplies a bit index on the stack and the player object in ECX. Index `0x71` (113) selects DWORD `player+0x3B84`, bit 17.

Stage17 hooks the setter entry, filters to incoming index `0x71`, and records the untouched caller stack so the exact return address and call instruction can be identified. It is a diagnostic observer; it does not change the setter's inputs or result.

## Stage16 evidence prompting this stage

The Stage16 run recorded five true 0→1 transitions:
- P1 at 13:02:52.914
- P1 at 13:02:58.677
- P2 at 13:02:58.803
- P1 at 13:03:04.912
- P2 at 13:03:08.790

Each transition executed the setter's `OR [EAX],EDX` with EAX equal to that exact player's watched `player+0x3B84` address and EDX=`0x00020000`. Five additional calls repeated the already-set bit without changing its state.

The strongest static caller candidate is VA `0x004C6502` (RVA `0x000C6502`):

```asm
004C64ED  mov ecx,[ebx]
004C64EF  push 2
004C64F1  call 00495B00
004C64F6  test al,al
004C64F8  jne 004C640B
004C64FE  push 0x71
004C6500  mov ecx,esi
004C6502  call 0040FAA0
```

The helper at `0x00495B00` tests bit 2 of the DWORD at `[object+0x3B70]`. If AL=0, the following conditional path calls the generic setter with index 113 and player object ESI. Because `test al,al`, `push`, `mov`, and `call` do not alter arithmetic flags after that test, the expected setter-entry ZF for this candidate is 1.

This is a candidate, not an established caller. Stage17 verifies the runtime CALL bytes and decodes their rel32 target.

## Instrumentation

Stage17 modifies the existing `src/AICommandSiteTrace.cpp`; no new project source, project-file, or Build.bat change is required.

- Expected setter entry RVA: `0x0000FAA0`.
- The first seven bytes must be `55 8B EC 56 8B 75 08`. The hook refuses to patch if they differ.
- The hook executes `pushfd` before comparing the incoming stack argument at original `[ESP+4]` (after PUSHFD, `[ESP+8]`). It calls the existing Stage14 probe logger only for `0x71`.
- It then restores registers/EFLAGS and replays the exact seven-byte prologue before jumping to `AALib.exe+0xFAA7`.
- Probe 11 interprets ECX as the player object and records registers, EFLAGS/ZF, caller EBP frame, player fields (including `+0x3B70`, `+0x3B84` and CPU control), the original return address, and the bit-index argument.
- For each record, it treats `returnAddress-5` as the candidate CALL site, reads its five bytes, decodes an `E8 rel32` destination, and reports `callTargetVerified=1` only when the destination equals the verified setter entry.
- Stage17 records use the existing bounded Stage14 probe ring but are routed to a separate `ArcanaAI_Bit113SetterTrace.log`. The Stage14 probe worker flushes this ring even when Stage14 itself is disabled.
- Stage17 records include `callerReturnRVA`, `callerCallRVA`, `callBytes`, `decodedTarget`, `callTargetVerified`, and `entryZF`. The candidate caller should report call RVA `0x000C6502`, return RVA `0x000C6507`, target `0x0040FAA0`, and usually entry ZF=1.
- Only the exact index `0x71` is logged; other bitfield setter calls continue through the original path without an extra probe record.

## Configuration

The supplied `ArcanaAI.ini` is the Stage16 INI plus:

```ini
EnableStage17Bit113SetterTrace=1
```

Stage16 write watchpoints remain enabled. Stage14 and Stage15 diagnostics remain enabled so the setter call can be correlated with bit17 state, the F6A68 branch, F2ED0 result, and queue acceptance.

## Build and install

Use a clean extraction of `ArcanaAIOverhaul_Stage11_Source.zip`, then apply the overlays in this exact order:

1. Stage12 overlay.
2. Stage13 overlay.
3. Stage14 overlay.
4. Stage15 overlay.
5. Stage16 overlay: replace `src/AITrace.cpp` and `ArcanaAI.ini`.
6. Stage17 overlay: replace `src/AICommandSiteTrace.cpp` and `ArcanaAI.ini`.

Build with the existing MSVC x86 Build.bat. Retain the full `Build.log`. Do not replace a known-working DLL if compilation fails. Install only after a successful build and start with clean runtime logs.

## Required logs

Upload fresh files to this branch:

- `Build.log`
- `ArcanaAI_Bit113SetterTrace.log`
- `ArcanaAI_PlayerBit17WriteTrace.log`
- `ArcanaAI_ProducerCallTrace.log`
- `ArcanaAI_AlternateCallTrace.log`
- `ArcanaAI_QueueTrace.log`
- `ArcanaAI_EventDispatchTrace.log`
- `ArcanaAI_FinalCommandTrace.log`

Keep `ArcanaAI_ParentFlowTrace.log` and `ArcanaAI_ProducerContextTrace.log` when available.

## How to interpret the next run

1. Confirm the Stage17 hook startup/install message and the seven-byte prologue validation.
2. Check that records have `bitIndex=0x00000071`.
3. Look for `callTargetVerified=1` and group by callerCallRVA.
4. Confirm or reject the candidate `callerCallRVA=0x000C6502` / `callerReturnRVA=0x000C6507`.
5. For the candidate, check `entryZF=1`, side, ECX/ESI player match, and `3B70` / `3B84` values. The call-site bytes must decode to the setter RVA; do not rely on the return address alone.
6. Correlate the setter call QPC and side with Stage16's subsequent `OR [EAX],EDX` record, Stage15's F6A68 branch and, where applicable, the later F2ED0-driven clear.
7. If another caller RVA appears, disassemble its caller and conditions. Do not assume `0x4C6502` is the only call site merely because it is the only statically identified literal `push 0x71`.
8. Check for Stage14 ring drop warnings. The setter hook is filtered, but repeated calls may still fill the bounded ring if the game calls it frequently.

## Status

Stage17 source overlay is prepared but has **not been compiled or runtime-tested**. Stage16 is the latest runtime-evidenced stage. Do not claim Stage17 installed until its new log confirms the hook was applied.
