# Stage61 same-session action definition → motion consumer → X write — RUNTIME VERIFIED

**Date:** 2026-11-10. **Status:** successful exact same-session, passive Stage46/47/59 correlation. **No Stage61 DLL was built or installed, and no improvement to AI/action choice is established.** This report uses previously built Stage59 r2 instrumentation.

## Evidence provenance / fixed source

- [New user upload commit c09bc526](https://github.com/Hawky11/1/commit/c09bc52690f149b0c7159014d01b86deb8ae5f79) contains **only the two newly provided** Stage46 and Stage47 text logs. Frozen exact root snapshot: [stage61-runtime-action-motion-x-evidence](https://github.com/Hawky11/1/tree/stage61-runtime-action-motion-x-evidence).
- Stage46 log blob SHA `c973b4593a6de15a045220b39c9b8d010db6c764` (928,960 bytes); Stage47 blob SHA `d87f182d4cd66fb19aeaf16439428c905c7b6ced` (3,018,532 bytes).
- Original **same-session** Stage59 r2 was uploaded earlier at [commit d2ce2444](https://github.com/Hawky11/1/commit/d2ce2444b569e439db66e076f8428ac4dfc611cc), blob SHA `a5a2e76dcc473e079d0445db6e9d7069a3784879` (4,755,661 bytes). **No Stage59 file was updated** in c09bc526. Stage59 existing build 22:07:10, DLL 1,153,024 bytes, successful DR watchpoint arming (see [Stage59 r2 report](../Stage59_Runtime/R2_RUNTIME_ANALYSIS.md)).
- [Stage61 read-only correlator source](https://github.com/Hawky11/1/blob/stage61-action-definition-motion-join/Stage61_Analysis/tools/correlate_stage61.py), [static analysis](https://github.com/Hawky11/1/blob/stage61-action-definition-motion-join/Stage61_Analysis/STATIC_ANALYSIS.md); software itself does **not** patch memory or disassemble the private EXE at runtime.

## Same-session integrity confirmed

| Observer | Rows | Unique contiguous record IDs | Object/TID |
|---|---:|---|---|
| Stage46 `STAGE46_MOTION_WINDOW` | **1,476** | stage44Seq `1..1476` | P1 `2A89A020`, P2 `2A89AC50`, TID `15132` |
| Stage47 `STAGE47_CONSUMER_WINDOW` | **5,292** | seq `1..5292` | Same objects, TID `15132`; callsite `AE70C` all rows |
| Stage59 `STAGE59_X_WRITE` | **16,121** | seq `1..16121` | Watched P1 X `2A89A09C`, P2 X `2A89ACCC`; same TID |

Stage46/47 start markers are **22:07:42**, observed rows begin **22:07:56**, and Stage59 started **22:07:43** / was armed **22:07:55**. Stage46 QPC begins `134609080...`, Stage47 and Stage59 share the later `134609319...` frame sequence. No cross-session normalization or time-shift was used. Both Stage46 and Stage47 have full `preMask=postMask=1FFF` for every data row; Stage46 has `resolverCount=1` for every row and non-null definition pointers. Stage47 also has `extraMask=F`, and 23 Stage47 HEALTH samples have `dropped=overflow=mismatch=badCaller=0`. Stage46 has no independent HEALTH lines: avoid treating absence as measured health. Previously reviewed Stage59 HEALTH samples are zero for dropped/readFail/ownership conflicts/failures/drift.

## Exact join algorithm and results

Require all of:
1. Same **P1/P2 side, object pointer, thread ID**. For Stage59 derive object = `watched − 0x7C`; no guessing.
2. `Stage47.qpcBefore <= Stage59.qpcAfter <= Stage47.qpcAfter`. Accept only post-write `RVA_after=001E70DE`, corresponding to original `1E70DB FSTP [ESI+0x7C]` within `1E66E0`; do **not** mix other write sites or interpret EIP_after as pre-store RVA.
3. Pick only the **latest earlier** same-actor Stage46 binding where `Stage46.qpcAfter <= Stage47.qpcBefore`; require `Stage46.definitionAfter == Stage47.def PRE`.
4. Compare `Stage46.f0 POST == Stage47.f0 PRE` for active action ID (object+0x60); record metadata mode `Stage46.meta30 & 0xFF`, `Stage47.f5` raw X PRE/POST and last Stage59 newRaw. This proves temporally consistent native *state*, not who selected it.

**Observed outcomes:**

| Check | Result |
|---|---:|
| Stage59 events with post-write RVA `1E70DE` | **4,711** |
| Nest in a unique matching Stage47 native QPC window | **4,617** |
| Stage47 windows containing these Stage59 events | **4,617** |
| Stage47 windows with net changed raw X and contained Stage59 | **1,906** |
| Strict preceding same-definition Stage46→Stage47→Stage59 triples | **4,616** |
| Triple with same active action ID in Stage46 POST / Stage47 PRE | **4,616/4,616** |
| Strict triples with net X change | **1,905** |
| Stage47 windows with contained Stage59: last newRaw equals Stage47 POST raw X | **4,617/4,617** |
| Stage47 windows with contained Stage59: first approximate oldShadow equals Stage47 PRE raw X | **4,617/4,617** |
| Stage47 windows without contained Stage59 at this RVA | **675**, **zero** net X change |

There are **94** original `1E70DE` events not nested in uploaded Stage47 windows: **47 P1 + 47 P2**, all **after** Stage47's final logged QPC. This is a tail-coverage boundary, not proof of a different writer or lost events. The present trace does not diagnose why Stage47 logging stops earlier than Stage59.

The **one** triple-definition mismatch is final Stage47 `seq=5292` P1: Stage47 PRE `def=261E62D8`, but latest recorded P1 Stage46 was `stage44Seq=1476`, `definitionAfter=261E627C`. Active ID still `00000700`; raw X changes `44AA07C8→44AB003E` and Stage59 seq `15767` matches POST. Because Stage46's last logged event predates this Stage47 window, a later matching Stage46 binding may be absent; do **not** call this an actual game-state mismatch, corrupted action, lost native call or AI error. Exclude it from the strict 4,616 triples.

## Metadata-mode correlation within 4,616 strict triples

Mode = **low byte** of `definition+0x30` observed in Stage46, not an attack/action identifier. Raw X difference is Stage47 PRE vs POST. Do not conflate with the separate O+60 active ID.

| Definition mode | Strict triples | Net changed X | Proportion |
|---|---:|---:|---:|
| 0 | 3,989 | 1,567 | 39.3% |
| 1 | 134 | 2 | 1.5% |
| 2 | 57 | 13 | 22.8% |
| 3 | 226 | 213 | 94.2% |
| 4 | 109 | 107 | 98.2% |
| 5 | 98 | 0 | 0% |
| 7 | 3 | 3 | 100% (tiny sample) |
| **Total** | **4,616** | **1,905** | **41.3%** |

The high observed change frequencies for mode 3/4 are consistent with Stage46's prior *static* observation that these modes initialize movement floats, but **not** proof of a COM AI choice or that this movement is preferable. Mode 0 still accounts for most absolute X changes (1,567), often reflecting continued integration from earlier state. The `Stage46.meta30` distribution among *binding events* differs from that among repeated Stage47 *time windows*; do not mix denominators.

## Interpretive limits and next Stage62

**Established for this one native session:** same actor/thread/time/definition and active ID can be carried from a definition binding to an observed position-integrator call and hardware-trapped original X store. The native read/compute/write path is no longer only an inference based on different runs. No code patch in Stage61 and no observed AI improvement.

**Not established:** that a COM controller intentionally selected a movement/action, which decision led to this state, rendered animation, attack/hit/damage outcome, or human-vs-COM causal difference. A previously active ID persisting into many frames is not a new AI decision each frame.

**Stage62 proposal:** connect *same-session* Stage40/42/44 to these matched actor/ID/definition windows to determine when an active ID was enqueued and became active. Existing GitHub root Stage40/42/44 logs are from **19:56**, not this **22:07** session, so do not join them. If the user has the 22:07 copies, upload only the Stage40, Stage42, Stage44 logs (or a smaller verified subset once selected); otherwise one new normal COM-vs-COM with the unchanged Stage59 r2 DLL can yield a homogeneous capture. A next source-only offline join should distinguish command request, actual activation, definition binding, repeated movement windows, and original X writes before considering any game-state modification.
