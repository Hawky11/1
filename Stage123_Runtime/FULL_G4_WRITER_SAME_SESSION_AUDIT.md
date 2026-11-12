# Stage123 — real Windows Stage122 positive G4 writer hit and same-object lifecycle

**12 November 2026 · WINDOWS BUILD AND RUNTIME VERIFIED, observational only.**

## Evidence provenance and session integrity

- Fresh user GitHub upload commit [4f1cc5b](https://github.com/Hawky11/1/commit/4f1cc5b47d9c138d5f5bdea82bd843f7112d9e0f), 11 files: Build.log, ArcanaAI.ini, Stage122/115/113/110/107/104 and Bit113 traces. **ArcanaAI.log was NOT among these uploaded files**; the current root ArcanaAI.log still contains the **21:19:30** COMvsCOM marker from an earlier run, so it is **NOT fresh proof of COMvsCOM in this Stage123 session**. Stage122/115/113/110/107/104 all have fresh one-session logs. Stage93/94 files at repo root are also from earlier sessions (prior TIDs) and **must not be joined with this session**.
- Win32 build at **23:47:30, 12 Nov 2026**, MSVC **14.51.36231** x86, all eight C++ compilation units successfully built, **BUILD SUCCEEDED, `build/dinput8.dll` 2,089,984 bytes**. The old nonfatal C4754 warning remains in `AICommandSiteTrace.cpp:786`; no compiler errors.
- Fresh INI: `EnableStage122ObjectWriterTrace=1`, Stage115/113/110/107=1, `EnableStage36ControlToCommandTrace=0`; COM `ForceCpuForBoth=1`. **Stage122 INSTALLED 3/3**: parent 1A0C81, object.state3 19F295, G4/G5 writer 19F2AD, originalReplayOnce=1, noExtraGameplayWrites=1, DRwrites=0. Stage113 installed **4/4**, Stage115 piggyback installed, Stage104/107/110 each installed **2/2**. Declared replay/no-extra-write flags are build/runtime self-reports, not a standalone proof of instruction-level correctness.

## Novel Stage122 positive native writer proof

**One process-session** `sid=01DD72E71C8AE662`, event TID **10720**, object pointer **0x24EB0788**. Every one of **1,770 sampled events** uses the same sid/TID/object. Counts:
- Sampled `kind=1` parent CALL at RVA1A0C81: **1,768 events**. The **51 HEALTH windows report 10,978 all-hit parent calls**. Do not equate sampled parent records with all-hit counts; trailing, unreported health windows cannot be ruled out.
- `kind=2` instruction `19F295` object.state3 store: **1 real hit**, all-hit HEALTH **state3=1**.
- `kind=3` instruction `19F2AD` selected slot7 store: **1 real hit**, all-hit HEALTH **slot7=1, slotG4=1, slotG5=0, slotOther=0**. All **51 HEALTH** windows have `dropped=0`, `mismatch=0`, `readFail=0`. One sampled event at native G4 site has `readMask=3FF`; all samples valid.

Exact contiguous sampled transition:
```text
seq=67 kind=1 qpc=3134233738596 obj=24EB0788 state=0 f2A=01 f2B=00
       g3=0 g4=0 selector=8 map0=1 map1=FFFFFFFF

seq=68 kind=2 qpc=3134234648365 obj=24EB0788 esi=24EB0788 edi=0DE0B640
       state=1 f2A=01 f2B=00 g3=0 g4=0 g5=0
       selector=8 map0=1 map1=FFFFFFFF valid=1 readMask=1FF
       native original at 19F295: MOV DWORD PTR [ESI+10],3

seq=69 kind=3 qpc=3134234648369 obj=24EB0788 esi=24EB0788 edi=0DE0B640
       slot=1 (actual rebased G4) state=3 f2A=01 f2B=00
       g3=0 g4=0 g5=0 selector=8 map0=1 map1=FFFFFFFF
       preTarget=00000000 valid=1 readMask=3FF
       native original at 19F2AD: MOV DWORD PTR [EDI],7
       kind2->kind3 lag=4 raw QPC ticks, same sid/TID/object

seq=70 kind=1 qpc=3134240547165 obj=24EB0788
       state=3 f2A=00 f2B=00 g3=3 g4=7 g5=0
       selector=8 map0=1 map1=FFFFFFFF
       after kind3 +5,898,796 raw QPC ticks
```

Important: **kind=3 is a pre-store observation, so g4=0 at seq69 is correct**. The original store at 19F2AD was reached with EDI pointing at actual loaded-image G4 `0x0DE0B640`, classified slot=1. Later observation at seq70 shows G4=7 and G3=3, and transient object flags have cleared, **exactly the temporal pattern contemplated by Stage120**. This is strong evidence that the native writer in this run is a source of observed G4=7; it is not a global-exclusive or historical Stage116 last-writer proof. The logs capture only one observer instruction and do not exclude other indirect/bulk writes between sampled events.

**Call-scope caveat:** previous sampled `parent` event seq67 precedes state3 by **909,769 QPC ticks**. Because parent records are intentionally sampled, **do not claim a unique same-invocation parent→state3 join** without a nested call token. Positive parent execution and same sid/TID/object are proven; state3→slot7 pair is immediate, unique and strongly paired by +4 ticks.

Before the writer, the sampled object sees selector=8 starting at seq34 and map0=1 at seq57; map1 remains FFFFFFFF in **all 1,770 sampled events**. This run shows the entire *combination* `selector=8,f2A=1,map1=-1` at the writer — a direct runtime confirmation of Stage118/119's required local predicates. It does not identify which *earlier* setter last established selector/map/flags.

## Independent same-run Stage104/107/110/113/115 results

- Stage104: **50** MODE0 events; **25** native special ESI=0x27, **25** CEA20 PRE. **25/25** unique ordered same-TID=10720/P1-P2/player/frame/ESI joins, **+5..16** QPC ticks; P1=8, P2=17.
- Stage107: **25** POST after PRE. **25/25** unique same-run joins, **+5..204** QPC ticks. **52? Do not infer**; this upload has **51 Stage107 HEALTH windows**, all nativeRead=0, error counters 0.
- Stage113: **51 HEALTH** windows, `parentA=16,490`, `gateA=16,490`, `gateAskip=16,490`, `gateApass=0`, `parentB=gateB=0`, dropped/mismatch/readFail=0. **2,996 sampled events**, 1,498 parentA and 1,498 gateA.
- Stage115: **1,498 sampled post-FA480 5-DWORD snapshots** (P1=688, P2=810). **Every snapshot** has `(G1,G2,G3,G4,G5)=(0,0,3,7,0)`, `readMask=1F`, `pred1=pred2=pred3=0`, `predictedAL=0`, `observedAL=00`, `zf=1`, `consistent=1`; **1,498/1,498** match Stage113 sampled gateA kind3 by **identical QPC and TID**, no unmatched. Its 51 reported HEALTH windows contain no partial reads or prediction differences; the final partial window may not be covered by its last HEALTH counter, so do not treat sum(full) as all sample count.
- Stage110 CEBE0 `entry=0, guard=0` in all **51 HEALTH** snapshots; Stage107 original CEC64 nativeRead=0. These results are **compatible** with the observed parent-A gate bypass, but do not classify all 229 possible original FA480 callsites.

## Conclusions and Stage124 recommendation

**Stage123 is complete: real Windows x86 build, 3/3 installed, positive parent/state3/G4 selected writer hits, clean reported health, exact same-object state transition, same-run Stage115 post-global snapshots and Stage113 FA480=0/JE bypass.** The previous *suspected absence* of a G4=7 original writer is refuted for this run; the original code naturally created the value 7 in G4 under selector8, map1=-1, object-flag +2A=1.

However, **FA480 remains zero by design for sampled (G3=3,G4=7)** because its native positive predicates are `(G1 in {8,14} && G2!=0) || (G3==1 && G4==4) || (G3==2 && G5==4)`. The current tuple does not satisfy any. **Do NOT conclude G4=7 is a bug**, nor that AI action is accepted/hit/improved or that all original FA480 callers are inactive.

**Next Stage124:** switch from G4 writer provenance to analyzing exactly which original application / AI action-processing gates require FA480 positive versus independent native action paths; begin static callsite classification and gameplay-side relevance instead of adding more write detours. Only propose further passive hooks if a specific unresolved AI behavior demands them, with no forced memory/gameplay changes.

**Backups:** Windows-tested Stage122 DLL **2,089,984 bytes**, Stage115 **2,082,304 bytes**, Stage113 **2,079,744 bytes**, Stage110 **2,073,088 bytes**; Stage36 OFF.

This checkpoint contains no additional DLL/hook or fabricated gameplay improvement claims.
