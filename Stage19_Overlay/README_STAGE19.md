# Stage19 — trace the bit-113 state-code mapping

## Goal

Stage18 established that the 0x4F2FD0 table gate returned AL=1 for all six captured helper calls and that the three inspected table bytes were 0xFA. A byte-for-byte comparison with the matching on-disk AALib.exe now shows that those three 0xFA values are already present in the PE's initialized .data image at their preferred VAs; the runtime addresses are the same values after relocation. This does not rule out later runtime mutation, but it removes the need to assume a writer for those three initial bytes.

Stage19 moves one step earlier in the pipeline: trace how the raw player state code becomes the stateIndex consumed by Stage18.

## Static path

- Wrapper: 0x004F6ED0 → 0x004F6BA0.
- Mapping helper: 0x004F02C0.
- Caller pushes raw code and player at RVA 0x000F6C01–0x000F6C03.
- New observer site: RVA 0x000F6C08, after helper return and before CMP EAX,5 / JNE.
- Original branch continuation: RVA 0x000F6C0E.

The hook logs the 0x4F02C0 return EAX, raw code, associated player WORD flags, derived expected stateIndex, and QPC. It recreates the original ADD ESP,8 and CMP EAX,5 behavior exactly after removing its extra CALL return address and the original cdecl arguments.

## What Stage19 adds

- Passive hook at RVA 0x000F6C08.
- New log: ArcanaAI_StateMapTrace.log.
- INI switch: EnableStage19StateMapTrace=1.
- Stage16, Stage17 and Stage18 traces remain enabled by the supplied INI.
- The observer does not consume DR0–DR3 and does not change the helper's EAX or original branch decision.

See STATIC_STATE_MAP_ANALYSIS.md for the exact flag-to-index mapping, PE/runtime table-cell comparison, hook byte signatures, and coverage limitations.

## Build and install

Start from a clean extraction of ArcanaAIOverhaul_Stage11_Source.zip and apply overlays in order:

1. Stage12 overlay.
2. Stage13 overlay.
3. Stage14 overlay.
4. Stage15 overlay.
5. Stage16 overlay.
6. Stage17 overlay.
7. Stage18 overlay.
8. Stage19 overlay: replace src/AITrace.cpp and ArcanaAI.ini with the files in this overlay.

No project-file entry is needed because AITrace.cpp is already part of the Stage11 project. Build Release | Win32 (x86) and keep the complete Build.log. This branch is source-only; it has not been compiled with MSVC. Do not install a DLL unless both compilation and link succeed. Back up the currently working dinput8.dll before installing.

## Startup validation

Start with clean logs. Before gameplay, ArcanaAI_StateMapTrace.log must contain:

- STAGE19_BIT113_STATE_MAP_BUILD=2026-11-08-r1
- STAGE19_INIT
- STAGE19_CONFIG with EnableStage19StateMapTrace=1
- successful installation at RVA 0x000F6C08, patchLength=6
- Stage19 trace active

The installer validates:
- push ESI / push EDI before the helper call;
- CALL at 0x000F6C03 resolves to 0x000F02C0;
- bytes at 0x000F6C08 are 83 C4 08 83 F8 05;
- branch at 0x000F6C0E is 75 0A.

If the marker exists without the installation line, stop and diagnose the Stage19 validation or patch error. Do not treat it as a successful hook.

## Runtime test and required upload

Run a short COM-vs-COM test that reproduces the same class of events. Do not force-edit state bytes or table entries to manufacture AL=0. Keep the runtime natural and collect:

- Build.log
- ArcanaAI_StateMapTrace.log
- ArcanaAI_Bit113GateTrace.log
- ArcanaAI_Bit113SetterTrace.log
- ArcanaAI_PlayerBit17WriteTrace.log
- ArcanaAI_ProducerCallTrace.log
- ArcanaAI_QueueTrace.log
- ArcanaAI_EventDispatchTrace.log
- ArcanaAI_FinalCommandTrace.log when available

Close the game normally so buffered logs flush.

## Analysis targets

1. Confirm mapResult 0 for the known P1 rawCode 0x20 → stateIndex 0x18 case.
2. Confirm mapResult 1 for the known P2 rawCode 0x5C → stateIndex 0x19 case.
3. Compare the captured WORD flags to the helper's exact priority order.
4. Correlate STATE_MAP_CALL and BIT113_GATE_RESULT using player pointer, raw code/state index and QPC.
5. Look for a naturally reachable valid row containing 0xFF and an actual Stage18 resultAL=0.
6. Compare the runtime table byte readback with the PE's initial 0xFA/0xFF values, without assuming the tables cannot change after load.

No semantic names are assigned to raw codes, WORD flags, table selectors or bytes without evidence.
