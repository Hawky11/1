# Stage27 — player+0xF8 bit0 lifecycle and dynamic parent-call candidates

**Analysis date:** 2026-11-09. **Method:** offline analysis of a fully recorded and preserved Stage26 run. **No new DLL hook, runtime capture, or MSVC build** was necessary.

## Source of truth

- Stage26 source commit: `a249f452353c340fe8f1dab23375a8a932b079b4`.
- Immutable Stage26 evidence branch: `stage26-object-bit0-write-runtime-evidence`, commit `4b7564ff71664fedd5745e3bcfc4210d74540075`. Original `ArcanaAI_Stage26ObjectBit0WriteTrace.log` blob SHA `eeb87824a4cd10e573ef84ca124f93c9cd9b684c`.
- New Stage27 derived data: `cycles_stage26.csv` contains every matched set/clear pair (70 rows); `stage27_summary.json` preserves aggregate counts. `tools/analyze_bit0_lifecycle.py` regenerates them from the preserved original log.
- **No full AALib.exe** is present in the Git repository. Previously recorded SHA256 of the exact game image: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. An optional offline PE inspection program is provided below; do not infer static CALL targets until that check succeeds.

## Confirmed results

- Stage26 starts with both P1 and P2 field `+0xF8=0`; 140 records (seq1–140, no gaps/dups), 70 sets `0→1`, 70 clears `1→0`. All 70 sets pair with one later same-side clear in the recorded window. No unmatched clear, open set, or repeated set before clear. **This describes monitored transitions, not all possible game states or threads.**
- All 70 setters use Stage25 DLL wrapper post-EIP `0x50B0C267`, preserving game instruction RVA `0xF58C9: OR DWORD PTR [EAX+0xF8],1`. Of these, target P1 41 and P2 29. In all 70 setter snapshots, EAX equals the target fighter and ESI equals the opposing fighter.
- Source-context register EBX takes many values: `0x2D` in 14 sets, `0x20` in 13, `0x1A` in 5, `0x28` in 5, `0x1E` in **2**, etc. **Do not assume EBX is unconditionally a gameplay payload without tracing its register provenance;** its correlation with type1/payload1E is observed in exactly two captures.

## Clearing instructions — exact bytes and state

| Original writer RVA | Post-write EIP RVA | Observed clears | P1 | P2 | Instruction effect |
| --- | --- | ---: | ---: | ---: | --- |
| 0x97873 | 0x9787A | 54 | 28 | 26 | `83 A7 C8 00 00 00 F2`: AND DWORD [EDI+0xC8], sign-extended imm8 0xF2 (=0xFFFFFFF2); clears bits0,2,3 |
| 0x975A6 | 0x975AC | 7 | 7 | 0 | `89 86 C8 00 00 00`: MOV DWORD [ESI+0xC8],EAX; preceding `AND EAX,0xFFFFFFFE` |
| 0x95F55 | 0x95F5B | 8 | 5 | 3 | `89 87 C8 00 00 00`: MOV DWORD [EDI+0xC8],EAX; observed EAX=0 |
| 0x97031 | 0x97037 | 1 | 1 | 0 | `89 9E C8 00 00 00`: MOV DWORD [ESI+0xC8],EBX; observed EBX=0 |

For each record at these paths EDI/ESI is `player+0x30`. The field is therefore `[player+0x30+0xC8]` = `player+0xF8`. **The three MOV paths may also overwrite other bits;** only the observed bit0 transition is asserted. The solitary 0x97031 record appears inside a wider sequence resetting `[ESI+0xC4]`, `[ESI+0xC8]`, `[ESI+0xCC]` with zero; a wider reset is plausible, gameplay meaning unproved.

## The primary clearer starts at RVA 0x97870

Stage26 captured the following bytes just before the post-write EIP `AALib+0x9787A`:

