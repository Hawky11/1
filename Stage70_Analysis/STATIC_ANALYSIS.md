# Stage70 r2 erratum — runtime original bytes must be relocation-aware

**2026-11-11: r1 runtime failed installation.** This Stage70 document describes the initial r1 source; the source at [Stage70 r2](../Stage70_Overlay/src/Stage70DeferredWriter.inl) was corrected after user log analysis. Original `AALib+15677D` uses an **absolute image-relocated 32-bit operand** at RVA `156780` (PE `IMAGE_REL_BASED_HIGHLOW`). At image base `00800000` its in-memory bytes are `8B 14 8D E8 54 C2 00`, while original-file bytes are `8B 14 8D E8 54 82 00`. Therefore r1's full seven-byte runtime guard fails. R2 matches only immutable `8B 14 8D` and tests relocated operand against `g_gameBase+0x4254E8`. Adjacent hook/displaced-instruction bytes remain exact. R2 adds `STAGE70_GUARD` diagnostics and PE relocation checks to the Python verifier. **R2 not built or runtime-tested yet.** See [r1 failure analysis](../Stage70_R1_Runtime/FAILURE_ANALYSIS.md) and [r2 fix instructions](../Stage70_R1_Runtime/README_STAGE70_R2_RU.md).

---

# Stage70 — native deferred writer hook, static-only release

