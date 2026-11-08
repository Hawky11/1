# Stage20 — trace the downstream bit-2 object guard

## Goal

Stage19 showed that Stage18's table gate is not sufficient to explain setter reachability: 31 calls for P2 state index 0x19 returned Stage18 AL=1 but were not followed by a Stage17 bit-113 setter call. The setter episodes that did proceed had an associated object DWORD at +0x3B70 with bit 0x4 clear.

Static disassembly now identifies a second conditional check at AALib.exe+0xC64F1. It calls helper 0x495B00 with ECX=[EBX] and argument 2; the helper tests bit 2 in [object+0x3B70]. A nonzero AL takes the following JNE and skips the setter.

## What Stage20 adds

- A passive call-wrapper around the original CALL at RVA 0x000C64F1.
- Logs the original helper's actual AL/EAX, ECX object pointer, EBX slot pointer, pointer value [EBX], [object+0x3B70], bit-2 state, player, EBP parent return, QPC and time.
- The original helper is called with the original ECX and duplicated argument 2. Its own RET 4 consumes the duplicate; the wrapper then restores the captured return context and RET 4 consumes the original argument.
- Original TEST AL,AL at 0xC64F6 and JNE at 0xC64F8 remain untouched.
- No DR0–DR3 breakpoints are added.
- New log: ArcanaAI_Bit113ObjectGuardTrace.log.
- INI switch: EnableStage20Bit113ObjectGuardTrace=1.

See STATIC_OBJECT_GUARD_ANALYSIS.md for the exact helper disassembly, call-wrapper stack contract and correlation targets.

## Build and install

Start from the Stage11 assembled project that already has Stage12–Stage19 overlays applied. Apply Stage20_Overlay/src/AITrace.cpp and Stage20_Overlay/ArcanaAI.ini over the Stage19 source. Build Release | Win32 (x86) and keep the full Build.log. Stage20 is source-only in this branch: no MSVC build or runtime test has been performed. Do not install unless compilation and linking succeed. Back up the working dinput8.dll before installation.

## Startup validation

Before gameplay, ArcanaAI_Bit113ObjectGuardTrace.log must show:

- STAGE20_BIT113_OBJECT_BIT2_GUARD_BUILD=2026-11-08-r1
- STAGE20_INIT and STAGE20_CONFIG with EnableStage20Bit113ObjectGuardTrace=1
- successful wrapper installation at RVA 0x000C64F1 with original helper RVA 0x00095B00
- Stage20 trace active

The installer validates:
- setup bytes 8B 0B 6A 02 (MOV ECX,[EBX]; PUSH 2);
- CALL bytes E8 0A F6 FC FF and decoded target 0x00495B00;
- following TEST bytes 84 C0 at RVA 0x000C64F6;
- following JNE bytes 0F 85 0D FF FF FF at RVA 0x000C64F8.

Only the CALL is replaced. If the marker exists without the install line, stop and diagnose the wrapper-specific error.

## Runtime test

Run a short natural COM-vs-COM session. Do not force-edit object fields or player state. Keep Stage16/17/18/19 enabled and collect:

- Build.log
- ArcanaAI_Bit113ObjectGuardTrace.log
- ArcanaAI_Bit113GateTrace.log
- ArcanaAI_Bit113SetterTrace.log
- ArcanaAI_StateMapTrace.log
- ArcanaAI_PlayerBit17WriteTrace.log
- ArcanaAI_ProducerCallTrace.log
- ArcanaAI_QueueTrace.log
- ArcanaAI_EventDispatchTrace.log
- ArcanaAI_FinalCommandTrace.log
- ArcanaAI_AlternateCallTrace.log

## Analysis targets

1. Correlate each Stage20 guard record with its preceding Stage18 gate result by side, player pointer and QPC.
2. Test whether the 31 P2 stateIndex-0x19 gate-only calls have Stage20 AL=1 and bit 0x4 set in the checked object DWORD.
3. Confirm that all four known Stage17 setter episodes have Stage20 AL=0 and bit 0x4 clear.
4. Confirm that Stage20 returns to the untouched TEST/JNE without changing the original helper result.
5. Continue looking for a natural Stage18 AL=0 result. Stage20 does not observe calls rejected by the earlier table gate.

