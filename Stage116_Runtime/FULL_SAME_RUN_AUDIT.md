# Stage116 — Same-session Stage115 FA480 global predicate runtime verification

**Date:** 2026-11-12. **Evidence:** user upload [ad3a541](https://github.com/Hawky11/1/commit/ad3a541b9eeced167901d6f82e2617d0485c67fa). **Scope:** independent Windows build and actual passive native instrumentation run, no additional detours, no gameplay assertions.

## Build, config, hook installation

- `Build.log` started **12.11.2026 21:18:41**. Visual Studio 2026 / MSVC **14.51.36231**, x86; all eight C++ TUs compiled and linked `build/dinput8.dll` **2,082,304 bytes**, **BUILD SUCCEEDED**. The familiar nonfatal C4754 warning in `AICommandSiteTrace.cpp:786` remains. No C2668.
- Fresh `ArcanaAI.ini`: `EnableStage115Fa480Trace=1`, Stage113=1, Stage110=1, Stage107=1, **`EnableStage36ControlToCommandTrace=0`**, COMvsCOM=1. Fresh general log: COMvsCOM active **21:19:30**.
- Stage113 **INSTALLED 4/4**, Stage115 **INSTALLED** as piggyback at RVA **9888D** with `addedHooks=0`, `globalRead=5`, `nativeSignatures=PASS`. Stage110 and Stage107 installed **2/2**; Stage104 installed **2/2**. Existing `F2DE0` E9+NOP detour remains in compatibility mode, owner unidentified. No new DR/gameplay data writes are reported by Stage104/107/110/113/115.

## Primary positive result: all 1,455 sampled native snapshots

**52** Stage115 HEALTH rows: `sampled=1455`, `full=1455`, `partial=0`, `pred1or2or3=0`, `predAllFalse=1455`, `predictionDiff=0`, `addedHooks=0`.

All **1,455** `STAGE115_SNAPSHOT` rows: `readMask=1F`, `pred1=pred2=pred3=0`, `predictedAL=0`, original **`observedAL=00`**, **`zf=1`**, `consistent=1`, `snapshotAfterReturn=1`, **TID=10288**. Side recovered from actor+0x154: **P1=670**, **P2=785**. There are exactly **1,455** matching `STAGE113_EVENT kind=3` rows with identical QPC; no missing or ambiguous QPC joins. Stage113 has 2,910 sampled rows (1,455 parent kind1 + 1,455 gate kind3).

Each sampled row has precisely the same five 32-bit values:

| Original preferred VA | Log key | Observed DWORD (hex) | Decimal |
|---|---|---|---|
| `0x0E3C3EFC` | `gE3C3EFC` | `0x00000000` | 0 |
| `0x0E3C4930` | `gE3C4930` | `0x00000000` | 0 |
| `0x0E10B37C` | `gE10B37C` | `0x00000003` | 3 |
| `0x0E10B640` | `gE10B640` | `0x00000007` | 7 |
| `0x0E10B8B8` | `gE10B8B8` | `0x00000000` | 0 |

Original helper logic mapped in Stage115 source-only (correcting preferred-VA operands for image relocation):
```
pred1 = ((G1 == 8 || G1 == 14) && G2 != 0);
pred2 = (G3 == 1 && G4 == 4);
pred3 = (G3 == 2 && G5 == 4);
predictedAL = pred1 || pred2 || pred3;
```
For captured G1=0, G2=0, G3=3, G4=7, G5=0, **all three conditions are false**, agreeing with returned **AL=0**, TEST ZF=1, original JE skip. The values are measured after the helper return, **not while its individual branches execute**; no direct live inspection of helper-internal predicates or proof of globals being invariant for unlogged calls.

## Stage113 full-hit counters versus sampled detail

**52** Stage113 HEALTH records sum to **parentA=16,760**, `gateA=16,760`, `gateAskip=16,760`, `gateApass=0`; `parentB=gateB=gateBpass=gateBskip=0`. `dropped=mismatch=readFail=0`. These HEALTH counters cover **all hits**; 1,455 paired detailed events are intentionally sampled and logged separately. Every sampled gate has `AL=00,ZF=1`.

Therefore in the observed experiment the parent `98780` is reached and its original FA480 result causes the unconditional-in-sample bypass `9888D JE 98965` past the two native reader CALLs `988EA`/`98948`. `CACE0` parent callsite `EFDD8` has no observed hit; cannot exclude other direct/indirect execution paths.

**Label caveat:** old Stage113 `side=P0` reflects player+0x154 nested actor pointer and is NOT evidence of a third player. New Stage115 side inference correctly gives 670 P1, 785 P2 for sampled rows.

## Independent downstream evidence, same upload and same run

- Stage104 logs: **58 MODE0** events, **24** with ESI `0x27`, **24** native `CEA20 PRE` events. Every PRE matches a prior matching MODE0 `0x27` in same TID=10288/player/side/frame/ESI and ordered QPC, **24/24 unique**, lag **6–78** raw QPC ticks. PRE sides P1=9, P2=15.
- Stage107 logs: **24 POST** rows, **24/24** unique PRE→POST joins, same TID=10288/player/side/frame/ESI and ordered QPC; lag **5–12** ticks.
- Stage110: 52 HEALTH records, **CEBE0 entry=0, CEC57 guard=0** (all error counters zero). Stage107 native `CEC64` READ=0 across its 52 HEALTH rows.
- Stage104/107/110/113/115 each show successful installation and **no reported dropped/mismatch/readFail** (where available). This is diagnostic integrity, not proof of improved AI, hits, or all-game stability.
- **IMPORTANT SAME-RUN LIMITATION:** Stage94 ProducerGate and Stage93 FinalCommand logs visible in root **were not among the eight fresh user-uploaded files** in commit ad3a541. Their TIDs **5480/12204** belong to an older run, whereas Stage115/113/104/107 fresh events have TID **10288**. **Do not join them** and do not claim Stage116 fresh Stage94→FinalCommand correlations. Those links remain proven only in previous Stage114 and earlier separate runs.

## Conclusion and recommended next stage

**Stage116 experimentally explains the zero CEBE0 executions on the observed original parent A path:** the native global-state predicates in all sampled windows produce `FA480=0`, precisely matching real AL/ZF and consequent JE skip. The 16,760 real gate hits all skipped according to unsampled full-hit counters. This is stronger than installation-only or static guesses; no further CEBE0 early-guard detour is justified for the **current mode/state**.

The semantic meanings of native global values (0,0,3,7,0) are **not proven**; they might represent modes/state outside the FA480 allow path. **Next Stage117 should begin with a STATIC-only caller/producer audit of the five globals and return gates**, aiming to identify what natural state transitions can make FA480=1, before deciding on any further observer. Do not force field values, patch FA480, or synthesize gameplay commands. No new DLL is necessary solely to complete Stage116.

**Rollback:** latest Windows-built Stage115 DLL **2,082,304 B**; previous known Stage113 DLL **2,079,744 B**; Stage110 fallback **2,073,088 B**. Stage36 remains OFF.
