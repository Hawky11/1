# Stage65 — runtime provenance VERIFIED (2026-11-11)

**Source:** the user's exact GitHub main upload commit [72f47bf63d3db14219a9007c8c18326162c6b400](https://github.com/Hawky11/1/commit/72f47bf63d3db14219a9007c8c18326162c6b400). Frozen unmodified on branch stage65-runtime-request-origin-evidence. User-owned original logs, same Windows x86 game session **2026-11-11 00:44:38–00:45:23**. Original Stage65 source remains on branch [stage65-request-origin-passive-source](https://github.com/Hawky11/1/tree/stage65-request-origin-passive-source) at 04ecb3370a66b4ed6dfd3d726df35f6f37546cc8.

## Build, installation and health

Windows MSVC x86 Build.log starts 00:44:06 and ends **BUILD SUCCEEDED**; DLL size **1,879,552 bytes**. Existing warning C4754 in AICommandSiteTrace.cpp remains; no compile/link errors. Runtime Stage65: **STAGE65_INSTALLED 1/1 RVA=E5BF0**, original native called once and no DR registers used.

Stage65 **429** writer windows (sequential seq 1..429; P1=162, P2=267). Same game TID **23432**, P1 player **0E4A1F78**, P2 player **0E4CCCA0**, objects **2A39D020 / 2A39DC50**. **All 429** have directE8=1, readMask=3F, and postRequest=request. 54 distinct original direct-CALL sites actually observed. Mode=0 in **427** records and mode=1 in **2**. All 26 Stage65 HEALTH samples had dropped=overflow=mismatch=readFail=0, and summed calls=429. Native EAX is not the request ID for every invocation; the actual postRequest is the correct write measure.

| Stage/log | Records | Health |
|---|---:|---|
| Stage40 original E8480 request consumer | **430** | installed 3/3 |
| Stage41 pending object | **430** | exact Stage40 seq/QPC/ID; full masks |
| Stage42 native activation | **318** | 31 HEALTH checks: dropped/overflow/mismatch=0 |
| Stage43 original activation caller | **318** | original AD75C source |
| Stage44 binding definition | **1,825** | HEALTH dropped/overflow/mismatch=0 |
| Stage39 selective callback action output | **61** | observed original callback subset |
| Stage47 native motion consumer | **6,208** | 26 HEALTH checks zero dropped/overflow/mismatch/badCaller |
| Stage49 linked motion | **6,270** | 26 HEALTH checks zero dropped/overflow/mismatch/badCaller |
| Stage56 ACDF0 writer | **440** | 19 HEALTH checks zero dropped/overflow/mismatch/readFail |
| Stage58 conditional helper | **942** | 19 HEALTH checks zero dropped/overflow/mismatch/badCaller |
| Stage59 DR0/DR1 X write watch | **18,783** seq 1..18783 | 28 HEALTH checks all zero dropped/readFail/ownershipConflicts/installFailures/teardownFailures/objectDrift |

Stage59 confirms WATCH_ARMED after objects became available at **00:44:52**, and 18,783 contiguous captured native X write events (P1 9,412; P2 9,371). No watcher ownership conflict or error was found in these logs. This is diagnostic stability, not proof of combat quality.

## Strict Stage65 → Stage40 → Stage41 native request handoff

For every Stage40 event choose the *latest earlier* Stage65 event with **identical player, side and native thread**, and Stage65.qpcAfter not greater than Stage40.qpcBefore. Require identical raw request ID (not an ID-only match). Independently require Stage41 same stage40Seq, same side/player/TID and identical QPC bounds/request.

**429/430 Stage40 events have a unique strict Stage65 writer**, and **all 429 Stage65 writer windows** have one unique downstream Stage40 consumer. **430/430 Stage40→Stage41 pairs** are exact. Matched Stage65→Stage40 QPC delays are **21 to 2,975,688 raw QPC ticks**, not milliseconds.

**One distinct unexplained Stage40 event:** seq **320**, P1, request **00000000**, qpcBefore **1440484223522**. Earlier Stage65 P1 seq303 was ID **00000304**, not zero. The next P1 Stage65 writer with ID zero happened **after** this consumer; it cannot be used as its cause. An unobserved alternative native writer, including previously identified E9E30/E9E60, is possible but **not proven**. Do not assert this is a lost AI action or definite E9E30 write.

## Stage42 activation and definition — 2 exact exceptions

