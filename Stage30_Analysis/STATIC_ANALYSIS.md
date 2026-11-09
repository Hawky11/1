# Stage30 — native alternate processing after F6A68

Date: 2026-11-09.
Status: offline static analysis complete; no DLL changes, build or new game run.

## Result

The bit17=1 path is an explicit native processing path with state writes and a possible **different queue operation**. It is not an empty return. Absence of type1/payload0x1E at F6AD7 does not establish that an action was lost. Conversely, this analysis cannot prove the resulting gameplay behavior is correct.

New downstream chain, conditional on the guards below:

`F6A8C -> 130A10 -> 12F3B0 -> 156DA0 -> F2DE0(type=0, payload in {0,1,4,5})`

The last CALL is at `156E47`, return RVA `156E4C`. This is distinct from type1/payload0x1E at F6AD7. Stage29 already contains aggregate queue records from 156E4C, but these lack the per-event QPC/parent context required to attribute them to the four bit17 episodes.

## Provenance / reproduction

- Original private `/AALib.zip`, member AALib.exe: 175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
- PE32 x86; preferred ImageBase 0x400000. Instruction labels and relative call targets in evidence are RVAs. Absolute memory operands/pointers emitted by objdump remain **preferred VAs**, requiring subtraction of ImageBase for RVA conversion.
- Runtime comparison: immutable Stage29 `d5f2cd0266759aa95f8617a3eb3e3770040d73d3`, particularly root `ArcanaAI_Stage29Bit17Transitions.log`, root `ArcanaAI_QueueTrace.log`, and `Stage29_Runtime/payload_1E_correlation.csv`.
- Read-only reproduction: `python3 Stage30_Analysis/tools/analyze_stage30.py --binary /path/to/AALib.exe --output /tmp/stage30-evidence`. Requires GNU objdump. SHA mismatch fails before reading RVAs. PE-section mapping is used, not a universal RVA/file-offset shortcut.
- Executed successfully against the original: 16 exact E8 call targets, 9 instruction signatures, 5 numeric constants, 12 bounded disassembly windows. These are static checks, not MSVC/runtime validation.
- No original EXE/ZIP/full executable disassembly is published. Evidence contains bounded relevant windows only.

## 1. Producer call/return contract

F6A73 JE selects the ordinary queue path when bit17=0. With bit17=1:

1. F6A79 calls E8560 with ECX=player and stack arg0; callee `ret 4`.
2. F6A83 calls 156C00 with `(snapshot, player)`.
3. F6A8C calls 130A10 with `(snapshot)`.
4. F6A91 cleans the three outstanding DWORD stack args; F6A96 explicitly writes AL=1 and returns.

Neither the AL from E8560 nor a register left by 130A10 gates this return. **AL=1 at the producer is not proof of successful downstream gameplay action.** There is no supported boolean success contract for 130A10 at this caller. The bit17=0 path also calls E8560 at F6ACC before F6AD7; E8560 is not exclusive to the alternate route.

156C00 writes exactly four DWORDs: `{player, [player+4], [player], [player+0x3ED4]}`. These are copied references/state, not a deep object snapshot. It neither inserts an event nor returns a checked success flag.

## 2. E8560: conditional direction-state adjustment

Let P=player and O=[P+4]. The routine compares the float at O+0x7C with the corresponding float in the object reached through [P]. It may clear/set O+0x24 bit3. If that bit changes and 95B00(P,2) is true, it negates floats O+0x88 and O+0x90. AL reports whether bit3 changed. Early guards return AL0 and skip those writes.

Important guards include P+0x3F7C; P+0x3978==0x1B with P+0x3F34<5; arg0 with P+0x3B70 bit2 set; tuple/state-dependent tests; and 1DD8E0 / P+0x3B90 bit2 before the float comparison. The exact instructions are in `evidence/orientation.asm`.

The direction/facing interpretation is an inference from comparisons, bit3 and sign inversions. The concrete fields/operations are confirmed; names such as X position or velocity are not symbol-backed.

## 3. 130A10: common effects and branch priorities

At entry it reads P+0x3B84 and computes two local booleans:

- A = bit17 OR bit19 OR bit20 OR bit21 (mask 0x003A0000).
- B = bit20 OR bit21 (mask 0x00300000).

Before any return in this function's normal control flow it clears P+0x4238 and P+0x423C, makes helper calls, calls E8560 again, invokes 156C30, clears O+0x1C bits3/4, writes the current [P+4]+0x1E4=0 and original O+0x290=-1, and ORs P+0x3B80 with 0x10000. These are not guaranteed final values: nested helpers can subsequently modify fields.

Calls through E6960 obtain a selected state block (usually P+0x3978, but with a redirect for a special 0x1B case). F5350 sets block+0x20=10000, block+0x24=100 and WORD block+0x34=0; F53A0 then sets BYTE block+0x34=1. Do not assume that byte remains 1: the later 12F3B0 continuation can write 0.

