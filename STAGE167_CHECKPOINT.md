# ArcanaAIOverhaul Stage167 — worker join / VEH callback retirement protocol (2026-11-14)

**CURRENT STATUS: Stage167 USER WINDOWS SOURCE_ONLY PASS (7/7 manifest, 165/165 tests, 13/13 current-source SHA, 79 test-double cases, 12 open gaps).** Stage166 remains the verified upstream; no native build, game DLL/process access, production source edits, hook installation or unload.

## User Windows Stage167 SOURCE_ONLY — PASS (2026-11-14)

**Actual user-pasted PowerShell output** after running `Invoke-Stage167Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'` from extracted `Stage167_SourceOnly`:

```text
STAGE167_MODE SOURCE_ONLY READ_ONLY NO_GAME_DLL_ACCESS NO_NATIVE_BUILD
STAGE167_MANIFEST_PASS files=7
Ran 165 tests in 1.677s
OK
STAGE167_UPSTREAM stage166_pass=True worker_sources=9 veh_sources=3
STAGE167_SOURCE_SHA_MATCHED 13/13
STAGE167_PROTOCOL workers=9 veh=3 modeled=79 open_gaps=12
STAGE167_TEST_DOUBLE_OUTCOMES DRAINED_TEST_DOUBLE_ONLY=1,DENIED_BEFORE_RETIREMENT_TEST_DOUBLE=8,QUARANTINED_TEST_DOUBLE_UNPROVEN=70
STAGE167_METADATA C:\Users\Noma\AppData\Local\Temp\ArcanaAI_Stage167_SourceOnly\stage167_worker_veh_retirement_metadata.json
STAGE167_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
STAGE167_LOCAL_SOURCE_ONLY_TESTS_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
```

**Verified user Windows outcome:** 7/7 package manifest, **165/165 tests PASS** in 1.677 s, upstream Stage166 source-only ledger PASS (9 worker and 3 VEH starter sources), SHA **13/13 current sources**, **79 isolated test-double protocol cases** divided into one toy drained control, eight pre-retirement denial cases, and seventy quarantined states. **Twelve production lifecycle obligations remain OPEN.** No live worker was joined, no actual VEH handler was removed/drained and no game module was unloaded as part of this source-only run. The full Stage167 JSON remains only on the user's Windows machine; evidence supplied here is console text.

