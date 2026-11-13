# ArcanaAIOverhaul — Stage 148–151 checkpoint (2026-11-13)

**Scope:** offline, source-only patch/transaction audit. These are results reported by the prior chat's generated audit packages, **not Windows game-DLL runtime validation**.

## Source snapshot / provenance baseline

- User's Windows Stage146 evidence: `ArcanaAI_Stage146_Evidence_493fc20dff504bdabc3abc4924a9426f.zip`, from `D:\INCUBATOR\ArcanaAIOverhaul`. Stage147 independently checked 85/85 copied source/project files by SHA-256+size; Stage146 collector reported 70/70 historical Stage134 records matching, zero drift/missing; additional paths included backups and config/state metadata.
- Actual .vcxproj has eight compile roots; literal include graph reaches 66 files. Dormant Stage83 writer is **not currently in that graph**; Stage84 is.
- None of these findings proves the owner of a detour in the **loaded** game DLL.

## Stage148 — source-only patch site/collision atlas

Package (chat artifact): `ArcanaAI_Stage148_SourceOnly_InstallerSiteCollisionAtlas_r1.zip`; report `ArcanaAI_Stage148_Audit_RU.md`.

- 131 static patch intervals, 53 source files, 181 follow-up write/call candidates; 85/85 source hashes match.
- Five source interval overlaps: four Stage83/Stage84 locations (`F3DE9`, `F3E02`, `F3F50`, `F3F69`; conditional because Stage83 dormant); one Stage36/Stage82 at `F79E4` (Stage36 must remain OFF).
- A source address reference is **not** proof of a runtime patch writer. Offline local test report: 6/6 PASS.
- Stage95 must check its expected original signature before Stage96 modifies the associated site.

## Stage149 — write/read dependency and transaction audit

Package: `ArcanaAI_Stage149_SourceOnly_WriteReadDependency_TransactionAudit_r1.zip`; report `ArcanaAI_Stage149_Audit_RU.md`.

- 181/181 source-call candidates classified; 307 total `memcmp` observations, 217 possible game-address signature checks (candidates, **not** confirmed installed signatures). 30 dependency graph edges, 11/11 offline tests.
- Stage80→Stage81 and Stage95→Stage96 order requirements retained.
- Stage67 appears to modify two hook sites and may return false on `FlushInstructionCache`/protection-restoration failure **after** writes, without demonstrated restoration in that path.

## Stage150 — Stage67/70 rollback-fault and site witness audit

Package: `ArcanaAI_Stage150_SourceOnly_Stage67_70_RollbackFault_SiteWitnessAudit_r1.zip`; report `ArcanaAI_Stage150_Audit_RU.md`.

- 11 source-level address→write witness mappings across Stage42, Stage67, Stage70, Stage95, Stage96; not live-memory owner proof. 17/17 offline tests.
- Stage67 writes `AD75C` and `E9E60`; Stage70 writes `156784`. Install flags may be set before all verification is complete.
- Stage67 fault model identifies four failure returns after both patches were applied, and two pre-write protection-restoration failure scenarios. Stage70 has a similar potential post-write failed return without byte rollback.
- In observed source control flow Stage67 false skips Stage70, but may not stop Stage59 along the parent path. This is a *source-level path*, not proof of runtime crash.

## Stage151 — isolated transaction/rollback/quarantine model

Package: `ArcanaAI_Stage151_SourceOnly_TransactionRollback_QuarantineFaultModel_r1.zip`; report `ArcanaAI_Stage151_Audit_RU.md`.

- Isolated model for two-site Stage67 and one-site Stage70, with fault injection during installation and rollback; reported **173** reached scenarios: 2 model successes, 70 verified clean rollback, 101 quarantine. Offline suite 30/30 PASS.
- State model distinguishes *ACTIVE*, *verified clean abort*, and *QUARANTINED*. Unknown memory bytes, unsuccessful rollback, uncertain page protection, or indeterminate thread resumption **must not be reported as clean**.
- Existing `ResumeThreads` callsite does not provide checked result in reviewed code; a transaction design must not declare success if thread state is unproven.
- No production hook write or game DLL integration in Stage148–151.

## Safety boundary / follow-up

- `SAFE_TO_INSTALL=false`, `Stage36 OFF`, `origin(0x27)=UNKNOWN`, no new gameplay writes, no install into AALib/game.
- Preserve previously tested Stage129 2,099,200-byte dinput8.dll and Stage122 2,089,984-byte rollback. Live hook-owner proof, full native ingress/ABI/SEH/FLS/worker shutdown coverage, safe DLL unload and native production rollback are still unproven.
- Stage152 package proceeds to **private/self-owned Win32 x86 harness**, not game injection; see [Stage152 checkpoint](STAGE152_CHECKPOINT.md) for the user's actual Windows native results.
