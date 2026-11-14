# ArcanaAIOverhaul Stage169 — Source-to-Runtime Safety Evidence Gap Review (2026-11-14)

**STATUS: Stage169 SOURCE_ONLY package BUILT and tested locally on the archived, hash-pinned source snapshot. USER WINDOWS Stage169 NOT YET RUN.** Last verified user Windows current-tree audit is Stage168 PASS (manifest 7/7, 166/166 unittest, 13/13 current source SHA, upstream Stage167 79 synthetic cases and Stage168 99 synthetic cases with 12 unresolved production proofs). See [Stage168 checkpoint](STAGE168_CHECKPOINT.md).

## Verified artifact and provenance

- User downloadable chat ZIP: `ArcanaAI_Stage169_SourceToRuntime_EvidenceGapMatrix_SOURCE_ONLY.zip`.
- **20,905 bytes; SHA-256 `cd86de5e05bfeff9742d7313908edbc4cc388b55c839970dcd10eeb57f9f3ad0`.**
- **8 ZIP entries CRC PASS; manifest 7/7 SHA+size PASS; 104/104 Python synthetic/adversarial tests PASS after fresh independent extraction** (0.201 s).
- Historical Stage146 archived 13-source snapshot preview: **13/13 source SHA pinned**, **47/47 exact line witnesses**, **12/12 source-to-runtime evidence gap entries OPEN**, `runtime_artifacts=0`; `STAGE169_TARGETED_SOURCE_ONLY_PASS`. This preview is **not** a Stage169 Windows current-tree run or a game/process test.
- The exact Stage168 sanitized test JSON was produced using the released Stage168 Python on the archived source and has stable canonical SHA-256 `9c9456bc0b8d29afd1515211bd6cd1890eee6c5b3268741d5a1c3bb61199ca2c`. Stage169 uses it solely as an exact deterministic ledger/drift check, not cryptographic evidence of real runtime ownership. The user's actual Stage168 JSON remains on Windows at `%TEMP%\ArcanaAI_Stage168_SourceOnly\stage168_callback_module_lease_metadata.json` and has not been uploaded.
- ZIP contains Python source-only auditor, PowerShell wrapper, README, 12-gap notes and acceptance matrix, manifest checker, synthetic/negative tests and **artificial sanitized Stage168 fixture**. It contains **no original game C++ file, DLL/EXE, native patch bytes, real process data, runtime hooks, binary writers or unload actions**.

## Evidence-based pivot: NOT more synthetic success counts

Stages 164, 167 and 168 demonstrated controlled *test-double* scenarios. Stage169 deliberately does **not** produce a new synthetic retirement success counter. It promotes **zero** production proof obligations. The matrix explains for each unproven requirement the exact witnessed lines, necessary independent artifact and testable acceptance criterion. It explicitly tracks `independent_runtime_artifacts_provided=0` and `live_hook_or_unload_tests_executed=0`.

**47 precise source-line anchors** are checked in only the same **13 hash-pinned C++ sources** as Stages166–168 (including `Proxy.cpp`, Stage38/59/67/70 and nine worker starter sites). Findings have strict scope:
- Stage59 source lines **330–331** expressly say to keep VEH registered for process lifetime; at line 333 it cleans up *owned* debug-register watchpoints. This is inconsistent with treating VEH removal as already proven, but does not prove callback behavior outside the reviewed code.
- Nine scoped worker starters call CreateThread and locally CloseHandle; this is not a worker termination/join witness. Three scoped VEH registration sites are pinned; no verified all-three remove-and-inflight-drain sequence was produced by the source-only review.
- Stage67 `AD75C` 5B and `E9E60` 6B use memcpy writes at line 275; Stage70 `156784` 6B uses memcpy at line 231. Expected-signature memcmp is **not** saved actual live preimage. Normal postwrite false, installed flags and no restored exact original bytes in reviewed installer bodies still block safe rollback.
- A separate installer in `AICommandSiteTrace.cpp:1979` **does** save original call bytes for a different hook. This witness is included explicitly to avoid falsely asserting there are *no* saved preimages anywhere in the project.
- Stage38 line 346 can call Stage59 after ordinary Stage67 false; a versioned no-new-work or indeterminate-patch admission gate has not been proved. Proxy export and three naked native stubs are source witnesses, **not** a complete verified foreign raw-caller set or ABI/SEH/unwind proof.

