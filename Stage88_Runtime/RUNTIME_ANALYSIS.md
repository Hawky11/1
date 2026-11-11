# Stage88 — same-session runtime audit (2026-11-11)

**Verdict: Stage88 runtime PASS for selector states 0 and 1.** The source-only Stage88 package was Windows MSVC x86-built and observed in game; its log's literal `SOURCE_ONLY` build marker is inherited from source naming, **not** a claim that runtime was absent.

Evidence: user upload [`ccbbf6a`](https://github.com/Hawky11/1/commit/ccbbf6a41cd7ed5ba0fdf194f557a4bbf112b3ca), based on Stage88 source-only [`eb92db8`](https://github.com/Hawky11/1/commit/eb92db82bae6afb155a58bef8e8429ef3abf7af3). Full raw files in repository root at uploaded commit: `Build.log`, `ArcanaAI_Stage87SelectorConsumerTrace.log`, `ArcanaAI_Stage88Stage24JoinTrace.log`, `ArcanaAI_Stage24ResolverConsumerTrace.log`, `ArcanaAI_FinalCommandTrace.log`, `ArcanaAI_EventDispatchTrace.log`. Do not mix these QPC records with the preceding Stage87 run.

## Build, installation and integrity

- `Build.log`: **BUILD SUCCEEDED**, `dinput8.dll` = **1,975,296 bytes**, non-fatal familiar **C4754** at `AICommandSiteTrace.cpp:785`.
- Stage87: `STAGE87_SITE_AUDIT exact=1`, `STAGE87_INSTALLED 1/1` at `EE2F0`; original MOV replayed once.
- Stage24: installer reports observing existing `EE306 → F57D0` callsite with original callee executed once; five separate legacy `type=1,payload=0x1E` records logged.
- Stage88: `STAGE88_READY`; **no second hook at EE306**, and log explicitly reports zero game-data/debug-register writes. Worker-only output. This verifies functioning integration, not a new independent hook-install marker.
- Across Stage87/88 HEALTH summaries: **1,482 entry calls**, **1,307 deliberately filtered**, **175 matched and completed**; `stale=0, mismatch=0, readFail=0, dropped=0` (Stage87 also `mismatch=0`, `readFail=0`, `dropped=0`).
- No explicit ERROR/WARNING in the five runtime trace files. Compiler C4754 is the only observed warning in Build.log.

## One-to-one QPC correlation

All **175 Stage87 seq IDs (1..175)** have exactly **one** Stage88 `stage87Seq` and no unpaired IDs. Checked same P1/P2, player pointer, TID, exact `qpc87` equality, ordered `qpcBefore/qpcAfter`, matching selector→mapped EAX→native `argSelector`, `match=1`, full read masks (`Stage87=F`, `Stage88 pre/post=7`, `dispatchMask=3`, `tableMask=1`). QPC Stage87→Stage88 entry gaps: **1–225 ticks**; F57D0 sampled pre→post gaps: **1–10 ticks**. Both P1 (80) and P2 (95) represented.

| P+3F50 selector | EE2F0 mapped code | F57D0 P+3A0C target bit | Stage88 events | 0→1 bit transition | Bit already 1 |
|---|---|---|---:|---:|---:|
| 0 | 0x0E | 0x200 | 110 | 12 | 98 |
| 1 | 0x0F | 0x400 | 65 | 1 | 64 |
| 2 | 0x10 | 0x800 | **0** | **not tested** | — |
| **Total** | | | **175** | **13** | **162** |

All 175 F57D0 events show descriptor DWORD `01FF2601` (first three bytes `01 26 FF`), post `P+3A08=0x0E` or `0x0F` matching the code, and the intended `P+3A0C` bit set. **58/175** changed one or more of the three observed DWORDs, and **117/175** were identical before/after. **13** cases are specifically target-bit 0→1; do not mistake the other 45 state changes for newly set target bits.

Stage88's dispatcher snapshot shows **171** `type=0,payload=0x22`, **2** `type=0,payload=0x23`, and **2** `type=2,payload=0x04` (not all `type=1` as the earlier legacy Stage24 filter). The 143 `FINAL_COMMAND` records (P1=60/P2=83) are **type-1 queue callsite** observations, so their mere presence does not prove same-event identity or gameplay execution for Stage88's predominantly type-0 events. EventDispatch contains 423 `EVENT_DISPATCH_DETAIL` records, but they are not a complete instrumentation of every Stage88 type/payload.

## What is established and what is not

**Established:** native `P+3F50 → (0x0E/0x0F) → EE306/F57D0 → P+3A08 and P+3A0C` for the 175 observed same-run events, including 13 actual target-bit sets. No diagnostic errors in those rows.

**Not established:** selector=2 → 0x10 → 0x800 **at runtime in this run** (Stage87's earlier different run did sample selector=2); any specific rendered move, hit, animation, or causal link from Stage88 output to FinalCommand's separate type-1 producer.

## Next — Stage89 scope

1. Preserve tested Stage88 DLL (1,975,296 bytes), source branch, and the immutable user-upload commit.
2. Perform SHA-pinned static study of real **readers** of `P+3A08, P+3A0C, P+3A10` (including known F58E0 predicate branches and command candidates), differentiate writer vs reader and identify one exact executable read boundary.
3. If a safe non-colliding point exists, implement a narrow **read-only**, signature-guarded consumer trace with exact same-thread/QPC attribution; do not install a second EE306 hook and do not enable colliding Stage36 instrumentation. Test in Windows x86 COM-vs-COM before making any downstream action claim.
4. Seek a natural selector=2 sample in a future run. **Do not synthesize selectors, inputs or events.**

See [machine-readable results](results.json). Historical source/evidence branches remain untouched.
