# Stage65 — passive native request-origin observation (SOURCE ONLY)

Date: 2026-11-11. Branch: stage65-request-origin-passive-source.

## Evidence and status

**SOURCE PREPARED; ORIGINAL-BINARY STATIC CHECKS PASS; WINDOWS x86 BUILD AND GAME RUNTIME PENDING.** No new DLL, original EXE modification, INI setting or AI behavior change. The original user-owned private AALib.exe (175,556,608 bytes; SHA-256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba) is PE32/i386 with preferred ImageBase 00400000. The original executable remains private, not committed.

The exact following original bytes were independently checked locally in the materialized executable:

| RVA | Original bytes | Meaning |
|---|---|---|
| E5BF0 | 55 8B EC 80 7D 0C 00 | Native function entry and flag compare, 7 bytes on whole instruction boundaries |
| E5BF7 | 74 17 | Conditional branch retained in original |
| E5C06 | 89 81 4C 3F 00 00 | Native request write when low byte of mode !=0 |
| E5C13 | 89 91 4C 3F 00 00 | Native request write when mode=0 |
| E5C0C | 5D C2 08 00 | First native return: RET 8 |
| E5C19 | 5D C2 08 00 | Second native return: RET 8 |

**Synthetic checks:** independent local stack models for mode=0, mode=1, mode=2 PASS. Structural examination of GitHub overlay and adapter: 15/15 anchors PASS (single native trampoline call, exactly two argument pushes, matching FXSAVE/FXRSTOR pairs, no debug-register writes, correct include/install/flush order). The published verifier is not yet confirmed executed on the user's Windows installation; no native runtime/ABI execution or MSVC build has occurred.

## Single-entry trampoline design

Native function RVA E5BF0 is thiscall with ECX=P, stack arguments request ID and mode, and RET 8 on both paths. Stage65 redirects **one function entry**, not 296 original E8 callsites. It checks the original 7-byte entry and exact JE/RET8 signatures, creates an executable trampoline copying those 7 bytes then jumping to E5BF7, and replaces the entry in memory with a five-byte JMP plus two NOPs. Both pre-install and suspended-thread validations fail closed on opcode mismatches. Code patch protection/ICache and the existing patch-install mutex follow Stage38 conventions; no original EXE disk modifications.

The entry JMP preserves the original caller stack. The observer saves/restores GPR, EFLAGS, x87 and SSE; copies the two native parameters via two PUSH [esp+8] operations; invokes the original trampoline **once**; native RET 8 consumes the copied parameters; after a second observation/restoration pass, the observer RET 8 consumes the original caller arguments. This is a **source-level ABI model**, not a proven Windows machine-code runtime result.

Stage65 is installed only after successful Stage58 3/3 and **before starting the independent Stage59 watchpoint worker**. Stage65 install failure does not block Stage59 startup. No hardware DR0–DR3 breakpoints are added and Stage59's existing watchpoint handling is untouched.

## Files and integration

- [Stage65_Overlay/src/Stage65RequestOrigin.inl](../Stage65_Overlay/src/Stage65RequestOrigin.inl) — new guarded entry observer, P1/P2 filter, TLS depth 8, 8192-row SRW ring, copied native-call ABI, QPC.
- [Stage65_Overlay/src/Stage38CallbackWindow.inl](../Stage65_Overlay/src/Stage38CallbackWindow.inl) — Stage59 r2 adapter plus one include, independent install before Stage59 startup, and FlushStage65 through existing Stage38 worker.
- [Stage65_Analysis/tools/verify_stage65.py](tools/verify_stage65.py) — pinned original byte signatures, source integration invariants and synthetic stack model. This reproducer requires a local original AALib.exe; no private executable is published.

New log: ArcanaAI_Stage65RequestOriginTrace.log. STAGE65_WRITER_WINDOW includes side/player/object/TID, QPC before/after, original raw native return address, verified original direct-E8 callsite RVA if possible, request ID, full mode DWORD, native EAX post, request PRE/POST, P+3B7C PRE/POST, masks. The mode decision uses its low byte, not full DWORD. directE8=1 verifies [return-5] is an original E8 rel32 instruction targeting E5BF0. Indirect or non-original return sites retain raw return for later analysis. Health counters include total calls, filtered nonplayer calls, ring dropped, TLS overflow, mismatch and failed safe reads.

**Coverage limit:** only the E5BF0 native setter is traced. The independent Stage63 original conditional source E9E30/E9E60 remains uninstrumented. The 158/202 Stage42 activations not tied to Stage39 are **not yet attributed** to any writer, and may include this alternate path or other requests. Static callback xrefs and action IDs alone do not prove choice causality.

## Runtime acceptance gate — NOT DONE

Only after Windows MSVC x86 build succeeds: inspect Stage65 INSTALLED 1/1, Stage59 WATCH_ARMED, both P1/P2 writes, read masks and QPC ordering, and errors/overflow/ownership conflicts. Compare Stage65 request PRE/POST with original ID on both mode branches, and ensure original commands appear identically in Stage40/41; Stage42 activation may occur later. Correlate by side, player, thread, QPC, original request and callsite, accounting for overwrites, duplicate ID 0x49 and capture-tail gaps. A passive native detour can still change timing or crash if ABI assumptions fail; maintain the known working Stage59 r2 1,153,024-byte DLL as rollback, and never claim improved COM AI from this observer alone.

## Next Stage66

Once verified Windows x86 build and natural COM-vs-COM logs exist, match individual Stage65 writer records to Stage40/41/42, classify original caller RVAs, then follow the responsible original decision/branch/scoring path. If E5BF0 is incomplete, investigate the independent E9E30 source first. Do **not** change action IDs, AI priorities, coordinate writers, inputs or damage during tracing.
