# Stage32 — runtime correlation of native type0/payload0 with EE200

**Session:** 2026-11-09, 22:19:32–22:20:16 (timestamps in logs). **Result:** exact QPC correlation confirms the native Stage31 alternate path sends its type0/payload0 event to the existing game dispatcher EE200 in all three observed alternate episodes. The original event is not silently lost between F2DE0 and EE200 in this run.

## Evidence / integrity

- Immutable Stage32 source branch: [stage32-type0-dispatch-correlation](https://github.com/Hawky11/1/tree/stage32-type0-dispatch-correlation) @ `44b23d1634139fe15928b1ff50bbddda056f633d`, with existing Stage31 and Stage29 observers retained.
- All **21** original .log Git blobs from user's upload [`main` @ `a7a99cfb`](https://github.com/Hawky11/1/commit/a7a99cfb5d7fca28f5ae8230c350a2b8183de07b) copied exactly as blob references under `Stage32_Runtime/`. They were **not** re-encoded or filtered.
- Main traces: EventDispatch `af3ed64d6ab0b8300033e9a1971fcd55340fd001` (137,783 bytes), Stage31 `0b2c662ef3e34cd6e3e11aa35317624bd87a01e8` (21,545 bytes), Stage29 `d192beb9c453651041c6a8ae84cb13fc300b0202` (15,116 bytes), ProducerCall `6b70130053520f0030d02e502e9da0cb7739fc57` (9,921,718 bytes).
- Stage32 marker `STAGE32_TYPE0_DISPATCH_BUILD=2026-11-09-r1`; type0_payload0=1; existing `EE200` dispatch hook installed. Stage31 hook `F6A8C→130A10` installed; Stage29 DR0/DR1 active; F2DE0 queue hook installed.
- No `Build.log` included in user upload: running Stage32 DLL confirmed, but native Windows MSVC compiler/linker transcript is **not independently checked**.
- Stage31 event seq 1..35 contiguous: 3 BEGIN + 3 END + 29 TYPE0_QUEUE, including 26 outside scopes. Stage32 dispatcher detail seq 1..131 contiguous: 77 type0/payload0 and 54 type1. Stage29 transitions seq 1..8 contiguous (4 sets, 4 clears). No explicit WARN/ERROR/positive drop/overflow in the main Stage31/32/29/queue/producer logs.

## Three complete episodes

All nested events share side, player, TID **3748**, and episode; all 3 Stage31 entry/end masks are `FFFFFFFF`. Stage31 queue return address `156E4C` has type0/payload0 and a readable queue snapshot `(guard,active,storedType,storedPayload): (0,0,65535,65535)→(0,0,0,0)`. At subsequent EE200 entry the queue snapshot is `active=1 current=(FFFF,FFFF) history=(0000,0000) guard=0`.

| episode | Side | Stage31 seq BEGIN/QUEUE/END | Stage32 EE200 seq | END→EE200 QPC ticks | EE200 entry→bit17 clear | bit17 clear→EE200 exit |
|---:|---|---|---:|---:|---:|---:|
| 1 | P1 | 7 / 8 / 9 | 43 | **11** | 698 | 327 |
| 2 | P2 | 15 / 16 / 17 | 60 | **11** | 715 | 338 |
| 3 | P2 | 30 / 31 / 32 | 120 | **9** | 522 | 247 |

QPC stage markers:

- Ep1: Stage29 set 489259875424; Stage31 BEGIN 489259878976, nested type0 return 489259879315, END 489259879534; Stage32 EE200 begins **489259879545**, Stage29 clear **489259880243**, EE200 returns **489259880570**. Stage15 F6A68 bit17=1, branchTaken=0.
- Ep2: Stage29 set 489290410219; BEGIN 489290413616, type0 return 489290413852, END 489290414051; EE200 begins **489290414062**, Stage29 clear **489290414777**, EE200 returns **489290415115**. Stage15 bit17=1 branchTaken=0.
- Ep3: Stage29 set 489408963456; BEGIN 489408966128, type0 return 489408966321, END 489408966531; EE200 begins **489408966540**, Stage29 clear **489408967062**, EE200 returns **489408967309**. Stage15 bit17=1 branchTaken=0.

Each Stage31 episode starts with `[player+3B84]=0x00020000`, ends with `0x00030000` (bit16 set while bit17 remains), then Stage29 clear `0x00030000→0x00010000` **inside that same EE200 dispatch call** (same TID/side). This nested temporal relation provides independent causal support for the matching.

## All 21 type0/payload0 queue events

Stage31 logged **21** per-event queue calls type0/payload0 (3 scoped + 18 unscoped). Each has one unique subsequent `EVENT_DISPATCH_DETAIL` with the same P1/P2 and TID=3748; delay from `qpcAfter` of F2DE0 to `qpcBefore` of EE200: **7..230 QPC ticks**. For all 18 unscoped calls: **7..70 ticks**; for scoped calls 210..230 ticks (because Stage31 END runs first); for scoped END to dispatch: 9..11 ticks. See [complete 21-row correlation](stage31_to_stage32_type0_dispatch.csv).

This is exceptionally strong same-event correlation, **not** a persistent unique parent ID in the EE200 hook. If another uninstrumented queue write intervened (or after crash/abnormal exit), the data could be incomplete. Do not overstate direct-action success.

## Fourth bit17 cycle is separate, not a dropped Stage31 episode

- P2 Stage29 set `seq5` QPC **489350287522** `0→0x20000`, later clear `seq6` QPC **489351991807** `0x20000→0`. Bit16 was not set in the clear shadow.
- No `F6A68` Stage15 branch probe and no Stage31 BEGIN occur during that particular bit17 interval. ProducerCall captured **4** F6A68 attempts in total: 3 alternate bit17=1 attempts tied to the three episodes above and one direct P1 bit17=0 attempt.
- Nevertheless, unscoped Stage31 type0/payload0 queue `seq23` ends QPC **489351991105**; dispatcher detail `seq87` P2 begins **489351991115**, ends **489351992252**; this independent bit17 clear `489351991807` is **inside EE200**. No invented `episode` association; Stage31 records `episode=0` correctly here.
- Direct control route: P1 F6A68 bit17=0 branchTaken=1 at QPC **489319650398**, Stage13 F6AD7 call at **489319651078**, writes `type1/payload0x1E` and passes to dispatcher; Stage21 classifier returns AL0. This matches prior findings.

## Conclusion and next research

**Stage32 completed for observed runtime:** alternate path through `130A10 → F2DE0(type0/payload0)` leads to `EE200` with active1 (3/3 scoped cases). Each same-event bit17 clear happened inside the dispatch window. All 21 type0/payload0 queue records have close matching dispatch records. This closes the queue-to-dispatch visibility gap.

It does **not** prove the specific gameplay action (attack/movement/animation) was executed successfully or that all cases behave identically. The type1/0x1E route remains distinct; there is no evidence here requiring bit manipulation, event duplication or behavior-changing patches.

**Stage33 handoff:** inspect the precise type0/payload0 branch **inside `EE200`** and its downstream consumers/effect writes with read-only original `AALib.exe` (already at ChatGPT Library `/AALib.zip`, SHA pinned in `BINARY_SOURCE_IN_LIBRARY.md`), then if necessary design a minimal passive hook for final effect rather than altering the event. Preserve source/evidence branches.
