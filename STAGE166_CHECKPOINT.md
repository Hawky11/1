# ArcanaAIOverhaul — Stage166 native caller / worker / VEH ownership lifetime review (2026-11-14)

**CURRENT STATUS: Stage166 user Windows SOURCE_ONLY PASS (7/7 manifest, 96/96 tests, 13/13 current source SHA, 9 workers, 3 VEH, 22 witnesses, 12 open gaps).** Stage165 is the confirmed upstream. No game process or DLL access, native build or production source edits from this audit.

## User Windows Stage166 SOURCE_ONLY verification — PASS (2026-11-14)

The user executed `Invoke-Stage166Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'` in the extracted `Stage166_SourceOnly` directory. **This is user-provided PowerShell console evidence (not the full uploaded Stage166 JSON and not live runtime testing).**

```text
STAGE166_MODE SOURCE_ONLY READ_ONLY NO_GAME_DLL_ACCESS NO_NATIVE_BUILD
STAGE166_MANIFEST_PASS files=7
Ran 96 tests in 3.813s
OK
STAGE166_UPSTREAM stage165_pass=True witnesses=25
STAGE166_SOURCE_SHA_MATCHED 13/13
STAGE166_LIFETIME workers=9 veh=3 witnesses=22 open_gaps=12
STAGE166_METADATA C:\Users\Noma\AppData\Local\Temp\ArcanaAI_Stage166_SourceOnly\stage166_native_lifetime_metadata.json
STAGE166_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
STAGE166_LOCAL_SOURCE_ONLY_TESTS_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
```

All **96/96 Python tests PASS** (3.813s), **7/7 manifest**, current source pins **13/13**, Stage165 upstream validated (`witnesses=25`), nine worker start sites and three VEH registration source sites, 22 exact source witnesses, and **12 open lifetime/ownership proof gaps**. The test pass includes adversarial cases that would reject new worker waits, VEH removal, modified proxy detach, or spoofed source/metadata; these reject *unreviewed drift*, not future safe changes.

**Scope limit:** reviewed local worker starter blocks close thread HANDLEs but this does not prove the thread has joined; the reviewed VEH registration source files contain no locally established removal/retirement protocol. The static audit does not prove absence throughout the codebase, determine the complete native raw caller set, or establish module unload safety. No game process/DLL access or code mutation was performed by this audit.

**Next Stage167 (NOT YET BUILT/RUN):** SOURCE_ONLY stop/join + VEH in-flight retirement + callback/owner lease protocol *design and isolated state machine*. Define stop request, acknowledgement, joinable handle ownership, in-flight callback entry/exit counter, preventing new callbacks, handler removal, module lease/drain, and fail-closed timeout/partial retirement states. Explicitly model Stage59 watch worker and Stage38 admission. No native build, no production DLL/source modification and no game unload.

