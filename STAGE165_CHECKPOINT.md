# ArcanaAIOverhaul Stage165 — native preimage/owner gap audit (2026-11-14)

**CURRENT STATUS: User Windows Stage165 SOURCE_ONLY PASS (7/7 manifest, 55/55 tests, 3/3 current SHA, 25 source witnesses, 12 open gaps).** Stage164 remains the verified upstream. This source-only audit does not access or modify the game DLL, process, or original C++ tree.

## User Windows Stage165 current-source audit — PASS (2026-11-14)

The user executed `Invoke-Stage165Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'` from the extracted SOURCE_ONLY package. **The following are actual user-pasted Windows PowerShell console markers**, not an uploaded full Stage165 JSON and not a game-process/runtime experiment:

```text
STAGE165_MODE SOURCE_ONLY READ_ONLY NO_GAME_DLL_ACCESS NO_NATIVE_BUILD
STAGE165_MANIFEST_PASS files=7
Ran 55 tests in 0.739s
OK
STAGE165_UPSTREAM stage164_pass=True test_double_cases=65
STAGE165_SOURCE_SHA_MATCHED 3/3
STAGE165_PROOF_GAPS open=12 source_witnesses=25
STAGE165_TARGET stage=STAGE67 rva=AD75C bytes=5 preimage=NOT_SAVED_IN_REVIEWED_INSTALLER owner=UNPROVEN rollback=NO
STAGE165_TARGET stage=STAGE67 rva=E9E60 bytes=6 preimage=NOT_SAVED_IN_REVIEWED_INSTALLER owner=UNPROVEN rollback=NO
STAGE165_TARGET stage=STAGE70 rva=156784 bytes=6 preimage=NOT_SAVED_IN_REVIEWED_INSTALLER owner=UNPROVEN rollback=NO
STAGE165_METADATA C:\Users\Noma\AppData\Local\Temp\ArcanaAI_Stage165_SourceOnly\stage165_preimage_owner_gap_metadata.json
STAGE165_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
STAGE165_LOCAL_SOURCE_ONLY_TESTS_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
```

**Interpretation:** manifest **7/7** and tests **55/55 PASS (0.739s)** on user's Windows source tree; Stage164 65-case upstream ledger verified, current SHA pins matched for **3/3** Stage67/Stage70/Stage38 source files; **25** source witnesses and **12** unresolved proof gaps are consistent with the reviewed installer scope. For all three 5/6/6-byte RVA targets, the analyzed installer does not save live preimage bytes in an independent rollback snapshot; owner is UNPROVEN and production rollback is NO. This does **not** assert no snapshot elsewhere in the codebase, nor demonstrate live bytes, writer provenance, foreign module ownership, trampoline/ABI/SEH or callback quiescence.

