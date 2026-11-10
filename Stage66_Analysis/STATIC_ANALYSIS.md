# Stage66 — original pending-ID promotion and alternate zero request (STATIC VERIFIED)

**Date:** 2026-11-11. **Status:** exact original machine-code paths verified, their execution in the exceptional runtime windows **NOT YET PROVEN**. New independent branch: stage66-native-pending-promotion-and-zero-origin from complete Stage65 runtime report commit 5e98fe587355ca9ec124d5c9db6ce7ecb6238ff2. **No new DLL, hooks, game test, INI, or COM behavior change.**

## Input integrity / offline verification

User-owned original AALib.exe (175,556,608 bytes) was read locally from already saved private Library AALib.zip, SHA-256 **28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba**, PE32/i386, ImageBase 00400000. **21 original opcode ranges and three exact native E8/CALL destinations were independently checked and PASS.** Original read-only global at preferred VA **008254E8 = 00000000**. Neither binary nor disassembly uploaded. A read-only Python 3 reproducer [verify_stage66.py](tools/verify_stage66.py) verifies original signatures, native edges, direct E8 census, and optionally same-run log anomalies. The *full newly committed CLI has not been run on user's Windows PC*, although all its original instruction constants and three call destinations were independently checked locally. Original encoded direct E8 target census: **E9E30=11, 1E9F00=476, 1E9F40=14**; these are static potential native calls, not runtime counts. AD75C, 1E9F62, E9E4E are individually verified decoded original CALL instructions.

## A. Two Stage42 ID25 activations after Stage41 ID2F pending

Original unmodified 2026-11-11 00:44:38–00:45:23 logs from main commit 72f47bf63d3db14219a9007c8c18326162c6b400:

| Same-run evidence | First case | Second case |
|---|---|---|
| Stage42 seq, side | **39**, P1 | **155**, P1 |
| Latest Stage40 seq | **38** | **204** |
| Latest Stage40 ID / Stage41 pending O+64 POST | **0000002F** | **0000002F** |
| Stage42 O+64 PRE and active O+60 POST | **00000025** | **00000025** |
| Stage42 O+68 PRE/POST | **FFFFFFFF / FFFFFFFF** | **FFFFFFFF / FFFFFFFF** |
| Stage43 native caller RVA | **000AD75C** | **000AD75C** |

Stage43 confirms **AD75C CALL 1E9F40**. The Stage42 PRE snapshot occurs at the *inner* 1E9F62 CALL inside original 1E9F40, **AFTER** 1E9F40's opening conditional pending-promotion code. Thus the 2F to 25 change may happen INSIDE the same native function before Stage42 starts observing; it need not be a hidden external write between Stage41 and Stage42.

Original exact instructions:

| RVA | Operation / effect |
|---|---|
| **1E9F40** | CMP BYTE [ECX+30],0: deferred-command flag gate |
| **1E9F44** | JE bypasses deferred-promotion block when zero |
| **1E9F46** | MOV EAX,[ECX+68]: load deferred action |
| **1E9F49–1E9F4C** | CMP EAX,FFFFFFFF; JE bypass if empty |
| **1E9F4E** | MOV [ECX+64],EAX: deferred ID replaces pending |
| **1E9F51** | MOV [ECX+68],FFFFFFFF: deferred cleared |
| **1E9F58** | OR [ECX+08],1: mark for activation |
| **1E9F62** | CALL 1E9C70: Stage42-observed activation |
| **AD75C** | CALL 1E9F40: Stage43 original caller |

**Specific compatible but unproven explanation:** at entry to original 1E9F40, O+30 might be nonzero and deferred O+68 might be **00000025**. The function would copy that value over pending O+64=0000002F, reset O+68 to FFFFFFFF and call the Stage42-observed activation. All observed Stage42 PRE values would then match. **Actual O+30 and O+68 at the function entry were NOT logged**. The separate original setter 1E9F00 can write O+64 at 1E9F22, with 476 encoded direct original calls; it and aliased writers are not excluded. These static callsites are not proven active in either anomalous window.

