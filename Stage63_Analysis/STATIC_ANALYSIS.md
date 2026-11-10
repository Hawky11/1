# Stage63 — original AALib P+3F4C request-writer provenance (STATIC VERIFIED)

**2026-11-11. Status: original-binary static verification complete. No Stage63 Windows runtime or instrumented DLL.** This stage follows the verified 2026-11-10 22:07 COM-vs-COM Stage62 handoff. Only source/report changes; game behavior, Stage59 r2 DLL, INI and original executable remain unchanged.

## Ground truth / exact original

Original AALib.exe was materialized from the user's existing Library AALib.zip for **read-only local analysis**. The complete 175,556,608-byte EXE SHA-256:

    28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba

PE32/i386, preferred image base 0x00400000. GNU objdump Intel syntax was used for disassembly. The independent Python 3 stdlib [verifier](tools/verify_stage63_static.py) was **executed against the original binary**: PASS ten raw RVA byte signatures, six relative CALL edges and three direct-target E8 xref counts. This script does not patch/execute the EXE. The private AALib.exe and its disassembly are **not published**. All following addresses are RVA, e.g. E5C06 means original preferred VA 004E5C06.

## Direct P+3F4C writes observed in .text

| RVA | Native opcode bytes | Meaning |
|---|---|---|
| E5C06 | 89 81 4C 3F 00 00 | MOV [ECX+3F4C],EAX — request with bit24 flag |
| E5C13 | 89 91 4C 3F 00 00 | MOV [ECX+3F4C],EDX — request without that flag |
| E9E60 | 89 96 4C 3F 00 00 | MOV [ESI+3F4C],EDX — conditional global request |
| E853F | C7 86 4C 3F 00 00 FF FF FF FF | MOV [ESI+3F4C],FFFFFFFF — consumed-request reset |

The shared request setter at RVA E5BF0 accepts ECX=this player, first stack argument [EBP+08]=requested ID and low byte of second stack argument [EBP+0C]=mode. If mode nonzero, instruction E5BFC ORs P+3B7C by 0x01000000 and E5C06 writes the ID from EAX to P+3F4C. Otherwise E5C13 writes ID from EDX without that OR. Both branches RET 8. **This request setter is engine plumbing, not by itself the COM decision/utility scorer.**

The separate gated function at E9E30 tests bit15 of P+3B7C, invokes native F2ED0 with state derived from P+3A2C, tests returned AL, and on success reads an ID from original preferred VA 008254E8 at E9E5A before writing it at E9E60. What the global ID represents remains unproven, so do not call it an AI choice or faulty override.

Native E8480 reads P+3F4C at E8489 and restores sentinel FFFFFFFF at E853F after processing. This is the existing downstream Stage40 consumer, not a new decision writer. The static scan covers **explicit fixed-displacement references and direct encoded calls**; indirect writes, aliases or bulk struct copies have not been exhaustively excluded.

## Original E8 direct CALL xrefs — SHA and original-byte verified

| Call destination | Direct E8 sites in original text section | Interpretation |
|---|---:|---|
| E5BF0 generic request setter | **296** | Many independent potential command producers |
| E9E30 conditional source | **11** | Separate conditional branch family |
| E8480 native consumer | **1** | AD72F calls the existing request consumer |

Verified six original rel32 CALL edges:

- AD72F → E8480: the known native per-frame consumer.
- 123B24 → 1BD6E0: one helper of Stage39-observed callback 123AF0.
- 12EB83 → 131CA0: one jump-table branch of Stage39-observed callback 12EB10.
- 131CC5 → E5BF0: a native helper directly calls the common request setter.
- 131D1C → E5BF0: another native helper directly calls the setter.
- 1DA592 → 1BD6E0: callback 1DA490 invokes the shared helper.

Thus **one verified static route** is callback 12EB10 → helper 131CA0 → request setter E5BF0. It is not proof every conditional branch executes, and the 1BD6E0 helper does not by this single CALL edge prove a P+3F4C write.

The published verifier produces a JSON inventory of all 296 and 11 original CALL site RVAs when run with --json. These are site *counts*, not native runtime call frequencies. The code was independently executed locally against the original SHA-pinned EXE. No Cemu, AH3 game process or Windows x86 DLL execution occurred in Stage63.

## What this adds to Stage62

The original **same-session 22:07** logs establish Stage40→Stage41 263/263, Stage40→Stage42→Stage43→Stage44→Stage46 202/202, and Stage37→Stage39→Stage40→Stage42→Stage44→Stage46 subset 44/44. Stage39 callback RVAs are 12EB10 (31 cases), 123AF0 (6) and 1DA490 (7). Stage40 has 61 extra same-ID 0x49 requests without distinct activation.

The **158 of 202** activations not matched to Stage39 action-output changes (P1=83, P2=75) remain unattributed upstream. Stage63 demonstrates that multiple other original request paths **exist**, but cannot say which callsite produced any individual 158 activations. This is an observer-coverage gap, **not evidence of 158 AI bugs**. Stage61 repeated original X movement writes also remain separate native-engine observations, not choices by COM.

## Stage64 proposal — discriminate action-request source classes, no behavior patch

1. Use the generated full callsite list to cluster the 296 E5BF0 callers and 11 E9E30 callers by original function family, especially existing Stage35/36/37/38/39 event/callback provenance. Distinguish generic engine commands from potential autonomous COM strategy choices; require actual byte-verified call graphs, not naming based only on an ID.
2. If still necessary, design a tightly scoped **passive native writer provenance observer** at E5BF0 and E9E30 or selected callsites, recording same P/O, thread, caller, request argument, mode, QPC and original outcome. Audit x86 ECX/RET8 ABI, volatile/nonvolatile registers, x87/SSE, hook conflicts, Stage38 worker capacity, and Stage59 debug-register ownership before any code build. Do not hook every 296 callsite or introduce a new DR watchpoint blindly.
3. One ordinary COM-vs-COM session only after source tests and successful x86 compilation. Join source-call events to Stage40/41/42 by actor/QPC/ID, distinguishing overwritten requests, duplicates, activations and idle callbacks.
4. Investigate COM utility/candidate scoring only once a concrete pre-request decision-site connection is verified; no modifications to action IDs, P+3F4C, coordinates, combat priority or damage during provenance work.

Existing successful **Stage59 r2** 1,153,024-byte DLL remains the rollback. **Do not rebuild for Stage63.**
