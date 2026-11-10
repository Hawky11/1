# Stage44 runtime verification — 2026-11-10

Upload: 73a93ede99755e063896c78b510a2acee70311ba (12:24:49 Moscow). Build began 12:22:57; runtime initialization 12:23:31–32, observations through 12:24:14.
Evidence branch preserves every original uploaded Git blob unchanged. Stage44 source/analysis trees copied from stage44-action-definition-binding-trace; source reference is not proof that every local source file was identical.

## Build and installation
Windows x86 BUILD SUCCEEDED, dinput8.dll 1,047,552 bytes. Only existing C4754 in AICommandSiteTrace.cpp. Stage44 r1 INSTALLED 1/1 at 1E9E2D->1E8490; Stage42 1/1, Stage43 enabled, Stage40 r2 3/3, Stage41 enabled.

## Direct binding results
321 contiguous Stage44 records: P1 166, P2 155. All object read masks FF/FF; all post metadata masks 7. The first two pre metadata masks are zero because pre definition pointer is zero.
All 321 post pointers are nonzero, all O+26D post bytes equal 1, all O+178 post values equal the low byte of metadata DWORD at definition+2C. Active ID remains unchanged across every binding wrapper. Definition pointer changes in 320/321 windows.
All 292 Stage42 windows contain exactly one distinct Stage44 binding with matching side/player/object/thread, nested QPC and matching active ID.
29 additional binding windows are outside Stage42 windows (P1 13/P2 16), including two initial ID=0 bindings. They demonstrate that this binding CALL is also reached outside the monitored Stage42 caller window; their upstream caller and producer are NOT established. See CSV for all sequence numbers/IDs.
Nonzero readable metadata does NOT distinguish a normal resolver result from fallback. No claim of correct non-fallback lookup, animation, hitbox, damage or improved AI.

## Upstream correlation
Stage42 has 292 contiguous activations: all pending O+64 promoted to O+60 and bit0 cleared; 291 active IDs change, one was already equal.
Stage43 has 292 records, matching Stage42 sequence/QPC; all callerRVA AD75C.
Stage41 has 355 contiguous windows: 291 NEW/REQUEUE by native predicate and 64 duplicate/current. All 291 NEW/REQUEUE requests match 291 distinct subsequent same-side activations before the next Stage41 window. One extra activation: Stage42 seq160 P1, object 29F03020, ID 33->26, QPC 995742367971..995742367993, pre O+30=1; source writer not established.
Stage42 HEALTH totals calls=2349, otherObjects=2057; Stage44 calls=7196, otherObjects=6875. Differences exactly equal 292 and 321 logged windows. Every dropped/overflow/mismatch counter is zero.

## Integrity and limits
Complete Stage40–44 ASCII text lengths match Git blob sizes, CRLF final records, sequences contiguous where defined. Stage36/37/39 full text retrieved and lengths match blobs. Queue and EventDispatch full text retrieved. No full-log integrity claim for Stage35/38 or other historical large logs: Stage38 text adapter returned empty content despite 4,880,209-byte Git blob. Exact uploaded blobs remain preserved in evidence branch.
No inference that an end-of-file record proves clean shutdown.

## Next task (Stage45 not prepared)
Resolve the native lookup at 1E84B3->1DF2D0: inspect original SHA-pinned AALib.exe, identify normal versus fallback return and capture ID/variant/context/result if justified. Consider original 1E9C70 caller attribution for the 29 outside-Stage42 bindings. Existing Stage44 run need not be repeated. No new DLL/source is claimed ready.
Original AALib.exe remains in Library; do not request another upload.
