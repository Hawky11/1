# Stage22 — verify the payload-0x1E controller flag clear

Date: 2026-11-08  
Base source: Stage21 r2, branch \`stage21-payload-1e-consumer-trace-r2\`, HEAD \`28fd26b76c22b9070eabaa33a85c713876234aaf\`  
Target binary: \`AALib.exe\`, PE32 x86, ImageBase \`0x00400000\`, SizeOfImage \`0x0E0EB000\`, timestamp \`0x5FE5AA91\`

## Question

Stage21 observed the fallback helper at dispatcher CALL RVA \`0x000EE446\`, but its log captured the controller DWORD only **before** the following AND. The Stage21 r1 sample had bit \`0x400\` already clear. Stage22 records the exact DWORD on both sides of the real instruction, so a nonzero-to-zero transition can be proved rather than inferred.

## Static instruction window

The dispatcher sequence is:

\`\`\`asm
004EE443 push 3
004EE445 push edi
004EE446 call 004F7EB0
004EE44B mov esi, [esi+4]
004EE44E add esp, 8
004EE451 and dword ptr [esi+8], 0xFFFFFBFF
\`\`\`

The instruction targeted at RVA \`0x000EE451\` is seven bytes. Its expected x86 encoding is:

\`\`\`
81 66 08 FF FB FF FF
\`\`\`

The preceding six bytes are checked as well:

\`\`\`
8B 76 04 83 C4 08
\`\`\`

Installation is refused unless both the prefix and the exact seven instruction bytes match. The executable PE fingerprint is validated by the inherited Stage21/queue trace startup path.

## Stage22 hook behavior

- Replaces the seven-byte AND instruction with a five-byte relative CALL followed by two NOPs.
- The naked wrapper captures the controller pointer and its DWORD at \`[controller+8]\` before the write.
- It executes the original operation \`AND DWORD PTR [ESI+8], 0xFFFFFBFF\` exactly once.
- It captures the DWORD after the write, then invokes the diagnostic logger.
- The wrapper restores all general registers and restores EFLAGS produced by the original AND before returning to RVA \`0x000EE458\`.
- The wrapper does not force the bit, alter the mask, skip the original clear, or call the original AND twice. Logging adds timing overhead but preserves the instruction's register/memory/flag result.

The installer uses the shared patch-install mutex, suspends other process threads, rechecks the exact bytes immediately before patching, changes protection only for the seven-byte target, flushes the instruction cache, and restores page protection.

## Runtime record

New file: \`ArcanaAI_Stage22PayloadPostAndTrace.log\`

Expected marker:

\`\`\`
STAGE22_PAYLOAD_POST_AND_BUILD=2026-11-08-r1
\`\`\`

The filtered record is:

\`\`\`
STAGE22_POST_AND seq=... side=P1/P2 tid=... qpc=... callReturnRVA=0x000EE456 player=... dispatchType=1 dispatchPayload=0x0000001E controller=... controllerFlagsBeforeAND=... controllerFlagsAfterAND=... bit400Before=... bit400After=... afterReadable=1 eflagsAfterAND=...
\`\`\`

Interpretation:
- \`bit400Before=1 bit400After=0\`: the fallback path actually cleared the flag.
- \`bit400Before=0 bit400After=0\`: the instruction executed, but no bit transition occurred.
- \`afterReadable=0\`: the post-write state could not be safely read; do not infer a transition.
- \`side=P0\`: controller pointer did not match the currently resolved P1/P2 controller pointers; correlate by raw controller pointer and thread/QPC, do not assign a side.

Correlate this with the Stage21 \`STAGE21_FALLBACK_SLOT_CLEAR\` record using side, player/controller, thread ID, dispatch type/payload and QPC. Stage21 r2 also fixes the old decoded action-code field: \`decodedActionCodePreCall\` captures EDI before helper \`0x4E8CF0\`.

## Limitations

- This is source-only. No MSVC build or runtime installation is claimed.
- Stage22 measures the controller bit clear, not the slot-3 pre/post write; Stage21 already reads slot 3 before/after helper \`0x4F7EB0\`.
- A real nonzero-to-zero slot-3 transition still requires a natural event where slot 3 is nonzero on entry.
- A zero-to-zero controller sample is useful execution evidence but does not establish a state transition.
- Keep all Stage21 r1/r2 and earlier stage branches unchanged.
