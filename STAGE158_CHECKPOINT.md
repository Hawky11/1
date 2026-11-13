# ArcanaAIOverhaul — Stage158 native/raw ingress, worker and VEH lifecycle SOURCE_ONLY (2026-11-14)

**Status: Stage158 r1 user Windows read-only 33/33 tests and 66/66 SHA PASS; review required for two unrecognized Stage129 RVA literal spellings. Stage158 r2 suffix-aware parser offline 45/45 tests PASS; Windows r2 still PENDING.** Stage157 r3 *was* user-confirmed on Windows (32/32; nine CreateThread site reconciliation; 12/12 Stage38/59 source signatures) and is not confused with this separate Stage158 work.

## User-confirmed Stage158 r1 Windows current-source audit (2026-11-14) — REVIEW_REQUIRED, one bounded RVA issue

User ran `Stage158_SourceOnly/Invoke-Stage158Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'`, and pasted the actual console transcript; no original project C++ or resulting JSON uploaded to chat. Results:

```text
STAGE158_MODE SOURCE_ONLY READ_ONLY NO_GAME_DLL_ACCESS NO_NATIVE_BUILD
STAGE158_MANIFEST_PASS files=6
Ran 33 tests in 2.324s
OK
STAGE158_SOURCE_HASHES matched=66 checked=66 roots=8 reachable=66
STAGE158_WORKERS source_sites=9 create_close_loop_anchored=9
STAGE158_VEH register_anchors=3 retirement_proof=NO
STAGE158_NATIVE_SOURCE_RVAS candidates=5 lexical_literals=10 live_owner_proof=NO
STAGE158_RVA_SOURCE stage=STAGE10 rva=F34D0 path=src/AIProducerContextTrace.cpp lexical_matches=5 owner=UNKNOWN
STAGE158_RVA_SOURCE stage=STAGE10 rva=F3930 path=src/AIProducerContextTrace.cpp lexical_matches=4 owner=UNKNOWN
STAGE158_RVA_SOURCE stage=STAGE129 rva=F30B0 path=src/Stage129ProducerProbe.inl lexical_matches=1 owner=UNKNOWN
STAGE158_RVA_SOURCE stage=STAGE129 rva=F325A path=src/Stage129ProducerProbe.inl lexical_matches=0 owner=UNKNOWN
STAGE158_RVA_SOURCE stage=STAGE129 rva=F326A path=src/Stage129ProducerProbe.inl lexical_matches=0 owner=UNKNOWN
STAGE158_RAW_CALLER_PROOF=NO WORKER_JOIN_PROOF=NO FUTURE_VEH_RETIREMENT=NO
STAGE158_REVIEW_REQUIRED reason=NATIVE_RVA_LITERAL_REVIEW_REQUIRED
STAGE158_SOURCE_REVIEW_REQUIRED SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
```

**Interpretation:** The **66 current source files** are byte-identical in SHA/length to the user's Stage156 metadata; all nine bounded worker create/close/loop triplets and three VEH register anchors match. Two Stage129 RVA spellings **were not recognized by r1's narrow lexical regex**. This says nothing conclusive about whether those hooks exist or are installed. In particular, r1's `\b` immediately after an RVA rejects normal C++ suffixed integers such as `0xF325Au` and `0xF326AUL`; the user's actual Stage129 lines have **not** been inspected, so this is a *plausible technical cause*, not confirmed as their exact syntax. Do NOT modify source to force the regex.

## Stage158 r2 read-only diagnostic patch — offline tested, Windows pending

- New ZIP `ArcanaAI_Stage158_r2_NativeRVA_LexerDiagnostics_SOURCE_ONLY.zip`, **18,415 bytes**, verified **SHA256 `7d2d0041aadb167a571b5d5e67d2c81834f849ea9996f2fc4fc8425288ddbf1d`**; **8 ZIP entries CRC PASS**, manifest **7/7 payload SHA and byte size PASS** on re-extracted ZIP.
- Re-extracted test suite **45/45 Python tests PASS** (33 original + 12 new) on synthetic inputs; no Windows user's r2 run has been performed yet.
- R2 replaces the RVA matching helper with a **whole-token C++ hex integer suffix-aware lexical check**: accepts 0x/0X, leading zeroes, case differences, valid U/L suffixes; rejects identifier substrings, longer hex values, malformed suffix tails and comments/strings. Emits relative path/line, `CPP_HEX_INTEGER` versus `BARE_HEX_TOKEN`, `cpp_integer_suffix` boolean and count; **never embeds original source text**.
- This change is **limited to the lexical source audit**; unchanged 66 source SHA cross-check, nine workers, three VEH, Stage157 r3 integrity, native owner and callback/worker DENY flags remain. Unknown computed expressions, macros or absent literals still `NATIVE_RVA_LITERAL_REVIEW_REQUIRED`, fail-closed.
- New metadata path: `%TEMP%\ArcanaAI_Stage158_r2_SourceOnly\stage158_r2_native_lifecycle_metadata.json`. Script `Invoke-Stage158Audit.ps1` reuses local Stage156 and Stage157 r3 JSON; Python 3, no VS/game needed.

Windows read-only r2 command from extracted `Stage158_r2_SourceOnly`:

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage158Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

If r2 still reports `NATIVE_RVA_LITERAL_REVIEW_REQUIRED`, next smallest evidence is numbered **lines 1–30 only** of current `src/Stage129ProducerProbe.inl` (review before sharing), *not* full project source. The r2 result must not be preclaimed as Windows PASS. Stage129 native return/unwind ownership and foreign raw-caller admission remain UNKNOWN.

**Safety invariant:** `SAFE_TO_INSTALL=false`; `SAFE_TO_UNLOAD_GAME_DLL=false`; Stage36 OFF; origin(0x27)=UNKNOWN; no new native hooks, no build, no game process/memory access, no FreeLibrary, no change to Stage129 2,099,200 B or Stage122 2,089,984 B game DLLs.

---

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
