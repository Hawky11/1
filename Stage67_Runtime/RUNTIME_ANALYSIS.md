# Stage67 — native promotion and independent zero writer RUNTIME VERIFIED

2026-11-11. Exact raw upload: `71e6462f6a7364451372a88d1961f51137986793`.
Source publication: [4fa25d90](https://github.com/Hawky11/1/commit/4fa25d902a8ec8aa1f6273791188f3603350b33f), branch `stage67-pending-promotion-zero-write-probes`. Its Git tree is exactly `b8643d57c7bb22507af529ca7ab5b7de57cf88c1`, matching the previously delivered local source commit 1723e59; publication through GitHub generated a different commit ID, not different source contents.

Evidence branch `stage67-runtime-native-paths-evidence` starts at the user's raw upload and preserves every uploaded log and ArcanaAI.ini byte-for-byte. Source overlays are included for reference; the available upload does not contain the complete local compiled source tree or DLL, so byte-for-byte compiled binary provenance is unavailable.

## Build, installation and session

User's Build.log: **11.11.2026 01:29:50**, MSVC 14.51.36231 / Visual Studio 18 Community, x86, **BUILD SUCCEEDED; DLL 1,887,232 bytes**. Only the existing C4754 warning at AICommandSiteTrace.cpp:785 is present. No analyst rebuild.

All audited log initialization headers are **2026-11-11 01:30:28**. Stage67 marker r1 and **STAGE67_INSTALLED 2/2** present. Runtime rows captured 01:30:40–01:31:04, game thread **10788**. P1 player/object `0E4A1F78 / 2A393020`; P2 `0E4CCCA0 / 2A393C50`. These differ from the older 00:44 session; none of its rows were mixed in.

| Observer | Actual rows | Integrity |
|---|---:|---|
| Stage67 | 4,586 | unique consecutive sequence set; 4,584 kind=0, 2 kind=1 |
| Stage40 / Stage41 | 280 / 280 | all matched by seq, actor, QPC, request and object |
| Stage42 / Stage43 | 220 / 220 | all matched by activation seq, actor/object and QPC |
| Stage65 | 278 | full 3F read masks |
| Stage59 | 14,040 | unique sequence set 1..14040, no missing/duplicate IDs |

All Stage67 masks are full: kind=0 **1F/1F**, kind=1 **07/07**. All Stage41/42 masks are 1F/1F. Stage67 has 20 HEALTH samples, **42,001 promotionCalls − 37,417 otherObjects = 4,584 P1/P2 windows**, writerCalls=2, otherPlayers=0; dropped/overflow/mismatch/readFail all zero. Stage65 19 HEALTH samples sum calls=278, errors zero. Stage42 27 HEALTH samples sum calls=1684, otherObjects=1464, leaving 220 recorded activations, errors zero.

Stage59 WATCH_ARMED and 61 thread-install records present; 22 HEALTH samples have zero dropped/readFail/ownershipConflicts/installFailures/teardownFailures/objectDrift. Final cumulative totalWrites=totalSeq=14,040, ownedThreads=60. Do not sum cumulative totalWrites across HEALTH samples. There is **one file-order sequence reversal (8219→8052)**, but no missing or duplicate event; use sequence/QPC, not text row order. This is not evidence of a dropped write.

## A. Original deferred promotion 2F→25 confirmed

**Stage67 seq4436**, P1, O=`2A393020`, TID=10788, callerReturn=`008AD761` (runtime base 00800000 + AD761). Outer QPC window **1467959966659–1467959966687**.

| Native field | Before 1E9F40 | After 1E9F40 |
|---|---|---|
| O+30 low byte | 01 | 00 |
| O+64 pending | 0000002F | 00000025 |
| O+68 deferred | 00000025 | FFFFFFFF |
| O+60 active | 0000002F | 00000025 |
| O+08 flags | 00000000 | 00000000 |

Inside this window, **Stage42 seq218** QPC **1467959966661–1467959966683** sees pending **25** and deferred **FFFFFFFF** already at PRE; O+08 bit0 is **1** at inner PRE and becomes 0 at POST, active **2F→25**. Thus the outer snapshots having bit0=0 at both ends do not mean the native pending bit was never set.

The exact Stage66 original code gates are now satisfied by observed entry state, and the nested PRE/POST values verify their effects: original 1E9F4E promotes O+68 into O+64, 1E9F51 clears deferred, then activation consumes the promoted value. This is the sought native mechanism, not a logger gap or injected replacement ID.

Latest preceding Stage40 **seq278** requested 2F at QPC 1467958862984–1467958862991. Stage41 seq278 explicitly shows pending **22→2F** while deferred **25→25**; its source Stage65 **seq276**, caller **156769**, wrote 2F at 1467958862442–1467958862446. Therefore deferred 25 was already present before this latest request; that Stage65 2F write is not the origin of deferred 25.

Only one armed/nonempty-deferred promotion appears in this capture. It accounts for the sole latest-request/activation-ID mismatch (Stage40 278 vs Stage42 218); other 219 activations agree with the latest request ID. **All 220/220 Stage42 activations are uniquely contained in same-actor/object Stage67 kind=0 windows**, with matching final active IDs. Native AL=1 occurs exactly for those 220 parent windows; the remaining 4,364 return AL=0 without a separate Stage42 window.

This confirms the mechanism in the new run. It does **not retroactively observe** the old Stage42 seq39/155 entry states, which were absent from their logs.

## B. E9E60 actually supplies both zero requests missed by Stage65

| Side | Stage67 store seq | Store QPC before→after | Stage40 consumer | QPC gap after store | Stage42 activation |
|---|---:|---|---:|---:|---:|
| P2 | 911 | 1467783633192→1467783633193 | 47 | 442 | 47 |
| P1 | 1886 | 1467832398042→1467832398043 | 88 | 493 | 88 |

Both exact MOV events have EDX/writeValue=0, request **FFFFFFFF→00000000**, runtime source global=0, P+3B7C=0, full masks. Both following Stage40 consumers see request 0 and clear it to FFFFFFFF; their Stage41 commit and Stage42 activation produce active ID0. Identity and QPC order agree. No intermediate observed Stage65/Stage67 request write or Stage40 consumption occurs on the respective actor between store and consumer.

Immediately later zero requests are **separate**: Stage65 seq47 supplies Stage40 seq48 (P2), and Stage65 seq87 supplies Stage40 seq89 (P1). They occur too late to explain Stage40 seq47/88. The correlator treats each consumed request as a distinct event and does not reuse one zero writer for both consumers.

Across the whole capture, **278 unique Stage65 writers + 2 unique Stage67 E9E60 writers explain 280/280 Stage40 consumers** through the latest preceding observed request-changing event. There are no unmatched consumers in this observed chain. Unknown uninstrumented writers are not globally ruled out; this does not establish AI choice semantics.

The old Stage40 seq320 is from another run: its origin remains a compatible historical inference, not a newly captured instruction event.

## Stage43 expected raw return behavior

All 220 Stage43 rows have nativeReturnRVA=001E9F67 and raw callerReturnRVA=4FDC3FFD / knownCaller=0, as predicted for the inserted wrapper. Do not interpret this as 220 unknown native callers. Every parent Stage67 record has callerReturn=008AD761, and strict actor/object/QPC containment restores provenance to AD75C without fabricating Stage43 addresses.

## Reproduction and next step

Run `python Stage67_Runtime/tools/correlate_stage67.py .` at the evidence branch root. [results.json](results.json) contains the audit, complete three target cases and activation-parent map. The parser rejects concatenated/mixed initialization sessions, actor mismatches, sequence gaps and nonzero health errors. Four deliberately corrupted-input checks (session, actor, dropped counter, missing sequence) were rejected.

**Stage68 next: static-first origin of O+68=25 and O+30 arming.** Inspect original writers and their native callsites, then compare existing snapshots to bracket first appearance. Do not add hooks or change COM behavior until the source path is narrowed. The pending→active handoff and separate zero-request writer are established for this capture; the higher-level reason for selecting/defering ID25 remains unproven.

**No new build/test/log upload needed now.** Preserve the successful Stage67 DLL **1,887,232 bytes**, Stage65 **1,879,552 bytes** and Stage59 r2 **1,153,024 bytes** for rollback. No gameplay logic changes have been made or AI improvements claimed.
