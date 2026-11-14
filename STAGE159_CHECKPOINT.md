# ArcanaAIOverhaul — Stage159 SOURCE_ONLY native hook writer/caller provenance gap graph (2026-11-14)

**Latest status: Stage159 r2 user-confirmed Windows SOURCE_ONLY PASS: 43/43 tests, 66/66 source hashes, 11/11 source RVA declarations. Zero proven writer→target dataflow edges; live ownership and safe unloading still DENY.** The previous user-confirmed Stage158 r2 Windows source audit **DID PASS**, with 45/45 tests and all 66 source SHA; do not confuse these separate stages.

## User-confirmed Stage159 r2 Windows current-project SOURCE_ONLY PASS (2026-11-14)

User ran the **Stage159 r2** `Invoke-Stage159Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'` after a process-scoped execution-policy bypass. The following is from the user's console (the complete private source or generated metadata JSON was **not** uploaded):

```text
STAGE159_R2_MODE SOURCE_ONLY READ_ONLY NO_GAME_DLL_ACCESS NO_NATIVE_BUILD
STAGE159_MANIFEST_PASS files=6
Ran 43 tests in 2.953s
OK
STAGE159_HASHES matched=66 expected=66
STAGE159_NATIVE_DECLARATIONS found=11/11 writer_api_lexical_candidates=418 proven_writer_target_edges=0
STAGE159_TARGET stage=STAGE10 rva=F34D0 declared_bytes=5 source_literals=6 owner=UNKNOWN
STAGE159_TARGET stage=STAGE10 rva=F3930 declared_bytes=6 source_literals=5 owner=UNKNOWN
STAGE159_TARGET stage=STAGE10_PARENT rva=F3280 declared_bytes=6 source_literals=6 owner=UNKNOWN
STAGE159_TARGET stage=STAGE10_PARENT rva=F4030 declared_bytes=6 source_literals=6 owner=UNKNOWN
STAGE159_TARGET stage=STAGE10_PARENT rva=F4360 declared_bytes=6 source_literals=6 owner=UNKNOWN
STAGE159_TARGET stage=STAGE129 rva=F30B0 declared_bytes=6 source_literals=3 owner=UNKNOWN
STAGE159_TARGET stage=STAGE129 rva=F325A declared_bytes=5 source_literals=1 owner=UNKNOWN
STAGE159_TARGET stage=STAGE129 rva=F326A declared_bytes=5 source_literals=1 owner=UNKNOWN
STAGE159_TARGET stage=STAGE81 rva=F7D22 declared_bytes=6 source_literals=1 owner=UNKNOWN
STAGE159_TARGET stage=STAGE81 rva=F7D38 declared_bytes=6 source_literals=1 owner=UNKNOWN
STAGE159_TARGET stage=STAGE81 rva=F7D43 declared_bytes=10 source_literals=1 owner=UNKNOWN
STAGE159_LIFECYCLE worker_sites=9 veh_register_sites=3 join=NO veh_retirement=NO
STAGE159_RAW_CALLER_PROOF=NO WRITER_DATAFLOW_PROOF=NO TRAMPOLINE_PROOF=NO ROLLBACK_PROOF=NO
STAGE159_METADATA C:\Users\Noma\AppData\Local\Temp\ArcanaAI_Stage159_SourceOnly\stage159_native_provenance_metadata.json
STAGE159_SOURCE_ONLY_PREFLIGHT_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
STAGE159_LOCAL_SOURCE_ONLY_TESTS_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
```

**What is now proven within the SOURCE_ONLY boundary:** Stage159 r2 package manifest verified; **43/43 Python tests PASS** on user Windows 3.14, including previously failing `test_30_symlink_escape_denied` now PASS without elevation. The [3/3] current-project scan ran and independently reported **66/66** unchanged Stage156/Stage158 r2 source hashes; **all 11/11 historically pinned native target literals found** across Stage10, Stage10 parent, Stage129 and Stage81; **418** lexical write API candidates; **9** worker sites, **3** VEH registrations. Previous Windows r1 WinError 1314 fixture blocker is resolved.

**What remains UNPROVEN:** 418 lexical occurrences are **not 418 installed hooks**; `proven_writer_target_edges=0` is a deliberate proof gap, not a report of zero runtime writers. No actual writer→RVA dataflow (target expression, patch length, write kind, original-byte snapshot/rollback), trampoline stolen-byte/return, foreign raw-caller origin, native ABI/SEH/unwind, global worker join, VEH handler retirement, live game owner/module lease or safe unloading was demonstrated. Stage67/70 transaction risk remains pending. The report's successful lexical source preflight does not supply install or unload permission.

**Next Stage160 (not yet executed):** narrow source-based writer-target **dataflow**, starting with `src/AIProducerContextTrace.cpp` Stage10 F34D0/F3930, `src/AIParentFlowTrace.cpp` F3280/F4030/F4360, `src/Stage129ProducerProbe.inl` Stage129 F30B0/F325A/F326A, then Stage81 sites F7D22/F7D38/F7D43. For each, trace declaration and executable writer call/target pointer construction, guard and call order, length, saved original bytes, trampolines and rollback path. Use actual user current source or pinned byte-equal snapshot, with source anchors; **do not infer owner from proximity**. Record any unknown intermediate as UNKNOWN, not PASS. Preserve negative tests for stage67/70 partial writes, cross-owner collision, raw foreign re-entry, and out-of-module callback after unload. No live/game access, no new patches.

