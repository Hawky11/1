# Next session handoff

**Read this file first in a new chat.** Inspect current GitHub branch HEADs and uploaded files before asking the user for anything.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- Latest runtime evidence branch: `stage17-callsite-init-diagnostics`
- Latest runtime evidence HEAD: `dcfd1efbe221438b1f2b3fae800ca76b1fa87e7d`
- Current source-only Stage18 branch: `stage18-bit113-gate-condition-trace`, HEAD `de8555f1eb34f647711595493ecee2cec22fb120`; it is pending MSVC build and runtime verification.
- Earlier Stage17 pre-fix branch (preserve unchanged): `stage17-bit113-setter-callsite-trace`, HEAD `0c2411187c47e56a6cb675b597eeb8a754f8f8a6`.
- Earlier Stage17 entry-hook source branch (preserve unchanged): `stage17-bit113-setter-caller-trace`, HEAD at last review `6b04b0a9ad272aecf8dcb481b5c52edfe5acfe1e`.
- Stage16 baseline evidence branch: `stage16-player-bit17-writer-trace`, HEAD `58313d8c0f6071bc8099fc5ec04ae04d2974dafd`.
- Stage17 overlay/readme: [Stage17 README](https://github.com/Hawky11/1/blob/stage17-callsite-init-diagnostics/Stage17_Overlay/README_STAGE17.md).
- Full chronology: [PROJECT_HISTORY.md](PROJECT_HISTORY.md).

## Latest build result — successful

The user supplied Build.log dated 2026-11-08 16:32:02. VS 2026 18.9.1 / MSVC 14.51.36231 x86 compiled all eight C++ translation units and linked `build\\dinput8.dll`, size **198,144 bytes**. One non-fatal C4754 warning remains in `AICommandSiteTrace.cpp` lines 782–785. Build.log was pasted in chat, not uploaded in the branch tree.

## Stage17 runtime result — caller attribution confirmed for this run

Runtime run: 2026-11-08, initialization at 16:34:05, relevant events 16:34:29–16:34:44.

- Stage17 r2 marker, worker entry, and INI settings are present.
- The passive observer installed successfully at callsite RVA `0x000C6502`, return RVA `0x000C6507`, original helper RVA `0x0000FAA0`; `suspended=34`.
- `ArcanaAI_Bit113SetterTrace.log` contains nine `BIT113_SET_CALL` records; every record uses `index=0x00000071`, the exact call/return RVAs, and the correct P1/P2 pointer.
- Stage16 PlayerBit17WriteTrace contains 20,456 records and eight actual bit17 transitions (four sets and four clears). There are 62 thread-watchpoint installations across 10 batches, with no warning/error/drop/skip lines.
- All four Stage16 0→1 writes at RVA `0x0000FAD2` have a preceding Stage17 call from `0x000C6502` with the same player pointer and bit index. QPC deltas are 367, 520, 396, and 353 ticks. The four corresponding clears again execute `0x000ED19F` (post-EIP `0x000ED1A9`).
- Five additional Stage17 calls occurred while the bit was already set; they did not create extra transitions.
- Four Stage15 F6A68 events all saw bit17=1 and the original JZ was not taken. No F6A9D/F6AAC/F6AD7 call was recorded; QueueTrace and EventDispatchTrace contain no type-1 / payload-`0x1E` insertion/dispatch in this run. This is a suppression-side sample, not a successful queue insertion sample.
- No warning/error/drop reports were found in the reviewed Stage17, Stage16, producer, queue, dispatch, or alternate-callsite logs.

**Conclusion:** the candidate direct caller at `AALib.exe+0xC6502` is runtime-confirmed as the origin of all four bit17 set transitions observed in this run. This does not prove it is the only possible setter path for every game state. Preserve the current evidence branch unchanged.

## Stage18 source-only status — build and runtime validation are next

- Source branch: [stage18-bit113-gate-condition-trace](https://github.com/Hawky11/1/tree/stage18-bit113-gate-condition-trace), HEAD \`de8555f1eb34f647711595493ecee2cec22fb120\`.
- The Stage17 runtime evidence branch remains unchanged and remains the latest completed runtime test.
- Stage18 adds a passive observer at RVA \`0x000C64A5\`, immediately after helper \`0x000F2FD0\` returns to caller \`0x000C63B0\`.
- New overlay files: \`Stage18_Overlay/src/AITrace.cpp\`, \`Stage18_Overlay/ArcanaAI.ini\`, \`README_STAGE18.md\`, and \`STATIC_GATE_ANALYSIS.md\`.
- Stage18 output: \`ArcanaAI_Bit113GateTrace.log\`.
- **No Stage18 MSVC build, DLL link, hook installation, or runtime test has been done.** Do not install or run until a successful local build is confirmed.

### Why this probe exists

The Stage17 callsite records' player+0x3A36 low words were 0x42 / 0x22 / 0x20 / 0x5E, not the direct-check values 0x1A / 0x18 / 0x19. Static disassembly shows the alternate path calls 0x4F2FD0 with player, state index from 0x4F6ED0, and key 0x19. Helper 0x4F2FD0 returns AL=1 exactly when its selected table byte at table + stateIndex*0x1A + 0x19 is not 0xFF. AL drives the JNE at RVA 0xC64AA.

The Stage18 observer logs actual AL/EAX, arguments, caller frame, selector-driving player fields, selected table pointers and the corresponding byte. It uses the original helper result, restores the original branch semantics, and performs bounded diagnostic reads only. It does not observe the alternative direct-check route at RVA 0xC64B1; Stage17 remains enabled to identify the actual setter caller.

### Exact next operation

1. Open the Stage18 branch README and apply the Stage18 overlay files over the already assembled Stage17 project. Preserve the exact historical Stage17 branch and logs.
2. Verify the marker in the assembled source:
   \`STAGE18_BIT113_GATE_BUILD=2026-11-08-r1\`.
3. Build **Release | Win32 (x86)** with MSVC. Keep the full Build.log. Do not install or launch the game if compilation or linking fails; upload Build.log for source correction.
4. After a successful build only, back up the currently installed game-folder dinput8.dll, install the newly linked DLL and matching Stage18 ArcanaAI.ini beside the actual game executable / AALib.exe.
5. Before gameplay, verify ArcanaAI_Bit113GateTrace.log contains the Stage18 marker, config with EnableStage18Bit113GateTrace=1, and successful installation at RVA 0xC64A5. If validation fails, stop.
6. Run a short COM-vs-COM test covering the same scenario and close the game normally.

### Files to upload after the run

Build.log, ArcanaAI_Bit113GateTrace.log, ArcanaAI_Bit113SetterTrace.log, ArcanaAI_PlayerBit17WriteTrace.log, ArcanaAI_ProducerCallTrace.log, ArcanaAI_AlternateCallTrace.log, ArcanaAI_QueueTrace.log, and ArcanaAI_EventDispatchTrace.log. Keep FinalCommandTrace, ParentFlowTrace and ProducerContextTrace as supporting logs when available.

### Analysis target

For each Stage18 record:
- Compare resultAL with whether the corresponding derived state-table byte is not 0xFF.
- Match player and stateIndex to a Stage17 BIT113_SET_CALL.
- Correlate QPC against the Stage16 0→1 write at player+0x3B84.
- Separate AL=1 records followed by the final 0x495B00 related-object guard/setter from AL=0 early returns.
- Look for repeated AL=1 invocations while bit17 is already set.

The four Stage17 QPC correlations (367, 520, 396, 353 ticks) and the suppression-side Stage15 evidence remain the runtime baseline. No semantic name should be assigned to table indices or bytes without additional evidence.

## Historical branch rules

- Do not rewrite Stage12–Stage16 evidence branches.
- Keep `stage17-bit113-setter-caller-trace` and `stage17-bit113-setter-callsite-trace` distinct; they use different Stage17 hook placements.
- Append each completed analysis to `PROJECT_HISTORY.md`, update this handoff, and keep `README.md` as the entry point.
- Distinguish successful compilation from hook installation; only runtime log evidence proves the latter.
