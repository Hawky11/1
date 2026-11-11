# Stage76 r1 — user runtime after-callback phase audit: selector writer remains upstream

**Evidence date:** 2026-11-11. **Frozen user upload:** [d0fddb14bdccadaf0e42be5208cba44fcf544f0d](https://github.com/Hawky11/1/commit/d0fddb14bdccadaf0e42be5208cba44fcf544f0d). This evidence branch starts **at the exact uploaded commit**, retaining the six raw files unmodified. **Source branch:** [stage76-after-callback-phase-snapshots-source](https://github.com/Hawky11/1/tree/stage76-after-callback-phase-snapshots-source). **Status: Windows x86 BUILD VERIFIED; Stage76 3/3 installed and PRE/POST logging RUNTIME VERIFIED; numeric selector writer EIP still NOT observed.**

## 1. Build and run integrity

- User Build.log started **2026-11-11 12:28:01**, MSVC **14.51.36231** Hostx86/x86, Visual Studio Community 18, eight C++ files compiled, **BUILD SUCCEEDED**, DLL **1,901,568 bytes**. This is **+8,192 bytes** versus the tested Stage74 DLL 1,893,376 bytes. Same existing **C4754** warning AICommandSiteTrace.cpp:785, not a build error. Compiled DLL itself was not separately uploaded for independent binary comparison.
- All trace initialization markers **12:28:33–12:28:34**, game events through **12:29:18**. Image base `00280000`, TID **16408**, players **P1=`0DF21F78`**, **P2=`0DF4CCA0`**, objects **P1 O=`29BE5020`**, **P2 O=`29BE5C50`**. All timeline correlations below use this one runtime session, without mixing historical seq IDs.
- Stage76 guard `x86=1 xmm=1 stage38=1 stage70=1 stage59Started=0 stage76=0 nativeOK=1`; `STAGE76_INSTALLED 3/3 EE7D1->EE480 EE7D6->E8DA0 EE7EA->EE5E0`.
- Stage38 `STAGE38_INSTALLED 2/2`; Stage65 `STAGE65_INSTALLED 1/1`; Stage70 `STAGE70_INSTALLED 1/1` with relocated table guard matched; Stage59 `WATCH_ARMED` with DR0/DR1. No new DR owner needed for Stage76.

| Observer | Captured records | Integrity and meaning |
|---|---:|---|
| Stage76 all native call phases | **11,478** | contiguous seq `1..11478`; **all** PRE/POST field masks `3FF`, 26 HEALTH samples with `dropped=overflow=mismatch=readFail=0` |
| Stage76 phase **0** `EE7D1→EE480` | **5,739** | **zero** selector `f0` changes; **13** companion state `f1` changes; **101** callback pointer `f5` changes |
| Stage76 phase **1** `EE7D6→E8DA0` | **5,739** | **no change in any of f0..f9** across all 5,739 valid pairs |
| Stage76 phase **2** `EE7EA→EE5E0` | **0** | this conditional path was **not observed** in this game run; do not claim it was proven inert |
| Stage38 callback windows | **5,705** | P1 **2,772** / P2 **2,933**; contiguous `1..5705`; 32-field f30/f31 present |
| Stage65 immediate request | **332** | contiguous `1..332`, 26 HEALTH samples all relevant errors zero |
| Stage70 deferred original-site writes | **6** | contiguous `1..6`, six valid full mask `07/07` and health zero |
| Stage59 DR0/DR1 X writer | **18,822** | all unique seq `1..18822` present; 29 HEALTH samples error counters all zero; a single ring flush emitted rows out of seq order (see caveat below) |

**Stage76 phase0 is visibly an event-record update/cleanup, not a selector producer in this run.** Across its 5,739 calls: 13 `P+3F54` state resets (`1→0` eleven times and `2→0` twice), **101** callback descriptor changes `P+3ED0 = nonzero→0` (all 101 to zero), **111** changes each in the event record DWORDs f2/f3/f4, and 111 callback-parameter f6 changes. The original `EE480` changes those observed fields but in no capture did it change `P+3F50`. Phase1 `E8DA0` changed none of the ten monitored fields, so it is excluded as the direct within-call selector-changer for this run.

**Stage59 flush-order caveat:** its log has every unique ID `1..18822` with no loss, but after file seq `16428` the writer emitted delayed seq `16267` before returning to increasing order. Specifically seq `16267..16384` arrived after `16428` as a delayed buffer chunk; sort by native `qpcAfter` / sequence when joining unrelated traces. No false dropped-record claim.

## 2. All FIVE real selector changes are after phase1 and before the next Stage38 PRE

All **5** same-player Stage38 previous POST → next PRE changes of selector f30 were checked against matching P/O/TID native Stage76 phase events with strict QPC ranges. Every Stage38 PRE and POST selector has valid `0xFFFFFFFF` masks; every Stage76 f0 has valid bit0 in both `3FF` masks. For each transition **both phase0 and phase1 complete with the old selector**, and next Stage38 PRE already has the **new** selector. There is no phase2 record in the whole run.

| Side | Stage38 window seq previous→next | P+3F50 transition | Stage76 phase0/1 seq after previous window | Last phase1 POST QPC | Next Stage38 PRE QPC |
|---|---|---|---|---|---|
| **P1** | **1859→1861** | **0→2** | **3717/3718**, still 0 | `1862720561917` | `1862720659040` |
| **P2** | **2761→2763** | **0→1** | **5521/5522**, still 0 | `1862769421111` | `1862769523035` |
| **P2** | **2875→2877** | **1→0** | **5749/5750**, still 1 | `1862775131699` | `1862775230210` |
| **P1** | **4370→4372** | **0→1** | **8739/8740**, still 0 | `1862853034590` | `1862853132550` |
| **P1** | **5120→5122** | **1→2** | **10239/10240**, still 1 | `1862892990889` | `1862893090995` |

Those remaining gaps are **97,123 / 101,924 / 98,511 / 97,960 / 100,106 raw QPC ticks** respectively. **QPC frequency is not recorded in the uploaded logs; do not report these ticks as milliseconds.** Importantly the Stage76 phases for the previous player frame complete in the *earlier* portion of the gap, and the transition occurs somewhere **after** the phase1 POST and **before** the next original Stage38 PRE (which samples state only **after native 156C00**).

In all five next Stage38 windows the callback is `158570` with input **callback argument `0x11`**, then the callback descriptor becomes `155160` and the argument becomes FFFFFFFF. This is a correlation with **prepared input**, not evidence that this callback writes the selector. Stage76 phase0/phase1 records before these next windows either still have `P+3ED0=0` or the unrelated prior callback descriptor; next PRE already has `158570` at the moment of sampling.

**Narrower conclusion:** original `EE480` and `E8DA0` are ruled out as the *immediate within-call cause* of these observed selector transitions. Because Stage76 phase2 was never invoked in this run, the gated `EE5E0` path remains **untested**, not ruled out globally. The remaining gap includes native work before/around the next `156C00` callback preparation and potentially other unsampled engine work. Do **not** declare `156C00` the writer before capturing its input state.

## 3. Same-run six native request → deferred pairs remain coherent

All six original `156787` Stage70 stores occur inside one Stage38 `1566B0` callback for the same actor/TID and contain an earlier Stage65 native request `156769` inside that same window. Original `O+68` in Stage38 PRE/POST matches the Stage70 deferred write.

| Actor | Stage38 callback seq | Stage65 request seq / ID | Stage70 write seq / selector / deferred ID |
|---|---:|---|---|
| P1 | **1895** | seq120: **33** | seq1: selector **2**, deferred **26** |
| P2 | **2921** | seq183: **2E** | seq2: selector **0**, deferred **24** |
| P1 | **3489** | seq219: **33** | seq3: selector **2**, deferred **26** |
| P2 | **4355** | seq290: **2E** | seq4: selector **0**, deferred **24** |
| P1 | **4410** | seq294: **2F** | seq5: selector **1**, deferred **25** |
| P1 | **5190** | seq326: **33** | seq6: selector **2**, deferred **26** |

The Stage65 POST → Stage70 PRE intervals in raw QPC ticks are **6 / 6 / 7 / 5 / 8 / 184**, respectively. The last interval is longer; this does not invalidate the containment within the same Stage38 callback. No unverified cross-session source inference is needed.

## 4. Stage77 next: distinguish **before** versus **during** 156C00

**Prefer a minimal read-only extension of the existing Stage38 `Stage38PrepareHook`, NOT another native detour or DR watchpoint.** The Stage38 hook currently executes original `156C00` and only afterward calls `Stage38Prepared` to read f30/f31 at PRE. Stage77 can capture selector, callback pointer and argument from P **immediately before original `156C00`** and compare them with the existing after-156C00 Stage38 PRE. This is the most direct unproven boundary now exposed by Stage76.

If Stage77 PRE-before-156C00 already has the new selector and callback, the decision occurred earlier in unsampled per-frame engine work; if before=old and after=new, narrow the writer to `156C00` or its native callees (still no exact EIP). Observe P/O/TID/QPC and masks, guarantee native `156C00` executes once and preserve original ABI, flags, GPR/x87/SSE, per-thread nesting and zero gameplay writes. Do not add eleven selector-writer hooks or allocate DR0/DR1 (Stage59-owned); do not claim DR2/DR3 are free. Audit current Stage38 assembly trampoline carefully before modifying and retain both tested Stage76 DLL (**1,901,568 bytes**) and Stage74 DLL (**1,893,376 bytes**) plus Stage70 r2 (**1,892,864 bytes**) as rollback. **No new Stage76 test is needed to reproduce its six deferred sites; Stage76 runtime question is answered within capture limitations.**

**Limits:** Stage76 confirms successful Windows build/install and finite same-run observed behavior, **not** that there is zero performance/timing effect, no possible unknown instruction paths, no other aliases, or any COM skill/AI gameplay improvement. Neither user raw logs nor original EXE were modified for this report.
