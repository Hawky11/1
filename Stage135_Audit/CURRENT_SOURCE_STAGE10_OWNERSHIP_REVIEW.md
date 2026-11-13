# Stage135 — actual current user source ownership and Stage10 hard overlap (2026-11-13)

**STATUS: USER'S CURRENT SOURCE SNAPSHOT VERIFIED; NEW NATIVE HOOKS STILL BLOCKED.** Do NOT deploy the earlier Stage133 10-site observer. This is an audited source review, not a new DLL, game-memory patch, runtime test, or gameplay acceptance proof.

## Source freeze

- User uploaded `ArcanaAI_Stage134_CurrentSources.zip`, SHA256 `e1ae1b0bb74b508a2d47ce600c37f67f730b64306c52fc3d4e6d16cf2077a84f`. ZIP CRC passed, contains 70 project files + `Stage134_SourceCapture_manifest.json` dated 2026-11-12T23:48:42.7293205Z. **70/70** source file size/SHA256 matched manifest, 0 missing, 0 extra.
- User original `AALib.exe` rehashed independently in user Library archive: **175,556,608 B**, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
- All **21/21** module SHA256 values from Stage134's 52-range historical `source_owner_ranges.json` matched the corresponding actual captured current source modules byte-for-byte. Hence the **52 archived Stage81–129 source patch ranges are confirmed to be from the current captured source tree**, but no claim that all installed in game RAM.
- Complete Stage135 ChatGPT artifact (NOT copied to GitHub): `ArcanaAI_Stage135_CurrentSource_Owner_Collision_Audit.zip`, **10,134 B**, SHA256 `4ce2541d6b1df71ce6ac3d662667608025e62d8d96273458c765704152e66910`, ZIP CRC PASS. 5 files: RU source analysis; JSON outcomes; 88-row source-declared register; portable verifier; 12 synthetic tests (**12/12 PASS**). No original game binary, no user's full source contents, no executable.
  
## CRITICAL: two Stage133 future hooks ALREADY owned by current Stage10 C++ module

`src/AIProducerContextTrace.cpp` is one of eight compiled C++ translation units. It declares exact native `g_hooks[]` and installs them via `InstallEntryHook`:

| Planned Stage133 parent entry | Current Stage10 owner `AIProducerContextTrace.cpp` | Overlap |
|---|---|---|
| F34D0, new proposed 6 B JMP | Existing Stage10 `ArcanaStage10HookF34D0`, 5 B prologue hook `55 8B EC 51 53` | **5 bytes** |
| F3930, new proposed 6 B JMP | Existing Stage10 `ArcanaStage10HookF3930`, 6 B prologue hook `55 8B EC 83 EC 18` | **6 bytes** |

Source positions: constant RVA lines 35–36, prologue bytes 130–131, `g_hooks` 161–165, `InstallEntryHook` 528–624, detour assignment 763–765 and install loop 788–795. `Proxy.cpp` calls `StartAIProducerContextTrace` before starting Stage81 worker; actual startup is asynchronous, so calling order does not prove installation order. Worker enables this observer with `GetPrivateProfileIntW` default `EnableAIProducerContextTrace=1` at line 722. **Runtime active ownership is conditional on current INI, successful install and absence of conflicting third-party hooks** (none of which is in source capture). No safe second JMP to those original entry addresses.

## BETTER owner-integrated design: use existing Stage10 wrapper

`CaptureAndCall()` (~lines 426–458) already executes *before* and *after* calling the original `F34D0/F3930` trampoline exactly once on its normal path, via its existing owner hook. Thus Stage136 should integrate unsampled unique parent-scope *inside this existing code owner* rather than add two duplicate native entry detours or five native RET detours. Required design: unique per-TID/depth/invocation ID; pre/post scope witness for every call (not the currently sampled failed-result statistic); publish provenance to Stage129 all-candidate27 bridge; exceptional unwind and missing scope → UNKNOWN, never inherited. This owner-integrated plan has NOT been coded or Windows-tested.

## Other collision and source coverage boundaries

- Source-declared bounded registry includes **88** site ranges: 52 current-SHA-matched archived Stage81–129 ranges, two Stage10 parent owner ranges, plus 34 manually inspected early C++/inl source native patch targets. Registry is **not a complete live-process owner census**, and not proof all 88 are simultaneously active; Stage38–79 and third-party patches still need exhaustive checking.
- 10 proposed Stage133 sites vs 52 archived/current module sites → **0 overlap**; vs currently confirmed two Stage10 owner sites → **two overlaps above**; vs additional 34 inspected C++/earlier inl sites → **0 overlap**.
- Source-declared owner-to-owner list includes **one conditional overlap at F79E4 (5 B)**: Stage82 existing hook vs Stage36 candidate observer. **Stage36 remains OFF**, as required by Stage129. Never enable Stage36 with Stage82 without owner-chain redesign.
  
## Critical native installation blocker in existing Stage10

`AIProducerContextTrace.cpp:~461–498` `SuspendOtherProcessThreads()` returns only number of peers suspended: snapshot failure returns 0 (indistinguishable from no peers), failed OpenThread/SuspendThread silently skips thread. `InstallEntryHook()` may still patch with `memcpy(target,patch,...)` despite unsuspended threads; no full CONTEXT.Eip or nested parent-stack safety proof. Existing Stage10's current worker is not a reusable fail-closed transaction design. Stage81 worker waits for Stage80 at E8A8E, *not* a quiescent barrier; a game thread may be in an original parent or nested helper before install. Thread snapshot does not prevent new threads.

Stage129 `Sample(kind)` at end of `Observe` emits only first 64 plus each 128th; Stage130 measured 87 sampled pairs across 3000 calls. No current unsampled complete `candidate==0x27` bridge.

## Next Stage136 — SOURCE DESIGN FIRST, NOT AUTOMATIC HOTPATCH

1. Audit actual currently enabled config/Stage10 success markers only if user supplies fresh logs or INI, without assuming a source-default means installed.
2. Extend Stage10 **in the same owner** with exception-safe unique parent scope pre/post around original trampoline; retain existing default behavior and pure observation.
3. Design three narrowly scoped original interior attribution probes at F3579, F35F9 and safer F3C11 (not F3C15), *only after complete owner/EIP/in-flight stack+new thread races/rollback audit*. F3C11 must replay CMP [EBP+14],0 and MOV [EBP+10],EAX once, preserve CMP EFLAGS for F3C27 JE, leave faultable MOV EAX,[EBX] original at F3C18.
4. Extend Stage129 with loss-accounted unsampled all-interesting-input27 parent/child correlation; UNKNOWN for dropped/orphan/unwound/late-installed states, never assert inherited by absence alone.
5. Separate Windows MSVC x86 build and fresh opt-in runtime validation **only after** all source/ABI gates pass.

**SAFE TO INSTALL NEW HOOKS = FALSE.** Preserve Windows-tested Stage129 DLL **2,099,200 B**, Stage122 fallback **2,089,984 B**, Stage36 OFF. No new game writes, DR writes, F2DE0 repatch, FA480 overrides or synthetic gameplay.
