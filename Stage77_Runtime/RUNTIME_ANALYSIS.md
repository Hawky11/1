# Stage77 — runtime verifies selector boundary and first positive selector-changing native callback

**Evidence 2026-11-11, commit [2ee8534](https://github.com/Hawky11/1/commit/2ee8534ba314221da8b980f7ebc8cc3458baf22a)**. Evidence branch starts at exactly that user upload: original five raw logs unchanged. Source adapter: [Stage77 source](https://github.com/Hawky11/1/tree/stage77-pre156c00-selector-boundary-source). **BUILD/INSTRUMENTATION RUNTIME VERIFIED; DIRECT SELECTOR-WRITER EIP STILL NOT CAPTURED.**

## 1. Build, identity and capture quality

- User Build.log 2026-11-11 **12:59:17**, MSVC x86 14.51.36231, eight translation units, **BUILD SUCCEEDED**, output `dinput8.dll` **1,905,664 bytes** (+4,096 from Stage76 1,901,568). Old C4754 warning persists, not a new error. The resulting DLL binary was not separately provided; build provenance is log-based.
- Single run 12:59:43–13:00:24, game base `00280000`, game thread TID **10968**, P1 `0DF21F78` / O1 `29DF4020`, P2 `0DF4CCA0` / O2 `29DF4C50`. Earlier Stage76 run (TID 16408, O=29BE...) is **not** joined to this capture.
- Stage38 **2/2**, Stage77 **ENABLED**, Stage76 **3/3**, Stage65 **1/1**, Stage70 **1/1**; no new DR ownership/engine writes from Stage77.
- **4,805** Stage38 callback-window rows, seq **1..4805**, 4,805 corresponding Stage77 before-native rows (exact pairing by seq), all mask `beforeMask=afterMask=1F`, all Stage38 `preMask=postMask=FFFFFFFF`. Stage77 native before→after values agree exactly with Stage38 PRE after-native values.
- **9,690** Stage76 phase records, seq **1..9690**, 4,845 each at EE480/E8DA0, **zero EE5E0** calls. All relevant PRE/POST masks 3FF; 21 HEALTH lines with zero error counters. 21 Stage77 HEALTH lines also have `overflow=mismatch=readFail=0`; all Stage38 health, Stage65 health and Stage70 health available in the files are clean.
- Stage65 **323** requests; Stage70 **2** original deferred writes, both P1 selector1, `O+68:FFFFFFFF→25`. Both in Stage38 callback `1566B0`: Stage38 **seq1208** with Stage65 **seq74** requester 2F and Stage70 seq1; Stage38 **seq4458** with Stage65 **seq312** requester 2F and Stage70 seq2. Exact same-run P/O/TID and QPC containment.

## 2. Before-native 156C00 result: no direct changes

**0/4,805** Stage77 observations changed n0 selector `P+3F50` inside original `156C00` (indeed, all five sampled n0..n4 fields were identical before/after, and every `changedMask=00`). This excludes native 156C00 as the *within-call source* of observed changes in this run only; it is not a global proof or direct EIP watch.

One strict same-player, same-object, same-TID between-window selector transition:
- P1 **Stage38 seq1168 POST selector 0**, QPC `1881397824662`.
- Previous frame Stage76 phase0/phase1 seq **2335/2336** still selector **0**.
- Next P1 Stage77 **seq1170** before 156C00 QPC **1881397925470**, n0 `1→1`; Stage38 PRE QPC `1881397925476` selector **1**. Stage77 reports zero changedMask.
- Therefore this **P1 0→1** occurred **after phase1** and **before native 156C00**. The gap includes unobserved engine activity; do not attribute to 156C00, EE480, E8DA0, or the following `158570` callback simply because it consumes selector 1. No exact writer EIP in this gap.

## 3. NEW positive callback-local selector assignment, Stage38 seq2910 (P2)

Separate from P1 inter-frame event, the Stage38 callback **seq2910 P2** is a real original callback-window selector writer observation:

| Evidence | Actual result |
|---|---|
| Actor/object/thread | P2 `P=0DF4CCA0`, `O=29DF4C50`, TID **10968** |
| Stage77 native-before and native-after `156C00` | selector `n0=0→0`, companion `n1=1→1`, masks `1F/1F` |
| Stage38 callback RVA | game VA `003D60B0` = gameBase `00280000` + **1560B0** |
| Stage38 callback QPC | **1881491555808→1881491555821** |
| Stage38 `f30=P+3F50` | **0→1** (fully valid masks; changedMask bit30) |
| Stage38 `f31=P+3F54` | **1→3** (changedMask bit31) |
| Stage76 immediately after callback, phase0 seq5819 | QPC **1881491555824→1881491555879**, selector `1→1`, state `3→3`, `f3=P+3A30` pre **00000021** |
| Stage76 phase1 seq5820 | selector1/state3 stable |
| Next P2 Stage38 seq2912 | `callback=158570`, argument **21**, selector1/state3 remain |

This is the **first captured positive selector assignment inside an identified original native callback** in these selector investigations, distinct from the lone P1 inter-frame assignment.

**Static support (previously independently verified on original AALib SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba):**
- Original `1560B0` calls `155B70` at `1560B9`.
- Original `155B70` has branch writing `P+3F54=3` at `155D74`, then joins literal `P+3F50=1` writer at `155D29` via `155D7E→155D27`, with `push 21h` and native `0F2DE0` event-record producer.
- The observed **state 1→3**, **selector 0→1**, followed by event-code **21** in the same actor's next Stage76 PRE match that original path unusually closely. This is strong **callback-window runtime + independent static native dataflow** linkage, not direct hardware/EIP observation of `155D29`; other nested/indirect writes are not ruled out without a focused hook.

**Critical distinction:** Stage77 observed **zero** changes *inside 156C00*; the **one** Stage38 callback-local change is *after* 156C00, during `EE7BE CALL EDI` executing native `1560B0`. These do not conflict.

## 4. Limits and suggested Stage78

The existing Stage77 assembly wrapper, native signature and source checks were already completed before this user build. This runtime proves installation and finite sample health, not no timing/performance effect or universal game safety. No user gameplay changes, AI improvements, hit/damage or skill use demonstrated.

**Stage78 candidate:** observational static audit of the exact original `1560B0→155B70→155D74/155D29` success path and its return/branch context. If the user wants direct proof of EIP, add *one* guarded, passive software-site observer at the known literal-one setter (or bounded before/after CALL to `155B70`), rather than 20 generic writers or unsafe DR2/DR3, with full original-once, register/flags/x87/SSE, nesting, safe relocation, rollback and single-session health checks. A negative result from that one site would NOT exclude the ten register-derived writer sites. The separate P1 inter-frame source remains unproven. **Do not modify the actual selector, event, action, movement or COM behavior.**

Keep known-good Stage77 **1,905,664-byte** DLL; rollback Stage76 **1,901,568-byte** and Stage74 **1,893,376-byte** DLL. No Stage77 re-run required merely to confirm these observations.