# Stage104 r1 Windows build succeeds, F2DE0 already detoured; Stage104 r2 source-only compatibility patch (2026-11-12)

## Uploaded evidence (user's same-run, 2026-11-12)

`Build(1).log` began at 17:19:51; MSVC 14.51.36231 x86 compiled all eight translation units, linked **2,057,728-byte dinput8.dll**, `BUILD SUCCEEDED`. One preexisting nonfatal warning C4754 at `AICommandSiteTrace.cpp:786`. Build does **not** establish hook installation.

`ArcanaAI_Stage104Mode0NativeTrace.log` contains only three Stage104 records:

```
STAGE104_BUILD=2026-11-12-r1 SOURCE_ONLY F7CC0(mode0)+F7DA3(CEA20) tailjmp-native originalOnce=1 extraGameWrites=0 DRwrites=0
STAGE104_SIGFAIL region=CALLEE_F2DE0 rva=0F2DE0 expected=558BEC8B450880780F00752F actual=E94BC9C4789080780F00752F
STAGE104_ERROR native signatures modified/not exact; no patch
```

Native entry has exact **E9 rel32 + NOP first 6 bytes**, while the rest of original 12-byte F2DE0 prologue from byte 6 onward is unchanged. This is consistent with a pre-existing entry detour whose **owner/module cannot be inferred** from these logs. The Stage104 r1 installer correctly refused to patch both CALL sites. No Stage104 `INSTALLED` or `MODE0`/`CEA20` data.

## Stage104 r2 source-only solution

**Incremental one-file replacement** of `src/Stage104Mode0NativePath.inl` with SHA256 `4e1bed4fc46b92ad23bae47feeeb8a9910273044fb0de24d343c2c578c211a69` (r1 original SHA `1fd486068bdc9b023623fdd662eded12231e36e5cf7cf320ec7d6e0b24048905`). Stage81 adapter remains `3b0f6e22194b0c6385a7030ea6aa101b3f8ba2e80f32feec2b93b6cf1ccb5ef7`; Stage98 r2 observer remains `a56fe5692b3284fae37822e6f08438e3a32a800accbad5ff4c72d36ea97a1591`.

The new `F2DE0EntryCompatible()` accepts either the exact 12-byte original prologue, OR only a **5-byte near JMP (E9 rel32) + NOP** overwriting exactly original first 6 bytes while original suffix bytes 6–11 remain intact, **and** resolved JMP target passes `VirtualQuery` MEM_COMMIT / executable / no PAGE_GUARD or PAGE_NOACCESS / MEM_IMAGE or MEM_PRIVATE checks. All original CALL site and surrounding-context signatures, original `CEA20` callee signature, SHA fingerprint and Stage94/98 installation prerequisites remain unchanged and strictly verified before and during suspended-peer patch transaction. Any other mutation: fail closed, no patch.

Stage104 tail-JMP still enters original `gameBase+F2DE0` as original CALL did; it thus preserves, rather than overwrites or bypasses, the existing entry detour. It does **not** prove the existing hook is trustworthy or ABI-correct. New `STAGE104_COMPAT F2DE0 ... targetVA=... ownerUnidentified=1` explains the accepted preexisting detour to aid later diagnosis. The actual runtime target VA is unknown until next log.

## Files and verification

User-facing ZIP delivered in ChatGPT: `ArcanaAI_Stage104_r2_F2DE0_Existing_Detour_Compatibility_SourceOnly.zip`, SHA256 `8a0c83f49117eccabc815102000959f22a22ef3aa2b707579136e00f0d89b7a0`, containing replacement module, unchanged adapter reference, exact SHA guarded `ApplyStage104r2.ps1` / `RollbackStage104r2.ps1`, original verifier, mode0/CEA20 correlator, RU instructions, machine JSON, and 35 offline tests. No original EXE or raw user runtime logs bundled.

Original private AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` reverified: **39/39 original opcode/CFG checks**, Stage104 r2 **36/36 source contracts**, **35/35 offline tests PASS** incl logged detour and negative malformed/non-executable cases. These are **NOT** MSVC build/ABI/runtime evidence for r2.

### Next user validation

Close AALib.exe, retain known-good Stage98 r2 2,051,584-byte DLL and Stage97 2,045,440-byte fallback. Run packaged `ApplyStage104r2.ps1 -Root "D:\INCUBATOR\ArcanaAIOverhaul"` atop **unchanged Stage104 r1** (no reapplication of Stage104 r1 installer), then Build.bat x86. Run brief ordinary COM-vs-COM. Require `STAGE104_BUILD=2026-11-12-r2`, `STAGE104_COMPAT`, **`STAGE104_INSTALLED 2/2`**, clean Stage104 HEALTH and natural `MODE0`/`CEA20` where triggered. Upload same-run Build.log, Stage104, Stage94, Stage93, Stage98, ArcanaAI.log. Fail-closed or unstable: restore Stage98 DLL, run source rollback. **No Stage104 r2 DLL or game run now; no AI/hit improvement claimed, Stage36 OFF, no game/DR writes.** Original source/evidence branches immutable.
