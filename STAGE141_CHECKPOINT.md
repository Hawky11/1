# ArcanaAIOverhaul — Stage141 sequence FIFO correction + FLS unload preflight (2026-11-13)

**SOURCE-ONLY; SAFE_TO_INSTALL=false; no game DLL/EXE or detour changes.**

## Starting point and bug reproduced

- User confirmed [Stage140](STAGE140_CHECKPOINT.md): isolated MSVC x86 test EXE **build/link PASS and 15/15 scenarios PASS**, including deterministic `order_repro`. That scenario's PASS **confirmed a defect** in the unmodified Stage139 `Enqueue`: `e.sequence=InterlockedIncrement64(&g_finalSeq)` happened **before** `AcquireSRWLockExclusive(&g_ringLock)`, allowing journal sequences `2,1` when another thread overtakes publication.
- Stage139 user-reported `/Zs` syntax PASS also retained. Neither result authorized live installation.
- Original Stage139 source SHA-256 **`5b3ece1fd68d9c23b67a24f65be224f3a3333155d192faf0c1b29c7725ed3e42`** is preserved byte-for-byte in this Stage141 package. Original `Stage139Contract.hpp` and `Stage139Win32Bridge.h` also unchanged.

## Source-only repair

- New `adapter/Stage141Win32Bridge.cpp` SHA-256 **`99f44e7be94b8f070c8b64204218e7b62e3367aa52b7314177027380e0810633`**.
- Patch `patches/Stage139_to_Stage141_sequence.patch` modifies **only** the header comment and `Enqueue(Event)`. It allocates `g_finalSeq` **inside** `g_ringLock` **after** the full/tainted queue rejection gate, immediately before `g_ring[g_tail]=e`. Rejected publications consume no sequence; signed counter rollover taints/block publication instead of emitting ambiguous IDs. Sequence describes successful in-memory queue publication; not game timestamp, source attribution, crash durability or ACK.
- No change to parent hooks F34D0/F3930; no added JMP, `Arm()` call, gameplay modification, game project change or game DLL build. Candidate references existing Stage139 names because it is a standalone isolated source alternative.

## Isolated Windows x86 test harness: NOT YET EXECUTED

- `Stage141_SourceOnly/tests/Test-Stage141NativeHarness.ps1` checks baseline/candidate SHA-256, selects VS with `Microsoft.VisualStudio.Component.VC.Tools.x86.x64`, compiles a test EXE **only in a new %TEMP% directory** (no `Build.bat`, game DLL, or injection) and runs **16 deterministic scenarios**.
- Carried forward 14 Stage140 scenarios; replaces old `order_repro` with **`order_fixed`** asserting two forced-overlap publications have strictly increasing sequence 1 then 2; strengthens `parallel` to assert a strictly monotonic sequence over all 400 events; adds **`fls_preflight`**. Retains no-arm, fallback, mismatch, wrongparent, nested, artificial SEH, pending FLS thread exit, double flush and forced IO failures.
- **Stage141 MSVC x86 build/link and 16/16 test result are NOT RUN / not yet received.** Do not infer PASS from Stage140 or GCC/Clang.

## FLS shutdown preflight (not an unload implementation)

- `adapter/Stage141FlsUnloadPreflight.hpp`: pure C++ policy requires **all eight** external assurances: parent owners quiescent; all module entrypoints blocked; complete live thread/fiber registry; no in-flight callbacks/scopes; zero live FLS slots; sink fully stopped/accounted; outside loader lock; module code pinned during shutdown.
- Missing any item -> **Blocked**. All present -> **EligibleForSeparateControlledTest**, NOT `SAFE_TO_UNLOAD`. The Stage141 candidate still has Stage139 FLS callback and does **NOT** call `FlsFree`. Neither a synthetic boolean gate nor 16/16 native EXE can prove real DLL unload safety.
- Relevant Microsoft FLS documentation: https://learn.microsoft.com/en-us/windows/win32/api/fibersapi/nf-fibersapi-flsfree and https://learn.microsoft.com/en-us/troubleshoot/developer/visualstudio/cpp/libraries/fatal-error-thread-exit-fls-callback . FlsFree can invoke registered callbacks for occupied slots. A callback address retained after DLL unload can be invalid at thread exit.

## Evidence from this session

- Stage141 static source audit **52/52 PASS**.
- Stage141 portable C++ policy **19/19 GCC** and **19/19 Clang**. 256 possible 8-bit FLS assertion combinations validated in the portable policy model; exactly all-eight-present earns *eligibility to test*, nothing more.
- Chat-delivered ZIP: `ArcanaAI_Stage141_SequenceFIFO_FLS_UnloadPreflight_SourceOnly.zip`, **36,059 bytes**, SHA-256 **`8097b0e05a3ee308824b33f79cd129cea56c63283fac346f098a6d55f9c860f5`**, **19 files**, ZIP CRC PASS, per-file SHA-256 manifest verified, **no binaries**. Package is in ChatGPT, not committed as GitHub binary.
- Full source and test instructions are in `Stage141_SourceOnly/README_STAGE141_RU.md` inside the ZIP.

## Windows handoff

From extracted `Stage141_SourceOnly\tests` in Windows PowerShell 5.1:

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
.\Test-Stage141NativeHarness.ps1
```

Expect source-hash checks, `STAGE141_MSVC_SELECTED`, `STAGE141_NATIVE_BUILD PASS`, per-scenario `STAGE141_PASS`, final `STAGE141_NATIVE_HARNESS PASS: 16/16`. **Expected output is not observed output.**

## Open deployment gates / next

- No actual game owner readback/complete live hook census, startup parent quiescence, native detour ABI/trampoline review or nonlocal-unwind parity.
- Stage139 bridge lacks a proven FLS shutdown barrier or complete thread/fiber registry; FLS unload safety remains **unproven**.
- Candidate origin of `0x27` remains **UNKNOWN**, even with a verified parent link.
- Sink ACK after Win32 flush does not guarantee durable exactly-once delivery through crashes.
- **SAFE_TO_INSTALL=false; Stage36 OFF**. Keep the Windows-tested Stage129 DLL **2,099,200 bytes** and Stage122 fallback DLL **2,089,984 bytes** untouched.
- Next: collect complete user Stage141 Windows native harness 16/16 log; fix errors source-only if necessary. Stage142 should analyze live-context registry, quiescent shutdown and controlled FLS unload in a separate test DLL/exe, **never install automatically into game**.
