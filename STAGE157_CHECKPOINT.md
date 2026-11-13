# ArcanaAIOverhaul — Stage157 ninth worker and Stage59 VEH callback lifetime SOURCE_ONLY (2026-11-13)

**Status: Stage157 historical-source classification COMPLETE; current-Windows targeted source validation PENDING.** No game process access, native build, DLL installation/unload, extra JMP, Arm() or gameplay write. `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, `Stage36 OFF`, `origin(0x27)=UNKNOWN`.

## Input and provenance

- User pasted **all nine** `kind=create_thread` rows from **their own current Windows Stage156 metadata** (`8 compile roots, 66 reachable files, 32/32 pinned line anchors, 7/7 read-only tests`). Paths / 1-based line numbers:
  1. `src/AICommandSiteTrace.cpp:2220`
  2. `src/AICommandTrace.cpp:834`
  3. `src/AIParentFlowTrace.cpp:854`
  4. `src/AIProducerContextTrace.cpp:829`
  5. `src/AIQueueTrace.cpp:3110`
  6. `src/AITrace.cpp:2893`
  7. `src/ComVsCom.cpp:242`
  8. `src/Stage59XWriterWatch.inl:349`
  9. `src/Stage81SelectorWriter.inl:550`.
- Eight sites match **all eight previously source-anchored Stage153 Proxy-started workers**. The ninth is Stage59's `StartStage59Watch` background `S59Worker` launch. These are nine source lexical callsites, NOT evidence that nine threads are currently running.
- Read the **actual historical GitHub Stage59 r2 source**: [Stage59XWriterWatch.inl](https://github.com/Hawky11/1/blob/stage59-r2-wait-for-combat-objects/Stage59_Overlay/src/Stage59XWriterWatch.inl), blob `f37b0fb706d1aed080263d2bf740d926b816e456`, and [Stage38CallbackWindow.inl](https://github.com/Hawky11/1/blob/stage59-r2-wait-for-combat-objects/Stage59_Overlay/src/Stage38CallbackWindow.inl), blob `6bfa41a2670a523aaa9efc203f4659d6b01dcfd1`. **This historical source is not automatically verified byte-identical to the entire current Windows file**; Stage156 table anchors its CreateThread at line349 and prior 32-anchor scan checks Stage59 VEH registration line292, but did not upload or inspect current Stage59 content.

## Source-level causal chain

| Surface | Archived Stage59 r2 position | Meaning |
|---|---:|---|
| Stage38 literal include | 27 | Include `Stage59XWriterWatch.inl` in Stage38 adapter |
| Stage38 conditional start | 243 | `if(InstallStage58Hooks())StartStage59Watch();` after Stage58 success |
| Stage59 start function | 341–353 | Gate `g_s58Installed==3` (344), CAS `g_s59Started=1` (348), `CreateThread(...,S59Worker,...)` (349), `CloseHandle(h)` (351) |
| Worker thread | 203 | `S59Worker` checks INI and conflicting Stage16 DR modes, waits for real game objects |
| VEH callback | 44, 292 | `S59ExceptionHandler` registered with `AddVectoredExceptionHandler`, stored in `g_s59Handler` |
| Diagnostic loop | 303–329 | `for(;;)` scans installed game-thread watchpoints, flushes, checks object pointer drift; conditional break on drift |
| Worker exit | 330–339 | Comment explicitly **keep VEH for process lifetime**; `g_s59Capturing=0`, `S59UninstallOwned()`, flush; return. No VEH removal |
| Game-thread handles | 14–19, 124–145, 146–202 | `g_s59Tracked[]` holds **watched game-thread handles**, NOT a retained handle to `S59Worker` |

Important: `CloseHandle` on the newly created S59Worker leaves no retained `join` handle at that start site. `S59UninstallOwned` disarms **owned DR0/DR1 watchpoints**, NOT the VEH registration. `g_s59Capturing=0` suppresses capture, NOT future exception handler entry. The archived source sets `g_s59Ready=1` at 294 and does not visibly clear it on the normal exit branch. Thus lifetime ownership and callback quiescence **remain unknown**. Separate source-level retry weakness: `g_s59Started` set before `CreateThread`; failed creation at 350 does not visibly restore 0. Neither is a claim that a runtime failure occurred.

Prior historical [Stage59 r2 runtime analysis](Stage59_Runtime/R2_RUNTIME_ANALYSIS.md) recorded genuine diagnostic watchpoint hits in its **historical test DLL**, not a current Stage129 unloaded-module safety proof.

## New bounded Stage157 package

- Chat-only ZIP: `ArcanaAI_Stage157_NinthWorker_Stage59_CallbackLifetime_SOURCE_ONLY.zip` **10,748 B**, SHA256 `383e801c59b3f2052bee2b2da791055b7893e91b994f7a65c91c651c9200729e`, **6 files**, CRC PASS and SHA256 manifest **5/5 PASS**. Does not copy private current project source or binaries.
- Contents: `Stage157_SourceOnly/stage157_worker_lifecycle_audit.py`, `Invoke-Stage157Audit.ps1`, `tests/test_stage157.py`, Russian audit, README, manifest. It consumes local Stage156 metadata and read-only current `src/Stage59XWriterWatch.inl`/`src/Stage38CallbackWindow.inl`, checks **nine expected sites and 12 Stage59/Stage38 signatures**, fail-closed on extras/missing/duplicate or signature drift; emits metadata under `%TEMP%\ArcanaAI_Stage157_SourceOnly`. **11/11 Python synthetic tests PASS in the assistant environment**, including corruption/missing/extra negative cases and forbidding output inside project. No native MSVC build or Windows Stage157 run.
- It never marks live owner, unload, raw caller closure, or VEH retirement as safe based on static tests.

## Windows next command — not yet executed

PowerShell 5.1, Python3, no MSVC or game launch:

```powershell
Set-ExecutionPolicy -Scope Process Bypass -Force
cd 'D:\INCUBATOR\ArcanaAI_Stage157_NinthWorker_Stage59_CallbackLifetime_SOURCE_ONLY\Stage157_SourceOnly'
.\Invoke-Stage157Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Reuses `%TEMP%\ArcanaAI_Stage156_SourceOnly\stage156_current_source_inventory.json`. User should paste console markers; produced metadata JSON may be shared after privacy review. If any Stage59 signature differs from historical r2, report the drift before trusting source attribution; do not patch to force a match.

## Safety / follow-up

**9 source CreateThread callsites**, **8 Proxy-start chains plus 1 Stage59 conditional Start**, but all nine worker stop/join and future callbacks **remain unproven**. Stage153/156 saw **three** reachable VEH registrations and **zero** removals, with Stage59 explicit intentional persistence. Raw foreign/native caller ownership, ABI, SEH/unwind, EIP/live hooks, rollback and exact worker/callback gate remain unknown. `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`; preserve Stage129 **2,099,200B** working DLL and Stage122 **2,089,984B** rollback. Next read-only stage: validate Stage157 Windows source signatures, then analyze raw/native callback provenance without game detour installation.
