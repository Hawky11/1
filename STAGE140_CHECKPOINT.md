# ArcanaAIOverhaul — Stage140 isolated Win32 x86 ABI/SEH/FLS and sink-fault harness (2026-11-13)

**SOURCE-ONLY / TEST-ONLY. SAFE_TO_INSTALL=false.**

## Confirmed predecessor

- User confirmed **Stage139 MSVC x86 `/Zs` PASS**, 2026-11-13. Selected `C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat`. Windows output listed `Stage139Win32Bridge.cpp`, `AIProducerContextTrace.cpp`, `Proxy.cpp` and final `STAGE139_MSVC_SYNTAX PASS: /Zs only, no object/DLL emitted; original project untouched.`
- This proves only source syntax, not linkage, Win32 function behavior, complete ABI, FLS disposal, hook ownership or game runtime.
- Prior [Stage136–139 checkpoint](STAGE136_139_CHECKPOINT.md) retains exact history.

## Stage140 deliverable

- Chat-only ZIP: `ArcanaAI_Stage140_IsolatedWin32_ABI_SEH_FLS_SinkFaultHarness_SourceOnly.zip` — **30,026 bytes**, SHA256 **`30e55725ee79266c1d00bd4528554391c36c57478286ac155a79e32c897f2cf2`**, **15 files**, ZIP CRC PASS. Not a GitHub binary and not automatically uploaded to repository.
- Includes byte-identical Stage139 bridge source files, standalone Windows x86 test EXE source, safety checks, portable tests and PowerShell driver. Native reference Stage139 bridge SHA256 `5b3ece1fd68d9c23b67a24f65be224f3a3333155d192faf0c1b29c7725ed3e42` unchanged.
- Harness compilation strategy: single test translation unit `#include "../reference_stage139/Stage139Win32Bridge.cpp"` and *test-only preprocessor substitutions* for CreateFileW / WriteFile / FlushFileBuffers / AcquireSRWLockExclusive. No edits to Stage139 original source or game/project, no new JMP or DLL.
- Windows script `Stage140_IsolatedHarness/tests/Test-Stage140NativeHarness.ps1` verifies SHA256 and C++ toolchain through vswhere `-requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64` (avoids old SSMS vswhere bug), copies test files to a GUID-named **%TEMP%** directory and compiles **Stage140Harness.exe** using `vcvarsall.bat x86`. Does **not** touch `D:\INCUBATOR\ArcanaAIOverhaul`, call Build.bat, generate game DLL, or attach to game.
- Windows executable's **15 designed scenarios** (now all 15/15 PASS in user-reported Windows native harness): `happy`, `noarm`, `fallback`, `mismatch`, `wrongparent`, `nested`, `exception`, `fls_exit`, `parallel`, `double_flush`, `write_fail`, `partial`, `flush_fail`, `open_fail`, `order_repro`. Covers a synthetic __cdecl ESP/out-params check, artificial SEH RaiseException, pending FLS callback at thread exit, 4x100 producers + 4 parallel flushers, exact ACK accounting and deterministic Win32 IO partial/failure paths.
- **Portable tests run in current environment:** static audit **40/40 PASS**, sink policy model **20/20 PASS**, Stage139 policy regression **39/39 GCC** and **39/39 Clang**, ZIP CRC PASS. These are **not MSVC native executable results**.
- **MSVC x86 Stage140 compile/link/run status: PASS (user-reported Windows output)** for isolated test EXE, 15/15 scenarios. No game DLL; not a live hook/runtime gate. Positive harness `Arm(...,true,true,true)` is confined to *synthetic test EXE*, not game, and does not prove real owner readiness or quiescence.

## User-reported Windows MSVC x86 native execution — PASS (2026-11-13)

- **Windows native test COMPLETED, user-reported output 2026-11-13:** VS2026 Community x86 selected, `Stage140Win32Harness.cpp` built and linked into isolated EXE, `STAGE140_NATIVE_BUILD PASS`, `STAGE140_NATIVE_HARNESS PASS: 15/15 deterministic scenarios`. No game DLL generated.
- **Scenario highlights:** `parallel` 400 queued / 400 ACK / 400 finalized with `sinkTainted=0`; `fls_exit` 1 seen, 0 finalized / ACK, `sinkTainted=1` (expected invalidation); `write_fail`, `partial`, `flush_fail`, `open_fail` have ACK=0, queued=1, tainted=1; `order_repro` passes because it **reproduces** Stage139 sequence-number ordering defect — **not fixed**.
- Native harness results are from isolated synthetic test harness, **not** native hook installation, full game ABI parity, production teardown or crash durability. **SAFE_TO_INSTALL=false**.

