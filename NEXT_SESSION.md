# Next session handoff

## Current handoff — Stage35 static verified and passive controller-consumer source ready (2026-11-10)

- [Stage35 source-only branch](https://github.com/Hawky11/1/tree/stage35-controller-consumer-trace), with [original PE study](https://github.com/Hawky11/1/blob/stage35-controller-consumer-trace/Stage35_Analysis/STATIC_ANALYSIS.md) and [runtime/install guide](https://github.com/Hawky11/1/blob/stage35-controller-consumer-trace/Stage35_Overlay/README_STAGE35_RU.md). Original exact SHA `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` from ChatGPT Library `/AALib.zip`, static verifier PASS **19 signatures, 7 direct calls**.
- Stage34 evidence is COMPLETE for observed type0 payload 0/1/4 (109/109 net-effect joins; 33/33 Stage31 queue→EE200; payload5 not naturally observed). Preserve the evidence branch `stage34-type0-variant-runtime-evidence` @ `ad801c2278f81cdb997d5773207efecb4211af65` and source branch `stage34-type0-variant-state-trace` @ `803e3ae15f43370dc0fe7969b0ec6242320124d7`.
- New verified controller consumer route: main path `A1106→ACA90`, `ACABE→F60C0` timer update of slot11 `P+3A6C` (`F611C→F7ED0` subtracts **current P+3A3C header value**), then `ACAC4→F5BD0` flags/object updater (`F5F4C` compares slot11, conditional OR `P+3B78` bit7). Another reader `EB7D9→F58E0` tests bitmap `P+3A0C/10` with index `0x1E`. These read/decision boundaries are real but not proof of visible attack/hit/movement.
- Stage35 overlays 3 files: `Stage35_Overlay/src/AIQueueTrace.cpp`, **new** `Stage35_Overlay/src/Stage35ConsumerTrace.inl`, `Stage35_Overlay/ArcanaAI.ini`. Keep installed Stage29 `src/AITrace.cpp`; replace only these Stage35 files. New INI `EnableStage35ControllerConsumerTrace=1`. Two passive x86 callsite wrappers `ACABE` and `ACAC4`, original cdecl helpers once, validated original bytes and rel32 targets, P1/P2 raw 12-field before/after read masks and QPC, buffered worker output `ArcanaAI_Stage35ControllerConsumerTrace.log`.
- **Stage35 Windows x86 compile/install/runtime NOT YET VERIFIED.** User must run `D:\INCUBATOR\ArcanaAIOverhaul\Build.bat` with x86 MSVC, install DLL+INI, archive old logs, perform natural COM-vs-COM and allow >=2 sec flush; upload Build.log, Stage35 log, EventDispatchTrace, Stage31, Stage29, QueueTrace and ProducerCallTrace. First check Build.log warnings/errors, Stage35 build marker and both hooks installed, no dropped/overflow, full read masks, and same-side/player/TID QPC sequence Stage34 EE200 exit → Stage35 TIMER → FLAGS.
- Interpret `P+3A6C=0xF00` as **3840 raw timer units at a writer boundary**, NOT 32 observed frames, attack, or controller input opcode. Stage35 TIMER compares f3 raw slot with f8 current header; FLAGS compares derived f4/f10/f11. Bit7 output alone cannot identify exact internal instruction executed. If needed later, instrument exact `F5F4C` compare/branch or `F58E0` return after proving safe invariants. Never synthesize payload5 or type1/0x1E.
- On new session review this handoff + Stage35 report, check updated GitHub head, and consult ChatGPT Library for original binary (no reupload needed). Update PROJECT_HISTORY after successful runtime, preserving earlier entries.

---


## Current handoff — Stage34 runtime complete; Stage35 downstream controller/action consumer analysis next (2026-11-09)

## Stage34 — real x86 runtime confirms native payload1 and payload4 consumers (2026-11-09)

- **Complete evidence:** [stage34-type0-variant-runtime-evidence](https://github.com/Hawky11/1/tree/stage34-type0-variant-runtime-evidence) @ `ad801c2278f81cdb997d5773207efecb4211af65`, with **21 exact Git-blob runtime logs + Build.log**, [full report](https://github.com/Hawky11/1/blob/stage34-type0-variant-runtime-evidence/Stage34_Runtime/RUNTIME_ANALYSIS.md), [109 before/after records](https://github.com/Hawky11/1/blob/stage34-type0-variant-runtime-evidence/Stage34_Runtime/stage34_all_effect_records.csv), [33 queue→dispatch QPC pairs](https://github.com/Hawky11/1/blob/stage34-type0-variant-runtime-evidence/Stage34_Runtime/stage31_to_stage34_variant_pairs.csv), and [payload summary](https://github.com/Hawky11/1/blob/stage34-type0-variant-runtime-evidence/Stage34_Runtime/stage34_variant_summary.csv). Source-only Stage34 branch `stage34-type0-variant-state-trace` @ `803e3ae15f43370dc0fe7969b0ec6242320124d7` is unchanged.
- **Build:** user's Windows x86 `Build.bat` at 23:46:47 succeeded, `dinput8.dll` **250880 bytes**, MSVC 14.51.36231. One preexisting nonfatal **warning C4754** in `AICommandSiteTrace.cpp:785`; no compiler/linker errors. `STAGE34_TYPE0_VARIANTS_BUILD=2026-11-09-r1` and Stage32/33 markers present; game EE200, Stage31 and Stage29 observers installed.
- **Integrity:** `EVENT_DISPATCH_DETAIL seq1..168` = 90 type0/payload0, **7 payload1**, **12 payload4**, **zero payload5**, 59 type1. **90 STAGE33_EFFECT_DETAIL** for payload0 and **19 STAGE34_EFFECT_DETAIL** for payload1/4. All **109/109** join exactly on seq/QPC/side/TID/player/payload and have **preMask=postMask=FFF**. All 19 new slot0B readings have **slot0BMask=3**; no seq holes or explicit positive dropped/overflow in core logs.
- Stage31 35 event rows = 1 BEGIN,1 END,33 kind3 queue records, payload **0:26, 1:3, 4:4**. **33/33** have one unique same-side/TID/payload QPC-correlated EE200 detail+effect (Δ 6..488 QPC ticks). One scoped alternate episode **P2 type0/payload1**, Stage31 END QPC542085815320 → EE200 **seq56** entry542085815332 (12 ticks) → bit17 clear at **542085815861** → EE200 exit542085816104. It is exactly the previously missing payload1 consumer. Net fields: P+3B84 `30000→10000`, P+3F64 `15→1`, P+3A08 `22→1`, P+3A0C `10002→2`, P+3B70 `20460→20420`.
- **Payload4 native special branch proven at boundaries:** 12 dispatch+effect records, P+3A6C changes in **8** of 12, always ending at **0x00000F00** (including 0→F00 and E88/E73→F00). These match static `EE360→991C0` slot 0x0B prediction; not proof of successful attack/movement. Per-payload net changes (including extra slot): payload0 **27/90**, payload1 **3/7**, payload4 **8/12**.
- **Limits:** no runtime payload5 observed; its native table path is statically analyzed only. Neither dispatcher entry nor controller/net state mutations establish a visible gameplay effect or an AI defect. No reason to change flags or synthesize type1/0x1E.
- **Next research Stage35:** no immediate need to change tested Stage34 observer; first static investigation of downstream **controller-slot consumers** P+3A08/0C/10 and P+3A6C, to identify an actual native command/action boundary, with read-only original SHA-pinned `AALib.exe` already in ChatGPT Library `/AALib.zip`. Separately obtain natural payload5 runtime if it occurs; do not force it. If further instrumentation is warranted, design another bounded passive observation only and preserve all historical branches.

**Immediate Stage35 work:** Check `main` history, [Stage34 runtime report](https://github.com/Hawky11/1/blob/stage34-type0-variant-runtime-evidence/Stage34_Runtime/RUNTIME_ANALYSIS.md) and CSVs first. Then read-only inspect the original SHA-pinned PE32 `AALib.exe` in ChatGPT Library `/AALib.zip`, study all relevant native readers of **P+3A08, P+3A0C, P+3A10 and controller slot P+3A6C** (plus `F57D0` and `991C0`). Map game command/action downstream, not just EE200 net changes, before deciding on a narrow passive Stage35 observer. Stage34 source and all earlier branches immutable, Stage29 AITrace.cpp preserved. `payload5` remains **unobserved**: natural sample only, never synthesize input/events. If user uploads new logs, first compare with Stage34 hashes to avoid reanalysis of unchanged data. Update README/PROJECT_HISTORY/NEXT_SESSION for the next stage as usual.

---


## Current handoff — Stage34 source ready; x86 build and natural runtime next (2026-11-09)

## Stage34 — type0 payload variants 0/1/4/5 statically verified; passive EE200 overlay ready (2026-11-09)

- **Source-only Stage34:** [stage34-type0-variant-state-trace](https://github.com/Hawky11/1/tree/stage34-type0-variant-state-trace) @ `803e3ae15f43370dc0fe7969b0ec6242320124d7`; [static analysis](https://github.com/Hawky11/1/tree/stage34-type0-variant-state-trace/Stage34_Analysis/STATIC_ANALYSIS.md), [Russian installation instructions](https://github.com/Hawky11/1/tree/stage34-type0-variant-state-trace/Stage34_Overlay/README_STAGE34_RU.md), [SHA-pinned verifier](https://github.com/Hawky11/1/tree/stage34-type0-variant-state-trace/Stage34_Analysis/tools/verify_stage34.py). Derived from immutable tested Stage33 source `774ddbbf3a8fec3d6ed09afc31232ade3731c2da`. Previous Stage33 runtime evidence `5eb0d96cd4a20cb713f4add43516857e6ba74389`.
- Native original AALib.exe from ChatGPT Library `/AALib.zip` has SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Read-only Stage34 verifier **PASS** on exact EXE: PE32+SHA, 15 instruction signatures, 6 direct CALL targets, 4 exact 8-byte payload rows and 8 derived table lookups.
- **Native table:** `EE200→158560` at preferred VA `6521F80+payload*8` yields `EDX flagsId,EDI controllerId,ECX output` = **(0,0,0) for payload0/4**, **(1,1,1) for payload1/5**. `158EC0` flag table at VA652222C: id0 byte01 (set P+3B70 bit10, clear bit5), id1 byte03 (set bit10 and bit5); `F57D0` descriptors at VA4DDF188: id0 `01 00 FF` (controller id0/bit0), id1 `01 01 FF` (controller id1/bit1). Payload4/5 take native branch `EE360→991C0` and write controller slot **P+3A6C**. Semantic gameplay action not proven.
- **Instrumentation:** [Stage34 overlay](https://github.com/Hawky11/1/tree/stage34-type0-variant-state-trace/Stage34_Overlay) replaces *only* `src/AIQueueTrace.cpp` and `ArcanaAI.ini`; **keep Stage29 `AITrace.cpp`**. `EnableStage34Type0VariantsTrace=1` adds read-only `EVENT_DISPATCH_DETAIL` and `STAGE34_EFFECT_DETAIL` for type0/payload1/4/5 with 12 prior Stage33 fields + `P+3A6C` read-mask, same seq/QPC/side/TID/player. Payload0 **preserves the original Stage33_EFFECT_DETAIL format**; type1 detail remains. Existing `EE200` hook only, native function invoked once, no new hook and no game-state writes. Stage31/32/33 markers remain.
- **Status:** original static analysis and structural source invariants checked, but **Stage34 has not yet been compiled with Windows MSVC x86 or tested in-game**; don't call it a completed runtime stage. Previous Stage33 x86 build was successful (with nonfatal C4754 in AICommandSiteTrace.cpp).
- **Next user action:** copy those two [Stage34 source/INI files](https://github.com/Hawky11/1/tree/stage34-type0-variant-state-trace/Stage34_Overlay) into `D:\INCUBATOR\ArcanaAIOverhaul`, archive old logs, x86 `Build.bat`, install new DLL+INI, natural COM-vs-COM, >=2 seconds worker flush, upload `Build.log` and especially fresh `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage31AlternatePathTrace.log`, `ArcanaAI_Stage29Bit17Transitions.log`, `ArcanaAI_QueueTrace.log`, `ArcanaAI_ProducerCallTrace.log`.
- **Next analyst:** use Stage33 single missed per-event `episode2 P2 type0/payload1` as specific target. Verify Stage34 build/markers, no dropped/overflow, and join Stage31 scoped episode → same-side/TID/QPC `EVENT_DISPATCH_DETAIL` payload1 → `STAGE34_EFFECT_DETAIL` same seq; compare P+3B70 bit5, P+3A08/0C, P+3B84 bit17, P+3A6C for 4/5. Never infer attack success from state mutations or synthesize type1/0x1E. Original EXE already in Library; do not ask user for reupload.

**Next specific task:** when logs uploaded to `main`, read complete fresh `Build.log`, verify marker `STAGE34_TYPE0_VARIANTS_BUILD=2026-11-09-r1`, `STAGE34_CONFIG`, Stage31/32/33 markers, EE200/F6A8C/F2DE0 installs, check C4754 warning separately and dropped/overflow. Join `STAGE31_EVENT kind3 episode>0 payload=1/4/5` → `EVENT_DISPATCH_DETAIL` by side/TID/QPC and payload → `STAGE34_EFFECT_DETAIL` using identical seq/QPC/player. Validate pre/post 12 field masks, extra slot0BMask, changes in P+3B70,3B84,3F64,3A08/0C/10,3A6C. The previous Stage33 ep2 was **payload1** and filtered, not missing native dispatch. Stage34 source branch is source-only and not yet Windows compiled. Library AALib.zip already exists; no reupload request. Preserve history/branches.

---


## Current handoff — Stage33 runtime complete; Stage34 payload-variant observation next (2026-11-09)

## Stage33 — real x86 build and runtime completed (2026-11-09)

- **Evidence archive:** [stage33-type0-consumer-runtime-evidence](https://github.com/Hawky11/1/tree/stage33-type0-consumer-runtime-evidence/Stage33_Runtime) commit `5eb0d96cd4a20cb713f4add43516857e6ba74389`, contains exact unchanged Git blobs **21 .log files and Build.log** from user's `main` uploads `7ca41fa` and `40eb5bb`; [full runtime analysis](https://github.com/Hawky11/1/blob/stage33-type0-consumer-runtime-evidence/Stage33_Runtime/RUNTIME_ANALYSIS.md), [episode mapping](https://github.com/Hawky11/1/blob/stage33-type0-consumer-runtime-evidence/Stage33_Runtime/episode_correlation.csv), [all 104 effect snapshots](https://github.com/Hawky11/1/blob/stage33-type0-consumer-runtime-evidence/Stage33_Runtime/stage33_effect_records.csv). Source-only `stage33-type0-consumer-state-trace` remains untouched @ `774ddbbf3a8fec3d6ed09afc31232ade3731c2da`.
- **Build:** 2026-11-09 23:02:48, Windows VS18 x86 MSVC 14.51, `BUILD SUCCEEDED`, `dinput8.dll` 249856 bytes. One nonfatal warning **C4754** in `src/AICommandSiteTrace.cpp:785` (related line782); no compilation or link errors. Stage33, Stage32, Stage31 markers and EE200/F6A8C/F2DE0 hooks installed at runtime.
- **Full snapshot integrity:** Stage32 detail seq1..176 (104 type0/payload0, 72 type1); Stage33 `STAGE33_EFFECT_DETAIL` **104/104 exact matching seq/QPC/side/TID/player** and **104/104 preMask=postMask=0xFFF** (12 of 12 fields readable before and after). **37/104** EE200 calls changed at least one monitored DWORD at exit; **67/104** unchanged across those 12 fields (not proof of no transient effects). Stage31 seq1..54 has 3 BEGIN,3 END,48 kind3, type0 payload distribution **0:31,1:9,4:5,5:3**; Stage29 seq1..6 = three bit17 cycles. No explicit positive drop/overflow or seq holes in key logs.
- **Two fully correlated bit17=1 alternate episodes:** Stage31 episode1 P1 type0/payload0 → Stage32+33 detail **seq47** after 12 QPC ticks from Stage31 END; episode3 P2 type0/payload0 → detail **seq161** after 9 ticks. Both show `P+3B84: 00030000→00010000` **during EE200**, `P+3F64: 0x15/0x16→0`, `P+3A08: 0x29→0`, `P+3A0C: 0→1`, `P+3A10: 0x10→0`; P2 also clears `O+184: 0x1E→0`. Of **31/31** Stage31 type0/payload0 queue insertions, all have unique close Stage32 detail + Stage33 net-effect matches within 6..392 QPC ticks.
- **Crucial third episode:** Stage31 episode2 P2 has **type0/payload1**, not payload0. Its Stage31 END QPC515844913931 → Stage29 bit17 clear QPC515844914498 at ED1A9, and aggregate `EVENT_DISPATCH` includes P2 type0/payload1 at 23:04:12, but **no event-level Stage32/33 data** because both intentionally filter to type0/payload0. This is an **instrumentation coverage gap**, not evidence of dropped event. Do **not** assert all 3 episodes have Stage33 effect records.
- **Direct-path controls:** Stage15 seven F6A68 attempts: three bit17=1 alternate (P1 one/P2 two), four bit17=0 direct F6AD7 type1/payload0x1E (P1 two/P2 two). Four Stage21 classifier AL0/fallback slot3=0. No justification to inject type1/payload0x1E or alter native flags.
- **Next Stage34:** static-diff the native lookup/handling for payloads `0/1/4/5` (especially payload1) in `158560/158EC0/F57D0/EC4F0` using original `AALib.exe` already in ChatGPT Library `/AALib.zip` (see `BINARY_SOURCE_IN_LIBRARY.md`). Then **extend the existing passive EventDispatchHook's per-event Stage32/33 filter to type0/payload0,1,4,5** with no new hook or gameplay modifications, retaining Stage29 AITrace and Stage31 wrappers. Verify Stage34 source, x86 build and natural COM-vs-COM; compare per-payload net field changes, never infer gameplay success solely from field writes.

**Exact next work:** first verify current `main`, `PROJECT_HISTORY.md` and [Stage33 runtime analysis](https://github.com/Hawky11/1/blob/stage33-type0-consumer-runtime-evidence/Stage33_Runtime/RUNTIME_ANALYSIS.md), then use original pinned `AALib.exe` already in ChatGPT Library `/AALib.zip` to inspect `158560` table entries and downstream `EE200` for type0/payload1,4,5 vs payload0. Keep Stage33 tested source `774ddbb` and Stage29 `AITrace.cpp` unchanged. Only if justified, create Stage34 **separate** branch derived from Stage33 source with existing EE200 hook passively reporting type0 payload variants and raw controller/object fields; no new hook, no gameplay modification or unverified success claims. Preserve source/evidence Stage31–33 branches. User can later x86-build/install and upload runtime as before. Stage33 already compiled and tested, no re-run needed to prove payload0 path.

---


## Current handoff — Stage33 source prepared, Windows build + natural runtime next (2026-11-09)

## Stage33 — native EE200 type0/payload0 consumer identified; passive state snapshots ready (2026-11-09)

- **Source-only Stage33:** [stage33-type0-consumer-state-trace](https://github.com/Hawky11/1/tree/stage33-type0-consumer-state-trace) @ `774ddbbf3a8fec3d6ed09afc31232ade3731c2da`. [Original EXE static analysis](https://github.com/Hawky11/1/tree/stage33-type0-consumer-state-trace/Stage33_Analysis/STATIC_ANALYSIS.md) · [RU build/installation guide](https://github.com/Hawky11/1/tree/stage33-type0-consumer-state-trace/Stage33_Overlay/README_STAGE33_RU.md) · [verified SHA-pinned Python checker](https://github.com/Hawky11/1/tree/stage33-type0-consumer-state-trace/Stage33_Analysis/tools/verify_stage33.py).
- **Native result:** when EE200 receives type0/payload0, it selects table index0 via `158560` (preferred VA `0x6521F80`, bytes `0F 00 00 00 00 00 00 00`, output bytes +2/+3/+4 zero), then calls `158EC0(P,0)` (bitmask flags P+3B70), `F57D0` (controller P+3A08) and `EC4F0(P,0,0)` (state/object coordination).
- **Exact bit17 clearer found:** `EC4F0` itself calls `F2ED0(queue,1,0x1E)` at `ED193`; if the *history tuple* does not match it, native `ED19F` executes `AND [P+3B84],0xFFF9FFFF`, clearing bit17/18 and preserving bit16. Stage32's three correlated EE200 dispatches had history=(0,0) and Stage29 observed this same post-write site `ED1A9` inside EE200. This accounts for the observed flag transition but **does not prove game action success**.
- **Verification:** pinned original `AALib.exe` from ChatGPT Library `/AALib.zip` (SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`). Checks passed: 24 byte signatures, 5 E8 CALL targets, 3 table values; wrong-binary rejection confirmed. **No Windows x86 MSVC build or Stage33 in-game run yet**.
- **Minimal source overlay:** [Stage33_Overlay](https://github.com/Hawky11/1/tree/stage33-type0-consumer-state-trace/Stage33_Overlay) adds only `EnableStage33Type0StateTrace=1`; extends existing Stage32 `EventDispatchHook` read-only before/after snapshots for 12 targeted DWORD fields for type0/payload0. It emits `STAGE33_EFFECT_DETAIL` in `ArcanaAI_EventDispatchTrace.log`, sharing `seq` and QPC with Stage32 `EVENT_DISPATCH_DETAIL`. No new hook, no gameplay writes, original dispatcher invoked once, existing type1/Stage29/Stage31/Stage32 tracing preserved.
- **Next user:** replace **only** `D:\INCUBATOR\ArcanaAIOverhaul\src\AIQueueTrace.cpp` and `ArcanaAI.ini` with Stage33 overlay; **retain Stage29 AITrace.cpp**, and all other files. Run existing x86 `Build.bat`, install DLL+INI, archive old logs, natural COM-vs-COM, let worker flush >=2s, upload `Build.log` plus fresh `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage31AlternatePathTrace.log`, `ArcanaAI_Stage29Bit17Transitions.log`, `ArcanaAI_QueueTrace.log`, `ArcanaAI_ProducerCallTrace.log`.
- **Next analyst:** verify build/marker/hook; join Stage31 episode → Stage32 detail → same-seq Stage33 effect, check full pre/post read masks, actual net bit/controller/object changes, and dropped counters. Never treat a state change as proof of rendered attack/animation; do not inject type1/0x1E or force bits. Immutable old branches preserved. Original EXE already in Library and must not be requested again.

**First action when user uploads:** check `Build.log`, `STAGE33_TYPE0_CONSUMER_BUILD=2026-11-09-r1`, `STAGE32_TYPE0_DISPATCH_BUILD`, Stage31 marker, installed EE200 and F6A8C hooks, stage33 config. Correlate `STAGE33_EFFECT_DETAIL` with `EVENT_DISPATCH_DETAIL` by identical `seq`, time, side, player; match Stage31 nested episodes with QPC. Use full read masks before interpreting zeros. Compare relevant net raw fields, Stage29 bit17 writes and C4F0 native mask; separate F2ED0 history logic from move success. Keep original EXE pointer `/AALib.zip` in ChatGPT Library, preserve all old branches, do not request file again.

---


## Current handoff — Stage32 runtime complete; Stage33 static consumer analysis next (2026-11-09)

## Stage32 — runtime complete: type0/payload0 reaches dispatcher EE200 (2026-11-09)

- **Immutable runtime evidence:** [stage32-type0-dispatch-runtime-evidence](https://github.com/Hawky11/1/tree/stage32-type0-dispatch-runtime-evidence), commit `a6ed15cce970d92e690f349a8dd7a2cf2e5e5f97`; [full analysis](https://github.com/Hawky11/1/blob/stage32-type0-dispatch-runtime-evidence/Stage32_Runtime/RUNTIME_ANALYSIS.md) and [21 event pairs CSV](https://github.com/Hawky11/1/blob/stage32-type0-dispatch-runtime-evidence/Stage32_Runtime/stage31_to_stage32_type0_dispatch.csv). Source-only branch remains [stage32-type0-dispatch-correlation](https://github.com/Hawky11/1/tree/stage32-type0-dispatch-correlation) @ `44b23d1`.
- 21 original fresh logs copied from user-upload `main` commit `a7a99cf`, retaining their exact Git blob SHA; `Build.log` **not uploaded**, so MSVC compiler/linker transcript remains unverified. Runtime Stage32 marker `2026-11-09-r1`, Stage31 hook at F6A8C and existing EE200 dispatcher hook are installed.
- All three **Stage31 bit17=1 alternate episodes** (P1 once, P2 twice) have nested type0/payload0 at F2DE0 and immediate matched EE200 dispatcher details (END→EE200 11/11/9 QPC ticks). Queue is `active=0` just after F2DE0, then `active=1, history=(0,0)` at EE200. **Stage29 bit17 clear occurs *inside* each matching EE200 call**, independently corroborating the chain.
- Across all Stage31 kind3 type0/payload0 insertions, **21/21** have unique close QPC-correlated dispatch details: 18 unscoped (7–70 ticks) and 3 scoped (210–230 ticks from F2DE0, inclusive of Stage31 wrapper). Stage31 seq1..35 and Stage32 detail seq1..131 contiguous; no explicit drops/warnings/errors in core logs. Four bit17 set/clear cycles in Stage29, but only three have F6A68/Stage31; the fourth P2 cycle clears during **unscoped** type0/payload0 dispatcher #87, not evidence of missing Stage31 logging.
- In same session one direct P1 bit17=0 F6A68 attempt takes F6AD7 type1/payload0x1E and reaches existing queue/dispatcher. **No gameplay effect (attack/movement success) is proven from dispatch alone; no reason to change flags or synthesize type1 events.**
- **Next Stage33:** static first, inspect original `AALib.exe`'s **type0/payload0 path within `EE200` and downstream effect writes/calls**; distinguish dispatcher entry from effective game action. Original EXE already available in ChatGPT Library `/AALib.zip`, pinned SHA in `BINARY_SOURCE_IN_LIBRARY.md`. If runtime is needed, design passive narrowly targeted observation only; preserve prior Stage branches and do not request EXE again.

**Immediate Stage33 task:** Inspect original SHA-pinned AALib.exe and isolate the type0/payload0 branch of `EE200` and downstream consumers (including bit17 clear and any observable action/effect writes). Use Stage32 evidence report and correlation CSV first. Avoid false success inference from `active=1` or entry into dispatcher. Start with static inspection, no new patch unless an unresolved specific runtime question remains. Keep Stage29 `AITrace.cpp` and Stage32 `AIQueueTrace.cpp` as latest tested combination; historical branches immutable. Do not ask user to upload AALib.exe again; it is in Library `/AALib.zip`.

---


## Current handoff — Stage32 source ready; MSVC + natural runtime next (2026-11-09)

## Stage32 — type0 dispatcher activation path verified statically (2026-11-09)

- **New source-only branch:** [stage32-type0-dispatch-correlation](https://github.com/Hawky11/1/tree/stage32-type0-dispatch-correlation), commit `44b23d1634139fe15928b1ff50bbddda056f633d` (based on immutable Stage31 runtime evidence `0a9ae75`). [Technical analysis](https://github.com/Hawky11/1/blob/stage32-type0-dispatch-correlation/Stage32_Analysis/STATIC_ANALYSIS.md) · [RU install guide](https://github.com/Hawky11/1/blob/stage32-type0-dispatch-correlation/Stage32_Overlay/README_STAGE32_RU.md).
- **Native finding:** `F2DE0` deliberately leaves `active=0` for type0/payload0, but `F2E20` checks changed current vs history, `F2E50` transfers the current event to history, and `EE480` activates it at `EE589` before calling dispatcher `EE200` at `EE5C6`. Stage31's six episodes all wrote type0/payload0 into a tuple whose history type was 1; they satisfy the comparison if no intervening overwrite. Existing Stage31 EventDispatchTrace shows type0/payload0 dispatched on both P1/P2 with `active=1`, but only in aggregated groups: no six-to-six event attribution yet.
- **Bit16:** original `E6C66` sets bit16 and `E9E66` can clear it later; cannot infer gameplay semantics or completion from the Stage31 post-snapshot alone.
- **Verification:** original Library `/AALib.zip`, `AALib.exe` SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Verifier `Stage32_Analysis/tools/verify_stage32.py` validates 21 native instruction signatures + 5 E8 targets (passed on original); Stage32 source predicate/invariants checked. **Windows MSVC x86 build and Stage32 runtime not yet tested.**
- **Install:** replace **only** `D:\INCUBATOR\ArcanaAIOverhaul\src\AIQueueTrace.cpp` and `ArcanaAI.ini` using [Stage32 overlay](https://github.com/Hawky11/1/tree/stage32-type0-dispatch-correlation/Stage32_Overlay); **keep Stage29 AITrace.cpp**, all other files and Stage31 observer. Stage32 adds `EnableStage32Type0DispatchDetail=1` to capture type0/payload0 via **existing EventDispatchHook** (no new hook), records in `ArcanaAI_EventDispatchTrace.log` as `EVENT_DISPATCH_DETAIL` with marker `STAGE32_TYPE0_DISPATCH_BUILD=2026-11-09-r1`. Type1 details remain enabled.
- **Next user action:** x86 `Build.bat`, save `Build.log`; install DLL and INI, archive old logs, natural COM-vs-COM, >=2 seconds flush; upload fresh `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage31AlternatePathTrace.log`, `ArcanaAI_Stage29Bit17Transitions.log`, `ArcanaAI_QueueTrace.log`, `ArcanaAI_ProducerCallTrace.log`.
- **Next assistant action:** verify MSVC build, Stage32 marker and EventDispatch hook installation; correlate individual type0/payload0 `EVENT_DISPATCH_DETAIL` QPC/side/player/current/history with Stage31 episodes and Stage29. Check overwrite intervals and dropped counters, never infer strict parentage from time alone. If confirmed, investigate downstream command semantics rather than injecting type1/payload0x1E. Preserve all historical branches.

**Do not ask for AALib.exe again:** verified original exists in ChatGPT Library `/AALib.zip`; pointer `BINARY_SOURCE_IN_LIBRARY.md`. Do not redo Stage31; it is archived and completed. Avoid changes to Stage31 evidence/source and historical Stage branches. Stage32's only file replacements are `AIQueueTrace.cpp` and `ArcanaAI.ini`; Stage29 `AITrace.cpp` must remain.

---


## Current handoff — Stage31 runtime completed; Stage32 analysis next (2026-11-09)


- **Evidence branch:** [stage31-alternate-path-runtime-evidence](https://github.com/Hawky11/1/tree/stage31-alternate-path-runtime-evidence), commit `0a9ae75cd952f9ec6e2d8588a3c44ce0974cb466`. Отдельная, неизменённая source-ветка: [stage31-alternate-path-runtime-trace](https://github.com/Hawky11/1/tree/stage31-alternate-path-runtime-trace), `e905316`.
- [Полный разбор](https://github.com/Hawky11/1/blob/stage31-alternate-path-runtime-evidence/Stage31_Runtime/RUNTIME_ANALYSIS.md) · [6-эпизодная корреляция CSV](https://github.com/Hawky11/1/blob/stage31-alternate-path-runtime-evidence/Stage31_Runtime/correlation.csv). Архивированы все **21** свежих лога из загрузки `main` commit `6e3c9a8`, без изменения blob SHA.
- Stage31 marker `2026-11-09-r1`, установка хука `F6A8C→130A10` подтверждена. `STAGE31_EVENT` seq1..64: 6 BEGIN, 6 END, 52 type0; ровно по одному `kind=3`, `type0/payload0` внутри каждого из шести `episode` (P1=3, P2=3), 46 внешних type0 имеют `episode=0`. BEGIN/END все с `mask=FFFFFFFF`, без дыр и явных ошибок/сбросов.
- Все шесть цепочек QPC подтверждены: Stage29 bit17 set → Stage15 `F6A68` bit17=1 / `branchTaken=0` → Stage31 BEGIN → вложенный `F2DE0` от `156E4C` → END → Stage29 bit17 clear. Во всех случаях P+3B84 `0x20000→0x30000` внутри helper (bit16 появляется). Очередь при вложенной type0 записи: `(guard,active,storedType,storedPayload)=(0,0,FFFF,FFFF)→(0,0,0,0)`; **active остаётся 0**, поэтому не приравнивать вызов очереди к выполненной игре-команде.
- Контрольный P1 Stage15 с bit17=0 отправил `type1/payload0x1E` через `F6AD7`, со сменой queue active 0→1 и Stage21 classifier AL0. Различие нативных ветвей подтверждено, **дефект игрового ИИ пока не доказан**.
- **Build.log не был загружен.** MSVC transcript отсутствует; работоспособность кода подтверждена игровым runtime и установкой хуков.
- **Следующий Stage32:** статически исследовать дальнейшую судьбу type0/payload0 при active=0 и функцию bit16 в том же состоянии, затем, только при необходимости, узко инструментировать type0 dispatch с QPC. Не менять значения битов, не внедрять события и не считать отсутствующую type1 команду багом без подтверждения.

**Next assistant action:** use [Stage31 runtime report](https://github.com/Hawky11/1/blob/stage31-alternate-path-runtime-evidence/Stage31_Runtime/RUNTIME_ANALYSIS.md) and [Stage30 native disassembly](https://github.com/Hawky11/1/blob/stage30-native-alternate-path-analysis/Stage30_Analysis/STATIC_ANALYSIS.md). Scope Stage32 to prove/disprove downstream handling of type0/payload0 and bit16; begin with offline disassembly of the pinned original `AALib.exe` available in ChatGPT Library `/AALib.zip` ([pointer](https://github.com/Hawky11/1/blob/main/BINARY_SOURCE_IN_LIBRARY.md)). Do not infer a bug or make gameplay modifications without further evidence. Do not repeat Stage31 build instructions; runtime is already observed. Retain Stage29 AITrace.cpp and Stage31 AIQueueTrace.cpp as latest tested overlays for any later experiments. Historical branches remain immutable.

---


## Stage31 — alternate-path observer source prepared (2026-11-09)

- **Source-only branch:** [stage31-alternate-path-runtime-trace](https://github.com/Hawky11/1/tree/stage31-alternate-path-runtime-trace), commit `e905316353b844f276a40dff33516d48aa1e4d99`, based on Stage30 `78c2358d57d7b31814cea61eeb94aa49bb996473`. [Installation, log schema, validation and limitations](https://github.com/Hawky11/1/blob/stage31-alternate-path-runtime-trace/Stage31_Overlay/README_STAGE31_RU.md).
- Apply **only** `Stage31_Overlay/src/AIQueueTrace.cpp` and `Stage31_Overlay/ArcanaAI.ini` to the working project. **Retain Stage29 AITrace.cpp and all other files.** New queue source is based on working Stage25 queue source, retaining Stage21–25 hooks. No project/header changes. Keep historical branches unchanged.
- New five-byte CALL wrapper at F6A8C -> 130A10 captures BEGIN/END with per-thread nested episode IDs and 32 raw DWORD fields/read mask. A separate per-event queue record observes type0 returnRVA156E4C, with before/after snapshots, QPC, current episode (0 when unscoped), and parent player. It does not change the old type1 detail filter or aggregate logger.
- Marker `STAGE31_ALTERNATE_PATH_BUILD=2026-11-09-r1`; dedicated `ArcanaAI_Stage31AlternatePathTrace.log`; INI `EnableStage31AlternatePathTrace=1`. Requires installed queue observer. Ring capacity1024; depth8; explicit dropped/overflow warnings. Formatting/I/O stays on worker. The alternate wrapper preserves GPR/EFLAGS and uses aligned FXSAVE/FXRSTOR around diagnostic calls; original helper invoked once.
- **Validation completed:** original EXE SHA and four source byte signatures, both call targets and INI; extracted C++ logic tests for read masks, nesting/parent IDs, exact-once mock call, filtering, TLS isolation and buffer overflow; 12 Unicorn stack/flags/FX-data comparisons. Emulator FIP/FDP metadata is excluded because Unicorn does not restore it; full native IA32 test explicitly skipped (host cannot execute IA32 ELF). **No MSVC build or in-game Stage31 validation yet.**
- Limitations: non-atomic snapshots; normal-return TLS lifetime (abnormal unwind may leave unclosed scope); buffered tail can be lost on exit; local state-block candidate is not the resolved redirected E6960 result. No gameplay fix, forced bit, duplicate event or boolean result inference.
- **Next user action:** x86 Build.bat, save Build.log, install DLL/INI, archive old logs, natural COM-vs-COM run, allow >=2s final flush, upload Build.log + all fresh logs (especially Stage31, Stage29, ProducerCallTrace, QueueTrace, EventDispatchTrace).
- **Next assistant action:** require marker + installation line + complete BEGIN/END pairs; reject absence conclusions with drops/unclosed scopes. Correlate Stage29 set -> Stage15 bit17=1 -> Stage31 BEGIN -> same-episode kind3 type0 (if selected) -> END -> clear. Evaluate bit19/20/21, object+20 and special-state alternatives before calling missing kind3 a defect. Archive exact tested source/INI and runtime separately after evidence arrives.

---

## Stage30 — native alternate path statically verified (2026-11-09)

- **Completed offline analysis:** [stage30-native-alternate-path-analysis](https://github.com/Hawky11/1/tree/stage30-native-alternate-path-analysis), commit `78c2358d57d7b31814cea61eeb94aa49bb996473`, parent Stage29 evidence `d5f2cd0266759aa95f8617a3eb3e3770040d73d3`. [Detailed findings](https://github.com/Hawky11/1/blob/stage30-native-alternate-path-analysis/Stage30_Analysis/STATIC_ANALYSIS.md), 12 bounded disassembly windows and reproducible SHA-pinned Python/objdump verifier. Exact original AALib.exe verified; 16 direct calls, 9 signatures and 5 constants passed. Wrong-binary rejection also checked. No DLL changes/build/new runtime.
- **New finding:** bit17 alternate route has real state writes and conditional chain `130A10 -> 12F3B0 -> 156DA0 -> F2DE0`, final CALL `156E47` / return `156E4C`, type0 and payload0/1/4/5. “Bypasses queue” means bypasses **type1/0x1E at F6AD7**, not necessarily all queue activity. The producer forces AL1 after 130A10; that is not a downstream success result.
- E8560 is shared by both producer routes; conditionally changes object direction bit3 and negates two floats. 156C00 copies four DWORD references. 130A10 clears counters, changes object flags/state block, and invokes E6C60 (which sets P+3B84 bit16); branch priorities depend on bits20/21 and entry bits17/19/20/21. Details distinguish confirmed operations from inferred gameplay meaning.
- Stage29 seq2/4/6/8 all show `0x30000 -> 0x10000`: bit16 present before bit17 clear, consistent with E6C60 but **not proof of its writer site**, because unchanged-bit17 writes were filtered. Existing QueueTrace contains accepted type0 records at 156E4C, but aggregation lacks per-event QPC/parent context: do not pair them with the four episodes.
- **Next Stage31 (not implemented):** bounded passive wrapper at F6A8C -> 130A10, entry/post QPC and relevant state snapshots; extend existing queue observer for per-event 156E4C type0 with same-thread scoped parent context. First inspect source filters/aggregation. Preserve state, args, registers/EFLAGS/FPU semantics. No forced bit17 or duplicate queue insertion. Gameplay defect remains unproven.

---

## Current handoff — Stage29 live correlation complete; Stage30 alternate-path analysis next (2026-11-09)

- **Completed immutable evidence branch:** [stage29-bit17-runtime-evidence](https://github.com/Hawky11/1/tree/stage29-bit17-runtime-evidence), HEAD `d5f2cd0266759aa95f8617a3eb3e3770040d73d3`, based on immutable Stage29 source `ad78d4668bd4ca14fc471289037bd6ef9ed62f0b`. Copied all 20 changed/new runtime logs exactly from user upload `main` commit `c5d49fec910f96e621e7502fb2a2cc5d5f1fb57d` (no history was rewritten). Analysis at [Stage29_Runtime/RUNTIME_ANALYSIS.md](https://github.com/Hawky11/1/blob/stage29-bit17-runtime-evidence/Stage29_Runtime/RUNTIME_ANALYSIS.md) and machine-readable [CSV](https://github.com/Hawky11/1/blob/stage29-bit17-runtime-evidence/Stage29_Runtime/payload_1E_correlation.csv). User **did not upload Build.log**; runtime execution confirmed from Stage29 marker and live hardware probes, MSVC transcript not independently confirmed.
- **Stage29 runtime:** marker `2026-11-09-r1`; DR7 `0x00DD0005`, P1 field `0x0E255AFC`, P2 field `0x0E280824`, initial both zero, 8 contiguous transitions (4 set via post-RVA `0xFAD2`, 4 clear via post-RVA `0xED1A9`; 2 cycles per fighter), last filter count `suppressedNoBit17Transition=21456 recorded=8`. Stage17 `0xC6502` had eight index 0x71 calls, two per cycle. TID17912 caught; excluded COMvsCOM TID17960 and other uninstrumented threads remain limitations.
- **Confirmed same-run producer matrix:** five Stage15 `F6A68` payload-0x1E attempts: P1 tick42674656 bit17=0 JE taken→F6A9D→F6AD7→queue/dispatch; P2 tick42674953 bit17=1 no queue; P1 tick42682546 bit17=1 no queue; P2 tick42689218 bit17=1 no queue; P1 tick42691156 bit17=1 no queue. At all four bit17=1 producer gates, same player's Stage29 set QPC **preceded** gate QPC and clear QPC **followed**. These are **not side-specific**; they explain branch selection but do **not** establish that the alternative branch is a bug. The Stage21 classifier AL0/fallback occurs only for queued bit17=0 event, with slot3/bit400 both zero before & after.
- **Next Stage30 task:** inspect original code for `0xF6A75..0xF6A9C` and helper `0x130A10` call chain, identify data/output/return effects of alternative pathway compared to queue/dispatcher, devise *passive* observation only if necessary. Avoid mutating bit17 or injecting events (would invalidate original behavior). Use original `AALib.exe` from user's ChatGPT Library **`/AALib.zip`** (SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`); never ask user to reupload without checking [BINARY_SOURCE_IN_LIBRARY.md](BINARY_SOURCE_IN_LIBRARY.md). The local Windows root is `D:\\INCUBATOR\\ArcanaAIOverhaul`.
- **Repo discipline:** do not alter earlier Stage branches, append stage outcomes to PROJECT_HISTORY, keep NEXT_SESSION current and README as entry point. Stage29 evidence is archived; if proceeding, create separate Stage30 branch.

---


## Previous handoff — Stage29 source ready, user build/runtime next (2026-11-09)

- **Source branch:** [stage29-bit17-producer-correlation](https://github.com/Hawky11/1/tree/stage29-bit17-producer-correlation), HEAD `ad78d4668bd4ca14fc471289037bd6ef9ed62f0b`, based on immutable Stage28 original-PE analysis `6a210f4be510ab264ddefcadd8d740bfe923cfa4`. Stage29 source-only, **not yet compiled/tested in game**.
- **Apply Stage29:** replace just `D:\INCUBATOR\ArcanaAIOverhaul\src\AITrace.cpp` and `D:\INCUBATOR\ArcanaAIOverhaul\ArcanaAI.ini` with `Stage29_Overlay/src/AITrace.cpp` and `Stage29_Overlay/ArcanaAI.ini`. Retain existing Stage25 `src/AIQueueTrace.cpp` and all other working sources. Build with Windows MSVC x86 `Build.bat`; check `BUILD SUCCEEDED` and 2/2 watchpoint addresses.
- **INI:** `EnablePlayerBit17WriteTrace=1`; `EnableStage29Bit17TransitionsOnly=1`; `EnableStage26ObjectBit0WriteTrace=0`; `EnableControlFieldTrace=0`. DR0/DR1 only one data-watch mode. Retain Stage14/15/17/18/19/20/21/22/23/24/25 and queue/alternate dispatch logging. Stage29 logs changes to bit17 only, keeps shadow current even on filtered writes, captures QPC, post-EIP, regs, code bytes and eight stack DWORDs. New log `ArcanaAI_Stage29Bit17Transitions.log` with marker `STAGE29_BIT17_TRANSITIONS_BUILD=2026-11-09-r1`, `STAGE29_CONFIG`, `STAGE29 active`, `STAGE29_BIT17_TRANSITION`, filter/dropped counters. Static source and 12 INI invariants verified; **no user MSVC build yet**.
- **Purpose/evidence:** the same Stage26 17:28 run has Stage15 payload0x1E attempts P2 tick31450296/31460765 with `bit17=0` and matching ALT call/dispatch, P1 tick31451546/31457515 with `bit17=1` and no alternate queue call. Original `0xF6A75` path with bit17 set calls `0xE8560`, `0x156C00` snapshot, `0x130A10` processing and returns AL1 without queue. P2 events classifier `0xE8CF0` AL0 statically required for payload0x1E; fallback clears an already-zero slot3 and bit400. **Different processes of exclusion vs normal post-dispatch cleanup**, no proof that bit17's upstream setter caused a gameplay defect.
- **Stage16 old 16:51 capture:** 24,176 raw write events, 0→1 twice (P1 and P2, post-EIP 0xFAD2), 1→0 twice (P1 and P2, post-EIP0xED1A9). These QPC values are from a different session and cannot be mixed with 17:28 evidence. Stage29's monitored baseline could already be set; if no bit change appears, don't claim there was no setter. Watchpoint threads exclude COMvsCOM worker; may miss some writes.
- **Next analysis after uploads:** check build and Stage29 marker, 2/2 watchpoint addresses, dropped/overflow, sequence continuity, same-session Stage15 P1/P2 `F6A68` transitions, Stage17 setter `C6502`, queue and Stage21 details by QPC. Archive complete exact logs and chat Build.log into separate immutable evidence branch. If compile fails, fix Stage29 source branch in a new commit; do not touch Stage28 and older branches. Full instructions and known matrix in `Stage29_Overlay/README_STAGE29.md` and `STAGE29_STATIC_BASELINE.md`.
- **Permanent original binary in Library:** `/AALib.zip`, contains `AALib.exe` SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Consult [BINARY_SOURCE_IN_LIBRARY.md](BINARY_SOURCE_IN_LIBRARY.md) and use `files__list` → `files__materialize` rather than asking the user to re-upload.

---


**FIRST ACTION FOR NEW ASSISTANTS:** Original game `AALib.exe` is available in the user's ChatGPT Library **`/AALib.zip`**, not GitHub. See [BINARY_SOURCE_IN_LIBRARY.md](BINARY_SOURCE_IN_LIBRARY.md) for retrieving it via `files__list` and `files__materialize`, SHA256 and PE layout. **Never ask for a reupload without checking Library.**


## Previous handoff — Stage28 original binary state lifecycle verified, Stage29 next (2026-11-09)

- **Stage28 complete, analysis-only:** [stage28-original-pe-state-lifecycle](https://github.com/Hawky11/1/tree/stage28-original-pe-state-lifecycle), HEAD `6a210f4be510ab264ddefcadd8d740bfe923cfa4`. Based on Stage27 archived `stage27-bit0-tailjump-verified` `78e48cdcef7a67fcdf64a50dfa95228357eedbe6`. Contains `Stage28_Analysis/STATIC_STAGE28_ANALYSIS.md` (full verified analysis), `Stage28_Analysis/README_STAGE28.md`, `Stage28_Analysis/tools/verify_stage28.py`, `Stage28_Analysis/tests/test_stage28.py`, `LOCAL_TEST_RESULTS.txt` and `VERIFICATION_SUMMARY.json`. No DLL source changed; no new game run required.
- User's original game executable was located in **ChatGPT Library** archive `/AALib.zip` (not a GitHub source path). It contains AALib.exe 175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, matching fingerprint from Stage16–27. Materialized locally for this session only; no game EXE or compressed archive was committed. PE32 base0x400000, .text RVA0x1000 raw offset0x400. Original .text section (2.87MB) disassembled with GNU objdump; 26 direct signatures, 16 rel32 CALLs and 1 rel32 tail-JMP validated by a Python verifier. Six tests passed locally including exact PE.
- **New `0x978B0` analysis:** call at RVA0x979D2 only direct reference detected in linear-sweep whole-.text; 0x978B0 selects fighter with 0x9EEA0(object+0x108/+0x10C), tests bit1 of `[object+0xC8]` (=player+0xF8). When set, clears bit1 and checks three `0xF2ED0` exact pair comparisons against selected fighter `[fighter+0x3A34/3A36] == (0,0x13)/(0,0x14)/(0,0x15)`; if NONE matches, calls `0x97830` directly to write object state1 and returns AL1. These are static control-flow possibilities, not observed Stage26 actions. `0xF2ED0` is a read-only pair predicate, **not the separate event queue writer 0xF2DE0**.
- Else common gates `[object+0xC4]&1==0`, `[object+0xCC]&2==0`, `[object+0xD4]==0`. It calls `0x991E0(object+0x10,0)` to **decrement** slot0 by its base stride, clamp the new value at zero and return **the prior value**. If prior<=0, calls `0x97830`, which writes object state1 `[object+0xD4]=1`, calls 0xF7EB0/0x976D0(mode0)/0x99150 and ORs object+0xC4 by0x400. The already proven `0x97870` instead clears object+0xC8 bits0,2,3, writes state2, 0x976D0(mode1), then similar helpers.
- Important data caveat: Stage26 transition-only monitor produced 140 full DWORD updates 70 (0→1) and 70 (1→0), **no bit1 at sampled boundaries**. Because unchanged-bit0 writes are omitted, bit1 could still change between captured records. Do not claim bit1 was absent throughout or that 0x978B0's special branch was taken.
- **Next concrete Stage29 planning:** focus again on original COM-vs-COM condition: Stage15 bit17 [player+0x3B84] gates at 0xF6A68, Stage16 observed bit17 setters/clearers (old 12MB blob not a fresh Stage26 run), Stage21 classifier AL0/fallback slot3 and Stage24 output mask0x10001. Analyze stage-specific timestamp/branch chronology from immutable logs before deciding whether a new passive hook is needed. Stage28 does not prove a direct causal link to these. Original game PE is present in user's Library and can be re-materialized as needed in future conversations; search exact `AALib.zip` by Library metadata instead of asking them to upload again.
- Historical branches Stage25 source/evidence, Stage26 source/evidence, and Stage27 evidence snapshots preserved. Continue appending PROJECT_HISTORY and updating README/NEXT_SESSION after each completed stage.

---


## Previous handoff — Stage27 full cleaner control flow verified, 2026-11-09

- Latest immutable branch [stage27-bit0-tailjump-verified](https://github.com/Hawky11/1/tree/stage27-bit0-tailjump-verified), HEAD `78e48cdcef7a67fcdf64a50dfa95228357eedbe6`. Parent static branch `stage27-verified-static-callgraph` `2e08b356fa9923e57b0c78c08c70adbf8e1015c1`. Original exact second user-uploaded report `Stage27_CompleteCallGraph/Stage27_CallFlow_97970.txt`, SHA1 Git blob `d5d8c9f90665baf1c625bb65515910c2984b7a64`, CRLF 4,257 bytes. Detailed annotated x86 disassembly `Stage27_CompleteCallGraph/VERIFIED_97970_BRANCH.md`. Original game SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` confirmed by the user's local script.
- **Confirmed graph:** `0x97609 CALL 0x97970`. `0x97970` reads `state=[ESI+0xD4]`, which maps to `player+0x104` because ESI=`player+0x30` in observed clear records. When `state=1` and guards pass, `0x979CC CALL 0x97870` returns to `0x979D1`, then calls `0x978B0`. Stage26 shows **34** clear records (P1=16, P2=18). When `state=2` and guards pass, `0x979AC JMP 0x97870` after POP ESI/EBP is a **tail jump**; `0x97870` returns directly to original caller RVA`0x9760E`; **20** records (P1=12, P2=8). State0 calls `0x978B0` without clearing, states other than0/1/2 return.
- **Guard for both state1/state2 clear routes:** `[ESI+0xC4]&1==0`, `[ESI+0xCC]&7==0`, `[ESI+0xC8]&1==1`. Original 97870 helper: `AND [EDI+0xC8],0xFFFFFFF2` clears bits0,2,3 at player+0xF8, writes `[EDI+0xD4]=2` (player+0x104) and sets `[EDI+0xC4]|=0x400` (player+0xF4), calls helpers `0xF7EB0`, `0x976D0`, `0x99150`.
- Stage26 original 140 transitions, 70 0→1/70 1→0, exactly 54 clears via 97870, 34+20 now fully explained; two P2 payload0x1E events Stage26 set seq1→clear seq2 and set seq87→clear seq88 both took state1 CALL path (0x979D1 return), not state2 tail jump in that capture.
- **Remaining tasks**: investigate `0x978B0` (called after state1 clear and on state0) and parent `0x97609` guard/state transitions using the same original fingerprinted PE. The current report includes partial 978B0 bytes, not the full function. Determine whether `player+0x104` and bit0 serve action-state lifecycle and whether any relation to Stage15 bit17 or Stage21 fallback exists — do NOT assert gameplay semantics/causality without evidence. Start with a read-only offline PE window report; no new hooks yet. If the user says gogo, implement a separate Stage28 offline static inspector in a new branch, preserve previous Stage source/evidence and maintain README/NEXT_SESSION/PROJECT_HISTORY.
- Prior Stage27 branches: `stage27-bit0-lifecycle-callers` `b490f87892791d45f0b0841a47e70d60329ec457`; `stage27-verified-static-callgraph` `2e08b356fa9923e57b0c78c08c70adbf8e1015c1`; Stage26 source `a249f452353c340fe8f1dab23375a8a932b079b4`, runtime evidence `4b7564ff71664fedd5745e3bcfc4210d74540075`. All immutable.

---


## Previous handoff — Stage27 original PE verified; inspect missing 0x97970 branch next (2026-11-09)

- **New immutable evidence branch:** [stage27-verified-static-callgraph](https://github.com/Hawky11/1/tree/stage27-verified-static-callgraph), HEAD `2e08b356fa9923e57b0c78c08c70adbf8e1015c1`, parent original Stage27 analysis branch `stage27-bit0-lifecycle-callers` HEAD `b490f87892791d45f0b0841a47e70d60329ec457`. Contains exact CRLF 2,389-byte user-uploaded `Stage27_VerifiedStatic/Stage27_AALib_StaticReport.txt`, SHA1 Git blob `7e3dc3a77f602fde1e82ca9fe70903d4d29dd9c7`, `VERIFIED_CALL_GRAPH.md`, read-only `tools/inspect_call_flow.py`, 5 local Python tests and test results. No DLL hook or build was created.
- **From exact pinned AALib.exe** SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`: all four Stage26 clearer signatures match, helper entry RVA0x97870 matches. E8 CALL at RVA0x979CC targets RVA0x97870, return0x979D1; E8 CALL at RVA0x97609 targets **RVA0x97970**, return0x9760E. This **corrects the previous Stage27 assumption** that RVA0x97609 called 0x97870 directly. The original Stage27 dynamic stack observations are still 34 (return0x979D1) +20 (return0x9760E) on the primary clearer, but **the transfer from 0x97970 into 0x97870 is unverified**.
- Additional static decoding from uploaded code bytes: `0x97870` AND clears bits0,2,3 at `[player+0xF8]`, writes `[player+0x104]=2`, calls 0xF7EB0,0x976D0,0x99150 and ORs `[player+0xF4]` with 0x400. `0x975A6` path requires `WORD [player+0x3A34]==1` and bit0 set, masks bit0 clear, returns. `0x95F55` and `0x97031` are parts of multi-field reset routines. These operations do not establish gameplay meaning.
- **Next user's concrete command** from `PS D:\INCUBATOR>`:
  `Invoke-WebRequest -Uri "https://raw.githubusercontent.com/Hawky11/1/stage27-verified-static-callgraph/Stage27_VerifiedStatic/tools/inspect_call_flow.py" -OutFile ".\inspect_call_flow.py"`
  `py -3 .\inspect_call_flow.py --binary "G:\Arcana Heart 3 - LS XTEND\AALib.exe" --output ".\Stage27_CallFlow_97970.txt"`
  Then upload `D:\INCUBATOR\Stage27_CallFlow_97970.txt` only. This logs RVA-labeled 0x97970 window and both actual caller CALL signatures without game hooks. Optional Capstone for live disassembly, but raw bytes suffice for offline objdump. No new MSVC build, source C++ patch or COM battle needed.
- Next assistant session: study uploaded 0x97970 bytes for control-flow transfer(s) to 0x97870, relate 34+20 dynamic stack groups, classify flags and branch guards, preserve historical branches, append new evidence in a new branch. The new inspector's 5 tests passed locally against **synthetic** PE/call data; the tool's original-game execution remains pending.
- Prior Stage26 runtime source/evidence remain `a249f452353c340fe8f1dab23375a8a932b079b4` / `4b7564ff71664fedd5745e3bcfc4210d74540075`; original Stage27 analysis remains `b490f87892791d45f0b0841a47e70d60329ec457`. No rewriting.

---


## Previous handoff — Stage27 offline analysis complete; verify static CALL chain next (2026-11-09)

- **Newest branch** [stage27-bit0-lifecycle-callers](https://github.com/Hawky11/1/tree/stage27-bit0-lifecycle-callers), HEAD `b490f87892791d45f0b0841a47e70d60329ec457`. Parent: Stage26 runtime evidence HEAD `4b7564ff71664fedd5745e3bcfc4210d74540075`. Contains `Stage27_Analysis/STAGE27_LIFECYCLE_ANALYSIS.md`, `Stage27_Analysis/README_STAGE27.md`, `Stage27_Analysis/cycles_stage26.csv`, `Stage27_Analysis/stage27_summary.json`, `Stage27_Analysis/tools/analyze_bit0_lifecycle.py`, `Stage27_Analysis/tests/test_stage27.py`. Stage27 is **analysis-only**, no new C++ source, no in-game hook or build.
- Stage26 original source is `stage26-object-bit0-write-trace` HEAD `a249f452353c340fe8f1dab23375a8a932b079b4`; complete successful MSVC/runtime evidence remains immutable at `stage26-object-bit0-write-runtime-evidence` HEAD `4b7564ff71664fedd5745e3bcfc4210d74540075`.
- From 140 Stage26 events: 70 per-side exact set→clear cycles, no open/unpaired/repeated setter. 54 clears by instruction RVA `0x97873` in apparent helper entry `0x97870`: `push edi; mov edi,eax; and dword ptr [edi+0xC8],0xFFFFFFF2`. Since saved old EDI is at stack[0], stack[1] appears to be return address. **Two dynamic caller-return groups:** `0x979D1` 34 (P1 16/P2 18) and `0x9760E` 20 (P1 12/P2 8); all 34 first-group samples also show `0x9760E` deeper at stack[4]. Candidate E8 CALL starting RVAs `0x979CC` and `0x97609`, still **unverified** without exact PE.
- Other clears: RVA`0x975A6` 7 P1, RVA`0x95F55` 8 P1/P2, RVA`0x97031` 1 P1. Observed setter post-EIP in Stage25 DLL wrapper `0x50B0C267`, original AALib writer RVA`0xF58C9`. EBX varied; EBX0x1E precisely two setter records seq1/seq87, both P2 source, P1 target and return path `0x979D1`. EBX not independently proved payload in all contexts.
- **Next concrete investigation (optional binary):** run standard-lib script on exact local AALib.exe to verify recorded SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, four writer bytes, helper entry and relative E8 call sites. Example commands:
  `py -3 Stage27_Analysis\tools\analyze_bit0_lifecycle.py --log ArcanaAI_Stage26ObjectBit0WriteTrace.log --binary "PATH\TO\AALib.exe" --static-report Stage27_AALib_StaticReport.txt`
  `py -3 -m unittest discover -s Stage27_Analysis\tests -p test_stage27.py -v`.
  Share only the resulting **text report**, not copyrighted game EXE, and keep it in a new evidence branch rather than overwriting previous ones.
- Validated Stage27 archived CSV/summary against Stage26 140 input records and 70 cycles using independent same-session JS analysis; **Python unit tests not yet executed**. If the binary validates candidate calls, investigate preceding branch conditions/call graph to understand *why* bit0 clears; don't equate bit0 to event suppression without evidence. Stage28 hooks are not yet warranted or implemented. Preserve all Stage25–Stage27 source/evidence histories.

---


## Previous handoff — Stage26 verified; Stage27 investigation next (2026-11-09)

- **Latest completed stage** Stage26: [source branch](https://github.com/Hawky11/1/tree/stage26-object-bit0-write-trace) `a249f452353c340fe8f1dab23375a8a932b079b4`; [full runtime evidence](https://github.com/Hawky11/1/tree/stage26-object-bit0-write-runtime-evidence) `4b7564ff71664fedd5745e3bcfc4210d74540075`, including exact 21 uploaded log blobs, chat Build.log text and `Stage26_Overlay/STAGE26_RUNTIME_ANALYSIS.md`. Main upload commit `8c6edc82d3146c11b3dd23d964471c31ba3fe816`. **Do not rewrite source/evidence branches.**
- Build: 2026-11-09 17:27:39 MSVC x86 14.51.36231, 8 files compiled, linked `dinput8.dll` 240,640 bytes, C4456 in new `AITrace.cpp:1671` due to shadowed `reported` local, existing C4754 in `AICommandSiteTrace.cpp:785`. Both non-fatal.
- Stage26 marker/config present. Both startup P1/P2+`0xF8` zero, DR7=`0x00DD0005`, Stage16 disabled, 59 thread watchpoint installations. 140 unique write records in game thread18044: P1 41 sets and 41 clears; P2 29 sets and 29 clears, all paired 0→1 and 1→0 transitions.
- **Original setter identified:** all 70 monitored 0→1 writes originate from Stage25 DLL wrapper EIP `0x50B0C267` of original AALib `OR [EAX+0xF8],1` RVA `0xF58C9`. **Clearers:** post-EIP RVAs `0x9787A` (54), `0x975AC` (7), `0x95F5B` (8), `0x97037` (1). Exact instruction starts and operations in `STAGE26_RUNTIME_ANALYSIS.md`. Registers holding object+0x30 allow [+0xC8] to address object+0xF8.
- Type1/payload0x1E from P2 reached ALT_CALLSITE, queue, dispatcher twice; selector31 and arg3=1 select target P1; each Stage26 0→1 trap occurs 222/197 QPC ticks BEFORE Stage25 logging and afterward the game clears P1 bit0 at post-RVA `0x9787A`. Stage24 output mask `0x10001`, Stage21 AL0, fallback slot3=0→0, Stage22 bit400=0→0.
- **Big logs read completely:** Stage14 producer 9,730 sequence IDs, Stage19 state map 2,266 IDs, unique and gapless, newline terminated, no explicit error/drop/overflow. Old `ArcanaAI_PlayerBit17WriteTrace.log` SHA `befd639d44731fd9cb5b35de44d49878406d716c` is historical Stage25, not a Stage26 write trace.
- **Next task — Stage27 static origin/lifecycle work:** examine possible clear-parent control flow around instructions `AALib RVA 0x97873`, `0x975A6`, `0x95F55`, `0x97031` and callers, distinguish natural object/action lifecycle from speculation. Existing 24-byte Stage26 code windows and saved registers/stack provide initial evidence; full fingerprinted AALib.exe was not stored in Git source, so request or retrieve exact binary before asserting full xrefs. Only create new passive observer if a specific unproven branch/caller question remains. If making source revision, preserve Stage26 tested branch, fix C4456 by renaming inner `reported`, add explicit new-stage marker, and require new MSVC/runtime validation.
- Project objective is to understand COM-vs-COM type1/payload0x1E queue suppression, not merely to label bit0. Existing Stage15/16 results concerning bit17 remain relevant and separate.

---


## Historical handoff — Stage26 source ready; subsequently tested

- Stage26 source branch: [stage26-object-bit0-write-trace](https://github.com/Hawky11/1/tree/stage26-object-bit0-write-trace), HEAD `a249f452353c340fe8f1dab23375a8a932b079b4`. Source snapshot includes `Stage26_Overlay/src/AITrace.cpp`, `Stage26_Overlay/ArcanaAI.ini`, `Stage26_Overlay/README_STAGE26.md`, `Stage26_Overlay/STAGE26_DESIGN.md` and `Stage26_Overlay/tests/verify_stage26.py`.
- Base is immutable Stage25 source `3f034550f9d1254cc136452dde363657f913a91c`. Latest proven runtime evidence is Stage25 commit `b426a8f74764b7606ce3b5da075bb3a61d3698b3`, no rewrite.
- **What to apply:** replace only working `src/AITrace.cpp` and `ArcanaAI.ini`; keep the previously compiled Stage25 `src/AIQueueTrace.cpp`. Run existing x86 `Build.bat`. New log expected: `ArcanaAI_Stage26ObjectBit0WriteTrace.log`; marker `STAGE26_OBJECT_BIT0_WRITE_BUILD=2026-11-09-r1`.
- Correct INI: `EnableStage26ObjectBit0WriteTrace=1`, `Stage26TransitionsOnly=1`, `EnablePlayerBit17WriteTrace=0`, `EnableControlFieldTrace=0`, Stage25 OR remains 1. These are **mutually exclusive DR0/DR1 modes** and code refuses conflicts.
- Mechanism: inherit Stage16's write-only hardware watchpoints, thread scanning and exception handler; target `player+0xF8`, mask0x1, P1 watched RVA `0x0DCA2070`, P2 `0x0DCCCD98`. Preserve DR2/DR3 and previous owners. Capture initial DWORDs and post-write EIP/RVA, QPC, old/new shadow bit0, registers, ESP stack snapshot, code bytes; suppressed no-change events periodically counted. This has not been MSVC-compiled or runtime-tested.
- **Review caveats:** Stage25 wrapper performs the OR inside the DLL, so a hardware post-write EIP may point into the DLL rather than AALib. Initial bit0 could already be 1 before DR watchpoints are installed. OldValue is a best-effort shadow and not proof that every thread was observed. Static writer xref analysis of *all* sites was not performed: complete AALib.exe binary is not in the Git repo. Do not attribute first setter without 0→1 evidence.
- **Next action:** inspect user's Build.log (warning C4754 at AICommandSiteTrace.cpp:785 was pre-existing), then new Stage26 marker/installation, initial P1/P2 bit0, write transitions, dropped/overflow and QPC correlations with Stage25. If MSVC compilation fails, fix only the new branch with a new revision. Preserve Stage25 evidence and append stage results to PROJECT_HISTORY/README/NEXT_SESSION.

---


## Prior handoff — Stage25 runtime complete, plan Stage26 (2026-11-09)

- Latest **build/runtime-tested** observer: Stage25 source [stage25-secondary-object-flag-trace](https://github.com/Hawky11/1/tree/stage25-secondary-object-flag-trace), source HEAD `3f034550f9d1254cc136452dde363657f913a91c`. Successful chat Build.log on 2026-11-09 16:50:57: MSVC x86, DLL 237,568 bytes, C4754 warning only.
- New preserved evidence: [stage25-secondary-object-flag-runtime-evidence](https://github.com/Hawky11/1/tree/stage25-secondary-object-flag-runtime-evidence), HEAD `b426a8f74764b7606ce3b5da075bb3a61d3698b`. Includes original Git blobs for all 20 uploaded logs, complete analysis, normalized Build.log transcript from chat (not a separately uploaded Build.log). Keep the source and evidence branches unchanged.
- Stage25 installer marker at 16:51:44 `STAGE25_SECONDARY_OBJECT_FLAG_BUILD=2026-11-09-r1`; the observer installed at `0xF58C9`. One target event recorded 16:52:07, source P1 `0x0E251F78`, target P2 `0x0E27CCA0`, field P2+`0xF8=0x0E27CD98` before/after `0x1→0x1`; selector31, args3/4 `0/0`, both field reads successful.
- Same P1/type1/payload0x1E dispatch sequence: ALT `292602968311`, queue `292602968399`, Stage23 resolver `292602968420`, **Stage25 OR `292602971617`**, Stage24 consumer completion `292602973136`, Stage21 classifier `292602975570` (AL0), fallback `292602976845` (slot3 0→0), Stage22 AND `292602977325` (bit400 0→0), dispatcher finish `292602977883`. Correct ordering: Stage25 happens *inside* Stage24 helper.
- Full Stage16 write trace: 24,176 records, no duplicate/missing sequence IDs, P1 bit17 set/clear and P2 bit17 set/clear. Full Stage14 producer trace: 11,383 records without duplicate/missing IDs; 10 Stage15 branch samples, including accepted P1 bit17=0 case. No explicit ERROR/WARNING/DROPPED/OVERFLOW in either large log.
- The result **does not** establish who set P2 `+0xF8 bit0` before the OR, nor the gameplay meaning. Do not confuse `+0xF8 bit0` with `+0x3B84 bit17`.

### Next task — design Stage26 bit0 writer/lifetime observation

1. Inspect the fingerprinted AALib.exe for direct/indirect writers to the P1/P2 object field `+0xF8`, and inspect current Stage16 hardware watchpoint design before choosing instrumentation.
2. If a source change is warranted, create a new *source* branch based on preserved Stage25 commit; use passive diagnostics only. Capture the initial P1/P2 `+0xF8` values and observed writer EIP/RVA, thread, QPC, old/new DWORD, bit0 transitions and call attribution for the first set and later clears. Avoid perturbing game writes/flags.
3. Stage16 reserves DR0/DR1; do not overwrite existing breakpoint owners. DR2/DR3 may also be owned: check per thread and preserve all original debug state; disabling other probes may be safer than unsound concurrent instrumentation.
4. Use an unmistakable Stage26 marker, unique log, source/INI validation and reproducible assembly tests. Runtime observations require user MSVC Build.log and in-game marker/hook installation. At least one event where bit0 transitions `0→1` is necessary before attributing the initial setter; a single `1→1` OR is insufficient.
5. Preserve all Stage21–Stage25 source/evidence snapshots. Append Stage26 results to `PROJECT_HISTORY.md`, update `NEXT_SESSION.md` and README after completion.

---


## Previous checkpoint — Stage25 prepared, Stage24 follow-up complete

- Source-only Stage25: [stage25-secondary-object-flag-trace](https://github.com/Hawky11/1/tree/stage25-secondary-object-flag-trace), commit `3f034550f9d1254cc136452dde363657f913a91c`. Apply `Stage25_Overlay/src/AIQueueTrace.cpp` and `Stage25_Overlay/ArcanaAI.ini` to the working Stage24 project. No project-file change.
- [Follow-up analysis](https://github.com/Hawky11/1/blob/stage25-secondary-object-flag-trace/Stage25_Overlay/STAGE24_FOLLOWUP_AND_STAGE25_STATIC.md) completes the two previously unread large Stage24 logs and corrects the argument dataflow using the fingerprinted EXE.
- Both large files fully retrieved and exact Git blob hashes verified. Write trace:21886 records, no missing/duplicate sequence IDs, blocks can appear out of order. Producer trace:10463 ordered records without gaps. No explicit errors/drops/overflow markers.
- P2 setter->bit17 OR confirmed: Stage17 QPC0x41E9BA1075 -> write seq15206 QPC0x41E9BA11CE, delta345. Clear seq15215 at ED19F uses **0xFFF9FFFF**, not the previously stated0xFFFDFFFF; it clears bits17/18, with observed0x30000->0x10000.
- **Argument issue resolved:** EE2F0 reloads ECX from player+3F34 (arg4); EE2F6 reloads EDX from player+3F30 (arg3). Resolver bytes3/18 are not the final arguments. Zero/zero in Stage24 is correct for those fields.
- **Secondary-path correction:** F58C1 calls9EF00(primary arg3,-1), not arg4. With arg3=0 it selects slot1; Stage24 returned EAX matches P2. Stage25 observes the original OR[target+F8],1 at F58C9 directly.
- Stage25 log:`ArcanaAI_Stage25SecondaryObjectFlagTrace.log`; marker:`STAGE25_SECONDARY_OBJECT_FLAG_BUILD=2026-11-09-r1`. Records source/target player, selector/args, before/after field and QPC. Original OR executes once; no extra game-state writes.
- Validation: exact PE SHA256 and patch signatures; extracted x86 wrapper assembled and compared with original OR in128 randomized Unicorn cases (registers, ESP, EFLAGS, output, logger context). **Not compiled with MSVC and not tested in-game yet.**
- Next: inspect user's new Build.log, then Stage25 marker/hook installation and records. Correlate Stage25 QPC between Stage23 and Stage24 records. Establish whether target bit0 transitions0->1 or is already1; gameplay semantics remain unknown.
- Stage24 evidence remains immutable at `983a7f4f205c99cd1c8f44814c58cc09ed76f0c8`. Its initial transport limitation is superseded by this follow-up; no old logs or historical source branches were rewritten.


---

The checkpoint sections below predate this Stage25 handoff and are retained as historical context.


**Read this file first in a new chat.** Inspect current GitHub branch HEADs and newly uploaded files before asking the user for anything.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- **Latest tested source:** [stage23-resolver-result-trace](https://github.com/Hawky11/1/tree/stage23-resolver-result-trace), HEAD `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`.
- **Latest runtime evidence:** [stage23-resolver-result-runtime-evidence-run2](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence-run2), HEAD `0e6ed619a4e812687e615b00535ef17e54118b5b`. This branch stores the second Stage23 r2 run's exact 18 runtime logs, pasted Build.log, and `Stage23_Overlay/STAGE23_RUNTIME_ANALYSIS_RUN2.md`. The first Stage23 r2 capture remains preserved at [stage23-resolver-result-runtime-evidence](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence), HEAD `306b006a2230c1226a49ab93b9b8b74722fd1683`.
- **Latest build:** 2026-11-09 09:04:32, Visual Studio 18 Community / MSVC 14.51.36231 x86. All eight C++ translation units compiled; `build\\dinput8.dll` linked successfully at **229,376 bytes**. Only diagnostic: non-fatal C4754 at `AICommandSiteTrace.cpp:785`.
- The first Stage23 r2 compile attempt failed at two pointer casts in `AIQueueTrace.cpp`; r2 fixed both with `reinterpret_cast<std::intptr_t>(resolver)`. The successful r2 binary has been built and tested in two separate captures.
- **Important stage identification:** the new 09:06 upload is still a Stage23 r2 run, **not Stage24**. It contains marker `STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r2`, Stage23 resolver installation at `0x000EE273 -> 0x0012EAC0`, and the header `Stage 12/21/22/23 tracing requested`. There is no Stage24 marker/log and no installation at CALL RVA `0x000EE306`.
- **Stage23 runtime:** three type-1 / payload-`0x1E` resolver records were captured, all readable and all on thread 16252:
  | Side | Tick | Resolver QPC | Args (arg1,arg2,arg3,arg4) | Record pointer |
  |---|---:|---:|---|---|
  | P2 | 1331046 | 13310813095 | `(0x17,0x17,0x1E,0x38)` | `0x05C8DCA0` |
  | P1 | 1332312 | 13323424234 | `(0x0A,0x0A,0x1E,0x26)` | `0x0586F490` |
  | P1 | 1350156 | 13501860595 | `(0x0A,0x0A,0x1E,0x26)` | `0x0586F490` |
- All three resolver records again contain `60 FF 03 1F 12 00 00 05 13 24 03 00`; byte +3 is selector `0x1F`. All correlate to ALT_CALLSITE RVA `0xF6AD7`, accepted insertion at `0xF6ADC`, EventDispatch at `0xEE5CB`, Stage21 classifier/fallback and Stage22 post-AND.
- For each of the three `0x1E` events, classifier helper `0xE8CF0` returned AL=0, fallback helper `0xF7EB0` ran with index 3, and the Stage22 post-AND observer ran. Slot 3 stayed `0 -> 0`; bit `0x400` stayed `0 -> 0`. No nonzero-to-zero fallback clear was captured.
- **New Stage18/20/17 sample:** eight Stage18 gate records and eight Stage20 object-guard records (all P2) correlate sequentially 8/8 by side/player/thread/tick/stack/QPC. Each object check followed its gate by 121–175 QPC ticks. All eight gate results were AL=1, key `0x19`, table byte `0xFA`. First six object helper results were AL=1 with object DWORD `0x4C`; last two were AL=0 with object DWORD `0x58` (bit2 clear), both at tick 1353484.
- Those last two AL=0 guards match two Stage17 setter calls at CALL RVA `0xC6502`, both index `0x71`, same P2 player and same EBP/stack frames. Guard #7 QPC `0x326C2BD8E` → setter #1 `0x326C2BDD6` = 72 ticks; guard #8 `0x326C2C03B` → setter #2 `0x326C2C081` = 70 ticks. The `player+0x3B84` snapshots show 0 at the first pair and `0x20000` at the second pair. This is strong correlation consistent with the first call setting bit17, but the exact store is not directly confirmed: `ArcanaAI_PlayerBit17WriteTrace.log` is empty.
- Empty logs: `ArcanaAI_PlayerBit17WriteTrace.log`, `ArcanaAI_StateMapTrace.log`, `ArcanaAI_ProducerCallTrace.log`. `FinalCommandTrace` has 45 records but no payload-`0x1E` at monitored RVA `0xF7C5D`; this does not contradict the separately confirmed `0xF6AD7` route. No ERROR/WARNING/DROP/OVERFLOW/FAILED markers were found in nonempty uploaded logs.
- Stage21 `preCallEdi` still equals the controller-array pointer, not resolver byte +3. Retain it as raw EDI only.

## Next task — Stage24 resolver consumer observer

Stage23 static analysis and both runtime runs establish resolver byte +3 = `0x1F`. Static disassembly shows this byte is passed as argument 2 to helper `0x004F57D0` at CALL RVA `0x000EE306`. The helper treats it as a selector into a three-byte mapping table at `0x04DDF188 + selector*3`; it builds a three-DWORD output structure at `player+0x3A08`. The three table bytes are one flags byte and two mapped selectors. High-level semantics such as attack/action/command/animation are not established.

### Stage24 implementation target

Create a new source branch from the unchanged Stage23 r2 source HEAD `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`. Do not modify either Stage23 source branch or either Stage23 runtime-evidence branch. Add a passive observational wrapper at CALL RVA `0x000EE306` -> helper RVA `0x000F57D0`, validating exact callsite bytes/context and helper prologue against the same AALib.exe fingerprint.

Capture:
1. The four original helper arguments; expected target values are output pointer `player+0x3A08`, selector `0x1F`, arg3 `0x03`, arg4 `0x12`.
2. The three runtime mapping bytes at `0x04DDF188 + selector*3` immediately before the helper call.
3. The three output DWORDs at `player+0x3A08` immediately before and after the original helper call.
4. Whether table flags select an initialization/secondary-helper path; first inspect the primary helper's control flow and do not add another intrusive hook unless necessary.

Replay the original args exactly once; preserve original returned registers/EFLAGS, caller-owned stack and cleanup. Diagnostics must be read-only; never write the table or output structure. Correlate Stage24 QPC with Stage23 resolver, Stage21 fallback and Stage22 post-AND for the same side/player/thread/tick.

**Make Stage24 unmistakable:** add `STAGE24_RESOLVER_CONSUMER_BUILD=...` and a dedicated `ArcanaAI_Stage24ResolverConsumerTrace.log`. Do not reuse Stage23's marker or filename. A successful compile alone is not runtime evidence; require both Build.log and the Stage24 runtime marker/installation line.

## Preservation rules

- Keep Stage21 r1/r2, Stage22 source/runtime evidence, Stage23 source, and both Stage23 runtime captures immutable.
- A source branch must not be described as tested until a successful Build.log and runtime marker/hook-installation log both exist.
- A completed run must preserve the exact source/INI, Build.log, all uploaded logs and concise runtime analysis in its evidence branch.
- Keep `main` as documentation index and append-only history; update README, NEXT_SESSION and PROJECT_HISTORY after each completed stage.
- Distinguish confirmed calls/reads/branches from semantic interpretations that remain hypotheses.

## Historical build/runtime details

The stage-specific records below are retained for chronology. The current Stage23 resolver-result build/runtime task is in the action list above.

## Latest build result — successful

The user supplied Build.log dated 2026-11-08 16:32:02. VS 2026 18.9.1 / MSVC 14.51.36231 x86 compiled all eight C++ translation units and linked `build\\dinput8.dll`, size **198,144 bytes**. One non-fatal C4754 warning remains in `AICommandSiteTrace.cpp` lines 782–785. Build.log was pasted in chat, not uploaded in the branch tree.

## Stage17 runtime result — caller attribution confirmed for this run

Runtime run: 2026-11-08, initialization at 16:34:05, relevant events 16:34:29–16:34:44.

- Stage17 r2 marker, worker entry, and INI settings are present.
- The passive observer installed successfully at callsite RVA `0x000C6502`, return RVA `0x000C6507`, original helper RVA `0x0000FAA0`; `suspended=34`.
- `ArcanaAI_Bit113SetterTrace.log` contains nine `BIT113_SET_CALL` records; every record uses `index=0x00000071`, the exact call/return RVAs, and the correct P1/P2 pointer.
- Stage16 PlayerBit17WriteTrace contains 20,456 records and eight actual bit17 transitions (four sets and four clears). There are 62 thread-watchpoint installations across 10 batches, with no warning/error/drop/skip lines.
- All four Stage16 0→1 writes at RVA `0x0000FAD2` have a preceding Stage17 call from `0x000C6502` with the same player pointer and bit index. QPC deltas are 367, 520, 396, and 353 ticks. The four corresponding clears again execute `0x000ED19F` (post-EIP `0x000ED1A9`).
- Five additional Stage17 calls occurred while the bit was already set; they did not create extra transitions.
- Four Stage15 F6A68 events all saw bit17=1 and the original JZ was not taken. No F6A9D/F6AAC/F6AD7 call was recorded; QueueTrace and EventDispatchTrace contain no type-1 / payload-`0x1E` insertion/dispatch in this run. This is a suppression-side sample, not a successful queue insertion sample.
- No warning/error/drop reports were found in the reviewed Stage17, Stage16, producer, queue, dispatch, or alternate-callsite logs.

**Conclusion:** the candidate direct caller at `AALib.exe+0xC6502` is runtime-confirmed as the origin of all four bit17 set transitions observed in this run. This does not prove it is the only possible setter path for every game state. Preserve the current evidence branch unchanged.

## Stage18 runtime result — complete for the tested scenario

- Latest runtime evidence branch: [stage18-bit113-gate-runtime-evidence](https://github.com/Hawky11/1/tree/stage18-bit113-gate-runtime-evidence), HEAD 0139152b9eeb778a23ec4f3197c57faff749a78a. This branch combines the user's 2026-11-08 18:00 uploaded runtime logs with the Stage18 overlay and Stage18_Overlay/STAGE18_RUNTIME_ANALYSIS.md.
- Source snapshot branch: [stage18-bit113-gate-condition-trace](https://github.com/Hawky11/1/tree/stage18-bit113-gate-condition-trace), HEAD 520eec4cd216795e8ffbaa8738b3602b5e40d39a. Keep it as the source-stage snapshot.
- Runtime start: 18:00:15. Stage18 marker/config and installation at RVA 0xC64A5 are present; Stage17 installation at RVA 0xC6502 is also confirmed. Both hooks report 44 suspended threads.
- **Build.log was not uploaded.** Runtime markers and installation messages prove the loaded DLL ran both hooks, but compiler/linker diagnostics and final DLL size are unavailable.

### Confirmed Stage18 findings

- Six Stage18 gate records; all returned AL=1 and the safely-read post-return byte was 0xFA. The unique state/table cases were P1 index 0x0E / selector 0x0A, P2 index 0x19 / selector 0x17, and P1 index 0x18 / selector 0x0A. No AL=0 sample was captured.
- Stage17 recorded six calls from 0xC6502, all passing bit index 0x71. Stage16 recorded 15,094 write records and three 0→1 bit17 transitions plus three clears.
- Each episode has two calls into caller function 0x4C63B0. One arrives directly from 0x4967FC (return 0x496801); the other is reached via indirect CALL EDX at 0x4CB9F8 (return 0x4CB9FA). Static table entry 0xA at 0x428AF18 points to 0x4C63B0, explaining the second invocation through callback/function-table dispatch.
- The first setter call in each episode sets bit17; the second repeats the OR when it is already set. QPC deltas are in PROJECT_HISTORY.md and STAGE18_RUNTIME_ANALYSIS.md.
- Three target Stage15 attempts had EDI=0x1E, bit17=1, ZF=1 and branchTaken=0. No F6A9D/F6AAC or F6AD7 invocation followed. QueueTrace/EventDispatchTrace/FinalCommandTrace each show 27 type-1 records for other payloads and none for 0x1E.

## Stage19 runtime result — mapping confirmed (2026-11-08)

Evidence branch: [stage19-bit113-state-map-runtime-evidence](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-runtime-evidence), HEAD `3e75c07c7bdbd0827112f6a488936587a41a1b4b`. Source branch remains [stage19-bit113-state-map-trace](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-trace), HEAD `841142895a976419a9bda197be286e7434993067`. The uploaded logs were placed in root `main`; the evidence branch preserves them with the exact Stage19 overlay and `Stage19_Overlay/STAGE19_RUNTIME_ANALYSIS.md`.

### Installation and limitations

- Run initialized 2026-11-08 at 19:05:37.
- Stage17 hook at RVA `0x000C6502`, Stage18 hook at RVA `0x000C64A5`, and Stage19 hook at RVA `0x000F6C08) all installed successfully; each reported 44 suspended threads.
- Stage19 marker/config show `STAGE19_BIT113_STATE_MAP_BUILD=2026-11-08-r1` and `EnableStage19StateMapTrace=1`.
- **Build.log was not uploaded.** Runtime proves a DLL containing the Stage19 instrumentation loaded and its hook installed, but compiler/linker output and final DLL size are unavailable.

### Confirmed Stage19 mapping results

There are 1,235 `STATE_MAP_CALL` records, all with `readMask=0x0F`, and no Stage19 drop/warning/error.

| Side | Raw code | Helper EAX | WORD flags | Derived state index | Calls |
|---|---:|---:|---:|---:|---:|
| P1 | `0x20` | 0 | `0x0400` | `0x18` | 200 |
| P1 | `0x22` | 0 | `0x0400` | `0x18` | 190 |
| P1 | `0x24` | 0 | `0x0400` | `0x18` | 130 |
| P1 | `0x29` | 0 | `0x0400` | `0x18` | 12 |
| P1 | `0x4A` | 3 | `0x2000` | `0x1B` | 152 |
| P2 | `0x58` | 1 | `0x0802` | `0x19` | 82 |
| P2 | `0x59` | 1 | `0x0982` | `0x19` | 297 |
| P2 | `0x5C` | 1 | `0x0802` | `0x19` | 56 |
| P2 | `0x5E` | 3 | `0x2002` | `0x1B` | 116 |

Helper EAX totals: 532×0, 435×1, 268×3. The observed WORD flags agree with the helper priority checks. QPC correlation with Stage18 confirms raw `0x59→EAX 1→index 0x19`, raw `0x22→0→0x18`, and raw `0x20→0→0x18` in the matching gate calls. Indexes generated by raw codes `0x4A` and `0x5E` did not appear in Stage18's bit-113 gate records in this run; do not assume every map-helper call proceeds through that caller.

### Stage18 / Stage17 / Stage16 / Stage15 correlation

- Stage18 logged 42 gate results: all `AL=1`, all post-return table-byte readbacks `0xFA`. No `AL=0) sample.
- Parent-return attribution: 38 direct-call path entries (`0x00DB6801`) and four callback path entries (`0x00DEB9FA`).
- Stage17 logged eight bit-113 setter calls (two per episode).
- Stage16 logged 17,361 writes, including exactly four bit17 `0→1` writes at RVA `0x0000FAD2) and four clears at RVA `0x000ED1A9).
- Stage15 captured four payload-`0x1E` attempts. Every `F6A68` saw bit17=1 and `branchTaken=0`; no `F6A9D`, `F6AAC), or `F6AD7` invocation followed. QueueTrace, EventDispatchTrace and FinalCommandTrace contain no payload-`0x1E) insertion/dispatch.
- Most important new observation: 31 P2 gate calls at state index `0x19) returned `AL=1) but were not followed by setter calls. Therefore the table gate is not sufficient to guarantee the setter is reached.

No warning/error/drop was found in the reviewed Stage19, Stage18, Stage17, Stage16, Stage15, Queue, Dispatch, FinalCommand or AlternateCall logs.

## Stage20 runtime result — confirmed

Evidence branch: [stage20-bit113-object-bit2-runtime-evidence](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-runtime-evidence). Full analysis: `Stage20_Overlay/STAGE20_RUNTIME_ANALYSIS.md`. Source: [stage20-bit113-object-bit2-guard-trace](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-guard-trace), HEAD `11dde281a08a3481517d0cb60d4752393fa0e578`.

Run began 2026-11-08 19:58:47. Stage20 marker/config were present. The observational wrapper installed at CALL RVA `0x000C64F1`, original helper `0x00095B00`, resume `0x000C64F6`, `suspended=40`; the original TEST/JNE remained untouched.

- 33 Stage20 object-guard records, all P2, matched all 33 Stage18 gate records by side/player/QPC; each Stage20 QPC followed its gate by 129–173 ticks.
- All 33 actual helper results were `AL=1`, with checked `[object+0x3B70]=0x4C` and bit `0x4` set; all read masks `0x3F`.
- Stage18 table gate records were all `AL=1`, key `0x19`; their separate `stateIndex` distribution was 0A (4), 04 (2), 14 (25), 10 (2). Do not conflate table key with `stateIndex`.
- Stage17 passive setter observer installed at `0xC6502` but logged zero `BIT113_SET_CALL` records. Stage16 captured 28,043 writes, all `oldBit17=0` / `newBit17=0`; no bit17 transitions.
- Stage19 map trace captured 2,974 records, but its map index must remain separate from Stage18's table key/stateIndex unless same-event correlation is proven.
- Three alternate callsite invocations at `0xF6AD7` carried payload `0x1E` (P2 once, P1 twice). Each was accepted into its CPU queue as type 1/payload `0x1E`, then observed by EventDispatchTrace.
- In all three alternate episodes bit17 was zero and the F6A68 branch was taken; F6AD7 still followed 620–851 QPC ticks later. Thus bit17=1 is not required for this observed alternate queue/dispatch path.
- FinalCommandTrace logged 60 records but no payload `0x1E` at its monitored callsite `0xF7C5D`; this does not negate the separate queue/dispatch evidence.
- No explicit error/warning/drop/failure records were found in the reviewed logs. The new runtime upload did not include a Build.log file; the successful build transcript was supplied in chat before the test.

### Current task — Stage21 follow-up

Stage21 r1 now confirms the payload-`0x1E` classifier fallback for P1/P2 and the complete P1 object-guard AL=0 → setter index 0x71 → bit17 0→1 write chain, followed by a later bit17 clear. Remaining runtime gaps are a nonzero slot-3 / set controller-bit-0x400 fallback sample and same-event Stage18 gate correlation. The r1 decoded-action field is invalid; use the corrected, unbuilt Stage21 r2 branch for a later capture if that field is required. Preserve r1 evidence and all Stage12–Stage20 historical branches unchanged.

## Historical branch rules

- Do not rewrite Stage12–Stage16 evidence branches.
- Keep `stage17-bit113-setter-caller-trace` and `stage17-bit113-setter-callsite-trace` distinct; they use different Stage17 hook placements.
- Append each completed analysis to `PROJECT_HISTORY.md`, update this handoff, and keep `README.md` as the entry point.
- Distinguish successful compilation from hook installation; only runtime log evidence proves the latter.
