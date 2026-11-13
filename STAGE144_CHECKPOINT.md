# ArcanaAIOverhaul — Stage144 real entrypoint coverage / admission gap source audit (2026-11-13)

**STATIC/READ-ONLY ONLY. SAFE_TO_INSTALL=false. Stage36 OFF. Working game DLL untouched.**

## Starting point

- Stage143 **r2 isolated Windows x86** test DLL/host: **MSVC build PASS, 16/16 test scenarios PASS** (user-provided log, retained in [Stage143 checkpoint](STAGE143_CHECKPOINT.md)). The test-only host Gate/Lease can block cooperating host calls, but it is not wired into native Stage10/Stage129 detours or Stage141 candidate bridge.
- Source evidence: user's **2026-11-12 UTC Stage134 current-source snapshot** `ArcanaAI_Stage134_CurrentSources.zip`, archive SHA256 `e1ae1b0bb74b508a2d47ce600c37f67f730b64306c52fc3d4e6d16cf2077a84f`; archive CRC PASS; **all 70/70** file SHA256/size comparisons reverified from the source-capture manifest in this conversation.
- Exact reference ZIPs CRC PASS: Stage139 SHA256 `b3a514b6349aa5603c2623583f15b82eabe9f3007380f857f518984ea863f934`, Stage141 `8097b0e05a3ee308824b33f79cd129cea56c63283fac346f098a6d55f9c860f5`, Stage143 r2 `6e3c1c77525421ed7f156144c8910f924fc8ff98209b83c1272bbbb5bfe95cf4`. These candidate and isolated test code snapshots are not proof of loaded game modules.

## Stage144 artifact

Chat-only ZIP: **`ArcanaAI_Stage144_RealEntrypoint_Coverage_AdmissionGap_SourceAudit.zip`**; **75,660 bytes**, SHA256 **`5adfe52843106df2a1832f6e7b70fdceaf645620b35d93d1df2ceadd696b32fc`**, **25 files**, ZIP CRC PASS, packaged manifest **24/24** SHA256+size checks PASS, no binary executable/DLL/LIB/OBJ/PDB/EXP. Archive attached in ChatGPT, **not stored in GitHub**.

The source-only package contains **11 byte-hashed reference sources**, a 38-entry line-anchored inventory, full Russian risk audit, original Stage134 SHA capture manifest, archive hashes, static source verifier, synthetic gate model, and a read-only Windows PowerShell current-tree drift checker.

## Verified inventory, not a complete live map

Static programmatic inventory: **38 selected surfaces**, grouped as:
- **2** baseline Proxy dispatch/loader entrypoints.
- **8** baseline Stage10 native hooks, trampoline wrapper, worker/install/suspension entries.
- **3** baseline Stage81 startup, worker, Stage129 periodic flush dispatch.
- **6** baseline Stage129 observer, 3 native hook entrypoints, install and flush.
- **13** Stage139/141 **compile-only / source-only** bridge+FLS entrypoints; not integrated into default game project.
- **6** Stage143 **isolated test-only** Gate/Lease/external call wrappers, unload and test DLL initialization.

Each record in `evidence/entry_surfaces.json` has exact snapshot-relative source path, **line number**, verified substring, scope and unresolved coverage requirement; source hashes are in `evidence/reference_sha256.json`. The full analysis and failure-mode classification are `docs/SURFACE_COVERAGE_AUDIT_RU.md`.

**This is a bounded, reviewed surface inventory. It does NOT enumerate every possible game entrypoint, owner, dynamic thread or foreign callback.**

## High-severity conclusions (source-grounded)

1. **Native calls bypass Stage143 cooperative host admission.** Stage10 owns optional F34D0 (5B) and F3930 (6B) naked detours; Stage129 owns F30B0 (6B), F325A (5B), F326A (5B). Their x86 native handlers call internal observer/wrapper functions directly. The Stage143 Gate/Lease lives **only** in the disposable test host and is never entered on these native game paths. `host_leases=0` and `dll_inFlight=0` could coexist with a **raw game call pending**.
2. **Patch-worker and output-writer lifecycle independent.** `Proxy.cpp` forwards `DirectInput8Create` and starts Stage10 and Stage81 workers. Stage81 `Worker` installs a chain including Stage129 then runs a recurring infinite Flush/Health loop. No proven stop+join global barrier. Stage10 `SuspendOtherProcessThreads` returns 0 on snapshot failure and skips failed individual suspends, not a full quiescence/parent-frame proof.
3. **FLS callback unload remains unproven for game candidate.** Stage141 `FlsAlloc(&FreeThreadState)` points to callback code in the module. No `FlsFree` or full thread/fiber registry/unload barrier is implemented in the source-only Stage141 bridge; `DisableThreadLibraryCalls` is **not** FLS deregistration.
4. **Candidate bridge has independent entries.** Stage139 conditional integration invokes `InvokeOriginalWithScope`, `OnStage129Entry`, `OnStage129Exit`, `FlushToPath`; Stage141 additionally exposes `FreeThreadState`, `State`, `HealthToPath`. A flush SRW lock orders IO, **not lifetime/entry admission**. Stage141 FIFO `sequence` fix does not solve DLL unload.
5. **No dynamic proof** of complete installed 2/2 Stage10 and 3/3 Stage129 ownership, all hook overlap sites, EIP-in-callee frames, native ABI/SEH/nonlocal unwind, caller capture, complete FLS teardown or crash-safe writer semantics. Candidate `origin(0x27)=UNKNOWN`.

## Evidence / tests completed now

- **102/102 static assertions PASS**: 11 reference file SHA256/length checks, 38 surface-line anchors, 70-file capture metadata/sampled-source comparisons, source-specific hook/worker/FLS/lease checks, source-only gate.
- **6/6 Python model unittests PASS**, including exhaustive **4096/4096** booleans for a *synthetic* 12-obligation shutdown eligibility policy and the counterexample where a raw native caller bypasses a fully drained cooperating test host.
- **70/70** original archived source-file hashes verified separately from the complete extracted capture (not all 70 copied into Stage144).
- **No Windows MSVC/native Stage144 test exists or is claimed**: this is a static audit. Read-only Windows current-tree drift check **NOT RUN** as of this checkpoint.

### Windows local source drift check (read-only, no game process)

Extract ZIP, open PowerShell 5.1 in `Stage144_SourceAudit\tests`:

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
.\Test-Stage144LocalSnapshot.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Expected: five `STAGE144_SOURCE_MATCH` rows then `STAGE144_SNAPSHOT_MATCH files=5`, **only if** current files exactly match **Nov 12 source snapshot**. If files changed: `STAGE144_SOURCE_DRIFT` / `STAGE144_SOURCE_MISSING` with exit code 2: require new source capture before assuming exact current code. This checker is **not** a live memory/hook scan and does not open, launch, inject or modify the game/DLL.

### Next Stage145

Source-only **shutdown obligations ledger + fail-closed inbound native call contract / orphan game-call adversarial model**, grounded in a *fresh user source snapshot if the Windows hash comparison reports drift*. No automatic runtime patch, no gameplay write, no new JMP, no `Arm()` enabling. The five native hook owners are conditional and need actual source+live readback before any integration.

**SAFE_TO_INSTALL=false**; preserve Stage129 Windows-tested `dinput8.dll` **2,099,200 B**, Stage122 rollback **2,089,984 B**, Stage36 OFF.
