# Next session handoff

**Read this file first in a new chat.** Inspect current GitHub branch HEADs and uploaded files before asking the user for anything.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- Latest runtime evidence branch: `stage17-bit113-setter-callsite-trace` (preserved historical snapshot)
- Runtime evidence branch HEAD: `0c2411187c47e56a6cb675b597eeb8a754f8f8a6`
- Current diagnostic-source branch: `stage17-callsite-init-diagnostics`
- Diagnostic-source branch HEAD: `c33317c9b2fa28c339c38f3aaebdcb9a378b73a1`
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

## Immediate next task: build and verify Stage17 initialization diagnostics

The 2026-11-08 15:40:27–15:41:09 runtime run did not create `ArcanaAI_Bit113SetterTrace.log`. Do **not** immediately request another match. The original callsite source had early-return paths that exited before writing any Stage17-specific line, so the missing file did not identify the failure stage.

A diagnostic-source revision is now available in branch `stage17-callsite-init-diagnostics`, HEAD `c33317c9b2fa28c339c38f3aaebdcb9a378b73a1`. It is based on the prior runtime-evidence branch; that original branch is unchanged. The revised `AITrace.cpp`:

- writes marker `STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r1` and a worker-entry line before validation;
- logs `EnableControlFieldTrace`, `EnablePlayerBit17WriteTrace`, and `EnableStage17Bit113SetterCallTrace`;
- records Stage17-specific messages for the former early-return paths;
- fixes the logger's CRLF formatting.

### Required procedure

1. Start from the Stage11 source archive and apply the overlays in order through Stage16, then apply `Stage17_Overlay/src/AITrace.cpp` and `Stage17_Overlay/ArcanaAI.ini` from `stage17-callsite-init-diagnostics`.
2. Before compiling, run this PowerShell check from the project root; it must find the marker:
   ```powershell
   Select-String -Path .\\src\\AITrace.cpp -SimpleMatch 'STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r1'
   ```
3. Build Win32/x86 and retain the full Build.log. A successful build must still show `AITrace.cpp` being compiled, but the source-marker check proves the correct overlay was present before the build.
4. Install the newly built `dinput8.dll` **and** this branch's `ArcanaAI.ini` beside the actual game executable / `AALib.exe`. Keep the previous working DLL backed up.
5. Launch the game and check for `ArcanaAI_Bit113SetterTrace.log` before doing another full test. Its first lines must include the build marker, `STAGE17_INIT`, and `STAGE17_CONFIG`. The config line should report `EnableStage17Bit113SetterCallTrace=1`.
6. If that file appears, inspect its install/validation message. Only when it reports the hook installed should a short COM-vs-COM session be used to collect `BIT113_SET_CALL` entries. Then upload the Stage17 log, Build.log and fresh Stage16/Stage15 correlation logs.

If the other ArcanaAI logs are written but the new marker file is still absent, stop: verify the path of the DLL actually loaded by the game and compare it with the newly built binary. Do not run another match merely to recreate logs.

The original callsite candidate remains unconfirmed until runtime records show `callRVA=0x000C6502`, `returnRVA=0x000C6507`, `index=0x00000071`, and the correct P1/P2 player pointer correlated with the Stage16 writer events. No build or runtime test has yet been performed on the diagnostic revision.
 
## Historical branch rules

- Do not rewrite Stage12–Stage16 evidence branches.
- Keep `stage17-bit113-setter-caller-trace` and `stage17-bit113-setter-callsite-trace` distinct; they use different Stage17 hook placements.
- Append each completed analysis to `PROJECT_HISTORY.md`, update this handoff, and keep `README.md` as the entry point.
- Distinguish successful compilation from hook installation; only runtime log evidence proves the latter.
