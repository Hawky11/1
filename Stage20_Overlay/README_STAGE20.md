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