## Twelve OPEN source-to-runtime gates (the delivered JSON gives full evidence requirements)

| ID | Evidence requirement | What acceptance requires |
|---|---|---|
| G01 | Real callback epoch gate | Versioned entry controls, negative stale/post-close entry evidence, active counter discipline |
| G02 | Real stop/ACK/join for all nine workers | Retained join handles and independently verified actual exits, including failures |
| G03 | Three VEH unregister + in-flight drain | Verified removed handlers, no active or future callbacks |
| G04 | Raw caller lease and x86 trampoline ABI/SEH | Complete inbound source/owner ledger, stack/return/unwind proof |
| G05 | Stage38/Stage59 admission | Prevent new Stage59 work on indeterminate patch or shutdown state |
| G06 | Stage59 DR register ownership | All DR slot borrow/restore and foreign-owner arbitration independently observed |
| G07 | Real original byte snapshots and rollback | Per RVA exact preimages, rollback/flush/verify under partial failures |
| G08 | Foreign hook exclusion | Noncooperating owner collision detection and ABA-safe compare-and-restore |
| G09 | Future dispatch quiescence | All export/raw/worker/VEH deferred ingress denied after closure |
| G10 | External module lease | Borrower/reference inventory and independently proven complete release |
| G11 | Timeout and exception cleanup | Verified protected cleanup, resume/release, rollback or quarantine |
| G12 | Stop/restart epoch race exclusion | Negative concurrent Stage59 restart and callback ABA evidence |

Every gap remains `OPEN_NEEDS_INDEPENDENT_EVIDENCE`, `runtime_proof=NOT_TESTED`, `production_approval=false`. A source-only PASS is an accurate evidence matrix, **never** a safe install/unload decision. This is **not a global-source absence census**.

## User Windows command (pending)

Unzip outside the project; inside `Stage169_SourceOnly` run:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage169Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Inputs: user local `%TEMP%\ArcanaAI_Stage168_SourceOnly\stage168_callback_module_lease_metadata.json` and 13 current hash-pinned sources.
Output: `%TEMP%\ArcanaAI_Stage169_SourceOnly\stage169_source_runtime_evidence_matrix.json`.

On unchanged Windows inputs expected: `STAGE169_MANIFEST_PASS files=7`, 104/104 tests, `STAGE169_SOURCE_SHA_MATCHED 13/13`, `STAGE169_EVIDENCE_MATRIX open=12 source_witnesses=47 runtime_artifacts=0`, and `STAGE169_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false`. The user has **not** run this yet. Source/metadata drift leads to `REVIEW_REQUIRED` with **zero positive gap/witness rows**; do not just update pins to bypass review.

## After Stage169 Windows evidence — prioritized real proof collection, not endless synthetic escalation

Stop increasing toy scenario counts. Next work should collect **actual scoped source/design proof artifacts** and, only following separate explicit authorization, safe nonproduction harness evidence for (1) ownership and original-byte save/restore semantics, (2) worker stop/ACK/join + three VEH registration removal/in-flight retirement, and (3) full native/callback future-entry lease, Stage59/DR register and module lifetime. Anything requiring the actual game's runtime must remain **NOT TESTED** until separately approved and observed.

**Non-negotiable safety:** `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`; Stage36 OFF, origin(0x27)=UNKNOWN, no project/DLL/process access or native build by Stage169. Working Stage129 (2,099,200 B) and Stage122 fallback (2,089,984 B) DLLs unchanged.
