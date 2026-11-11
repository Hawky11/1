# Stage74 — existing native callback-window selector snapshots: RUNTIME VERIFIED

**User upload:** [211f0882e6b8e220877e4c15b072e5a7b8a7b067](https://github.com/Hawky11/1/commit/211f0882e6b8e220877e4c15b072e5a7b8a7b067), 2026-11-11. This evidence branch was created **at that exact commit**, retaining the five user-uploaded files unchanged. **Source:** [stage74-callback-selector-boundary-snapshot](https://github.com/Hawky11/1/tree/stage74-callback-selector-boundary-snapshot), a four-edit replacement of the tested Stage70 r2 Stage38 adapter. **Status: Windows x86 BUILD VERIFIED; SAME-RUN INSTRUMENTED SELECTOR SNAPSHOTS RUNTIME VERIFIED; ACTUAL SELECTOR WRITER EIP UNKNOWN.**

## 1. Build, installation, provenance

The user-supplied `Build.log` began **2026-11-11 11:36:55**, MSVC `14.51.36231` Hostx86/x86, Visual Studio Community 18; eight .cpp sources compiled and **BUILD SUCCEEDED**, producing `dinput8.dll` **1,893,376 bytes** (Stage70 r2 prior baseline 1,892,864 bytes, difference +512 bytes). The existing compiler warning **C4754** in `AICommandSiteTrace.cpp:785` persists; no build errors. Build provenance is from the supplied transcript; the actual compiled DLL bytes were not independently uploaded/rebuilt by the analyst.

All four observer logs begin in the **same 11:38:06–11:38:07** initialization and continue through **11:38:42**, with observed game TID **19292**, players P1=`0DF21F78`, P2=`0DF4CCA0`, objects O1=`2A10F020`, O2=`2A10FC50`, runtime game image base `00280000`. The Stage70 relocated table guard passes. No cross-session files were joined.

| Observer | Rows | Observed integrity |
|---|---:|---|
| Stage38 callback snapshots | **3,708** (P1 **1,802**, P2 **1,906**) | exact contiguous `seq=1..3708`; `STAGE38_INSTALLED 2/2`; **3,708/3,708** with valid f30/f31 PRE/POST mask bits 30 & 31 |
| Stage65 native immediate request | **313** | exact contiguous `seq=1..313`; `STAGE65_INSTALLED 1/1`, 17 health samples zero `dropped/overflow/mismatch/readFail` |
| Stage70 native deferred writer | **3** | exact contiguous `seq=1..3`; `STAGE70_INSTALLED 1/1`; all `preMask=postMask=07` and health counters zero |
| Stage59 X writer DR0/DR1 | **11,957** | all **11,957 sequence IDs 1..11957 present exactly once** (file-order flush anomaly: seq 8179..8192 printed after seq 8352, not a gap); `STAGE59_OBJECTS_READY`, `STAGE59_WATCH_ARMED`, 63 thread-install records, 18 health samples zero `dropped/readFail/ownershipConflicts/installFailures/teardownFailures/objectDrift` |

**Stage59 file-order caveat:** the Stage59 log is not strictly sorted by sequence or QPC at one flush boundary: the 14 rows with seq **8179..8192** were emitted **after** seq **8352**. All IDs 1..11957 are present exactly once, all health counters are zero; any timeline join must sort by QPC/seq rather than trust physical line order. This does not affect the Stage38 callback-window correlation reported below.\n\n**Minor nonfunctional provenance bug:** the Stage38 initialization banner still says "**30 fields**" / `STAGE38_CALLBACK_WINDOW_BUILD=2026-11-10-r1`, inherited from the old adapter. Every one of the **3,708 actual window records has f30 and f31 populated**, with `preMask=postMask=FFFFFFFF`, so the banner is stale, *not* evidence the new fields were absent. Correct the banner if a later source edit is made; no rebuild solely for cosmetic text.

## 2. Main discovery — five inter-frame selector changes, zero within sampled callback windows

For all 3,708 records, the `f30=P+3F50` before/after values are readable and **identical within the sampled callback invocation**. `changedMask` never marks bit 30. Grouping adjacent snapshots by identical **side/player/object/TID** and strict increasing QPC instead reveals **five** selector transitions **between** previous POST and next PRE.

| Actor | Stage38 previous → next | Selector previous POST → next PRE | Last POST QPC → next PRE QPC | New callback/parameter → installed callback |
|---|---|---|---|---|
| P2 | **278 → 280** | **0 → 1** | `1832393670534 → 1832393770261` | `158570 / 11h → 155160` |
| P2 | **651 → 653** | **1 → 0** | `1832413895210 → 1832413998535` | `158570 / 11h → 155160` |
| P1 | **2528 → 2530** | **0 → 1** | `1832517335179 → 1832517440561` | `158570 / 11h → 155160` |
| P1 | **2746 → 2748** | **1 → 2** | `1832528246746 → 1832528347305` | `158570 / 11h → 155160` |
| P1 | **3164 → 3166** | **2 → 1** | `1832549172798 → 1832549275645` | `158570 / 11h → 155160` |

Each gap spans only **99,727–105,382 raw QPC ticks**, not milliseconds; QPC frequency was not recorded here. The immediate Stage65 request observer has **zero** same-actor request rows strictly between the paired Stage38 boundaries for these five transitions (this excludes only its covered hook path). In all five next PRE windows, `f31=P+3F54` remains `0`; after the `158570` callback `f3=P+3ED0` changes from `158570` to `155160`, and input `f4=P+3ED4` changes from `0x11` to `FFFFFFFF`.

**Important causal boundary:** the value has **already changed before Stage38 PRE**, which is taken after native `156C00` and before `EE7BE CALL EDI`. The observed `158570` callback and the follow-on `155160` are strongly **associated with the downstream state**, but no evidence says either wrote the selector. The changes could have occurred in the per-frame preparation path or even in another actor's callback between two samples. The old `Stage38` callback is not a continuously armed memory watchpoint.

Additional `f31=P+3F54` observations: **7** within-callback `0→1` transitions (P1 **3**, P2 **4**), all in original callback `1580F0`; **16** between-window state transitions (P1 **5**, P2 **11**). This state is distinct from selector source. No `f30` transition occurred in any of the seven within-window state transitions.

## 3. Static cross-check of the common callback parameter 11h

Independently examined the original SHA-pinned private `AALib.exe` SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`; x86 PE32 base 00400000. Native callback `158570` dispatches through table VA **00558DC4**. Entry index **11h** is native VA **00558ACB** (RVA **158ACB**). Its first instructions check player **`P+3B7C` bit15**, branch if clear, and on its branch read **`P+3F50` at `158ADC`** before an action-dependent path:

```asm
158ACB  mov eax,[edi+3B7C]
158AD1  shr eax,0Fh
158AD4  test al,01h
158AD6  je  158B7D
158ADC  mov eax,[edi+3F50]     ; SELECTOR CONSUMER (conditional path)
158AE2  mov ecx,edi
158AE4  test eax,eax
158AE6  jne 158B18
```

Thus **the known callback key 11h leads to an original native branch that may consume the selector**; the exact `158ADC` instruction was **not** instrumented with an EIP probe and its bit15 gate is not claimed to pass in every frame. This static reader is **not a producer site** and cannot be equated with Stage71's literal-one writer `155D29`. All ten register-derived selector writers remain candidates in appropriate contexts.

## 4. Deferred action source — three independently linked within-window cases

All three Stage70 original-store observations fall **strictly inside an individual Stage38 `1566B0` callback** for the exact same P/O/TID/QPC. Each has a Stage65 native requester inside the same callback and immediately earlier by **7 raw QPC ticks** after Stage65 POST:

| Side | Stage38 | Stage65 seq / request / native call RVA | Stage70 seq / selector / deferred |
|---|---|---|---|
| P1 | **2568** | seq **230**, `2F`, `156769` | seq **1**, selector **1**, `FFFFFFFF→25` |
| P1 | **2786** | seq **248**, `33`, `156769` | seq **2**, selector **2**, `FFFFFFFF→26` |
| P2 | **3081** | seq **274**, `2E`, `156769` | seq **3**, selector **0**, `FFFFFFFF→24` |

Each Stage38 wrapper's `f20=O+68` PRE/POST agrees with Stage70 `deferred` PRE/POST (FFFFFFFF→value). The P1 selector values 1 and 2 had been established earlier in distinct between-window Stage38 transitions; P2 selector 0 is already present in the corresponding window. This confirms **three original native request/deferred pairs** with different selector values and **does not show how COM chose those values**.

## 5. Conclusion and next investigation: Stage75 STATIC FIRST

**Stage74 confirmed** source-ready Stage38 f30/f31 sampling compiled and ran, no data-read failures for f30/f31, no selector PRE→POST changes in any callback, and precisely five selector changes across prior-POST/next-PRE windows. The immediately following original callback has parameter **0x11** in every case and dispatches through switch key 11h. The selector writer is **upstream of the sampled callback invocation**, not demonstrated to be `158570`, `155160`, or literal `155D29`.

**Stage75 scope:** trace original **pre-callback setup path** that writes selector P+3F50 and sets callback `P+3ED0=158570` / argument `P+3ED4=11h`; statically inventory direct callers and field stores in the same native call tree. Account for Stage38 PRE being after `156C00`. Prefer narrow, original-byte-guarded, passive observer only if static call/dataflow yields bounded sites. **Do not automatically install 11 competing site hooks**, use DR2/DR3 without ownership review, or force values. Preserve the verified Stage74 DLL **1,893,376 bytes** and Stage70 r2 **1,892,864 bytes** as rollback. The raw source logs from commit `211f088` remain unchanged, no new DLL or runtime is needed to accept these Stage74 conclusions.
