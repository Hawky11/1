# Stage130 — Stage129 F30B0 original producer branch runtime audit

**Date:** 2026-11-13 (Latvia local); **status:** Windows-build + same-session runtime **CONFIRMED**. This report audits passive instrumentation; it is **not proof of successful gameplay attack, command commit, or AI improvement**.

## Evidence freeze

- Immutable starting main commit **19290b916e0f7ece195beab9067915cf28272250** ("Add files via upload"), 2026-11-12 22:44:40 UTC; it contains the freshly uploaded root logs. This branch was cut from that commit. Do not overwrite or silently combine with older root log versions.
- Fresh uploaded `Build.log`, `ArcanaAI.log`, `ArcanaAI_Stage129ProducerProbe.log`, `ArcanaAI_Stage104Mode0NativeTrace.log`, `ArcanaAI_Stage107ConsumerReadTrace.log`, `ArcanaAI_Stage110Cebe0GuardTrace.log`, `ArcanaAI_Stage113ParentGateTrace.log`, `ArcanaAI_Stage115Fa480GlobalsTrace.log`, `ArcanaAI_Stage122ObjectWriterTrace.log`.
- Stage129 Git blob SHA **9b13e38079e5f39f2d9668069bc93fcd548e7859**; Stage104 **dc8a083ae6c0f2c0a52f087c65124f815b8fdcda**; Stage122 **78fa7977abe4ee6898bd44724162e51a6e704b50**; Build.log **a3a6964dc9c7d2bbc874c502be80ae9ca2dd9f69**.
- Stage93 FinalCommand QPC / Stage94 producer log examples currently in root start on **2026-11-12 20:29** (thread **5480**, QPC ~3015e9), whereas this fresh Stage129 session uses thread **13856**, QPC ~3202e9. They are **stale, different-session evidence**, not joined here.
- The fresh `ArcanaAI.log` says COMvsCOM active **2026-11-13 01:41:52** with both control DWORDs forced to CPU.

## Build/installation

- MSVC **14.51.36231**, VS2026 Community, **x86**; build began **2026-11-13 01:34:02**. All eight C++ translation units compiled; **BUILD SUCCEEDED**, `build/dinput8.dll` **2,099,200 B** (**+9,216 B** compared with the last tested Stage122 2,089,984-B DLL). Existing non-fatal **C4754** at `AICommandSiteTrace.cpp:786` persists.
- Fresh Stage129 marker `STAGE129_BUILD=2026-11-13-r1`, `STAGE129_INSTALLED 3/3`, `sid=01DD72F704247563`, sites **F30B0** entry, **F325A** forced, **F326A** input-return. Marker declares replayOnce=1, noGameplayWrites=1, DRwrites=0. **Install succeeded**; absent first-launch logs are not evidence of a permanently broken installation.
- Fresh Stage122 `INSTALLED 3/3` shares the same SID. Stage110, 113, 115, 104, 107 all report installed.

## Stage129 exhaustive HEALTH (53 intervals)

| Counter | Sum across nonoverlapping emitted HEALTH windows |
|---|---:|
| F30B0 entry | **3000** |
| F325A forced fallback 0x27 | **0** |
| F326A original input return | **3000** |
| paired | **3000** |
| dropped | **0** |
| mismatch | **0** |
| readFail | **0** |
| overflow | **0** |
| orphan | **0** |
| unknownCaller | **0** |

- Exactly **87 sampled call scopes**, **174 STAGE129_EVENT records** (87 entry and 87 returnInput). Every sampled pair matches same SID/TID/unique scope/frame/object/candidate, has consecutive event sequence IDs, `returnInput.result == entry.candidate`, and the sample's entry-to-return QPC delta is **1–4 ticks**. 27 sampled entry calls have return RVA **F3CC1** (parent F3930); 60 have return RVA **F3659** (other original direct caller).
- Sampled entry candidate distribution: `FFFFFFFF` 78, `00000027` 2, `00000004` 2, and one each `0000001A`, `00000043`, `00000006`, `0000001B`, `00000026`. **Do not extrapolate candidate distributions from 87 sampled entries to 3000 calls**.
- The zero `fallback27` count is **exhaustive per Stage129 HEALTH** and **does not depend on event sampling**. All observed, hooked F30B0 invocations in this session took native return-input path; the exact number of input-0x27 invocations among 3000 is not established by sampled events.

