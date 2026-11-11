# Stage73 — selector-write provenance: complete direct-caller audit and Boolean source paths

**2026-11-11 — STATIC VERIFIED, RUNTIME WRITER IP UNOBSERVED.** Based on Stage72 (same original SHA-pinned binary) and the frozen user Stage70 r2 capture. No DLL, injection, new runtime capture, modification to AI, EXE, or `main`. Original private `AALib.exe` SHA-256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. The original image was independently rechecked as x86 PE32 with ImageBase `00400000`.

## New exact result 1 — 10 competing register-derived selector=1 sources

Stage71 identified 20 original direct DWORD MOV sites targeting displacement `+3F50`. Stage73 decoded the source Boolean operations for all ten register-derived sites, not merely the fact that they could write 1 or 2:

| Opcode family | Direct writer sites (RVA) | Stored result | Produces **1** when |
|---|---|---|---|
| `XOR reg,reg; TEST AL,AL; SETNE lowreg; INC reg; MOV [P+3F50],reg` | `0F3DE9`, `0F3F50`, `0F466F`, `0F48A9`, `0F7D22` | `1 + (AL != 0)` | **AL=0** |
| `XOR reg,reg; TEST AL,AL; SETE lowreg; INC reg; MOV [P+3F50],reg` | `0F3E02`, `0F3F69`, `0F4688`, `0F48C2`, `0F7D38` | `1 + (AL == 0)` | **AL != 0** |

Each expression is established by its exact original consecutive opcodes, not a name-based inference. `0F3xxx` and `0F46xx/0F48xx` families are gated on `P+16E` or `P+170` and engine helper/flag outcomes; `0F7D22/0F7D38` use descriptor bytes `+1A/+1C`. These are **real competing sources of numeric selector 1**, not synonyms for `155D29`. Stage73 does **not** infer which source executed in the target frame.

The nine literal-zero MOV sites remain `0F3E0A`, `0F3F71`, `0F4690`, `0F48CA`, `0F7D43`, `124CFE`, `155669`, `1BDDDE`, `1BF38E`. Stage71's sole literal-one site `155D29` and its associated state `P+3F54=3/4` are unchanged. This inventory covers the 20 explicit displacement-encoded DWORD stores, **not possible alias/indirect writes**.

## New exact result 2 — only two direct relative CALLs to the literal-one producer

Independently scan **every byte** of the original `.text` for `E8 rel32` whose computed target is `AALib+155B70`. Exactly two candidate sites exist: **`1560B9` and `1566F0`**; both independently match valid original instructions. A byte scan does not rule out indirect calls, alternative entry, or data-as-code ambiguity, but these sites are confirmed instructions.

```asm
1560B9  call 155B70
1560C1  test al,al
1560C3  jne 156110     ; function epilogue
...
1566F0  call 155B70
1566F8  test al,al
1566FA  jne 156728     ; function epilogue
1566FC  ...            ; failure path calls 155B10 and still returns via 156728
...
156769  call 0E5BF0    ; immediate action requester
156770  mov eax,[P+3F50]
156787  mov [O+68],edx ; deferred 25 in frozen Stage70 event
```

**Stronger same-invocation exclusion:** when `1566B0` *enters* its `1566F0→155B70` path, **whether 155B70 succeeds or fails, that branch returns from 1566B0 and does not fall through to 156769/156787**. Therefore the observed Stage38 seq4116 `1566B0` callback that executed Stage65 request 2F and Stage70 write 25 could **not also call `155B70` on that invocation**. This follows from original control flow; it does not depend on assuming AL=1. Stage38 does not record the selector at every prior callback.

If `155D29` produced selector 1 and it remained until Stage70 seq1, its successful write had to occur in an **earlier callback or other invocation**, not the Stage38 seq4116 writer invocation itself. Candidate earlier Stage38 P1 callbacks are seq4092..4114 (which ran `1566B0` repeatedly), but an even earlier unobserved writer or a competing register-derived MOV remains possible. No runtime EIP or first-transition QPC was captured.

## Safe instrumentation decision / Stage74 handoff

Stage59's verified observer reserves **DR0/DR1**, explicitly preserves DR2/DR3 and leaves independent exception status untouched. The source warns of possible other DR2/DR3 owners (`AICommandTrace`). A new hardware `P+3F50` watchpoint cannot be declared safe without a full owner/VEH/DR7 audit. A single `155D29` probe could show positive hits but **a negative result would not exclude the ten register-derived sites**, and callback-boundary snapshots only narrow a window without revealing the writer EIP.

**Next:** decide whether exact *runtime* selector-writer attribution is worth the instrumentation complexity. If yes, first inspect existing DR/VEH ownership across the active Stage70 r2 source and record an abort-on-conflict/rollback plan. Then design read-only attribution covering **all 11 possible numeric-one stores** (or a validated watchpoint with per-thread ownership), logging P/O/TID/QPC, site EIP, selector old/new, state `P+3F54`, and callback context. Do not patch selector, force an action, modify movement, or claim a gameplay improvement. Preserve proven Stage70 r2 DLL **1,892,864 bytes**, Stage67/65/59 rollback builds, raw logs, and default `main`.

Reproduce: `py -3 Stage73_Analysis/tools/verify_stage73.py "D:\\PATH\\AALib.exe"`. Script uses only Python stdlib; it rejects wrong SHA or opcode changes and confirms both direct CALL targets, two early-exit branches, all 20 writer site signatures, and both Boolean families.
