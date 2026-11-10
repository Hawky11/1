# Stage38 — actual guarded callback execution window (source only)

2026-11-10. Base: Stage37 runtime evidence `d9808dbf483524c720f7085c754fcddd74824d11`; 52/52 type1 state commits, six unique positive F4360 chains. Original PE32/i386 AALib.exe: 175556608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Reused the existing verified local copy originally obtained from Library; no binary redistribution.

**Status: source/static ready. Windows x86 compilation and Stage38 runtime NOT performed.**

## Findings and corrected interpretation

P+3F64 has direct readers AC79C/AC7A8: AC780 copies it to P+3A14 and copies [P]+3F64 to P+3A18. This is state propagation, not sufficient evidence of an animation identifier. O+278 is incremented at EC5FA and copied/compared across objects at A1856/A1937 and 12BA7A/12BB67 with byte O+27C guards. This supports a generation/lifecycle interpretation rather than animation numbering. These observations motivated following the **caller of the dispatcher**, rather than continuing through every copied field.

The original EE5E0 native function calls EE480 at EE604. EE480 dispatches EE200 at EE5C6, then returns. If the native queue active byte remains nonzero, EE5E0 takes the following path:

| RVA | Original operation | Implication |
|---|---|---|
| EE621 | reads signed WORD P+3A34 | chooses type-specific initialization |
| EE63B | CALL 12EEE0(P), for type1 | selects a command handler |
| 12EF0A | reads signed WORD P+3A36 | command payload used for table lookup |
| 12EF82 / 12EFC3 | CALL F7E70(P+3ED0, function, argument) | writes callback pointer and argument |
| F7E7C / F7E7E | [P+3ED0]=function, [P+3ED4]=argument | direct storage proof |
| EE72A | EDI=[P+3ED0] | caches the actual callback target |
| EE752 | CALL ED580(P,type,payload) | intervening native initialization; outside Stage38 window |
| EE79A–EE7A7 | P+3B74 bit7 may zero EDI | actual callback may be suppressed |
| EE7AE | CALL 156C00(context,P) | prepares stack context |
| EE7B6 / EE7B8 | TEST EDI,EDI / JZ EE7C3 | zero target skips callback |
| EE7BA–EE7C0 | LEA EDX,[EBP-38]; PUSH EDX; CALL EDI; ADD ESP,4 | actual invocation and return |
| EE7C7 | CALL FC60(context) | subsequent native queue side effects; excluded from window |
| EE7D1 | CALL EE480(P) | subsequent event dispatch; also excluded |

156C00 writes context[0]=P, context[1]=[P+4], context[2]=[P], context[3]=[P+3ED4]. The common type1 callback selected for some commands is preferred VA 52EB10 (RVA 12EB10); other callbacks are selected from native tables. **The actual EDI is recorded**, never guessed from P+3ED0 or a constant. Nonzero target and normal return through the validated corridor prove execution of this native callback, but do not themselves prove rendered animation, attack, collision, damage, or improved AI.

## Instrumentation and exclusions

Only two signature-validated five-byte CALL sites are replaced: EE7AE->156C00 and EE7C7->FC60. **The original conditional branch and indirect CALL EDI are unchanged.** PREPARE wrapper calls original 156C00 once with both original arguments, then observes. FINISH wrapper observes first, then tail-jumps to original FC60 with the original stack/return address. Neither helper is included in the before/after state window.

The same-thread pending stack (16) matches exact player, native EBP, context pointer EBP-38 and context[0]. Only exact known P1/P2 players are observed. The pending record stores actual EDI before callback; EDI after callback is checked as a nonvolatile register. Both hook helpers preserve GPR/EFLAGS and use aligned FXSAVE/FXRSTOR around C++ instrumentation, with FNINIT/CLD only inside the saved region. No gameplay fields are written, no synthetic command/input is supplied.

