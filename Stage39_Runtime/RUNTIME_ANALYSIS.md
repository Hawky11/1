# Stage39 — Windows x86 runtime verification and native action-request observation

Date: 2026-11-10. User-uploaded [commit `fe8b2207`](https://github.com/Hawky11/1/commit/fe8b2207e73af6dc5f6e89cba453369afe0e7828). Source-only Stage39 branch: [stage39-native-action-output-trace](https://github.com/Hawky11/1/tree/stage39-native-action-output-trace), `0fa652958df7f6ed4f4a6055e90df40c1b533d56`. Previous Stage38 evidence branch `stage38-native-callback-runtime-evidence` commit `12dd482bdb3070131016f9fc234ba9574580094c` is preserved. This evidence branch archives all **27 newly uploaded original blobs byte-for-byte** under `Stage39_Runtime/`, plus this report. No prior stage branch is modified. Original SHA-verified EXE remains in Library `/AALib.zip`, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`; no need to request it again.

## Build, deployment and log integrity

`Build.log` at 2026-11-10 07:57:55: genuine MSVC x86 `BUILD SUCCEEDED`, `dinput8.dll` **285,184 bytes**. Only historical warning C4754 in `AICommandSiteTrace.cpp`; no compile/link errors. `ArcanaAI_Stage39ActionOutputTrace.log`: `STAGE39_ACTION_OUTPUT_BUILD=2026-11-10-r1`, `STAGE39_ENABLED piggyback=Stage38 2/2`, targetRVA `(12EB10,123AF0,1DA490)`, no additional hooks/gameplay writes. Stage38 still reports `STAGE38_CALLBACK_WINDOW_BUILD=2026-11-10-r1` and `STAGE38_INSTALLED 2/2`, with original EE7BE indirect guarded CALL untouched.

Complete logs read and checked (no UTF-8 truncation): Stage38 **4,976,009 bytes, 5,272 sequential windows** seq1..5272, newline terminated, no explicit errors/warnings/overflow/drop/mismatch. Stage39 **50,985 bytes, 51 windows**, newline terminated, no explicit errors/warnings. Every Stage39 record has 30 valid before/after DWORD pairs, correct recomputed readable `changedMask`, nonnegative same-window QPC order and a unique exact Stage38 match via `stage38Seq/side/player/tid/callback/callbackAfter/qpcBefore/qpcAfter/type/payload/objectBefore/generationBefore`. Stage39 `a2/a3` mirrors Stage38 `f3/f4` exactly, supporting common per-window sampling; **51/51 joins PASS**, no unmatched target Stage38 records. Stage38 targets outside those three intentionally have no Stage39 row.

All 51 Stage39 callbacks have nonzero EDI, type=1, and net state change; by target: RVA `12EB10` 39 (P1/P2 combined), `123AF0` 4, `1DA490` 8. **All 51** `a0 = P+3F4C` change `FFFFFFFF -> non-FFFFFFFF`, demonstrating native action-request ID generation. In all 51, next callback `a2 = P+3ED0` and next argument `a3=P+3ED4` change; this is **different** from the saved actual EDI that ran. Native primary action structure `P+3EE4` changed in 12 windows: `a8` (offset +28) 12, `a9/a10` (offset +30/+34) 8 each, `a7` (+0C) 2. `P+40AC` alternate state indices a11..a17 did not change across these complete windows. Other changes: actor pointer a18 one time, actor-relative a20/a22/a23 one time each, O+280 (a29) three times. P+3B7C (a1) did **not** change in this sample, so do not claim bit24 set. If actor pointer changes or is null, actor-relative fields need mask-aware interpretation: 35 rows have null actor both sides, full 30-bit masks for 15 rows, 35 rows mask `3F07FFFF` both sides, one row mask `3F07FFFF -> 3FFFFFFF`. None of that indicates a logging failure. All 51 raw P+3F4C values were FFFFFFFF *before* the callback.

Other observers: `EVENT_DISPATCH_DETAIL` **162 contiguous rows** (105 type0, 57 type1); all **57/57** `STAGE37_TYPE1_EFFECT` rows exactly join type1 dispatch on seq/side/player/tid/payload/QPC. Stage35 **11,176** contiguous full-mask (FFF) rows, perfect TIMER/FLAGS alternation: 2,794 pairs per side, no malformed order. Stage36 **348** contiguous CANDIDATE rows. `ProducerCallTrace.log`: 10,320,329 bytes / 13,265 STAGE14_PROBE records, no explicit errors. ParentFlow, FinalCommand, Queue, Stage29, Stage31 logs checked; no explicit warning/error in key streams. All checked main evidence log files are nonempty and newline terminated; this does **not** prove full worker flush at process exit.

## Accepted vs non-accepted native candidate outcomes

Stage36 348 candidates: **336 sentinel FFFFFFFF**, **12 nonnegative/nonsentinel EAX**. All 12 EAX match preexisting native input arg3; not every non-sentinel EAX means accepted type1.

**10 positive EAX with output flag out4=1** proceed through unique `F7C62` type1 `QUEUE_EVENT_DETAIL`, `EVENT_DISPATCH_DETAIL`, `STAGE37_TYPE1_EFFECT`, and a same-player/payload/generation next Stage38 callback. Eight have target RVA among Stage39's three (and exact Stage39 join). Two have **RVA 122800**, deliberately uninstrumented by Stage39, and must NOT be called dropped.

| Stage36 seq | Side | Native ID decimal | out4 | Queue seq | Dispatch seq | Stage38 seq / callback RVA | Stage39 raw a0 after |
|---:|:---:|---:|:---:|---:|---:|---|---|
| 10 | P2 | 6 | 1 | 14 | 48 | 1173 / 12EB10 | 84h |
| 29 | P2 | 12 | 1 | 15 | 49 | 1223 / 12EB10 | 88h |
| 61 | P2 | 92 | 1 | 16 | 53 | 1295 / 1DA490 | 302h |
| 70 | P2 | 6 | 1 | 19 | 57 | 1388 / 12EB10 | 84h |
| 87 | P2 | 14 | 1 | 20 | 58 | 1431 / 12EB10 | 8Ah |
| 142 | P2 | 82 | 1 | 33 | 99 | 2238 / 122800 | N/A, intentionally not sampled |
| 155 | P2 | 95 | 1 | 38 | 119 | 3084 / 1DA490 | 305h |
| 315 | P2 | 6 | 1 | 53 | 157 | 4583 / 12EB10 | 84h |
| 331 | P2 | 12 | 1 | 54 | 158 | 4624 / 12EB10 | 88h |
| 348 | P2 | 82 | 1 | 55 | 159 | 4673 / 122800 | N/A, intentionally not sampled |

**Two positive EAX=39, but out4=0**: Stage36 seq254 P1, at qpcAfter=836727662069; seq306 P2, at qpcAfter=836754998977. In both native outputs pre/post unchanged and out4=0, out5 nonzero; ParentFlow F3280/F4360 corroborate each actual value and result in same P/TID/QPC interval. Neither produced a new observed F7C62 type1 queue/dispatch. Queue's type0 payload0x27 from F7CC5 in the broader run is a **different** path and cannot be presented as the missing F7C62 type1 event. `out4=0` vs `out4=1` is a strong *native selection/forwarding* distinction, **not proof of a gameplay AI bug**. Investigating out4 interpretation in the original F79E9 caller is a valid future branch if needed.

The 10 accepted chains and 51 Stage39 snapshots confirm native action **request/state creation**, not completed animation, hitbox/collision, damage or skillful decisions.

## New original-EXE static downstream consumer (Stage40 candidate)

Disassembled SHA-matched PE32/i386 `AALib.exe` with objdump using preferred base 0x400000. Unique direct CALL **RVA AD72F -> E8480** (encoded bytes `E8 4C AD 03 00`) inside native per-player iteration at AD710. At E8489 native E8480 reads `P+3F4C`, tests FFFFFFFF, and on non-sentinel uses `RVA 1E9F00` (callsite E84AB), `125920` (E84E3), and `1357F0` (E8505). In active branch it writes actor-relative `[eax+10] |= 0x8000` (E850A), `[eax+2D0]=[P+4]` (E8514), **`[eax+280] = P+3F4C`** (E8523), sets P+3B88 bit24 (E8529), then **resets `P+3F4C` to FFFFFFFF** at E853F and clears bit24 of P+3B7C at E8549. If the native selection result at E84ED is sentinel, it takes a different flag update and still clears P+3F4C. Thus request consumption / output-binding path is verified statically; **no runtime evidence yet** that the Stage39 raw IDs reach this consumer and cause the actor+280 write. Beware `E8480` calls E9F00 from many other sites; the unique AD72F callsite is the focused next candidate.

Next Stage40 should instrument that consumer passively using an exact validated callsite (AD72F) or bounded read-only before/after consumer observation, with strict P1/P2 identity and the same native object/actor pointers, QPC, raw ID before and after, actual actor+280 and return branch. Do not synthesize inputs or action IDs. Do not call actor+280 a rendered animation/hit until the renderer/animation engine is traced. For runtime correlation, compare Stage39 `a0.post` to later Stage40 raw request pre (same P/TID), and consider other intervening writes and frame boundaries. Preserve Stage29 AITrace.cpp, working Stage35/36 inls and all branches.

