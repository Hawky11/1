# ArcanaAIOverhaul Stage168 — callback epoch & module owner lease integration (2026-11-14)

**CURRENT STATUS: Stage168 USER WINDOWS SOURCE_ONLY PASS (7/7 manifest, 166/166 tests, 13/13 current source SHA, 99 synthetic epoch/lease cases; 12 open production gaps).** Previous Stage167 user Windows PASS remains the exact upstream. No game process, game DLL or production source files were accessed or modified by Stage168.

## User Windows Stage168 current-source execution — PASS (2026-11-14)

User executed `Invoke-Stage168Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'` from the extracted `Stage168_SourceOnly`. The following is **actual user-pasted Windows PowerShell console evidence**; the full Stage168 JSON remains user-local and was not uploaded here. No live game process/DLL instrumentation was performed.

```text
STAGE168_MODE SOURCE_ONLY READ_ONLY NO_GAME_DLL_ACCESS NO_NATIVE_BUILD
STAGE168_MANIFEST_PASS files=7
Ran 166 tests in 0.428s
OK
STAGE168_UPSTREAM stage167_pass=True modeled=79
STAGE168_SOURCE_SHA_MATCHED 13/13
STAGE168_PROTOCOL workers=9 veh=3 modeled=99 open_gaps=12
STAGE168_TEST_DOUBLE_OUTCOMES DRAINED_TEST_DOUBLE_ONLY=1,DENIED_BEFORE_ADMISSION_TEST_DOUBLE=9,QUARANTINED_TEST_DOUBLE_UNPROVEN=89
STAGE168_METADATA C:\Users\Noma\AppData\Local\Temp\ArcanaAI_Stage168_SourceOnly\stage168_callback_module_lease_metadata.json
STAGE168_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
STAGE168_LOCAL_SOURCE_ONLY_TESTS_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
```

**Verified source-only outcome:** manifest **7/7**, **166/166 Python tests PASS (0.428s)**, exact Stage167 upstream data (`stage167_pass=True modeled=79`), **13/13 current source SHA** and 99 **artificial** epoch/module-lease cases (1 toy drained, 9 pre-admission denied, 89 quarantined). **12 production proof gaps remain open**. The tests validate a proposed design under toy conditions, not actual worker joins, VEH removal, byte ownership, trampoline ABI/SEH or module unload.

### Stage169 next (not built/run)

Proceed with **SOURCE_ONLY Source-to-Runtime Safety Evidence Gap Review**, specifically *not merely another larger synthetic-state model*. Inventory the exact artifacts and independently testable source/runtime facts needed to promote each of the existing 12 proof gaps (raw caller/trampoline provenance, native byte preimages and exclusive hook owner, original-byte rollback under partial failure, worker stop/ACK/join, VEH removal and in-flight drain, Stage38/59 admission, debug-register ownership, future callback closure and external module lease). Where running code is necessary, label **NOT TESTED**, propose a constrained, isolated offline or nonproduction test plan, and require separate explicit approval before any live hook or DLL edit. Keep unknowns unknown; do not equate SHA-pinned source witnesses or toy-drained states with runtime safety.

