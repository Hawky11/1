# Stage115 — FA480 globals, guarded Stage113 piggyback SOURCE ONLY (2026-11-12)

**Status: offline native/source verified, NOT Windows built or runtime tested.** Original user PE32 AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` verified from the user's Library; no executable distributed.

## New source snapshot

Package delivered to user's ChatGPT session: `ArcanaAI_Stage115_FA480_Globals_SourceOnly.zip`, 36,799 bytes, SHA256 `e77ad4f0a54774439c3b687423d2d1b947dfdee2d8ec269a120acfdab8328efe`; 13 files including full corrected Stage113 module, exact SHA-guarded Apply/Rollback PowerShell, README RU, analysis, static verifier, log correlator, tests, expected-source fixtures and metadata. No original AALib.exe or DLL included.

Exactly **one project source file** changed: `src/Stage113ParentGateTrace.inl` (C2668-fixed parent SHA `a5c8edecfceadd60716cf3a51f70d06619eb6d2ec8f9269ff9c5a0d0096a755b` → Stage115 SHA `273059538a37cb1cfb82ab9c6eb891d0e535c59bcae1e41c42f06c27ff336d3a`). Stage113 adapter `d4026d45217188213aad701e278b00067815c4f7bae9ce9f5d88013e414de2d0` and Stage110 module `e183d8ba84ac07e9dcdcd8fd740604bb42e48bed1c52c36892354353bf8511b2` unchanged. Stage113's original 4 naked assembly hooks and RVAs unchanged. Stage115 adds **ZERO** hook sites.

## Native checks

Original FA480 first calls 1F4700; both are checked for original instruction signatures and five relocated operand references to globals:
`0x0E3C3EFC` (G1), `0x0E3C4930` (G2), `0x0E10B37C` (G3), `0x0E10B640` (G4), `0x0E10B8B8` (G5). The five operand immediate RVAs `1F4701,1F470C,FA48D,FA498,FA4A9` are all PE HIGHLOW relocations. Use `loadedImageBase + (nativePreferredVA - 0x00400000)`, never fixed preferred VAs if the game module is rebased.

Static FA480 result model:
```
pred1 = (G1 == 8 || G1 == 14) && G2 != 0
pred2 = (G3 == 1 && G4 == 4)
pred3 = (G3 == 2 && G5 == 4)
predictedAL = pred1 || pred2 || pred3
```
**These are predictions from post-return snapshots, not direct observations of helper branches; values may change between return and capture.** Captures happen for **sampled** original Stage113 gateA `9888D` events, not every call. Uses bounded existing ring=2048 and original all-hit Stage113 HEALTH counters, separate `ArcanaAI_Stage115Fa480GlobalsTrace.log` with BUILD/INSTALLED/SNAPSHOT/HEALTH. Partial reads report `predictedAL=2` unknown. Side attribution uses actor substructure base+0x154 without modifying old Stage113 P0 logs.

## Validation and safety

- Original PE + source checks **42/42 PASS**.
- Offline Python unit tests **27/27 PASS** (negative corrupted native call/operands, static source preservation, prediction truth cases, partial/duplicate and distinct-session log cases).
- Installer requires closed game and exact current C2668-fixed Stage113 module SHA, Stage113 adapter SHA, Stage110 SHA, prior `.stage113_backup` manifest; backs up current module to `.stage115_backup`. Rollback returns exactly that corrected Stage113 source.
- Stage115 opt-in `EnableStage115Fa480Trace=1` requires `EnableStage113ParentGateTrace=1` and **Stage36 OFF** (`EnableStage36ControlToCommandTrace=0`). No F2DE0 repatching, synthetic commands, DR writes or gameplay-data writes.
- No new Windows MSVC build, runtime hook installation, global snapshots, accepted move, hit or AI quality improvements have been proven yet.

## Next Stage116 — Windows

Backup Stage113 built **2,079,744-byte** DLL and Stage110 fallback **2,073,088 B**. Close AALib, execute the packaged `ApplyStage115.ps1 -Root "D:\INCUBATOR\ArcanaAIOverhaul"`, enable Stage115 in INI, compile Win32 with Build.bat, run brief natural COM-vs-COM. Upload **fresh** Build.log, general log, INI, Stage113 and Stage115 logs plus Stage110/107/104. Require Stage113 INSTALLED 4/4, Stage115 INSTALLED piggyback (addedHooks=0), positive Stage115 SNAPSHOT, error-free health, and same-session comparisons. If any install/build issue, use Stage115 rollback then restore known-good DLL. This stage is **source-only**, NOT a Windows-tested build.

[Previous Stage114 runtime audit](https://github.com/Hawky11/1/blob/stage114-fa480-zero-parent-gate-runtime-audit/Stage114_Runtime/FULL_SAME_RUN_AUDIT.md): 15,726/15,726 Stage113 parentA/FA480 gate skips, Stage110 entry=0; Stage115 investigates underlying predicate values, not game behavior.