**Date:** 2026-11-11. **SOURCE ONLY / ORIGINAL OPCODES VERIFIED / WINDOWS BUILD AND RUNTIME PENDING**.
Parent branch: \`stage69-deferred-writer-observation-design\`. Never modify the private original \`AALib.exe\`, game state, ID selection, AI movement or damage.

## Target

The original EXE SHA-256 remains \`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba\`. Original x86 RVA \`156784\` is a complete 3-byte \`8B 45 08\` \`mov eax,[ebp+8]\`; \`156787\` is a complete 3-byte \`89 50 68\` \`mov [eax+68],edx\`. The next instruction begins at \`15678A\` (\`8B 0F\`). Seven original preceding bytes \`8B 14 8D E8 54 82 00\` load the deferred action from the action table. Original contiguous verified block:

\`\`\`asm
15677D  8B 14 8D E8 54 82 00  mov edx,[ecx*4+008254E8]
156784  8B 45 08              mov eax,[ebp+08]
156787  89 50 68              mov [eax+68],edx
15678A  8B 0F                 mov ecx,[edi]
\`\`\`

Stage69 demonstrated an earlier native callback original CALL from RVA \`156769\` to Stage65-observed action request helper; fall-through reaches this original deferred write. The selector is reread after the call, and its writable index table might vary at runtime.

## Overlay, not full project

The full local compiled Stage67 source tree was not uploaded. Apply **exactly two replacement/source files** from \`Stage70_Overlay/src\` onto the working Stage67 source tree:

1. Add \`Stage70DeferredWriter.inl\` to \`D:\INCUBATOR\ArcanaAIOverhaul\src\`.
2. Replace the existing \`Stage38CallbackWindow.inl\` in that same directory with the Stage70 adapter.

Keep all other working Stage67 files, Build.bat, INI and AALib.exe **unchanged**. The modified adapter includes the new source after Stage67, installs Stage70 only if Stage67 installed successfully, **before** starting the independent Stage59 DR0/DR1 watch thread, and flushes Stage70 through the existing worker. No Stage70 DR registers, VEH, external debugger or local tool required.

### Single native patch

Only the six original bytes at \`AALib+156784..156789\` are patched to a 5-byte JMP plus NOP. The emitted probe executes the original **two instructions in order exactly once** after PRE snapshot, captures POST, then jumps to \`15678A\`. Trampoline preserves/restores native PUSHFD/PUSHAD, x87, XMM and MXCSR state via FXSAVE/FXRSTOR and restores ESP. The original native store is **not** bypassed or simulated.

**Installation gates:** x86 only, XMM available, Stage65 installed, Stage67 installed 2/2, Stage59 watcher not yet started, original bytes match 3 guard arrays (preceding, six-byte site, next), peer threads suspended/queried with \`CONTEXT_CONTROL\`, no peer EIP in patch window, single patch transaction and instruction cache flush. No DR context access or SetThreadContext, no additional VEH. A failed protect/cache flush after patch is invalid and requires game exit/rollback. New threads created between thread enumeration and patch are a residual race inherited from the existing install architecture; build/runtime evaluation remains essential.

### Data and proof standard

\`ArcanaAI_Stage70DeferredWriterTrace.log\` contains:

- \`STAGE70_DEFERRED_BUILD=2026-11-11-r1\`
- \`STAGE70_INSTALLED 1/1 site=156784 store=156787 resume=15678A\`
- \`STAGE70_DEFERRED_WRITE\`: original native site/operation labels, P1/P2 player P, object O, TID, QPC PRE/POST, old/new O+68, actual EDX operand, selector P+3F50 PRE/POST, effective object pointer P+4 PRE/POST, full masks \`07/07\`, native EAX PRE/POST and EDX POST
- \`STAGE70_HEALTH\`: calls, filtered, dropped, overflow, mismatch, readFail (must all be zero except calls/filtered).

The \`originalStoreRVA=156787\` field is a static exact-original-site attribution: **during the patch, CPU EIP is in the DLL while replaying the instruction**. It must not be mislabeled a directly sampled CPU EIP of unpatched AALib.exe.

A qualifying direct source observation is a Stage70 row with full masks, the actual same object P/O/TID, post \`O+68 == EDX\`, and no health errors. Stage65 same original caller \`156769\` + following Stage41 same-actor PRE seeing deferred ID + subsequent Stage67 promotion adds a stronger joined evidence chain. A missing \`25\`/ \`26\` event in one short run is an absence of coverage, not a disproof.

## Reproducibility and safety gates

First, from this branch root with an unmodified private original EXE:

\`\`\`powershell
py -3 .\Stage70_Analysis\tools\verify_stage70.py "D:\PATH\TO\AALib.exe"
py -3 .\Stage70_Analysis\tools\test_stage70.py
\`\`\`

Static verifier asserts Stage69 EXE SHA + original 27 native sites and table values, Stage70 exact 3 adjacent byte arrays, the one-and-only-one displaced original instruction sequence, register/FPU/stack preservation assembly shape, DR/VEH non-usage and correct adapter ordering. Synthetic correlator tests cover positive and negative join logic. This is **not a differential execution of compiled x86 instrumentation**. A full extracted-wrapper differential test under Unicorn or an equivalent tool and an actual Windows MSVC x86 build remain pending.

Preserve known-good Stage67 DLL **1,887,232 bytes**, Stage65 DLL **1,879,552 bytes**, and Stage59 r2 DLL **1,153,024 bytes**. Do not run the new DLL unless the static gate + 32-bit build pass and existing Stage67 probes remain healthy. Before runtime, archive old logs and capture one ordinary COM-vs-COM session, then allow ≥2 seconds for worker flush before exit.

## Runtime artifacts and offline join

Minimum same-run files: \`Build.log\`, Stage70 log, Stage65 log, Stage41 log, Stage67 log, Stage59 log. The additional Stage40/42 logs are useful to confirm activation but not required by \`correlate_stage70.py\`. If no rebuild occurred, do not claim Stage70 test.

\`\`\`powershell
py -3 .\Stage70_Analysis\tools\correlate_stage70.py "D:\PATH\TO\LOGS" --output stage70_results.json
\`\`\`

The correlator rejects mixed session headers, missing/duplicated sequences, nonzero health errors, identity mismatches, wrong native site, impossible new-value/EDX combinations, and incomplete PRE/POST masks. It links only strict same-actor/time-ordered Stage65→Stage70→Stage41→Stage67 windows and reports nonmatched stages rather than inventing a COM choice.

**Stage70 exit status:** guarded probe **SOURCE READY**, native bytes independently verified from original SHA-pinned EXE; compiled wrapper/ABI differential and live effect remain **UNVERIFIED**. Do not equate source correctness with harmless gameplay behavior.
