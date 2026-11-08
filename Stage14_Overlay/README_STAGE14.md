# Stage 14 — producer and guard-call diagnostics

Stage 14 extends the working Stage 13 diagnostic source. It does not replace or modify the Stage 12 or Stage 13 branches.

## Research target

The Stage 13 runtime code dump established the following sequence inside the parent routine at `AALib.exe+0xF69F0`:

- `CALL F2F50` at RVA `0xF6A14`;
- the raw result is consumed at RVA `0xF6A19`, before the parent splits the packed value into its high-byte/low-byte components;
- `CALL F2ED0` at RVA `0xF6A53`;
- the boolean-like `AL` result is tested at RVA `0xF6A58`;
- the alternate queue insertion at `0xF6AD7` remains covered by the Stage 13 hook.

Stage 14 records the values on both sides of those helper calls, rather than assuming their meanings.

## Five probes

| Probe | Site | What it captures |
|---|---|---|
| Parent entry | `F69F0` | Caller return address, player argument, entry registers |
| F2F50 call | `F6A14` | Registers and stack arguments passed into F2F50 |
| F2F50 return | `F6A19` | Raw EAX result before the parent transforms it |
| F2ED0 call | `F6A53` | Queue pointer, type/payload candidate and registers |
| F2ED0 return | `F6A58` | Raw EAX/AL result before the original conditional test |

The new log is `ArcanaAI_ProducerCallTrace.log`. It also includes untouched code windows from `F69F0`, `F2F50` and `F2ED0` so those routines can be disassembled without another diagnostic build.

## Behavior preservation and installation checks

- The two helper CALL probes are installed only if their original five bytes match and their decoded destinations are exactly `F2F50` and `F2ED0`.
- The parent entry and return-site probes verify their complete original instruction bytes before patching.
- Each code patch is written while the other process threads are suspended and while the existing patch-install mutex is held.
- The CALL shims log their context and tail-jump to the original callee. The callee retains its original return address and original stack arguments.
- The F2F50 return probe replays the original `mov ecx,eax; cdq; and edx,0xFF` sequence.
- The F2ED0 return probe replays the original `add esp,0x0C; test al,al` sequence, so the original branch flags are preserved.
- The F69F0 entry probe replays the verified original six-byte function prologue.
- Probe records are buffered and flushed by the worker thread; the game thread does not write the log file directly.
- If any individual byte/target check fails, that probe is skipped and the failure is reported. Other successfully installed probes remain active.

The five hooks are independent observers around one parent routine; they do not change arguments, payloads, queue state, AI decisions, or helper return values.

## Apply and build

Start from a clean extraction of `ArcanaAIOverhaul_Stage11_Source.zip`.

1. Apply Stage 12 first: replace `src/AIQueueTrace.cpp`, `Build.bat`, `ArcanaAIOverhaul.vcxproj` and `ArcanaAI.ini` using `Stage12_Overlay`.
2. Apply Stage 13 next: replace `src/AICommandSiteTrace.cpp` and `ArcanaAI.ini` using `Stage13_Overlay`.
3. Apply Stage 14 last: replace `src/AICommandSiteTrace.cpp` and `ArcanaAI.ini` using `Stage14_Overlay`.
4. The Stage 12 project already includes `AICommandSiteTrace.cpp`; no project-file change is needed for Stage 14.
5. Run `Build.bat` using the MSVC x86 toolchain and retain the complete `Build.log`.
6. Do not replace the known-working DLL unless the build succeeds. Back up the current DLL and INI first.
7. Rename prior logs, install the new DLL/INI, run a short COM vs COM bout, then close the game normally.

## Upload after the run

Required:
- `Build.log`
- `ArcanaAI_ProducerCallTrace.log`
- `ArcanaAI_AlternateCallTrace.log`
- `ArcanaAI_QueueTrace.log`
- `ArcanaAI_EventDispatchTrace.log`
- `ArcanaAI_FinalCommandTrace.log`

ParentFlow and ProducerContext logs may also be retained; they help correlate the parent routine with the wider helper flow.

## Status

This is a source-only diagnostic overlay. It has **not** been compiled with MSVC or runtime-tested here. Confirm a successful build and inspect the Stage 14 installation messages before relying on the new log.
