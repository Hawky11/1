# ArcanaAIOverhaul — Stage154 source-only worker shutdown/VEH retirement contract (2026-11-13)

**SOURCE_ONLY.** NO game DLL, process or source tree changes. `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`; Stage36 OFF, origin(0x27)=UNKNOWN. **MSVC Windows x86 Stage154 WAS run by user and passed on isolated local harness (see confirmed transcript below).** Stage152 Win32 private sandbox was earlier user-confirmed PASS, not equivalent.

## User-confirmed Windows MSVC x86 run — PASS (2026-11-13)

User executed `D:\INCUBATOR\ArcanaAI_Stage154_SourceOnly_ShutdownAdmission_WorkerVEH_Contract_r1\Stage154_SourceOnly\Invoke-Stage154Audit.ps1` and pasted complete console output. **This replaces the previous pending native-run status.** No game DLL or game process was touched by the harness, per its reported instrumentation; not independent live verification.

- `STAGE154_MANIFEST_PASS files=11`
- Python unittest: **37 total; 32 ok + 5 skipped**, no failed tests, `Ran 37 tests in 0.022s OK (skipped=5)`. Skipped `test_28_cpp_gcc`, `test_29_cpp_clang`, `test_30_cpp_compilers_agree`, `test_31_model_exact_cases`, `test_32_model_full_predicate_exhaustive`; all skipped due to absence of `g++`/`clang++` on this Windows session. Previously completed Linux portable compiler verification remains separate.
- Runner phase 3/4: `Stage146 ZIP check skipped (optional; no original ZIP provided)`. **Do not claim current Windows recheck of 85 original source blobs or 38 exact anchors** in this run; those belong to earlier pinned Stage146 snapshot audit.
- MSVC x86 built/ran `shutdown_contract.cpp` and `win32_private_fixture.cpp`.
- `STAGE154_PREDICATES count=20 exhaustive=1048576 hypothetical_ready=1 denied=1048575 single_missing_denied=20`.
- `STAGE154_TRANSACTION scenarios=16 quarantined=15 model_ready=1 all_unload_denied=YES`. `WORKER_TIMEOUT` produced `joined=4`, `VEH_UNREGISTER_FAILED` produced `veh_retired=1`; both quarantined as designed. No fault scenario allowed unload.
- `STAGE154_GAME_DLL_ACCESS=NO patch_writes=NO FreeLibrary_calls=NO live_owner_proof=NO`.
- `STAGE154_WIN32_X86_SELF_OWNED workers_created=8 workers_joined=8 veh_registered=3 veh_removed=3 pre_join_denied=YES callback_lease_denied=YES bad_wait_denied=YES external_process_access=NO`
- `STAGE154_WIN32_NATIVE_FIXTURE=PASS GAME_DLL_UNLOAD=FORBIDDEN`
- Final: **`STAGE154_LOCAL_SOURCE_ONLY_TESTS_PASS (NOT safe to install or unload game DLL)`**.