## B. Stage40 seq320: zero without Stage65 E5BF0 request

Observed Stage40 **seq320 P1 request 00000000**, QPC-before 1440484223522. The last preceding Stage65 P1 writer (seq303) wrote **00000304**. Later Stage65 seq321 wrote ID zero only at QPC 1440484323395, too late to cause seq320. 429/429 Stage65 writer rows have their own Stage40 consumer and Stage65 health has zero dropped/overflow/mismatch/readFail, so no E5BF0 event is observed for seq320. **This is a visibility gap, not lost AI action.**

Exact original alternate native writer **E9E30**:

- At E9E36 it reads P+3B7C; E9E3C SHR 15, E9E3F TEST AL,1, E9E41 **JNE skip** when bit15 **SET**. Thus proposed store is possible only when **bit15 CLEAR** (do not reverse the branch).
- Original helper F2ED0 is called at E9E4E; E9E56 TEST AL,AL; E9E58 JE bypasses write if helper returned zero.
- At **E9E5A** EDX is loaded from preferred original read-only VA **008254E8**, independently verified original DWORD **00000000**.
- At **E9E60** original MOV [ESI+3F4C],EDX writes **P+3F4C=00000000** when both above gates permit execution.
- At E9E70 native code clears P+3B7C bit11 after the branch. Stage40 seq320 P+3B7C PRE=00000000 is compatible, **not causal proof**.

The E9E30 path is **not intercepted by Stage65** (which only wraps E5BF0), so it is a strong candidate for seq320, but there is NO runtime event proving that E9E60 executed in the window. E9E30 has **11 original encoded direct E8 callers**, not proven real same-frame callers.

## Stage65 evidence preserved

Stage65 runtime 429/430 E5BF0→Stage40, 430/430 Stage40→Stage41, 316/318 Stage42 matching latest Stage40 ID, 315/318 with continuous E5BF0→42, 54 observed native direct E5BF0 caller sites and 114 duplicate requests concentrated at 153ECA remain valid. Stage59 from same run has 18,783 contiguous X writes with no reported drop/readFail/debug-register ownership conflict. No claims of improved AI or verified attack/damage/hit outcomes follow from the new static evidence.

## Stage67: minimal runtime discrimination, not a broad new watch

**First and highest-value probe:** guarded original **single CALL AD75C → 1E9F40**, already the exact Stage43 caller for both exceptions. Observe **entry** O+30 byte, O+64, O+68, O+08 and P1/P2, TID, QPC BEFORE executing original 1E9F40 **exactly once**; then capture POST. If entry O+30!=0, entry O+68=25, entry O+64=2F and POST O+68=FFFFFFFF, O+64=25, the native deferred-promotion explanation becomes directly verified. If not, trace original other direct O+64 writers (including 1E9F00) without guessing.

**Second conditional probe:** a *single original E9E60 six-byte store* (rather than 11 E9E30 caller patches) with signature-guarded trampoline executing the MOV once, capture ESI=P, EDX=global value, P+3F4C PRE/POST, QPC, TID. This is exactly the native write instruction and can prove or rule out that source for new seq320-equivalent events; need clear ABI/register/flag preservation and stage install collision checks before implementation. **No new DR0/DR1/DR2/DR3 watchpoints**; Stage59 retains DR0/DR1 and foreign DR owners must be respected. No game-state value injection.

Stage67 source and runtime not yet built. First audit x86 ABI, preserved caller EAX/EFLAGS and SSE/FPU, reentrancy, ring capacity, code signatures on the correct original PE and patch mutex. Maintain known successful Stage65 DLL **1,879,552 bytes** and earlier Stage59 r2 DLL **1,153,024 bytes** as rollback. No new user action required for this static Stage66 branch.
