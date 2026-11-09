# Stage36 r2 — Windows x86 native candidate-to-command runtime VERIFIED

**Analysis date:** 2026-11-10, original natural COM-vs-COM game session 01:17:45–01:18:22 local (MSVC x86 build at 01:17:20). New user uploads: main [fc6e140](https://github.com/Hawky11/1/commit/fc6e14016371c39809f1d803fa84d3116ce9038c). Archive baseline: main `e40f028`; **23** newly changed/uploaded exact Git blobs (22 .log + Build.log, **14,176,794 bytes**) are present **unchanged** under `Stage36_R2_Runtime/` in evidence branch `stage36-control-to-command-r2-runtime-evidence`, initial archive commit **`8a657de36b66ab8de5aad9bbee4c0784c6e5e0f2`**. The three unchanged root logs `ArcanaAI_PlayerBit17WriteTrace.log`, `ArcanaAI_Stage26ObjectBit0WriteTrace.log`, and `ArcanaAI_Trace.log` were deliberately **not** mislabeled fresh. Source-only fixed Stage36 r2 remains intact in `stage36-control-to-command-r2-stage10-compat` @ `4918328a2eca6b42eb9525802530827359eefe4e`; r1 @ `29ed074` remains unchanged.

## 1. Build and successful native hook installation

- Windows VS18/MSVC **x86** `Build.bat` started 2026-11-10 **01:17:20**, compiler/linker succeeded, **`BUILD SUCCEEDED. DLL size: 266240 bytes.`** Zero MSVC errors, one previously known **C4754** warning in `src/AICommandSiteTrace.cpp:785`.
- `ArcanaAI_Stage36ControlToCommandTrace.log`:
  - `STAGE36_CONTROL_TO_COMMAND_BUILD=2026-11-10-r2`
  - `STAGE36_R2_ENTRY F4360=0x006A4360 bytes=E98B37365090 mode=STAGE10_OWN_DLL detourShape=1 target=0x50A07AF0`
  - **`STAGE36_R2_INSTALLED EB718->F58E0 index=1F EB7D9->F58E0 index=1E F79E4->F4360 candidate; originals called once; 3/3 sites.`**
- Hence the r1 mismatch was definitively caused by preexisting **Stage10 F4360 inline detour**: six-byte `E9 rel32 + 90` transfers to observed Stage10 wrapper address 0x50A07AF0 in our DLL, previously impossible to accept when demanding the original native F4360 prologue. In this run Stage10 parent-flow also reports F4360 installation 6 bytes, prior to Stage36 installation. Three Stage36 caller CALL patch sites are installed. **Success does not imply that every one of the sites was reached.**
- Stage10 `ParentFlowTrace`, Stage35 TIMER/FLAGS, Stage31, Stage29, `FinalCommand`, CPU `QueueTrace`, EE200 `EventDispatch` remained active. None of the examined **core** Stage36, Stage35, Stage10, Stage31, Stage29, FinalCommand, Queue and EventDispatch logs reported explicit positive dropped/overflow or ERROR/WARNING. Preserve other background-log statuses independently.

## 2. Stage36 raw record integrity

Exactly **412 Stage36_RESULT** rows, consecutive **seq 1..412** without gaps/duplicates/malformed records. Every row:

- `kind=CANDIDATE`, `callReturnRVA=0xF79E9`, same game thread TID **18044**, correctly identified exact player pointer.
- **293 P1 + 119 P2**; CANDIDATE may have asymmetric naturally reached call counts.
- `argMask=0x1F` (all 5 native args), `preMask=postMask=0x3FF` (10/10 fields), `outMask=0x3>0x3` (both F4360 output DWORD pointer reads valid before/after). QPC `qpcAfter-qpcBefore` positive **8..93 ticks** in all records.
- Native EAX result: **405 times `0xFFFFFFFF`** sentinel (candidate rejected/unavailable), **seven non-sentinel** values `0x2A` (42), `0x2B` (43), `0x0A` (10), `0x42` (66), `0x13` (19), `0x08` (8), `0x29` (41). All **412/412 nativeEAX equal incoming third arg** `args[2]`. The 2 outputs and 10 read-only player/object snapshots showed no net before/after changes in this **particular callsite/run**. This is an observed identity for the selected path, **not a global claim that F4360 never changes a result**.
- **ZERO** `kind=INPUT_1F` or `kind=INPUT_1E` records despite 3/3 installed. Their native AL results and branch outcomes are **not runtime-verified** in this run. Do not equate zero observed records with zero bitmap usage elsewhere in the native game.

All 412 raw rows, with input/return/masks/QPC: [stage36_all_412_candidate_records.csv](stage36_all_412_candidate_records.csv).

## 3. Verified seven-result native output chain

Every **one of the seven** non-sentinel Stage36 F4360 returns has:
1. Exactly **one** corresponding Stage10 `PARENT_FLOW helper=F4360` observation with matching P1/P2, exact player/TID, EAX and QPC **inside Stage36's call interval** (`qpcBefore ≤ stage10.qpc ≤ qpcAfter`). Stage10's observer remains operational.
2. Exactly **one** immediate `QUEUE_EVENT_DETAIL`, **callerRVA `0xF7C62`**, `eventType=1`, identical **payload=Stage36 nativeEAX**, matching exact P/TID, QPC **11..17 ticks after Stage36 return** (windows 0..9999 QPC ticks used, all unique).
3. Exactly **one** subsequent `EVENT_DISPATCH_DETAIL` `eventType=1` with the **same exact payload and player/TID**, QPC **14..50 ticks after the queue insertion** (window 0..9999 ticks, all unique). Dispatcher function RVA `0xEE200`.

| Stage36 seq | Side | F4360 EAX / queued payload | Stage10 seq | Queue seq | EE200 seq | Return→queue QPC ticks | Queue→EE200 QPC ticks |
|---:|---|---:|---:|---:|---:|---:|---:|
| 13 | P1 | 42 (`0x2A`) | 18 | 11 | 41 | 16 | 50 |
| 46 | P1 | 43 (`0x2B`) | 21 | 12 | 42 | 13 | 20 |
| 55 | P2 | 10 (`0x0A`) | 36 | 18 | 66 | 17 | 18 |
| 71 | P2 | 66 (`0x42`) | 41 | 20 | 68 | 14 | 21 |
| 193 | P2 | 19 (`0x13`) | 71 | 29 | 98 | 11 | 14 |
| 348 | P1 | 8 (`0x08`) | 94 | 35 | 113 | 16 | 25 |
| 378 | P1 | 41 (`0x29`) | 98 | 37 | 118 | 17 | 18 |

[Exact seven-row multi-trace QPC/payload correspondence](seven_candidate_to_queue_to_dispatch_matches.csv). The chain **confirms actual native command *proposal* and dispatch** for seven observed selections, beyond static conjecture; it is **not** confirmation of attack animation, hit, movement, guard execution or a gameplay AI bug. All seven F4360 results equal the input arg3; upstream `F3280`/other decision logic must be studied to interpret choice.

The remaining **405 sentinel** returns do not, by themselves, constitute lost attacks, invalid state or bugs. Stage10's separate `ParentFlowTrace` recorded **34 F4360 rows**, comprising all **7 successful** EAX plus **27 sampled sentinel** failures; its tracing policy sampled failed calls about 1/16, so 34 vs 412 is expected, not dropped data.

**Important probe side effect:** Stage10's `PARENT_FLOW callerRVA` for its 34 F4360 records is now `0xFFFFFFFF`, because the native F4360 entry is invoked **from Stage36's wrapper inside `dinput8.dll`**, not directly from the original game CALL return RVA `0xF79E9`. Stage36 records the *original* callsite `callReturnRVA=0xF79E9`, and the Stage10 `qpc` falls inside its interval. Thus loss of Stage10's callerRVA attribution under nested hooks is a **diagnostic observation/limitation**, not proof of game state corruption. Do not use Stage10 callerRVA to infer a different native caller in this r2 run.

## 4. Prior runtime infrastructure remains sound

- Stage35 recorded **8935 full records, seq1..8935** no gaps/malformed rows and all **`preMask=postMask=0xFFF`**. Counts: TIMER P1 2234 / P2 2234; FLAGS P1 2234 / P2 2233. Hence **4467 complete adjacent TIMER→FLAGS pairs** and **one trailing P2 TIMER record seq8935** without corresponding FLAGS in the saved log; this is a bounded end-of-capture observation, **not an overflow/error claim**. **197** observed timer slot11 changes.
- Stage32 EE200 `EVENT_DISPATCH_DETAIL` exactly **137 seq1..137** without gaps: **95 type0**, **42 type1**. Type0 payload variants `payload0=70`, `payload1=12`, `payload4=10`, `payload5=3` (sum 95). **95/95** type0 Stage33/34 effect rows with exact matching dispatcher `seq/QPC/side` and full `preMask=postMask=0xFFF`.
- Existing `FINAL_COMMAND` has **37** type1 native command proposals; all 37 match their own `payloadESI=payloadArg`. Existing `QUEUE_EVENT_DETAIL callerRVA=0xF7C62` has **37** type1 events; all have matching (P/TID/payload) FinalCommand records, including the seven newly proven Stage36 successful candidates. The other 30 queue events may arise via intermediate selection/fallback logic and cannot automatically be attributed to successful F4360 outputs.
- Stage31 recorded **29** rows: **2 BEGIN, 2 END, 25 type0 queue kind3**. Stage29 captured **4 bit17 transitions** (verify two complete set/clear pairs against detailed Rvas). Other early-stage observers still active.
- Natural payload5 again occurred (three events). Do not conflate payload4/5 timer reloads with physical attacks or control-completion proof.

## 5. Stage37 next handoff

**Stage36 r2 successfully verifies the seven concrete F4360 candidate→type1 queue→EE200 pipeline events**, while the bitmap INPUT_1E/INPUT_1F branch has no runtime samples. Next Stage37 should identify the actual **native action/animation/object execution boundary** following EE200 type1 handling rather than merely re-measuring queue writes; or, if bitmap semantics are crucial, first find which native EB7xx branch is naturally reached by inspecting captured parent/queue callsites and static control flow. No forced type1/0x1E, type0/payload5, bit17 or game-state writes. Reuse original SHA-pinned `AALib.exe` already in ChatGPT Library `/AALib.zip`, do not request again. Keep Stage29 `AITrace.cpp`, all tested r2 source and earlier branches unchanged.

**Status: Stage36 r2 build/hook runtime SUCCESSFUL on observed candidate-to-command path; actual visible gameplay action and AI behavior correctness still NOT verified.**
