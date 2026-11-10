# Stage64 — original action request callsite classification (STATIC VERIFIED)

**Date:** 2026-11-11. **Scope:** native request origins upstream of Stage40/42; no new DLL, hook, INI setting, memory write, or COM runtime test. This branch begins at Stage63 verified native P+3F4C writer mapping.

## Input integrity and offline verification

Original, privately stored AALib.exe from the user's Library AALib.zip; SHA-256:

    28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba

PE32 Intel i386, ImageBase 0x00400000, section .text. Original binary and original Intel-syntax objdump disassembly were inspected locally; **neither is uploaded**. All published addresses are RVA, not process VA.

Two independent executable programs were run locally on the exact original:

- [analyze_stage64.py](tools/analyze_stage64.py): **PASS**, original SHA + PE layout, exact original E8 rel32 destinations and decoded x86 CALL instruction boundaries. It rejects any apparent E8 target that is not actually disassembled as an original five-byte CALL. 296/296 E5BF0, 11/11 E9E30, 1/1 E8480.
- [verify_stage64_callback_edges.py](tools/verify_stage64_callback_edges.py): **PASS**, original SHA and all fixed callback-helper edge checks, including 19/11 internal callback branches and 34 direct callers of shared helper 1BD6E0.
- [test_stage64.py](tools/test_stage64.py): **4/4** synthetic conservative-argument tests PASS: immediate ID + flag0; register ID + flag1; conditional-branch ambiguity; missing-flag ambiguity.

The optional JSON inventory of 308 original direct-CALL site RVAs is generated **locally** by the first tool, not committed. No full original disassembly/bytes or AALib.exe are redistributed.

## Exact direct request-setter CALLs

| Original target | Decoded direct E8 sites | Meaning |
|---|---:|---|
| E5BF0 (writes P+3F4C at E5C06/E5C13) | **296** | General native two-stack-arg request setter |
| E9E30 (conditional P+3F4C write E9E60) | **11** | Separate gated global request path |
| E8480 (reads request and resets it at E853F) | **1** | Original request consumer, called from AD72F |

**Neither direct xref count represents execution frequency.** E8 decoding of the exact original establishes callsite possibilities only. Indirect calls, aliases and bulk structure writes are outside this specific scan.

The 296 E5BF0 xrefs by **64 KiB RVA bucket** (not function taxonomy):

| RVA high byte | Count |
|---|---:|
| 00 | 2 |
| 0F | 8 |
| 10 | 60 |
| 11 | 41 |
| 12 | 17 |
| 13 | 33 |
| 15 | 72 |
| 1B | 2 |
| 1C | 26 |
| 1D | 35 |
| **Total** | **296** |

Using only observed Intel disassembly INT3 padding boundaries, these 296 sites occupy **201 distinct padding-defined code islands**. **A code island is not automatically a real C/C++ function or an AI subsystem.** Largest examples: 158570 **18** setter sites, 1C1B00 **9**, 10E5B0 **7**, 154590 **7**, 157720 **6**, 112FB0 **5**. At 158570 the original has a native computed jump table indexed by a callback-derived field, confirming a *dispatch-style* control structure, not its semantic purpose.

## Conservative argument classification (E5BF0 only)

Inspect the **two closest decoded PUSH instructions before each native CALL**, strictly within 48 bytes and without traversing any CALL, JMP, Jcc or RET. First pushed argument is the second native parameter (flag); second pushed argument is ID. If code branches or an argument is inherited from another block, return **unresolved**. The classifier does not guess across branches.

| Flag argument class | Sites |
|---|---:|
| Immediate 0 | **212** |
| Immediate 1 | **12** |
| Dynamic register/operand | **14** |
| Unresolved straight-line context | **58** |

ID operand classes: **120 immediate-constant PUSH**, **118 register PUSH**, **58 unresolved**. The 120 immediate PUSH sites expose **93 distinct static numeric IDs**. These are local encoded argument patterns, NOT 120 known COM decisions.

