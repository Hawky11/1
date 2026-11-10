# Stage55 static analysis and validation

## Basis

Original private AALib.exe SHA256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, 175,556,608 bytes, PE32 x86 ImageBase 00400000. Retrieved from the existing AALib.zip; no original binary is committed.

Source parent: Stage54 r2 `4661af49287e106cb9f9d911f63829dbd5f2fae4`. Runtime evidence: upload `a49661be904b7ef6427b8a885a852089f798801e`; analysis `6a4c1f777ba91b5528f017f2b0d15b510a5f87aa`. Stage54 confirmed 19,734 unchanged windows; 6,500 unique Stage47-to-first-Stage54 pairs contained 504 X differences and zero Y differences. Those facts do not identify a specific writer.

## Static corridor

A2B81 belongs to A2990, called at 18D2D8. Its earlier calls A2994→741F0, A2999→77130 and A299E→1E01E0 precede a conditional branch to A2B81. The last helper is a bare RET. A2990 is later than the motion update corridor; simply adding observers to those three calls would not directly surround the Stage47 parent.

At A1196 the game calls AEA40, then the following consecutive no-stack-argument functions:

| Callsite | Original bytes | Native | Reachable instructions | Plain RET |
|---|---|---|---:|---|
| A119B | E8 40 D5 00 00 | AE6E0 | 137 | AE872 |
| A11A0 | E8 4B BC 00 00 | ACDF0 | 171 | ACFE6 |
| A11A5 | E8 F6 B6 00 00 | AC8A0 | 154 | ACA8C |

ABCB0 follows at A11AA and remains outside the new observers. There are additional calls/branches before the later Stage54 boundary; Stage55 does not cover every writer in that gap.

AE6E0 contains Stage47's AE70C→1E66E0. After its return at AE711, optional ADF00/AE060 helpers and further object-list processing run before AE872. The alternate Stage47 site AE93E is in AE880, **not** AE6E0. Runtime joins must use containment and actual callsite, never assume both Stage47 sites are inside the new first window.

ACDF0 contains `ACF2A: D9 58 7C` and `ACFA3: D9 58 7C` (FSTP DWORD PTR [EAX+7C]), after adding a computed value to the object's current X. These stores do not simultaneously update O+AC. This is compatible with the observed current-X/anchor-X difference, but is only a static candidate. ACDF0 also calls other helpers; even a positive whole-function delta would not by itself prove either individual store caused it. AC8A0 adds a following boundary.

## Instrumentation

Three direct CALL replacements; original no-argument target executes once at each site. TLS pending depth 8, ring 8192, both P1/P2 objects, 13 fields matching Stage53/54, QPC and health counters. The wrapper preserves GPR/EFLAGS plus FXSAVE/FXRSTOR state around each logger. Original calls execute with the original input registers; no synthetic input or game-state write is added. A wrapper adds its own native return address, so it is not a byte-for-byte execution environment; native stack/return behavior was inspected.

Installation requires Stage54/53/52/47 installed. Exact call bytes, decoded targets and full-instruction prologues are verified before patching and again while other threads are suspended. All six chosen byte signatures were checked against the physical PE relocation table: no HIGHLOW overlap. Unlike Stage54's original A1B80 signature, no relocated absolute operand is compared verbatim. Existing Stage54 r2 source is retained unchanged. Adapter delta only adds the include, conditional install and flush.

## Checks executed

- `python Stage55_Analysis/tools/verify_stage55.py --aalib <AALib.exe>`: PASS SHA, architecture, signatures, direct call targets, relocation exclusion and reachable CFG. All branches resolve within each function; one plain RET each; no EBP+8-or-higher stack argument references. Source signatures and exact adapter delta checked.
- `python Stage55_Analysis/tools/test_wrapper_abi.py`: PASS **384** synthetic cases, 128 for each extracted actual assembly body, assembled by GNU as and executed in Unicorn. Compare direct synthetic native against wrapped native: one native call, input/output GPR, EFLAGS, ESP, x87 CW/SW/tag/register payloads, MXCSR, XMM, and logger saved-stack layout. Varies initial registers, flags, x87 depth and rounding modes. x87 instruction/data-pointer metadata is excluded from the emulator equality comparison; it is not validated by this test. This harness does not execute the real game helpers or C++ ring/logger implementation.
- `git diff --check`: PASS.

**Windows MSVC compile/link and actual game runtime are PENDING.** No ready DLL is supplied or claimed. Synthetic checks do not establish absence of timing effects or prove gameplay correctness.

## Next runtime analysis

Require build success, Stage55 3/3 and retained Stage54 3/3, complete masks, unique contiguous seq and zero health errors.

For each same side/object/thread: match Stage47 AE70C windows contained inside the Stage55 A119B window. If more than one Stage47 window is contained, use the last relevant POST and report multiplicity. Compare that POST with the enclosing AE6E0 POST. Separately classify AE93E windows; do not force containment. Then join AE6E0 POST→ACDF0 PRE→ACDF0 POST→AC8A0 PRE/POST→first Stage54 PRE by time and object identity; reject ambiguous/reused or stale targets and unmatched tail.

If X changes inside ACDF0, inspect its internal stores/helper paths next; if inside the AE6E0 remainder, inspect the post-AE711 helpers/list traversal. If already changed before ACDF0 and not explained by a contained Stage47 window, preserve that ambiguity. If all three remain unchanged, investigate later ABCB0 and the remaining corridor. Do not claim rendered animation, collision semantics or improved AI from coordinate logs alone.
