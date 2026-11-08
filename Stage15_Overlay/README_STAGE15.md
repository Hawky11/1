# Stage 15 — post-guard branch-flow diagnostics

Stage 15 extends the Stage 14 producer-call diagnostics on a new branch. It does not alter the Stage 12, Stage 13, or Stage 14 branches.

## Research target

Stage 14 confirmed four calls where F2F50 returned 0x11E (type candidate 1, payload candidate 0x1E). In all four cases F2ED0 returned AL=0; only one case proceeded through the alternate queue writer at F6AD7.

Stage 15 instruments the conditional decisions between the F2ED0 result at F6A58 and the alternate queue insertion at F6AD7. The probes also snapshot the relevant player fields, current queue pair, and selected parent locals.

## Five branch probes

| Probe | RVA | Original condition / decision |
|---|---:|---|
| F6A5D_F2ED0_BRANCH | 0xF6A5D | Original JNE F6AE8 after test al,al; observes the F2ED0 result |
| F6A63_PAYLOAD_BRANCH | 0xF6A63 | Replays cmp edi,1Eh; jne F6AC8 |
| F6A68_PLAYER_BIT17_BRANCH | 0xF6A68 | Reads player+0x3B84, replays shr eax,11h; test al,1; je F6A9D |
| F6A9D_LOCAL_GUARD_BRANCH | 0xF6A9D | Replays cmp dword ptr [ebp-4],2; jne F6AC8 |
| F6AAC_SECONDARY_HELPER_BRANCH | 0xF6AAC | Observes test al,al; jne F6AC8 after the secondary helper and replays the fall-through mov eax,[esi+3F30h] |

Every Stage 15 record is prefixed STAGE15_BRANCH and is appended to the existing ArcanaAI_ProducerCallTrace.log. For each branch record:

- branchTaken=1 means the original conditional jump would be taken; 0 means fall-through; NA means the required source value could not be read.
- zf is the saved Zero Flag at the hook entry.
- Player snapshots include fields 3A34, 3A36, 3B70, 3B7C, 3B80, 3B84, 3B88, 3F20, 3F30, 3F50, and 3F58.
- bit17 is bit 17 of player+0x3B84.
- queuePair reads the current queue words at queue+0x08 and queue+0x0A.
- localFC and localF8 are [EBP-4] and [EBP-8], respectively; localRead reports whether both reads succeeded.

## Behavior preservation

The Stage 15 patches are fingerprint-checked against the original instruction bytes before installation. Each patch is installed while the other process threads are suspended and while the existing patch-install mutex is held.

The branch shims preserve registers and EFLAGS while taking the snapshot, then replay the original comparisons/tests and make the same conditional decision:

- F6A5D branches on the original saved ZF from test al,al.
- F6A63 re-executes the original cmp edi,1Eh.
- F6A68 re-executes the original player-field load, shift, test, and bit-clear branch.
- F6A9D re-executes the original local comparison.
- F6AAC uses the original saved flags and replays the original fall-through MOV.

The Stage 15 hooks do not write player fields, queue contents, event types, or payloads. They do not force either branch outcome. Each of the five probes is installed independently; a failed byte fingerprint skips only that probe and is reported in the producer-call log.

Stage 15 reuses the Stage 14 bounded record buffer and worker flush. Keep Stage 14 enabled so both Stage 14 and Stage 15 records are flushed. The Stage 14 parent code dump also includes an original code window beginning at F6A5D for later disassembly.

## Apply and build

Start from a clean extraction of ArcanaAIOverhaul_Stage11_Source.zip.

1. Apply Stage 12: replace src/AIQueueTrace.cpp, Build.bat, ArcanaAIOverhaul.vcxproj, and ArcanaAI.ini from Stage12_Overlay.
2. Apply Stage 13: replace src/AICommandSiteTrace.cpp and ArcanaAI.ini from Stage13_Overlay.
3. Apply Stage 14: replace src/AICommandSiteTrace.cpp and ArcanaAI.ini from Stage14_Overlay.
4. Apply Stage 15 last: replace src/AICommandSiteTrace.cpp and ArcanaAI.ini from Stage15_Overlay.
5. The Stage 12 project already includes AICommandSiteTrace.cpp; no project-file change is required for Stages 14 or 15.
6. Build with the MSVC x86 toolchain using Build.bat and retain the complete Build.log.
7. Back up the known-working DLL and INI. Do not replace the DLL unless the Stage 15 build succeeds.
8. Rename prior logs, install the new DLL/INI, run a short COM vs COM bout, and close the game normally.

The Stage 15 INI enables both EnableStage14ProducerCallTrace=1 and EnableStage15BranchTrace=1. Keep the existing Stage 13 alternate-callsite, Stage 12 queue/dispatch, and Stage 11 final-command tracing enabled for correlation.

## Upload after the run

Required:

- Build.log
- ArcanaAI_ProducerCallTrace.log (contains both STAGE14_PROBE and STAGE15_BRANCH records)
- ArcanaAI_AlternateCallTrace.log
- ArcanaAI_QueueTrace.log
- ArcanaAI_EventDispatchTrace.log
- ArcanaAI_FinalCommandTrace.log

ParentFlow and ProducerContext logs may also be retained.

## Status

This is a source-only diagnostic overlay. It has not been compiled with MSVC or runtime-tested here. Verify the build log and the Stage 15 installation summary (branch probes installed=5/5) before relying on the new traces.