# ArcanaAIOverhaul — Stage157 r3 Stage38 branch-structure validation (2026-11-14)

**SOURCE_ONLY. r3 portable offline tests PASS (32/32); user's current Windows r3 run PENDING. Game DLL installation and unload remain FORBIDDEN.**

## User-provided current Windows Stage38 source: exact branch facts

User pasted actual `D:\INCUBATOR\ArcanaAIOverhaul\src\Stage38CallbackWindow.inl` lines **325–353**. The relevant control flow is:

- **325–329** nested `if(InstallStage54Hooks())`, Stage55, Stage56, Stage57, then **`if(InstallStage58Hooks()){`** at **329**.
- **330–332** comment says Stage65 independent, and the call `InstallStage65Hooks();` is made without its return value gating Stage59.
- **333–345** `const bool stage67Ready=InstallStage67Hooks();`; guarded Stage70; guarded Stage76, Stage78, Stage79 then Stage80. Their return-false outcomes are gates for those nested additional installs.
- **346** `StartStage59Watch();`, **at Stage58 body level** after all optional nested braces close.
- **347–350** outer `else` logs errors if Stage58 or Stage57 failed.

**Precise conclusion:** For normal-returning callees and the source fragment shown, Stage59 is attempted after Stage58 succeeds irrespective of boolean failure values from Stage65/67/70/76/78/79/80. Stage59 does not lie inside `if(stage67Ready)` or `if(stage70Ready)`. **Do not claim unconditional runtime execution**: exceptions, abnormal exits, infinite waits and alternate preprocessors are not proven absent, nor that every earlier installer succeeds.

Stage157 r2's failed `stage38_installs_then_starts_stage59` regex expected a one-line, *unbraced* `if(InstallStage58Hooks())StartStage59Watch();` matching historical Stage59 r2. The user's current **braced, optional-stage** source is not a match for that regex. **This is a source-pattern limitation, not a demonstrated game logic or DLL failure.**

## r3 limited structural scanner fix

- Based on previous r2 reader with all nine source worker callsites, Stage59 11 unchanged source signatures, VEH sparse counter contract, and fail-closed CLI.
- Replaces the **single** Stage38 direct unbraced regex with a bounded source-text bracket-depth/order check. Checks exactly one Stage58 success `if` and one Stage59 call, balanced enclosing Stage58 block, direct depth=1 Stage59 statement, seven optional stages present in expected order and levels, no detected earlier source-level return/goto/break/throw/continue or preprocessor branch in the inspected span.
- Historical immediate/unbraced Stage38 form remains supported; code comments and ordinary strings are masked so fake calls/braces do not pass. Missing optional stage, reordered calls, duplicate Stage59, Stage59 nested inside Stage67/70/78, moved outside Stage58, or unbraced conditional before Stage59 produce metadata-only REVIEW_REQUIRED.
- **Bounded lexical C++ parsing is not a compiler:** nonliteral macros, ABI/SEH, callee internals, external/raw callers, callback retirement, worker quiescence, hook rollback, loaded-owner lifetimes remain unverified. A static PASS does not license game DLL unload.
- Read-only source and metadata inputs, output ONLY to `%TEMP%\ArcanaAI_Stage157_SourceOnly\stage157_worker_lifecycle_metadata.json`, no original private source text copied into artifacts.

## Actually completed offline package integrity

- ZIP `ArcanaAI_Stage157_r3_Stage38_StructuralLifecycle_SOURCE_ONLY.zip`, **16,368 bytes**, actual **SHA256 `3df439d0066516b5efa9db69c7e7dc03442c1f5eea39ad3649bcb0fc487cd757`**, **6 archive entries** (five payload + manifest), ZIP CRC PASS, SHA manifest **5/5 PASS**.
- Actual local Python unittest `Ran 32 tests ... OK`, **32/32 PASS**, includes 18 r2 regressions + 14 new bounded branch checks and denials. Test runs use synthetic fixtures only, not real game DLL.
- No Windows r3 run has occurred in the user-provided evidence. Do not imply r3 current-source PASS until the actual output is supplied.

## Next Windows read-only check

Unpack ZIP, in its `Stage157_r3_SourceOnly` directory:

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage157Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Expected, **if source matches**: `STAGE157_STAGE38_BRANCH shape=BRACED_STAGE58_DIRECT_STAGE59 structure=MATCH reason=MATCH`, `STAGE157_STAGE59_SOURCE_SIGNATURES matched=12 total=12`, `STAGE157_NINE_WORKER_SITES_RECONCILED=YES`, `STAGE157_VEH_REMOVE_SPARSE_COUNT candidates=0 count_consistent=YES`, final `STAGE157_R3_LOCAL_SOURCE_ONLY_TESTS_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false`. Output is metadata only and still forbids module unload.

## Safety and future

**`SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, origin(0x27)=UNKNOWN.** Stage59 worker start still closes its only initial handle without demonstrated join; worker's thread-watching handles are distinct, and Stage59 intentionally keeps its VEH registered until process exit. Other eight worker stop/join proofs and all raw native callback ownership/SEH/unwind/rollback proof remain UNKNOWN. Preserve Stage129 working game DLL **2,099,200 B** and Stage122 fallback **2,089,984 B**; no game process access or DLL installation/FreeLibrary in Stage157 r3.