Exact submitted console output:

```text
STAGE140_MSVC_SELECTED C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat
Stage140Win32Harness.cpp
STAGE140_NATIVE_BUILD PASS: isolated test EXE only (not game DLL)
STAGE140_PASS scenario=happy ack=1 queued=1 seen27=1 finalized=1 sinkTainted=0
STAGE140_PASS scenario=noarm ack=1 queued=1 seen27=1 finalized=1 sinkTainted=0
STAGE140_PASS scenario=fallback ack=1 queued=1 seen27=1 finalized=1 sinkTainted=0
STAGE140_PASS scenario=mismatch ack=1 queued=1 seen27=1 finalized=1 sinkTainted=0
STAGE140_PASS scenario=wrongparent ack=1 queued=1 seen27=1 finalized=1 sinkTainted=0
STAGE140_PASS scenario=nested ack=2 queued=2 seen27=2 finalized=2 sinkTainted=0
STAGE140_PASS scenario=exception ack=1 queued=1 seen27=1 finalized=1 sinkTainted=0
STAGE140_PASS scenario=fls_exit ack=0 queued=0 seen27=1 finalized=0 sinkTainted=1
STAGE140_PASS scenario=parallel ack=400 queued=400 seen27=400 finalized=400 sinkTainted=0
STAGE140_PASS scenario=double_flush ack=1 queued=1 seen27=1 finalized=1 sinkTainted=0
STAGE140_PASS scenario=write_fail ack=0 queued=1 seen27=1 finalized=1 sinkTainted=1
STAGE140_PASS scenario=partial ack=0 queued=1 seen27=1 finalized=1 sinkTainted=1
STAGE140_PASS scenario=flush_fail ack=0 queued=1 seen27=1 finalized=1 sinkTainted=1
STAGE140_PASS scenario=open_fail ack=0 queued=1 seen27=1 finalized=1 sinkTainted=1
STAGE140_PASS scenario=order_repro ack=2 queued=2 seen27=2 finalized=2 sinkTainted=0
STAGE140_NATIVE_HARNESS PASS: 15/15 deterministic scenarios (order_repro reproduces a KNOWN Stage139 sequence ordering bug)
STAGE140_SAFE_TO_INSTALL false: synthetic EXE tests do not prove game ABI, ownership, quiescence or live detour safety.
```

## New source-proven defect — sequence ordering

Inside unmodified Stage139 `Enqueue(Event e)`, `e.sequence=InterlockedIncrement64(&g_finalSeq)` executes **before** `AcquireSRWLockExclusive(&g_ringLock)`. Thus, thread A can reserve sequence 1, stall before the lock, then thread B reserve sequence 2 and queue before A. Journal order may be **2 then 1**, even with no missing event. This is not a proven log data loss, but `sequence` does **not** reliably encode FIFO order under concurrency.

Stage140's `order_repro` holds thread A at the test-only redirected SRW acquire, lets B enqueue, then releases A. **A PASS in this test would mean the defect was successfully reproduced, NOT fixed.** Future Stage141 source-only fix: assign sequence while holding the same ring lock at actual publication time, re-evaluate accounting and regression tests before any install.

## Unresolved live gates

- Real Stage10 **2/2** + Stage129 **3/3** owner readback and validated no ownership conflicts; global startup quiescence, including nested parent frames and new threads.
- Native hook-trampoline ABI parity (flags, register/SSE state, stolen instructions), complete SEH/C++/nonlocal unwind and `TerminateThread` cases.
- FLS callback address and thread lifecycle across DLL unload — no proven safe shutdown in Stage139.
- True sink durability/crash-safe exactly-once: `FlushFileBuffers` alone and RAM queue do not ensure it.
- Source-level candidate `VerifiedParent` ≠ origin of `0x27`; **origin=UNKNOWN**.
- Stage139 sequence-order race must be fixed and tested.

**SAFE_TO_INSTALL=false**, Stage36 OFF. Preserve Windows-tested Stage129 DLL **2,099,200 B**, Stage122 rollback DLL **2,089,984 B**. No DR, gameplay writes, F2DE0 repatching or second detour on F34D0/F3930.

## Next

1. Windows (completed, user-reported PASS): PowerShell 5.1 from extracted `Stage140_IsolatedHarness\tests`:
   ```powershell
   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force
   .\Test-Stage140NativeHarness.ps1
   ```
2. Windows native EXE build/link and all 15 synthetic scenario results now captured above; do not interpret as live game ABI/hook safety.
3. Stage141: address confirmed source ordering defect and FLS teardown gate, then revisit ABI/ownership gates. No auto-install.
