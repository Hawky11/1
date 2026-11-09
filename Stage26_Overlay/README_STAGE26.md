# Stage26 — trace first writer of P1/P2 +0xF8 bit0

**Status: source prepared; not yet compiled with MSVC or tested in-game.**

Previous completed run: Stage25 evidence at `stage25-secondary-object-flag-runtime-evidence`, commit `b426a8f74764b7606ce3b5da075bb3a61d3698b3`. P1 payload 0x1E selected target P2; original OR at F58C9 wrote P2+`0xF8` from 1 to 1. It did not identify the initial setter.

## Apply (two files)

On the working **Stage25** project replace:

1. `src/AITrace.cpp` with `Stage26_Overlay/src/AITrace.cpp`
2. `ArcanaAI.ini` with `Stage26_Overlay/ArcanaAI.ini`

**Keep the Stage25 version of `src/AIQueueTrace.cpp`.** Nothing in Stage26 patches the original AALib.exe and no extra C++ translation unit or csproj modification is needed. Run the existing x86 `Build.bat`.

After BUILD SUCCEEDED, install the resulting `build/dinput8.dll`, launch a fresh COM-vs-COM session, play long enough to capture the initialization/bout state, then upload **Build.log and all runtime logs to `main`**. Do not overwrite a prior evidence branch.

## Verify at runtime

- New log: `ArcanaAI_Stage26ObjectBit0WriteTrace.log`.
- Marker: `STAGE26_OBJECT_BIT0_WRITE_BUILD=2026-11-09-r1`.
- Expect `STAGE26_CONFIG` with `EnableStage26ObjectBit0WriteTrace=1`, `EnablePlayerBit17WriteTrace=0`, `EnableControlFieldTrace=0`.
- Expect `STAGE26 active` header with both `player+0xF8` addresses, initial DWORDs, DR7=0x00DD0005, and `Hardware watchpoint install batch` messages. If no game thread accepts DR0/DR1, do **not** interpret an empty trace as absence of writes.
- For bit0 transitions: `STAGE26_OBJECT_BIT0_WRITE seq=...`, `oldBit0=0 newBit0=1` or `oldBit0=1 newBit0=0`. Captures QPC, source thread, post-instruction EIP/RVA, registers, eight DWORD stack snapshot, DR6, and a 24-byte code window around EIP.
- Check `ArcanaAI_Stage25SecondaryObjectFlagTrace.log` for original OR behavior, including no-change writes, which are suppressed by Stage26's default transition-only filter.
- Check `STAGE26_FILTER_STATUS` (emitted each 250 suppressed writes) and any dropped-record warnings.
- For complete write traffic at the cost of larger logs, set `Stage26TransitionsOnly=0` before launch.

A field that is already 1 at initial baseline **cannot** identify its earlier setter. The pre/post DWORD pair is a watchpoint shadow, not guaranteed to be the immediately preceding write when other threads were not covered. The observed EIP is **after** the writing instruction; Stage25's wrapper OR executes in the DLL, so its EIP may be outside AALib.exe (`inAALib=0`). Use code bytes and QPC correlation rather than assigning an unsupported AALib RVA.

## Safety and preservation

The Stage26 mode reuses Stage16's existing single-step VEH, write-only DR0/DR1 configuration and thread-scanning installation. Stage16 bit17 and legacy read/write control mode **must be disabled**; all three modes are checked for conflicts. Existing DR0/DR1 owners and unrelated debug-register fields remain untouched; COMvsCOM writer thread is excluded. Stage17-20 and Stage21-25 hooks remain unchanged. Observations are passive; no new game-memory writes or inline patches.

The source branch is a **build candidate only**. `tests/verify_stage26.py` is a static safety/INI check, not a substitute for a compiler, instruction-level emulation or game capture.
