# ArcanaAIOverhaul Stage142 — isolated FLS test DLL / unload lifecycle (2026-11-13)

**SOURCE-ONLY / TEST-ONLY. SAFE_TO_INSTALL=false. Stage36 OFF. Working DLL unchanged.**

## Parent checkpoint / verified prerequisites

- [Stage141](STAGE141_CHECKPOINT.md) Windows PowerShell 5.1 user log: VS2026 Community **MSVC x86 isolated EXE BUILD PASS** and **16/16 native scenarios PASS**. Stage141 candidate fixed the Stage139 queue publication `sequence` ordering race under the ring lock. `fls_preflight` remained a *predicate only*, did **not** call FlsFree or test real DLL unload.
- This Stage142 deliverable addresses that separate Windows FLS API lifecycle question using a new disposable test DLL + test-only host, **not** the Stage139/141 bridge, original AIProducerContextTrace/Stage129 hooks, or game executable.

## Stage142 artifact and evidence

- Chat-generated archive `ArcanaAI_Stage142_IsolatedFLS_DllUnload_Lifecycle_SourceOnly.zip`, **23,093 B**, SHA-256 **`2fa2d5c4f7d778079925f130b19d57998d1ea97f8797d1f8d717d13c3553461b`**, **15 files**, ZIP CRC PASS and individual source SHA-256 manifest validated. Source-only: no EXE/DLL/OBJ/LIB/PDB inside. The ZIP is attached to the ChatGPT conversation and **is not stored as a GitHub binary**.
- Package root `Stage142_IsolatedFlsUnload/`. Source files: `src/Stage142FlsProbe.cpp` (disposable test DLL), `src/Stage142Host.cpp` (host test runner), `src/Stage142Api.hpp`; byte-identical Stage141 pure FLS preflight `reference_stage141/Stage141FlsUnloadPreflight.hpp` SHA256 `58aa1a1def7c93dfcdc2736b773da2e79795ec0b014dfe109653bc54fa468a60`.
- Current-session **offline checks**: static safety audit **44/44 PASS**, portable Stage141 FLS preflight / lifecycle policy **291/291 PASS on GCC** and **291/291 PASS on Clang** (256/256 combinations of 8 predicate bits covered). These are **not Windows API tests**.
- Windows MSVC x86 DLL/EXE build, linking, FLS callback, FlsFree, FreeLibrary, and 13/13 native scenario results are **NOT RUN** as of this checkpoint. Do not infer native PASS from Stage141 or the Linux tests.

## Test architecture

- PowerShell `tests/Test-Stage142NativeHarness.ps1` is Windows PowerShell 5.1 compatible: verifies key source SHA-256, finds VS using `-requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64` (avoids selecting SSMS), copies sources into a GUID-named **%TEMP%** folder, compiles isolated `Stage142FlsProbe.dll` + `Stage142Host.exe` with **MSVC x86 /MD**, then runs separate-process scenarios. No Build.bat, game DLL/EXE, hook installer, patch to F34D0/F3930/F30B0, gameplay writes or additional JMP.
- DLL initializes FLS through explicit exported `Stage142_Initialize()` (not DllMain). Callback performs only HeapFree and interlocked accounting. `BeginStop()` gates new admissions; `TryFinalize(0xff)` additionally checks phase draining, no live FLS values, no in-flight exported work, no callback in progress; it calls **real FlsFree** only if gates pass. Host calls **real FreeLibrary** only after controlled test shutdown and joining its own workers, while holding LoadLibrary reference during teardown.
- 8 assertion bits passed by the host are **claims, not independent proof of all other process threads/fibers or external callers**. A positive preflight state is eligibility for the isolated controlled test only. The host deliberately refuses unsafe early unload.
- 13 planned Windows scenarios: `empty`, `thread_exit`, `explicit_clear`, `live_slot_block`, `inflight_block`, `preflight_missing`, `double_stop`, `double_finalize`, `denied_after_stop`, `fiber_delete`, `fiber_switch`, `parallel_exit` (8 threads × 64 Touch/Clear plus thread exit callback), `reload_twice`.

## Windows handoff (NOT EXECUTED YET)

From extracted `Stage142_IsolatedFlsUnload\tests`:

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
.\Test-Stage142NativeHarness.ps1
```

Or specify `-Vcvarsall "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat"`.

Expected (not yet observed):

```text
STAGE142_MSVC_SELECTED ...
Stage142FlsProbe.cpp
Stage142Host.cpp
STAGE142_NATIVE_BUILD PASS: isolated test DLL + host EXE only (not game DLL)
STAGE142_PASS scenario=...
STAGE142_NATIVE_HARNESS PASS: 13/13 isolated DLL lifecycle scenarios
STAGE142_SAFE_TO_INSTALL false: ...
```

Collect full compiler/linker diagnostics and 13 scenario rows before declaring Windows pass.

## Microsoft FLS / unload semantics and remaining blockers

- FlsAlloc: https://learn.microsoft.com/en-us/windows/win32/api/fibersapi/nf-fibersapi-flsalloc
- FlsFree: https://learn.microsoft.com/en-us/windows/win32/api/fibersapi/nf-fibersapi-flsfree
- PFLS_CALLBACK_FUNCTION: https://learn.microsoft.com/en-us/windows/win32/api/winnt/nc-winnt-pfls_callback_function
- FreeLibrary: https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-freelibrary
- Microsoft documents the risk of registered FLS callbacks pointing to unloaded DLL code: https://learn.microsoft.com/en-us/troubleshoot/developer/visualstudio/cpp/libraries/fatal-error-thread-exit-fls-callback
- Real Stage141 integration still lacks a proven **complete live thread/fiber registry**, complete owner EIP/in-flight scope census, external caller admission barrier, pinning/code lifetime, loader lock proof, nonlocal unwind, native detour ABI and cross-module shutdown coordination. The simple test DLL is *not* proof these are solved. `origin(0x27)=UNKNOWN`.
- **SAFE_TO_INSTALL=false** even if 13/13 test EXE succeeds. Preserve Windows-tested Stage129 `dinput8.dll` **2,099,200 B** and Stage122 rollback **2,089,984 B**, Stage36 OFF.

## Next Stage143 (after Windows log)

Analyze and potentially prototype a **source-only admission / callback registry + unload barrier** grounded in real Stage141 code and test DLL lifecycle. Avoid automatic game deployment, new native patch sites, or prematurely calling FlsFree in the game DLL.
