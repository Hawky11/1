# ArcanaAIOverhaul — Stage145 shutdown obligations / adversarial native ingress model (2026-11-13)

**SOURCE-ONLY / READ-ONLY. SAFE_TO_INSTALL=false. Stage36 OFF. No game DLL, hook, INI or project modification.**

## Source facts and predecessor

- [Stage144 checkpoint](STAGE144_CHECKPOINT.md): user ran read-only Windows PowerShell source verifier against `D:\INCUBATOR\ArcanaAIOverhaul`. **5/5 current files match** the Nov 12, 2026 source capture: `src/AIProducerContextTrace.cpp`, `ArcanaAIOverhaul.vcxproj`, `src/Proxy.cpp`, `src/Stage129ProducerProbe.inl`, `src/Stage81SelectorWriter.inl`. Script explicitly printed `STAGE144_NO_LIVE_OWNER_PROOF`. This is NOT a 70/70 **current** project comparison (70/70 belongs to archived snapshot), installed 2/2 Stage10 or 3/3 Stage129 live proof, or source validation for unrelated files.
- Stage144 inventory **38 selected** byte-hashed line-anchored surfaces (19 baseline game code / 13 undeployed Stage139/141 candidates / 6 Stage143 isolated test-only). Stage143 cooperative host lease, even after 16/16 native isolated DLL/host PASS, does not cover native game ingress at Stage10 F34D0/F3930 or Stage129 F30B0/F325A/F326A.

## Stage145 source-only deliverable

- Exact conversation ZIP: **`ArcanaAI_Stage145_ShutdownObligations_AdversarialNativeIngress_SourceAudit_r2.zip`**, **90,872 bytes**, SHA-256 **`d59c55d30524263994bf9e2e2aa432cb352a4afb1a540196790ec3374d5a208f`**, **34 files**, ZIP CRC PASS, per-file SHA-256/size manifest **33/33 PASS**. No EXE/DLL/OBJ/LIB/PDB/EXP/BIN/PYC, no temp `__pycache__`. Attached to ChatGPT, **not uploaded to GitHub as a binary**.
- A preliminary packaging pass inadvertently included two generated Python cache files; the **r2** archive is the corrected verified source-only deliverable and is the only file to use. Its static checker explicitly bans `.pyc` along with native binaries. The internal `evidence/package_manifest.json` excludes only itself from the self-hash list.
- **17 evidence obligations** in `evidence/shutdown_obligations.json` and `docs/SHUTDOWN_OBLIGATIONS_LEDGER_RU.md`, mapped to **all 38/38 bounded Stage144 surfaces** (not full live call census). Each is `UNPROVEN_OR_NOT_IMPLEMENTED_IN_GAME` and requires independently sourced evidence, not a user-flipped boolean.
- `docs/NATIVE_INGRESS_CONTRACT_RU.md` defines a **fail-closed theoretical admission/unload contract**, not a runnable native hook. Examples: raw native caller queued before instrumented C++ entry, parent trampoline on nested helper stack, untracked FLS callback, independent Stage10/Stage81 workers, sink flush/health, partial installer suspension.
- Python stdlib adversarial model `tests/model_shutdown.py` and `tests/test_shutdown.py`: distinguishes cooperative `host leases`, DLL in-flight, raw native pending/active, nested trampoline, installer/worker, sink IO, FLS, foreign callers, telemetry loss. **Closing host gate does not block raw native calls** in the actual unintegrated source model. Synthetic `native_gate_installed=true` exists only for a contrast unit test, not a claim about real game integration.

## Verified offline results (2026-11-13)

- **Stage145 static audit 92/92 PASS**, including 11 reference SHA+sizes, 38 line anchors, all 17 obligation statuses, all 38/38 mapped, no binaries and safety gate.
- **Stage144 static regression 102/102 PASS** against the same 11 unchanged source snapshots.
- **Stage145 14/14 Python model tests PASS**, covering **14 modeled adversarial cases** and exhaustive **65,536 / 65,536** 16-bit combinations of *claimed* shutdown proof flags.
- **Core counterexample:** host `leases=0`, DLL `inFlight=0`, but game `native_pending=1` or `trampoline_active=1` -> **BLOCK**. Additional scenarios: worker still active, FLS slot or callback, pending sink write, unknown raw caller, nonzero loss accounting, host gate still open.
- If the synthetic mask has all 16 bits and all modeled counters are zero, the verdict is **`ELIGIBLE_FOR_ISOLATED_TEST_ONLY`**. The model **never returns `SAFE_TO_INSTALL` or `SAFE_TO_UNLOAD_GAME_DLL`**, and 16 asserted booleans cannot establish live runtime proof.

## Windows local read-only evidence check — NOT YET RUN

Unzip Stage145 r2, open Windows PowerShell 5.1 in `Stage145_SourceOnly\tests`:

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
.\Test-Stage145LocalEvidence.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

The script verifies all **11 archive reference** SHA-256/size pairs, five **current Windows project** SHA-256/size pairs, and the ledger's 38 surfaces / 17 obligations. It is read-only: no game process access, no Build.bat, no attachment, no injection, no DLL load/unload. If mismatched: fail-closed `STAGE145_LOCAL_EVIDENCE_FAIL` and exit 2.

Expected **only** (not observed): `STAGE145_LOCAL_EVIDENCE_PASS refs=11 current=5 obligations=17; source-only, SAFE_TO_INSTALL=false`. This test does not verify game hook ownership, installer/worker runtime state, or native ABI/SEH.

## Safety blockers / next Stage146

- Native game→detour ingress has **no installed complete admission gate**. Host test lease counters cannot count a native thread before it reaches C++; `EIP` outside a detour does not rule out parent frame inside nested helper.
- Stage10 installer and Stage81 flush/health workers need independently proven stop+join and no late new threads; FLS callbacks need complete registry/code pin and controlled teardown outside loader lock; detached callbacks/raw exports/foreign pointers must be accounted.
- Actual owner byte readback (2/2 Stage10 and 3/3 Stage129) and conflict census, exact x86 ABI/SEH/nonlocal unwind, admission before code free, crash-safe sink semantics **UNPROVEN**.
- Next Stage146: source-only blueprint for **full current project owner/source/config inventory** and non-invasive dynamic *read-only* verification plan, not installation or runtime DLL unloading. Avoid new JMP, source auto-patching, `Arm()` or gameplay writes.
- **SAFE_TO_INSTALL=false; Stage36 OFF; origin(0x27)=UNKNOWN**. Preserve the Windows-tested Stage129 DLL **2,099,200 bytes** and Stage122 rollback **2,089,984 bytes**, unchanged.