## Strict same-session downstream correlations

- Stage104: **36** MODE0 events (P1=20, P2=16); **17** have `special27=1` (P1=7, P2=10). **17** Stage104 CEA20 events.
- Stage107: **17** POST_CEA20 events, **0** native CEC64 read events. All 17 MODE0 0x27 events pair to CEA20 with identical P1/P2 player/TID/frame and sequential QPC (MODE0→CEA20 **8–66 QPC ticks**), and each CEA20 then matches Stage107 POST at **5–8 QPC ticks**. Stage104 and Stage107 HEALTH errors/drops all zero.
- Two particularly strong **input 0x27** cases have a freshly sampled **Stage129 entry→returnInput** and a **downstream Stage104 MODE0 0x27**, with equal player pointer `0DDCCCA0`, TID **13856**, increasing QPC, and nearby timing:
  1. **scope=43**, caller **F3659**, player P2, `candidate=00000027`, `returnInput.result=00000027`, entry QPC **3202788970136**, return QPC **3202788970137**, Stage104 MODE0 QPC **3202788970231** (**+94 QPC ticks after exit**) → CEA20 **3202788970244** → Stage107 POST **3202788970250**.
  2. **scope=56**, caller **F3CC1**, P2, same `candidate/result=00000027`, entry QPC **3202805788363**, return QPC **3202805788365**, Stage104 MODE0 QPC **3202805788398** (**+33 ticks after exit**) → CEA20 **3202805788406** → Stage107 POST **3202805788412**.
- These are **temporal and identity correlations**, not direct end-to-end scope propagation through Stage104. They strongly show at least two cases where 0x27 was *already* the F30B0 input when a downstream MODE0 0x27 appeared. The Stage129 `fallback27=0` means native **F325A did not force any 0x27** in these observed calls. Do not assert all 17 downstream MODE0 records are F30B0-correlated merely because F325A was never hit.
- Another near-time validation: Stage129 sampled scope **64**, candidate **00000026** from caller F3CC1, precedes a P1 Stage104 MODE0 with `esi=00000026` by **70 QPC ticks** (not special27). This supports the general dataflow.

## Surrounding hooks, same session

| Trace | Exhaustive health summary |
|---|---|
| Stage122 | **parent 11301**, `state3=1`, `slot7=1`, `slotG4=1`, `slotG5=0`; zero drops/mismatches/read failures; two writer events at QPC **3202629158890** (state3) and **3202629158894** (slot7), same object `24F10788` |
| Stage115 | **1282** full snapshots, `pred1or2or3=0`, `predAllFalse=1282`, `predictionDiff=0`, no new hooks |
| Stage113 | **14510** parentA/gateA, **14510** `gateAskip`, `gateApass=0`, parentB=0/gateB=0 |
| Stage110 | `entry=0`, `guard=0`, all error counters zero |
| Stage107 | `post=17`, `nativeRead=0` |

Counts are all scoped to the uploaded same-session HEALTH windows; neither observed **FA480 AL0** nor absent **CEBE0** within this hook establishes an AI-wide acceptance or rejection outcome.

## Conclusion and next stage

1. **Hypothesis resolved for the sampled window:** F30B0 original force-fallback `F325A` did **not** run across **3000** hooked entries. `F326A` returned the supplied candidate all **3000** times. **Two** sampled input-candidate-0x27 events have strong matching Stage104 MODE0 0x27 downstream episodes.
2. **Remaining uncertainty:** where each caller (F3654 and F3CBC) obtains or chooses its **input candidate 0x27**, especially the exact pre-F30B0 origin at F3930 and the other producer family; and whether any produced command ultimately causes a real gameplay action.
3. **Stage131, STATIC-FIRST**: inspect exact original argument prep and caller control flow before **F3654** and **F3CBC**. Verify original AALib SHA and all native edge/signature facts; examine producers and ranking filters that supply 0x27. Decide whether a *new* narrow passive trace would materially answer an unresolved question. **Do not add speculative gameplay writes or broad hooks**. Keep Stage36 OFF, preserve current Windows-tested Stage129 DLL 2,099,200 B and previous Stage122 rollback 2,089,984 B.