All **318/318** Stage42 activations have matching original Stage43 caller **AD75C** and one uniquely enclosed Stage44 action-definition window with the same active ID, actor, TID and QPC.

The *latest preceding Stage40 consumed ID* matches the newly activated Stage42 ID in **316/318** windows. Of these, **315** have a distinct Stage65 writer; the other corresponds to the exceptional Stage40 seq320.

Two Stage42 windows fail this strict latest-consumer test; **do not join an older same-ID request to make the numbers 318/318**:

| Stage42 seq | Side | Latest Stage40 seq | Latest Stage40 ID / Stage41 POST O+64 | Stage42 PRE O+64 / POST O+60 |
|---|---|---|---|---|
| **39** | P1 | **38** | 0000002F | **00000025** |
| **155** | P1 | **204** | 0000002F | **00000025** |

The pending native O+64 value is different by the time native activation begins. This demonstrates an **unobserved intervening state change**, but not which original instruction wrote it, nor that any error occurred. Investigate native O+64 writers statically before adding new hooks or changing behavior.

## 114 repeated requests localized to one original CALL site

Of 430 Stage40 request-consumption events, **114** do not map to a distinct Stage42 activation. **Every one** has Stage65 original callerCallRVA **00153ECA**, and the original Stage41 shows O+08 flags **unchanged**, pending O+64 **unchanged**, and pending O+64 equal to active O+60 and request ID. ID distribution: **0x49: 96**, **0x4A: 12**, **0x48: 6**.

Original CALL RVA **00153ECA** executed **118** times total: **114** already-current duplicates and **4** distinct activations. Original SHA-pinned disassembly shows that this native call invokes E5BF0 with mode0 and an ID derived from preceding native values and a global at preferred VA **008255A0**. No tactical COM scoring intent is inferred.

## Native original caller attribution

All 429 Stage65 requests have **54 distinct real direct E8 original caller RVAs**, out of 296 statically possible calls mapped in Stage64. Most frequent:

| Original caller RVA | Recorded calls | Matching separate Stage42 activations |
|---|---:|---:|
| **00153ECA** | 118 | 4 |
| **00133569** | 31 | 31 |
| **001BD65A** | 25 | 25 |
| **00158691** | 23 | 23 (all request ID zero) |
| **00133173** | 19 | 19 |
| **00131CC5** | 16 | 16 |
| **00156BB6** | 15 | 15 |
| **001587D1** | 14 | 14 |
| **0015867C** | 14 | 14 |
| **00131D1C** | 13 | 13 |

Stage64 native callback helper calls **131CC5** and **131D1C** are now confirmed executed in a real match (16 and 13 calls respectively). This is request-source provenance; **NOT** proven autonomous COM evaluation, actual animation, hit, damage, AI improvement or strategic intent.

**61/61 Stage39 selective callback action-output events** join a unique later Stage40 and native Stage42 activation. Callback RVA distribution: 12EB10=43, 1DA490=9, 123AF0=9. The remaining **257/318 Stage42 activations** do not have Stage39 output records in this run: **254** nonetheless have Stage65→40→42 evidence, **1** has Stage40 without a Stage65 writer, and **2** are the 0x25/0x2F exceptions above. This is an observer-scope difference, not a bug count.

## Next research task — Stage66

1. Inspect original **O+64 pending-ID write sites** and the control path between Stage41 seq38/204 and Stage42 seq39/155. Precisely explain the recurring 0x2F → 0x25 transition before treating it as an AI decision problem.
2. Inspect original alternative **P+3F4C writer E9E30/E9E60** and any alias writers to explain Stage40 seq320 ID0; do not claim responsibility without a matching event.
3. Concentrate on the **54 real original Stage65 caller RVAs**, especially 153ECA, 133569, 1BD65A, 158691, 131CC5/131D1C, rather than all 296 static possibilities. Follow native call graph toward actual candidate/utility selection; keep engine action transmission distinct from COM strategy.
4. Do not modify action IDs, AI weights, coordinate writers, combat timing or outcomes. Keep successful **1,879,552-byte Stage65 test DLL** for comparison and earlier verified **1,153,024-byte Stage59 r2 DLL** for rollback.

A [read-only Python correlator](tools/correlate_stage65.py) was added for local reproduction. All numerical runtime results above were independently computed directly from the current GitHub log texts; the Python script was not separately run on the user's Windows installation.