**Unchanged safety:** `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, origin(0x27)=UNKNOWN; installed byte owner/rollback, trampoline ABI/SEH and actual nine-worker/three-VEH retirement remain UNPROVEN.

---

## Artifact and verification

- Conversation ZIP: `ArcanaAI_Stage166_NativeCaller_WorkerVEH_OwnershipLifecycle_SOURCE_ONLY.zip`
- **17,844 bytes**, SHA-256 `7a5f3143d87836a23b08b263814b64e98da4d02665acb096fdf2fc77f08d7040`.
- **8 ZIP entries**, CRC PASS; **7/7 SHA-256+size manifest PASS**; **96/96 self-contained Python unittest PASS** from a fresh, separately extracted copy; exact source-only audit **PASS** on the archived Stage146 snapshot and separately reconstructed Stage165 sanitized test JSON: `STAGE166_SOURCE_SHA_MATCHED 13/13`, `STAGE166_LIFETIME workers=9 veh=3 witnesses=22 open_gaps=12`. This historical preview is **NOT** a Stage166 Windows current-project run.
- Contents: Python source-only auditor, PowerShell wrapper, synthetic/negative tests, a sanitized reconstructed Stage165 synthetic fixture, README/notes and SHA manifest verifier. **NO actual game C++, DLL, EXE, memory dump or live process data in ZIP**.

## Pin scope: 13 source files, not a full current-source census

Stage166 rehashes the following currently user-local files against previously uploaded Stage146 source snapshot pins:

| Role | File (src/) | SHA-256 |
|---|---|---|
| W01 | AICommandSiteTrace.cpp | `1d11bb063895a3530160fc9fdb81c979b6f59d9875b645da6ca759709a090395` |
| W02, V01 | AICommandTrace.cpp | `77d30020b14cd9b6b37062ebde0255cdc8a897440ec042d3ed1e69b9d4b22dc8` |
| W03 | AIParentFlowTrace.cpp | `1e3050313a25085c924d39a731ea71bf55d05ae7556b1f42ae8830c61054c850` |
| W04 | AIProducerContextTrace.cpp | `3a48575125c7da69338e4112812708648850e4724c0fd458a376929fdbb688ad` |
| W05 | AIQueueTrace.cpp | `965689e810c31355e57afe5197294287d81c7c28059f5adf26ca84a7cb340346` |
| W06, V02 | AITrace.cpp | `55ca8e99d5e3ceb16d5013f233f0d9d50bbfb63ff0d6101f2dd3c71254177977` |
| W07 | ComVsCom.cpp | `0599b9d4a3ea962b0c5119347b012d9f17d5ace43dd1eb5982d558bb8544ee0c` |
| W08, V03 | Stage59XWriterWatch.inl | `b1c282cf6089b522190606f59c015d7b328508c2c81bd170744ce43019e45296` |
| W09 | Stage81SelectorWriter.inl | `5dab6f6c6b688e0ca1831f453661034889a90ae9c3764f980910ee246b2ad90c` |
| Proxy ingress | Proxy.cpp | `4a43128e0e23a38f7354ca274ba4ed9c3134e8be745f22fa332482fe84e549cc` |
| Call gate | Stage38CallbackWindow.inl | `d0ad34650a94102e6c9dfc8b74228cf841510d04a690739dd11933af2e51557a` |
| Native stubs | Stage67NativePaths.inl | `f919af18379d85594c442e021b559a298b5a068b06ce7ebefc33f0cde94d3620` |
| Native stub | Stage70DeferredWriter.inl | `bf3becbf1893426002fb3dfee0eb849caf1836c2d42059379c40958b7995b8df` |

### Exact local source witnesses

- **Nine worker starter sites**: `CreateThread` in AICommandSiteTrace.cpp:2220, AICommandTrace.cpp:834, AIParentFlowTrace.cpp:854, AIProducerContextTrace.cpp:829, AIQueueTrace.cpp:3110, AITrace.cpp:2893, ComVsCom.cpp:242, Stage59XWriterWatch.inl:349, Stage81SelectorWriter.inl:550. Each of these local start blocks contains a nearby `CloseHandle(thread/h)` without local wait; immediate close **is not a join**, and cannot exclude a different thread retirement mechanism in another file. No global join-absence claim.
- **Three VEH registrations**: AICommandTrace.cpp:805, AITrace.cpp:2738, Stage59XWriterWatch.inl:292. No `RemoveVectoredExceptionHandler` call was found **within each of the three reviewed registration files**; does not claim global absence or no in-flight callbacks.
- **22 further precise anchored witnesses**: Proxy.cpp export/8 worker initializers/DllMain attach, Stage38:333–346 conditional Stage67→Stage70 and Stage59 after the inner decision, Stage59 source Stage58/once gates, Stage67 two `__declspec(naked)` hooks + code writer and Stage70 one hook + writer. An anchored lexical source witness is **not** a validated live callback graph, native ABI/SEH or patch ownership.
- **12 still-open Stage166 gates**: worker stop/join, immediate HANDLE closing versus thread completion, three VEH removals, in-flight retirement, proxy detach/lease, raw caller set, trampoline ABI/SEH/unwind, foreign byte owner, Stage67/70 preimage and rollback, Stage38/Stage59 transaction admission, Stage59 debug-register retirement, external module/future dispatch lease. See shipped `STAGE166_MODEL_NOTES.md`.
- The upstream Stage165 JSON must match the exact deterministic sanitized contract. Its canonical SHA check catches metadata drift; it is **not** cryptographic provenance proof. User actual Stage165 JSON has not been uploaded, so Stage166 must still validate the user-local file.

## Windows read-only command (successfully executed)

Extract ZIP *outside* `D:\INCUBATOR\ArcanaAIOverhaul`, open PowerShell in `Stage166_SourceOnly`, run:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage166Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Default upstream `%TEMP%\ArcanaAI_Stage165_SourceOnly\stage165_preimage_owner_gap_metadata.json`; output `%TEMP%\ArcanaAI_Stage166_SourceOnly\stage166_native_lifetime_metadata.json`. The user's Windows source audit has now confirmed manifest 7/7, 96/96 tests, SHA 13/13, workers=9, veh=3, witnesses=22, open_gaps=12 and `STAGE166_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false`. If a source hash/line or upstream ledger has changed, the script returns REVIEW_REQUIRED and clears positive witness ledgers. Never bypass SHA drift by changing pins without investigating.

## Subsequent work (proposal only)

After the confirmed user Windows Stage166 PASS: Stage167 **SOURCE_ONLY stop/join/VEH retirement protocol feasibility design**, with synthetic acknowledgement barriers and module callback lease model. No real game unload, no project C++ edit or game process access.

**Permanent hard gate until independent proof:** `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`; Stage36 OFF; origin(0x27)=UNKNOWN. Original game DLL (Stage129 working and Stage122 fallback) unchanged.
