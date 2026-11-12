# Stage93 r3 — live native direct-bit3 and FinalCommand QPC results (2026-11-12)

**Evidence:** user GitHub upload [c623b607](https://github.com/Hawky11/1/commit/c623b607a), including `Build.log`, `ArcanaAI_Stage93DirectBit3Trace.log`, `ArcanaAI_Stage93FinalCommandQpcTrace.log`, Stage92/91/90, `ArcanaAI_FinalCommandTrace.log`, EventDispatch and historical files. Counts independently computed from full Git blobs. **Windows x86 BUILT and runtime INSTALLED; read-only Stage93 observational logic (original native OR re-executed once).** Do not conflate native flags with gameplay actions.

## Windows build and installation

- Build began 2026-11-12 08:04:13 local. All eight translation units compiled and `build/dinput8.dll` linked: **BUILD SUCCEEDED, 2,019,840 bytes**. Only previously known nonfatal `C4754` in `AICommandSiteTrace.cpp:786`.
- `STAGE92_STAGE35_COMPAT adjacentACABE=paired verified=1 ... guardBypass=0`, `STAGE92_INSTALLED 3/3`, `STAGE91_INSTALLED mask=7 sites=3/3`.
- `STAGE93_BUILD=2026-11-12-r1 SOURCE_ONLY` and **`STAGE93_INSTALLED 1/1 site=0F5A68 resume=0F5A6F originalOnce=1`**. Compile-time marker contains “SOURCE_ONLY” but is now **runtime verified**.
- Stage93: **47 HEALTH** records, `filtered=overflow=dropped=mismatch=readFail=0`. Stage92 and Stage91: also **47 HEALTH** records each, all diagnostic error counters zero. Installation runtime does not independently prove visually correct gameplay.

## Exact native results

- Stage92 `F5990`: **15,814** complete ticks; Stage91 direct bit9/10/11 reader records **25,898**, all predictionValid=predictionMatch=1 and all exactly within relevant Stage92 actor/thread/QPC ranges; no missing reads.
- Stage92 recorded **7,907 helper selections**: **10 SET_9ED20**, **7,897 CLEAR_9ED60**.
- Stage93 direct `F5A68` OR: **7,907 records**, all valid, all matched to their unique parent Stage92 tick by tick ID/actor/thread/QPC (no extra or unmatched). Native direct-OR route classification:
  - **6,526** F5990 ticks take the early bit2-bypass path with zero special F58E0 reads;
  - **585** go through bit9 AL=1;
  - **796** go through bit10 AL=1;
  - sum **7,907**, and all exactly execute the F5A68 OR.
  - remaining **7,907** ticks have all three bit9/10/11 reads and proceed to SET/CLEAR helper, with **zero** Stage93 direct-OR events.
- P+3B70 **bit3 rises**: **81** Stage92 F5990 ticks with `before&8=0` and `after&8=1`. **77/77** rises without SET_9ED20 exactly match a Stage93 direct F5A68 before/after `OR` transition (P1=44, P2=33), and **4/4** other rises occur on native SET_9ED20 helper ticks (P1); zero unexplained rises inside observed F5990. The 10 SET selections include six invocations not producing net rising edge. This confirms both native bit3-setting routes for this capture; does not guarantee there are no other writers elsewhere.

## Final-command QPC bridge

- Existing `FINAL_COMMAND`: **133** real type1 records (P1=70, P2=63), **43 distinct payloads**.
- Companion `STAGE93_FINAL_QPC`: **133** records, sequence 1..133 and **133/133 exact one-to-one joins** on player, side, TID, tick, payloadESI, payloadArg, and `P+3B70` full DWORD. No duplicate or missing FinalCommand row.
- Snapshot `bit3=1`: **46** final commands (P1=19, P2=27); `bit3=0`: **87** (P1=51, P2=36).
- For all **133/133** final commands, last *completed* same-player/thread Stage92 tick before command QPC has a matching sampled bit3 value, with QPC gap **533–2,303 raw ticks**. This is precise temporal state consistency, **not** causal proof that bit3 chooses an action/payload.
- Same-run EventDispatch has **430 `EVENT_DISPATCH_DETAIL`** entries (287 type0, 143 type1), all with QPC; 133 FinalCommands are not assumed 1:1 with the 143 type1 dispatch events. Same-payload comparisons alone can be misleading with repeated payload codes.

## Evidence scope / session separation

- New Stage90 `F57D0` contains **1,392** calls with the new player addresses and QPC range; **47** HEALTH rows, all error counters zero.
- Root `Stage87SelectorConsumerTrace.log` and `Stage88Stage24JoinTrace.log` still contain **old-session** rows (QPC ~2.296e12, player ~0DF4CCA0), distinct from Stage93's new QPC ~2.57e12, player P1=0DDA1F78/P2=0DDCCCA0. Do **not** combine their 220 rows with this session as a causal join.

## Next-stage question

Stage93 **closes the 119/127 no-helper rise question from the previous Stage92 run** by capturing the alternate direct OR and explaining **77/77** no-helper rises in this fresh run. The next research target is native *command-choice causality*: track specific same-event state/predicate and eventual FinalCommand payload or exact producer/queue path, rather than equating temporal proximity to cause. Choose a single precise, guarded, passive native boundary after inspecting the original AALib.exe and existing hook owners; avoid overlapping callsite patches, Stage36 remains disabled, preserve verified Stage93 DLL **2,019,840 bytes** and Stage92 r3 fallback **2,011,648 bytes**.

[Machine counters](results.json).
