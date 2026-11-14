# ArcanaAIOverhaul Stage170 — Native Preimage & Hook Ownership Evidence (2026-11-14)

**STATUS: Stage170 SOURCE_ONLY release ZIP CREATED, CLEAN-EXTRACTED, AND TESTED LOCALLY; USER WINDOWS STAGE170 NOT YET RUN.** The last independently user-confirmed Windows current-source audit is **Stage169 PASS** (7/7 manifest, 104/104 tests, SHA-pinned 13/13 current sources, 47 exact source-line witnesses, 12 OPEN production gaps, zero independent runtime artifacts). See [Stage169 checkpoint](STAGE169_CHECKPOINT.md). No game process, game DLL, native build, C++ production source modification, real hook installation or unload.

## Release package and reproducibility

- Chat artifact: `ArcanaAI_Stage170_NativePreimage_HookOwner_Evidence_SOURCE_ONLY.zip`
- **21,009 bytes, SHA256 `ff85b16373c4c3410c24165d567dd2627f6d687884d4b4f1caa12100bae91c4f`**, eight ZIP members and **CRC PASS**.
- **7/7 SHA-256+size manifest PASS**, independently rerun after fresh ZIP extraction; **107/107 Python negative/synthetic tests PASS** after clean extraction (3.277s). In addition, exact Stage169 synthesized sanitized ledger fixture matched its expected canonical SHA.
- Historical uploaded Stage146 source snapshot check: `STAGE170_SOURCE_SHA_MATCHED 4/4`, `STAGE170_EVIDENCE targets=3 source_witnesses=48 open_requests=12 production_gaps=12 runtime_artifacts=0`, `STAGE170_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false`. **This is an archived-source preview, not a Stage170 Windows actual-project result and not game runtime evidence.**
- Included files: read-only Python auditor `stage170_native_preimage_evidence.py`, PowerShell runner, SHA manifest verifier, synthetic/negative tests, **sanitized artificial Stage169 metadata fixture**, README, and narrow per-RVA evidence notes. No original C++ game source files, patched original instruction bytes, DLLs or EXEs are bundled.

## Exact Stage170 source boundary

Reads **only four** user-local, SHA-pinned sources, each unchanged in the Stage146 snapshot and previously within the user Windows 13/13 Stage169 checks:

| File | SHA-256 |
|---|---|
| `src/Stage67NativePaths.inl` | `f919af18379d85594c442e021b559a298b5a068b06ce7ebefc33f0cde94d3620` |
| `src/Stage70DeferredWriter.inl` | `bf3becbf1893426002fb3dfee0eb849caf1836c2d42059379c40958b7995b8df` |
| `src/Stage38CallbackWindow.inl` | `d0ad34650a94102e6c9dfc8b74228cf841510d04a690739dd11933af2e51557a` |
| `src/AICommandSiteTrace.cpp` | `1d11bb063895a3530160fc9fdb81c979b6f59d9875b645da6ca759709a090395` |

Also reads the **actual user-local Stage169 sanitized JSON** at `%TEMP%\ArcanaAI_Stage169_SourceOnly\stage169_source_runtime_evidence_matrix.json` and validates its complete canonical SHA-256 `7237f6457883514851a26ac80e21e84a3cc5057b46e8f6b6d1899ee9bf8eff03`, 13 historical pin metadata entries, 47 previous line witnesses, all 12 open gap IDs, and zero runtime proof artifacts. The deterministic hash is a **drift check**, not a cryptographic attestation of who generated the metadata. The full user's Windows Stage169 JSON has not been uploaded here.

There are **48 exact Stage170 source anchors**, each with file and line. Source pins are all-or-nothing: a hash or line drift yields REVIEW_REQUIRED with zero positive target and witness ledgers.

## Exact site findings — SOURCE ONLY; runtime proof = NOT TESTED

| Stage/RVA | Length | Compiled expected signature | Actual source write | Captured live preimage in reviewed installer? | Foreign owner? | Production byte rollback? |
|---|---:|---|---|---|---|---|
| Stage67 `AD75C` | 5B | line210, constant near-CALL | source line275 first `memcpy` | **NOT SAVED IN REVIEWED INSTALLER** | **UNPROVEN** | **NO path observed in reviewed installer** |
| Stage67 `E9E60` | 6B | line213, expected original MOV | source line275 second `memcpy` | **NOT SAVED IN REVIEWED INSTALLER** | **UNPROVEN** | **NO path observed in reviewed installer** |
| Stage70 `156784` | 6B | line155, expected original MOV | line231 `memcpy` | **NOT SAVED IN REVIEWED INSTALLER** | **UNPROVEN** | **NO path observed in reviewed installer** |

