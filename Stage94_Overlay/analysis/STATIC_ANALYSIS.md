# Stage94 r1 — original AALib.exe and passive native boundary audit

**Status: source-only.** The user's verified Stage93 DLL (2,019,840 bytes) is the last Windows-built/game-tested version. Stage94 has NOT been built in MSVC or run in-game.

## Exact native evidence

- Original AALib.exe SHA-256: 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba; PE32 x86, TimeDateStamp 5FE5AA91, ImageSize 0E0EB000.
- RVA F7AD1: E8 5A BE FF FF — CALL F3930, which returns a candidate command EAX.
- RVA F7AD6: 83 C4 14 8B F0 — ADD ESP,14h; MOV ESI,EAX, replacing previous candidate ESI. Stage94 observes post-F3930 true EAX, then replays these original five bytes exactly once through explicit _emit.
- RVA F7ADB: 8B 4D F8 83 F9 FF — another branch may overwrite ESI later at F7AEE; not every type1 result originates in F3930.
- RVA F7B07: 83 F9 01 — CMP ECX,1.
- RVA F7B0A: 0F 85 58 01 00 00 — native JNE F7C68. Stage94 observes original EFLAGS after CMP and replays JNZ with both original branch destinations unchanged: allow F7B10, reject F7C68.
- RVA F7B10: 83 FE 20 — CMP ESI,20h.
- RVA F7C59: 56 6A 01 53 — PUSH ESI; PUSH 1; PUSH EBX.
- RVA F7C5D: E8 7E B1 FF FF — original CALL F2DE0, whose existing FinalCommand observer/Stage93 QPC bridge must NOT be hooked again.

## Scope and safety

Only two native sites are patched: F7AD6 (5 bytes) and F7B0A (6 bytes). Stage93 installation is verified first. Exact original bytes and surrounding instructions, bounded ring, peer EIP exclusion, patch mutex, recheck after suspension, and rollback on partial patch failure are built in. No added gameplay-state/DR writes and no Stage36. Existing FinalCommand code unchanged.

Stage94_PRODUCER logs native EAX after real F3930; Stage94_GATE logs ESI, ECX, actual ZF and side/player/thread/EBP/QPC. The most recent producer sharing player/thread/stack frame within a bounded QPC interval is marked only as a *candidate*, never a guaranteed source. Correlate Stage94_GATE (forward type1) to the already-proven STAGE93_FINAL_QPC real payload. Actor/TID/QPC proximity plus matching ESI is strong correlation, not necessarily a unique causal explanation if alternate native writers intervene.

## Verification and runtime handoff

Original executable and source/offline verifier: **38/38 PASS**, including 4,096 branch-flag equivalence cases. Additional installer/correlator tests: **13/13 PASS**, covering wrong player, stale QPC, payload mismatch, health errors, exact Stage93 wiring, rollback guards. GNU x86 assembler confirms the original six-byte near-JNE; producer replay uses explicit original five bytes to avoid equivalent but differently encoded MOV instructions.

The ChatGPT-delivered Stage94 source ZIP includes the exact core files here, full SHA manifest, offline verification and correlation tools. Windows MSVC compilation and live game installation are **not yet verified**. Keep the tested 2,019,840-byte Stage93 DLL and Stage36 OFF. Apply SHA-gated ApplyStage94.ps1, build MSVC x86, require STAGE94_INSTALLED 2/2 and clean STAGE94_HEALTH in the same run as STAGE93_FINAL_QPC. On errors stop/rollback without disabling guards.
