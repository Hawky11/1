# Stage107 — Source-only original CEA20 POST + CEBE0 indexed-read observer

**2026-11-12: source prepared, original PE / offline checks pass. NO Windows compile, DLL install, new COM-vs-COM run or gameplay improvement established.** This is an incremental, opt-in passive diagnostic extension of Stage104 r2, not a behavior modification. The original private AALib.exe and user raw logs are **not** included in the chat ZIP.

## Provenance
- Original AALib.exe SHA256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, PE32/x86. Original native byte/CFG checks **23/23 PASS**.
- Stage104 r2 Windows-built DLL remains the last live-validated binary: **2,058,240 bytes**; [Stage105 same-run 245/245 gate joins and 17/17 0x27 CEA20 events](https://github.com/Hawky11/1/blob/stage105-mode0-27-runtime-confirmed/Stage105_Runtime/RUNTIME_ANALYSIS.md). Stage106 demonstrated static shared CEA20 writer→CEBE0 reader but NOT runtime consumption.
- Original Stage104 r2 observer source SHA `4e1bed4fc46b92ad23bae47feeeb8a9910273044fb0de24d343c2c578c211a69`; Stage81 adapter `3b0f6e22194b0c6385a7030ea6aa101b3f8ba2e80f32feec2b93b6cf1ccb5ef7`; Stage98 observer `a56fe5692b3284fae37822e6f08438e3a32a800accbad5ff4c72d36ea97a1591`.

## Original-read probe design

**RVA F7DA8 POST_CEA20:** original 6 bytes `80 7B 55 00 74 DF` correspond to `CMP BYTE PTR [EBX+55],0; JE F7D8D`. Capture after CEA20 returns, with native ECX=state and EAX=index for the observed nonnull argument path; guard read of three indexed fields (+28010/18/20). Original CMP executes once and original branch routes unchanged to either F7D8D or F7DAE. The earlier Stage104 F7DA3 CALL must resolve to **our exact Stage104CEA20Hook** before installation; source refuses unknown detours.

**RVA CEC64 READ_CEC64:** original 7 bytes `3B 94 87 20 80 02 00`, native `CMP EDX,[EDI+EAX*4+28020]`, followed by original `CEC6B JG CEC83`. The observer fires only when CEBE0 has actually reached this indexed read. Native CMP executes exactly once and a JMP (which leaves FLAGS intact) returns to CEC6B. EDI = state, EAX = index, ESI = input record, EDX = record code. It captures reader's own stack frame and return RVA (known direct callers: 988EF/9894D/CAD7E/CADDF), guarded field values and record status.

**Important correctness:** PRE (Stage104) → POST (Stage107) remains within **F7150** and shares frame/actor/TID. CEBE0 READ is in a **different stack frame**. READ→POST requires matching **state + index + TID + QPC + parentPost**, *not matching EBP/frame*. A dedicated negative test catches this. A CEC64 native read proves only that the third shared field was read; the two additional values are observer reads, **not** proof the original executed the corresponding CEC6F/CEC86 instructions. State values being equal cannot independently prove that a particular prior POST supplied them or that an action was accepted.

## Source changes / protection

- New `src/Stage107ConsumerReadTrace.inl` SHA256 `a3d2d74b7cb5c0b4858850fe0b55d8ab76c262b5b40bd19b7c8319b38472368f`.
- Incremental updated `src/Stage81SelectorWriter.inl` SHA256 `9581b85fd38000f6b1f79b4c214aeeea4ee4c05706d0274dc2b30de59258a333`.
- Parent Stage104 r2 and Stage98 sources unchanged. Two code-patch redirects guarded by exact signatures, original EXE fingerprint, Stage104 ownership, patch mutex, suspended-peer EIP exclusion, under-suspension recheck and rollback. FXSAVE/FXRSTOR and PUSHFD/PUSHAD preserve context; no synthetic commands, DR changes, gameplay data writes or Stage36.
- **Opt-in only** `[AIResearch] EnableStage107ConsumerTrace=1`; default **0**. No redundant F2DE0 detour.
- Bounded ring with worker flush; correlated CEC64 reads only for POST-matched state/index within 10 seconds. HEALTH monitors dropped, mismatch, readFail.
- SHA-guarded `ApplyStage107.ps1` / `RollbackStage107.ps1` update exactly two source files on an exact Stage104 r2 checkout **with the game closed**.

## Validation, archive and runtime handoff

- Original PE + source-contract checks: **46/46 PASS** (23 original, 23 source).
- Python offline unit/negative parser tests: **21/21 PASS**, including cross-function frame mismatch, ambiguous PRE, duplicate seq, wrong state/thread/index, replay guards and SHA-checked installer.
- Chat archive: `ArcanaAI_Stage107_CEA20_Post_CEBE0_Read_SourceOnly.zip`, SHA256 **`fec9d0597d72aa7947686dee7e44cdd9f7f1f3d37efc745555845dda70e032ee`**, contains new source, adapter, installer/rollback, read-only audit and correlator, RU README, machine results; **no game binary/DLL/raw logs**.
- **Next user Windows test (Stage108):** backup Windows-tested Stage104 r2 `dinput8.dll` 2,058,240 B and Stage98 r2 2,051,584 B/Stage97 2,045,440 B fallbacks; close AALib.exe; apply exact SHA-guarded installer; enable Stage107 in INI; run Win32 Build.bat; perform short *natural* COM-vs-COM; upload fresh same-run Build.log, general ArcanaAI.log, Stage104 and Stage107 traces, plus Stage93/94/95/96/97/98 where available. Require `STAGE107_INSTALLED 2/2`, clean HEALTH, and strict PRE→POST→READ join. If no read appears, it is inconclusive, not a proof of no action. Any error/crash/signature failure → rollback.

**Not proven:** Windows compilation, Stage107 runtime installation, same-event CEC64 consumer, attack animation, hit, improved AI or game-wide stability. Stage36 OFF.
