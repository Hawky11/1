# Stage84 r1 long-run exact selector writer evidence (2026-11-11)

**Status: WINDOWS X86 BUILD SUCCEEDED; RUNTIME INSTALLED 6/6; WRITER EIP VERIFIED FOR 15/16 NET SELECTOR TRANSITIONS.**
This is observational instrumentation, **not a gameplay/AI enhancement**. Do not infer a behavior improvement from these traces.

## Reproducible evidence and provenance

- Current same-run log upload: [commit `364429c`](https://github.com/Hawky11/1/commit/364429c76dbfbfecd7c1679dd3e4dacd90e4897c), lines timestamped **2026-11-11 17:36:20–17:38:08** local. Use the five root logs at this exact commit: `ArcanaAI_Stage79SelectorFamilyTrace.log`, `ArcanaAI_Stage80F7150LateTrace.log`, `ArcanaAI_Stage81SelectorWriterTrace.log`, `ArcanaAI_Stage82NestedSelectorTrace.log`, `ArcanaAI_Stage84F3930WriterTrace.log`.
- Build.log is unchanged from the preceding upload [`abef411`](https://github.com/Hawky11/1/commit/abef4114a210ac7dac014a9fdd016233b833f0d5), dated **17:31:32**, Windows MSVC x86 **BUILD SUCCEEDED**, DLL **1,952,768 bytes**, one pre-existing **C4754** warning. The later runtime uses a Stage84 marker; no separate build log was supplied for the later run.
- Stage83 preceding test: [`394d68c`](https://github.com/Hawky11/1/commit/394d68c567b41012d41a49007229cb4bf1594080), DLL **1,952,256 bytes**, **Stage83 4/4 installed**, five `STAGE83_EXACT_WRITE` events (2 changed), established actual direct writer RVAs `0F3F50` and `0F3F69`.
- Stage84 first short test: [`abef411`](https://github.com/Hawky11/1/commit/abef4114a210ac7dac014a9fdd016233b833f0d5), Stage84 **6/6 installed**, six exact writes, three changes, plus actual `F3F71` execution `0→0`.
- Original `AALib.exe` reference SHA256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` (private original, not committed).
- Stage36 must remain **disabled** (`EnableStage36ControlToCommandTrace=0`) while Stage82 hooks `F79E4`, since Stage36 also detours that exact `CALL`. Stage82 r2 live-byte audit previously demonstrated the conflict. Stage81/82/84 successfully installed in the current run. Keep existing verified DLL backed up.

## Installation and integrity

- Stage79 installed 1/1 at `0EFFE8→0E89E0`.
- Stage80 installed 1/1 at `0E8A8E→0F7150`.
- Stage81 installed 3/3 at `0F7D22`, `0F7D38`, `0F7D43` (zero native writer hits in this run; not the observed F3930 writer path).
- Stage82 installed 2/2: `0F79E4→0F4360` and `0F7AD1→0F3930`. Both original CALL bytes matched in memory.
- Stage84 `STAGE84_INSTALLED 6/6`: exact signatures of all native writes matched at `0F3DE9`, `0F3E02`, `0F3F50`, `0F3F69` (6-byte stores) and `0F3E0A`, `0F3F71` (10-byte immediate-zero stores).
- Stage84 logs `originalMOV=once`, `DRwrites=0`, `extraGameDataWrites=0`; sampled HEALTH in Stages79, 80, 81, 82, 84 has **zero** dropped, overflow, mismatch, readFail, filtered and other reported error counts. These claims are about observer diagnostics, not a proof of global gameplay equivalence.

## One-session QPC containment

Match requires **same side, player, object, TID** and full integer-QPC containment; do not match by approximate wall-clock times or across runs.

| Observation | Count | Unique parent matches |
| --- | ---: | ---: |
| Stage79 `COMMON_WINDOW` | 15,116 | N/A |
| Stage80 `LATE_F7150` | 13,849 | 13,849/13,849 inside Stage79 |
| Stage82 `NESTED` | 3,784 | 3,784/3,784 inside Stage80 |
| Stage84 `EXACT_WRITE` | 21 | 21/21 inside Stage82 |

Stage82 site distribution: **1,974** `F4360` and **1,810** `F3930`. All Stage84 events match Stage82 site 1 (`F3930`). No missing or ambiguous parent matches.

Stage80 observed **16 net changes** of selector `player+0x3F50`, of which **15** have a same-interval Stage84 exact store with identical before/after; Stage84 records **15 changed** and **6 non-changing** native stores total. Each changed Stage84 event matches the Stage79/80/82 containing QPC window (same side/player/object/TID).

### Exact native writers in Stage84

| Native writer RVA | Native store | Hits in this run | Confirmed effects |
| --- | --- | ---: | --- |
| `0F3DE9` | `mov [esi+3F50h], ecx` | 0 | Not executed in sample |
| `0F3E02` | `mov [esi+3F50h], edx` | 0 | Not executed in sample |
| `0F3F50` | `mov [esi+3F50h], edx` | 9 | `0→1`, `2→1`, and idempotent stores |
| `0F3F69` | `mov [esi+3F50h], ecx` | 7 | `0→1`, `1→2`, and idempotent stores |
| `0F3E0A` | `mov [esi+3F50h], 0` | 0 | Not executed in sample |
| `0F3F71` | `mov [esi+3F50h], 0` | 5 | **`1→0` twice, `2→0` once**, `0→0` twice |

High-value source rows from Stage84:
- `seq=5` P2 `2→0` at **`F3F71`** (`qpcBefore=2047485065344`).
- `seq=7` P2 `1→0` at **`F3F71`** (`qpcBefore=2047574185421`).
- `seq=16` P2 `1→0` at **`F3F71`** (`qpcBefore=2047757914777`).
- `seq=12` P1 `1→2` at **`F3F69`** (`qpcBefore=2047639153058`).
- `seq=14` P1 `2→1` at **`F3F50`** (`qpcBefore=2047722462811`).

## Unattributed alternate `1→2` path — Stage85 target

One net selector `P+3F50` change is **not** explained by any of the six Stage84 exact writer hooks:

| Boundary | Seq | Actor / QPC | `P+3F50` | `P+3F54` |
| --- | ---: | --- | --- | --- |
| Stage79 | 2,478 | P2, same native window | `1→2` | Review raw row |
| Stage80 `F7150` | **2,160** | P2, `2047470733250–2047470733393` | **`1→2`** | **`1→2`** |
| Stage82 `F3930` | **513** | P2, `2047470733285–2047470733364` | **`1→1`** | **`1→2`** |
| Stage84 | — | no matching `EXACT_WRITE` | — | — |

The `F3930` boundary reports its companion field `P+3F54` changing `1→2`, while `P+3F50` is unchanged at its return. The latter changes between the Stage82 return and the Stage80 return. **The exact instruction EIP responsible is not established.** Do not attribute it to `F3F50/F3F69` or any other writer on timing alone.

**Stage85 proposed scope:** static disassembly/verified call graph of `F7150` after its `F3930` call returns; find direct or indirect stores to `P+3F50` and identify an exact EIP for this surviving `1→2` route. Only add a guarded read-only observer if exact native instruction bytes and ABI/control-flow are independently verified. Do not change AI behavior, mutate the selector, overwrite a pre-existing hook, or bypass signature guards. Preserve the known-good Stage84 **1,952,768-byte** DLL and current logs. No need to rerun solely to confirm already-proven `F3F71` resets.

## Verification boundaries

- These are **separate same-session** Stage83 and Stage84 captures. Never merge QPC between different runs.
- “Exact writer” means the x86 instruction was observed executing and recording before/after values; it does not establish its upstream policy/semantic choice.
- No COM AI behavior improvement has been demonstrated. All gameplay behavior remains the original game's path, subject to the usual perturbation risk of diagnostic instrumentation.
