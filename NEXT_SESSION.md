# Next session handoff

**Read this file first in a new chat.** Inspect current GitHub branch HEADs and uploaded files before asking the user for anything.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- Latest runtime evidence branch: `stage19-bit113-state-map-runtime-evidence`, HEAD `3e75c07c7bdbd0827112f6a488936587a41a1b4b`; contains Stage19/18/17/16/15 logs and the exact Stage19 overlay plus runtime analysis. Build.log was not included.
- Current diagnostic source branch: `stage20-bit113-object-bit2-guard-trace`, HEAD `c46738caf098fe187fdb2dae16310efc4da13390`; source-only, awaiting MSVC build and runtime verification.
- Stage19 source snapshot (parent of Stage20): `stage19-bit113-state-map-trace`, HEAD `841142895a976419a9bda197be286e7434993067`; keep separate from the Stage19 evidence branch.
- Stage18 source snapshot: `stage18-bit113-gate-condition-trace`, HEAD `520eec4cd216795e8ffbaa8738b3602b5e40d39a`; Stage18 runtime evidence remains preserved in `stage18-bit113-gate-runtime-evidence`.
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

## Stage19 runtime result — mapping confirmed (2026-11-08)

Evidence branch: [stage19-bit113-state-map-runtime-evidence](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-runtime-evidence), HEAD `3e75c07c7bdbd0827112f6a488936587a41a1b4b`. Source branch remains [stage19-bit113-state-map-trace](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-trace), HEAD `841142895a976419a9bda197be286e7434993067`. The uploaded logs were placed in root `main`; the evidence branch preserves them with the exact Stage19 overlay and `Stage19_Overlay/STAGE19_RUNTIME_ANALYSIS.md`.

### Installation and limitations

- Run initialized 2026-11-08 at 19:05:37.
- Stage17 hook at RVA `0x000C6502`, Stage18 hook at RVA `0x000C64A5`, and Stage19 hook at RVA `0x000F6C08) all installed successfully; each reported 44 suspended threads.
- Stage19 marker/config show `STAGE19_BIT113_STATE_MAP_BUILD=2026-11-08-r1` and `EnableStage19StateMapTrace=1`.
- **Build.log was not uploaded.** Runtime proves a DLL containing the Stage19 instrumentation loaded and its hook installed, but compiler/linker output and final DLL size are unavailable.

### Confirmed Stage19 mapping results

There are 1,235 `STATE_MAP_CALL` records, all with `readMask=0x0F`, and no Stage19 drop/warning/error.

| Side | Raw code | Helper EAX | WORD flags | Derived state index | Calls |
|---|---:|---:|---:|---:|---:|
| P1 | `0x20` | 0 | `0x0400` | `0x18` | 200 |
| P1 | `0x22` | 0 | `0x0400` | `0x18` | 190 |
| P1 | `0x24` | 0 | `0x0400` | `0x18` | 130 |
| P1 | `0x29` | 0 | `0x0400` | `0x18` | 12 |
| P1 | `0x4A` | 3 | `0x2000` | `0x1B` | 152 |
| P2 | `0x58` | 1 | `0x0802` | `0x19` | 82 |
| P2 | `0x59` | 1 | `0x0982` | `0x19` | 297 |
| P2 | `0x5C` | 1 | `0x0802` | `0x19` | 56 |
| P2 | `0x5E` | 3 | `0x2002` | `0x1B` | 116 |

Helper EAX totals: 532×0, 435×1, 268×3. The observed WORD flags agree with the helper priority checks. QPC correlation with Stage18 confirms raw `0x59→EAX 1→index 0x19`, raw `0x22→0→0x18`, and raw `0x20→0→0x18` in the matching gate calls. Indexes generated by raw codes `0x4A` and `0x5E` did not appear in Stage18's bit-113 gate records in this run; do not assume every map-helper call proceeds through that caller.

### Stage18 / Stage17 / Stage16 / Stage15 correlation

- Stage18 logged 42 gate results: all `AL=1`, all post-return table-byte readbacks `0xFA`. No `AL=0) sample.
- Parent-return attribution: 38 direct-call path entries (`0x00DB6801`) and four callback path entries (`0x00DEB9FA`).
- Stage17 logged eight bit-113 setter calls (two per episode).
- Stage16 logged 17,361 writes, including exactly four bit17 `0→1` writes at RVA `0x0000FAD2) and four clears at RVA `0x000ED1A9).
- Stage15 captured four payload-`0x1E` attempts. Every `F6A68` saw bit17=1 and `branchTaken=0`; no `F6A9D`, `F6AAC), or `F6AD7` invocation followed. QueueTrace, EventDispatchTrace and FinalCommandTrace contain no payload-`0x1E) insertion/dispatch.
- Most important new observation: 31 P2 gate calls at state index `0x19) returned `AL=1) but were not followed by setter calls. Therefore the table gate is not sufficient to guarantee the setter is reached.

