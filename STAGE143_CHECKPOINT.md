# ArcanaAIOverhaul — Stage143 isolated host admission registry and unload barrier (2026-11-13)

**SOURCE-ONLY / TEST-ONLY. SAFE_TO_INSTALL=false. Stage36 OFF. Working game DLL untouched.**

## Stage143 r2 — Windows PowerShell automatic variable collision FIX (2026-11-13)

- **Observed user failure of r1 before compilation:** after `STAGE143_MSVC_SELECTED C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat`, Windows PowerShell 5.1 raises `VariableNotWritable` at `Test-Stage143NativeHarness.ps1:46`, `$host=Join-Path $work 'Stage143Host.exe'`. PowerShell variables are case-insensitive, and automatic `$Host` is read-only. Neither `Stage143FlsProbe.cpp` nor `Stage143Host.cpp` was compiled in that attempt.
- **Fix:** replace all **four** standalone references to variable `$host` with `$hostExe` (assignment, linker/host command, file-existence check, scenario invocation). The **four Stage143 C++ source files are byte-identical** to r1; no DLL/game code modification or gameplay hooks. Static auditor now checks against using a read-only `$Host` name.
- **Corrected chat artifact**: `ArcanaAI_Stage143_IsolatedAdmission_Registry_UnloadBarrier_SourceOnly_r2.zip`, **31,171 B**, SHA-256 **`6e3c1c77525421ed7f156144c8910f924fc8ff98209b83c1272bbbb5bfe95cf4`**, 18 files, ZIP CRC and per-file manifest **PASS**, source-only (no compiled DLL/EXE). Script SHA-256 `2eabc1d2b5407cf4c81c02cf1084637fd657a5f0d2a4ec66fd3bd7ff6a92ec4c`.
- **Offline recheck of r2:** **66/66** static, **526/526 GCC** and **526/526 Clang** portable policy assertions. No Windows PowerShell interpreter or MSVC available in current environment; **r2 Windows native build/link + 16/16 scenarios NOT RUN**. Do not claim a native PASS yet.
- **Windows rerun:** extract r2; from `Stage143_IsolatedAdmission\tests` run `Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force` then `.\Test-Stage143NativeHarness.ps1`. Capture complete compiler/linker/scenario output. Original Stage143 r1 archive unchanged.
- **Safety:** `SAFE_TO_INSTALL=false`, Stage36 OFF; Stage129 game DLL/Stage122 rollback preserved, candidate origin of `0x27` remains UNKNOWN.

---


## Predecessor confirmed

- [Stage142 Windows native FLS DLL lifecycle](STAGE142_CHECKPOINT.md) completed **13/13 isolated DLL + host scenarios PASS** (user-submitted VS2026 Community MSVC x86 transcript), including thread/fiber callbacks, controlled local FlsFree/FreeLibrary, registry counters and eight-thread cleanup.
- The Stage142 DLL tracked calls that had entered it, but **DLL `inFlight==0` cannot prove there is no host thread holding a raw entrypoint pointer that has not entered yet**. A second barrier outside the module is required for the controlled case. The test contract **does not cover foreign callers that bypass the host**.

## Stage143 experiment

