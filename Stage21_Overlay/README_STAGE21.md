# Stage21 — payload 0x1E consumer trace

Stage21 follows the Stage20 source branch `stage20-bit113-object-bit2-guard-trace` and adds a diagnostic-only trace for the event consumer reached after dispatch of event type 1 / payload `0x1E`.

## What changed

- `src/AIQueueTrace.cpp`: replacement source for the assembled project. Adds two validated CALL-site wrappers inside `AALib.exe+0xEE200`:
  - `0xEE426 -> 0x4E8CF0`: records the original helper's real EAX/AL.
  - `0xEE446 -> 0x4F7EB0`: records the control-array slot-3 value before and after the original helper clears it.
- `ArcanaAI.ini`: enables `EnableStage21PayloadConsumerTrace=1`.
- `STATIC_PAYLOAD_1E_CONSUMER_ANALYSIS.md`: static control-flow and data-effect analysis.

New runtime output: `ArcanaAI_Stage21PayloadConsumerTrace.log`.

## Apply to the assembled Stage20 source

1. Start with the complete Stage20 source package/project. Keep Stage20's `src/AITrace.cpp`, Stage12–20 source additions, project settings, and other existing files unchanged.
2. Replace the assembled project's `src/AIQueueTrace.cpp` with `Stage21_Overlay/src/AIQueueTrace.cpp`.
3. Replace the assembled `ArcanaAI.ini` with `Stage21_Overlay/ArcanaAI.ini`.
4. Build `Release | Win32` using the project's existing `Build.bat` or Visual Studio solution.
5. Check the build output, then launch a natural COM-vs-COM session. Do not use Cheat Engine to force the helper result or change the object bit.

The hook validates the known PE fingerprint and exact CALL-site bytes/destinations before patching. It uses the existing shared patch-install mutex. If either callsite is different, Stage21 refuses the install rather than patching an unknown instruction sequence.

## Expected verification

The log should contain:

- `STAGE21_PAYLOAD_1E_CONSUMER_BUILD=2026-11-08-r1`.
- An installation line naming both callsite wrappers.
- `STAGE21_CLASSIFIER ... eventType=1 payload=0x0000001E ... resultAL=0`.
- `STAGE21_FALLBACK_SLOT_CLEAR ... index=3 ... slotValueBefore=... slotValueAfter=0x00000000`.

Correlate those lines with the existing `ArcanaAI_EventDispatchTrace.log` records by side, player, thread ID, and QPC. Send the new Stage21 log, the matching EventDispatch log, and the complete build log.

## Direct-setter negative control remains open

Stage20's real object-guard trace stays enabled in the inherited INI. Keep a copy of `ArcanaAI_Bit113ObjectGuardTrace.log` from the same run. The required sample is a natural object DWORD at `player+0x3B70` with bit2 clear, producing actual helper `AL=0`. Do not manually edit this value. Stage21 does not yet close that separate experiment.

## Scope

Stage21 does not alter any AI helper result or game state beyond the original game helper effects. The only in-game code changes are observational CALL wrappers, and both original callees are invoked once. The code has not yet been compiled or runtime-tested; those checks are pending the user's Windows/MSVC build and fresh in-game run.