Stage20's expected correlation pattern is a hypothesis to validate; no semantic interpretation should be assigned to the object field beyond the directly observed bit test.


## Build result — successful (2026-11-08 19:55:42)

The user supplied the complete build transcript in chat. The Stage20 overlay was applied to the assembled Stage11 project and the build completed successfully:

- Visual Studio 18 Community; MSVC 14.51.36231; x86 / Win32.
- All eight C++ translation units compiled, including the new `AITrace.cpp`.
- Link step succeeded and produced `build\\dinput8.dll`.
- Final DLL size: **216,576 bytes**.
- One non-fatal existing C4754 warning remains in `AICommandSiteTrace.cpp` lines 782–785 (the arithmetic comparison warning is outside the Stage20 object-guard code).

The complete Build.log was pasted in chat but has not been uploaded as a repository file. The build is **build-verified** (VS 18 Community / MSVC 14.51.36231 / Win32; all eight translation units compiled; link succeeded; DLL 216,576 bytes; one non-fatal C4754 warning in AICommandSiteTrace.cpp 782–785). The subsequent runtime logs confirm that this Stage20 build's marker and wrapper were loaded and installed successfully.

## Runtime result — Stage20 wrapper confirmed

Runtime test: 2026-11-08, initialization 19:58:47; Stage20 wrapper installed at 19:58:48.575.

- The log contains the exact Stage20 build marker and `EnableStage20Bit113ObjectGuardTrace=1`.
- The observational wrapper installed at CALL RVA `0x000C64F1`, targeting helper RVA `0x00095B00`, resuming at `0x000C64F6`; 40 threads were suspended during installation. The original TEST/JNE remain untouched.
- 33 Stage20 object-guard records correlate one-to-one with 33 Stage18 gate records by side, player pointer and QPC (129–173 QPC ticks between each pair).
- All 33 are P2 calls. Every helper result was AL=1; every checked object DWORD was `0x0000004C`, with bit `0x4` set; every record has `readMask=0x3F`.
- Stage17's observer at RVA `0x000C6502` installed, but captured no setter call in this run. The Stage16 writer trace captured 28,043 writes and no bit17 transition; `oldBit17` and `newBit17` were zero in every record.
- No AL=0 / bit-2-clear control sample was captured. The four earlier Stage17 setter episodes belong to the previous run and are not retroactive Stage20 samples.
- Separately, the alternate callsite at RVA `0x000F6AD7` fired three times with payload `0x1E` (P2 once, P1 twice). All three queue inserts were accepted as type 1 / payload `0x1E`, and all three were observed in EventDispatchTrace.
- In each of those three episodes, Stage15 logged bit17=0 and a taken branch at `F6A68`; `F6AD7` was reached 620–851 QPC ticks later. The alternate queue/dispatch path therefore operated in this sample while bit17 was zero. Do not conflate it with the direct setter path at `0xC6502`.
- FinalCommandTrace captured 60 records but no payload `0x1E` at its monitored callsite RVA `0x000F7C5D`. This is a scope-limited no-hit, not proof that the dispatched event was unhandled.

See `STAGE20_RUNTIME_ANALYSIS.md` for full counts, QPC correlations, Stage19 map context and limitations. The runtime logs uploaded on `main` are preserved at the root of this evidence branch.

## Next investigation

1. Obtain a natural Stage20 call where `[object+0x3B70] & 4` is clear and capture the actual helper AL=0 together with the corresponding Stage18 gate and Stage17 setter outcome.
2. Continue tracing P1 as well as P2; this run produced Stage20 object-guard records only for P2.
3. Statically map the consumer of dispatched type-1 / payload-`0x1E` beyond the observed `0x000EE5CB` dispatch point. Do not assume the `0x000F7C5D` FinalCommand hook covers this alternate path.
4. Investigate the `F6A68` / `F6A9D` branch semantics: all three observed episodes had bit17=0 and taken branches, yet still reached `F6AD7` and queued payload `0x1E`.

Keep the Stage19 source/evidence and all Stage12–Stage19 historical branches unchanged.