**Safety boundary:** a test-double `DRAINED_TEST_DOUBLE_ONLY=1` is **not** proof of runtime quiescence, worker joins, callback retirement, native/raw ingress exclusion, trampoline ABI/SEH/unwind, original bytes rollback or module lease. `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, origin(0x27)=UNKNOWN. Existing working and fallback game DLLs were not accessed or modified by this source-only audit.

**NEXT Stage168 (NOT YET CREATED/RUN):** bounded SOURCE_ONLY **Callback/Module Owner Lease Integration Specification**. Build explicit design and synthetic tests for versioned callback ingress epochs, admission closure before retirement, real join ACK invariants, in-flight VEH exit counters, module owner and raw caller leases, Stage38→Stage59 admission, rollback/preimage obligations, and crash/timeout `QUARANTINED` treatment. Reuse current Stage167 output and exact 13-file SHA pins; do not run game hooks, build a native DLL, modify C++ or grant unload permissions.

---

## Release package

- Chat artifact: `ArcanaAI_Stage167_WorkerJoin_VEHRetirement_Protocol_SOURCE_ONLY.zip`
- **17,107 bytes**, SHA-256 **`ca0229e1a06814b0a9abfdae338ffff1271544e4f6c9a02306b5dc2ea15d73ad`**.
- **8 ZIP members, CRC PASS; SHA+size manifest 7/7 PASS; 165/165 Python synthetic/adversarial tests PASS after clean extraction** (3.573 s in sandbox), with a separate source-only preview from historical Stage146 archived 13-source snapshot and Stage166's sanitized synthetic report: `STAGE167_SOURCE_SHA_MATCHED 13/13`, `STAGE167_PROTOCOL workers=9 veh=3 modeled=79 open_gaps=12`, `STAGE167_TARGETED_SOURCE_ONLY_PASS`. This preview is **historical artifact validation**, NOT the user's current Windows Stage167 execution or a real worker/VEH retirement.
- Contents: `Invoke-Stage167Audit.ps1`, `stage167_retirement_model.py`, readme + model notes, `verify_manifest.py`, 165-test unittest, sanitized Stage166 synthetic fixture JSON and manifest. **No private C++ source, DLL/EXE, original patch bytes, or native code included.**

## Stage166 upstream contract and source boundary

The Stage167 runtime-independent scanner requires the **user's local sanitized Stage166 JSON** at `%TEMP%\ArcanaAI_Stage166_SourceOnly\stage166_native_lifetime_metadata.json`. It validates its exact deterministic canonical JSON plus explicit key fields, nine worker IDs W01–W09, three VEH IDs V01–V03, 22 Stage166 source witnesses, 12 old open gaps and all negative runtime/safety flags. A canonical SHA of a metadata structure is a **drift guard**, not independently authenticated provenance. Then it hashes exactly 13 previously pinned current source files (defined in Stage166), with **zero broad scanning**.

The scoped Stage166 findings remain: nine specific CreateThread starter blocks locally close HANDLEs rather than await their termination; three registration files have AddVectoredExceptionHandler without locally evidenced handler removal and in-flight drain. This **does not prove** the absence of joins/removals elsewhere or show actual execution.

### Proposed shutdown phases (test-double only)

1. **Precondition gate:** require synthetic owner lease, foreign/raw caller quiescence, trampoline ABI/SEH/unwind contract, verified original-byte rollback, Stage59/debug-register transaction admission and future module dispatch lease. Missing anything => refuse before beginning.
2. **Close new ingress:** block Stage38/Stage59 new work and future native/VEH entry in synthetic epoch.
3. **Stop and join all nine workers:** preserve joinable handles, issue stop signals, demand explicit ACK, verified termination/join; dropping a handle with CloseHandle is not a join.
4. **Retire three VEH registration epochs:** bar new callbacks, remove registrations, wait for preexisting in-flight callbacks to exit (zero active counters). RemoveVectoredExceptionHandler alone cannot guarantee this.
5. **Release owner module lease:** only after no worker/VEH/native callbacks can execute and every patch restore obligation is satisfied. Late foreign ingress/Stage59 restart/lease failures => quarantine.
6. **Toy-only drained state:** successful synthetic run is NOT permission to unload real DLL.

**79 synthetic model cases, zero observed runtime events**:
- **1** `DRAINED_TEST_DOUBLE_ONLY`;
- **8** `DENIED_BEFORE_RETIREMENT_TEST_DOUBLE` for incomplete prerequisites;
- **70** `QUARANTINED_TEST_DOUBLE_UNPROVEN` = 45 per-worker failures (9 × 5), 18 per-VEH failures (3 × 6), 7 late lease/ingress/race failures.

Every case reports `game_install_approved=false`, `game_unload_approved=false`, real worker joins/VEH retirement unproven. The real production project contains no implemented Stage167 protocol.

## Test coverage and scope

165/165 tests cover every of 79 model scenarios independently; 13 source hash drift tests; 11 forged upstream safety flags; per-worker/per-VEH proof-spoof denial; canonical full-JSON tampering; fail-closed missing/symlink/unsafe files; stale metadata; missing/invalid/inside-project input and output paths; fake Stage166 JSON CLI result with **zero positive scenarios**, deterministic sanitized output, 9 worker joins not confused with 9 HANDLE closes, and no escalation on toy success. No game engine operations are performed.

**12 remaining Stage167 real-world requirements** include stop requests, stop ACK, retained join handles, Stage59 worker/debug-register retirement, new entry exclusion, three handler removals, draining in-flight callback epochs, future dispatch exclusion, native raw caller/trampoline safety, original-byte owner/preimage/rollback, external owner module lease, and exception-safe timeout cleanup.

## Windows SOURCE_ONLY run command (now executed successfully)

Unpack ZIP **outside** game project. In `Stage167_SourceOnly`:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage167Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Expected input: `%TEMP%\ArcanaAI_Stage166_SourceOnly\stage166_native_lifetime_metadata.json`.
Default sanitized output: `%TEMP%\ArcanaAI_Stage167_SourceOnly\stage167_worker_veh_retirement_metadata.json`.

The actual user Windows run has now confirmed `STAGE167_MANIFEST_PASS files=7`, 165 tests PASS, `STAGE167_SOURCE_SHA_MATCHED 13/13`, 9/3 worker/VEH, 79 synthetic cases (1/8/70), 12 open gaps, `STAGE167_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false`. On drift, fail closed with no positive 79-case model ledger. Never update pins blindly.

## Handoff after Stage167 user Windows evidence

Stage168 candidate: **SOURCE_ONLY retirement-state integration specification** for actual ownership/callback/worker/byte-rollback prerequisites, with limited source witnesses and a test-double versioned lease/epoch mechanism. No production patch or native build without a separate explicit go/no-go decision backed by independent runtime safety evidence. The existing tested Stage129 game DLL (2,099,200 bytes) and Stage122 fallback (2,089,984 bytes) remain unchanged.

**Absolute gates:** `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, origin(0x27)=UNKNOWN. Native foreign raw callers, trampoline ABI/SEH/unwind, nine real joins, three VEH in-flight retirements, external module lease and byte rollback remain NOT PROVEN.
