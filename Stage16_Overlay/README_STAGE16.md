# Stage 16 — P1/P2 player bit-17 write watchpoints

## Purpose

Stage15 found that the route to the alternate type-1 / payload-`0x1E` queue writer at RVA `0xF6AD7` correlated with bit 17 of `[player+0x3B84]`:

- bit17=1: three observed calls did not reach F6AD7;
- bit17=0: the observed P2 call reached F6AD7 and was accepted by QueueTrace and EventDispatchTrace.

Stage16 traces writes to the exact runtime DWORD addresses for P1 and P2. It does not guess individual setter addresses and does not change the game field.

## Static lead

The Stage16 static review identified one particularly relevant direct write:

`AALib.exe+0x0ED19F`:

```asm
and dword ptr [esi+3B84h], 0xFFF9FFFF
```

This clears bits 17 and 18. Immediately before it, the routine calls the mailbox helper `AALib.exe+0x4F2ED0` with arguments type=1 and payload=0x1E. The clear is executed only when the helper returns `AL=0`. This is a candidate path, not yet proof that this instruction caused the Stage15 state transition. See `STATIC_XREF_REVIEW.md` and `WRITE_XREFS_3B84.txt`.

## Instrumentation design

Stage16 extends the existing `src/AITrace.cpp` hardware-watchpoint module already compiled and started by the project.

- P1 player object RVA is derived from the already-confirmed P1 COM control field RVA minus `0x3F2C`; the Stage16 watched address is player RVA plus `0x3B84`. The same derivation is used for P2.
- DR0 and DR1 watch those exact 4-byte DWORD addresses.
- DR7 uses `0x00DD0005`: local enable for DR0/DR1, write-only access (`RW=01`), 4-byte length (`LEN=11`). The address is DWORD-aligned.
- The handler records writes from **any user-mode module in the target process**, not just AALib.exe. It logs absolute EIP-after plus `inAALib=0/1`; RVA-after is populated only when EIP lies inside AALib.exe. This keeps writes performed through a helper in another loaded module from being silently filtered out.
- Each record includes side, thread ID, tick, local time, QPC, EIP-after, RVA-after (when applicable), P1/P2 field and player addresses, DR6, all general-purpose registers, EFLAGS, and 24 code bytes around EIP (16 bytes before EIP, 8 bytes from EIP).
- It reads the post-write DWORD and tracks the last observed value in a per-player shadow to estimate old/new transitions. `shadowInitialized=1` means only that the initial value was read successfully. It does **not** guarantee that every earlier write was observed: a write from an uninstrumented thread or a full record buffer can make the old shadow stale. The new DWORD and the EIP-after/code bytes are the primary evidence.
- Records are buffered in a 4096-entry fixed array and flushed by the worker thread. The log reports dropped records.
- A thread is eligible only when DR0 and DR1 address registers are both zero and all DR0/DR1 enable and RW/LEN bits are clear (`DR7 & 0x00FF000F == 0`). Existing active or dormant DR0/DR1 owners are not overwritten. Threads with occupied slots are tracked but skipped and are not retried; their writes will not be captured. Stage16 logs skipped thread counts.
- The COMvsCOM writer thread is excluded. DR2/DR3 are preserved for Stage11 command-resolver tracing.
- Stage16 and the old `EnableControlFieldTrace` mode cannot run at the same time because both need DR0/DR1. The supplied INI keeps `EnableControlFieldTrace=0`.
- If an exception reports simultaneous DR0/DR1 and DR2/DR3 hits, Stage16 records its data hit before passing the DR2/DR3 event to AICommandTrace.

The dedicated log is `ArcanaAI_PlayerBit17WriteTrace.log`.

## Build / install

Use a clean extraction of `ArcanaAIOverhaul_Stage11_Source.zip`, then apply the existing overlays in this exact order:

1. Stage12: replace `src/AIQueueTrace.cpp`, `Build.bat`, `ArcanaAIOverhaul.vcxproj`, and `ArcanaAI.ini` from `Stage12_Overlay`.
2. Stage13: replace `src/AICommandSiteTrace.cpp` and `ArcanaAI.ini` from `Stage13_Overlay`.
3. Stage14: replace `src/AICommandSiteTrace.cpp` and `ArcanaAI.ini` from `Stage14_Overlay`.
4. Stage15: replace `src/AICommandSiteTrace.cpp` and `ArcanaAI.ini` from `Stage15_Overlay`.
5. Stage16: replace `src/AITrace.cpp` and `ArcanaAI.ini` from this `Stage16_Overlay`.

No project-file or Build.bat change is required for Stage16: Stage11 already compiles `AITrace.cpp`, and `Proxy.cpp` already calls `StartAIFieldTrace`.

Build with the MSVC x86 toolchain using the existing Stage12 `Build.bat`. Retain the complete `Build.log`. Back up the known-working DLL and INI; do not install the new DLL unless the build succeeds.

After successful build:

1. Rename or move all previous runtime logs so this run starts with clean files.
2. Install the new DLL and Stage16 INI.
3. Run a short COM-vs-COM match that exercises the same CPU event path.
4. Close the game normally to allow worker buffers to flush.
5. Upload the fresh logs to this branch.

## Required upload after the run

- `Build.log`
- `ArcanaAI_PlayerBit17WriteTrace.log`
- `ArcanaAI_ProducerCallTrace.log` (contains Stage14 and Stage15 records)
- `ArcanaAI_AlternateCallTrace.log`
- `ArcanaAI_QueueTrace.log`
- `ArcanaAI_EventDispatchTrace.log`
- `ArcanaAI_FinalCommandTrace.log`

Keep `ArcanaAI_ParentFlowTrace.log` and `ArcanaAI_ProducerContextTrace.log` when available.

## How to validate the run

- Check for the Stage16 startup record with the exact P1/P2 watch addresses and their initial values.
- Confirm one or more `Hardware watchpoint install batch` records in the Stage16 log.
- Investigate every warning about skipped game threads or dropped records before interpreting the absence of writes.
- Confirm that records include both P1 and P2 sides where relevant.
- Disassemble each `eipAfter` from the saved code window. The writer instruction starts before EIP; do not treat EIP-after itself as the write instruction address. For `inAALib=0`, use the absolute EIP and byte window to identify the external in-process module/helper.
- Build a QPC/timestamp timeline and compare the write records with Stage15's F6A68 bit17 branch, F6A9D branch, F6AD7 call, QueueTrace, and EventDispatchTrace.
- For each write, compare the full DWORD and bit17 state. Distinguish a write that leaves bit17 unchanged from a true 0->1 or 1->0 transition.
- Confirm whether the statically known `0xED19F` path actually writes either watched player address, and whether other code sites set bit17.
- Do not claim exhaustive coverage if any relevant thread was skipped, record drops occurred, or the shadow may have missed a write.

## Status / limitations

This is a source-only diagnostic overlay. It has **not** been compiled with MSVC or runtime-tested in this environment. The user must verify a successful x86 build, the Stage16 startup record, actual watchpoint installation batches, and the new runtime traces before relying on its results. Stage12–Stage15 historical branches remain untouched.