**Current state:** Stage165 user Windows SOURCE_ONLY audit is complete, but **no installation or unload is authorized**. `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, origin(0x27)=UNKNOWN. No game DLL/source mutation reported. The Stage165 JSON remains on the user's Windows system; only console output is present in this conversation.

**Next Stage166 (not built or executed):** narrow SOURCE_ONLY native/raw caller ingress and hook-owner lifetime gap review, reusing Stage165 hash pins and earlier Stage156–164 inventories: identify real producer/owner of each native entry, 9 worker stop/join paths, 3 VEH registrations/removals and in-flight callback retirement, trampoline EIP/ABI/SEH/unwind, Stage38→Stage59 gating, and external module lease. Every ungrounded owner/retirement claim remains UNKNOWN; no broad API scan and no live game access.

---

## Artifact, integrity and local evidence

- Conversation ZIP: `ArcanaAI_Stage165_PreimageOwnership_RollbackGap_SOURCE_ONLY.zip`, **17,861 bytes**, SHA256 **`c19cfab7f43235cf0bb83d9e4a9cc23b1752b41c0087fb82f732abeebdba7b64`**; **8 ZIP entries CRC PASS**, **7/7 SHA+size package manifest PASS**.
- **55/55 offline synthetic and adversarial Python unittest PASS**, including 55/55 rerun after clean ZIP extraction. Tests cover exact Stage164 ledger, forged safety claims, source SHA drift, missing sources, line witness drift, installer-scope changes, missing/different original-buffer/restore helpers, fail-closed in-project paths, sanitization and deterministic output.
- A **historical Stage146 source snapshot** (previously uploaded) matched all three pinned SHA and showed **25/25 narrow source lexical witnesses** and **12/12 still-open safety proof gaps**. The upstream Stage164 sanitized preview JSON matched its exact deterministic canonical ledger SHA. This preview is **NOT** a Stage165 Windows current-project execution, nor independent live ownership proof.
- ZIP includes only a Python source-only analyzer, PowerShell wrapper, README, safety notes, SHA manifest verifier, synthetic test cases, and a *sanitized artificial upstream JSON fixture*. **No original private C++ source files, EXE, DLL, original instruction-byte dump or game binary included.** No native build, process attach, mutation or unload.

## Exact target and scope

| Stage | RVA | Length | Writer source line | Preimage/owner/rollback classification |
|---|---|---:|---:|---|
| 67 | `AD75C` | 5 B | `src/Stage67NativePaths.inl:275` | no saved live original-byte snapshot observed **in reviewed installer**, owner UNPROVEN, production rollback NO |
| 67 | `E9E60` | 6 B | `src/Stage67NativePaths.inl:275` | same |
| 70 | `156784` | 6 B | `src/Stage70DeferredWriter.inl:231` | same |

Pinned source digest requirements are unchanged from Stage162–164:

- `src/Stage67NativePaths.inl`: `f919af18379d85594c442e021b559a298b5a068b06ce7ebefc33f0cde94d3620`
- `src/Stage70DeferredWriter.inl`: `bf3becbf1893426002fb3dfee0eb849caf1836c2d42059379c40958b7995b8df`
- `src/Stage38CallbackWindow.inl`: `d0ad34650a94102e6c9dfc8b74228cf841510d04a690739dd11933af2e51557a`

Stage165 uses exact upstream sanitized Stage164 model contract including method, 3 target ledger entries, 65 test-double outcomes (**2 commit, 10 abort-before-write, 31 verified synthetic rollback, 22 quarantine**), twelve prior proposed obligations, negative runtime flags and canonical sanitized metadata fingerprint `62ad55519c0ed946728b440145f4840425931190a899a9bba609c820a556d694` (a drift guard, NOT a cryptographic assertion of origin). The three current source files must match SHA, and 25 additional precise line witnesses are matched before any positive source-only audit report.

### Source-defined findings, not live observations

- Stage67 `S67OriginalOK()` at 210/213 compares live memory against **compiled expected signature constants**, repeated at line 273. Stage70 does the same at line 155 and line 229. Such comparison **is not a durable original-byte snapshot** for later rollback. Neither Stage67 nor Stage70 reviewed installer body visibly stores original live bytes in an independent rollback buffer or performs exact-byte restoration. This **is scoped to reviewed installers only**: other sources and third-party modules are outside the narrow check. A pre-recorded signature is not a proof of current byte ownership.
- Stage67 names a mutex at 262; Stage70 at 220. A shared name coordinates cooperating users only, not any noncooperating foreign hook owners; source-level EIP-range checks at Stage67 236–237/Stage70 179–180 do not establish full future native/raw/trampoline quiescence, protect from ABA, or establish an exclusive byte lease.
- Stage67 copies AD75C/5B then E9E60/6B at line 275, sets install flag 2 at 278 and can return false after flush/protection failures (285–287), without original-code rollback. Stage70 sets flag 1 at 233 and can return false after patching (241–243). Both installers call `ResumeThreads`/`ReleaseMutex` at Stage67:283/Stage70:236 without verifying returned status in these bodies; no complete exception-safe cleanup proof.
- Stage38 branches on the **return value** of Stage67 at 333–335, so Stage70 can be skipped even with Stage67 patch bytes already written and installed flag set; `StartStage59Watch()` at 346 remains reachable after ordinary return. Future callback/worker/VEH retirement, foreign ingress and safe module ownership cannot be promoted from syntax to runtime proof.

### Twelve blocked proof obligations

1. Live exact preimage snapshot in reviewed installers not observed.
2. Original instruction bytes restore path in reviewed installers not observed.
3. Signature comparison is not an atomic exclusive owner lease.
4. Named mutex cannot exclude noncooperating foreign writers.
5. Bounded EIP scan is not exhaustive quiescence.
6. Native/raw caller, trampoline ABI/SEH/unwind proof missing.
7. Partial-copy exception-safe recovery not established.
8. Install flag may be published before a false return.
9. Peer resume/unlock results not checked in reviewed installers.
10. Stage38 Stage59 start not gated on transaction indeterminate state.
11. Nine worker joins / three VEH callback retirement not proven.
12. External module lease / installed-byte owner not proven.

**The audit can PASS while all twelve remain OPEN**: PASS means that the *source-scoped negative and positive witness classifications* match pinned evidence, not that hook installation is safe.

## User Windows command (successfully executed)

Unzip outside game project, enter `Stage165_SourceOnly` and run:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage165Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Default input: `%TEMP%\ArcanaAI_Stage164_SourceOnly\stage164_rollback_feasibility_metadata.json`.
Default output: `%TEMP%\ArcanaAI_Stage165_SourceOnly\stage165_preimage_owner_gap_metadata.json`.

User Windows results confirmed the following expected source-only baseline: `STAGE165_MANIFEST_PASS files=7`, 55/55 tests, `STAGE165_SOURCE_SHA_MATCHED 3/3`, `STAGE165_PROOF_GAPS open=12 source_witnesses=25`, and `STAGE165_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false`. If any SHA/upstream/witness drift, status is `REVIEW_REQUIRED`, **with no positive target/proof ledger released**. Do NOT blindly update hashes to bypass changes.

## Next phase

With actual user Windows Stage165 PASS now recorded, Stage166 may carry out a bounded **SOURCE_ONLY** ownership/caller/worker-VEH lifetime gap inventory, with small source windows, no broad unscoped scan, no real hook installation, no project code change, no unload. A safe rollback implementation needs separate explicit review, actual byte owner lease evidence and quiescence guarantees. Existing working Stage129 DLL (2,099,200 B) and Stage122 backup (2,089,984 B) unchanged.

**HARD GATES:** `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`; Stage36 OFF, origin(0x27)=UNKNOWN, no game DLL access or native build.