**Interpretation:** self-owned Windows thread handles and VEH registration/removal are verified **within the fixture**. Real `Proxy.cpp` continues to close original thread handles; live native ingress, callback quiescence, exact patch ownership and full safe production shutdown are UNPROVEN. `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, `Stage36 OFF`, `origin(0x27)=UNKNOWN` remain unchanged.

## Deliverables, exact checksums

- Chat ZIP: `ArcanaAI_Stage154_SourceOnly_ShutdownAdmission_WorkerVEH_Contract_r1.zip`, **20,488 B**, SHA-256 `e6f6c38cdee7fb2c2596a73f2d818730cf82622218d1a2133a6e17f83d2f08ab`; **12 files**, ZIP CRC PASS, extracted internal sha256 manifest **11/11 PASS**. Source-only content only (no private user project source, INI, EXE, DLL or game data).
- Separate Russian audit: `ArcanaAI_Stage154_Audit_RU.md`, **10,968 B**, SHA-256 `360bff231622dbba68ee6e5a4a8ed52e90558268fd12dd83c5447314ff9d67d8`.
- Upstream **pinned** Stage146 evidence SHA `6d9c39d59b4883dab6cc4ea95f08deac56fe8c477fd6b4746bd1c764f96a207b` checked. All **85/85** source blobs match Stage146 `current_sources.json`, and **38/38 exact line anchors** confirmed against that same ZIP. Stage153 ZIP provenance hash `031d1a58229021768923da530d7e9bc95e338f4ed6c2c2f6fee08fc99edb1e0b`. Source pins reflect the Nov 13 *snapshot*, not loaded runtime.
- Offline tests actually executed on Linux: **37/37 Python PASS**, independently compiled GCC+Clang portable model with byte-identical output. Re-extracted final ZIP, reran manifest 11/11 and tests 37/37 PASS, source anchors 38/38 PASS. **Windows fixture later executed by user: MSVC x86 compilation and runtime tests PASS (see below).**

## Stage153 source blockers refined into contract (do not mistake tests for production fix)

- `src/Proxy.cpp:81–88` export `DirectInput8Create` calls **eight** `Start*` functions, `Proxy.cpp:92–100` DllMain attach only. Native shutdown with waiting MUST be externally orchestrated **outside DllMain loader lock**.
- Eight worker paths at Create/Close/for(;;): `ComVsCom.cpp:242/248/215`, `AITrace.cpp:2893/2903/2850`, `AICommandTrace.cpp:834/840/824`, `AIQueueTrace.cpp:3110/3116/3028`, `AICommandSiteTrace.cpp:2220/2226/2158`, `AIProducerContextTrace.cpp:829/833/811`, `AIParentFlowTrace.cpp:854/858/836`, `Stage81SelectorWriter.inl:550/552/542`. Immediate `CloseHandle(thread)` does NOT stop/join that thread and no retained join handle was established at reviewed start sites.
- **Three source VEH registrations** `AITrace.cpp:2738`, `AICommandTrace.cpp:805`, `Stage59XWriterWatch.inl:292`; **zero reachable RemoveVectoredExceptionHandler** in Stage153 graph, Stage59 explicitly says keep its VEH for process lifetime (line330). Owned handler removal requires generation quiescence, active callback drain and safe owned watchpoint removal (DR ownership remains unproven).
- Full native ABI/SEH/nonlocal-unwind and live loaded hook owner/caller proof still **UNPROVEN**, as do real Stage67/70 transactional rollbacks, external-owner reference barrier and protection restoration. **Do not invoke FreeLibrary** on current game DLL. Prior Stage142/143 test-DLL FLS probes cannot establish game FLS lifetime.

## New Stage154 synthetic tests and planned admission sequence

- Portable `src/shutdown_contract.cpp`: **20 synthetic preconditions**, **2^20 = 1,048,576** masks; only 1 hypothetical all-true mask, **1,048,575 DENIED**, 20/20 single-missing denied. Separate 16 named transaction cases: **1 hypothetical MODEL_READY, 15 QUARANTINED**, including double fault and failures at stop, join, ingress/VEH drain, debug register ownership, patch restore, cache/protection, VEH retirement, repeated shutdown. No code path calls FreeLibrary or accesses game.
- Planned order (**not implemented in production**): independent external owner lease → close all export+native/raw ingress (including foreign callers) → cooperative stop → **join all 8 retained thread handles** → drain all native frames, including exception/unwind and callbacks → disarm only own DR watchpoints → restore only owned verified hook bytes transactionally with protection/cache checks → unregister own **3** VEH after no exceptions can enter, ensure zero active callbacks → maintain owner lease until all obligations are proven, else quarantine.
- `src/win32_private_fixture.cpp`: **new standalone x86 EXE source** planned to start 8 **test-owned** threads retaining handles until join, register and remove 3 **test-owned** passive VEH, and reject incorrect admission while workers or simulated callback lease remain live. Tests `WaitForSingleObject(nullptr,0)` failure. Does **not** create game hooks, inspect/modify DR, invoke a VEH callback, access foreign process or unload DLL. **MSVC x86 run USER-CONFIRMED PASS on standalone fixture**.

## Windows x86 command, successfully executed by user

Extract Stage154 ZIP and run in the same PowerShell session:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch x86 -HostArch x86
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
cd 'D:\INCUBATOR\ArcanaAI_Stage154_SourceOnly_ShutdownAdmission_WorkerVEH_Contract_r1\Stage154_SourceOnly'
.\Invoke-Stage154Audit.ps1
```

Optional `-Stage146Zip 'D:\...\ArcanaAI_Stage146_Evidence_493fc20dff504bdabc3abc4924a9426f.zip'` to repeat pinned read-only check; **archive is not required** for synthetic C++/Win32 tests. PowerShell expects Python+MSVC x86 in PATH. The runner builds only temporary stand-alone EXEs under `%TEMP%`, executes and cleans them, and has strict output asserts.

**NEXT Stage155:** Windows Stage154 logs received and verified as reported below. Develop further source-only external admission/lease ownership and callback safety audit, not game DLL installation. Preserve working Stage129 **2,099,200 B** DLL and Stage122 **2,089,984 B** rollback untouched.
