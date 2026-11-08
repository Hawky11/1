# Next session handoff

**Read this file first in a new chat.** The user may provide only the repository link. Inspect the current branch HEAD and files before asking for material; the user has already uploaded Stage16 runtime logs.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- Latest runtime-evidenced stage: **Stage16 — player bit17 setter/clearer trace**
- Current diagnostic/evidence branch: `stage16-player-bit17-writer-trace`
- Stage16 HEAD at this handoff: `58313d8c0f6071bc8099fc5ec04ae04d2974dafd`
- Previous evidence branch (unchanged): `stage15-postguard-flow-diagnostics`, HEAD `4d1e2a049c835a991647cb04565d31d330aeac95`
- Full Stage16 analysis: [Stage16_Overlay/RUNTIME_ANALYSIS.md](https://github.com/Hawky11/1/blob/stage16-player-bit17-writer-trace/Stage16_Overlay/RUNTIME_ANALYSIS.md)
- Stage16 overlay instructions: [Stage16_Overlay/README_STAGE16.md](https://github.com/Hawky11/1/blob/stage16-player-bit17-writer-trace/Stage16_Overlay/README_STAGE16.md)
- Static reference review: [Stage16_Overlay/STATIC_XREF_REVIEW.md](https://github.com/Hawky11/1/blob/stage16-player-bit17-writer-trace/Stage16_Overlay/STATIC_XREF_REVIEW.md)
- Full chronology: [PROJECT_HISTORY.md](PROJECT_HISTORY.md)

Always re-check the remote branch HEAD and changed files first. If the user has uploaded a newer Stage16 or Stage17 run, inspect those changes before continuing. Stage12–Stage15 historical branches must remain unchanged.

## What Stage15 established

The event of interest is type `1`, payload `0x1E`, produced in the parent routine at `AALib.exe+0xF69F0`.

In the last Stage15 analysis, the route from the parent routine through the alternate writer at `AALib.exe+0xF6AD7` was instrumented at five branch points. The uploaded Stage15 run found that bit17 in `[player+0x3B84]` determined whether the original JZ at F6A68 was taken. This was the hypothesis Stage16 was built to test.

## Stage16 runtime findings (2026-11-08)

Run window: `13:02:27.719`–`13:03:18.651`.

Uploaded files include:
- `ArcanaAI_PlayerBit17WriteTrace.log` — 30,114 write records.
- `ArcanaAI_ProducerCallTrace.log` — Stage14 and Stage15 probes.
- `ArcanaAI_AlternateCallTrace.log`
- `ArcanaAI_QueueTrace.log`
- `ArcanaAI_EventDispatchTrace.log`
- `ArcanaAI_FinalCommandTrace.log`
- `ArcanaAI_ParentFlowTrace.log`
- `ArcanaAI_ProducerContextTrace.log`

### Instrumentation coverage

Stage16 startup confirms:
- P1 player `0x0E9C1F78`; watched field `0x0E9C5AFC` (AALib RVA `0x0DCA5AFC`).
- P2 player `0x0E9ECCA0`; watched field `0x0E9F0824` (AALib RVA `0x0DCD0824`).
- Both initial values were `0x00000000`.
- DR7 configuration `0x00DD0005` (local, write-only, four-byte DR0/DR1 watchpoints).
- Eight installation batches recorded 60 game-thread watchpoint installations.
- The Stage16 log contains no skipped-thread, dropped-record, or warning/error entries.

**Build.log was not uploaded.** Runtime records prove the Stage16 module ran and installed watchpoints, but the compiler transcript, warnings, and exact build command were not independently reviewed.

### Confirmed bit17 setter

All five observed bit17 0→1 transitions came from:
- VA `0x0040FAD0` / RVA `0x0000FAD0`:
  `or dword ptr [eax], edx`
- EAX was the exact P1/P2 watched field address.
- EDX was `0x00020000`.

This instruction is inside a generic player bitfield setter at VA `0x0040FAA0` / RVA `0x0000FAA0`. It accepts a bit index:
- Bitfield base is `player+0x3B78`.
- Bit index `0x71` (113) maps to DWORD `player+0x3B84`.
- 113 modulo 32 is 17, so the mask is `0x00020000`.

The same helper had 85 observed writes: 10 with EDX=`0x00020000` (five real 0→1 transitions and five repeated 1→1 writes), and 75 with EDX=`0x00400000` (bit 22, not bit17).

### Strong static caller candidate, not yet runtime-proven

Among 129 direct call references to the generic helper `0x0040FAA0`, one passes literal index `0x71`, at VA `0x004C6502`:

```asm
004C64ED  mov ecx, dword ptr [ebx]
004C64EF  push 2
004C64F1  call 00495B00
004C64F6  test al, al
004C64F8  jne 004C640B
004C64FE  push 0x71
004C6500  mov ecx, esi
004C6502  call 0040FAA0
```

The preceding helper tests bit index 2 in the `+0x3B70` bitfield of the object in ECX. When it returns `AL=0`, this caller sets bit113 on the player object in ESI.

This is the only direct call reference found with an immediate `push 0x71`, making it the strongest static candidate. **Stage16 did not record the generic setter's return address**, so do not state that runtime setter invocations have been proven to originate at `0x004C6502`. A different call site could theoretically supply 113 dynamically.

### Confirmed bit17 clearer

All five observed bit17 1→0 transitions came from VA `0x004ED19F` / RVA `0x000ED19F`:

```asm
004ED188  push 0x1E
004ED18A  lea eax, [esi+0x3A2C]
004ED190  push 1
004ED192  push eax
004ED193  call 004F2ED0
004ED198  add esp, 0x0C
004ED19B  test al, al
004ED19D  jne 004ED1A9
004ED19F  and dword ptr [esi+0x3B84], 0xFFF9FFFF
004ED1A9  mov eax, [esi+0x3978]
```

For every transition, ESI matched the exact watched P1/P2 player object. The preceding F2ED0 helper returned `AL=0`; the mask clears bits 17 and 18. The clear instruction was observed 597 times, but only five executions changed bit17 1→0.

### Correlation with the parent branch and queue

Six matching parent events were recorded in this Stage16 session. For all six:
- F2F50 returned EAX=`0x11E`.
- F2ED0 returned `AL=0`.
- EDI=`0x1E` passed the payload comparison.

Five events:
1. Generic setter changed bit17 from 0 to 1 before F6A68.
2. F6A68 observed bit17=1; original JZ was not taken.
3. The event did not reach F6A9D/F6AD7.
4. RVA `0xED19F` then cleared bit17.

The sixth event (P1, `13:03:12.258`) arrived at F6A68 with bit17=0:
1. F6A68 took its JZ.
2. F6A9D was reached.
3. F6AD7 queued type `1`, payload `0x1E`.
4. QueueTrace reported `ACCEPTED_BY_QUEUE`.
5. EventDispatchTrace observed the same event.

FinalCommandTrace has no matching payload `0x1E` entry because this successful event used the alternate writer at F6AD7, not the F7C5D/F7C62 callsite.

This strongly ties temporary bit17=1 to suppression of this particular type-1/`0x1E` branch in the six sampled events. Do not generalize beyond the observed sample or declare the full semantic meaning of bit113 solved yet.

## Next task: Stage17 — verify who calls the bit113 setter

Recommended branch name: `stage17-bit113-setter-caller-trace`, based on the current Stage16 HEAD above after rechecking it.

Add a passive, narrowly scoped trace to the generic setter at VA `0x0040FAA0` (or to the candidate call at `0x004C6502`) that records only calls with bit index `0x71`. Capture:
- caller return address;
- ECX/player object;
- stack bit-index argument (expected `0x71`);
- EBP and outer caller/frame context;
- registers and timestamp/QPC.

The target is to verify whether the runtime setter calls actually come from `0x004C6502`, identify the caller above `0x4C63B0` (static callers include `0x4967FC` and `0x4F6FD0`), and expose the condition that sets bit113. Preserve the original helper arguments, return value, flags, and behavior exactly. Use the same PE fingerprint and byte/target validation as previous probes, coordinate code patch installation with the shared patch mutex, and do not overwrite occupied debug-register ownership.

A useful refinement is to log the preceding `0x00495B00` test result / the branch condition at `0x4C64F6`, but the mere presence of the call at `0x4C6502` already demonstrates that the test returned AL=0.

After Stage17 is prepared:
1. Update the Stage17 README with exact source overlay/build steps.
2. Do not modify Stage16 or Stages12–15; keep them as historical evidence.
3. Ask the user to build/run only after the source overlay and safety validation are ready.
4. Require fresh Build.log and runtime logs before claiming Stage17 instrumentation succeeded.
5. After upload, correlate setter caller records with bit17 watch events and Stage15 branch/QPC records.
6. Append actual results to PROJECT_HISTORY.md and refresh this handoff and README.

## Rules for future sessions

- Inspect repository files and existing Library uploads before requesting anything from the user.
- Never modify Stage12–Stage16 historical evidence branches after a run has been analyzed.
- Never infer build/compiler success from runtime logs; report the missing Stage16 Build.log limitation accurately.
- Distinguish proven writer instructions from static caller hypotheses.
- Never invent hook installation, build results, or log contents.
- Keep the next task clear; do not ask for information already present in the repository or conversation.
