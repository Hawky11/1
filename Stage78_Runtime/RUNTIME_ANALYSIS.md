# Stage78 — runtime: exact literal-one writer hook installed, zero hits; five upstream selector transitions and one callback-local reset

**User upload:** [8fae5d4](https://github.com/Hawky11/1/commit/8fae5d444ab55cb0cc25806499228eec7c7e563d), 2026-11-11. Evidence branch created **at the exact user commit**, preserving all four raw uploaded logs unchanged. **SOURCE:** [Stage78 exact-site observer](https://github.com/Hawky11/1/tree/stage78-literal-one-writer-exact-site-source). **STATUS: Windows x86 BUILD VERIFIED; Stage78 installed; NO Stage78 hit observed; Stage38/76/77 runtime verified; exact producer EIP for observed 0→1/2 assignments remains UNOBSERVED.**

## 1. Build, installation and same-run integrity

- `Build.log` started **13:26:13.39**, Visual Studio 2026 MSVC 14.51.36231 Hostx86/x86, 8 .cpp sources, **BUILD SUCCEEDED**, `dinput8.dll` **1,911,808 bytes** (+6,144 from Stage77 1,905,664). Pre-existing `AICommandSiteTrace.cpp` C4754 warning remains; no new error. DLL bytes were not themselves supplied.
- Instrumentation loaded 13:26:42–13:27:23, game image base `00280000`, observed game TID **19736**, P1=`0DF21F78` O1=`29B22020`, P2=`0DF4CCA0` O2=`29B22C50`. This run is not combined with earlier Stage77 TID10968/O29DF*. Existing startup markers show Stage38 installed 2/2, Stage77 enabled, Stage76 installed 3/3, Stage78 guard `x86=1 xmm=1 stage38=1 stage76=3 stage70=1 stage59Started=0 stage78=0 nativeOK=1`, and `STAGE78_INSTALLED 1/1 RVA=155D29 bytes=10 replay=originalOnce resume=155D33 DRwrites=0`.
- Stage38 **5,064** callback windows, seq **1..5064** exact; Stage77 **5,064** matching pre/post-original-156C00 snapshots, seq **1..5064** exact. All Stage38 masks `FFFFFFFF`, all Stage77 masks `1F/1F`, **zero** selector changes inside original 156C00, all five Stage77 snapshot fields unchanged. **22 STAGE77_HEALTH** lines with zero `overflow/mismatch/readFail`; Stage38 has no dropped/pending overflow warnings.
- Stage76 **9,704** phase rows seq **1..9704**, **4,852** EE480 phase0 and **4,852** E8DA0 phase1, conditional EE5E0 phase2 **0** in this capture; all observed masks `3FF/3FF`. **22 STAGE76_HEALTH** lines have zero `dropped/overflow/mismatch/readFail`.
- `ArcanaAI_Stage78LiteralOneWriterTrace.log` has **three initialization lines only**, no `STAGE78_LITERAL_ONE`, no `STAGE78_HEALTH`. Since Stage78 only emits health when any hit/error counter is nonzero, the absence of a HEALTH row is **expected for an idle observer**, not proof that its callback path executed or the observer could capture a hit. It confirms an installed guarded detour and **zero observed hits in this run**. There is no fresh Stage59, Stage65 or Stage70 log in this upload; do not invent their event/health counts. Runtime safety is bounded to this observed session.

## 2. Selector runtime changes with Stage78 zero-hit

Stage38 `f30=P+3F50` had **one within-callback** selector change and **five between-window** changes when grouped by actor P1/P2 (fixed player pointers) / object / TID and strictly increasing QPC. Stage77 `n0` before/after original 156C00 is unchanged at all five following PRE samples.

| Actor | Stage38 previous POST → next PRE | Selector | Last POST QPC → Stage77-before-156C00 QPC | After-callback Stage76 phase0/phase1 selector |
|---|---|---|---|---|
| P2 | seq **767→769** | **0→1** | `1897550064682→1897550164928` | seq 1533/1534 **0→0** |
| P1 | seq **1034→1036** | **0→2** | `1897565688821→1897565786230` | seq 2067/2068 **0→0** |
| P1 | seq **1948→1950** | **2→1** | `1897617454014→1897617553103` | seq 3895/3896 **2→2** |
| P2 | seq **2017→2019** | **0→2** | `1897620860894→1897620961741` | seq 4033/4034 **0→0** |
| P2 | seq **2658→2660** | **2→1** | `1897655905234→1897656003888` | seq 5315/5316 **2→2** |

In each row, last same-player after-callback Stage76 phase1 still has the previous selector; Stage77 before 156C00 already has the new selector. Therefore these **five** transitions are upstream of original 156C00 and outside sampled EE480/E8DA0. This does **not** identify a specific writer EIP or exclude non-sampled intervening code/other actors/threads.

**All five upstream assignment/transition observations happened without any Stage78 literal-one writer hit.** In particular `0→2` is not even a possible direct effect of literal-one store `155D29`; original Stage71/73 verified **ten competing register-derived MOV writers** that can assign 1 or 2 depending on AL, plus possible indirect/alias stores. A zero-hit single-site run does not disprove Stage77's prior P2 positive-path correlation at a different timestamp/game session.

## 3. New positive selector reset inside original callback 154070 (P2)

**Stage38 seq1643, P2** exact same-run facts:
- Original callback VA `003D4070` = base `00280000` + native RVA **154070**, QPC **1897599629378→1897599629397**, player `0DF4CCA0`, object `29B22C50`, TID19736.
- Selector `f30:1→0`, companion `f31:1→1`, full `FFFFFFFF/FFFFFFFF` masks, changedMask bit30 set. Stage77 matching seq1643 before original 156C00 selector `1→1` with full masks.
- Immediately after, Stage76 **phase0 seq3285**, QPC `1897599629400→1897599629453`, `f0 selector=0→0`, `f1 state=1→1`; **event-record current code `f3=P+3A30=0x22` BEFORE EE480**, then record helper transfers/invalidates it. Phase1 seq3286 stays selector0/state1.
- Next P2 Stage38 callback seq1645 executes `158570` with callback argument `f4=0x22`, selector 0/state1.

The original SHA-verified AALib.exe static path aligns:
```asm
154091 CALL 152C90
154096 TEST AL,AL
154098 JE  1540FB
1540FB PUSH 0
1540FD PUSH 64h
1540FF PUSH EBX            ; descriptor, [EBX] is P
154100 CALL 155640
155667 MOV ECX,[ESI]       ; ECX=P
155669 MOV DWORD [ECX+3F50],0      ; selector reset
155673 MOV EDX,[ESI]       ; EDX=P
155675 MOV DWORD [EDX+3F54],1      ; state=1
155681 PUSH 22h
15568B CALL 0F2DE0        ; event-record write (mode 0, code 22h)
```
Exact native bytes independently checked against private original SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` (not uploaded):
`154091=e8faebffff`, `154100=e83b150000`, `155669=c781503f000000000000`, `155675=c782543f000001000000`, `155681=6a22`, `15568B=e850d7f9ff`.

**Interpretation:** callback-local 1→0 followed by state1 and event22 is strongly consistent with native `154070→155640→155669` actual reset. **No direct EIP watcher at 155669**, so association remains source-runtime correlated rather than instruction-hit verified. State f31 stayed 1 in the Stage38 window because it was already 1 before the helper rewrote it.

## 4. Consequence / Stage79

The Stage78 targeted instrument achieved its specific test: guarded `155D29` hook installed and **no execution hit observed** over this session. It cannot prove that all numeric-one assignments avoid the literal-one path; Stage77 earlier saw an independent callback-local 0→1 in `1560B0`, not reproduced in this particular session.

**Next Stage79 should prioritize source mapping of the five upstream 1/2 transitions:** based on SHA-pinned Stage71/73 ten register-derived writer MOV sites and the original caller/callback paths, identify a minimally invasive, *bounded* subset to instrument. Candidate sets: `0F3DE9/0F3E02`, `0F3F50/0F3F69`, `0F466F/0F4688`, `0F48A9/0F48C2`, `0F7D22/0F7D38`; branch and player/actor provenance must be verified first. A single `155669` probe could confirm the zero writer, but would not resolve upstream 0→1/2 origin. Avoid automatically patching ten sites, taking DR2/DR3 over existing owners, or editing gameplay. Prefer a static source-routing/ABI/callsite audit before a new probe.

Rollback tested Stage78 DLL **1,911,808 bytes** and Stage77 **1,905,664 bytes**, Stage76 **1,901,568 bytes**. Stage78 tested code did not establish smarter AI, movement, damage, hit quality or gameplay changes. All raw logs from [8fae5d4](https://github.com/Hawky11/1/commit/8fae5d444ab55cb0cc25806499228eec7c7e563d) remain unmodified.
