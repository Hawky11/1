# Next session handoff

**Read this file first in a new chat.** Inspect current GitHub branch HEADs and uploaded files before asking the user for anything.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- Latest runtime evidence branch: `stage17-callsite-init-diagnostics`
- Latest runtime evidence HEAD: `dcfd1efbe221438b1f2b3fae800ca76b1fa87e7d`
- Earlier Stage17 pre-fix branch (preserve unchanged): `stage17-bit113-setter-callsite-trace`, HEAD `0c2411187c47e56a6cb675b597eeb8a754f8f8a6`.
- Earlier Stage17 entry-hook source branch (preserve unchanged): `stage17-bit113-setter-caller-trace`, HEAD at last review `6b04b0a9ad272aecf8dcb481b5c52edfe5acfe1e`.
- Stage16 baseline evidence branch: `stage16-player-bit17-writer-trace`, HEAD `58313d8c0f6071bc8099fc5ec04ae04d2974dafd`.
- Stage17 overlay/readme: [Stage17 README](https://github.com/Hawky11/1/blob/stage17-callsite-init-diagnostics/Stage17_Overlay/README_STAGE17.md).
- Full chronology: [PROJECT_HISTORY.md](PROJECT_HISTORY.md).

## Latest build result — successful

The user supplied Build.log dated 2026-11-08 16:32:02. VS 2026 18.9.1 / MSVC 14.51.36231 x86 compiled all eight C++ translation units and linked `build\\dinput8.dll`, size **198,144 bytes**. One non-fatal C4754 warning remains in `AICommandSiteTrace.cpp` lines 782–785. Build.log was pasted in chat, not uploaded in the branch tree.

## Stage17 runtime result — caller attribution confirmed for this run

Runtime run: 2026-11-08, initialization at 16:34:05, relevant events 16:34:29–16:34:44.

- Stage17 r2 marker, worker entry, and INI settings are present.
- The passive observer installed successfully at callsite RVA `0x000C6502`, return RVA `0x000C6507`, original helper RVA `0x0000FAA0`; `suspended=34`.
- `ArcanaAI_Bit113SetterTrace.log` contains nine `BIT113_SET_CALL` records; every record uses `index=0x00000071`, the exact call/return RVAs, and the correct P1/P2 pointer.
- Stage16 PlayerBit17WriteTrace contains 20,456 records and eight actual bit17 transitions (four sets and four clears). There are 62 thread-watchpoint installations across 10 batches, with no warning/error/drop/skip lines.
- All four Stage16 0→1 writes at RVA `0x0000FAD2` have a preceding Stage17 call from `0x000C6502` with the same player pointer and bit index. QPC deltas are 367, 520, 396, and 353 ticks. The four corresponding clears again execute `0x000ED19F` (post-EIP `0x000ED1A9`).
- Five additional Stage17 calls occurred while the bit was already set; they did not create extra transitions.
- Four Stage15 F6A68 events all saw bit17=1 and the original JZ was not taken. No F6A9D/F6AAC/F6AD7 call was recorded; QueueTrace and EventDispatchTrace contain no type-1 / payload-`0x1E` insertion/dispatch in this run. This is a suppression-side sample, not a successful queue insertion sample.
- No warning/error/drop reports were found in the reviewed Stage17, Stage16, producer, queue, dispatch, or alternate-callsite logs.

**Conclusion:** the candidate direct caller at `AALib.exe+0xC6502` is runtime-confirmed as the origin of all four bit17 set transitions observed in this run. This does not prove it is the only possible setter path for every game state. Preserve the current evidence branch unchanged.

## Next investigation — caller conditions (Stage18)

The remaining question is why/under which state path caller function RVA `0x000C63B0` invokes the bit-113 setter.

Useful observations from Stage17 records:
- `otherObject` equals the player pointer; `other3B70` is `0x650` or `0x640`, both with bit index 2 clear. This is consistent with the final `0x495B00` guard permitting the call.
- At the callsite, the low word captured at player `+0x3A36` is `0x42`, `0x22`, `0x20`, or `0x5E`, not the `0x1A/0x18/0x19` values in the documented straight-line comparison sequence. Reconstruct earlier incoming branches and determine whether an alternate branch reaches the final block or an intervening helper changes the field. This is an open question, not a reason to reject the proven callsite.
- The five repeated setter calls while bit17 was already set should be related to their parent-frame/branch context; do not assume they are additional state transitions.

Recommended Stage18 workflow:
1. Reinspect the complete function at `0x004C63B0` from the verified AALib.exe and map every branch target that can reach the final call at `0x004C6502`.
2. Correlate Stage17 `EBP/EBX/ESI`, player fields, and `other3B70` with the earlier helper calls (`0x00401F30`, `0x004F6ED0`, `0x004F2FD0`, `0x00495B00`). First determine statically which branch sites are decisive; only add narrow runtime probes where static review cannot settle the path.
3. Keep the instrumentation passive and validate each original byte sequence/target before patching. Preserve Stage17 r2 logs and source branch intact; create a new Stage18 branch for any new probes.
4. Continue using Stage15/16/17 QPC/player-pointer correlation to verify any new hypothesis. Avoid another broad run until the Stage18 trace has a specific unanswered question.

## Historical branch rules

- Do not rewrite Stage12–Stage16 evidence branches.
- Keep `stage17-bit113-setter-caller-trace` and `stage17-bit113-setter-callsite-trace` distinct; they use different Stage17 hook placements.
- Append each completed analysis to `PROJECT_HISTORY.md`, update this handoff, and keep `README.md` as the entry point.
- Distinguish successful compilation from hook installation; only runtime log evidence proves the latter.
