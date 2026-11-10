# Stage46 — definition-driven motion-state initialization

2026-11-10. SOURCE ONLY. Windows x86 build and game runtime pending.

Original AALib.exe: 175556608 bytes, SHA256
`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
Existing Library-derived executable reused; no binary redistribution.
Stage45 runtime evidence: 3fdb8647fae45cc517281c7f74ffaa4a9789c203.

## Verified native consumer

The same native binding function 1E8490 continues after storing the resolver
result at O+34. At 1E8A1F it compares signed O+70 with O+6C. The dispatch runs
only if O+70 < O+6C and O+08 bit9 is clear. At 1E8A3F it reads byte
`definition+30`, subtracts 1 and dispatches cases 1..9 through table 1E8D30.

| Byte +30 | Target RVA | Native effect in this branch |
|---|---|---|
| 1 | 1E8A54 | Zero float fields O+88/+8C/+90/+94 |
| 2 | 1E8A71 | Conditional changes to O+7C/+80 using signed definition +32/+36, object flags and helper predicates; zero four fields afterwards |
| 3 | 1E8B6D | Convert signed definition +32/+34/+36/+38 to floats divided by 100 into O+88/+90/+8C/+94; conditional sign inversions |
| 4 | 1E8C1C | Set O+90/+94 from signed definition +34/+38 divided by 100; conditional sign inversions |
| 5 | 1E8C84 | Zero O+88/+8C/+90/+94 and O+150/+154 |
| 6 | 1E8C9C | Zero O+150/+154 |
| 7 | 1E8CAA | Call RVA FA00, with bit index 0F; no float assignment in this branch |
| 8 | 1E8CB7 | Call RVA FA40 with bit index 0F; no float assignment in this branch |
| 9 | 1E8CC4 | Set O+94 from signed definition +38 /100 |

Constants at preferred VA 6C6AC8 and 6C7D28 are 100.0 and -1.0.
The shared tail may clamp O+80 to zero, depending on O+08 bit13 and its value.
At 1E8D06..1E8D0E O+70 advances to O+6C if it is smaller.
These are concrete native float-state operations, consistent with motion
initialization. Rendered movement, animation frames, hitboxes, damage and AI
quality are not established by these addresses alone. Do not label the separate
definition+2E helper chain as animation without tracing its downstream semantics.

## Observation boundary

No new hook or assembly wrapper. Extend two existing files:

- Stage45 leave callback, immediately after original resolver returned EAX,
  calls S46Resolved while the matching Stage44 TLS record is active. It records
  the new definition's data and current object fields **before native 1E8490
  resumes and stores EAX to O+34**. Native GPR/flags/FPU/SSE are already saved.
- Stage44 leave callback records the same object fields after 1E8490 returned.
  Stage44's existing ring stores both snapshots and emits the new Stage46 log
  alongside its original record, retaining stage44Seq and parent QPC.

Fields f0..f12 map to object offsets:
60, 6C, 70, 08, 24, 7C, 80, 88, 8C, 90, 94, 150, 154.
f5..f12 are logged as raw IEEE754 bits; decode to floats for analysis.
Metadata DWORDs at +20/+30/+34/+38 preserve the command byte and packed signed
WORD parameters. Expected full object masks are 1FFF/1FFF and metadata mask F.
`definitionAfter` is Stage44 POST O+34; join it with `definition` and Stage45 EAX.
`resolverCount` must be 1 for uncomplicated records. Zero is explicitly retained
as missing resolver observation; repeated calls are counted, first snapshot kept.

This is an enclosing function window, not an instruction-level branch trace.
Other operations before/after the switch may affect the same fields. Therefore
an endpoint mismatch must be investigated using guards/helper calls; do not call
it corruption or infer that a skipped switch proves lost commands.

## Integrity and testing

Stage44/45 assembly wrappers are byte-for-text unchanged from working sources;
no extra MakeRelativeCall, worker, queue, INI option or game-state write added.
Existing Stage44 ring drop/overflow/mismatch counters govern Stage46 records;
also inspect Stage45 counters. Added per-record storage enlarges the DLL.
One extra file append per Stage44 record increases diagnostic I/O; this is not
a performance benchmark. New log: ArcanaAI_Stage46DefinitionMotionTrace.log.
Marker STAGE46_DEFINITION_MOTION_BUILD=2026-11-10-r1 and STAGE46_ENABLED appear
only after successful Stage45 install.

verify_stage46.py checks original SHA, native guard/dispatch/tail bytes, all nine
table targets, constants and unchanged wrapper assembly. 64 original-fragment
Unicorn cases pass, exercising dispatch/skip guards, modes 0/1/3/4/5/6/9/10,
sign handling and O+70 advancement. The bit-query helper is stubbed false;
modes 2/7/8, whole-function behavior and runtime are NOT validated by this test.

## Next analyst

Require successful Windows x86 build, Stage44/45 installation and Stage46 marker.
Join Stage46 stage44Seq/P/O/TID/bindingQpcBefore/qpcAfter to Stage44, and
definition to Stage45 EAX via same parent QPC. Inspect resolverCount, masks,
definitionAfter, metadata mode distribution, guards, float changes and shared
health. Correlate nested Stage42 and outside-Stage42 bindings separately.
No claim of actual animation/hit/damage yet. Original EXE already in Library.
