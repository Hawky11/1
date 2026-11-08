# Stage23 — type-1 resolver-result trace

Stage23 follows the corrected Stage22 source and captures the result returned by the type-1 mapping helper before downstream code reuses EDI.

- Hook site: `AALib.exe+0x000EE273`
- Original helper: `AALib.exe+0x0012EAC0`
- Exact executable: SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`
- Marker: `STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r1`
- Log: `ArcanaAI_Stage23ResolverResultTrace.log`

The observer filters to P1/P2, dispatcher type 1 and payload `0x1E`. It records the resolver's four original stack arguments, returned 12-byte record, and bytes `+2`, `+3`, `+4`, `+0x0B`. It does not call the resolver more than once, change its arguments, or write game data.

## Important dataflow correction

The old Stage21 static note incorrectly treated `EDI` at `0xEE3E1` as the resolver's byte `+3`. In the actual binary, byte `+3` is read at `0xEE28D`, used by the next mapper call, and then EDI is overwritten at `0xEE30B` with the dispatcher event type. Thus `CMP EDI,1` at `0xEE3E1` checks the dispatcher event type. Later `LEA EDI,[ESI+0x3A3C]` at `0xEE3FC` sets the controller-array pointer; this is what Stage21 r2 observed at the later classifier callsite.

See `STATIC_RESOLVER_RESULT_ANALYSIS.md` for the exact disassembly and stack/register contract.

## Apply

This is a full replacement `AIQueueTrace.cpp` for the assembled Stage22 source tree:

1. Replace project `src/AIQueueTrace.cpp` with `Stage23_Overlay/src/AIQueueTrace.cpp`.
2. Replace project `ArcanaAI.ini` with `Stage23_Overlay/ArcanaAI.ini`.
3. Build `Release | Win32`; keep the complete `Build.log`.
4. Only install the new DLL after a successful build. Keep a copy of the prior working DLL and INI.
5. Rename/move old runtime logs before testing, run a normal COM-vs-COM bout, then close the game normally.

## Upload

First upload:
- `Build.log`
- `ArcanaAI_Stage23ResolverResultTrace.log`
- `ArcanaAI_Stage21PayloadConsumerTrace.log`
- `ArcanaAI_Stage22PayloadPostAndTrace.log`
- `ArcanaAI_EventDispatchTrace.log`
- `ArcanaAI_QueueTrace.log`

Keep same-run Stage18 gate, Stage20 guard, Stage17 setter, Stage16 bit17 writer and Stage19 state-map logs when available.

## Status

Source-only: this overlay has not been compiled or runtime-tested. Confirm the Stage23 marker and successful hook installation line before interpreting its new records.
