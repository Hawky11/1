# Stage101 — real Stage98 r2 F34D0 return-value and FinalCommand joins (2026-11-12)

**Existing Windows runtime logs analyzed, not a new Windows build or game run.** This closes the Stage100 evidence gap. All five full log contents were fetched via GitHub `fetch_blob` at exact blob SHA; the ordinary large-file GitHub contents response was empty, so relying on its blank text would have incorrectly suggested no data. Run upload [8919f8e](https://github.com/Hawky11/1/commit/8919f8ec242f1137c447d2eb4afc7ba0ed0c0e1e). Same run TID **11260**, P1 `0DDA1F78`, P2 `0DDCCCA0`, frame `0E58FC2C`.

## Source blob provenance and counts

| Exact GitHub log | Git blob SHA | Parsed runtime rows |
|---|---|---:|
| ArcanaAI_Stage98CommandRouteTrace.log | `fdac35245f47c3b19bee346aa6b53e964ab14cf7` | 14,107 ROUTE |
| ArcanaAI_Stage96EsiWriterTrace.log | `1e98d712ac78549e46e4e2e0719fc8995d1706fd` | 18,129 WRITER, **461 F7A8F** |
| ArcanaAI_Stage95AlternateEsiTrace.log | `3dba2624412b23842b9bd0a2a9a33647e256954c` | 12,474 BYPASS |
| ArcanaAI_Stage94ProducerGateTrace.log | `fba1df695c4fcac0af357c782cf521af24b91b93` | 1,633 PRODUCER, 201 GATE |
| ArcanaAI_Stage93FinalCommandQpcTrace.log | `4fa6b5dd3747e9c2abed7ea4daa11d77a637453b` | 167 real type1 FinalCommand |

## Method

For every valid Stage98 `branchClass=5`, require exact `player+tid+frame`, increasing QPC, the first next native Stage94 PRODUCER or Stage95 BYPASS *before the next same-context Stage98 snapshot*, and at most 10,000 QPC ticks. Require exactly one Stage96 `siteRVA=0F7A8F` between route and bypass. Verify **preEsi**, **resultEax**, **nativeNextEsi**, **Stage95 chosenEsi**, plus no reuse of writer/outcome. For an immediate Stage94 GATE in the same window, compare ESI and `forwardType1`. For forwarded gates, match original Stage93 FINAL_QPC by exact player/TID, ordered QPC and matching **both** `payloadESI` and `payloadArg`. Stage93 does not include frame; do not fabricate frame matching for that last link.

## Observed, not inferred

- **461/461** class5 routes uniquely match **461 F7A8F** post-F34D0 native writers and **461 F7A94** bypass records; all class5 expected modes 0000=**208**, 0001=**253**; no class5 mode2 captured.
- **461/461** have `preEsi=resultEax=nativeNextEsi=Stage95 chosenEsi`, so **zero changes** by F34D0 in this run. **459** remain `FFFFFFFF`, **2** remain `00000029`. Return `0x27` observed **0** times; special-return possibility is static, not observed.
- Route→writer lag **4–75 QPC ticks** (median14, p99 61); writer→bypass **1–168** (median2, p99 17); route→bypass **6–173** (median16, p99 67). Raw QPC ticks, not milliseconds.
- **459** class5 events never reached Stage94 GATE in their bounded event window. **2** finite (`0x29`) events both reached Stage94 GATE with `forwardType1=1`, and both matched a real Stage93 FinalCommand by payload/time/actor. No unexplained, ambiguous, cross-event or duplicate assignments.

| Native chain | Stage98 | Stage96 F7A8F | Stage95 F7A94 | Stage94 gate | Stage93 final | QPC deltas |
|---|---:|---:|---:|---:|---:|---|
| P1 ESI=0x29 | 7833 | 10228 | 6897 | 1050 | 93 | 22 / 6 / 3 / 7 |
| P1 ESI=0x29 | 8665 | 11036 | 7668 | 1122 | 103 | 21 / 6 / 4 / 8 |

`resultEax` already equaled incoming `ESI=0x29` in both finite cases. They were **not** created by substituting `0x27`. Native final-command delivery was directly observed; a command does not establish an animation, hit, AI improvement or safe action forcing.

## Static/native source and reproducibility

Stage100 original SHA-pinned EXE analysis established `F34D0/F30B0` output is either inherited ESI or special `0x27` under the inspected return paths. The exact original binary is private in ChatGPT Library (`AALib.zip`), SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. No binary copied to GitHub.

Stage101 Python offline five-log correlator + **10/10 synthetic unit tests** + results JSON are packaged as `ArcanaAI_Stage101_RealClass5_Return_FullLog_Analysis.zip`, SHA256 `a54e5441b83173ee834e58086e86d0bcde4896ff69a0fcf4b7db15117735b95c`. Full raw logs are *not* bundled; use their existing GitHub or local copies. The real counts above came from processing all five full Git blob contents through an independent in-session strict event join, not from running the synthetic tests. The ZIP has a portable script to reproduce the joins from full local same-session files.

**Next Stage102 (only if useful):** static interpretation of F34D0 internal conditions that suppressed substitution in this session, or natural mode2 evidence. Keep Windows-tested Stage98 r2 DLL **2,051,584 B**, Stage97 rollback **2,045,440 B**, Stage36 OFF; no new DLL/hooks/game or debug-register writes.
