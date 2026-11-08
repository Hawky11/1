# Stage 13 — alternate queue callsite diagnostics

This overlay is based on the Stage 11 source archive plus the Stage 12 overlay. It adds one passive observer for the alternate queue writer seen in the Stage 12 runtime logs.

## Why this hook exists

Stage 12 confirmed two type-1 insertions from caller return RVA `0x000F6ADC` (P1 and P2, payload `0x1E`). The matching near-CALL instruction, if present, starts five bytes earlier at `AALib.exe+0x000F6AD7`. The Stage 11 research notes did not statically decode this caller.

Stage 13 does **not** assume what payload `0x1E` means. It records the actual call context so the instructions and data flow can be decoded:

- all general-purpose registers and EFLAGS at the call boundary;
- the exact return address and three original stack arguments (queue, event type, payload);
- 16 DWORDs from the original stack, including surrounding stack arguments;
- DWORDs from `EBP-0x100` through `EBP+0x20`, each guarded by a safe read;
- a `0x380`-byte code window from `RVA 0xF67D7` through `RVA 0xF6B56`;
- the original five CALL bytes are restored in the recorded code window, even though the live instruction is temporarily patched.

Records are written to `ArcanaAI_AlternateCallTrace.log`.

## Safety / validation

- The hook is installed only after the existing AALib.exe PE fingerprint check succeeds.
- It refuses to patch unless `AALib.exe+0xF6AD7` is a five-byte near CALL whose decoded destination is exactly `AALib.exe+0xF2DE0`.
- It uses the same verified queue-helper target as the existing tracer; the Stage 13 hook can run even when the Stage 11 F7C5D observer is disabled, and the Stage 12 queue observer remains in the call path.
- The shim restores all general-purpose registers and EFLAGS before tail-jumping.
- The callsite patch is applied while other process threads are suspended and while holding the existing shared patch-install mutex.
- It records only the exact P1/P2 queue addresses; it does not change event type, payload, queue state, AI decisions, or helper arguments.
- If the Stage 13 check fails, the existing Stage 11 final-command hook (when enabled) continues to run; the alternate hook is simply not installed. The two callsite hooks have independent INI switches.

## Apply and build

1. Extract `ArcanaAIOverhaul_Stage11_Source.zip` into a clean folder.
2. Apply the Stage 12 overlay first: replace `src/AIQueueTrace.cpp`, `Build.bat`, `ArcanaAIOverhaul.vcxproj`, and `ArcanaAI.ini` with the files in `Stage12_Overlay`.
3. Apply this Stage 13 overlay: replace `src/AICommandSiteTrace.cpp` with the file in `Stage13_Overlay/src`, and replace `ArcanaAI.ini` with the Stage 13 copy.
4. Run `Build.bat` on Windows using the MSVC x86 toolchain. Retain the complete `Build.log`.
5. Do not replace the current DLL unless the build succeeds. Back up the working DLL and INI first.
6. Rename the old logs, install the new DLL/INI, run one short COM vs COM bout that exercises normal CPU behavior, then close the game normally.

## Files to upload after the run

- `Build.log`
- `ArcanaAI_AlternateCallTrace.log`
- `ArcanaAI_QueueTrace.log`
- `ArcanaAI_EventDispatchTrace.log`
- `ArcanaAI_FinalCommandTrace.log`

The existing ParentFlow and ProducerContext logs may be retained, but they are not required for the main Stage 13 question. `EnableAlternateCallsiteTrace` controls this hook independently of `EnableFinalCommandTrace`.

## Build/runtime status

This overlay is source-only and has **not** been compiled with MSVC or runtime-tested in this environment. Check for the successful install message and a non-empty alternate-call log after the Windows build/run.
