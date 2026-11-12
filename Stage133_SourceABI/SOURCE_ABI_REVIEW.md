# Stage133 — SOURCE/ABI audit of original parent scope lifetime (2026-11-13)

**Status:** static byte/branch/exit coverage **PASS**; Windows runtime installation **BLOCKED** pending owner/quiescence/unwind proof. **NO** new DLL, hooks, exe patches or gameplay writes.

Full Russian offline report/verifier, original objdump excerpts and 39-test synthetic state machine supplied in conversation file **`ArcanaAI_Stage133_ParentScope_ABI_ExitCoverage_StaticAudit.zip`**, 11 files, SHA256 **`e2fa357a573324d7111d9618a5c5c11a864451bac1cc707bb8dacf7829c6587a`**, ZIP CRC PASS. Chat artifact is not uploaded to GitHub. Original user AALib.exe 175,556,608 B SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` was freshly extracted, hashed and disassembled.

## Native source discoveries

Two original parent functions have precisely five plain `RET` sites, including an early return without F30B0:

| Parent | RET RVA | 5-byte prospective epilogue observation site | Original five bytes |
|---|---|---|---|
| `F34D0` | `F3662` | `F365C` | `5F 5E 5B 8B E5` |
| `F34D0` | `F366B` | `F3666` | `5E 5B 8B E5 5D` |
| `F3930` | `F3E4A` | `F3E42` | `5F 5E 8B C3 5B` |
| `F3930` | `F3E5D` | `F3E55` | `5F 5E 8B C3 5B` |
| `F3930` | `F4017` | `F4011` | `5F 5E 5B 8B E5` |

A plain RET cannot carry a 5-byte E9. Future parent exit instrumentation must run **before** the original stack/frame teardown and replay each original POP/MOV precisely, preserving GPR/EFLAGS/ESP/EBP/return EAX. Entry+literal+commit without exit cleanup is insufficient for strict positive-control parent scope. These five are normal returns only; SEH, C++ exceptions, nonlocal unwinds can skip them; still requires safe stale-scope invalidation.

The **new safer table commit detour candidate** is `F3C11` (7 bytes: `83 7D 14 00 89 45 10`) instead of Stage132's `F3C15` (5 bytes: `89 45 10 8B 03`). F3C11 steals **CMP [EBP+14],0; MOV [EBP+10],EAX**, leaving faultable `MOV EAX,[EBX]` intact at original F3C18. Caveat: replay native CMP then MOV exactly once, pass **CMP's** flags unchanged through original later `F3C27 JE F3D6E`, and only mark a successful post-write witness after native MOV. F3C15 instead moves the potentially faulting `[EBX]` load to a trampoline; not recommended.

**Future-only minimum normal-return plan: 10 sites**: parent entries F34D0/F3930; literal writer sites F3579/F35F9; safer table commit F3C11; five epilogue observers F365C/F3666/F3E42/F3E55/F4011. Optional F3B71 is a raw table READ and is not a candidate commit. Source classification = last successfully witnessed original assignment *before* child F30B0 entry. A read without a post-transform original write is not enough; initial candidate may be inherited.

## Verified static checks

- Original PE32 image base 0x400000 and SHA-256 checked; all **21/21** selected original native opcode site signatures match exact bytes and whole-instruction boundary lengths.
- Original 2/2 direct F3654/F3CBC E8 calls target F30B0.
- Full local objdump ranges F34D0..F366B and F3930..F4017 scanned: **2+3 RET** exactly, **150 local direct control-flow branches** parsed, **no direct jump into stolen bytes of future 10-site plan**.
- F3930 indirect jump F3B12 jump-table: original VA 004F4018 two destinations **004F3C49** and **004F3B19**; byte index table at 004F4020 uses entries 0/1.
- Nonoverlap with **26 documented, non-exhaustive** exclusion/owner intervals: **305/305** comparisons = `10*26 + C(10,2)`. These intervals include 17 older Stage128 envelopes, 6 Stage84 writers, 3 Stage129 hooks. Do **NOT** construe them as all current live owner patches.
- Read-only verifier and synthetic strict lifecycle tests **39/39 PASS**; tests enforce five exits, stale scope reuse rejection, all-writer coverage, last writer wins, table READ not commit, candidate/actor/frame/caller exact join, and fail-closed UNKNOWN on loss/unwind. These tests do not emulate game CPU or prove SEH.

## Source/ABI blockers to native deployment

1. Full integrated installed hook source/ownership inventory missing for some Stage82–129 modules; current user project is not in this GitHub repository. Static non-overlap is only a partial check.
2. Checking only suspended threads' EIP in the parent function cannot detect a parent frame on the **call stack of a thread currently inside another callee**. Need proven quiescent startup window or reliable in-flight stack detection before entry hooks are installed; refuse if uncertain.
3. Five RET hooks do not cover exceptions, longjmp, thread termination or abnormal unwinding; stale TLS parent scope may be mistaken for a new invocation at a reused EBP. UNKNOWN on uncertainty; reliable nonlocal cleanup or a verified fail-closed strategy required.
4. Correct per-site x86 ABI/flags, SSE/x87/MXCSR/DF and stack/exception fidelity must be implemented and tested; F3C11 CMP flags are essential.
5. Stage129 `Observe` counts all hits but `Sample(kind)` emits only first 64 and each 128th after; **87** sampled entry/return pairs for **3000** calls in Stage130. Per-event upstream provenance needs an additional UNSAMPLED all-`candidate==0x27` bridge, unique parent-child correlation, loss counters; **not yet implemented**.
6. Need atomic install/rollback with owner byte guards, thread suspension & safe in-flight-frame preflight, fail-closed errors, then Windows MSVC x86 build and fresh runtime verification. This Stage has done none of those.

**Next Stage134**: retrieve/inspect full up-to-date integrated source, enumerate current owner ranges and prove startup quiescence/unwind handling before a source patch. Preserve Windows-tested Stage129 DLL **2,099,200 bytes**, Stage122 rollback **2,089,984 bytes**, Stage36 OFF; no F2DE0 repatch, no FA480 override, no DR or synthetic gameplay action.
