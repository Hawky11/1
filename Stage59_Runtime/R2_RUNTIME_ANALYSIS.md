# Stage59 r2 — dynamic X-write attribution VERIFIED (2026-11-10)

**Status:** Windows x86 build and short COM-vs-COM runtime capture VERIFIED; actual after-write EIP and native instruction sites established for the observed P1/P2 X stores. **Diagnostic only; no improvement in AI or gameplay behavior established.**

## Provenance and build

- User's six-file same-run upload: [commit d2ce2444](https://github.com/Hawky11/1/commit/d2ce2444b569e439db66e076f8428ac4dfc611cc); immutable raw snapshot: [stage59-r2-runtime-writer-attribution-evidence](https://github.com/Hawky11/1/tree/stage59-r2-runtime-writer-attribution-evidence). Do not change the source or raw-evidence snapshots.
- Source branch: [stage59-r2-wait-for-combat-objects](https://github.com/Hawky11/1/tree/stage59-r2-wait-for-combat-objects), source HEAD at review `02f0c02f0d710d754df408667ca45ebbf7e642b3`. Stage59 r2 changes only the wait/readiness logic relative to r1; Stage55–58 overlay retained.
- Local x86 MSVC Build.log started 2026-11-10 **22:07:10.52**: **BUILD SUCCEEDED**, `build/dinput8.dll` **1,153,024 bytes**. Eight C++ translation units compiled, old C4754 warning at AICommandSiteTrace.cpp only. Do not confuse the DLL size with Stage59 r1's 1,152,512 bytes.
- Runtime Stage59 worker **22:07:43**; `STAGE59_OBJECTS_READY waitedMs=12750` and `STAGE59_WATCH_ARMED` at **22:07:55**. P1 player `0E4A1F78`, object `2A89A020`, watched X `2A89A09C`; P2 player `0E4CCCA0`, object `2A89AC50`, watched X `2A89ACCC`. DR0/DR1 write-only 4-byte, DR7 `00DD0005`, DR2/DR3 preserved.
- Stage55 3/3, Stage56 2/2, Stage57 3/3, Stage58 3/3 installed. Stage59 reported **62 THREAD_INSTALLED** lines; final `ownedThreads=61`. All 16,121 X_WRITE events were on observed game thread TID 15132, despite watchpoints being installed on many other threads. Final Stage59 HEALTH: `totalWrites=16121 totalSeq=16121 dropped=0 readFail=0 ownershipConflicts=0 installFailures=0 teardownFailures=0 objectDrift=0`.

## Capture integrity

| Trace | Contiguous event seq | Distribution | Sampled HEALTH |
|---|---:|---|---:|
| Stage55 | 15,966 | 5,322 each `A119B/AE6E0`, `A11A0/ACDF0`, `A11A5/AC8A0` | 23 |
| Stage56 | 270 | 270 `ACF27/ACF2A`, zero `ACFA0/ACFA3` | 13 |
| Stage57 | 15,984 | 5,328 each `ACE11/A2C40`, `ACFD6/15CEC0`, `ACFDB/15B8A0` | 23 |
| Stage58 | 664 | 664 `ACF0F/B86A0`, zero other two conditional sites | 14 |
| Stage59 | **16,121** | EIP-after distributions detailed below | 25 |

All Stage55/57/58 PRE/POST masks are `1FFF`. Inspected HEALTH lines have zero missingObjects, otherObjects, dropped, overflow, mismatch, badCaller, readFail, conflicts and failures in the fields applicable to each stage; no Stage59 seq gap.

**Important distinction:** Stage59 `oldShadow` is explicitly approximate; a shadow-difference count is not by itself a guaranteed independent change. In this run the 2,649 changed Stage55 windows below have matching Stage59 QPC, raw old/new values, and original instruction attribution.

## Stage59 watchpoint RVA_after distribution

| EIP_after RVA | Hits | `oldShadow != newRaw` | Native store (original PE) |
|---|---:|---:|---|
| `000ACC5A` | 10,816 | 379 | `ACC57: FSTP DWORD PTR [EBX+7C]` |
| `001E70DE` | 4,711 | 1,985 | `1E70DB: FSTP DWORD PTR [ESI+7C]` |
| `00126A9D` | 148 | 148 | `126A9A: FSTP DWORD PTR [ESI+7C]` |
| `00126B4C` | 148 | 148 | `126B49: FSTP DWORD PTR [ESI+7C]` |
| `FFFFFFFF` (EIP `78E2FEF2`) | 288 | 50 | Outside AALib; strongly correlates to Stage56 detour; confirm DLL module map before naming its code address |
| `001E8ABE` | 6 | 5 | `1E8ABB: FSTP DWORD PTR [ESI+7C]` |
| `000EB3CC` | 2 | 1 | `EB3C9: FST DWORD PTR [EAX+7C]` (not FSTP) |
| `001276A3` | 2 | 2 | `1276A0: FST DWORD PTR [ESI+7C]` (not FSTP) |
| **Total** | **16,121** | **2,718** | |

The outside-AALib `EIP_after` must not be interpreted as an unknown native RVA; of its 288 events, **270** nest uniquely within matching Stage56 `ACF27+ACF2A` before/after QPC windows, including **all 50 Stage56 changed-X events**. Another 18 non-changing events occur beyond the logged Stage56 windows; do not claim their specific owning module without a module map. In all cases EIP_after is the **next instruction**, not the store RVA.

The *original private executable* was rechecked locally (not uploaded): SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`; `objdump` disassembly confirmed the quoted original instructions and their preceding arithmetic. This static check does not prove higher-level movement, physics, collision or AI semantics.

## Strict same-run parent-to-writer correlation

Join rules: **same side, TID, watched address == object+0x7C**, and `parent.qpcBefore <= STAGE59.qpcAfter <= parent.qpcAfter`. Stage57 helpers must also be uniquely contained in each parent by object and QPC; Stage56 and Stage58 join within same native invocation. Never combine numerical counts from earlier Stage55–58 sessions.

### AE6E0 early motion native

- Stage55 `A119B→AE6E0`: **5,322** complete P1/P2 parent windows; **1,930** net raw-X changes.
- All **1,930/1,930** changed parents contain a Stage59 `1E70DE` event with nonzero oldShadow difference. First matched oldShadow equals parent PRE X and last matched newRaw equals parent POST X, for 1,930/1,930. `1E70DB FSTP [ESI+7C]` is the native instruction immediately preceding the observed EIP_after; this directly attributes the early-window change in this run.
- `A11A5→AC8A0` has zero net raw-X changes in its 5,322 side-windows.

### ACDF0 native

- Stage55 `A11A0→ACDF0`: **5,322** complete side-windows, including exactly one of each Stage57 `ACE11`, `ACFD6`, `ACFDB` with full matching object/thread/QPC. **719** net X changes (P1 **485**, P2 **234**); 4,603 no net X change.
- **All 719/719 changed parents contain Stage59 X_WRITE with actual changed raw value**. The first matching shadow is the Stage55 PRE X and the last matching raw is Stage55 POST X, 719/719. Zero unexplained changed windows under the observed coverage.
- **429 changes in the `ACE11 POST→ACFD6 PRE` pre-tail gap:** **379** finish at `ACC5A` (original `ACC57 FSTP [EBX+7C]`); **50** finish at EIP `78E2FEF2`, nested in Stage56's instrumented `ACF2A` original FADD/FSTP, 50/50 changed.
- **290 changes inside `ACFD6→15CEC0`:** 145 finish at `126A9D` (original store `126A9A`) and 145 finish at `126B4C` (original store `126B49`).
- No overlap of the two changed phases *in this run*: **429 + 290 = 719**. Original Stage57 log contains **294** changed ACFD6 side-events overall, of which **290** belong to the 5,322 complete Stage55 parents. The remaining four changes fall in the Stage57 unmatched tail; exclude them instead of claiming 294/719 complete-parent matches.
- Every ACDF0 window had Stage59 write events (including unchanged repeated stores): 11,204 Stage59 events nested in all parents, of which 719 show oldShadow difference; the remaining 4,917 Stage59 events occur outside these parent windows.

### Stage58 branch control flow

- Stage58 has **664** per-side observations from **332** physical calls to `ACF0F→B86A0`, all its own PRE/POST X snapshots unchanged.
- **270 calls returned EAX=0**; each has precisely one Stage56 ACF2A writer event after Stage58 (4–77 QPC ticks later), including unchanged stores.
- **62 calls returned EAX=1**; none has a Stage56 ACF2A writer before the next Stage58 call. This reproduces the original branch control-flow relationship on this separate new run, without implying all those stores change coordinates.

## Interpretation / next Stage60

**What is established:** Stage59 r2 startup wait works, dynamic DR0/DR1 observations run with no reported ownership failures, and Stage55 early and ACDF0 net-X changes are exhaustively attributed to observed after-write instructions in this short capture. The pre-tail unidentified writer is the original `ACC57` FSTP and the final helper contributes `126A9A`/`126B49`. This closes the *observed X-writer attribution gap* from Stages 55–58.

**What is not established:** whether any measured coordinate write is an AI decision, improved attack selection, hover-dash causal change, collision outcome, or gameplay quality improvement; whether other game modes/threads have unobserved paths; exact module identity for the detour EIP. Do not patch motion values or command state on the strength of these traces.

**Recommended Stage60:** stop expanding generic coordinate boundary hooks. First perform **static caller/dataflow analysis** of the verified original X stores `ACC57`, `126A9A`, `126B49`, `1E70DB`; map their call chains, input terms and relation (if any) to COM-vs-COM AI action/command production. Decide whether a *small targeted passive observation* of an upstream AI decision is warranted. Explicitly separate engine motion/physics from AI behavior; if behavior changes are tested later, use an A/B run and repeatable gameplay evidence. Keep the Stage59 r2 working DLL as rollback. Preserve both Stage59 source branches and this frozen evidence branch.

**Status:** Stage59 r2 BUILD + RUNTIME + X-writer attribution VERIFIED for this session. Stage60 source, Windows build and runtime are NOT prepared.
