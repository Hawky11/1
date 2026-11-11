# ArcanaAI Stage81 r2 — same-run Windows runtime analysis

Date: **2026-11-11**. Evidence: [exact commit c003261e](https://github.com/Hawky11/1/commit/c003261e9e63e817c92bd5e58fd4afa35ee34e52) containing four logs from the 15:41–15:43 test session. This is an evidence report; **no AI behavior modifications and no demonstrated AI improvement**.

## Installation and build

- `Build.log`: MSVC x86 BUILD SUCCEEDED; `build\dinput8.dll` **1,934,336 bytes**. One already-present C4754 warning (AICommandSiteTrace.cpp:785); no build failures.
- `Stage79`: `STAGE79_INSTALLED 1/1`; native `0EFFE8→0E89E0`; 5,628 call windows.
- `Stage80`: `STAGE80_GUARD ... nativeOK=1`; `STAGE80_INSTALLED 1/1`; native `0E8A8E→0F7150` called exactly once per instrumented invocation; 5,185 windows.
- `Stage81 r2`: `STAGE81_WAIT_STAGE80`, then `STAGE81_PREREQ_STAGE80_OK` followed by `STAGE81_INSTALLED 3/3`; original MOV execution (if reached) is preserved. Hook addresses `0F7D22`, `0F7D38`, `0F7D43`. Each stage reports `DRwrites=0`.
- Every recorded observer HEALTH sample has zero `dropped`, `overflow`, `mismatch`, `readFail`, and Stage81 `pendingOverflow`.

## Cross-stage correlation

**All 5,185 Stage80 windows have one and only one matching enclosing Stage79 window** with matching side, actor P, object O, game thread ID 17220, and `qpcBefore79 ≤ qpcBefore80 ≤ qpcAfter80 ≤ qpcAfter79`.

Exactly four matching `P+3F50` transitions were observed in both Stage79 and Stage80:

| Side | Stage79 seq | Stage80 seq | Selector P+3F50 | Stage80 QPC before–after |
|---|---:|---:|---|---|
| P2 | 1088 | 803 | 0→1 | 1978958203679–1978958203724 |
| P1 | 2743 | 2401 | 0→2 | 1979042613911–1979042613945 |
| P1 | 3521 | 3156 | 2→1 | 1979083070622–1979083070668 |
| P2 | 5052 | 4622 | 0→2 | 1979161173562–1979161173591 |

Stage79 changedMask `0041` and Stage80 changedMask `081` on all four events: selector changed with another tracked field; original raw PRE/POST values agree exactly.

Stage81 r2 HEALTH: **callsEdx=0, callsEcx=0, callsZero=0** during the same capture; zero `STAGE81_NATIVE_WRITE` event lines. Installation alone does **not** establish those EIPs were executed.

## Interpretation / limits

The same-player and same-QPC nested boundaries assign the four selector changes to original native `F7150` execution intervals. They do not identify a writer EIP or prove why three direct native MOV candidates saw zero events. The cause may be alternative/nested code, branch reachability, or another tracing assumption. Do not label any particular MOV executed without direct EIP proof.

## Next step: Stage82

Perform original-binary static writer and callee analysis under F7150 (including indirect calls and aliasing). Design guarded, source-only passive trace at a discriminating boundary or verified write site; keep the current DLL as rollback baseline. Do not change COM AI behavior or write gameplay data. Raw Stage79/80/81 logs and Build.log remain in repository root; Stage81 r1 failing logs preserved in `r1_failed/`.

## Historical note

This latest run was committed to the long-lived `stage80-e8a8e-late-f7150-boundary-source` branch; the documentation/main reconciliation merges the branch without rewriting its original evidence commit.
