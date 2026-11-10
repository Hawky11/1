# Stage67 targeted native paths — SOURCE ONLY

Date: 2026-11-11. Branch: `stage67-pending-promotion-zero-write-probes`.
Base: Stage66 `748a0a61dfaeea816592228fdf6248ab81757b5e`.
Adapter: exact Stage65 source `04ecb3370a66b4ed6dfd3d726df35f6f37546cc8`, with only three added integration lines. Previous source, evidence branches and main are unchanged.

## Verified locally

Private original AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, PE32/i386/ImageBase 00400000. Binary retrieved from existing AALib.zip; no binary is committed.

The reproducible [verifier](tools/verify_stage67.py) reads the **actual six runtime guard arrays from the new source**, compares them against this EXE, and verifies the source operand/global and adapter installation order. The exact copied native 1E9F40 body has no stack arguments/frame and returns via RET at 1E9F69 or 1E9F6C. The E9E60 overwrite is one six-byte MOV; E9E66 resumes at the following ten-byte AND. No relocated instruction or added hardware breakpoint is required.

The same verifier extracts the actual two inline assembly bodies, assembles them with Keystone 0.9.2 and runs **512 differential Unicorn 2.1.4 cases** (256 per hook). Comparison includes all GPRs, ESP, EFLAGS including DF, nonempty x87 registers/status/control/tag, XMM0–7, MXCSR and affected memory. PRE/POST observer stubs deliberately clobber caller-saved GPRs, flags/DF and FPU/SSE state. Tests cover enabled/disabled O+30, empty/nonempty O+68, pending bit0 on/off, native EAX outcomes and zero/nonzero EDX values. Actual original 1E9F40 machine code runs in both paths; its deep 1E9C70 activation is mocked identically in both. Each wrapper executes the native function/store exactly once. **All 512 cases PASS.**

This validates extracted wrapper instruction behavior under the model, **not C++ TLS/ring implementation, Windows APIs, installation races, MSVC code generation or gameplay**. No DLL was built or installed. SOURCE ONLY is retained until the user's MSVC x86 Build.log and a matching runtime capture exist.

## Hook/ABI contract

| Probe | Patch | Native operation | Continuation | Captured state |
|---|---|---|---|---|
| kind=0 | AD75C five-byte E8 | thiscall 1E9F40, ECX=O, no stack args, once | original AD761 via wrapper RET | P/O/side/TID/QPC, caller return, native EAX, O+08/+30 byte/+60/+64/+68 |
| kind=1 | E9E60 six-byte E9 + NOP | MOV [ESI+3F4C],EDX, once | JMP E9E66 | P/O/side/TID/QPC, actual EDX, P+3F4C/+3B7C, runtime global source |

Observers use PUSHFD/PUSHAD and aligned FXSAVE/FXRSTOR around each C++ call. They run FNINIT/CLD for diagnostics and restore the native flags/FPU afterward. No volatile ECX preservation is assumed across the real native function. Kind=1 checks ESI/EDX consistency across its MOV. The writer is a JMP detour and does not push a native return address. Timing and added stack consumption remain instrumentation effects.

## Installation and coexistence

- Require Stage42 1/1, Stage58 3/3, Stage65 1/1 and Stage59 worker not yet started. Failure does not block independent Stage59 startup.
- Use the shared patch mutex. All enumerated peer threads must suspend and yield CONTEXT_CONTROL; reject overflow, enumeration/open/suspend/context errors and EIP in either overwritten range. Resume every suspended peer on failure. No debug-register context is requested or written.
- Obtain write permission on **both** code ranges and recheck signatures before writing either. The installer deliberately excludes already-detoured 1E9F62 from original-byte comparisons. Validate the relocated global operand against `gameBase+4254E8`; do not assume EDX is zero at runtime.
- Flush both instruction ranges and restore page protections. Post-write cache/protection failure logs explicit ERROR and invalidates capture; it does not pretend no patch occurred.
- Peer enumeration is a snapshot, not an OS-wide thread-creation barrier. Installation still assumes the existing initialization window; emulation does not prove race freedom. If busy EIP is found, installation refuses without automatic retries.
- Ring capacity 8192, per-kind per-thread depth limit 8. Overflowing depth stays balanced. Sequence increments even when ring drops a row; HEALTH reports dropped/overflow/mismatch/readFail and filtered object/player totals. Only the worker performs file logging, not hot-path observers. The two kinds share a sequence, not separate consecutive sequences.

## Log schema

All numeric fields except kind/seq/TID/QPC and side are hexadecimal. `callerReturn` is an absolute address, only meaningful for kind=0; compare it to runtime gameBase+AD761. `nativeEAX` is a raw full register (native return semantics may only define AL). QPC-before is recorded after PRE snapshots, QPC-after before POST snapshots. Callback/file flush order is not event order; join by QPC and actor identity.

| Field | kind=0 promotion | kind=1 store |
|---|---|---|
| v0 PRE>POST | O+08 DWORD | P+3F4C DWORD |
| v1 PRE>POST | O+30 low byte | P+3B7C DWORD |
| v2 PRE>POST | O+60 DWORD | global gameBase+4254E8 DWORD |
| v3 PRE>POST | O+64 DWORD | unused zero |
| v4 PRE>POST | O+68 DWORD | unused zero |
| Full masks | preMask=postMask=1F | preMask=postMask=07 |
| writeValue | unused zero | original EDX, never substituted |

**Stage43 provenance changes predictably:** its raw callerReturn for wrapped AD75C now points inside the diagnostic DLL, so knownCaller=0 is expected. Stage43 is unchanged and does not fabricate an original address. Stage67 records the real outer CALL return and supplies provenance through actor/QPC containment. Other 13 native callers retain their previous behavior. Old Stage65/66 correlators assuming every Stage43 callerRVA=AD75C must not be used unmodified on Stage67 evidence.

## Runtime proof still outstanding

For 2F→25, demand full Stage67 entry masks, nonzero gate, pending 2F/deferred 25, plus the matching nested Stage42 window showing promoted pending and cleared deferred with active 25. A generic promotion is not the exact anomaly, and failure to reproduce is not a disproof.

For zero origin, kind=1 EDX=0 and actual postRequest=0 directly establish execution of E9E60. Correlate a subsequent Stage40 consumer only within one session and identical side/player/thread, excluding intervening observed writers/consumers. Capturing this instruction does not exclude every unknown writer elsewhere in the engine.

Require new Build.log, Stage67 installation/health and same-run Stage40/41/42/43/65/59. Preserve Stage65 1,879,552-byte DLL and Stage59 r2 1,153,024-byte rollback. See [Russian installation guide](../Stage67_Overlay/README_STAGE67_RU.md).
