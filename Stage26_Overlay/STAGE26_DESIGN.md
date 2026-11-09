# Stage26 design and review

Date: 2026-11-09. Stage25 was compiled and run; its target +0xF8 bit0 already equalled 1 in the only captured event.

## Evidence and target

- Source P1 player `0x0E251F78`, target P2 `0x0E27CCA0`; Stage25 OR `[target+0xF8],1` at RVA F58C9 (patched by a faithful wrapper), field `0x0E27CD98` 1→1.
- User-uploaded Stage25 logs do not include a full AALib.exe binary, so a complete static scan of all +0xF8 writers cannot be verified from this repository. Known writer F58C9 is one site, not proven unique.
- P1 control field RVA DCA5EA4 and P2 control field RVA DCD0BCC, player base offset 3F2C. Target watchpoint RVAs:
  - P1: `0x0DCA5EA4 - 0x3F2C + 0xF8 = 0x0DCA2070`.
  - P2: `0x0DCD0BCC - 0x3F2C + 0xF8 = 0x0DCCCC98`.
- Stage26 uses the most recent version-controlled AITrace.cpp (Stage20 overlay) because Stage21–25 only changed AIQueueTrace.cpp and its INI.

## Chosen mechanism

1. Add a dedicated INI mode `EnableStage26ObjectBit0WriteTrace=1` and target selection `player+0xF8`. Reuse the proven Stage16 write-only hardware watchpoint pipeline, not a new inline hook.
2. Add a 3-way mutual-exclusion check with Stage16 (`player+0x3B84`) and legacy control-field mode; all occupy DR0/DR1.
3. Dynamically route the existing record buffer to `ArcanaAI_Stage26ObjectBit0WriteTrace.log` and print bit0 rather than bit17. Preserve Stage16's original format/field when its mode is selected.
4. Record eight DWORDs starting at trapped ESP (with read mask), in addition to EIP-after, QPC, registers, code window, field old/new, side and DR6. Source code never writes to the watched game field.
5. Default to `Stage26TransitionsOnly=1`. Filter no-bit0-change writes **after** updating the shadow. Suppressed writes do not consume sequence IDs. Emit occasional filtered count summaries and buffer-overflow warnings. `0` logs all observed writes, including the Stage25 OR's 1→1.
6. Validate pointer offsets and readable P1/P2 DWORDs before enabling DR0/DR1. Preserve pre-existing debug-register owners; refuse to modify threads that already use the slots. Keep DR2/DR3 fields and status bits untouched. Other passive hooks may run concurrently.

## Known limitations

- A hardware watchpoint triggers after a write and produces an *after-instruction* EIP.
- It only covers eligible threads after their DR0/DR1 have been installed; the initial snapshot may be late.
- OldValue is the last observed shadow and not an atomic pre-instruction memory read.
- A future write may be executed by the Stage25 DLL wrapper: report its DLL EIP, not an invented AALib RVA.
- Stage16 bit17 watchpoint logging is disabled during Stage26, so it cannot independently verify bit17 cycles in the same run. Stage17 setter and other independent probes may remain enabled.
- MSVC x86 compilation, binary/hook correctness, workload safety and runtime 0→1 evidence remain **untested**. Do not call Stage26 runtime-confirmed until the user uploads them.
