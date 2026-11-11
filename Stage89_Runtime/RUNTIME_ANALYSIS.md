# Stage89 — live audit, all 4 reader hooks installed, one site active (2026-11-11)

**Status:** Windows x86 **BUILD SUCCEEDED**, DLL **1,984,000 bytes**, all four exact native CALL-site hooks **INSTALLED 4/4**, but **only AAB55 executed**. Four passive reader observations, all `AL=0`. No observed same-event immediate join with Stage88 output. Existing nonfatal C4754 persists.

**Raw evidence:** user-upload [`7475572`](https://github.com/Hawky11/1/commit/7475572ea25d04edb01f383367af83ceb2ad7711), root same-run logs `Build.log`, `ArcanaAI_Stage89F58E0ReaderTrace.log`, `ArcanaAI_Stage88Stage24JoinTrace.log`, `ArcanaAI_Stage87SelectorConsumerTrace.log`, `ArcanaAI_Stage24ResolverConsumerTrace.log`, `ArcanaAI_FinalCommandTrace.log`, and `ArcanaAI_EventDispatchTrace.log`. Stage89 source: [`7ede68b`](https://github.com/Hawky11/1/commit/7ede68b825faadea5caed2a59b9afa4881cc24a0).

## Native CALL-site reader coverage

Stage89 install row: `STAGE89_INSTALLED mask=F sites=4/4 rvas=09A870,09AA20,09AB00,0AAB55 nativeTarget=0F58E0 originalOnce=1 noNewEE306=1 DRwrites=0 gameWrites=0`. This verifies *installation* of all sites, not execution of all sites.

| CALL site | Recorded calls | AL=0 | AL=1 |
|---|---:|---:|---:|
| `9A870` | 0 | 0 | 0 |
| `9AA20` | 0 | 0 | 0 |
| `9AB00` | 0 | 0 | 0 |
| `AAB55` | 4 | 4 | 0 |
| **Total** | **4** | **4** | **0** |

`AAB55` emitted **P1 3**, **P2 1**, common TID **5480**, full pre/post masks **7/7**, `predictionValid=1` and `predictionMatch=1` for all four. Native return `EAX=0`, `AL=0`, flags `0x246`. No bitmap triple changes inside `F58E0` in these four samples.

| Stage89 seq | Side | Pre/post bitmap `(P+3A08, P+3A0C, P+3A10)` | AL |
|---|---|---|---:|
| 1 | P1 | `00000000,00000001,00000000` | 0 |
| 2 | P1 | `00000000,00000001,00000000` | 0 |
| 3 | P1 | `00000006,00001002,00000000` | 0 |
| 4 | P2 | `00000018,00000001,00000008` | 0 |

All 4 snapshots satisfy `P+3A0C & 0xE00 == 0`, plus generic bit38 `P+3A10 & 0x40 == 0`. Thus `AL=0` correctly follows the native `F58E0` predicate. `nextBranchPredictedTaken=0` at AAB55 is a **prediction** for its JNE, not an instrumented E8560 call. AAB55's zero branch can proceed toward E8560 but does not demonstrate that E8560 executed.

Stage89 HEALTH summed over all 51 intervals: `calls0=0 calls1=0 calls2=0 calls3=4 events=4 filtered=0 overflow=0 dropped=0 mismatch=0 readFail=0`.

## Same-run Stage87→Stage88: new decisive selector=2 runtime confirmation

**170/170 exact one-to-one Stage87 seq → Stage88 stage87Seq** with same player, side, TID, exact QPC and native selector mapping; all necessary masks present. Stage87 and Stage88 each saw 1,614 attempted calls, 1,444 normal filtered and 170 events. All `stale/mismatch/readFail/dropped=0` in the respective HEALTH rows.

| P+3F50 | F57D0 arg | P+3A0C target bit | Matched calls | Actual target bit 0→1 | Already set |
|---|---|---|---:|---:|---:|
| 0 | `0x0E` | `0x200` | 81 | 4 | 77 |
| 1 | `0x0F` | `0x400` | 62 | 0 | 62 |
| 2 | `0x10` | `0x800` | **27** | **1** | 26 |
| Total | | | **170** | **5** | **165** |

**First full runtime proof for selector 2:** Stage88 **seq=31**, P1, `P+3A08,0C,10` `(0x29,0,0x10) → (0x10,0x800,0)`. Of 170 events, **54** changed at least one of these three DWORDs and 116 were byte-identical before/after. Dispatch type/payload counts: 166 `type=0,payload=0x22`; two `type=0,payload=0x23`; two `type=2,payload=0x02`. Do not carry over Stage88 prior-run absence of selector2 as a current limitation.

## Relationship between Stage88 writes and Stage89 reader calls

Stage89 and Stage88 logs share exact process P pointers and TID 5480 and form a QPC-ordered same-session capture. **However, no Stage89 sampled a target bit while it was set.** Nearest earlier Stage88 F57D0 QPC gaps for same P and TID: Stage89 seq1 **none**, seq2 **51,066,255 ticks** (earlier Stage88 seq60, selector2), seq3 **12,718,636 ticks** (earlier Stage88 seq95, selector1), seq4 **36,346,291 ticks** (earlier Stage88 seq170, selector0). At each later reader call, the relevant special bits are absent. These are **not** nested or immediate causal joins. The earlier controller bitmap must have changed before the sampled reads, or the observations belong to separate state phases; this trace does **not** locate the clearing writer.

This means **the four observed AAB55 AL=0 reads do not prove an active Stage88 selector was consumed to emit an AI command**. The remaining three readers have zero hits in this run. The separately logged FinalCommand type-1 observations (167 records) do not confer same-event attribution.

## Build and health notes

- Windows x86 `Build.log`: `BUILD SUCCEEDED. DLL size: 1984000 bytes.` One earlier compiler warning `C4754` remains at `AICommandSiteTrace.cpp(785)`.
- Stage24 legacy callsite `EE306 → F57D0` remains installed (4 type-1 payload=0x1E rows), no added EE306 hook; Stage87 installed 1/1, Stage88 READY and 170 completed.
- No explicit `STAGE89_ERROR`, overflow or read error observed; this does **not** establish that all possible hooks are crash-free or that the AI plays better.

## Next: Stage90 investigation

1. **Do not simply rerun the same AAB55-only scenario expecting AL=1**; native predicate got 0 because its bits were absent at each observed read.
2. Map **actual readers and clear/reset writers** of the three output DWORDs, especially `P+3A0C`, during the QPC intervals *after* Stage88 F57D0 and *before* Stage89 AAB55. Determine whether a different fast-path consumer reads active bits before they are cleared.
3. Source-only/static first: exact RVA and original-byte guard for a noncolliding, read-only targeted observation if evidence justifies it; retain Stage89 working DLL backup **1,984,000 bytes**, Stage36 disabled, no global F58E0 detour, no second EE306 hook, no synthetic inputs/game data mutation.
4. Capture same-run source→consumer→queue identity before making a behavior/attack claim.

[Machine-readable results](results.json). Runtime uploaded at `7475572`, not conflated with previous Stage88 capture `ccbbf6a`.
