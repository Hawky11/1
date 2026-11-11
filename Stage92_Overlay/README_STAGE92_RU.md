# Stage92 r1 — native F5990 branch consequences (SOURCE ONLY)

Stage91 had 34,790 F58E0 bit reads, including 1,825 AL=1. Stage92 observes their immediate native branch consequences. **Status: not Windows-built or game-runtime tested.** Back up the tested Stage91 DLL (1,999,872 bytes).

## Native control flow

- ACAB8 calls original F5990(actor); Stage91's F5A4D/F5A5C/F5A76 read bitmap indices 9/10/11.
- A positive result for index9 or index10 reaches F5A68 OR [ESI+3B70],8. Stage92 observes the F5990 PRE/POST flag; it does **not** directly hook the F5A68 instruction.
- Index11 AL=1 reaches F5A86 CALL 9ED20; native 9ED20 sets P+3B70 bit3 (0x8).
- Index11 AL=0 reaches F5A8F CALL 9ED60; native 9ED60 clears that bit.
- The helpers use ECX as actor, stack argument bit index3, and RET 4. Stage92 passes control through a **tail JMP** so that the original helper itself pops the argument exactly once.

## Three guarded CALL sites

| Original call site | Native target | Observer |
|---|---|---|
| ACAB8 | F5990 | PRE/POST P+3B70 and P+3A0C, player/TID/QPC and Stage91 seq bracket |
| F5A86 | 9ED20 | Actual set-helper selection, parent tick and bit3 PRE |
| F5A8F | 9ED60 | Actual clear-helper selection, parent tick and bit3 PRE |

No global F58E0 detour, no duplicate EE306 hook, no gameplay/DR writes or synthetic commands. Full exact PE/opcode guards, peer thread suspension, patch mutex and rollback are implemented. Log is ArcanaAI_Stage92F5990BranchTrace.log, with STAGE92_TICK, STAGE92_HELPER, STAGE92_HEALTH.

## Run on Windows x86

1. Exit AALib.exe. Preserve last known-good Stage91 DLL (1,999,872 bytes) and source.
2. Extract source ZIP, then from its Stage92_Overlay folder run PowerShell:
   powershell -ExecutionPolicy Bypass -File .\ApplyStage92.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'
3. Build with existing Build.bat (MSVC x86), check BUILD SUCCEEDED. Keep EnableStage36ControlToCommandTrace=0, Stage87–91 enabled, EnableStage92BranchTrace=1.
4. Run 3–5 natural COM-vs-COM rounds. Confirm STAGE92_INSTALLED 3/3 and Stage92 HEALTH overflow/dropped/mismatch/readFail=0, alongside Stage91 installation.
5. Upload same-session Build.log, Stage92, Stage91, Stage90, Stage88, Stage87, FinalCommand and EventDispatch logs. Run the included Python correlator from the ZIP to verify same actor+thread+QPC and helper counts.
6. On compile/runtime failure, crash or diagnostic counters, stop the game and run RollbackStage92.ps1 or restore the Stage91 DLL.

Static original-PE/source audit: 67/67 PASS, plus 2 positive and 3 negative synthetic correlator tests PASS. No claim of COM action/hit or AI improvement until downstream event correlation.
