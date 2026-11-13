# ArcanaAIOverhaul — Stage156 native entrypoints, raw callers and callback lifecycle (2026-11-13)

**SOURCE_ONLY. No game DLL patch, live game process access, injection, unload, or installed native hook work.** `SAFE_TO_INSTALL=false`; `SAFE_TO_UNLOAD_GAME_DLL=false`; `Stage36 OFF`; `origin(0x27)=UNKNOWN`.

## Baseline and provenance

- Verified the previous [Stage153](STAGE153_CHECKPOINT.md), [Stage154](STAGE154_CHECKPOINT.md), and [Stage155](STAGE155_CHECKPOINT.md) repository records in this session; Stage155 user-confirmed Windows x86 isolated self-owned fixture passed **5,040 synthetic interleavings / 18 of 18 special tests**. The UNKNOWN raw-caller mode denied all 2,520 schedules, including **1,020 false-empty-counter cases**; no production game safety established.
- Prior Stage146 user source snapshot was pinned as `ArcanaAI_Stage146_Evidence_493fc20dff504bdabc3abc4924a9426f.zip`, SHA256 `6d9c39d59b4883dab6cc4ea95f08deac56fe8c477fd6b4746bd1c764f96a207b`. Previous stages checked **85/85 source hashes** and **50/50 source anchors** through Stage155. **This Stage156 session did not rematerialize/re-extract that ZIP or scan the user's current Windows project.** Treat every source coordinate below as an earlier verified snapshot anchor, not a newly measured live address.
- A chat-only Stage156 read-only source audit package was generated: `ArcanaAI_Stage156_Entrypoints_RawCallers_CallbackLifecycle_SOURCE_ONLY.zip`, **15,028 bytes**, SHA256 `63ed358cc4cfe23469f7e2f3feec281bc18c236491ae42436cae3843011e86ad`, **6 entries**, ZIP CRC and internal SHA256 manifest **5/5 PASS**. It contains `stage156_source_audit.py`, a PowerShell runner, **7 synthetic Python tests**, and a detailed Russian audit; no private game sources or binaries.
- The offline test command `python -m unittest discover -s Stage156_SourceOnly/tests -p test_stage156.py -v` passed **7/7 on Linux/Python 3.13**; no Stage156 Windows native/actual-user-tree run claimed.

## Source-level ingress map (NOT installed ownership)

| Source surface | Previous source anchors | Admission / lifetime gap |
|---|---|---|
| Export, loader | `src/Proxy.cpp:58–90` `DirectInput8Create`; `:92–100` `DllMain` | Export gate does not cover other raw native paths; attach-only DllMain, no safe unload coordinator |
| Stage10 raw parent | `src/AIProducerContextTrace.cpp:161–165,838–852`; RVA **F34D0/5 bytes**, **F3930/6 bytes** | Existing declared naked entries/trampolines/CaptureAndCall; Stage143 test-host lease does not cover game callers; duplicate Stage133 JMP would overlap 5/6 bytes |
| Stage129 native | `src/Stage129ProducerProbe.inl`; RVA **F30B0/6**, **F325A/5**, **F326A/5** | Direct game ingress, unsampled caller completion/exception return and retained foreign pointers unproven |
| Stage81 worker/patch chain | `src/Stage81SelectorWriter.inl`, including F7D22/F7D38/F7D43 bounded declarations | Worker install and recurring health loop have no global stop/join/native barrier |
| Other naked native entries | Stage153 snapshot counted **122 reachable `__declspec(naked)` occurrences** | These are source occurrences, **not 122 proven installed hooks**; dynamic owner/return/unwind graph incomplete |
| VEH callbacks | `AITrace.cpp:2738`, `AICommandTrace.cpp:805`, `Stage59XWriterWatch.inl:292` | **3** reachable registrations / **0** reachable unregistrations in Stage153 literal-include graph; Stage59 keep-until-process-exit comment line330; future callbacks remain possible |
| FLS | **0** FLS allocations/frees/setters in Stage153 66-file reachable production graph | Stage142/143 FLS tests were isolated, not game callback proof; add new FLS => re-audit |

