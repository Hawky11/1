# Stage132 — точная атрибуция upstream `0x27`: STATIC DESIGN (2026-11-13)

**STATUS: DESIGN ONLY; NO hooks/DLL/Windows build/gameplay run.** Complete RU report, portable original-game verifier and strict synthetic event model are in the ChatGPT conversation ZIP `ArcanaAI_Stage132_Upstream27_ExactProvenance_Feasibility_Design.zip`, **23,537 bytes**, SHA256 `b6b23572c01b3be2513a7f547c55ef859e3ea47e09cf6198b3a7b3bc9dd983dd`. ZIP CRC checked, **14** text/Python/JSON files, no EXE/DLL/installer. This GitHub branch publishes the handoff and audit summary; the ZIP source remains in the conversation.

## Evidence and checks

- User's private original AALib.exe size **175,556,608** bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` checked against original bytes. RVA translated using PE32 x86 section table (image base 0x400000).
- Native six prospective site signatures **6/6**; 8/8 selected original E8 call targets; Stage131 underlying audits re-run **162 opcode anchors, 13 direct CALL targets, 20 native branch edges**. **171/171** pairwise non-overlap checks against **26 documented hook/exclusion envelopes** (17 from Stage128, **6 Stage84** hook sites, **3 Stage129** hooks) plus 15 mutual site checks. **27/27** portable design/unit tests PASS. These are NOT live in-memory owner or global patch-overlap checks.

## Critical correction to the prior proposed three-site observer

Three points `F3579`, `F35F9` and `F3B71` cannot uniquely attribute the source of input `0x27`:

1. `F3B71` **loads** mapped byte to EDI but can fail later `0xFF`, `F3790` and selector predicates. It does **not** commit an input candidate. A successful table-derived candidate is written only at original `F3C15 MOV [EBP+0x10],EAX` after `F38D0`.
2. `F3579` and `F35F9` can both execute in one F34D0 invocation; record last executed literal writer. F3930 group loop can reach F3C15 repeatedly; record last committed candidate before F3CBC.
3. Parent frame EBP/TID repeats across invocations. Stage129 child-frame timestamp proximity cannot identify a specific parent call. `inherited` is a result only after complete positive-control entry and full write coverage, otherwise `UNKNOWN`.
4. Current Stage129 sampled only **87 pairs out of 3,000**. Exact attribution needs a **non-sampled bridge for every input-candidate-0x27 call**, with lost/overflow accounting.

## Prospective *five-site* minimum design (NOT INSTALLED)

| RVA | Bytes | Native operation(s) and purpose |
|---|---|---|
| `F34D0` | `55 8B EC 51 53 56` (6) | parent F34D0 entry, positive scope control |
| `F3930` | `55 8B EC 83 EC 18` (6) | parent F3930 entry, positive scope control |
| `F3579` | `C7 45 10 27 00 00 00` (7) | literal candidate=0x27 (group0x24) |
| `F35F9` | `C7 45 10 27 00 00 00` (7) | literal candidate=0x27 (group0x25) |
| `F3C15` | `89 45 10 8B 03` (5) | actual table-derived candidate write plus following MOV EAX,[EBX] |

**Optional sixth:** original `F3B71` 8 bytes `0F B6 BC 91 40 AE C7 04` only if raw table-load byte/address required. It is not a substitute for commit site `F3C15`.

### Original instruction-replay hazard

A 5-byte E9 at F3C15 consumes **two whole instructions**, `89 45 10` (3) and `8B 03` (2). Prospective trampoline would need to execute **both in order exactly once**, preserving GP, EFLAGS, SSE/FPU and any native exception semantics of [EBX] load; jumping after only the 3-byte MOV is incorrect. Parent entry hooks likewise need exact prologue replay.

### Correct scope design

- On every parent entry derive expected parent EBP = originalESP − 4, capture unique SID/TID/parentScope/depth, known object+initial candidate. On literal/table commit, verify same live parent EBP, TID, function and parentScope.
- On existing Stage129 F30B0 entry, CPU EBP **before F30B0 prologue** is still parent EBP. Match it directly to the registered parent frame, plus child return address `F3659` or `F3CC1`, object, candidate and session.
- Observe all candidate0x27 child calls (not just Stage129's sample) as atomic, loss-accounted events. On parent early return or exception, scope must be disposed; an incomplete exit/unwind design cannot produce a valid `inherited` classification.
- Strict synthetic-only Python model enforces separate parent/child scope, last write, reject orphan/stale frames, and deliberately does not classify `F3B71` READ alone as table commit.

## Known hooks vs unresolved safety

Stage84 already patches `F3DE9,F3E02,F3F50,F3F69,F3E0A,F3F71`, none of which overlap these prospective sites. Stage129 installs `F30B0,F325A,F326A`, also separate. Stage128 older owner range catalog is **non-exhaustive**. **Cannot assert all Stage82–98 or unknown external live patches are disjoint** until exact source/live owner preflight. Runtime fail-closed must include game executable SHA, original byte signatures, installed adapter/module owners, suspension + in-flight call detection, mutex, transactional rollback, and safe preservation of original CPU/stack state.

## Stage130 baseline preserved

[Stage130 audited Windows runtime](https://github.com/Hawky11/1/blob/stage130-stage129-producer-runtime-audit/Stage130_Runtime/RUNTIME_ANALYSIS.md) already proved `F30B0 entry=3000, forced F325A=0, returned input F326A=3000` and two sampled input 0x27 near downstream Stage104 special27. **No accepted attack, damage or commit proven**. Preserve tested Stage129 DLL **2,099,200 bytes** and Stage122 rollback **2,089,984 bytes**; Stage36 OFF.

**NEXT Stage133: SOURCE/ABI REVIEW FIRST**, especially original parent exit/unwind lifecycle, complete owner inventory and full candidate27 bridge, before any opt-in code patch, Windows build or runtime test.