Snapshot before precedes qpcBefore; qpcAfter precedes snapshot after. Thus QPC measures the interior callback interval including small hook return/entry costs; it is not a pure CPU benchmark. Field reads are sequential, not an atomic cross-thread snapshot. Nested callbacks are paired LIFO; sequence numbers reflect completion/enqueue order, so nested qpcBefore need not increase with sequence. Window records are buffered (4096) and written by the existing worker. Dropped records, pending overflow and pairing failures emit explicit warnings. An unfinished call at process exit cannot produce a completed window record; absence is not proof of no invocation.

## Exact 30-DWORD field schema

O=[P+4]. Object pointer is sampled independently before/after. Do not compare object-relative values as the same object if f0 changes. f1 packs two native WORDs: low16 event type, high16 payload (interpret signed if needed). Other values are raw, including possible floats/pointers; no invented animation labels.

| Index | Address |
|---|---|
| 0 | P+4 (O pointer) |
| 1 | P+3A34 (type/payload WORD pair) |
| 2 | P+3F64 |
| 3..7 | P+3ED0, 3ED4, 3ED8, 3EDC, 3EE0 |
| 8..14 | P+3B70, 3B74, 3B78, 3B7C, 3B80, 3B84, 3B88 |
| 15..17 | O+08, 0C, 10 |
| 18..26 | O+60, 64, 68, 6C, 70, 78, 80, 88, 8C |
| 27..29 | O+184, 278, 27C |

Full masks = 3FFFFFFF; changes only count bits readable on both sides. `guardedCall=1` means recorded pre-EDI was nonzero and native guarded CALL path is inferred from the verified unchanged corridor; pairing, EDI equality and no overflow are prerequisites. Zero-target rows are retained as controls, not counted as executed callbacks. P+3ED0 may change during the callback; it need not equal EDI afterward.

## Verification performed

- `verify_stage38.py <AALib.exe>`: SHA256, PE32/i386, **11 exact signature ranges, 5 direct CALL targets, original FF D7 indirect call PASS**. Runtime validation checks the entire 45-byte corridor and both helper entries before patching and again while threads are suspended; mismatches refuse both patches.
- `test_wrapper_abi.py`: extracts both actual inline assembly blocks, assembles with GNU as i386, executes under Unicorn against native baseline, **6 cases PASS**. Checks one native call, arguments/stack, saved context indices, GPR/EFLAGS including DF, x87/MMX/XMM/MXCSR state. Unicorn's emulated FXRSTOR does not restore the x87 FIP field: four FIP bytes are excluded from comparison and this test does not certify that field. Runtime code uses the actual FXSAVE/FXRSTOR instructions.
- `test_pending_pairing.py`: real C++ snapshot/pairing functions compiled with GCC and stubbed OS calls; nested pairing, zero target, unreadable masks, overflow, mismatch, full-ring drops/sequence gaps and unknown-player filtering PASS. This is not a Windows DLL build.
- Source integration diff retains Stage29 AITrace and old Stage35/36 inl files unchanged; AIQueueTrace changes only include/config/install/flush/no-hooks guard for Stage38. Prior observers remain enabled.
- No MSVC/Windows toolchain exists in this execution environment; **do not label the source compiled**. User x86 Build.bat and natural COM-vs-COM are required.

## Runtime review next

Require BUILD SUCCEEDED, Stage38 r1 marker and STAGE38_INSTALLED 2/2. Parse every row; verify masks, 30 indices, changedMask, sequences, QPC ordering per window, callback equality and warning counters. Retain zero-callback and unchanged-field windows. First correlate Stage37 post-O pointer/278, player/TID and QPC with Stage38 pre f0/f28 and packed type1/payload. Check uniqueness and intervening dispatches before attributing a window to a specific command. Handler may run repeatedly in one generation and may enqueue another event; repeated windows are not new commands. Distinguish callback-local net changes from earlier ED580 writes and later FC60 effects. Use actual callback targets to select the next narrowly justified native output/animation boundary; do not force a command or claim a hit.