```asm
; RVA 0x97870 — apparent helper entry, preceded by 0xCC padding in captured window
97870  57                         push edi
97871  8B F8                      mov edi, eax
97873  83 A7 C8 00 00 00 F2       and dword ptr [edi+0xC8], 0xFFFFFFF2
9787A                             ; Stage26 hardware watchpoint traps here
```

Because `PUSH EDI` is the only visible stack modification before the trapped AND, the saved stack slot 0 contains old EDI and **stack slot 1 is a credible caller return address** for this helper. This particular inference does *not* extend automatically to other three writers.

For 54 calls to the primary clearer:

| Return RVA inferred from stack[1] | Count | P1 | P2 | Deeper context |
| --- | ---: | ---: | ---: | --- |
| `0x979D1` | 34 | 16 | 18 | All 34 records also show `0x9760E` at stack[4] |
| `0x9760E` | 20 | 12 | 8 | A different stack shape contains caller-related player arguments and sometimes `0x394335` |

**Working dynamic graph (not a verified static call graph):**

```text
Higher-level caller / region near RVA 0x97609
    |
    +-- 20 observed returns directly at RVA 0x9760E --+
    |                                                   |
    +-- 34 observed nested paths near RVA 0x979CC ------+--> helper at RVA 0x97870
            (deeper stack retains RVA 0x9760E)              |
                                                            +--> bit0 cleared
```

The `0x97609` and `0x979CC` addresses are **candidate five-byte CALL instruction starts**, calculated as return minus5, not yet byte-validated. The script `--binary` checks whether each is `E8 rel32` targeting `0x97870` in the exact pinned executable. If a candidate instead branches indirectly or is not E8, the tool reports it rather than treating this graph as proven.

## Pair lifetimes (QPC ticks)

- Primary clearer 0x9787A: 54 pairs; median **97,068 ticks**, min317, max9,214,547; dynamic return 0x979D1: 34 records, median97,068 ticks; direct return 0x9760E: 20 records, median97,278 ticks.
- 0x975AC: 7 pairs; median99,990 ticks, all P1.
- 0x95F5B: 8 pairs; median3,704,260 ticks, P1 five/P2 three.
- 0x97037: 1 P1 pair; 20,731,060 ticks.
- These figures are raw QPC ticks, not guaranteed milliseconds without verifying the session's QPC frequency. The very different lifetimes suggest distinct cleanup contexts but **do not prove action/animation semantics**.

## Same-event correlation with payload0x1E

The only two observed setter records whose EBX=`0x1E` are Stage26 seq1 and seq87, both P1 (source P2). Their paired clears are seq2 and seq88, **both at post-EIP 0x9787A and stack return 0x979D1**.

| Set→clear | Stage26 QPC difference | Corresponding Stage25 P2→P1 `0x1E` |
| --- | ---: | --- |
| 1→2 | 97,457 | Stage25 OR record seq1, before0 after1 |
| 87→88 | 97,161 | Stage25 OR record seq2, before0 after1 |

This is compatible with bit0 being temporary state cleared by a subsequent helper call. We **cannot conclude** that it is the suppression condition for payload0x1E or that the clearing is caused by the dispatcher; it can be executed as part of normal per-frame maintenance.

## Remaining unknowns and next task

1. Validate the exact `AALib.exe` SHA256 and disassemble around candidate caller sites `0x97609`, `0x979CC`, primary helper `0x97870`, and other three writer functions. Export short byte windows rather than the entire executable where practical.
2. Determine preceding branch conditions and caller chains. Classify whether 0x975A6, 0x95F55, 0x97031 are lifecycle reset, transition, or another controller path. The Stage26 register and QPC context alone cannot establish that.
3. Only after static control-flow review should a new passive Stage28 hook be considered, at a uniquely verified callsite and with specific remaining questions.
4. Existing unrelated nonfatal C4456 in `AITrace.cpp` (Stage26) may be renamed in a future source-only revision; **do not alter the immutable tested Stage26 source branch just for this.**

No new game instrumentation or build is required for this Stage27 offline analysis. The user can run the local optional PE validator against the exact installed `AALib.exe` and share the exported static report.