No warning/error/drop was found in the reviewed Stage19, Stage18, Stage17, Stage16, Stage15, Queue, Dispatch, FinalCommand or AlternateCall logs.

## Current task — build and runtime-test Stage20

Source branch: [stage20-bit113-object-bit2-guard-trace](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-guard-trace), HEAD `c46738caf098fe187fdb2dae16310efc4da13390`. It branches from the immutable Stage19 source snapshot, not from the evidence branch.

### Why Stage20 targets the object guard

Static disassembly of the matching `AALib.exe` (SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`) confirms the code after the table gate:

```asm
004C64ED  mov ecx,[ebx]
004C64EF  push 2
004C64F1  call 00495B00
004C64F6  test al,al
004C64F8  jne 004C640B
004C64FE  push 71h
004C6500  mov ecx,esi
004C6502  call 0040FAA0
```

Helper `0x00495B00` uses argument 2 to form mask `0x4), checks `[object+0x3B70]`, and returns AL=1 when the bit is set. The JNE skips the setter on AL=1. The four known setter episodes had `OtherObject3B70` values `0x440`, `0x640`, `0x460`, `0x640`, with bit 2 clear. A set bit 2 could explain the 31 P2 index-`0x19` gate-only cases, but this is **not yet runtime-confirmed**.

### Stage20 observer contract

The new wrapper replaces only the five-byte CALL at RVA `0x000C64F1`. It:
- saves the original ECX object pointer for logging;
- duplicates stack argument 2 and calls the real `0x00495B00` helper;
- lets the real helper return with its own `RET 4);
- logs the actual AL/EAX, saved original ECX, EBX slot pointer, `[EBX]`, `[object+0x3B70]`, bit-2 state, player fields, EBP parent return and QPC;
- restores helper return registers/EFLAGS, discards the saved ECX with flag-neutral LEA, and executes `RET 4) for the original caller argument;
- leaves the original `TEST AL,AL` at `0x000C64F6) and JNE at `0x000C64F8) untouched.

It adds no DR0–DR3 watchpoints. New log: `ArcanaAI_Bit113ObjectGuardTrace.log`. Stage16–Stage19 switches remain enabled.

### Next actions

1. Apply `Stage20_Overlay/src/AITrace.cpp` and `Stage20_Overlay/ArcanaAI.ini` over the assembled Stage19 project; follow [README_STAGE20.md](https://github.com/Hawky11/1/blob/stage20-bit113-object-bit2-guard-trace/Stage20_Overlay/README_STAGE20.md).
2. Build `Release | Win32` with MSVC and upload the complete `Build.log`. This is a source-only branch; it has not been built or runtime-tested.
3. Back up the working DLL. Install only after a successful compile/link, then first verify the Stage20 build marker/config and successful wrapper installation at RVA `0x000C64F1).
4. Run a short natural COM-vs-COM session. Upload `ArcanaAI_Bit113ObjectGuardTrace.log`, Stage18/17/16/19 logs and the related Stage15/queue/dispatch logs.
5. Test whether the 31 P2 index-`0x19` gate-only cases show object-guard AL=1 and bit 2 set, while all four setter episodes show AL=0 and bit 2 clear. Do not force-edit object/player memory to manufacture outcomes.
6. Continue looking for a natural Stage18 `AL=0) on a valid `0xFF` table cell; Stage20 does not cover attempts rejected by the earlier table gate.

Preserve the Stage19 source/evidence branches and all Stage12–Stage19 historical branches unchanged.

## Historical branch rules

- Do not rewrite Stage12–Stage16 evidence branches.
- Keep `stage17-bit113-setter-caller-trace` and `stage17-bit113-setter-callsite-trace` distinct; they use different Stage17 hook placements.
- Append each completed analysis to `PROJECT_HISTORY.md`, update this handoff, and keep `README.md` as the entry point.
- Distinguish successful compilation from hook installation; only runtime log evidence proves the latter.
