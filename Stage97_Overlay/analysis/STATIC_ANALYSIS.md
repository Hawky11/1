# Stage97 r1 — inherited ESI at F77D4, source-only static audit

Date: 2026-11-12. Original user-Library `AALib.exe`, 175,556,608 bytes,
SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
This note is native static evidence, **NOT** a confirmed live explanation for Stage95 bypass seq 9461.

## Missing edge

Stage96 r1 live evidence: Stage95 bypass `seq=9461` P1, chosen `ESI=0x2C`, Stage94 gate `seq=1484` forwarded; the last Stage96 writer in same player/TID/EBP context was stale and carried `0x2B` (F79F0). 55/56 other bypass gates had a value-matching Stage96 writer. The unresolved event must not be marked attributed without a NEW same-run native witness.

Original `AALib.exe` static disassembly (RVA, `.text` file offset RVA-0xC00):

- F7181 `89 4D D4`: initialize stack local `[EBP-2C] = -1`.
- F7714 `C7 45 D4 1F 00 00 00`: exceptional candidate local becomes `0x1F`.
- F7749 `8B 55 CC; 8D 04 13`: load `[EBP-34]` and compute `EAX=EBX+EDX`; F7756 `89 45 D4`: store dynamic candidate `[EBP-2C]`.
- F77D4 `83 7D F8 FF`: compare `[EBP-8]` to `-1`; F77D8 `8B 75 D4`: MOV ESI,[EBP-2C]. F77DB `0F 85 BD 00 00 00`: JNE F789E.
- If original JNE at F77DB is taken: F789E->F7929, with F7924 skipped, conditional branches at F7939/F7953/F7964/F7974/F797D can reach F7A00, bypassing F79B6/F79D4/F79F0. From F7A00, branch to F7A94 without executing F7A8F is possible (F7A2E, F7A40, F7A47, F7A68); F7A94 is already Stage95's known bypass hook.
- Other early path F77B9 sets ESI=-1 and goes to F77E1, so not a direct 0x2C source unless a later writer overrides (which Stage96 sees).

Thus a complete **statically possible** path exists from F7756 dynamic candidate to F77D8 ESI to F7A94 bypass without visiting a Stage96 observer. For value `0x2C`, `[EBP-2C]` must come from dynamic F7756 in the original code (initial -1 and fixed 0x1F cannot yield 0x2C without corruption). Runtime needs to confirm the specific branch.

## Proposed witness and replay

New single 7-byte detour at F77D4; patch `E9 rel32 90 90` only after original signatures and exact installed Stage96 ownership all pass. Replay exactly `CMP [EBP-8],-1; MOV ESI,[EBP-2C]` (the seven overwritten bytes) once, restore native flags before replay and return to the untouched JNE at F77DB. Observe `player, side, tid, EBP, qpc, preEsi, localD4, localM8, flag3B70`, with validity/read counters. The logger records **before** the original CMP+MOV, so `nativeNextEsi=localD4` predicts the native post-instruction register; the original JNE is still executed normally.

Exact guards on F77D4 (7 B), F77DB (6 B and target), F7181 (3 B), F7714 (7 B), F7756 (3 B), verified Stage96 five exact native JMP owners, Stage95 and Stage96 installed, original AALib fingerprint. Suspended-peer EIP exclusion only for 7 new bytes; shared patch mutex, second guard check, native rollback on failed patch. GPR/EFLAGS/x87/XMM preserved. No Stage36, no synthetic commands, no new data/DR writes, no F7A94/F7AD6/F7B0A/F7C5D/EE306 collision.

## Evidence required next

Windows x86 `Build.log`, `STAGE97_INSTALLED 1/1`, `STAGE97_HEALTH` counters zero, and full same-run Stage97/96/95/94/93 logs. The original one-off seq9461 cannot be directly matched across runs. Use `tools/correlate_stage97.py` to test whether a *new* finite bypass with no later Stage96 writer has a recent exact-value Stage97 `localD4` entry. Guard against stale matches and later contradictory writers. Do not claim improved AI until actual gameplay-specific intervention is separately demonstrated.
