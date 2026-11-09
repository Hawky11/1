# Stage24 — resolver consumer trace

Stage24 follows the immutable Stage23 r2 source and instruments the downstream consumer that receives the resolver-selected byte.

## Target

- Caller: `AALib.exe+0x000EE306`
- Original helper: `AALib.exe+0x000F57D0`
- Output structure: `player+0x3A08` (three DWORDs)
- Runtime table base: `g_gameBase + 0x049DF188` (preferred VA `0x04DDF188`, preferred ImageBase `0x00400000`)
- Target event filter: known P1/P2 player, dispatcher type 1, payload `0x1E`
- Marker: `STAGE24_RESOLVER_CONSUMER_BUILD=2026-11-09-r1`
- Dedicated log: `ArcanaAI_Stage24ResolverConsumerTrace.log`

Stage23 remains enabled in the supplied INI, so the same run can correlate resolver byte +3 with the actual helper arguments and output structure.

## What it records

For each matching call, the wrapper captures the original four cdecl arguments. Expected for the previously observed payload-`0x1E` resolver records:

| Argument | Expected value | Meaning |
|---|---:|---|
| arg1 | `player+0x3A08` | output structure pointer |
| arg2 | `0x1F` | resolver mapping selector |
| arg3 | `0x03` | resolver record byte +2 |
| arg4 | `0x12` | resolver record byte +4 |

Immediately before calling the original helper, it snapshots:
- the three table bytes at `g_gameBase + 0x049DF188 + selector*3` (labelled `tableBytesBeforeHelper`);
- the three DWORDs at `player+0x3A08`.

Static flow shows the primary helper makes an unconditional call to `0x0049EEA0` before it loads the mapping bytes. Stage24 does not hook that internal call or the exact table-load instruction. Therefore the recorded table bytes are the pre-entry snapshot; if the internal call were to modify the table, those bytes could differ from the values later consumed.

Immediately after the original helper returns, it reads the same three output DWORDs again. Read masks distinguish successful reads from zero values.

The log also reports two **static expectations** derived from the table flag byte:
- bit 0: the helper's flag-controlled initialization clears output DWORDs at offsets +4 and +8;
- bit 1: the helper follows its conditional secondary-call path.

These two fields are expectations from the inspected control flow; Stage24 does **not** add a second hook at the secondary helper and does not claim to observe that helper's execution directly.

## Safety and preservation

- The installer checks the 10-byte callsite prefix, exact five-byte CALL, 15-byte suffix, helper prologue, decoded rel32 destination, and known AALib PE fingerprint.
- The wrapper copies the four original stack arguments, replays them exactly once, restores the original helper return registers/EFLAGS, and returns with the caller-owned arguments and original cleanup intact.
- The table and output structure are read only. No game values or mapping entries are changed.
- The patch uses the shared mutex, thread suspension, immediate byte revalidation, instruction-cache flush, and page-protection restoration.
- The Stage23 source branch and both Stage23 runtime-evidence branches remain unchanged.

## Apply

Apply this Stage24 overlay on top of the assembled Stage23 r2 project:

1. Replace project `src/AIQueueTrace.cpp` with `Stage24_Overlay/src/AIQueueTrace.cpp`.
2. Replace project `ArcanaAI.ini` with `Stage24_Overlay/ArcanaAI.ini`.
3. Build `Release | Win32` with the existing MSVC x86 workflow.
4. Upload the complete `Build.log` before installing or testing the DLL.
5. After a successful build, use a fresh runtime-log folder and run a normal COM-vs-COM bout.

No project-file change is required; `AIQueueTrace.cpp` is already part of the build.

## Required runtime evidence

- `ArcanaAI_Stage24ResolverConsumerTrace.log` — must contain the r1 marker and successful installation at `0x000EE306 -> 0x000F57D0`.
- `ArcanaAI_Stage23ResolverResultTrace.log` — resolver records and QPC correlation.
- `ArcanaAI_Stage21PayloadConsumerTrace.log` and `ArcanaAI_Stage22PayloadPostAndTrace.log`.
- `ArcanaAI_QueueTrace.log` and `ArcanaAI_EventDispatchTrace.log`.
- Same-run Stage18 gate, Stage20 object guard, Stage17 setter, Stage16 writer and Stage19 state-map logs when available.
- Complete `Build.log`.

## Status

**Source-only.** The Stage24 overlay, INI and static analysis are committed, but this exact source has not yet been compiled or run. Do not infer successful hook installation from source presence. Verify the Stage24 marker and installation line in the dedicated runtime log after a successful user-side build.
