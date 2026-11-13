# ArcanaAIOverhaul — Stage153 source-only native ingress / worker / VEH unload audit (2026-11-13)

**IMPORTANT: game DLL installation/unload remains BLOCKED.** Stage153 only read the pinned user Stage146 evidence ZIP, produced metadata and exercised a self-owned pure C++17 gate model. Windows **MSVC x86 Stage153 run NOT YET performed**. The user's Stage152 Windows x86 harness was separately confirmed PASS (see [Stage152](STAGE152_CHECKPOINT.md)).

## Artifact / provenance

- Chat ZIP: `ArcanaAI_Stage153_SourceOnly_NativeIngress_WorkerVEH_UnloadPreflight_r1.zip` — **23,611 bytes**, SHA-256 **`031d1a58229021768923da530d7e9bc95e338f4ed6c2c2f6fee08fc99edb1e0b`**, **13 members**, CRC PASS, manifest **12/12 PASS**. Standalone Russian report: `ArcanaAI_Stage153_Audit_RU.md`.
- No user source/INI/EXE/DLL, patch bytes, game processes or live debugger touched; only analysis code, metadata and synthetic tests packaged.
- Input was the user's Stage146 `ArcanaAI_Stage146_Evidence_493fc20dff504bdabc3abc4924a9426f.zip` SHA-256 `6d9c39d59b4883dab6cc4ea95f08deac56fe8c477fd6b4746bd1c764f96a207b`, 94 ZIP entries CRC PASS and 85 source records independently checked by path, size, SHA-256.
- Compile/include reconstruction: **8** original .vcxproj `ClCompile` roots, **66** literal-include reachable files, 67 local literal include edges. Not a complete preprocessor/loaded-native proof.

## Bounded source facts by anchor

1. **Worker lifetime (blocking)** — `src/Proxy.cpp:58–90` `DirectInput8Create` calls eight Start* paths on export. All eight contain `CreateThread` plus immediate `CloseHandle(thread)`; original start-call handle is **not retained for join at those sites**, and their worker bodies have `for (;;)` loops. The concrete sites, in Create/Close/loop order: `ComVsCom.cpp:242/248/215`, `AITrace.cpp:2893/2903/2850`, `AICommandTrace.cpp:834/840/824`, `AIQueueTrace.cpp:3110/3116/3028`, `AICommandSiteTrace.cpp:2220/2226/2158`, `AIProducerContextTrace.cpp:829/833/811`, `AIParentFlowTrace.cpp:854/858/836`, `Stage81SelectorWriter.inl:550/552/542`. Some workers may exit early if disabled; **no comprehensive stop/join+native ingress drain was proven**. Closing a handle is not stopping a thread.
2. **Detach (blocking)** — `src/Proxy.cpp:92–100`: `DllMain` has `DLL_PROCESS_ATTACH`, no `DLL_PROCESS_DETACH` branch in the 66-file graph. This does NOT mean joins should be inserted into DllMain; under loader lock a blocking shutdown is unsafe. Dedicated external shutdown protocol needed.
3. **VEH (blocking)** — three reachable `AddVectoredExceptionHandler` sites: `src/AICommandTrace.cpp:805`, `src/AITrace.cpp:2738`, `src/Stage59XWriterWatch.inl:292`. **Zero** reachable `RemoveVectoredExceptionHandler` calls. Stage59 explicitly documents keeping its VEH registered for process lifetime; disarming owned watchpoints is NOT callback registration retirement.
4. **Native ingress (blocking)** — **122** reachable occurrences of `__declspec(naked)`. Count of *source declarations*, NOT installed hooks. No proven live owner/native entry–return–SEH/unwind census; foreign raw caller / quiescence safety unresolved.
5. **FLS scope** — **0** `FlsAlloc` / `FlsFree` / `FlsSetValue` in current reachable game sources. Stage142/143 isolated FLS host experiments do not mean current game DLL actually has FLS registrations. If new work adds FLS, re-audit.
6. **Live owner proof** — NOT TESTED. No proof of restoring real Stage67/70 original bytes, running threads, native ABI, FLS callback retirement, loaded hook ownership, or origin(0x27). Preserve Stage36 OFF.

## Offline tests (actually run)

- Stage153 test suite **24/24 PASS** on Linux, including independent GCC/Clang C++17 builds with matching console output and corrupt ZIP/hash fail-closed checks. Extracted final ZIP independently re-verified: 12/12 payload manifest PASS and 24/24 tests PASS.
- Self-contained pure C++17 lifecycle-preflight model exhaustively evaluated **65,536** synthetic 16-bit masks: **65,535 denied**, **1 hypothetical all-preconditions-true**; all 16 single-missing cases denied. The lone `hypothetical_ready=1` is NOT permission to unload a game DLL; it contains no live operating-system observations or Win32 accesses.
- **MSVC Windows x86 Stage153 has NOT been run**. Windows runner available in ZIP: `Invoke-Stage153Audit.ps1`. It compiles only pure model code and optionally rechecks the pinned Stage146 ZIP.

## Windows x86 next command

In the **same** PowerShell process:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch x86 -HostArch x86
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
cd 'D:\INCUBATOR\ArcanaAI_Stage153_SourceOnly_NativeIngress_WorkerVEH_UnloadPreflight_r1\Stage153_SourceOnly'
.\Invoke-Stage153Audit.ps1
```

No Stage151/152 archives needed. Optional `-Stage146Zip 'D:\...\ArcanaAI_Stage146_Evidence_....zip'` if the user's pinned evidence ZIP is available locally; no need to upload private sources to GitHub.

## Safety / Stage154

`SOURCE_ONLY`; `SAFE_TO_INSTALL=false`; `SAFE_TO_UNLOAD_GAME_DLL=false`; `Stage36 OFF`; `origin(0x27)=UNKNOWN`; game files, user source tree, Stage129 tested working DLL **2,099,200 B** and Stage122 rollback **2,089,984 B** remain unchanged.

**NEXT Stage154:** design read-only/source-only admission and shutdown contract for the eight workers and three VEH registrations, requiring a proven external owner, stop/join outside loader lock, native callback and foreign-caller accounting, installed hook/rollback ownership, and fail-closed quarantine. Do NOT implement game DLL unload automatically.