Specific source-anchored concerns:

- Stage67 line275 writes A 5B followed by B 6B. Lines276–278 flush code cache and mark `g_s67Installed=2`; lines281–283 restore page protection/resume/unlock; lines285–287 may return FALSE *after changing the code bytes*. There is no verified exact original-byte snapshot or reverse byte-restore/readback in the reviewed installer. A fault between writes can leave a mixed state (hypothetical source-path risk, not observed runtime).
- Stage70 line231 copies 6 patch bytes, line232 flushes, line233 sets `g_s70Installed=1`, line235 restores page protection, and lines241–243 can return FALSE after the write. Return status and installed-byte state are not proven transactionally consistent under postwrite failures.
- Named mutex and peer EIP window checks coordinate known/cooperating patch writers but do **not** demonstrate an exclusive lease against a foreign/noncooperating hook or ABA/hot reentry.
- Stage38 lines333–335 gate Stage70 on Stage67's BOOLEAN return, while Stage59 starts at line346 within the broader Stage58-success branch even after an ordinary Stage67 false; the runtime occurrence is **NOT TESTED**.
- A different hook in `AICommandSiteTrace.cpp` has a real **source-defined snapshot** (`g_alternateOriginalCallBytes`, line522; copy line1979). That other hook's saved bytes do not prove Stage67 or Stage70 preimages; nor does a saved snapshot alone prove **the other hook's rollback**.

## Evidence deliverable: three sites × four independent proof requests

Each of three targets has precisely four **OPEN** evidence requirements:

1. `EXACT_LIVE_PREIMAGE`: independently sourced, retained original byte snapshot *before* the first modification.
2. `EXCLUSIVE_BYTE_OWNER`: reliable arbitration with foreign noncooperating writers, owner lease, expected vs current bytes and ABA-safe release.
3. `PARTIAL_WRITE_ROLLBACK`: restore to correct preimage in reverse order, verify cache flush and readback, quarantine on any failed restore.
4. `POSTWRITE_FINALIZATION`: verifiable protection/cache/peer-resume/unlock cleanup and consistent state flags on every postwrite error.

Twelve per-RVA requests are **not** twelve new proofs. **All twelve separate, high-level Stage169 production safety gaps remain OPEN**, and there are **zero independent runtime artifacts**. No production safety flags promoted.

## User Windows run — PENDING

Extract outside `D:\INCUBATOR\ArcanaAIOverhaul`; open PowerShell in `Stage170_SourceOnly` and run:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage170Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Default Stage169 input: `%TEMP%\ArcanaAI_Stage169_SourceOnly\stage169_source_runtime_evidence_matrix.json`.
Default output: `%TEMP%\ArcanaAI_Stage170_SourceOnly\stage170_native_preimage_owner_evidence.json`.

Expected **only if the actual user metadata/current four sources still match**: `STAGE170_MANIFEST_PASS files=7`, 107/107 tests, `STAGE170_SOURCE_SHA_MATCHED 4/4`, `STAGE170_EVIDENCE targets=3 source_witnesses=48 open_requests=12 production_gaps=12 runtime_artifacts=0`, `STAGE170_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false`. On SHA/upstream drift the script fails closed and emits `STAGE170_SOURCE_REVIEW_REQUIRED`.

## Stage171 proposal (after actual Stage170 Windows result)

Instead of repeating status-only audits: prepare a **reviewable SOURCE_ONLY C++ transaction design/patch proposal** for *isolated nonproduction* preimage/foreign-owner/rollback semantics at these three sites. Preserve existing production C++ unchanged until explicit separate consent and independent native/loader/worker/VEH safety evidence. Do not imply offline tests validate safe live patch installation or unload.

**HARD DENY:** `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, origin(0x27)=UNKNOWN. Nine real worker joins, three VEH in-flight retirements, foreign raw callers/trampoline ABI/SEH and module owner lease unproven. Working Stage129 DLL and fallback Stage122 DLL remain untouched.