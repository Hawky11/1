# ArcanaAIOverhaul Stage171 — Offline C++ Transaction & Rollback Design (2026-11-14)

**STATUS: Stage171 SOURCE_ONLY C++ design package CREATED and validated locally. USER WINDOWS STAGE171 NOT YET RUN.** The latest confirmed **user Windows** source-only result is Stage170 PASS: manifest 7/7, Python `Ran 107 tests ... OK (skipped=1)` = **106 pass, 1 skipped Windows symlink test**, 4/4 current pinned source SHA, 48 exact source witnesses, 3 RVA targets, 12 independent per-RVA proof requests, 12 broader production gaps and 0 runtime artifacts. [Stage170 checkpoint](STAGE170_CHECKPOINT.md). Nothing in Stage171 patches production C++ or a game DLL.

## Release and independent offline validation

- Conversation ZIP `ArcanaAI_Stage171_OfflineNativeTransaction_RollbackDesign_SOURCE_ONLY.zip`
- **24,741 bytes, SHA-256 `ee6266ee3e3a0c356a838c95d27bbba83e23b6b40b371037ed7062c6bc54a646`.**
- **11 ZIP members CRC PASS**; **10/10 exact SHA+size manifest PASS** on a fresh extraction; **141/141 Python synthetic + negative tests PASS** (2.827 s clean-extract; 3.133 s in build directory). **One symlink test can be skipped on Windows** if creating test symlinks is unavailable, as with Stage170; report the actual Windows skipped count rather than asserting all cases executed.
- **Portable C++17 proposal source checked with `clang++ -std=c++17 -Wall -Wextra -Werror -fsyntax-only`** after clean extraction: PASS, **no binary was built, linked or executed**. This is syntax validation only, *not* C++ runtime/behavior testing.
- Historical Stage146 archived source preview with reconstructed, sanitized exact Stage170 JSON: `STAGE171_UPSTREAM stage170_pass=True three_rvas=3 witnesses=48 runtime_artifacts=0`; `STAGE171_SOURCE_SHA_MATCHED 4/4`; `STAGE171_CPP_DESIGN cpp_files=2 invariants=12 targets=3 site_requests=12 production_gaps=12 runtime_artifacts=0`; `STAGE171_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false`. This is an **archived-source preview**, NOT user Windows current-tree Stage171 proof.

## Files and scope

ZIP includes:
- `cpp/Stage171OfflineTransaction.hpp` — **reviewable, standalone C++17 generic transaction sketch using only three locally allocated toy arrays** (5 B, 6 B, 6 B). No RVA constants in executable sketch, Win32 APIs, process handles, native DLL build, trampoline, installed byte writer or game code hook.
- `cpp/Stage171SyntaxOnly.cpp` — type/syntax review translation unit (no `main`, executable, or build target).
- `stage171_toy_reference.py` — **independent Python toy reference**, tested with fail-closed preconditions, partial write/rollback, foreign mutation, cleanup/release failures. These tests do **not** execute the compiled C++ sketch and cannot attest C++ behavior.
- `stage171_design_audit.py` — read-only SHA/source-line and exact Stage170 JSON contract auditor; checks current four user-local files and verifies two C++ sketch SHA-pinned artifacts. Any source/JSON/C++ proposal drift yields REVIEW_REQUIRED and clears positive templates/anchors.
- `tests/test_stage171.py`, sanitized **artificial** `tests/fixtures/stage170_synthetic_fixture.json`, `verify_manifest.py`, `Invoke-Stage171Audit.ps1`, README and `STAGE171_TRANSACTION_DESIGN_REVIEW.md`.

The Stage170 JSON is expected **from the user's current machine** at `%TEMP%\ArcanaAI_Stage170_SourceOnly\stage170_native_preimage_owner_evidence.json`. Stage171 checks its exact deterministic canonical SHA-256 `3517fc077d5046b9d934bb6efc10eee3e925b35da4270fe5ce4fc1ec18142224`, source/witness ledger, target identities, all 12 open per-RVA requests and 12 high-level gaps, safety flags and zero runtime artifacts. A hash of sanitized JSON is a drift guard only, not provenance attestation.

