# Stage114 — Stage113 real Windows parent/gate verification, FA480=0 bypass

**2026-11-12 | RUNTIME VERIFIED (observational only)**

Evidence: [user's fourteen-file upload commit `0a990a8`](https://github.com/Hawky11/1/commit/0a990a8801bd5538c482aa4f6572c633217f05ca). This branch originates from the user-uploaded `main` snapshot, preserving the exact logs. Earlier Stage113 source branch and historical evidence branches remain unchanged.

## Build and source/runtime interpretation

- User Win32 VS2026/MSVC14.51.36231 Build.log started **20:28:55**, completed **BUILD SUCCEEDED**, new `build/dinput8.dll` **2,079,744 bytes**. The prior Stage113 C2668 ambiguity is no longer present; source now uses qualified `arcana_stage113::SuspendAll`.
- Two game sessions are appended to several logs. The first Stage113 session explicitly emitted `STAGE113_DISABLED EnableStage113ParentGateTrace=0` (the general ArcanaAI.log has a COMvsCOM start at 20:29:28). The **second** session emitted `STAGE113_INSTALLED 4/4`, positive events, and COMvsCOM start at **20:30:35**. Its event TID is **12204**. The uploaded INI has `EnableStage110Cebe0GuardTrace=1`, `EnableStage113ParentGateTrace=1`, `EnableStage107ConsumerTrace=1`, and `EnableStage36ControlToCommandTrace=0`. Do not pool unrelated TID5480 events from the first session in same-event correlations.
- Stage113 source marker contains `SOURCE_ONLY` as literal build metadata even when built: successful Build.log + 4/4 install + positive hits prove **actual runtime instrumentation**, not just source readiness.

## Stage113 exact second-session counts

46 `STAGE113_HEALTH` windows:

| Counter | Value |
|---|---:|
| `parentA` CALL 95CD6→98780 | **15,726** |
| `gateA` TEST AL,AL + JE at 9888D | **15,726** |
| `gateApass` (ZF=0) | **0** |
| `gateAskip` (ZF=1, AL=0) | **15,726** |
| `parentB` CALL EFDD8→CACE0 | **0** |
| `gateB`, `gateBpass`, `gateBskip` | **0** |
| `dropped`, `mismatch`, `readFail` | **0** |

The Stage113 event stream contains **2,734** sampled lines: **1,367** `kind=1` (parentA) and **1,367** `kind=3` (gateA). Each sampled gate has **AL=00, ZF=1, valid=1**. **Do not mistake the sampled event count for the full-hit count**: source increments HEALTH counters before log sampling and records the first 32 plus powers-of-two per window. No dropped records reported.

**Stage113 `side=P0` is a pointer-classifier limitation:** at parentA `kind=1` the recorded `arg0` is 0DDA1FA8 (630 samples) or 0DDCCCD0 (737 samples), which are respectively Stage104 P1/P2 base+0x30; the logged `player` / ESI is 0DDA20CC or 0DDCCDF4, i.e. base+0x154 (nested actor substructure). The normal `Side` helper therefore does not recognise them as the exact Stage104 player-base pointers. The `arg2` field is 0 for the first group and 1 for the second. **P0 is not a third participant, and it does not invalidate AL/ZF.** `count=0x37/0x44` is read from nested ESI+0x10CC, not Stage104 player-base+0x10CC.

## Strict same-session upstream joins

Filter events on **TID=12204**; Stage113's `frame` is a different function frame from Stage94/104 and must NOT be equated. All following native upstream joins unique by TID, P1/P2, code, compatible frame and ordered QPC:

- Stage94 `STAGE94_GATE`: **212** = **173 mode1** + **39 mode0**.
- **173/173** gate mode1 → Stage93 `STAGE93_FINAL_QPC`, unique 4–73 QPC ticks, no missing or ambiguous.
- **39/39** mode0 gate → Stage104 `STAGE104_MODE0`, unique 10–85 QPC ticks.
- Stage94 chosen `0x27` observed **19 gates**: **18 mode0 + 1 mode1**. Therefore do **not** claim that every 0x27 is mode0. Of 39 Stage104 MODE0, **18** use ESI=0x27.
- **18/18** Stage104 MODE0 0x27 → Stage104 original `F7DA3→CEA20` PRE, unique 6–27 ticks; P1 8, P2 10.
- **18/18** PRE → Stage107 post `F7DA8`, unique 5–9 ticks, matching actor/TID/frame/ESI/QPC; all valid.
- Stage110 `STAGE110_INSTALLED 2/2` in both sessions; all **57** Stage110 HEALTH rows across both sessions have `entry=0` and `guard=0`, errors 0. Stage107 `nativeRead=0` in all **57** rows. No observed CEBE0 entry/CEC64 read.
- Stage94/104/107/110/113 inspected HEALTH records contain no dropped/mismatch/readFail. This does not prove whole-game stability or an accepted animation, hit, or improved AI.

**Root observation:** the instrumented original `98780` path is reached **15,726** times but every tested `FA480` result is AL=0, sending control via original JE at 9888D directly to 98965, bypassing both direct CEBE0 loops at 988EA / 98948. The absence of CEBE0 calls on **this observed path** now has a positive explanation. The other parent B site EFDD8 was not observed; zero hits at this particular site cannot exclude other indirect callers or other conditions.

## Independent original-native static FA480 criteria

Original private `AALib.exe` SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` was reverified locally, PE32 x86. Original `0xFA480` first calls `0x1F4700`. It returns **AL=1** if any of:
1. `1F4700` returns nonzero. Original 1F4700 returns 1 iff `[0x0E3C3EFC]` is **8 or 14** and `[0x0E3C4930] != 0`.
2. `[0x0E10B37C] == 1` and `[0x0E10B640] == 4`.
3. `[0x0E10B37C] == 2` and `[0x0E10B8B8] == 4`.

Otherwise `FA480` returns 0. For the original code this logically explains how all observed AL=0 can happen: all three allow conditions are false. **The individual global fields are NOT sampled in the uploaded runtime.** Do not invent their values, assert which specific predicate failed, or interpret the numeric values as verified gameplay phase/semantics. The static conclusion assumes the original code at these helpers is not replaced by unobserved third-party detours; read-only runtime capture could verify this.

## Next Stage115 (source-only before another Windows run)

Avoid new original detours. Extend the existing Stage113 `kind=3` event at native JE `9888D` to passively snapshot the five global DWORDs referenced by FA480/1F4700, plus the actual `ESI` and `AL/ZF`, with readMask, guarded direct reads and a bounded sampling ring. Optionally record the three **predicted** booleans; call them predictions rather than observed helper return. Correct side attribution using actor substructure offsets only after verifying P1/P2 base +0x154. Preserve original Stage113 hooks 4/4, code ownership, original flags/stack, no F2DE0 rehook, no DR/gameplay writes and Stage36 OFF. Source SHA gate, rollback, offline tests, and separate Stage115 build marker/log. Backup verified Stage113 DLL **2,079,744 B**; Stage110 fallback **2,073,088 B**.

This Stage114 report is **runtime analysis + static investigation** only. No new Stage114 DLL, hook or gameplay intervention.
