# Stage131 — Original native source of F30B0 incoming 0x27 (2026-11-13)

**Status: STATIC AUDIT VERIFIED, NO GAME HOOK OR NEW WINDOWS BUILD.** The complete 12-file Russian report, original opcode excerpts, 256-row CSV, offline Python verifier/tests, manifest and machine results were delivered as conversation archive `ArcanaAI_Stage131_F30B0_UpstreamCandidate27_StaticAudit.zip` (**21,750 B**, SHA256 **ded1de041852fa49656bc631f4b9ef6152aa8fe061aa1c0b4a07fdd6edaf12c8**, ZIP CRC PASS). The archive contains no original EXE, DLL or .pyc. It is **not stored in this GitHub branch**; this branch contains the audit synopsis and machine results.

## Original evidence and verification

Original private user's `AALib.exe` 175,556,608 B SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, x86 PE32 image base 0x400000, reverified from the user's ChatGPT Library ZIP. Analysis translated RVA to PE section offsets. Native opcode anchors **162/162**, direct relative CALL targets **13/13**, branch edges **20/20**, five full-original .text CALL-census targets / **16** direct E8 sites, 11 original group-table rows, 256 selector-row checks and **496/496** stdlib regression tests PASS. All data is from the original **on-disk** game file; runtime .data may differ.

## Branch A — F34D0 → F3654 → F30B0

- F34D0 code has two actual native writes **F3579** and **F35F9**: `MOV DWORD PTR [EBP+0x10],0x27`, before F3648 loads this value and F3650 pushes it as candidate to F3654 CALL F30B0.
- F3579 requires the original early gate F353C, nonzero player+0x264, CBF50 AL1 at F355B and the mapping compare `BYTE [0x04C7AE51+20*selector]==0x27` at F356A. The second F35F9 analog requires player+0x265, CBF50 AL1 F35DD and compare F35EC. CBF50 AL1 is a ranking predicate, NOT gameplay acceptance.
- Both direct original F34D0 callers **F79B1/F7A8A** pass selector index **0 or 0x22** (mask from player+0x3B70). Both original table rows have column 17=0x27. Both caller paths also pass their preexisting candidate via ESI, so **inherited 0x27** remains another source if the two literal writes do not execute.
- Original F353C can skip this whole block and return without calling F30B0.

## Branch B — F3930 → F3CBC → F30B0

- Original group loop uses code IDs at VA **0x007FE4EC**, per-group column offsets VA **0x007FE47C**, sentinel code `0x40`. 11 groups; code **0x24** and **0x25** (ordinal 1 and 2) both select **column 17**. F3B32 checks enabled-group byte, F3B55 compares via CBF50, F3B71 `MOVZX EDI,BYTE [0x04C7AE40+20*selector+groupColumn]` loads candidate from original data.
- After required additional native filters F3C08 passes selected EDI through F3C0C CALL F38D0 and F3C15 stores EAX into incoming candidate [EBP+0x10]. Original F38D0 transforms only `0x0E↔0x10` and `0x25↔0x26` (under flags), **never 0x27**. F3C82/F3C9F/F3CA8 govern optional F3CBC CALL F30B0. F3CB0 pushes [EBP+0x10] to F30B0; this is not F325A's forced result.
- Across original 256 selector rows exactly **26** indices map to 0x27 in column 17: `0,1,2,3,4,5,6,7,8,9,10,11,12,13,18,23,24,34,35,36,37,38,42,47,49,50`.
- Original direct F3930 callers: F705C (input candidate 0), F7106 (input candidate -1), F7AD1 (preexisting ESI); thus **inherited 0x27** can also reach F3CBC without a fresh table assignment.

## Runtime boundary and next

The same-session Stage130 evidence [audit](https://github.com/Hawky11/1/blob/stage130-stage129-producer-runtime-audit/Stage130_Runtime/RUNTIME_ANALYSIS.md) showed **3000 F30B0 entries**, **0 F325A forced27**, **3000 F326A returnInput**, **0 errors**. Two sampled P2 input27 events (scope43 caller F3659; scope56 caller F3CC1) precede same-session Stage104 MODE0 0x27 by **94/33 QPC ticks**. Stage131 establishes plausible original input-provenance mechanisms but **does not identify which precise upstream instruction produced these two individual runtime candidates**. There is no evidence here for native accepted attacks, executed movement/damage, or a gameplay commit.

**NEXT Stage132:** if per-event attribution matters, review feasibility of narrow positive-controlled observer at original F3579, F35F9 and F3B71, or statically trace inherited ESI first. First prove byte guards, hook ownership/overlap, original instruction replay once and full CPU state preservation; no speculative patches. Preserve Windows-tested Stage129 DLL **2,099,200 B** and Stage122 rollback **2,089,984 B**, Stage36 OFF, no FA480 overrides, F2DE0 rehook, synthetic actions, DR writes or added game-state writes.
