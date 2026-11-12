# Stage97 r1 — independently recomputed LIVE runtime analysis (2026-11-12)

## Evidence, scope and method

**Fresh user runtime upload:** [GitHub main commit 61e62b6](https://github.com/Hawky11/1/commit/61e62b6de57b47fdeb1767a71f931e5463ca782a), and same-session root logs `Build.log`, `ArcanaAI_Stage97InheritedEsiTrace.log`, `ArcanaAI_Stage96EsiWriterTrace.log`, `ArcanaAI_Stage95AlternateEsiTrace.log`, `ArcanaAI_Stage94ProducerGateTrace.log`, and `ArcanaAI_Stage93FinalCommandQpcTrace.log`. [Stage97 source-only implementation](https://github.com/Hawky11/1/tree/stage97-inherited-esi-source-only/Stage97_Overlay). This run is distinct from the preceding Stage96 run; **never join historical seq/QPC identifiers across runs**.

Native AALib.exe source fingerprint verified during Stage97 static analysis: SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Stage97 observes `F77D4 CMP [EBP-8], -1; F77D8 MOV ESI,[EBP-2C]` before original instruction replay, and predicts the native destination of F77DB JNE. The exact old Stage96 bypass `seq9461` **does not occur in this session**; the identical 0x2C route reoccurred twice with new sequence numbers.

## Windows build and native installation

- **BUILD SUCCEEDED** (MSVC x86). `build/dinput8.dll` **2,045,440 bytes**. One existing nonfatal C4754 warning in `src/AICommandSiteTrace.cpp:786`.
- `STAGE97_INSTALLED 1/1 rva=0F77D4 originalOnce=1 Stage96Owner=1 nativeJNE=1 gameWritesExtra=0 DRwrites=0`.
- `STAGE96_INSTALLED 5/5`, `STAGE95_INSTALLED 2/2`, `STAGE94_INSTALLED 2/2`; Stage94 confirms the **exact Stage82 CALL-chain owner**, `viaStage82=1`.
- All Stage94/95/96/97 observers reported **46 HEALTH snapshots each**, with `filtered=0 dropped=0 mismatch=0 readFail=0`; Stage97 records have `valid=1 localRead=1 m8Read=1 flagRead=1`. These are observer-reported counters, not independent gameplay or memory-safety proof.

## Full counts, same run

| Event | Count |
|---|---:|
| Stage97 INHERITED (F77D4) | **12,997** |
| Stage97 finite localD4/ESI | **66** |
| Stage97 predicted F77DB JNE taken | **66** |
| Stage96 WRITER (five sites) | **14,071** |
| Stage95 BYPASS (F7A94) | **11,563** |
| Stage95 finite BYPASS | **61** |
| Stage94 PRODUCER | **1,434** |
| Stage94 finite F3930 results | **139** |
| Stage94 GATE | **200** (162 forward, 38 reject) |
| Stage93 FINAL_QPC | **162** |

All 200 Stage94 gates have value-supported, disjoint immediate routes: **139/200 F3930 producer** and **61/200 F7A94 bypass**. The 61 finite bypasses match exactly one Stage94 gate each with **2–49 raw QPC ticks** lag; no missing or ambiguous links. All **162/162** forward gates match unique real Stage93 FinalCommand QPC records, matching player, TID, payload ESI, payload arg and type1, with **3–73 QPC ticks** lag; no unpaired final records.

## ESI provenance — Stage97 closes observed Stage96 gap

For the 61 finite Stage95 bypass events, compare the most recent same-(player/TID/frame) Stage97 or Stage96 witness within a conservative **10,000 raw QPC ticks**, including later invalid/FFFFFFFF writers instead of cherry-picking a matching older value. **61/61 exact latest-writer value matches; 0 missing, 0 mismatches.**

| Latest native witness | Finite bypasses | Witness→bypass lag |
|---|---:|---:|
| Stage96 F79F0 | **39** | 1–8 QPC ticks |
| Stage96 F7924 | **13** | 1–8 QPC ticks |
| Stage96 F7A8F | **7** | 1–12 QPC ticks |
| **Stage97 F77D4** | **2** | **68–72 QPC ticks** |
| Unknown / mismatch | **0** | — |
| **Total** | **61** | |

Stage96 post-call writers include **zero** `nativeNextEsi=0000002C` records. Stage97 emitted exactly **two** `nativeNextEsi=0000002C` events; both had `localD4=0000002C`, `localM8=00000001`, `predictedJneTo789E=1`, `valid=1`, on P1. Each led in strict same-run time order to Stage95 F7A94 (ESI=0x2C), Stage94 F7B0A (forwardType1=1) and Stage93 FinalCommand (payload ESI and arg=0x2C):

| Witness→Final chain | First recurrence | Second recurrence |
|---|---:|---:|
| Stage97 INHERITED seq | **3471** | **8905** |
| Stage97 QPC | 2762727495409 | 2763072551015 |
| Stage95 BYPASS seq | **3140** | **7987** |
| Stage95 QPC | 2762727495477 | 2763072551087 |
| Stage97→Stage95 lag | **68** | **72** |
| Stage94 GATE seq | **386** | **1058** |
| Stage94 GATE QPC | 2762727495480 | 2763072551090 |
| Stage93 FINAL_QPC seq | **46** | **114** |
| Stage95→Stage93 lag | **11** | **15** |
| P1 / payload | 0x2C | 0x2C |

**Interpretation:** Stage97's original native `[EBP-2C]→ESI` route is proven to account for two new previously-uncovered bypass commands, closing the **observed** Stage96 writer-provenance gap in this run. It substantiates a plausible explanation of the earlier Stage96 `seq9461` / 0x2C anomaly, but **does not directly identify the source of that individual historical event**, whose QPC belongs to a different session. The upstream dynamic assignment at F7756 is supported by original static code; Stage97 did not hook F7756 and does not independently witness that store in the new run.

## Boundaries and follow-up

This is precise **native command provenance and real-command consistency**, not evidence of improved AI, different attacks, hit causality, or that ESI alone decides the payload in all possible game situations. Sample 61 finite bypasses and 200 gates belong only to this run. All hooks are passive by design and no new game/DR writes are reported; Stage36 is to remain OFF. Preserve tested Stage97 DLL (2,045,440 B) and Stage96 DLL (2,039,808 B). Further work should target a concrete gameplay/AI behavior question or native command-selection predicate, using narrow, exact guarded observation rather than adding speculative broad hooks.

