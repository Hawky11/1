# Stage23 runtime analysis — second r2 run (2026-11-09 09:04–09:07)

## Scope check: this is a repeat Stage23 r2 run, not Stage24

The uploaded runtime set contains `STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r2`, Stage23 installation at `0x000EE273 -> 0x0012EAC0`, and the existing Stage21/22 logs. The queue/dispatch log header says `Stage 12/21/22/23 tracing requested`. There is no Stage24 build marker, no hook installation at CALL RVA `0x000EE306`, and no Stage24 resolver-consumer result log in this upload.

The build and runtime evidence below therefore belongs to a second Stage23 r2 run. It does not verify the Stage24 consumer observer proposed in the previous handoff.

## Build

User-supplied Build.log started 2026-11-09 09:04:32,60. Visual Studio 18 Community / MSVC 14.51.36231 x86 initialized successfully. Project checks and compiler/linker path checks passed. All eight C++ translation units compiled: Proxy.cpp, ComVsCom.cpp, AITrace.cpp, AICommandTrace.cpp, AIQueueTrace.cpp, AICommandSiteTrace.cpp, AIProducerContextTrace.cpp and AIParentFlowTrace.cpp.

The linker produced `build/dinput8.dll` at **229,376 bytes**. The only diagnostic is the known non-fatal MSVC C4754 warning at `AICommandSiteTrace.cpp:785` (related bounds calculation begins at line 782). No compile or link errors.

## Stage23 runtime installation

Runtime startup: 2026-11-09 09:06:20. The r2 marker is present. The Stage23 resolver-result observer installed at CALL RVA `0x000EE273` -> helper RVA `0x0012EAC0`, reporting 39 suspended threads.

The executable image base in this run is `0x005B0000`; RVA-based hook locations remain consistent.

## Three resolver records and downstream correlation

All three events are type 1 / payload `0x1E`, thread 16252, with readable 12-byte resolver records. Byte +3 is again decimal 31 (`0x1F`) for both players.

| Side | Tick | ALT_CALLSITE QPC | Resolver QPC | Δ | Resolver args (arg1,arg2,arg3,arg4) | Record pointer |
|---|---:|---:|---:|---:|---|---|
| P2 | 1331046 | 13310812900 | 13310813095 | 195 | `(0x17,0x17,0x1E,0x38)` | `0x05C8DCA0` |
| P1 | 1332312 | 13323424185 | 13323424234 | 49 | `(0x0A,0x0A,0x1E,0x26)` | `0x0586F490` |
| P1 | 1350156 | 13501860518 | 13501860595 | 77 | `(0x0A,0x0A,0x1E,0x26)` | `0x0586F490` |

Each returned the same twelve bytes as the prior Stage23 run:
`60 FF 03 1F 12 00 00 05 13 24 03 00`
(DWORDs `0x1F03FF60, 0x05000012, 0x00032413`; byte +2=`0x03`, +3=`0x1F`, +4=`0x12`, +0x0B=`0x00`).

All three match the Stage13 ALT_CALLSITE call at RVA `0xF6AD7`, accepted QueueTrace insertion at return RVA `0xF6ADC`, and EventDispatch at caller RVA `0xEE5CB`. Queue history matches the dispatch arguments. The same event identity is corroborated by side, player pointer, thread, tick and QPC ordering.

| Side | Resolver QPC | Classifier QPC | Fallback QPC | Post-AND QPC |
|---|---:|---:|---:|---:|
| P2 | 13310813095 | 13310818324 | 13310820264 | 13310820882 |
| P1 | 13323424234 | 13323426193 | 13323426682 | 13323427028 |
| P1 | 13501860595 | 13501862845 | 13501863381 | 13501863736 |

