# Stage134 — archived hook-owner map, startup quiescence and exceptional unwind safety

**Date:** 2026-11-13 (Europe/Riga). **Status: ARCHIVAL STATIC PASS, NEW HOOK INSTALLATION BLOCKED.** No new DLL, gameplay changes, native hooks, registers, or memory patches were generated or run.

The complete, locally verified conversation attachment is `ArcanaAI_Stage134_OwnerMap_Quiescence_Unwind_StaticAudit.zip`, **17,878 bytes**, SHA256 **96f4988a99e67d5bc596dc700d5d385e5daf4fae8902081863c963b24af77211**, 11 text/JSON/Python/PowerShell files, ZIP CRC pass, **no .exe/.dll/.pyc**. The archive is *not* uploaded in this GitHub branch. It contains the full 52-record SHA/source/original-opcode owner inventory, 10 prospective site byte records, stand-alone verifier, 19 tests and a read-only Windows source collector.

## Verified original game bytes and archival source

Original user `AALib.exe`: 175,556,608 B, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, PE32 x86, base VA 0x400000. Exact original file was available and independently rehashed and mapped into sections. Across **21 archived patch-owner modules** (Stage81 + 20 modular Stage82–129 source stages) **52 patch-site spans** were extracted; each RVA is explicitly present as a numeric literal in the respective archived module source. The Stage129 adapter includes **22 named .inl modules**, all module names were recovered from individual older ZIPs. **Stage88/Stage90 add no new native hook; Stage115 piggybacks Stage113**.

| Check | Result |
|---|---:|
| Archived patch site intervals | **52** |
| Original bytes verified | **52 owner + 10 prospective = 62** |
| Archived-owner vs archived-owner | **1,326** pair checks; **0** overlaps |
| Prospective Stage133 sites vs archived-owner sites | **520** pair checks; **0** overlaps |
| Prospective vs prospective | **45** pair checks; **0** overlaps |
| **Total** checked comparisons | **1,891**, **0** overlaps |
| Archived module name coverage vs Stage129 includes | **22/22**, NOT same-version/current hash proof |
| Portable independent Python tests | **19/19 PASS** |

### Critical non-exhaustiveness

The source texts above are from *different historical ZIPs*. Matching module names is not proof that the user's **currently built** `D:\INCUBATOR\ArcanaAIOverhaul\src` files have those bytes. They are not a complete live process patch-owner inventory. The Stage129 build has **eight C++ translation units** (Proxy, ComVsCom, AITrace, AICommandTrace, AIQueueTrace, AICommandSiteTrace, AIProducerContextTrace, AIParentFlowTrace). Some may install additional native hooks. The Stage81 worker explicitly waits for a separate **Stage80 owner detour at E8A8E**; that detour is not counted among the 52. Earlier Stage24/35 and external/third-party in-memory detours are also outside the bounded archival scan. There is **no proof that any particular byte or owner is current/live**.

### Startup quiescence is not established

Current Stage81 adapter source starts a worker thread. That worker waits until the game's `AALib.exe` is loaded and **Stage80 is installed**, then starts a nested chain of Stage81/82/84/.../129 Install() functions. This is *not* a guaranteed pre-gameplay barrier.

In Stage129's actual source, `SuspendAndCheck()` iterates a Toolhelp thread snapshot, suspends existing threads, reads `CONTEXT_CONTROL`, and rejects EIP in `F30B0..F3274` or the three patch windows. It does not inspect a complete native stack or prove absence of an active `F34D0/F3930` parent frame whose thread is now inside a callee such as F38D0 or CBF50. `CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD)` is not a global prevention of newly created threads. Therefore **parent quiescence at install time is UNPROVEN**, even if all scanned EIPs pass.

### Exceptional unwind and ABI are blocked

Stage133 statically mapped 5 normal parent returns: F34D0→F3662/F366B, F3930→F3E4A/F3E5D/F4017. Hooking all five normal epilogues does **not** cover SEH, abnormal nonlocal unwinding, thread exit or nested calls abandoned without matching return. Per-thread EBP values may be reused. No genuine native parent-scope cleanup/invalidation mechanism or complete all-`0x27` unsampled Stage129 bridge exists in the currently inspected source. Missing parent entry/exit, writer attribution, loss accounting, or exceptional exit → **UNKNOWN**, never **inherited**.

Prospective F3C11 table commit must replay `CMP [EBP+14],0; MOV [EBP+10],EAX` exactly once, propagate all native CMP flags to the later `JE` and leave native faultable `MOV EAX,[EBX]` at F3C18. No trampoline implementation/Windows ABI test exists. Prior Stage129 hooking success does not certify a future ten-site implementation.

## Stage134 decision and next input

**ARCHIVED STATIC OWNER MAP VERIFIED; SAFE TO INSTALL NEW HOOKS: FALSE.** Maintain Windows-tested Stage129 `dinput8.dll` **2,099,200 bytes** and Stage122 rollback DLL **2,089,984 bytes**, Stage36 OFF. Do not invent a new DLL, repatch F2DE0, override FA480, touch debug registers or synthesize gameplay.

The attached archive contains `tools/CollectStage134Sources.ps1` for **user-run Windows source capture**. Default source is `D:\INCUBATOR\ArcanaAIOverhaul`; destination is `%TEMP%\ArcanaAI_Stage134_CurrentSources.zip`. It copies only source/build file types and SHA256 manifest, **excludes .ini, .log, .exe, .dll, save data** and modifies no project/game files. **The PowerShell collector was not run or validated on Windows in this environment**. User should inspect the resulting ZIP before uploading.

**Stage135:** analyze that exact current integrated source snapshot, enumerate all hook writes/owners (including the eight .cpp TUs and Stage80), then assess a proven quiescent installation window and nonlocal unwinding policy. Any unverified precondition remains fail-closed. Current Stage134 package is only an audit, not an injectable patch.