- Source-only ZIP `ArcanaAI_Stage143_IsolatedAdmission_Registry_UnloadBarrier_SourceOnly.zip` is **30,386 bytes**, SHA-256 **`1842562f03488399e1170a80767faa2a229d2b35c5496af9f84cd1546cf2d01a`**, **18 source/text/JSON files**, ZIP CRC PASS and all packaged-file SHA-256 values verified. Archive is attached in ChatGPT, **not uploaded into GitHub as a binary**. No EXE/DLL/OBJ/LIB/PDB inside.
- New experimental Win32 x86 code: `Stage143_IsolatedAdmission/src/Stage143FlsProbe.cpp` (disposable DLL), `Stage143Host.cpp` (test host) and `Stage143Api.hpp`, plus portable model `Stage143HostGatePolicy.hpp`.
- Reference snapshots: unchanged Stage142 original test DLL source hash **`babf666485936315dd7e8814fe6c6f9bd3527ef5a5186eb5e84cc623250f817d`**, Stage142 test host hash **`c5756d5018391b38543309b00ab0065b1f07114826fecb83c7401f494c7d50bb`**, Stage141 FLS predicate hash **`58aa1a1def7c93dfcdc2736b773da2e79795ec0b014dfe109653bc54fa468a60`**.
- Host acquires a **call-lease before** testing admission state. A caller accepted before `Close()` is counted even if not yet inside DLL. New arrivals after closure are denied by the host without executing DLL code. Host must close then observe **zero outstanding leases**, and joins its test-owned workers/fibers, before the DLL's BeginStop/Finalize and host's FreeLibrary.
- DLL maintains its **own** admission phase and `inFlight` count. Bounded CAS registry (512 entries) records each FLS value before FlsSetValue; manual clear and FLS callback unregister it. Failure to register is fail-closed. Finalization requires zero live values, registry entries, active callbacks and in-flight entries, no registry errors and all eight Stage141 host-provided assurances; only then **test DLL** calls real FlsFree. Host releases its module reference via FreeLibrary afterward.
- **Important distinction:** counters, byte-level source inspection and positive preflight flags **cannot prove** the absence of arbitrary foreign callers, unregistered fibers, owner detours, direct call pointers, SEH/nonlocal unwind or late callbacks in the actual game process. Module reference lifetime and external-entrypoint coverage remain unproven.

## Tests completed in this conversation — PORTABLE ONLY

- Static safety/source audit **64/64 PASS**.
- GCC C++17 portable policy **526/526 assertions PASS**.
- Clang C++17 portable policy **526/526 assertions PASS**.
- Portable policy checks all **256** host+DLL prerequisite combinations and all **256** Stage141 FLS preflight combinations, plus an 8-thread lease admission/closure race.
- SHA-256 manifest and ZIP CRC PASS.
- **Windows MSVC x86 native DLL+host compilation/linkage/16 scenarios: NOT RUN.** Do not claim a Windows pass.

## Windows native test plan

Open PowerShell 5.1 inside extracted `Stage143_IsolatedAdmission\tests`:

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
.\Test-Stage143NativeHarness.ps1
```

If required, explicit MSVC path:
```powershell
.\Test-Stage143NativeHarness.ps1 -Vcvarsall "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat"
```

Script checks exact source SHA-256, locates VS with `Microsoft.VisualStudio.Component.VC.Tools.x86.x64` (SSMS-safe), builds **Stage143FlsProbe.dll** and **Stage143Host.exe** in a random directory under **%TEMP%** only, executes 16 separate-process scenarios, and removes temporary artifacts unless `-KeepScratch` is specified. No game path/Build.bat, injectors, detours, gameplay writes or default project integration.

**Sixteen scenarios:** `empty`, `thread_callback`, `manual_cleanup`, `live_blocks`, `inflight_blocks`, `orphan_host_lease`, `denied_after_host_close`, `preflight_flags`, `fiber_delete`, `fiber_switch`, `parallel`, `race_gate`, `double_stop`, `double_finalize`, `registry_audit`, `reload_twice`.

Expected *only, not observed* final lines:
```text
STAGE143_NATIVE_BUILD PASS: isolated test DLL + host EXE only (not game DLL)
STAGE143_PASS scenario=...
STAGE143_NATIVE_HARNESS PASS: 16/16 isolated admission and unload scenarios
STAGE143_SAFE_TO_INSTALL false: ...
```

## Safety / next stage

- `SAFE_TO_INSTALL=false`; tested Windows Stage129 DLL **2,099,200 bytes**, Stage122 fallback **2,089,984 bytes** untouched; **Stage36 OFF**, no extra JMP at F34D0/F3930, no `Arm()`.
- Candidate source provenance of `0x27` remains **UNKNOWN**; this lifecycle study does not establish origin.
- Real module FLS shutdown requires verified *all-caller* admission, module lifetime protection, complete thread/fiber registry, owner quiescence even through nested helpers, x86 hook ABI and exceptional-unwind parity. This isolated prototype does not supply them.
- **Immediate next:** obtain full Stage143 Windows x86 compiler/linker output and all 16 scenario results. Stage144 can then audit the coverage gap between a cooperative host gate and actual Stage10/Stage129 module entrypoints, source-only, before considering further runtime work.
- Relevant Microsoft sources: https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-freelibrary , https://learn.microsoft.com/en-us/troubleshoot/developer/visualstudio/cpp/libraries/fatal-error-thread-exit-fls-callback , https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain .
