# Stage45 — native definition resolver

2026-11-10. Source only; Windows x86 build and game runtime pending.

Original AALib.exe: 175556608 bytes, SHA256
`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
Read from existing AALib.zip. No binary redistribution in this branch.

## Native algorithm verified from original bytes

CALL RVA `1E84B3 -> 1DF2D0`, bytes `E8 18 6E FF FF`.
ECX=context, stack args ID and variant; ESI is the native object.
Both resolver exits are RET 8. Native result EAX is stored at O+34 by 1E84B8.

1. Read byte `[context + ID + B0000]`. Zero returns fallback.
2. Read signed WORD `[context + ((ID << 7) + variant) * 2]`.
3. Decrement the sign-extended index. Negative returns fallback.
4. Return `[context+B0B0C] + (index-1)*5C`.

Fallback instruction RVA 1DF2FA: `MOV EAX,0E2E49E0` at preferred image base 00400000.
Its immediate DWORD at RVA 1DF2FB has a verified PE HIGHLOW relocation.
Fallback RVA is therefore `0DEE49E0`, and at the Stage44 run's base 00800000
the fallback pointer is **0E6E49E0**. All 321 Stage44 POST O+34 values from
upload 73a93ede99755e063896c78b510a2acee70311ba differ from this pointer.
This closes the specific fallback-pointer uncertainty for that run. It does
not validate the semantic choice of action or prove animation/hit/damage.

## Instrumentation

One additional strict CALL wrapper at 1E84B3. Installation validates the original
CALL target, complete resolver bytes through MOV EAX opcode, relocation-adjusted
fallback immediate, and final POP EBP/RET 8. Validation is repeated while other
threads are suspended. Stage44 must already be installed.

Copies both original stack arguments, invokes native exactly once; native RET 8
cleans copies and wrapper RET 8 cleans original args. PUSHFD/PUSHAD and aligned
FXSAVE/FXRSTOR preserve native register/flag/FPU/SSE state around C++ callbacks.
No new worker thread or INI settings; uses existing Stage38 flush worker.

Only calls inside an active captured Stage44 window with matching native ESI
are logged. The parent binding QPC, player/object/thread identify that window.
Read-only snapshots capture context, ID, variant, gate byte, signed index WORD,
table base, predicted return, actual EAX and fallback address. Paths 0=unknown,
1=zero-gate fallback, 2=nonpositive-index fallback, 3=table result. Gate/index
are safely read as DWORDs and masked; a failed wider read yields UNKNOWN rather
than asserting the native byte/WORD was unreadable. Native arithmetic wraps at
32 bits. A prediction mismatch may reflect changed memory; inspect before
claiming a game bug.

Mask bits: 01 ID, 02 variant, 04 gate, 08 index, 10 table base. Normal table path
expects 1F; fallback paths need only their applicable reads. OtherObjects counts
all calls excluded by the active-parent/object filter, including same-player
calls outside captured Stage44; do not interpret it as exclusively other objects.

## Validation

`tools/test_stage45.py` verifies SHA, full resolver bytes, CALL target, fallback
relocation and integration. It extracts the actual wrapper assembly from source
and compares it to the original native resolver in Unicorn for 128 cases,
covering zero/nonzero gates, signed index boundary values, randomized ID/variant,
GPR/ESP/EFLAGS, XMM0/7 and FPCW/MXCSR, once-only native execution and argument order.
PASS does not establish full C++ compilation, all FPU states, concurrency or game runtime.

## Runtime acceptance and open questions

Require Stage45 marker and INSTALLED 1/1; join bindingQpcBefore to Stage44 QPC
for the same P/O/TID, with nested resolver QPC, matching ID/variant and EAX=POST O+34.
Inspect all masks, path predictions, fallback equality and health counters.
Stage44's 29 prior bindings outside Stage42 are still not attributed to an
upstream 1E9C70 caller. Stage45 can classify their resolver result if they recur,
but does not identify their producer. Preserve this distinction.