A secondary, strictly noncausal comparison with the same-run Stage42 log: **35/158** activations without matched Stage39 output have an ID equal to one of those 93 literal constants, whereas **123/158** have no such exact literal match. This merely prioritizes dynamic-source investigation: an ID match does NOT identify its native callsite; absence does not exclude dynamic setters or the gated global writer. The static comparison includes repeated activations, not distinct tactical decisions.

## Stage39 callback-class native paths

One observer covers three callback targets in the verified **22:07** session: Stage39 callback RVA 12EB10 (31 output rows), 123AF0 (6), 1DA490 (7). Original PE analysis separates their source branches.

### Callback 12EB10 — helper-branch fan-out

The exact original callback 12EB10 has **30 decoded direct E8 callsites** within its 12EB10..12EE60 code extent that call two specialized helpers:

- **19** separate conditional/switch branch callsites invoke **131CA0**.
- **11** separate branch callsites invoke **131D00**.
- Handler 131CA0 calls general request setter E5BF0 at **131CC5**, after native 531600 and an AL conditional gate.
- Handler 131D00 calls the same setter E5BF0 at **131D1C**, after its own gating logic.
- Both calls pass dynamic action ID through a register and an immediate flag0. This is validated by the original instructions, not inferred from action-ID matching.

This is a **statically verified request-producing callback path**, but it does not establish which branch each of the 31 recorded callback outputs took or which combat utility score selected it.

### Callback 123AF0 and 1DA490 — shared helper

- **123B24 → 1BD6E0** in callback 123AF0, behind a prior result check against FFFFFFFF.
- **1DA592 → 1BD6E0** in callback 1DA490.
- Shared original helper **1BD6E0 has 34 decoded direct E8 incoming callsites in .text**, including those two callbacks.

Do **not** assume that 1BD6E0 itself directly calls E5BF0 or that each native helper invocation writes P+3F4C; indirect/subsequent dispatch requires separate proof. This is a shared engine callback path, not enough for COM strategic scoring attribution.

## Remaining source gap and risks

Stage62 still proves 263/263 Stage40→41 requests, 202/202 Stage40→42→43→44→46 activations and 44/44 Stage37→39→40→42→44→46 callback-subset chains. **158/202** Stage42 activations lack Stage39 action-output observations (P1 83, P2 75). Stage64 verifies many *potential* native request paths but does **not yet attribute** these 158 events to a specific static caller. Repeated request ID 0x49 without separate activation is not evidence of lost commands. Stage61 X-coordinate updates are downstream mechanics; no claimed COM improvement, damage/hit change or animation proof.

No original EXE patches, no x86 DLL build, no runtime hooks added. The original Stage59 r2 1,153,024-byte dinput8.dll remains the tested rollback.

## Stage65 proposal — targeted passive request-origin trace

1. Prioritize a **single shared request setter entry** E5BF0 (or a narrow selected original caller group) and separately decide whether E9E30 conditional writer needs attribution. The most valuable fields are original return address/caller RVA, P, O=[P+4], TID, first ID argument, flag second argument, QPC and request before/after. Verify native ECX thiscall / RET 8 and save/restore registers/EFLAGS/x87/SSE. Do not patch all 296 CALLs.
2. Preserve natural call behavior; include fail-closed opcode/rel32/prologue checks. Audit Stage38 worker ring capacity/counter overflow, existing hook collisions and Stage59 debug-register ownership **before** attempting a DLL build. Do not add DR watchpoints.
3. On one new homogeneous COM-vs-COM run **only after build succeeds**, correlate actual original writer-call events to Stage40/41 requests and Stage42 promotions by actor/TID/monotonic QPC/ID. Explicitly handle multiple overwritten requests before consumption, duplicate 0x49 requeues and coverage-tail events.
4. Then trace upstream from verified source caller to decision candidate/scoring; do not alter AI logic or P+3F4C/action IDs/coordinates/hit or damage in Stage65 diagnosis.

**Next user action for Stage64: none.** Keep existing DLL. The proposed Stage65 native instrumentation is not built or installed yet.