The user source boundary remains **4/4 SHA pins**:
- `src/Stage67NativePaths.inl`: `f919af18379d85594c442e021b559a298b5a068b06ce7ebefc33f0cde94d3620`
- `src/Stage70DeferredWriter.inl`: `bf3becbf1893426002fb3dfee0eb849caf1836c2d42059379c40958b7995b8df`
- `src/Stage38CallbackWindow.inl`: `d0ad34650a94102e6c9dfc8b74228cf841510d04a690739dd11933af2e51557a`
- `src/AICommandSiteTrace.cpp`: `1d11bb063895a3530160fc9fdb81c979b6f59d9875b645da6ca759709a090395`

## Source-grounded transaction targets and design invariants

Exact *source evidence* only: Stage67 `AD75C` 5 B and `E9E60` 6 B have two writes on line275; Stage70 `156784` 6 B writes on line231. Stage38 can admit Stage59 even on ordinary Stage67 false, as previously observed. `AICommandSiteTrace.cpp:1979` preserves bytes for a **different hook**; does not prove rollback of these three sites.

Proposed offline toy C++ design:
1. DENY before any write unless toy evidence gates are all true (owner, future/native ingress, Stage59 admission, worker join+VEH drain, trampoline ABI/SEH, module lease). **No real gate is proven.**
2. Capture exact *toy* preimage for all three sites before any toy writes, and verify current toy bytes match expected originals.
3. Apply three toy local-array writes in **5 → 6 → 6 byte** order, treating both Stage67 sites as one logical transaction; support partial byte prefix failure.
4. For errors, reverse restore all possibly modified sites. Compare current byte prefix against expected preimage/replacement mixture before restoring. **Foreign drift or restore failure ⇒ quarantine, do not blindly overwrite.** Verify restored bytes by readback.
5. Toy postwrite finalization and toy lease release must complete before `CommittedToyOnly`. Cleanup/release failure ⇒ rollback or quarantine.
6. Regardless of toy `CommittedToyOnly` / `RolledBackToyOnly`, the C++ result and audit JSON explicitly set **real install/unload approved = false**. Simulated lease cannot arbitrate a noncooperating native writer, and array writes do not simulate page protection, instruction flush, EIP, debugger/VEH, loader epochs or actual module lifetime.

The audited source+design report includes **12 design invariants**, **two C++ proposal files**, **three target templates**, **12 still OPEN per-RVA evidence requests**, all **12 broader real production safety gaps OPEN**, and **0 independent runtime artifacts**. A **SOURCE_ONLY PASS is design completeness under reviewed static constraints, not actual C++ executable correctness, native ownership or game unload safety**.

## User Windows command (pending)

Unzip outside `D:\INCUBATOR\ArcanaAIOverhaul`. In `Stage171_SourceOnly`:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage171Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Default output: `%TEMP%\ArcanaAI_Stage171_SourceOnly\stage171_cpp_transaction_design_review.json`. Expected only if user-local exact Stage170 JSON and four source SHA match: `STAGE171_MANIFEST_PASS files=10`, 141 tests (potential one Windows symlink skip), `STAGE171_SOURCE_SHA_MATCHED 4/4`, `STAGE171_CPP_DESIGN cpp_files=2 invariants=12 targets=3 site_requests=12 production_gaps=12 runtime_artifacts=0`, `STAGE171_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false`. On drift, REVIEW_REQUIRED and empty positive ledgers.

## Handoff after user Windows Stage171

**Potential Stage172 (NOT BUILT/RUN): host-only executable C++ test harness** for the actual portable toy header, using isolated local arrays and explicit fault-injection cases. This would validate runtime behavior of the C++ sketch rather than merely a separate Python reimplementation; **never compile/link against the game, modify production C++, read the game process, or install/rollback real hooks**. A subsequent independent native/thread/VEH/owner evidence review remains necessary.

**Permanent gates:** `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, origin(0x27)=UNKNOWN. Actual live preimage, exclusive foreign writer lease, real byte rollback, 9 worker joins, 3 VEH retirements, raw caller/trampoline ABI/SEH and module lease are **UNPROVEN**. Working Stage129 DLL and Stage122 fallback remain untouched.
