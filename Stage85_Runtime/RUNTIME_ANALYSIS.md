# Stage85 r1 — runtime verification of F7150 post-F3930 join (2026-11-11)

**Status: WINDOWS X86 BUILD SUCCEEDED; STAGE85 INSTALLED 1/1; 109/109 UNIQUE SAME-RUN QPC JOINS. THE RARE UNATTRIBUTED P2 1→2 PATH DID NOT RECUR.**

This is read-only diagnostics, **not** a demonstrated gameplay/COM AI improvement. Retain the Stage84 positive rare-route evidence separately; do not join timestamps/QPC across runs.

## Immutable evidence and build

- Source: [user upload 428b6b27895b1b1fb3015ab225169370e541050c](https://github.com/Hawky11/1/commit/428b6b27895b1b1fb3015ab225169370e541050c), root Build.log and Stage79/80/81/82/84/85 logs from the *same* 2026-11-11 local 18:03:50–18:04:53 test session (Build started 18:03:23).
- Windows MSVC x86: **BUILD SUCCEEDED**, DLL **1,957,376 bytes**, existing C4754 warning at AICommandSiteTrace.cpp:785; Stage85 is thus runtime-tested, not merely source-ready.
- Stage85 signature: `0F7C68` 10-byte `837DF8000F85E0000000` `exact=1`; `STAGE85_INSTALLED 1/1`, original native `CMP/JNE` replay once, fallthrough `F7C72`, taken `F7D52`.
- Stage79/80/81/82/84 retained; Stage81 3/3 installed but direct F7150 selector writer hits remained zero; Stage84 6/6 installed.
- Original privately held `AALib.exe` SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` (already statically checked when Stage85 source prepared).
- Keep `EnableStage36ControlToCommandTrace=0`: Stage36 and Stage82 compete for the same F79E4 CALL.

## Same-session event counts and correlation

Correlate on identical side, player address, object address, TID, **integer QPC** and unique parent interval. Only this one 18:04 test run was considered.

| Event | Count | Strict unique parent |
| --- | ---: | --- |
| Stage79 COMMON_WINDOW | 8,598 | — |
| Stage80 LATE_F7150 | 7,830 | **7,830 / 7,830** inside Stage79 |
| Stage82 NESTED | 2,100 | **2,100 / 2,100** inside Stage80 |
| Stage84 EXACT_WRITE | 12 | **12 / 12** inside Stage82 |
| Stage85 JOIN at `0F7C68` | 109 | **109 / 109** inside Stage80 |

**All** sampled HEALTH error counters remained zero for Stages79, 80, 81, 82, 84 and 85 (no reported drops, overflow, mismatch or read failures). This confirms logged observer health, not proof that instrumentation can never perturb timing.

For each of 109 Stage85 JOINs, selector equals both Stage80 PRE and POST. In **103** cases, a preceding Stage82 nested interval is present inside the *same* Stage80 interval, and the Stage85 selector also equals that nested call's POST; those preceding sites are F3930 (83) and F4360 (20). In the other **6**, no preceding Stage82 interval in that Stage80 call is logged, so no F3930 comparison is possible. The join is a conditional path and does **not** run in every Stage80 invocation.

## Selector net transitions in this run

Seven Stage80 `P+3F50` net changes occurred and **all seven are uniquely explained by an exact Stage84 F3930 store**, with no Stage85 JOIN in their Stage80 windows:

| Stage80 seq | Side | Selector | Stage82 site/seq | Stage84 exact writer/seq |
| ---: | --- | --- | --- | --- |
| 10 | P2 | 0→1 | F3930 / 10 | F3F69 / 1 |
| 1716 | P2 | 1→0 | F3930 / 546 | F3F71 / 4 |
| 1955 | P2 | 0→2 | F3930 / 614 | F3F50 / 5 |
| 2063 | P2 | 2→0 | F3930 / 690 | F3F71 / 6 |
| 4198 | P2 | 0→1 | F3930 / 1249 | F3F50 / 9 |
| 5096 | P2 | 0→1 | F3930 / 1448 | F3F69 / 11 |
| 5705 | P1 | 0→1 | F3930 / 1707 | F3F69 / 12 |

Stage84 12 observed exact writes consist of **7 changes and 5 idempotent writes**. Stage85 itself has 109 JOINs, all non-changing compared with the encompassing Stage80 boundary values. There is **no** net `1→2` selector change in this test, so Stage85 did not bisect the formerly unexplained case.

## Preserve earlier Stage84 evidence

The separate [Stage84 long-run report](../Stage84_Runtime/RUNTIME_ANALYSIS.md) from [commit 364429c](https://github.com/Hawky11/1/commit/364429c76dbfbfecd7c1679dd3e4dacd90e4897c) contains **16** Stage80 net transitions, **15** attributed to F3930 exact writers, and an unmatched **P2 1→2** (Stage80 seq2160; nested F3930 seq513 with selector 1→1 and companion `P+3F54` 1→2). That event falls after F3930 returned and before F7150 returned; the precise writer EIP is still unknown.

The original SHA-pinned F7150 disassembly confirms direct selector MOVs at `F7D22/F7D38/F7D43`, but Stage81 had zero executions in both captures. They cannot be blamed for the rare transition without an actual matching hit. Indirect/aliased writes and intervening helper calls remain candidate explanations.

## Next diagnostic action — capture rare path, *same Stage85 build*

1. **Preserve** current proven 1,957,376-byte Stage85 DLL, previous Stage84 DLL 1,952,768 bytes and existing immutable logs.
2. Reuse Stage85 r1 (not a speculative Stage86 hook); perform a **longer** ordinary COM-vs-COM capture until at least one Stage80 net transition without a Stage84 exact writer occurs, if reasonably obtainable.
3. Upload *same-session* Build.log plus Stage79/80/81/82/84/85 logs. Keep `EnableStage36ControlToCommandTrace=0`, install markers valid and all HEALTH errors zero.
4. When P2 1→2 occurs again, match only the same P/object/TID/QPC Stage80 and nested F3930 call. If a Stage85 join exists, compare F3930 POST vs Stage85 selector vs Stage80 POST to delimit before-join vs after-join. If no join exists, this branch bypassed the checkpoint and needs a different passive point based on the **new positive window**, not a guess.
5. Only then consider Stage86 with a precise SHA-pinned instruction/ABI audit of the narrowed helper or dispatch segment. Do not modify original AI selection or skip strict live-byte guards.

**Nothing in Stage85 proves the exact writer of the previously unmatched transition or any AI quality improvement.**