**Safety unchanged:** `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, `origin(0x27)=UNKNOWN`. Working Stage129 game DLL **2,099,200 bytes** and Stage122 backup **2,089,984 bytes** must remain intact.

---

## User-confirmed first Windows Stage159 attempt — environment-only unit-test failure (2026-11-14)

User executed `Invoke-Stage159Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'` on Windows Python **3.14**, in an ordinary unelevated PowerShell. The package SHA manifest verified `STAGE159_MANIFEST_PASS files=6`. The 40-test synthetic unit suite reported **39 `ok` and one `ERROR`**:

```text
test_30_symlink_escape_denied ... ERROR
File "...\tests\test_stage159.py", line 198, in test_30_symlink_escape_denied
    (self.root/'src/link.cpp').symlink_to(outside)
OSError: [WinError 1314] Клиент не обладает требуемыми правами
Ran 40 tests in 2.809s
FAILED (errors=1)
STAGE159_TESTS_FAIL SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
```

This error occurs **during the construction of a synthetic test symlink under %TEMP%**; Windows does not allow the unelevated token to create one. It is not an actual game/native runtime error and does not show a source mismatch. Crucially, **[3/3] current source SHA/native graph audit was never reached**, so **no Stage159 current-project source preflight result exists**. The hard safety gate behaved correctly.

### Stage159 r2 non-admin fixture repair — offline tested, Windows pending

New chat-only ZIP `ArcanaAI_Stage159_r2_WindowsNonAdminSymlinkFixture_SOURCE_ONLY.zip`, **19,554 bytes**, SHA256 `8655cb0d25f6d8c8f5b066efcac88918d8123074b1975fe75d84b1888065c497`, **7 archive entries CRC PASS; 6/6 SHA+length manifest PASS after fresh extraction**, Python tests on clean extract **43/43 PASS** (~4.243 s). The **production `stage159_provenance_audit.py` is byte-identical to Stage159 r1**.

Only the synthetic tests, package marker and release docs changed:
- `test_30_symlink_escape_denied` first tries a **real** filesystem symlink. It now handles **only** Windows `OSError.winerror==1314` by simulating `Path.resolve()` yielding an external path and asserting production `checked_source` raises `SOURCE_SYMLINK_ESCAPE`. Other exceptions **still fail the test**; no elevation or Windows Developer Mode needed.
- `test_41` always checks simulated external-path resolution; `test_42` forces the WinError 1314 branch cross-platform; `test_43` proves unrelated OS errors are not swallowed. Simulation is **not** proof of real Windows junction/reparse-point integration.
- Wrapper prints `STAGE159_R2_MODE SOURCE_ONLY READ_ONLY NO_GAME_DLL_ACCESS NO_NATIVE_BUILD`; package remains source-only, no native build or game DLL access.