**HARD DENY unchanged:** `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, origin(0x27)=UNKNOWN. Working Stage129 and fallback Stage122 DLL files unaffected by source-only audit.

---

## Verified release

- Conversation artifact: `ArcanaAI_Stage168_CallbackEpoch_ModuleOwnerLease_Integration_SOURCE_ONLY.zip`
- **16,340 bytes**; SHA-256 `240130f2066a22b3035ef9013f34895b0cbd6cb9f9ba457d7e23ec7e00752b06`.
- **8 ZIP entries / CRC PASS**; exact SHA/length manifest **7/7 PASS** after fresh extraction; **166/166 Python synthetic/adversarial unittest PASS** (clean extract: 2.394 seconds). ZIP contains only Python, PowerShell, markdown, manifest and a **sanitized artificial Stage167 metadata fixture**. It contains no original game source, private patch bytes, DLL or EXE.
- Independently read the user-uploaded Stage146 evidence snapshot: **13/13 archived source SHA pins match** Stage166/167 baseline, and Stage168's `assess` reports `STAGE168_SOURCE_SHA_MATCHED 13/13`, `STAGE168_UPSTREAM stage167_pass=True modeled=79`, 99 toy states (1/9/89), **12 open gaps**, `STAGE168_TARGETED_SOURCE_ONLY_PASS`. This is an **historical snapshot preview**, not a user Stage168 run or a new proof of live ownership.
- SHA-256 canonical drift guard for the exact synthetic Stage167 upstream report: `480b73c417a9996a4fc26205631a38eb1ffc5fbc0cec4910e9b9d9ae9cb0636a`. Rebuilt from Stage167's own released analyzer. The user's actual Stage167 JSON remains on their Windows machine and will be checked on execution; hash equality of JSON is a **drift guard**, not authenticated executable provenance.

## Scope and architecture

Stage168 reuses exactly 13 Stage166/167 source SHA pins and verifies the entire canonical sanitized Stage167 JSON ledger (9 workers, 3 VEH, 79 artificial cases, 12 open gaps, all live-safety flags false). It performs **no broad API scan** and no live code introspection. Any SHA/upstream ledger mismatch results in `STAGE168_SOURCE_REVIEW_REQUIRED` and **0 positive toy protocol scenarios**; do not bypass pins. Read-only checks never promote safety flags.

### Proposed purely synthetic epoch/lease protocol (not implemented in the game DLL)

1. Require independent evidence for external module lease, raw/native caller quiescence, trampoline ABI/SEH, original preimage+rollback, Stage59 admission, debug-register owner and future dispatch. Without these, **DENY** before retirement.
2. Close all new ingress and Stage38/Stage59 task admission, advance artificial callback epoch **41 → 42**; both old-token and post-closure callback entries are rejected by model.
3. Request stop, verify ACK and retain joinable handles until completion for all **9** worker sites. Closing a HANDLE is not proof of thread exit.
4. For **3** VEH registration sites, unregister and wait for every existing in-flight callback count to drain; deletion without active callback quiescence is insufficient.
5. Release synthetic owner/module lease only when all nine joined, all three removed/drained, no new native/callback/Stage59 dispatch, and ownership/rollback prerequisites hold. Late reentry, failed unregister, lost join handle, timed out stop ACK, callback counter underflow, ABA epoch race or exception ⇒ **QUARANTINED**.
6. Even the toy drained control is never approval to install, restore or unload actual game code.

**99 deterministic artificial cases** (none represents observed runtime behavior):

| Outcome | Count |
|---|---:|
| `DRAINED_TEST_DOUBLE_ONLY` | **1** |
| `DENIED_BEFORE_ADMISSION_TEST_DOUBLE` | **9** |
| `QUARANTINED_TEST_DOUBLE_UNPROVEN` | **89** = 9 × 6 worker faults + 3 × 7 VEH faults + 14 late lease/ingress/ABA/race faults |

All 99 cases report `game_install_approved=false`, `game_unload_approved=false`, `observed_game_events=0`, `real_worker_joins_proven=false`, `real_veh_retired_proven=false`.

**Still open:** production callback epoch gate, all nine stop/ACK/joins, all three VEH unregister/in-flight drain, raw/native and trampoline ABI/SEH, Stage38→Stage59 admission and debug-register owner, original-byte preimage/rollback, foreign hook owner exclusion, future dispatch quiescence, external module lease, exception-safe timeout cleanup, no ABA/restart races. `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`. Stage36 OFF, origin(0x27)=UNKNOWN.

## Windows SOURCE_ONLY run — completed

Extract the ZIP outside `D:\INCUBATOR\ArcanaAIOverhaul`; PowerShell in `Stage168_SourceOnly`:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage168Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Expected upstream: `%TEMP%\ArcanaAI_Stage167_SourceOnly\stage167_worker_veh_retirement_metadata.json`.
Output (outside project): `%TEMP%\ArcanaAI_Stage168_SourceOnly\stage168_callback_module_lease_metadata.json`.

Now confirmed by the user's Windows console: manifest 7/7, 166/166 Python tests, 13/13 source SHA, Stage167 79 toy cases, Stage168 99 toy cases (1 drained/9 denied/89 quarantined), 12 open gaps and `STAGE168_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false`. If drift, REVIEW_REQUIRED. No project source/DLL changes, native build, game access or module unload.

## Next after verified user Windows Stage168 PASS

Proposed Stage169 SOURCE_ONLY review of **missing real-world bridge** between artificial lease/epoch invariants and actual worker/VEH/raw-caller/native-ownership source control flow. Demand verifiable, source-anchored evidence before any production changes. Do not automatically continue endlessly with synthetic-only tests as a substitute for observed real safety. Existing working Stage129 and backup Stage122 game DLLs remain unchanged.
