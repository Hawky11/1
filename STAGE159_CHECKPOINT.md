# ArcanaAIOverhaul — Stage159 SOURCE_ONLY native hook writer/caller provenance gap graph (2026-11-14)

**Status: Stage159 package prepared; 40/40 offline synthetic/negative Python tests PASS, ZIP/manifest verified. Current Windows project Stage159 audit NOT YET EXECUTED.** The previous user-confirmed Stage158 r2 Windows source audit **DID PASS**, with 45/45 tests and all 66 source SHA; do not confuse these separate stages.

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
