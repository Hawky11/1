# ArcanaAIOverhaul — Stage158 native/raw ingress, worker and VEH lifecycle SOURCE_ONLY (2026-11-14)

**Status: Stage158 offline SYNTHETIC source-only tests 33/33 PASS; Windows CURRENT PROJECT Stage158 audit NOT YET RUN.** Stage157 r3 *was* user-confirmed on Windows (32/32; nine CreateThread site reconciliation; 12/12 Stage38/59 source signatures) and is not confused with this separate Stage158 work.

## Objective and evidence boundary

Stage158 moves beyond Stage157's one Stage38 branch. It rehashes **all 66** prior Stage156 reachable source files in the user's *local* tree, then emits a metadata-only ledger of worker start/close/loop anchors, three known VEH registration sites, five high-risk Stage10/Stage129 source RVA literals, and raw/native candidate counts. This is a **bounded lexical and anchored source review**, not an installed hook ownership map or a live caller/callback drain test.

Inputs expected on user's Windows host, not included in distributed package:
- `%TEMP%\ArcanaAI_Stage156_SourceOnly\stage156_current_source_inventory.json`: user-confirmed Stage156 Windows metadata (8 compile roots, 66 reachable source files, 32/32 pinned anchors, 1,511 lexical findings). Stage156 reported 9 CreateThread and 3 VEH registration sites, 0 VEH removal candidates.
- `%TEMP%\ArcanaAI_Stage157_SourceOnly\stage157_worker_lifecycle_metadata.json`: last Stage157 **r3**, user-confirmed source-only 9/9 worker paths, 12/12 signatures, `BRACED_STAGE58_DIRECT_STAGE59` structure MATCH.
- Read-only `D:\INCUBATOR\ArcanaAIOverhaul` sources; no private C++ text, INI, EXE, DLL or game data included in the ZIP, reported metadata or GitHub checkpoint. Each source file SHA is independently computed against Stage156's archived user-local SHA and byte length. Prior Stage157 two source content digests (Stage38 and Stage59) are independently reconciled.

## Actual offline deliverable, tested in container

- ZIP: `ArcanaAI_Stage158_NativeRawIngress_WorkerVEH_Lifecycle_SOURCE_ONLY.zip`
- **Exact size: 18,335 bytes; SHA256: `5d1c9ed5cd2d003211dd4c4c4bd7f5bf4f22fa8a79b7f9531e6b4df524843edd`**.
- **7 ZIP entries, CRC PASS**, manifest **6/6 SHA256 and size PASS**. Re-extracted ZIP into a fresh temporary directory, ran `verify_manifest.py` and reran all Python tests: **33/33 PASS**. Full standalone offline suite also **33/33 PASS**.
- Package `Stage158_SourceOnly` includes `stage158_native_lifecycle_audit.py`, `Invoke-Stage158Audit.ps1`, `verify_manifest.py`, `tests/test_stage158.py`, `docs/STAGE158_AUDIT_RU.md`, `README.md`, and `manifest.json`. No live process access or native game build.
- Synthetic fixture resembles 66 hashed files and 8 roots **only for regression testing**. Tests cover stale and missing source SHA, inconsistent Stage156 sparse counters, nine source worker+close+loop anchors, three VEH registration sites, five RVA **lexical** literals, staged JSON identity, Stage157 mismatched digests, duplicate/missing candidates, output path inside project denied, source strings not leaked, and fail-closed CLI exit on drift.

## Nine worker start/close/loop source anchors (not nine proven live threads)

| Source | CreateThread | CloseHandle | for(;;) |
|---|---:|---:|---:|
| `src/ComVsCom.cpp` | 242 | 248 | 215 |
| `src/AITrace.cpp` | 2893 | 2903 | 2850 |
| `src/AICommandTrace.cpp` | 834 | 840 | 824 |
| `src/AIQueueTrace.cpp` | 3110 | 3116 | 3028 |
| `src/AICommandSiteTrace.cpp` | 2220 | 2226 | 2158 |
| `src/AIProducerContextTrace.cpp` | 829 | 833 | 811 |
| `src/AIParentFlowTrace.cpp` | 854 | 858 | 836 |
| `src/Stage81SelectorWriter.inl` | 550 | 552 | 542 |
| `src/Stage59XWriterWatch.inl` | 349 | 351 | 303 |

