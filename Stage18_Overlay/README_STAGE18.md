# Stage18 — trace the bit-113 state-table gate result

## Goal

Stage17 confirmed the runtime caller that issues bit index 0x71 (113) to the generic player bitset setter:

- caller: AALib.exe+0x000C63B0
- setter CALL: AALib.exe+0x000C6502
- argument: 0x71
- target helper: AALib.exe+0x0000FAA0

Stage17 also showed that real setter calls used player+0x3A36 values outside the direct comparison set {0x1A, 0x18, 0x19}. The consistent route is the alternate branch accepted by helper 0x004F2FD0. Stage17 did not capture the helper's return AL, so Stage18 observes that result directly.

## What Stage18 adds

The new passive observer is installed at:

- helper call: RVA 0x000C64A0 → 0x000F2FD0
- observer site: RVA 0x000C64A5
- original continuation: RVA 0x000C64AA
- following branch: original JNE 0x000C64ED

The observer captures:

- helper return AL / EAX;
- player pointer, state-index argument, and key argument (0x19);
- caller frame and parent return/argument values;
- player fields that control table selection;
- primary and secondary table pointers;
- the derived table + stateIndex*0x1A + 0x19 byte address and its safely-read value.

The log is ArcanaAI_Bit113GateTrace.log.

The hook replaces only the original five-byte sequence 83 C4 0C 84 C0 (add esp,0x0c; test al,al). It accounts for its own CALL return address, performs the equivalent add esp,0x10, recreates test al,al, and resumes at the untouched original JNE. It does not modify the helper result or the game's decision.

The exact static helper and selector analysis is in STATIC_GATE_ANALYSIS.md.

## Build and install

Use a clean extraction of ArcanaAIOverhaul_Stage11_Source.zip, then apply the overlays in order:

1. Stage12: apply the Stage12 files as described in its README.
2. Stage13: replace src/AICommandSiteTrace.cpp and ArcanaAI.ini.
3. Stage14: replace src/AICommandSiteTrace.cpp and ArcanaAI.ini.
4. Stage15: replace src/AICommandSiteTrace.cpp and ArcanaAI.ini.
5. Stage16: replace src/AITrace.cpp and ArcanaAI.ini.
6. Stage17: replace src/AITrace.cpp and ArcanaAI.ini.
7. Stage18: replace src/AITrace.cpp and ArcanaAI.ini with the files in this overlay.

The Stage11 project already compiles AITrace.cpp; no new project-file or Build.bat entry is required. Build Release | Win32 (x86) and keep the complete Build.log. This repository stage is source-only: it has not been compiled with MSVC or runtime-tested here. Do not install the DLL unless the build and link both succeed. Keep a backup of the previously working dinput8.dll.

The Stage18 INI keeps the Stage16 and Stage17 evidence active:

    EnableControlFieldTrace=0
    EnablePlayerBit17WriteTrace=1
    EnableStage17Bit113SetterCallTrace=1
    EnableStage18Bit113GateTrace=1

All existing QueueTrace, EventDispatchTrace, FinalCommandTrace, alternate callsite, ProducerCall/Context and ParentFlow switches remain as in Stage17 so the same event path can be correlated.

## Startup validation

Start with clean log files. Before gameplay, ArcanaAI_Bit113GateTrace.log must contain:

- STAGE18_BIT113_GATE_BUILD=2026-11-08-r1
- STAGE18_INIT: FieldTraceThread entered.
- STAGE18_CONFIG: EnableStage18Bit113GateTrace=1 ...
- Installed Stage18 passive gate-result observer at RVA=0x000C64A5 ...
- Stage18 trace active.

The installer verifies the original argument setup and all three code signatures:

- helper arguments immediately before 0xC64A0: 6A 19 50 56 (push 0x19 / push EAX / push ESI);
- CALL at 0xC64A0: E8 2B CB 02 00, decoded target RVA 0xF2FD0;
- observer bytes at 0xC64A5: 83 C4 0C 84 C0;
- branch at 0xC64AA: 75 41.

If the marker is present but the observer-install line is absent, stop and inspect the preceding validation or patch warning; do not treat a build as a successful runtime installation.

## Runtime test and required upload

Run a short COM-vs-COM session exercising the same type-1 / payload-0x1E scenario. Close the game normally so buffered traces flush.

Upload:

- Build.log
- ArcanaAI_Bit113GateTrace.log
- ArcanaAI_Bit113SetterTrace.log
- ArcanaAI_PlayerBit17WriteTrace.log
- ArcanaAI_ProducerCallTrace.log
- ArcanaAI_AlternateCallTrace.log
- ArcanaAI_QueueTrace.log
- ArcanaAI_EventDispatchTrace.log

Keep ArcanaAI_FinalCommandTrace.log, ArcanaAI_ParentFlowTrace.log and ArcanaAI_ProducerContextTrace.log too when available.

## What the next analysis will establish

For each Stage18 result, compare:

1. resultAL with table.byte != 0xFF;
2. player and state index with Stage17's subsequent BIT113_SET_CALL;
3. QPC/time with the Stage16 player+0x3B84 0→1 write;
4. resultAL=1 records with whether the caller reached the final 0x495B00 bit-2 guard and the setter;
5. resultAL=0 records with early returns and later Stage15/QueueTrace observations.

Important: the Stage18 observer covers the helper-gated branch through 0x4F2FD0, not the direct 0x4C64B1 comparison route. Keep Stage17 enabled; it remains the proof of the actual setter callsite.
