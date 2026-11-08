# Next session handoff

**Read this file first in a new chat.** Inspect current GitHub branch HEADs and uploaded files before asking the user for anything.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- Latest runtime evidence branch: `stage17-bit113-setter-callsite-trace`
- Latest runtime branch HEAD: `0c2411187c47e56a6cb675b597eeb8a754f8f8a6`
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

## Only immediate missing item: Stage17 caller log

The uploaded branch does not contain `ArcanaAI_Bit113SetterTrace.log`. The active Stage17 callsite source in `stage17-bit113-setter-callsite-trace` hooks the direct CALL at RVA `0x000C6502`, expected target RVA `0x0000FAA0`. Its supplied INI key is `EnableStage17Bit113SetterCallTrace=1`.

The separate Stage17 log should contain either an install/validation result or an error, and if the site executes, `BIT113_SET_CALL` entries with:
- `callRVA=0x000C6502`
- `returnRVA=0x000C6507`
- `index=0x00000071`
- player identity, registers, related-object values, stack and QPC.

**Ask for/upload only this missing log first.** Do not ask the user to rebuild or rerun yet. If the file exists in the same folder as the other ArcanaAI logs, upload it. If it was never created, then verify the actual game-folder `ArcanaAI.ini` contains `EnableStage17Bit113SetterCallTrace=1` and that the newly built `dinput8.dll` was installed beside the game executable; only then decide whether a short targeted rerun is needed.

Until the Stage17 log is reviewed, do not claim the runtime setter calls have been attributed to RVA `0x000C6502`. The bit17 setter/clearer instructions and their branch/queue relationship are confirmed; the caller remains unconfirmed.

## Historical branch rules

- Do not rewrite Stage12–Stage16 evidence branches.
- Keep `stage17-bit113-setter-caller-trace` and `stage17-bit113-setter-callsite-trace` distinct; they use different Stage17 hook placements.
- Append each completed analysis to `PROJECT_HISTORY.md`, update this handoff, and keep `README.md` as the entry point.
- Distinguish successful compilation from hook installation; only runtime log evidence proves the latter.
