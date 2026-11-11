# Stage92 r3 — live 27,574-row native sequence verified (2026-11-12)

**Evidence:** latest user upload [7e1df728bbfb85b797c06785cd8cf7e407c53a35](https://github.com/Hawky11/1/commit/7e1df728bbfb85b797c06785cd8cf7e407c53a35). All values below independently computed by parsing complete GitHub Git blobs (rather than truncated Contents API files). No game-state modifications by this stage.

## Build and installation

- MSVC x86 `BUILD SUCCEEDED`; `dinput8.dll` **2,011,648 bytes**; only known warning C4754.
- `STAGE92_STAGE35_COMPAT adjacentACABE=paired verified=1 TIMER_target=78E45DF0 FLAGS_target=78E45C90 sameDll=1 exec=1 originalStage92Calls=3 guardBypass=0`.
- `STAGE92_INSTALLED 3/3 ACAB8->F5990 F5A86->9ED20 F5A8F->9ED60 originalOnce=1 noEE306=1 gameWrites=0 DRwrites=0`.
- 48 `STAGE92_HEALTH` rows; all `overflow=dropped=mismatch=readFail=0`. Log 17,788 TICK and 9,786 HELPER; all `valid=1`, exact P1/P2 actor argument on all 17,788; 9,786 helper-to-tick joins by seq/actor/player, no orphans/duplicates.

## Exact Stage91 -> Stage92 runtime control flow

For **all 17,788 F5990 tick records**, the `stage91SeqBefore..stage91SeqAfter` ranges map to exactly the observed 31,605 Stage91 `F58E0` reader records: **0 missing, 0 actor/TID mismatch, 0 QPC-outside-tick**. All 31,605 `predictionMatch=1`, all 48 Stage91 health rows have `overflow=dropped=mismatch=readFail=0`.

| Stage91 reads per F5990 tick | Count | Native result |
| --- | ---: | --- |
| No index 9/10/11 read | 6,502 | no 9ED20/9ED60 helper |
| Bit 9 = 1; early exit | 753 | no helper |
| Bit 9 = 0, bit 10 = 1; early exit | 747 | no helper |
| 9=0, 10=0, 11=1 | **154** | **SET_9ED20** |
| 9=0, 10=0, 11=0 | **9,632** | **CLEAR_9ED60** |
| **Total** | **17,788** | **9,786 helper calls** |

Reads: index 9 = 11,286 (AL1=753), index 10 = 10,533 (AL1=747), index 11 = 9,786 (AL1=154). All index 11 AL1 and all 154 SET helpers occurred **on P1 only** in this session; P2 had 4,196 CLEAR calls. CLEAR by side: P1=5,436, P2=4,196. One helper per relevant tick, exactly one native call path.

## Bit 11 input linked to Stage90 writes

Stage90 `F57D0`: 1,742 events (P1=808, P2=934), 705 changed snapshots vs 1,037 unchanged. Bit 11 (`0x800` in bitmap `P+3A0C`) P1 **10 rise, 10 fall**, P2 0/0. All 154 positive Stage91 bit11 reads have a same-player **last preceding** Stage90 post bitmap with bit 11 on (binary-search by QPC; no missing). The exact source of *every* underlying memory write cannot be proven merely from snapshots; the ten Stage90 up-transitions are observed.

**Concrete trace:**
1. `STAGE90_F57D0 seq=252` P1 `qpcAfter=2295930537268`: post=`0000000D,00000900,00000000`, `setMask=00000800`.
2. `seq=253` P1 `qpcAfter=2295930640005`: post still `00000900` bitmap word.
3. `STAGE91_F58E0 seq=4429/4430/4431` P1 QPC=2295930640069…0081: bit9 AL=0, bit10 AL=0, bit11 AL=1, exact predicted results.
4. `STAGE92_HELPER seq=3816` P1 QPC=2295930640083: `SET_9ED20 index=3`.
5. `STAGE92_TICK tick=3815` QPC=2295930640063…0087: flag `P+3B70 00000000 → 00000008` and bitmap `00000900 → 00000900`.

## Consequences of F5990 (observed state, not gameplay inference)

- `P+3B70` bit3 rose **127** ticks and fell **123** ticks. For SET helper ticks: **8** rising, **146** remained already set. For CLEAR helper ticks: **123** falling, **9,509** remained clear. For 8,002 ticks without helper: **119** rising, **7,883** remained set; no observed falling. These other rises prove **bit3 is not solely set via the captured 9ED20 callsite**. A next-stage investigation must locate alternative native bit3 writes/branches rather than claiming SET_9ED20 is the unique origin.
- 1,013 / 17,788 F5990 calls altered *some* bits of `P+3B70`; 16,775 did not. Bitmap DWORD sampled by Stage92 did not change in any F5990 tick.
- Stage92 all tick observations valid, all 9,786 helper-to-tick records matched actor/player, no ring overflow/drops/read failures.

## Same-session independent evidence

- Stage87 `EE2F0` 220 events, all `match=1`; Stage88 `EE306→F57D0` 220 matching QPC joins; Stage90 contains **all 220** stage87Seq references exactly once, no missing.
- Stage90 1,742 dispatches (type0=1,259, type1=183, type2=300); all health rows error-free. Stage91 31,605 native readers; Stage92 17,788 native F5990 call boundaries.
- `ArcanaAI_FinalCommandTrace.log`: **174 actual type-1 FINAL_COMMAND records** (P1=78, P2=96), all `payloadMatch=1, ediMatchesPlayer=1, localsReadable=1`; 43 unique ESI payloads. **55/174** final-command snapshots had P+3B70 bit3 on (P1=18, P2=37).
- `ArcanaAI_EventDispatchTrace.log`: **602 EVENT_DISPATCH_DETAIL** records (419 type0 + 183 type1), all `afterReadable=1`; these provide real downstream event traffic, **not** a proven one-to-one linkage of the bit3 transition to a specific final command. The FinalCommand log lacks QPC at callsite, so tick-level cooccurrence cannot establish causality.
- Stage89 four-site F58E0 observer installed; 3 site 0AAB55 reads with AL=0 in this run, and no reported health errors.

## Interpretation and next work

**Native state chain proven:** Stage90 F57D0 bitmap write/state -> Stage91 F58E0 bit9/10/11 reads -> Stage92 helper choice 9ED20 or 9ED60 -> Stage92 P+3B70 bit3 snapshot changes. It is **not** proven that this native flag directly chooses an attack, move, defense, or produces a specific FINAL_COMMAND payload. P+3B70 bit3 can rise even with no captured SET helper in the F5990 invocation.

**Stage93 recommendation:** read-only **QPC bridge at existing final-command boundary** (do not introduce synthetic events), plus precise disassembly-guided passive observations of alternative P+3B70 bit3 setters within F5990; connect actor/TID and QPC to concrete final command payloads and dispatcher rows. Do not use broad DR0/DR1 watchers while existing stage hooks may own those debug registers; Stage36 must remain OFF. Preserve safe Stage91 DLL and known-working r3; do not change gameplay state.

**Verification scope:** statistical correlation of uploaded runtime traces. No claims of visual action outcomes or Windows reproduction by assistant.
