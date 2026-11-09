# Stage25 — secondary-object flag observer

Apply on the working Stage24 project. This is source-only: no MSVC build or in-game installation has yet been observed.

1. Copy `Stage25_Overlay/src/AIQueueTrace.cpp` over project `src/AIQueueTrace.cpp`.
2. Copy `Stage25_Overlay/ArcanaAI.ini` over project `ArcanaAI.ini`.
3. Run the existing x86 Build.bat. No project-file changes are needed.
4. Install the newly built dinput8.dll and run a normal COM-vs-COM bout with fresh logs.
5. Upload Build.log and all runtime logs to `stage25-secondary-object-flag-trace`.

Expected log: `ArcanaAI_Stage25SecondaryObjectFlagTrace.log`.
Expected marker: `STAGE25_SECONDARY_OBJECT_FLAG_BUILD=2026-11-09-r1`.
Expected installation: `Installed Stage25 secondary-object post-OR observer at RVA 0x000F58C9`.

## Question answered

Stage24 mapped selector0x1F to (0x1F,0x10001,0). Flags03 also select a secondary path. This observer captures the original `OR DWORD PTR [EAX+0xF8],1` after the secondary call at F58C1. It reports source player, selector, primary helper arg3/arg4, returned target object and its known P1/P2 identity, field before/after, read status, QPC and EFLAGS.

For the observed P1 arg3=0, secondary helper 9EF00(0,-1) selects the other side's active-object slot. Stage24 EAX already matched P2. Stage25 tests the concrete field update; it does not establish the gameplay meaning of bit0.

## Preservation and validation

The original 7-byte OR is replaced by CALL plus two NOPs. The wrapper executes that OR exactly once for every invocation, regardless of logging filter. No additional game-state writes are introduced. It saves the previous field value, preserves all general registers and the original OR's EFLAGS, and restores caller ESP. Logging filters to known source P1/P2, dispatcher type1/payload0x1E. Stage24 remains enabled.

The installer validates the original secondary CALL/cleanup prefix, OR bytes and POP EBP/RET suffix against the known PE fingerprint; it revalidates under thread suspension and shared patch mutex. No secondary helper call is replayed: the game's original call is left intact.

Verified locally against SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba:
- exact instruction signatures and dispatcher argument-load bytes;
- extracted wrapper assembly assembled as x86;
- 128 randomized Unicorn comparisons against the original OR, covering general registers, stack, EFLAGS, output DWORD and logger context/local offsets;
- source diff and INI section review.

Run `python tests/verify_stage25.py /path/to/AALib.exe` with pefile/unicorn and GNU binutils for the assembly test. This test does not compile the Windows C++ translation unit. MSVC x86 Build.log and runtime marker/installation are still required.
