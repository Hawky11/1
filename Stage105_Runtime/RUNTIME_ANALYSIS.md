# Stage105 — Stage104 r2 runtime confirms original zero-status 0x27 Mode 0 → CEA20 (2026-11-12)

**WINDOWS BUILD SUCCEEDED, HOOKS INSTALLED, NATIVE ROUTE OBSERVED.** No additional game-state/DR writes, no forced commands, Stage36 remains OFF. New DLL/hook behavior beyond the monitored passive patches is not independently proved ABI-equivalent for every scenario. The log evidence does not prove a hit, animation, combat balance change or improved AI.

## Provenance: same-run user upload

- Raw evidence uploaded to [commit 24885c9](https://github.com/Hawky11/1/commit/24885c90922251cd065ae812c7f1d30b000c7b66). This Stage105 branch is based directly on that commit and preserves raw logs without alteration.
- Local Windows Build.log: started **2026-11-12 17:33:10** per host log. All eight sources compiled using MSVC 14.51.36231 x86; **BUILD SUCCEEDED**, `build\dinput8.dll` **2,058,240 B**, only known nonfatal C4754 at AICommandSiteTrace.cpp:786. No new build was made by analysis.
- Stage104: `STAGE104_BUILD=2026-11-12-r2`; `STAGE104_COMPAT F2DE0 preexisting=E9_REL32_NOP targetVA=78E3F920 suffixOriginal=1 executableCommitted=1 preserved=1 ownerUnidentified=1`; **`STAGE104_INSTALLED 2/2`** at F7CC0/F7DA3, originalOnce=1, noF7C5D=1, extraGameWrites=0, DRwrites=0. The preexisting F2DE0 detour's owner remains unknown.
- Unique TID **16500**, frame **0E58FC74**, player P1 **0DDA1F78**, P2 **0DDCCCA0**, throughout relevant records.

### Exact Git blob provenance

| Data | Git blob SHA | Parsed records |
|---|---|---:|
| Build.log | `328667f251750a73327563702f6b275e074152a8` | 1 successful build |
| ArcanaAI_Stage104Mode0NativeTrace.log | `7fee1be00c73a367bf72cb4d60dc3a62988bf6d8` | 46 MODE0, 17 CEA20, 55 HEALTH |
| ArcanaAI_Stage94ProducerGateTrace.log | `768cda1820f9c31a7f484b5e6dab91581e135061` | 1,982 PRODUCER, 245 GATE, 55 HEALTH |
| ArcanaAI_Stage93FinalCommandQpcTrace.log | `b7b36b07d0d60e6dcfdec596307061ee3236d967` | 199 FINAL_QPC |
| ArcanaAI_Stage95AlternateEsiTrace.log | `22617acf9c15d253bea58d066a8bde148e33a8bf` | 14,838 BYPASS, 55 HEALTH |
| ArcanaAI_Stage96EsiWriterTrace.log | `0ff729ed265b544a391353af094ab58eb7cde200` | 19,059 WRITER, 55 HEALTH |
| ArcanaAI_Stage97InheritedEsiTrace.log | `672d88e9fbc8cb07d83d1b0a7932da107584e45b` | 16,820 INHERITED, 55 HEALTH |
| ArcanaAI_Stage98CommandRouteTrace.log | `997664586965e7bca5a3d9701698da7e7857c137` | 16,820 ROUTE, 55 HEALTH |

Large Stage95–98 GitHub contents previews were empty. Their full contents were fetched by **exact blob SHA**, not inferred from the preview. The Stage93 DirectBit3Trace file is uploaded too but was not necessary for this path join.

## Full native gate reconciliation — confirmed 245/245

- **245 Stage94 GATEs**: **199 ECX=1** and **46 ECX=0**, with no other ECX value.
- **199/199** ECX=1 matched unique Stage93 `FINAL_QPC type=1` with same actor + TID, identical payloadESI/payloadArg, QPC increasing from Stage94 by <=1,000 raw ticks. None had ESI=0x27.
- **46/46** ECX=0 matched unique Stage104 `MODE0 F7CC0` with same actor + TID + frame, identical chosenESI, increasing QPC <=1,000 raw ticks. No ambiguity, missing or duplicated matches. Across all 245 joins gate-to-terminal QPC deltas range **2–80 raw ticks**.
- Those 46 MODE0 events are **17 ESI=0x27** (P1 **6**, P2 **11**) plus **29 other ESI**: 0x0E=12, 0x10=8, 0x26=4, 0x25=2, 0x20=3. The mode-0 path is not exclusive to code 0x27.
- Every one of **17/17** `MODE0 special27=1` has exactly one subsequent `STAGE104_CEA20 special27=1` with same player, TID, frame, ESI, and QPC delta **7–82 raw ticks** (median **13**). All have `valid=1`. This is **live execution of F7CC0 and F7DA3** in this run. A CEA20 entry after MODE0 also supports that control returned/progressed past the F2DE0 call, but CEA20's eventual effect was not captured.
- The 17 observed 0x27 cases have 15 immediate Stage94 `F3930 PRODUCER` sources and **two Stage95 BYPASS** sources; all 17 have unique immediate matches. Do not attribute producer 0x27 to F34D0.

## Important new positive class5 substitution: F34D0 really returned 0x27 in a live class5

Earlier Stage101 evidence reported **461/461 class5 unchanged** in the DIFFERENT Stage98 r2 session. This new same-run Stage98 log has **479 branchClass=5** and 479 Stage96 F7A8F writers: **476 FFFFFFFF, 1 inherited 0x60, 1 inherited 0x22, 1 actual FFFFFFFF→0x27 change**. The one change:

**P2, thread 16500, player 0DDCCCA0, frame 0E58FC74:**
1. Stage98 `ROUTE seq=12768 branchClass=5`, QPC **2910778963976**; initial ESI=FFFFFFFF.
2. Stage96 `WRITER seq=14812 siteRVA=0F7A8F`, QPC **2910778964003**, `preEsi=FFFFFFFF resultEax=00000027 nativeNextEsi=00000027`.
3. Stage95 `BYPASS seq=11195`, QPC **2910778964006**, `chosenEsi=00000027`.
4. Stage94 `GATE seq=1755`, QPC **2910778964009**, `ecx=00000000 forwardType1=0`.
5. Stage104 `MODE0 seq=43`, QPC **2910778964026**, `esi=00000027 status=0 special27=1`.
6. Stage104 `CEA20 seq=44`, QPC **2910778964040**, `esi=00000027 special27=1`.

**Second distinct bypass origin** is P1 class3 F79B6:
Stage96 `F79B6 seq1052` (QPC 2909994332997, FFFFFFFF→0x27), Stage96 F79F0 seq1053 (QPC 2909994333030), Stage98 class3 seq1095 (QPC 2909994333034), Stage95 bypass seq826 (QPC 2909994333040), Stage94 gate seq287 (QPC 2909994333045), Stage104 MODE0 seq5 (QPC 2909994333072), CEA20 seq6 (QPC 2909994333079). Do not merge the two distinct F34D0 callsites.

## Health/limits

All **55 Stage104 HEALTH** snapshots show dropped=0, mismatch=0, readFail=0, filtered=0; summed mode0=46, cea20=17. Stage94, 95, 96, 97, 98 each report **55 HEALTH snapshots**, all with no dropped/mismatch/readFail. The event rows inspected have `valid=1`; no signature/installer errors seen in these logs. This is scoped logger health, **not proof of game-wide stability or improved AI**; the new user upload does not include a refreshed general ArcanaAI.log or video of combat.

**Conclusion:** the original native **status0 + ESI0x27** route is no longer just statically inferred: **F7CC0 MODE0 → F7DA3 CEA20 is executed**. Additionally the previously unobserved **class5 F34D0 substitution FFFFFFFF→0x27** is live-confirmed, once in 479 class5 samples. Original Stage101 counts remain valid for their earlier run. Do not patch ECX, force attacks or count motion/hits on the basis of this alone.

### Next Stage106

If further work is needed, first audit **CEA20 post-return behavior and passive available logs** to see whether a generated mode-0 event gets consumed. Avoid a redundant hook at already-detoured F2DE0. Make no new gameplay writes, DR use, or hit-quality claims. Stage104 r2 is the **latest Windows-built, installed and live-tested observer**, DLL 2,058,240 bytes; proven-safe fallback from prior run Stage98 r2 2,051,584 bytes, Stage97 2,045,440 bytes. Preserve all historical branches and the uploaded raw log snapshot.
