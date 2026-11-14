# ArcanaAIOverhaul — Stage163 source-only Stage67/70 fault-state matrix and rollback obligations (2026-11-14)

**CURRENT STATUS: Stage163 user Windows SOURCE_ONLY PASS (55/55 tests, 6/6 manifest, 3/3 current source SHA and 31 source-path scenarios); real installed-byte rollback remains NO/UNPROVEN.** Last actual Windows source audit is now Stage163. Stage162 upstream PASS is documented in [Stage162 checkpoint](STAGE162_CHECKPOINT.md).

## User Windows Stage163 current-tree confirmation — PASS (2026-11-14)

User executed `Invoke-Stage163Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'` from the extracted Stage163 SOURCE_ONLY package. **This is user-supplied PowerShell console evidence, not an uploaded full Stage163 JSON and not runtime fault injection.**

```text
STAGE163_MODE SOURCE_ONLY READ_ONLY NO_GAME_DLL_ACCESS NO_NATIVE_BUILD
STAGE163_MANIFEST_PASS files=6
Ran 55 tests in 0.187s
OK
STAGE163_UPSTREAM stage162_targeted=True bounded=12 blocked=15
STAGE163_TARGET stage=STAGE67 rva=AD75C bytes=5 writer_line=275 rollback=NO
STAGE163_TARGET stage=STAGE67 rva=E9E60 bytes=6 writer_line=275 rollback=NO
STAGE163_TARGET stage=STAGE70 rva=156784 bytes=6 writer_line=231 rollback=NO
STAGE163_SOURCE_SHA_MATCHED 3/3
STAGE163_SCENARIOS modeled=31 evidence=SOURCE_PATHS_ONLY
STAGE163_METADATA %TEMP%\ArcanaAI_Stage163_SourceOnly\stage163_fault_state_metadata.json
STAGE163_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
STAGE163_LOCAL_SOURCE_ONLY_TESTS_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
```

**Expected negative test diagnostics are not a failed actual audit:** tests 51 and 52 deliberately invoke rejected output paths and print `STAGE163_REVIEW_REQUIRED type=ValueError`; test 53 deliberately forges the Stage162 install safety flag, prints `STAGE163_SCENARIOS modeled=0` and `STAGE163_REVIEW_REQUIRED reason=STAGE162_UNSAFE_FLAG_SAFE_TO_INSTALL`, and asserts fail-closed behavior. All three tests report `ok`. The separate **final** real source preflight reports **31 modeled scenarios** and `STAGE163_TARGETED_SOURCE_ONLY_PASS`.

The actual current source tree passed the Stage163 pinned three-file SHA check, and the source-model witness relationships from Stage162 remained valid. **31 are static source-path/hypothetical fault scenarios, not 31 observed game failures.** Output contains no live installed-byte, ABI/SEH, original-byte rollback or owner-lease proof.

**Next Stage164 (not yet created or run)**: bounded SOURCE_ONLY transaction/rollback feasibility audit for the three write targets. Define per-site original-byte snapshot, protected write sequence/commit point, verify-on-write and restore-on-failure obligations, concurrency/EIP & foreign hook ownership conflicts, exception/SEH cleanup, mismatch of install flags and return values, plus Stage38 admission and Stage59 behavior. Use a private test-double/fault injection simulator only; never touch a running game or the DLL. External native callers/trampolines, nine worker joins, three VEH retirements, module unload remain separate blocked proof obligations.

**Hard gates unchanged:** `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`; Stage36 OFF, origin(0x27)=UNKNOWN. Game DLL not modified by these source-only audits.

---

## Release artifact and reproducibility

- Chat artifact: `ArcanaAI_Stage163_Stage67_70_FaultState_RollbackObligations_SOURCE_ONLY.zip`.
- Size **16,486 bytes**; SHA-256 **`e654a8515b37f8fc6ddf9f6ed64fd133339b9063cb907ec555b91673063bfb99`**.
- ZIP 7 entries, CRC PASS; package SHA-256 manifest **6/6** PASS. Fresh extraction unittest **55/55 PASS** (includes tampered Stage162 metadata, source hash drift, forged install/unload/rollback/owner/worker/VEH flags, missing/duplicate RVA, wrong bytes/API role/source line, malformed Stage38 evidence, input/output inside game project, synthetic exception pathways, deterministic sanitized report).
- Pure source-only Python auditor, PowerShell entrypoint, manifest verifier, 55 synthetic/negative tests, README and `STAGE163_MODEL_NOTES.md` with fault matrix. No original game C++, no native binaries, no source-tree modification, no process/DLL access or new hooks.

## Evidence boundaries