Eight previously Stage153 anchored workers are started by Proxy export; ninth Stage59 is conditional on Stage58 branch as Stage157 r3 source proof. `CloseHandle` of original created handle is **not stop/join**. Stage59 `g_s59Tracked` holds handles to **watched game threads**, *not* a retained `S59Worker` join handle.

## VEH and native/raw scope

- VEH source anchors: `src/AITrace.cpp:2738`, `src/AICommandTrace.cpp:805`, `src/Stage59XWriterWatch.inl:292`. Stage156 counted 0 reachable VEH removal calls. Stage59 explicit comment keeps VEH registered for process lifetime after owned DR0/DR1 cleanup. Neither 0 text removals nor cleanup proves callback retirement or no future dispatch.
- Native raw source candidates: Stage10 RVA **F34D0** and **F3930** in `src/AIProducerContextTrace.cpp`; Stage129 **F30B0**, **F325A**, **F326A** in `src/Stage129ProducerProbe.inl`. Script finds their **source literals** (and reports review if missing); this is not a complete installed site inventory. Other Stage81 patch sites and 122 prior `__declspec(naked)` source occurrences are NOT claimed fully owned/covered. Stage143/155 test-host lease is not real native foreign-call admission proof.
- Additional Stage156 lexical category totals include `native_naked`, `native_asm_transfer`, `trampoline`, `indirect_entry`, `patch_writer`, `dr_context`, `veh_register/removal`, `thread_wait`. No source snippet is embedded; only relative filenames, line numbers, SHA256, simple counts.

## Windows PowerShell next step — NOT YET EXECUTED

Unzip the chat artifact. Open a PowerShell in the extracted `Stage158_SourceOnly` directory:

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage158Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

This checks manifest, runs 33 Python tests, reads local sources/metadata only, and writes `%TEMP%\ArcanaAI_Stage158_SourceOnly\stage158_native_lifecycle_metadata.json`. **Python 3 required; MSVC and game process NOT required**. Console markers: `STAGE158_SOURCE_HASHES matched=N checked=66 roots=8 reachable=66`, `STAGE158_WORKERS source_sites=9 create_close_loop_anchored=N`, `STAGE158_VEH register_anchors=N retirement_proof=NO`, `STAGE158_NATIVE_SOURCE_RVAS candidates=5 lexical_literals=N live_owner_proof=NO`, `STAGE158_RAW_CALLER_PROOF=NO WORKER_JOIN_PROOF=NO FUTURE_VEH_RETIREMENT=NO`.

If source hashes/lines/RVA syntax drift, expect **`STAGE158_SOURCE_REVIEW_REQUIRED`**, review exact reasons and do not force pass. If all bounded source preflights match, terminal `STAGE158_LOCAL_SOURCE_ONLY_TESTS_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false`. This latter is a STATIC/source-only success, **not** runtime game safety or permission.

## Blocking safety decision

**`SAFE_TO_INSTALL=false` and `SAFE_TO_UNLOAD_GAME_DLL=false` always**, Stage36 OFF, origin(0x27)=UNKNOWN. No external real owner-module lease, no closed foreign raw caller map, no native ABI/SEH/nonlocal unwind + EIP frame quiescence, no nine-worker retained stop/join, no three-VEH future callback retirement, no installed byte owner/transaction rollback proof. Stage67/70 failure paths remain suspicious and MUST NOT be changed/armed by this audit. Preserve working Stage129 DLL **2,099,200 B** and Stage122 backup **2,089,984 B** intact.

**Next after Windows Stage158 console**: classify any flagged stale hashes/RVAs/source anchors and prioritize exact native raw caller/trampoline provenance or worker/VEH proof obligations only on source-backed grounds. Do NOT attempt DLL unload.