**NEXT (not yet performed on user's Windows r2):**

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage159Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Run from unpacked `Stage159_r2_SourceOnly`. It reuses existing local Stage156 and Stage158 r2 JSON in `%TEMP%`. Expect `STAGE159_MANIFEST_PASS files=6` and 43 tests; only afterwards will the **actual [3/3] current-source** source graph run. If that step returns `REVIEW_REQUIRED`, analyze it separately instead of treating the WinError fix as source proof.

Both `SAFE_TO_INSTALL=false` and `SAFE_TO_UNLOAD_GAME_DLL=false` remain. `RAW_CALLER_PROOF=NO`, `WORKER_JOIN_PROOF=NO`, `FUTURE_VEH_RETIREMENT=NO`; Stage36 OFF, origin(0x27)=UNKNOWN; Stage129 working DLL **2,099,200 B** and Stage122 backup **2,089,984 B** untouched.

---

## Deliverable integrity and provenance

- ZIP (Chat attachment, **not published to GitHub**): `ArcanaAI_Stage159_NativeHook_WriterCallerProvenance_SOURCE_ONLY.zip`, **17,451 bytes**, SHA256 **`23486989d255c0d137398ed5ff5b94e02214a34c5ccb2bb13c4d849d76c01ffa`**.
- ZIP contains **7 entries** (six payload files + `manifest.json`), ZIP CRC PASS. Manifest **6/6 SHA256 and length PASS** after independent extraction into a fresh temporary folder.
- Tests: Python stdlib unittest **40/40 PASS** (fresh extracted ZIP, ~2.905 s). Negative cases: stale SHA and length, missing source, changed historical RVA, spoofed Stage156/158 metadata, 9-worker/3-VEH ledger identity drift, unsafe live-safety flags, C++ fake comments/literals, unsupported raw strings, duplicate ledger entries, input/output paths inside project, symlink traversal outside root, writer proximity not owner, metadata code-leak prevention, deterministic report.
- Payload files: `Stage159_SourceOnly/stage159_provenance_audit.py`, `Invoke-Stage159Audit.ps1`, `verify_manifest.py`, `tests/test_stage159.py`, `README.md`, `docs/STAGE159_AUDIT_RU.md`, and manifest. **Contains no user's original source file, INI/config value, log, native DLL/EXE or PDB.**

## Core scope and constraints

Reads **two user-local metadata files** created by prior verified Windows source-only runs:
- `%TEMP%\ArcanaAI_Stage156_SourceOnly\stage156_current_source_inventory.json`;
- `%TEMP%\ArcanaAI_Stage158_r2_SourceOnly\stage158_r2_native_lifecycle_metadata.json`.

Reads all **66** local Stage156/158 known reachable C++ source paths under `D:\INCUBATOR\ArcanaAIOverhaul` without writing to the project. Strictly rehashes each file against both Stage156 and Stage158 metadata digests/byte lengths, validates Stage158 r2 source preflight and the exact nine worker + three VEH identities, and rejects drift or inconsistent input safety flags.

New target scope is **11 historically pinned declared patch sites**, not merely the five Stage158-literal baseline addresses:

| Group | Source path | RVA / previous declared stolen-byte length |
|---|---|---|
| Stage10 | `src/AIProducerContextTrace.cpp` | F34D0/5, F3930/6 |
| Stage10 parent | `src/AIParentFlowTrace.cpp` | F3280/6, F4030/6, F4360/6 |
| Stage129 | `src/Stage129ProducerProbe.inl` | F30B0/6, F325A/5, F326A/5 |
| Stage81 | `src/Stage81SelectorWriter.inl` | F7D22/6, F7D38/6, F7D43/10 |

Source declarations/lengths were previously pinned in Stage147/153 historical reviews; **no new live patch byte or hook owner evidence was collected**. The analyzer searches whole masked source C++ hexadecimal literals with U/L suffix support, records source line numbers, and enumerates `VirtualProtect/VirtualProtectEx`, `WriteProcessMemory`, `memcpy/memmove`, `FlushInstructionCache` **lexical candidates** in all 66 reachable source files. It additionally emits a bounded **within-60-lines proximity** between target literal and same-file writer API, explicitly marked **NOT dataflow**. Every target still has `writer_to_rva_link=UNKNOWN`, `installed_site_owner=UNKNOWN`, `trampoline_to_entry_link=UNKNOWN`, `native_return_unwind=UNKNOWN`, `raw_caller_census=UNKNOWN`. Source candidates and line proximity are never inferred as actual live patch operations or correct trampoline byte replay. Source emitted in metadata is only path/line/digest/count, not C++ contents.

## Stage67/70 risk context

The source-only report also lists three previously anchored declared risk targets `AD75C/5`, `E9E60/6`, `156784/6`. Prior Stage150/151 analysis described potentially non-atomic install failure paths after writes with no demonstrated byte rollback; the Stage159 scanner **does not restore or patch these sites**, and `runtime_transactional_rollback_proven=false` remains. If any of the three literal spellings are absent in actual current sources, the script returns `STAGE67_70_RVA_SOURCE_REVIEW` rather than assuming the hook disappeared.

## Windows read-only command (not yet run)

After extracting the ZIP, open PowerShell inside `Stage159_SourceOnly`:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage159Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Requires Python 3, **no MSVC, no game process, no DLL**. Runs manifest check, 40 Python tests, then current source metadata/SHA provenance audit; metadata output path `%TEMP%\ArcanaAI_Stage159_SourceOnly\stage159_native_provenance_metadata.json`. Expected (not yet observed) markers include:

```text
STAGE159_HASHES matched=66 expected=66
STAGE159_NATIVE_DECLARATIONS found=11/11 writer_api_lexical_candidates=<number> proven_writer_target_edges=0
STAGE159_LIFECYCLE worker_sites=9 veh_register_sites=3 join=NO veh_retirement=NO
STAGE159_RAW_CALLER_PROOF=NO WRITER_DATAFLOW_PROOF=NO TRAMPOLINE_PROOF=NO ROLLBACK_PROOF=NO
STAGE159_SOURCE_ONLY_PREFLIGHT_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
STAGE159_LOCAL_SOURCE_ONLY_TESTS_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
```

If an address is represented as a computed expression, macro, or otherwise fails the bounded lexical rule, the audit must **fail closed** with exact `STAGE159_REVIEW_REQUIRED reason=...`; do not assert the code is broken or force a PASS.

## Safety and follow-up

**`SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, origin(0x27)=UNKNOWN**. Real foreign/raw native caller and trampoline frame census, retained process module lease, ABI/SEH/unwind, all nine worker joins, three VEH unregister/future callback drain and exact transactional rollback remain unproven. Do not build/install/Arm or unload the game DLL. Preserve existing Windows-tested Stage129 **2,099,200 B** DLL and Stage122 **2,089,984 B** fallback.

Next: inspect actual user Windows Stage159 output. If all 11 pinned source RVA declarations are found, prioritize *semantic writer-target dataflow* in narrow source ranges (not more lexical counts), retaining source-only hard DENY until all live ownership requirements are independently proven.
