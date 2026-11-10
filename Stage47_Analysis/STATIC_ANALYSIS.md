# Stage47 — static discovery of native motion-field consumer (SOURCE READY)

Date: 2026-11-10. No Windows x86 build or game runtime from Stage47 yet.

Input original: AALib.exe from ChatGPT Library AALib.zip; size 175,556,608, SHA-256
`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
PE32 image base 0x00400000. All offsets below are **RVAs**, not absolute runtime VAs.
Read-only analyzed source; original proprietary EXE is not redistributed.

## Verified consumer and concrete arithmetic

At native **RVA 1E66E0**, ECX is the object pointer, native entry begins
`55 8B EC 83 EC 24 56 8B F1`, and both early return at 1E67F1 and
ordinary return at 1E78D4 are `C2 04 00` (one argument, RET 4).

Within this function the SHA-pinned original code does the following:

| RVA | Native bytes | Observed instruction/effect |
|---|---|---|
| 1E7089 | D9 86 88 00 00 00 | `fld [esi+88]` |
| 1E70B1 | D9 46 7C | `fld [esi+7C]` |
| 1E70DB | D9 5E 7C | `fstp [esi+7C]` |
| 1E70DE | D9 86 90 00 00 00 | `fld [esi+90]` |
| 1E70E8 | D8 86 88 00 00 00 | `fadd [esi+88]` |
| 1E70EE | D9 9E 88 00 00 00 | `fstp [esi+88]` |
| 1E70F4 | D9 86 8C 00 00 00 | `fld [esi+8C]` |
| 1E7114 | D8 86 80 00 00 00 | `fadd [esi+80]` |
| 1E711A | D9 9E 80 00 00 00 | `fstp [esi+80]` |
| 1E7349 | D9 86 94 00 00 00 | `fld [esi+94]` |
| 1E7353 | D8 86 8C 00 00 00 | `fadd [esi+8C]` |
| 1E7359 | D9 9E 8C 00 00 00 | `fstp [esi+8C]` |

The branch around 1E7089..1E711A uses intermediate x87 arithmetic and
time/normalization terms before storing updated fields; **do not** model this
as a simple sum of floats without the branch-dependent factors.
Object O+88/+8C/+90/+94 are concrete read inputs that participate in
updating O+7C/+80/+88/+8C. Stage46 observed native definition-based
initialization of these same fields. This establishes a native producer /
consumer *structural path*. Position in world coordinates, render animation,
hitboxes, damage or AI quality still require separate observation.

## Callers

Six disassembled direct CALLs target RVA 1E66E0; every site is preceded by
`6A 00 8B CE` (PUSH 0, MOV ECX,ESI), and the native callee ends with RET 4.

| Call RVA | CALL bytes | Object-list group | Stage47 hook |
|---|---|---|---|
| **AE70C** | E8 CF 7F 13 00 | E2E02C8 | **yes** |
| AE79F | E8 3C 7F 13 00 | E2E02BC | no |
| AE7BD | E8 1E 7F 13 00 | E2E02B0 | no |
| AE849 | E8 92 7E 13 00 | E2E02C0 conditional | no |
| **AE93E** | E8 9D 7D 13 00 | E2E02C8 | **yes** |
| AEA0E | E8 CD 7C 13 00 | E2E02BC | no |

The first two selected object-list roots are loaded at AE6ED and AE886
(`8B 35 C8 02 2E 0E`). Previous Stage43 runtime P1/P2 activations were
all called from AD75C inside the same E2E02C8 linked-list group.
This motivates the **two selected sites** to limit instrumentation, but
static grouping **does not guarantee** they execute for P1/P2 in every game
state. New Stage47 runtime must confirm nonzero P1/P2 observations and site
coverage; silent or skipped paths are not proof of absent motion.

## Source design, integrity, and scope

[Stage47MotionConsumer.inl](../Stage47_Overlay/src/Stage47MotionConsumer.inl)
implements exactly **two** strict passive CALL redirections to a common
native wrapper, no native instruction-byte splice. Original native is called
**once** with the original ECX and a copied stack DWORD; its RET 4 removes
the copy, wrapper RET 4 removes the untouched original arg. PUSHFD/PUSHAD and
16-byte-aligned FXSAVE/FXRSTOR guard diagnostic C++ callbacks. Both CALLs
reside in the same 4 KiB page; installer suspends other threads, validates
all signatures again, patches both, and restores page protection with one
VirtualProtect region. On failed prerequisites/signature/rel32/protection no
new CALL is redirected; working Stage46 remains unaffected.

Existing Stage38 worker flushes the new 4096-capacity ring. Only P1/P2
objects (matched using the working player+4 lookup) enter the ring; all other
calls are counted. Each record snapshots the same 13 fields as Stage46:
O+60/+6C/+70/+08/+24/+7C/+80/+88/+8C/+90/+94/+150/+154,
plus O+34 definition pointer and definition+30 metadata before/after.
Raw IEEE754 bits are logged, not recalculated in injected code.
New log: `ArcanaAI_Stage47MotionConsumerTrace.log`.
Health includes calls/otherObjects/dropped/overflow/mismatch/badCaller,
and callerRVA distinguishes AE70C from AE93E.

[Stage38CallbackWindow.inl](../Stage47_Overlay/src/Stage38CallbackWindow.inl)
is based byte-for-text on working Stage45 Stage38 adapter except for:
one Stage47 include, conditional InstallStage47 after successful Stage45
and FlushStage47 after FlushStage45. Stage44/45 replacement files must be
the **working Stage46 versions**, not old Stage45 overlay.

Static independently checked locally: exact SHA; six native CALLs and two
list roots; native prologue/returns and field access signatures (PASS).
[verify_stage47.py](tools/verify_stage47.py) preserves these checks and
structural source/wrapper checks for repeatability. This is **not** a
CPU-level wrapper equivalence test. In particular, no Windows x86 compilation,
live hook installation, runtime call distribution, float update outcomes
or performance impact can be asserted from static tests.

## Next runtime verification

Require BUILD SUCCEEDED, STAGE47_MOTION_CONSUMER_BUILD r1,
STAGE47_INSTALLED 2/2 and all previous Stage42/44/45 installed; new log
with STAGE47_CONSUMER_WINDOW, P1/P2, callRVA AE70C/AE93E, argument=00000000,
full pre/post masks 1FFF, metadata extraMask F when definition is valid.
Check QPC monotonic, seq, raw float changedMask, per-side/callsite coverage,
no health drops/overflows/mismatches/badCaller. Join with last preceding
Stage46 definition binding for same P/O, ID and pointer using QPC; multiple
per-frame Stage47 readings per Stage46 binding are expected.
Callsite absent or zero records is not game corruption. Stage47 does not
directly prove world-position/visual-animation or improved AI.
