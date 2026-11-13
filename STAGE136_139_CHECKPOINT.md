# ArcanaAIOverhaul — Stage136–139 checkpoint (2026-11-13)

This checkpoint records source-only bridge work and the user-reported Windows syntax verification, without treating model checks or syntax-only checks as runtime evidence. Files in this repository remain unchanged by the source-only artifacts unless a future explicit integration is authorized.

## Stage136 — owner-integrated, unsampled 0x27 model

- Confirmed Stage10 already owns native parent entry detours **F34D0/5B** and **F3930/6B** in `AIProducerContextTrace.cpp`. Installing another JMP at those sites would collide. Prefer reuse of the existing `CaptureAndCall()` owner boundary.
- Modeled parent invocation identity by thread, depth, unique invocation/token and lifecycle; never join parent and Stage129 by equal `EBP` or QPC proximity.
- A separate, unsampled every-interesting-`0x27` Stage129 observation path is necessary: detailed Stage129 exit logging has sampling, and its sampled logs cannot establish completeness.
- Portable model: **20/20 GCC and 20/20 Clang**; model exercise includes 3000 candidate27 events. This is **synthetic evidence only**; no native install, no DLL.
- Chat artifact: `ArcanaAI_Stage136_OwnerIntegrated_Unsampled27_OfflineModel.zip`, **16,980 B**, SHA-256 `9c66602c0da73943a43c31b5e61b6fc9cf8304d13ade24f8c8cdf0a06ae5ccf7`, **11 files**, ZIP CRC PASS.

## Stage137 r2 — Win32 x86 compile-only FLS bridge

- Produced guarded candidate integration at existing Stage10 and Stage129 callbacks, with a unique invocation token, FLS per-thread context, `__try/__finally` cleanup and independent entry-27 observation. All candidate code is guarded by `ARCANA_STAGE137_COMPILE_ONLY`, the bridge is not part of the default `.vcxproj`, and there are no new JMP/Arm call sites.
- Offline evidence: **22/22** GCC and **22/22** Clang, **32/32** static checks; Stage136 regression **20/20** each.
- First Windows syntax-check script mistakenly let `vswhere -latest -products '*'` select **SQL Server Management Studio 22** instead of VS with C++ tools. r2 filters for `Microsoft.VisualStudio.Component.VC.Tools.x86.x64` and supports explicit `-Vcvarsall`.
- **User-reported Windows MSVC x86 syntax PASS for Stage137 r2** (2026-11-13): selected `C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat`, parsed `Stage137Win32Bridge.cpp`, `AIProducerContextTrace.cpp` and `Proxy.cpp`, printed `STAGE137_MSVC_SYNTAX PASS: /Zs only, no object/DLL emitted; original project untouched.` This verifies syntax only: **not** linkage, ABI, native unwind or game runtime.
- Chat artifact: `ArcanaAI_Stage137_Win32_X86_SourceOnly_CompileGate_r2.zip`, **58,399 B**, SHA-256 `8780387adc9fee18a2bbc2ecbb82de317dc8823523fe83938934d04af2239a2e`, **25 files**, ZIP CRC PASS.

## Stage138 — ABI, SEH, completion and writer safety audit

- Stage137 made a parent-link decision at child entry rather than after the Stage129 input/exit pair was verified, creating a false-confirmation risk.
- Its queue also popped an event before successful `WriteFile`; concurrent writers and failed writes could lose/duplicate evidence.
- Required correction: `PENDING -> VERIFIED_PARENT/UNKNOWN` only on validated child exit, with completed native entry/exit pair; explicit loss reporting; serialization; never label an ambiguous case as inherited.
- Offline static audit **31/31**, model checks **31/31**; runtime ABI, SEH/nonlocal unwind, linkage, native writer failures remain unverified.
- Chat artifact: `ArcanaAI_Stage138_Win32_ABI_SEH_CompletionGate_SourceAudit.zip`, **15,617 B**, SHA-256 `61765ed54d606f32aad531daba9ce99d5799faf8199774b383c87b61a7cb5810`, **9 files**, ZIP CRC PASS.

## Stage139 — pending/exit verification + loss-safe sink candidate

- Candidate integration uses `OnStage129Entry` before sampling to create a **pending** ticket; `OnStage129Exit` before sampling verifies native Stage129 pairing, TID, scope, native child frame *within Stage129 only*, result, LIFO and live parent token. Only validated `kReturnInput` can become `VerifiedParent`; forced fallback or any uncertainty -> `UNKNOWN`.
- Do **not** conflate `VerifiedParent` (which call owned the child) with candidate provenance: origin of `0x27` remains **`UNKNOWN`** (literal vs table vs inherited unproven).
- Sink policy: single flush lock, queue **peek -> WriteFile -> FlushFileBuffers -> ACK/pop**. An ambiguous or partial write freezes the stream rather than blindly retrying, records incomplete evidence and does **not** promise durable exactly-once delivery or crash safety. Unobservable candidate inputs mean `candidate27Seen` is only a **lower bound**.
- Offline checks: **39/39 GCC and 39/39 Clang** policy tests, **20/20** sink-model tests, **51/51** static checks. Regression: Stage136 **20/20** GCC/Clang, Stage137 **22/22** GCC/Clang, Stage138 **31/31**. No new JMP, no `Arm()` call, no gameplay writes, default build not changed.
- **User-confirmed Windows MSVC x86 syntax PASS (2026-11-13):** `Test-Stage139CompileOnly.ps1` selected the installed VS2026 Community C++ toolchain, processed all three translation units and printed the final success marker:
  ```text
  STAGE139_MSVC_SELECTED C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat
  Stage139Win32Bridge.cpp
  AIProducerContextTrace.cpp
  Proxy.cpp
  STAGE139_MSVC_SYNTAX PASS: /Zs only, no object/DLL emitted; original project untouched.
  ```
  This closes the **syntax-only** Stage139 MSVC x86 gate. It is not MSVC full compilation/linkage, SEH/FLS native fault injection, ABI parity, hook installation or gameplay proof.
- Chat artifact: `ArcanaAI_Stage139_PendingExit_Verification_LossSafeSink_SourceOnly.zip`, **61,188 B**, SHA-256 `b3a514b6349aa5603c2623583f15b82eabe9f3007380f857f518984ea863f934`, **23 files**, ZIP CRC PASS. Artifact is attached in ChatGPT **and is not automatically committed to GitHub**.

## Safety / next handoff

- **SAFE_TO_INSTALL=false** for Stage136–139. The Stage139 bridge has not been linked or installed in the game. No evidence here of live `2/2` Stage10 and `3/3` Stage129 owner readback, complete startup quiescence (including parent frames inside callees), native x86 ABI/SEH/FLS teardown safety, or controlled Win32 concurrent/partial-write fault-injection.
- Preserve user's existing Windows-tested Stage129 `dinput8.dll` **2,099,200 bytes**, Stage122 rollback DLL **2,089,984 bytes**, **Stage36 OFF**. No DR writes, F2DE0 repatch, new hooks or gameplay writes in these source-only stages.
- **Immediate next action:** Stage139 Windows `/Zs` syntax PASS is confirmed; proceed to Stage140 isolated ABI/SEH/FLS and Win32 sink-failure harness (no automatic installation).
- The ZIP files above are available in the user's ChatGPT conversation, not linked as GitHub binary artifacts. Hashes/size/inventory were rechecked against current session files.
