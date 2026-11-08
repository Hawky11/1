# Next session handoff

**Read this file first in a new chat.** Inspect current GitHub branch HEADs and uploaded files before asking the user for anything.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- Latest runtime evidence branch: `stage18-bit113-gate-runtime-evidence`, HEAD `0139152b9eeb778a23ec4f3197c57faff749a78a`; Stage17 runtime evidence remains preserved in `stage17-callsite-init-diagnostics`.
- Current diagnostic source branch: `stage19-bit113-state-map-trace`, HEAD `841142895a976419a9bda197be286e7434993067`; source-only, awaiting MSVC build and runtime verification.
- Stage18 source snapshot (parent of Stage19): `stage18-bit113-gate-condition-trace`, HEAD `520eec4cd216795e8ffbaa8738b3602b5e40d39a`. Its runtime result is documented below and its separate evidence branch remains immutable.
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

## Stage18 runtime result — complete for the tested scenario

- Latest runtime evidence branch: [stage18-bit113-gate-runtime-evidence](https://github.com/Hawky11/1/tree/stage18-bit113-gate-runtime-evidence), HEAD 0139152b9eeb778a23ec4f3197c57faff749a78a. This branch combines the user's 2026-11-08 18:00 uploaded runtime logs with the Stage18 overlay and Stage18_Overlay/STAGE18_RUNTIME_ANALYSIS.md.
- Source snapshot branch: [stage18-bit113-gate-condition-trace](https://github.com/Hawky11/1/tree/stage18-bit113-gate-condition-trace), HEAD 520eec4cd216795e8ffbaa8738b3602b5e40d39a. Keep it as the source-stage snapshot.
- Runtime start: 18:00:15. Stage18 marker/config and installation at RVA 0xC64A5 are present; Stage17 installation at RVA 0xC6502 is also confirmed. Both hooks report 44 suspended threads.
- **Build.log was not uploaded.** Runtime markers and installation messages prove the loaded DLL ran both hooks, but compiler/linker diagnostics and final DLL size are unavailable.

### Confirmed Stage18 findings

- Six Stage18 gate records; all returned AL=1 and the safely-read post-return byte was 0xFA. The unique state/table cases were P1 index 0x0E / selector 0x0A, P2 index 0x19 / selector 0x17, and P1 index 0x18 / selector 0x0A. No AL=0 sample was captured.
- Stage17 recorded six calls from 0xC6502, all passing bit index 0x71. Stage16 recorded 15,094 write records and three 0→1 bit17 transitions plus three clears.
- Each episode has two calls into caller function 0x4C63B0. One arrives directly from 0x4967FC (return 0x496801); the other is reached via indirect CALL EDX at 0x4CB9F8 (return 0x4CB9FA). Static table entry 0xA at 0x428AF18 points to 0x4C63B0, explaining the second invocation through callback/function-table dispatch.
- The first setter call in each episode sets bit17; the second repeats the OR when it is already set. QPC deltas are in PROJECT_HISTORY.md and STAGE18_RUNTIME_ANALYSIS.md.
- Three target Stage15 attempts had EDI=0x1E, bit17=1, ZF=1 and branchTaken=0. No F6A9D/F6AAC or F6AD7 invocation followed. QueueTrace/EventDispatchTrace/FinalCommandTrace each show 27 type-1 records for other payloads and none for 0x1E.

### Current task — build and runtime-test Stage19

Stage19 source branch: [stage19-bit113-state-map-trace](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-trace), HEAD `841142895a976419a9bda197be286e7434993067`. It was created directly from the immutable Stage18 source snapshot `stage18-bit113-gate-condition-trace` (HEAD `520eec4cd216795e8ffbaa8738b3602b5e40d39a`), not from the Stage18 evidence branch.

#### New static findings

- The three Stage18 cells that read back as `0xFA` are already `0xFA` in the matching on-disk PE `.data` image. Their preferred VAs (`0x04C7DBDD`, `0x04C8073B`, `0x04C7DCE1`) relocate by `+0x00920000` to the exact Stage18 runtime addresses. This proves those initial values are present in the PE; it does not prove that writable runtime tables can never change.
- State index conversion occurs in `0x4F6BA0`, via helper `0x4F02C0`. Raw codes `<=0x17` are returned directly; `0x18..0x1F` are rejected; other mapped codes use the WORD at `player+0x2B9E+(rawCode-0x20)*0x14`.
- The observed P1 raw code `0x20` mapped to state index `0x18`; P2 raw code `0x5C` mapped to `0x19`. The exact helper return values (expected 0 and 1 respectively) have not yet been captured directly; Stage19 records them.
- For the observed primary selectors, candidate valid `0xFF` rows at key `0x19` are P1 selector `0x0A`: `01,03,09,0F,14,15,16,17`; P2 selector `0x17`: `01,03,07,09,0D,0F`. The mapped rows `0x18..0x1B` are all `0xFA` for these selectors. Candidate direct states bypass the Stage19 converter hook but remain covered by Stage18's gate observer.

#### Stage19 observer

The new passive hook replaces six bytes at RVA `0x000F6C08` (original `ADD ESP,8; CMP EAX,5`) with a CALL and NOP. It logs the 0x4F02C0 EAX result, raw code, WORD flags, entry count, derived stateIndex, QPC and caller context to `ArcanaAI_StateMapTrace.log`. It removes its synthetic CALL return plus the two original cdecl arguments, recreates `CMP EAX,5`, and resumes at the untouched JNE at `0x000F6C0E`. It does not consume DR0–DR3. Stage16/17/18 remain enabled in the supplied INI.

#### Next actions

1. Apply `Stage19_Overlay/src/AITrace.cpp` and `Stage19_Overlay/ArcanaAI.ini` over the assembled Stage18 project; follow [README_STAGE19.md](https://github.com/Hawky11/1/blob/stage19-bit113-state-map-trace/Stage19_Overlay/README_STAGE19.md).
2. Build `Release | Win32` with MSVC and upload the complete `Build.log`. This source-only branch has not been compiled or runtime-tested; do not install the DLL unless compilation and linking both succeed.
3. Back up the working DLL, install the Stage19 build, and first check the Stage19 marker/config and successful six-byte hook installation at RVA `0x000F6C08`.
4. Run a short natural COM-vs-COM test. Do not force-edit state bytes to manufacture AL=0. Upload `ArcanaAI_StateMapTrace.log`, Stage18 gate, Stage17 setter, Stage16 bit17, and the relevant queue/dispatch/producer logs.
5. Verify the known conversions (`0x20 -> result 0 -> index 0x18`; `0x5C -> result 1 -> index 0x19`), compare the helper result with the logged WORD flags, and correlate by player pointer/QPC. Use Stage18's actual `resultAL` as the authoritative test for a reachable `0xFF` cell.

Preserve the Stage18 source and runtime-evidence branches and all Stage12–Stage17 historical branches unchanged.

## Historical branch rules

- Do not rewrite Stage12–Stage16 evidence branches.
- Keep `stage17-bit113-setter-caller-trace` and `stage17-bit113-setter-callsite-trace` distinct; they use different Stage17 hook placements.
- Append each completed analysis to `PROJECT_HISTORY.md`, update this handoff, and keep `README.md` as the entry point.
- Distinguish successful compilation from hook installation; only runtime log evidence proves the latter.
