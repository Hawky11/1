# Stage34 — native type0 payload1/4/5 runtime confirmed

**Date and source:** 2026-11-09 COM-vs-COM, in-game trace timestamps 23:47:33–23:48:15. User uploaded 21 fresh logs in [`main` commit `4e03cab8`](https://github.com/Hawky11/1/commit/4e03cab8949a91bc3583ad1eadf7cb20d5c3a14a), plus `Build.log` in [`1039963c`](https://github.com/Hawky11/1/commit/1039963cfb2593000632352acb1dfb68e2641952). All 22 files archived as original Git blobs unchanged. Source-only Stage34 [stage34-type0-variant-state-trace](https://github.com/Hawky11/1/tree/stage34-type0-variant-state-trace) @ `803e3ae15f43370dc0fe7969b0ec6242320124d7`, unchanged.

## Build and hook integrity

- **BUILD SUCCEEDED**, x86 `Build.bat` 09.11.2026 23:46:47, VS18 Community / MSVC 14.51.36231, `vcvarsall x86` exit0, compiler and linker found, linked `dinput8.dll` size **250880 bytes**. **One warning C4754** `AICommandSiteTrace.cpp:785` (references 782), not an error. No other compiler/linker errors found. The text in the original log is mojibake due to codepage.
- Runtime installed the existing `EE200` dispatch hook and Stage31 `F6A8C→130A10`, F2DE0 queue hook, and Stage29 bit17 hardware observer; `STAGE32_TYPE0_DISPATCH_BUILD=2026-11-09-r1`, `STAGE33_TYPE0_CONSUMER_BUILD=2026-11-09-r1`, `STAGE34_TYPE0_VARIANTS_BUILD=2026-11-09-r1` markers present. Stage34 changes only capture logic, **not** native EE200/gameplay.
- **Zero observed seq holes** in Stage32 `EVENT_DISPATCH_DETAIL 1..168`, Stage31 `STAGE31_EVENT 1..35`, Stage29 `STAGE29_BIT17_TRANSITION 1..4`. Stage34 and Stage33 effect seq values share Stage32's global sequence and do **not** need be contiguous. No explicit ERROR, positive dropped or overflow in inspected core logs; uninstrumented threads/buffer termination remain generic limits.
- Stage32 detail: **168 events** = **90 type0/payload0**, **7 type0/payload1**, **12 type0/payload4**, **0 type0/payload5** and **59 type1**. Stage33 effects=90 (payload0, original schema), Stage34 effects=19 (7 payload1 and 12 payload4).
- **All 109/109** effect records match the **same unique Stage32 seq, QPC pre/post, side, TID, player, exact payload**. For each, `preMask=postMask=0xFFF` (all 12 raw DWORDs readable). For all **19 Stage34 variant records**, additional slot `P+3A6C` `slot0BMask=0x3` (successful before+after DWORD reads). No missing/duplicate effect records for recorded type0/0,1,4.

## Stage31 → Stage32 → Stage33/34: **33/33** per-event matches

Stage31 logged **35 events**: 1 BEGIN, 1 END and **33 type0 queue insertions** from callerRVA `156E4C`. Their payload distribution is **0=26; 1=3; 4=4; 5=0**.

All **33/33** `type0` queue returns find a **unique** subsequent `EE200 EVENT_DISPATCH_DETAIL` matching side, TID and payload with delta **6..488 QPC ticks** and a **matching effect record**. Grouped latency:
- payload0: 26 matches, Δ 7..70 ticks
- payload1: 3 matches, Δ 8..488 ticks
- payload4: 4 matches, Δ 6..10 ticks
[Full 33-row reproducible correlation](stage31_to_stage34_variant_pairs.csv) lists every queue+dispatch seq and QPC. These correlations are very strong but not an explicit queue-to-EE200 parent ID; intervening unobserved writes remain a general limitation.

### Stage31 scoped episode1 P2 type0/payload1 — gap closed

All entries are from **P2**, game thread TID=17228, player `0x0E27CCA0`:

| Point | Stage seq / details | QPC |
|---|---|---:|
| Stage29 bit17 set 0→1 | seq3, post RVA 0xFAD2 | 542085812208 |
| Stage31 BEGIN | seq11, bit17=1, 32 DWORD mask `0xFFFFFFFF` | 542085814640 |
| Stage31 type0/payload1 insert return | seq12, native caller 0x156E4C | 542085814844 |
| Stage31 END | seq13, flags 0x30000 | 542085815320 |
| Stage32 EE200 type0/payload1 begin | seq56, `active=1, current=(FFFF,FFFF), history=(0,1)` | 542085815332 |
| Stage29 bit17 clear 1→0 | seq4, writer post RVA 0xED1A9 | 542085815861 |
| Stage32 EE200 return | seq56 | 542085816104 |

**Stage31 END → EE200 entry 12 QPC ticks; bit17 clear is inside the exact EE200 call**, and Stage34 `STAGE34_EFFECT_DETAIL seq=56` has identical QPC/side/TID/player with full masks.

Scoped payload1 net mutations:
- `P+3B70` `00020460→00020420`
- `P+3B84` `00030000→00010000`
- `P+3F64` `00000015→00000001`
- `P+3A08` `00000022→00000001`
- `P+3A0C` `00010002→00000002`

Extra `P+3A6C` slot `00000000>00000000`, read mask `0x3`; no net change for payload1 in this episode. This confirms native state handling and bit17 clearing, **not** a gameplay attack/movement success.

## New payload4 native special slot proof

Stage34 `payload4` generated **12** paired dispatch+effect records. `slot0BMask=0x3` in every record. In **8 of 12** records `P+3A6C` **changed** and every such record ended at **0x00000F00**:

- `seq18`: `00000000>00000F00`
- `seq20`: `00000E88>00000F00`
- `seq80`: `00000000>00000F00`
- `seq82`: `00000E73>00000F00`
- `seq132`: `00000000>00000F00`
- `seq134`: `00000E88>00000F00`
- `seq160`: `00000000>00000F00`
- `seq162`: `00000E73>00000F00`

This agrees with native `EE360 → 991C0` branch and slot11 `P+3A6C` predicted by Stage34 static analysis. Value `0xF00` is 3840 (32×120), but input x=32 must not be inferred as player action semantics. For 4 remaining records slot was already `0xF00` on entry, with no net change. **These observations prove the actual slot write/reset effect at EE200 boundaries, not every internal instruction or visible gameplay result.**

Payload1 has 7 records; controller outputs are id1, `P+3A0C` bit1=2 in changed instances, `P+3F64=1`; slot0B remained `0` in all payload1 records. Payload4 changed examples set `P+3A08=0`, `P+3A0C bit0=1`, `P+3F64=0`. These are consistent with original 158560/158EC0/F57D0 tables from [Stage34 static report](../Stage34_Analysis/STATIC_ANALYSIS.md).

### All net-change counts by variant

A boundary change means at least one monitored DWORD differs before/after, not proof that a move occurred. Field-change counts are per-effect record, not per individual write.

| type0 payload | EE200 with effect | Main 12 fields changed | Extra slot changed | Any monitored field/slot changed |
|---:|---:|---:|---:|---:|
| 0 | 90 | 27 | 0 | 27 |
| 1 | 7 | 3 | 0 | 3 |
| 4 | 12 | 4 | 8 | 8 |
| 5 | 0 | 0 | 0 | 0 |

[All 109 effect snapshots CSV](stage34_all_effect_records.csv) provides raw 12×2 values and slot masks. [Per-payload summary CSV](stage34_variant_summary.csv) provides per-field change counts.

## Out-of-scope Stage29 bit17 cycle and direct-route controls

- Stage29 has **two** set/clear cycles (both P2). First: set QPC542032743936 `0→0x20000`; clear QPC542033143334 `0x20000→0` occurred inside **unscoped** type0/payload0 EE200 detail **seq13** (QPC542033142630..542033143887). It has no Stage31 BEGIN and must not be fabricated as a second Stage31 alternate.
- Second: proper Stage31 episode1 payload1 with set QPC542085812208, clear QPC542085815861, old 0x30000/new0x10000. Both clearing events are post-write `ED1A9`, matching original `EC4F0`'s conditional `ED19F` mask.
- Stage15 `F6A68_PLAYER_BIT17_BRANCH`: **5** captured calls: one bit17=1 alternate P2 (same Stage31 episode) and four bit17=0 direct `F6AD7` calls, two P1/two P2, `type1/payload0x1E`. Do not confound with type0 route.

## Conclusion and bounded next task

**Stage34 runtime completed for the three naturally observed variants 0/1/4.** New instrumentation works, previous payload0/type1 behavior is preserved, per-event evidence closes the Stage33 `payload1` coverage gap, and payload4's extra controller slot changes are proven. **Payload5 was not emitted in this run**, so it remains **statically understood, but not independently runtime-observed in Stage34**.

**No gameplay bug, attack success, or need to alter bit17 is established.** The next task should be evidence-led: preserve current working Stage34 source and wait for a naturally occurring payload5 if completeness is desired, while investigating which downstream controller consumer converts `P+3A08/P+3A0C/P+3A6C` into an actual input/action. Use the original SHA-pinned AALib.exe from ChatGPT Library `/AALib.zip`, no reupload. If a new Stage35 probe is needed, design a read-only exact consumer observation after static verification, not synthetic event injection.

