# Stage37 runtime evidence

## Stage37 runtime verified — 2026-11-10

Upload: `0a87d4c5e93b5115e9288cbde78acf491f18bf4f` (07:10:17 Moscow), evidence branch: [stage37-type1-runtime-evidence](https://github.com/Hawky11/1/tree/stage37-type1-runtime-evidence). Full report: [Stage37_Runtime/RUNTIME_ANALYSIS.md](https://github.com/Hawky11/1/blob/stage37-type1-runtime-evidence/Stage37_Runtime/RUNTIME_ANALYSIS.md).

- Windows x86 build at 07:07:57: BUILD SUCCEEDED, DLL 268288 bytes; only old C4754 warning. Stage37 r1 marker and ENABLED present, EE200 installed; Stage36 r2 3/3 installed with STAGE10_OWN_DLL.
- 173 consecutive EventDispatch records, 52 type1 and 121 type0. All 52 type1 have exact Stage37 join on dispatchSeq, side, player, tid, payload and both QPC stamps. P1=22/P2=30. All 30 fields readable before/after (3FFFFFFF), all changed masks recomputed correctly; every row changes state.
- O+278 increments by one in 52/52; P+3A08 and P+3A0C change 51/52, P+3A10 23/52, P+3F64 51/52, P+3B70 49/52. O+184 changes only 3/52 (0->1E); do NOT label it animation ID.
- 478 consecutive Stage36 CANDIDATE records: 472 FFFFFFFF, 6 positive IDs 6,12,82,20,42,89. All six uniquely match nested ParentFlow F4360, subsequent same P/TID/payload F7C62 queue and EE200/Stage37. INPUT_1E/1F remain unobserved. ID42/dispatch164 changes only P+3F64 and O+278; unchanged bitmap is not a lost command.
- Stage35 10812 consecutive full-mask rows, 5406 TIMER/FLAGS pairs. No explicit runtime ERROR/WARNING/dropped/overflow in successfully retrieved logs. ProducerCallTrace could not be retrieved (transport closed); its exact blob is archived unchanged, do not claim it was empty or checked. All other 24 uploaded files read, nonempty and newline terminated; no malformed Stage37 rows or sequence gaps in checked streams. Tail flush completeness cannot be proved solely from logs.
- Next Stage38: trace readers of changed object lifecycle fields (especially O+278 and player P+3F64) to a precise native per-frame action/animation consumer using original SHA-pinned AALib.exe already in Library; no need to request EXE. This proves controller/object state changes, not visible attacks/hits or an AI bug. No new DLL needed for this completed review. Preserve Stage37 source and all older branches, keep Stage29 AITrace.cpp.

## Field changes across 52 dispatches

|Index|Field|Changed rows|
|---|---|---|
|0|P+3978|0|
|1|P+3A08|51|
|2|P+3A0C|51|
|3|P+3A10|23|
|4|P+3A6C|0|
|5|P+3B70|49|
|6|P+3B74|0|
|7|P+3B78|0|
|8|P+3B7C|0|
|9|P+3B80|0|
|10|P+3B84|4|
|11|P+3B88|0|
|12|P+3F20|0|
|13|P+3F30|0|
|14|P+3F34|0|
|15|P+3F50|0|
|16|P+3F64|51|
|17|P+3F84|0|
|18|P+04 (object pointer)|0|
|19|O+08|1|
|20|O+0C|0|
|21|O+10|0|
|22|O+18|0|
|23|O+60|0|
|24|O+184|3|
|25|O+278|52|
|26|O+27C|0|
|27|O+280|0|
|28|O+2A8|0|
|29|P+3F78|0|

## Exact candidate joins

Each match requires same player, side, TID and payload; parent QPC inside candidate interval, queue and dispatch within 10000 QPC ticks, exactly one match at each step. Actual queue delays 13–28 ticks; dispatch delays 16–195 ticks. Full rows in candidate_matches.json. All six queue callerRVA values are F7C62. Other 46 type1 events must not be attributed to these six F4360 results.

## Reproducibility and archive

Original 25 changed files are preserved by copying original Git blob SHA references, including ProducerCallTrace (unread due transport limit). Sources: Stage37_Overlay/src/AIQueueTrace.cpp on stage37-type1-controller-object-commit-trace. O=[P+4]; object pointer is unchanged in all 52 rows. Field statistics count net pre/post differences only; transient writes can be missed. No runtime modification or forced gameplay input performed.