At 130C52 it **re-reads** the flags. Priority:

| Condition | E6C60 argument | Continuation |
|---|---:|---|
| current bit20=1 | -0.5 | returns at 130C7C |
| otherwise current bit21=1 | -0.25 | returns at 130C9B |
| otherwise entry A=1 | -2.0 | conditional 12F3B0 and later case tables |
| otherwise entry A=0 | -2.0 if current bit18, else -1.0 | other callback/effect path |

With entry flags exactly 0x20000, A=1/B=0. Reaching the -2.0 continuation additionally assumes nested calls do not set bits20/21 before the re-read. This is not established by an entry snapshot alone.

E6C60 unconditionally ORs P+0x3B84 bit16 at E6C66. Its later F5390 writes a selected block+0x24 using `int(argument*100)` (with character/state-specific argument and object adjustments). For an ordinary unadjusted -2.0 call this is -200; do not assign a game mechanic name or assume all cases use -200. The verified constants are in verification.json.

## 4. Concrete alternate output via 12F3B0 and 156DA0

130CFB calls 12F3B0 only if current bit19=0. Within that helper:

- If O+0x20 bit1=1, it takes a separate callback setup and returns.
- Otherwise current bit17=1 jumps at 12F40C directly to 12FAE9. This is a **forward processing branch**, not an early return.
- At 12FB04, 991C0 writes slot index0x38 in P+0x3A3C using a table value scaled by 120. Exact destination is P+0x3B20.
- A special P+0x3978==0x1D and bit17 path installs callback RVA156F20 through F7E70, sets P+0x3EDC=0 and later writes selected block+0x34=1.
- Otherwise 12FB53 calls 156DA0. This compares object coordinates/direction and P+0x3B70 bit5 to select payload 0/1/4/5. At 156E47 it calls **the known F2DE0 queue writer** on P+0x3A2C with type0, then ORs P+0x3B70 bit17. That last bit is in a different DWORD from producer P+0x3B84 bit17.
- The normal tail also calls 1E42B0, zeros O+0x150 and O+0x154, sets selected block+0x34=0, and writes O+0x290 from 1E4870. These are observable continuation effects, not a semantic proof of an attack or animation.

Thus “bypasses the queue” must be read narrowly as **bypasses the type1/0x1E insertion at F6AD7**. A transitive type0 insertion remains possible. No claim is made that all nested callees or all case-table behaviors have been semantically identified.

## 5. Existing runtime evidence: what it does and does not show

All four Stage29 clear records (seq2/4/6/8, alternating P2/P1) show `old=0x00030000 new=0x00010000`. Setter records show `0 -> 0x20000`. Hence the retained snapshots show bit16 present by the time bit17 is cleared, matching the E6C60 OR described above.

Stage29 filters out writes that leave bit17 unchanged, so it did not retain the actual bit16 write site/QPC. The source cannot be uniquely attributed to E6C66 from this log. Shadow old values are best-effort and uninstrumented threads remain a limitation.

The same Stage29 QueueTrace contains ACCEPTED_BY_QUEUE aggregate records with callerRVA156E4C, type0 and payloads0/1/4 (no need to infer these from type1 traces). They occur throughout the session. `tid(first)`, second-resolution log time and cumulative `totalHits` are not individual-event QPC records. **Do not pair these rows to the four bit17 cycles or sum cumulative totals as new events.** Other callers can reach 156DA0.

The bit17 clear site ED19F is gated by F2ED0 on the current tuple `(1,0x1E)` at P+0x3A2C; AL0 allows mask0xFFF9FFFF, clearing bits17/18 and preserving bit16. This tests the current tuple, not completion of every alternate action.

## Next bounded task — Stage31 observation, not a gameplay patch

Before changing behavior, capture the alternative routine's input/output in the same episode. A passive callsite wrapper at F6A8C -> 130A10 can log entry/post QPC, side/player/thread, snapshot pointers, full flags, P+0x3978/+0x3F20, P+0x4238/+0x423C, current tuple P+0x3A2C, slot0x38, O+0x20/+0x24/+0x290/+0x150/+0x154 and the resolved state-block pointer/fields. Preserve original register/EFLAGS/FPU semantics and all caller-owned arguments; never treat the leftover EAX as a boolean result.

For exact nested type0 attribution, add bounded per-event logging for returnRVA156E4C in the existing queue observer, with QPC and same-thread scoped alternate-call context. A pre/post snapshot alone cannot prove a transient insertion that is overwritten later. Validate the existing queue logger filter/aggregation and source first. Do not force bit17, duplicate the queue event, or change type0 to type1.

Stage31 is **not implemented or compiled here**. Stage30 gives a concrete target and expectations; no user rebuild is needed for this static checkpoint.
