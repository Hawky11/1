# Stage45 runtime verified — 2026-11-10

Upload: 4f8d227d01cf624526569aa89999e092ca65ef9d, 12:40:11 Moscow. Build starts 12:38:34, runtime initialization 12:39:00–01, last captured observations 12:39:38.
Source: fe91fb09a31579f71e58d211670839838bdd33b1 (stage45-definition-resolver-trace).
Evidence preserves all original uploaded Git blobs unchanged, plus Stage44/45 overlay and analysis trees. Source snapshot inclusion does not prove byte identity of every local compilation input.

## Build and installation
Windows x86 BUILD SUCCEEDED, DLL 1,052,672 bytes. Existing C4754 warning only.
Stage45 r1 INSTALLED 1/1 at 1E84B3->1DF2D0; Stage44 1/1, Stage42 1/1, Stage43 enabled, Stage40 r2 3/3, Stage41 enabled.

## Resolver result
231 contiguous Stage45 records: P1 106, P2 125.
All 231 have readMask=1F, path=3, predictionMatches=1, resultIsFallback=0.
Independently recalculated tableBase + (indexWord-1)*5C equals nativeEAX in every row. All variants are zero; no nonzero-variant or fallback branch coverage is claimed.
All 231 join distinct Stage44 bindings by exact bindingQpcBefore, side/player/object/TID; resolver QPC is inside the binding interval, ID/variant match binding PRE fields and actual EAX equals binding POST O+34.
All Stage44 object masks FF/FF, post metadata masks 7, ready byte O+26D=1 and O+178 equals metadata +2C low byte.
Thus the observed active IDs produce the native table-derived definition pointers actually committed to the objects. This proves native binding, not semantic optimality, animation, hitboxes, damage or better AI.

## Activation and request chain
209/209 Stage42 activations each contain a corresponding Stage44 binding. All promote pending O+64 to active O+60 and clear pending bit0; 205 actual active-ID changes and 4 already equal IDs.
305 Stage41 windows match Stage40 sequence, side/player/thread and QPC. Native request predicate yields 209 NEW/REQUEUE and 96 duplicates.
All 209 NEW/REQUEUE match distinct subsequent same-object Stage42 activations before the next Stage41 window. No extra Stage42 activation in this capture.
All 209 Stage43 records match Stage42 sequence, side/player/object/thread and QPC; all callerRVA AD75C and callerRead=1.
22 additional Stage44/45 pairs lie outside Stage42 windows, including the two initial ID0 bindings. Their resolver results also match the table (not fallback). Upstream activation caller/writer is still unknown; do not call these lost commands.
CSV records all joins, including blank stage42_seq for those 22.

## Health and integrity
Stage42 totals: calls 2525, otherObjects 2316, difference 209.
Stage44 totals: calls 9069, otherObjects 8838, difference 231.
Stage45 totals: calls 43539, otherObjects 43308, difference 231.
Every reported dropped/overflow/mismatch counter is zero. Stage45 otherObjects includes calls excluded by active Stage44-parent filtering, not exclusively other game objects.
Stage40/41/42/44/45 sequences are contiguous. Full retrieved Stage36/37/39–45, Queue and EventDispatch ASCII content sizes match Git blob sizes; all end with complete newline records. No ERROR/FAILED/WARNING tokens in those runtime logs. Clean process shutdown is not established by EOF.
Stage39: 31/31 output IDs match immediately next same-side Stage40 request; Stage37 36 effect records; Stage36 284 results, 6 nonsentinel.
Stage38 adapter again returned no content despite nonempty Git blob. Stage35 and other large historical logs were not comprehensively read. Exact original Git blobs are preserved; no global all-logs integrity claim.

## Next task
Stage46 source/DLL NOT prepared. Inspect the native downstream use of the bound action-definition pointer and identify the first concrete execution-state/animation-related consumer before choosing a passive observation boundary. Continue distinguishing native execution state from hit/damage/AI improvement. The 22 outside-Stage42 bindings remain an independent caller-attribution question.
No repeat of Stage45 needed. Original AALib.exe already exists in Library.