For all three: classifier helper `0xE8CF0` returned AL=0; fallback helper `0xF7EB0` ran with index 3; Stage22 observer after the original AND ran. Slot 3 was readable and `0 -> 0` before/after all three fallbacks. Controller flags before the AND were zero, and Stage22 read bit `0x400` as `0 -> 0` in all three records. Thus this repeated run confirms the path executes but still does not capture a nonzero-to-zero slot or controller-bit mutation.

The Stage21 `preCallEdi` values remain the controller-array pointers (`0x0E2806DC` for P2 and `0x0E2559B4` for P1), not resolver byte +3.

## New Stage18 → Stage20 → Stage17 correlation

Stage18 logged 8 gate-result records and Stage20 logged 8 object-guard records, all for P2 in this run. Sequentially pairing the records by side, player, thread, tick, stack/parent return and QPC gives **8/8 matches**. The object-guard result follows its gate result by 121–175 QPC ticks.

- All 8 Stage18 gate results returned `AL=1`, key `0x19`, table byte `0xFA`.
- The first 6 Stage20 object-helper results returned `AL=1`; checked object DWORD was `0x4C`.
- The last two Stage20 object-helper results returned `AL=0`; checked object DWORD was `0x58`, whose bit 2 is clear. Both occurred at tick `1353484`.
- The two AL=0 guards have separate parent paths: first parent return `0x00646801`, second `0x0067B9FA`.

Stage17 logged exactly two setter calls for the same P2 player `0x0E27CCA0`, both at CALL RVA `0x000C6502`, passing index `0x71`. They match the last two Stage20 guard entries by tick, player, EBP/stack frame and parent path:
- Guard #7 QPC `0x326C2BD8E` → setter #1 QPC `0x326C2BDD6`: **72 ticks**. Both pre-setter snapshots show `player+0x3B84 = 0`.
- Guard #8 QPC `0x326C2C03B` → setter #2 QPC `0x326C2C081`: **70 ticks**. Both snapshots show `player+0x3B84 = 0x00020000`.

The field was therefore zero at the first AL=0 object guard/setter entry and already `0x20000` by the second AL=0 object guard/setter entry. The first setter call lies between those observations. This is strong temporal/stack attribution consistent with the first call to setter helper `0xFAA0` setting bit17; however the exact store instruction is not independently captured in this run. `ArcanaAI_PlayerBit17WriteTrace.log` is zero bytes, so do not claim a directly observed 0→1 memory write from this log alone.

## Missing traces and warnings

- `ArcanaAI_PlayerBit17WriteTrace.log`: 0 bytes.
- `ArcanaAI_StateMapTrace.log`: 0 bytes.
- `ArcanaAI_ProducerCallTrace.log`: 0 bytes.
- `ArcanaAI_FinalCommandTrace.log`: 45 records, but none carries payload `0x1E` at its monitored callsite. This does not contradict the separately observed alternate queue/dispatch route at `0xF6AD7`.
- No ERROR, WARNING, DROP, OVERFLOW or FAILED markers were found in the nonempty uploaded runtime logs.

## Conclusion and next action

Stage23 r2 built successfully (DLL 229,376 bytes), installed its resolver observer, and reproduced three readable resolver results with byte +3=`0x1F`. All three payload-`0x1E` events were accepted by the queue, dispatched, classified with AL=0, passed through fallback index 3, and reached the post-AND observer. Slot 3 and bit `0x400` remained zero before/after.

This is an additional Stage23 r2 capture, not a Stage24 run. Preserve it separately from the earlier Stage23 run.

The next source task remains Stage24: a passive observer at CALL RVA `0x000EE306` -> helper `0x000F57D0`. Capture the four arguments, three runtime mapping bytes at `0x04DDF188 + selector*3`, and the three DWORDs at `player+0x3A08` before/after the helper. Preserve original args, returned registers/EFLAGS and caller cleanup; do not write to the mapping table or player structure. Create Stage24 from the unchanged Stage23 r2 source branch and add a Stage24 marker/log so its build and runtime can be distinguished unambiguously.