- Inputs: user-local `%TEMP%\ArcanaAI_Stage162_SourceOnly\stage162_targeted_write_order_metadata.json` (actually generated during user's Stage162 Windows PASS, but not uploaded to chat) plus only 3 **current** source files:
  - `src/Stage67NativePaths.inl` SHA256 `f919af18379d85594c442e021b559a298b5a068b06ce7ebefc33f0cde94d3620`
  - `src/Stage70DeferredWriter.inl` SHA256 `bf3becbf1893426002fb3dfee0eb849caf1836c2d42059379c40958b7995b8df`
  - `src/Stage38CallbackWindow.inl` SHA256 `d0ad34650a94102e6c9dfc8b74228cf841510d04a690739dd11933af2e51557a`.
- Stage163 refuses stale/forged Stage162 identity, unsigned/mismatched three-file provenance, rewritten 3 RVA witness records, missing 12/15 ledger, unexpected game access, or any proof escalation. No fresh 443-API broad scan.
- **Earlier historical local verification (now superseded by the user's Windows Stage163 current-tree PASS):** a local copy of uploaded archived Stage146 evidence snapshot plus the Stage162 snapshot preview produced `STAGE163_SOURCE_SHA_MATCHED 3/3`, `STAGE163_SCENARIOS modeled=31`, `STAGE163_TARGETED_SOURCE_ONLY_PASS`. This is not runtime installation proof.
- All 31 scenarios are **source-control-flow or explicitly hypothetical fault injections** in a model, **not** game events, executed Win32 fault injection or partial-memory-copy observations.

## Source-path conclusions and detailed failure classes

| Source site | Ordered writer operations | Important failure outcome |
|---|---|---|
| Stage67 `AD75C` 5 B, `E9E60` 6 B | `src/Stage67NativePaths.inl` 270–271 both `VirtualProtect`; `memcpy(a,pa,5)` then `memcpy(b,pb,6)` (275); flush 276–277; install flag 2 (278); restore protection **b then a** 281–282; resume/unlock 283; error decision 285–287 | On normal-path flush/restore failure, byte writes and installed flag remain, **return false**, no byte rollback. Hypothetical exception between copies can leave only AD75C patched and skip normal cleanup. If b protection request fails, normal path writes neither site, but a protection may remain uncertain if restoration fails. |
| Stage70 `156784` 6 B | `src/Stage70DeferredWriter.inl` protection 227; memcpy 231; flush 232; flag 1 (233); restore 235; resume/unlock 236; error decision 241–243 | A flush or protect-restore failure **after** patching returns false with installed flag 1 and no byte rollback. Hypothetical mid-copy exception can leave partial bytes and unproven cleanup. |
| Stage38 call gate | `src/Stage38CallbackWindow.inl` 333 Stage67 result, 334–335 Stage70 call iff result true, 346 Stage59 unconditional **inside Stage58-success outer block** | Stage67 **ordinary false** (even after bytes changed) skips Stage70, but Stage59 remains source-path reachable. Stage70 ordinary false also does not prevent Stage59. After an uncaught exception, neither subsequent call/reachability nor cleanup is guaranteed. |

- Two distinct failures: normal Win32 API **return failure** still follows cleanup calls, while C++/SEH/memory-copy **fault** has no proved local exception recovery/finally and may abandon ordinary cleanup; a modelled exception is not an observed exception. Success of a call `ResumeThreads` or `ReleaseMutex` is **not checked/proven** by Stage163.
- Stage67's second `FlushInstructionCache` attempt is executed even when the first returns failure (the second call appears on LHS of an `&&` whose RHS is the prior `cacheOK`).
- A filesystem DLL backup, `VirtualProtect` restoring page rights, and a source installed flag do not restore original instruction bytes. An installed flag being 2 or 1 despite returning `false` is a source-level state mismatch, not live owner proof.
- Output is sanitized fixed-schema JSON. On upstream/hash mismatch, `REVIEW_REQUIRED` and **no failure matrix released**. No original source text, private identifiers/patch bytes or local source paths copied to report.

## Next read-only Windows action

From extracted `Stage163_SourceOnly`:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage163Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Default output `%TEMP%\ArcanaAI_Stage163_SourceOnly\stage163_fault_state_metadata.json`, **outside project**. Expected only if current sources and Stage162 metadata are unchanged: manifest 6/6, 55/55 tests, 3/3 target SHA and 31 source model scenarios; do not preclaim Windows PASS.

## Future gates

Next after Stage163 Windows evidence: **Stage164 SOURCE_ONLY transaction/rollback design feasibility audit**. Prove original-byte snapshot, exclusive owner/foreign hook collision handling, safety to patch/restore while EIP/foreign raw callers exist, verified rollback of each site under a controlled test double, callback/thread/module lifetime, failure cleanup and Stage38 admission contracts. Any real production patch remains **explicitly out of scope** until all safety gates proven.

**Absolute current gate:** `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, origin(0x27)=UNKNOWN. Native foreign caller/trampoline ABI/SEH/unwind, all nine worker joins, all three VEH retirement, installed exact byte owner, rollback and module lease unproven. Working Stage129 (2,099,200 B) / Stage122 fallback (2,089,984 B) DLLs unchanged.