### Eight worker create/close/loop source triplets

From Stage153 pinned snapshot: `ComVsCom.cpp 242/248/215`, `AITrace.cpp 2893/2903/2850`, `AICommandTrace.cpp 834/840/824`, `AIQueueTrace.cpp 3110/3116/3028`, `AICommandSiteTrace.cpp 2220/2226/2158`, `AIProducerContextTrace.cpp 829/833/811`, `AIParentFlowTrace.cpp 854/858/836`, `Stage81SelectorWriter.inl 550/552/542`. Each triplet denotes **CreateThread / CloseHandle / infinite-loop** line anchors. Closing an initial thread handle does not join or stop a running worker. No 8/8 production stop + retained handle + join proof.

## Callback/foreign-caller lifecycle must be proved before unload

1. **Pre-entry:** external owner references module; raw/foreign/native/function-pointer callback sources fully enumerated and blocked before closing admission; no pending unleased EIP.
2. **In flight:** owner generation + thread/fiber identity, stack/EIP and trampoline/parent frame accounting; native ABI, SEH, exceptional and nonlocal unwind correctness, no lost leases.
3. **Drain:** stop and join all eight real workers outside DllMain loader lock; drain native frames and all callback sources; own DR slots only; transactionally restore **owned** hooks and page protections/cache, including Stage67/70 failure paths.
4. **Future entry:** retire/unregister own three VEH callbacks after zero pending; exclude future exception dispatch, raw borrowed pointers, cross-thread asynchronous callbacks, stale epoch/ABA and late re-entry; only then consider *independently proven* shutdown eligibility.

**None of these complete production proofs exists.** The Stage155 test fixture's `host_leases=0` and `dll_inFlight=0` cannot preclude a pending raw native caller. Missing owner or caller provenance is a hard DENY, not an optimistic inference. Source-only Stage156 does not add hook writes, `Arm()`, or `FreeLibrary`.

## New Stage156 read-only Windows audit package

`Stage156_SourceOnly/Invoke-Stage156Audit.ps1` runs Python unittest, scans local `ArcanaAIOverhaul.vcxproj` compile roots and reachable literal source includes, hashes files, inventories lexical categories and **32** Stage153/155 line anchors as `SNAPSHOT_ANCHOR_MATCH` or `MISSING_OR_DRIFT`. Output is metadata JSON only (relative names/hash/line/category, no original source contents), outside project under `%TEMP%\ArcanaAI_Stage156_SourceOnly\`. Macro includes, inactive #if, source comments, inter-module calls and runtime pointers may cause missing/false-positive lexical matches; **never treat results as live owner proof**.

PowerShell 5.1, Python 3 (MSVC not needed):

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
cd 'D:\INCUBATOR\ArcanaAI_Stage156_SourceOnly\Stage156_SourceOnly'
.\Invoke-Stage156Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Expected messages: `STAGE156_SOURCE_COUNTS`, `STAGE156_BASELINE_ANCHORS`, `STAGE156_RAW_CALLER_PROOF=NO FUTURE_CALLBACK_PROOF=NO LIVE_OWNER_PROOF=NO`, `STAGE156_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false`. **Windows actual tree check is pending**, not asserted to have happened. If sharing generated JSON, review relative path/hashes first; do not commit original private source.

## Decision / next stage

Remain **SOURCE_ONLY** and keep `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, `origin(0x27)=UNKNOWN`. Preserve tested Stage129 `dinput8.dll` **2,099,200 B** and Stage122 fallback **2,089,984 B** without changes. Next: review Stage156 Windows **metadata only** for source drift, manual provenance of native raw entry and callback-source edges, enumerate unowned/unknown paths, and build separate isolated fail-closed tests as needed. No game DLL install/unload.
