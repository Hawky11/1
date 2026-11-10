# Stage42 — Windows x86 runtime activation VERIFIED (2026-11-10)

Evidence origin: user uploaded fresh complete root log set in [main commit a1096e9](https://github.com/Hawky11/1/commit/a1096e980b6d40c290741693b6e8bc9116af01c8) at 06:32:28 UTC / 09:32:28 local. This evidence branch was created at that exact commit, so original user-uploaded Build.log and all runtime log Git blobs at repo root are preserved **without modification**. Previous Stage41, Stage40 and earlier source/evidence branches remain untouched. Full 356-line data join: [stage41_to_stage42_activation_join.csv](stage41_to_stage42_activation_join.csv).

## Build, install and health

Build.log started 10.11.2026 09:30:43 local; Windows MSVC x86 BUILD SUCCEEDED; dinput8.dll 810496 bytes; only pre-existing C4754 warning in AICommandSiteTrace.cpp, no compilation/link errors. ArcanaAI_Stage42ActionActivationTrace.log has STAGE42_ACTION_ACTIVATION_BUILD=2026-11-10-r1 followed by STAGE42_INSTALLED 1/1 callsite=1E9F62 native=1E9C70 original called once. Working Stage40 r2 INSTALLED 3/3 and Stage41 r1 ENABLED in corresponding logs. 356 contiguous Stage42 seq=1..356 with full pre/post 5-bit read masks=1F, stable P1/P2 object pointers, monotonic QPC, no explicit ERROR/WARNING.

36 STAGE42_HEALTH messages over the whole captured interval: sum calls=3492, otherObjects=3136, accounting for exactly 356 captured P1/P2 activations. Dropped=0, overflow=0, mismatch=0 in all 36 health messages. No missing sequence numbers. The monitored callsite handles many native objects; unrelated object calls were counted, not logged as P1/P2.

## Native O+64 -> O+60 activation proved in 356/356 windows

P1: 154, P2: 202; 94 distinct active IDs. All 356 show O+08 bit0 1->0 and O+60 POST == O+64 PRE. Native O+64 did not change during the activation window, and original O+60 actually changed in 354 cases (P1 152, P2 202); the other 2 already held the desired ID. DWORD O+30 changed in 159; O+68 changed in none. The static disassembly prediction E9C70 clears pending bit and promotes pending ID has now been confirmed under real COM-vs-COM runtime for both players. This is **native active command state**, not evidence of successful animation, hitbox, damage or AI gameplay quality.

## Earlier Stage41 / Stage40 / Stage39 correlation in SAME run

Stage40 426 continuous consumer windows; Stage41 426 continuous native object-snapshot windows; full matching by stage40Seq, side, player, tid, raw request, both QPCs, object pointer. Masks 1F/1F on all Stage41, stable object before/after. Stage41 branch classification: 350 NEW/REQUEUE, all 350 result in O+64 POST=raw ID, O+08 bit0 POST=1, O+30 lowbyte POST=0. 76 CURRENT/DUPLICATE, all 76 meet the predicate (bit24Before=0, O+60 pre==request, O+68 post=FFFFFFFF), but do not claim an observable O+68 write unless before differs (in prior run it was already FFFFFFFF). O+60 changed in zero Stage41 windows; O+64 changed in 346; bit0 changed 0->1 in 348; 92 distinct raw IDs.

For each Stage42 activation, select the last earlier Stage41 for the same side and unchanged object pointer. **350 of the 356** activations match that last Stage41 object's pending ID exactly. Crucially, these 350 matches reference **350 distinct Stage41 seq IDs**, exhausting **all 350** Stage41 NEW/REQUEUE rows exactly once. There are zero unmatched newly queued Stage41 records in this run. QPC lag from matched Stage41 qpcAfter to Stage42 qpcBefore: 2..376 ticks, median 6; this is QPC ticks, NOT seconds or milliseconds. No duplicate Stage42 association for any one new Stage41 entry. The 76 current/duplicate Stage41 records do not create additional activations in this direct-join sense.

The remaining six Stage42 activations are **genuine correct state promotions, not mismatches** (each satisfies bit0 clear and O+60 post==O+64 pre), but their IDs differ from the most recent Stage41 pending ID for the same object:

- P1 Stage42 seq=204, 229, 292: ID=00000025, last Stage41 pending=0000002F; Stage41 O+68 POST=00000025 and Stage42 pre O+30 low byte=01, O+68 already FFFFFFFF. This is consistent with original native 1E9F40's alternate path: if O+30 !=0 and O+68 !=FFFFFFFF, it overwrites O+64=O+68, resets O+68=FFFFFFFF, sets O+08 bit0 before calling 1E9C70 at 1E9F62. The intermediate write is not directly captured, so mark as static/runtime-consistent, not direct proof of the precise writer.
- P2 Stage42 seq=83, 285, 286: ID=0000024C / 0000024D / 0000024C after last Stage41 pending=00000248, but Stage42 pre O+30=0, O+68=FFFFFFFF, so the 1E9F40 O+30-gated alternate path cannot be the immediate explanation in these windows. Their upstream writer is unknown.
- Exact six detailed rows in the CSV show last Stage41 raw ID, QPC lag, O+30/O+68 and native active ID.

Stage39 58/58 raw output windows have immediately following same-side Stage40 request with exactly same ID before any next same-side Stage39; no unobserved tail. Stage37 66/66 join exact EventDispatch type1 detail rows; EventDispatch 204 detail rows total (type0 138 and type1 66). Stage36 r2 495 candidate rows, 12 non-sentinel results. No explicit ERROR/WARNING in these smaller retrieved logs. Stage38CallbackWindowTrace and Stage35ControllerConsumerTrace are very large: the GitHub text connector returned no text from them; their complete uploaded original Git blobs are preserved but cannot claim their contents or health counters were independently checked here.

## Next diagnostic

The user goal is downstream behavioral effect, not merely command state. Original game EXE (SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba) disassembly confirms native 1E9F40 logic at RVAs 1E9F40..1E9F6C and 14 static direct CALL sites to 1E9F40 (10164, 1023A, AD75C, AD778, AD797, AD7BC, AEA69, AEAD9, AEB09, 13A20B, 13DFB9, 1917AB, 19C2E1, 1A8E48). A low-risk follow-up can add just native 1E9F40 caller provenance to the **existing** Stage42 before/after observer, using the original return-address stack while preserving the already validated CALL-site patch. This could distinguish the P2 alternate origin without adding more hooks. Alternatively continue static analysis of code that uses O+60 to identify actual attack/animation/physics effects, but do not claim them until verified in runtime.

Original EXE available in Library, never request re-upload. Do not repeat Stage42-only tests unless an additional hypothesis and new fields are deployed.
