# Stage23 runtime analysis — resolver-result observer (2026-11-09)

## Build and runtime installation

The user supplied Build.log from 2026-11-09 00:39:38.25. Visual Studio 18 Community / MSVC 14.51.36231 x86 initialized successfully. All eight C++ translation units compiled: Proxy.cpp, ComVsCom.cpp, AITrace.cpp, AICommandTrace.cpp, AIQueueTrace.cpp, AICommandSiteTrace.cpp, AIProducerContextTrace.cpp and AIParentFlowTrace.cpp.

The linker produced build/dinput8.dll at 229,376 bytes. The only compiler diagnostic was the existing non-fatal C4754 warning in AICommandSiteTrace.cpp:785, associated with the stack-address bounds calculation around line 782. No compile or link errors occurred.

Runtime start: 2026-11-09 00:40:04. The Stage23 r2 marker is present: STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r2.

The observer installed successfully at dispatcher CALL RVA 0x000EE273, calling resolver helper RVA 0x0012EAC0. Installation reported 40 suspended threads. The log confirms the original four cdecl arguments were replayed once and result registers/EFLAGS plus caller-owned stack were preserved.

## Resolver records

Three type-1 / payload-0x1E records were captured, all on thread 11360:

| Side | Tick | Resolver QPC | Resolver result pointer | Resolver args (arg1,arg2,arg3,arg4) |
|---|---:|---:|---|---|
| P2 | 823151515 | 8231589249280 | 0x063FDCA0 | 0x17, 0x17, 0x1E, 0x38 |
| P1 | 823158406 | 8231658248588 | 0x05FDF490 | 0x0A, 0x0A, 0x1E, 0x26 |
| P1 | 823166531 | 8231739431884 | 0x05FDF490 | 0x0A, 0x0A, 0x1E, 0x26 |

Every resolver record was readable. All three returned the same raw 12-byte content: 60 FF 03 1F 12 00 00 05 13 24 03 00.

The three little-endian DWORDs were (0x1F03FF60, 0x05000012, 0x00032413). Selected fields were identical in all samples: byte +2 = 0x03 (3), byte +3 = 0x1F (31), byte +4 = 0x12 (18), and byte +0x0B = 0x00.

The P2 resolver arguments agree with captured player state fields: arg1 and arg2 were 0x17, player+0x3978 and player+0x3F20 were both 0x17, and arg4 / player+0x1224 was 0x38. For P1, arg1 and arg2 were 0x0A, player+0x3978 and player+0x3F20 were both 0x0A, and arg4 / player+0x1224 was 0x26. Arg3 was the original payload 0x1E for all three.

The record pointer differs between P1 and P2, but the 12 data bytes match. P1's pointer was reused for its two events. Do not call byte +3 an attack ID or command ID based on these samples alone.

## One-to-one event correlation: producer → resolver → classifier → fallback → post-AND

All three Stage23 records correlate to Stage13 ALT_CALLSITE, QueueTrace, EventDispatchTrace, Stage21 classifier/fallback and Stage22 post-AND records by side, player, thread 11360, tick and QPC order.

| Side | ALT_CALLSITE QPC | Resolver QPC | Δ ALT→resolver | Classifier QPC | Δ resolver→classifier | Fallback QPC | Post-AND QPC |
|---|---:|---:|---:|---:|---:|---:|---:|
| P2 | 8231589249194 | 8231589249280 | 86 | 8231589253347 | 4067 | 8231589256114 | 8231589257163 |
| P1 | 8231658248530 | 8231658248588 | 58 | 8231658251710 | 3122 | 8231658252447 | 8231658252837 |
| P1 | 8231739431782 | 8231739431884 | 102 | 8231739434850 | 2966 | 8231739435766 | 8231739436176 |

For all three records:
- ALT_CALLSITE is the call at RVA 0x000F6AD7 with type 1 / payload 0x1E.
- QueueTrace records ACCEPTED_BY_QUEUE at return RVA 0x000F6ADC and stores type 0x0001 / payload 0x001E.
- EventDispatchTrace records the same type/payload at caller RVA 0x000EE5CB; queue history matches the arguments.
- Stage23 observes the resolver return at RVA 0x000EE273.
- Stage21 classifier helper 0x000E8CF0 returns AL=0.
- Stage21 fallback helper 0x000F7EB0 runs with index 3.
- Stage22 post-AND observer at RVA 0x000EE451 runs afterward.

Stage21's preCallEdi field continues to equal the controller-array pointer, not the resolver byte. It must remain a raw register value.

