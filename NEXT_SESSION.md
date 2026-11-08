# Next session handoff

**Read this file first in a new chat.** Inspect current GitHub branch HEADs and uploaded files before asking the user for anything.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- Latest runtime evidence branch: `stage17-bit113-setter-callsite-trace` (preserved historical snapshot)
- Runtime evidence branch HEAD: `0c2411187c47e56a6cb675b597eeb8a754f8f8a6`
- Current diagnostic-source branch: `stage17-callsite-init-diagnostics`
- Diagnostic-source branch HEAD: `ebabe7faa0521115425e56d9b6880b76e1a48806`
- Earlier Stage17 entry-hook source branch (preserve unchanged): `stage17-bit113-setter-caller-trace`, HEAD at last review `6b04b0a9ad272aecf8dcb481b5c52edfe5acfe1e`.
- Stage16 baseline evidence branch (preserve unchanged): `stage16-player-bit17-writer-trace`, HEAD `58313d8c0f6071bc8099fc5ec04ae04d2974dafd`.
- Stage17 callsite overlay README: [Stage17_Overlay/README_STAGE17.md](https://github.com/Hawky11/1/blob/stage17-bit113-setter-callsite-trace/Stage17_Overlay/README_STAGE17.md).
- Full chronology: [PROJECT_HISTORY.md](PROJECT_HISTORY.md).

## Latest build result — successful

The user supplied Build.log dated 2026-11-08 15:39:59. Visual Studio 2026 18.9.1 / MSVC 14.51.36231 x86 initialized successfully. All eight C++ translation units compiled and `build\\dinput8.dll` linked successfully; DLL size 190,976 bytes. One non-fatal C4754 warning occurred in `AICommandSiteTrace.cpp` lines 782–785 for the stack snapshot address calculation. Do not describe this build as failed. Build.log was pasted in conversation but is absent from the GitHub branch tree.

## Runtime findings already established

Run: approximately 15:40:27–15:41:09 on 2026-11-08.

- `ArcanaAI_PlayerBit17WriteTrace.log`: 22,739 write records, 60 game-thread watchpoint installs, six actual bit17 transitions (three set, three clear), and no warnings/errors/drops in the file.
- All three set operations used generic bitfield setter VA `0x0040FAD0` post-EIP `0xFAD2`, with EAX equal to the exact P1/P2 field address and EDX=`0x00020000`.
- All three clears used VA `0x004ED19F` post-EIP `0xED1A9`.
- Six Stage15 F6A68 events were recorded: three with bit17=1 and JZ not taken; three with bit17=0 and JZ taken. The three set events align by QPC with bit17=1 at F6A68 and a subsequent clear. The three bit17=0 events reached F6AD7.
- The three F6AD7 type-1/payload-`0x1E` writes were accepted by QueueTrace and observed by EventDispatchTrace.
- All six matching parent events had F2F50 return EAX=`0x11E`, F2ED0 return AL=0, and EDI=`0x1E`.
- Reviewed producer, queue, dispatcher and write logs show no warnings/errors/drops.

This is a second independent six-event confirmation of the bit17/F6A68 correlation.

## Immediate next task: install and verify the successfully built Stage17 r2 DLL

The 2026-11-08 15:40:27–15:41:09 runtime run did not create `ArcanaAI_Bit113SetterTrace.log`. The corrected Stage17 r2 source has now been successfully built, so the next step is runtime installation and startup verification—not another build. Do **not** immediately run a full match.

A diagnostic-source revision is now available in branch `stage17-callsite-init-diagnostics`, current HEAD `34fbf14975b3e5d1b9dd6bdec0c3d6c8756bc25f`. It is based on the prior runtime-evidence branch; that original branch is unchanged. The first build with the r1 marker present exposed 14 MSVC C3861 errors because `ReadDwordAtAddress` was used before declaration in the Stage17 recorder. This has been fixed in the current branch: a forward declaration now appears immediately before `QueueStage17Bit113SetterRecord`, and the implementation remains later in the same anonymous namespace.

The revised `AITrace.cpp`:

- declares `ReadDwordAtAddress` before its first use;
- writes marker `STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r2` and a worker-entry line before validation;
- logs `EnableControlFieldTrace`, `EnablePlayerBit17WriteTrace`, and `EnableStage17Bit113SetterCallTrace`;
- records Stage17-specific messages for the former early-return paths;
- fixes the logger's CRLF formatting.

### Required procedure

1. Back up the previously installed proxy DLL, then copy the new `build\\dinput8.dll` (198,144 bytes) and the matching `ArcanaAI.ini` from the diagnostic Stage17 overlay beside the actual game executable / `AALib.exe`.
2. Launch the game. Before a full COM-vs-COM match, check whether `ArcanaAI_Bit113SetterTrace.log` is created.
3. Its first lines must include:
   - `STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r2`
   - `STAGE17_INIT: FieldTraceThread entered.`
   - `STAGE17_CONFIG:` with `EnableControlFieldTrace=0`, `EnablePlayerBit17WriteTrace=1`, and `EnableStage17Bit113SetterCallTrace=1`.
4. Inspect the next Stage17 line. It should report the validated passive observer installed at callsite RVA `0x000C6502`, return RVA `0x000C6507`, helper RVA `0x0000FAA0`, and suspended-thread count. If there is a validation/patch error, stop and inspect it before running a match.
5. Only after successful hook installation, run a short COM-vs-COM session and upload `ArcanaAI_Bit113SetterTrace.log` with fresh Stage16/Stage15 correlation logs. The relevant records must show `callRVA=0x000C6502`, `returnRVA=0x000C6507`, `index=0x00000071`, and the matching P1/P2 player pointer.
6. If the other ArcanaAI logs are written but no Stage17 marker file appears, stop and verify the actual loaded DLL/INI paths. Do not repeat a full test blindly.

The successful r2 build proves the corrected source compiles and links; it does **not** prove the DLL has been installed/loaded or the hook has been installed. Do not claim runtime caller attribution until the Stage17 records correlate with Stage16 writer events.

## Historical branch rules

- Do not rewrite Stage12–Stage16 evidence branches.
- Keep `stage17-bit113-setter-caller-trace` and `stage17-bit113-setter-callsite-trace` distinct; they use different Stage17 hook placements.
- Append each completed analysis to `PROJECT_HISTORY.md`, update this handoff, and keep `README.md` as the entry point.
- Distinguish successful compilation from hook installation; only runtime log evidence proves the latter.