## Fallback state did not change in these three events

For all three Stage21 fallback records, slot 3 was readable and was 0 before and after the helper. Controller flags immediately before the AND were 0.

All three Stage22 post-AND records were readable and showed controller bit 0x400 as 0 before and 0 after. The original AND observer ran, but no nonzero-to-zero transition was captured.

Thus this run again proves execution of the fallback and AND path, not an actual slot-3 clear or controller-bit-0x400 clear.

## Independent Stage18 → Stage20 → Stage17 chain in this run

Stage18 contains 27 gate-result records and Stage20 contains 27 object-guard records. Pairing by sequence, side, player, thread, tick and parent return gives 27 one-to-one matches; the object-guard QPC follows its paired gate QPC by 124–204 ticks.

All 27 Stage18 gate calls returned AL=1, key 0x19 and table byte 0xFA. Stage20's object helper returned AL=1 in 25 records. Two P1 records at tick 823152734 returned AL=0 with object DWORD 0x00000058 and bit2 clear.

Stage17 then recorded two P1 direct setter calls for the same tick, both at CALL RVA 0x000C6502 with index 0x71. Their QPC values follow the two AL=0 object-guard calls by 76 and 73 ticks respectively. The first setter snapshot saw player+0x3B84 = 0; the second saw 0x00020000. This is consistent with bit17 becoming set between the two setter calls, but the exact write is not confirmed in this run because ArcanaAI_PlayerBit17WriteTrace.log is zero bytes.

ArcanaAI_StateMapTrace.log is also zero bytes, so this run does not add a same-event raw-code/state-map mapping.

## Static interpretation of resolver byte +3

The Stage23 runtime samples establish byte +3 = 0x1F for payload 0x1E in both P1 and P2 states. Reinspection of the exact AALib.exe disassembly shows this byte is then passed as argument 2 to helper 0x004F57D0 at CALL RVA 0x000EE306.

At that callsite the argument layout is:
- arg1 = pointer to player+0x3A08;
- arg2 = resolver record byte +3 (0x1F in this run);
- arg3 = resolver record byte +2 (0x03);
- arg4 = resolver record byte +4 (0x12).

Inside helper 0x004F57D0, ESI is loaded from [EBP+0x0C], i.e. arg2. The helper indexes three consecutive bytes at base 0x04DDF188 + ESI*3: the flag byte at +0, and two mapped selector bytes at +1 and +2. It stores ESI at the output structure's first DWORD and uses the two mapped bytes to set bits in the DWORDs at output offsets +4 and +8; flag bits can alter initialization and trigger an additional helper call.

Therefore byte +3 = 0x1F is demonstrably a selector/index consumed by a three-byte per-index mapping table in this downstream helper. Its higher-level semantic name (attack, action, command, animation, or other category) is not established. The table is in writable .data and may be initialized at runtime; static on-disk bytes must not be assumed to equal runtime table contents.

## Missing or empty traces

- ArcanaAI_PlayerBit17WriteTrace.log: 0 bytes; no exact bit17 store instruction recorded.
- ArcanaAI_StateMapTrace.log: 0 bytes; no raw state-map trace.
- ArcanaAI_ProducerCallTrace.log: 0 bytes.
- ArcanaAI_FinalCommandTrace.log has no payload-0x1E record at its monitored callsite RVA 0x000F7C5D. This does not contradict the separate confirmed 0x000F6AD7 queue insertion and dispatch path.
- No ERROR, WARNING, DROP, OVERFLOW or FAILED markers were found in the nonempty uploaded runtime logs.

## Conclusion and next stage

Stage23 r2 compiled successfully, linked a 229,376-byte DLL, and its resolver-result observer installed and produced three readable runtime records. All three samples return the same 12-byte resolver data, including byte +3 = 0x1F, and are fully correlated through queue acceptance, dispatch, classifier AL=0, fallback, and post-AND.

The next useful diagnostic is a narrowly scoped observer at CALL RVA 0x000EE306 -> helper 0x004F57D0. It should capture:
1. The four original helper arguments, especially selector 0x1F.
2. The three runtime bytes at 0x04DDF188 + selector*3.
3. The player+0x3A08 structure (three DWORDs) before and after the original helper call.
4. Any flag-controlled secondary call and the final output structure.

Preserve Stage23 source and runtime evidence branches unchanged. Treat the proposed consumer observer as Stage24 source work; do not force any game values or write to the table/player structure.
