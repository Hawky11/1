# Stage69 — native source of deferred action ID: exact original write chain

**Date:** 2026-11-11. **STATIC VERIFIED / RUNTIME-LINKED, not direct writer-IP runtime verified.**
Based on `stage68-deferred-origin-static-evidence`, plus the already archived Stage67 capture (2026-11-11 01:30:28). No new DLL, hook, run, modification to `AALib.exe`, or AI behavior.

## Original binary provenance

Original private `AALib.exe`, from Library `AALib.zip`, SHA-256:
`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
PE32/i386 image base `00400000`. All addresses marked `AALib+...` are RVAs. The stdlib verifier [verify_stage69.py](tools/verify_stage69.py) PASSed against that exact original EXE: **27 exact opcode ranges**, the native pointer flow, and both table lookups. The 3 action mappings below were read directly from file-backed sections.

## Newly identified positive-ID writer

In original callback function `AALib+1566B0`:

```asm
1566B6  mov edi,[ebp+08]                    ; callback argument
1566B9  mov esi,[edi]                       ; player P
1566BB  cmp dword [esi+3F54],3             ; one callback state gate
1566C2  mov eax,[esi+04]                   ; object O = [P+4]
1566C5  mov ebx,[esi+3F50]                 ; selector
1566CB  mov [ebp+08],eax                   ; stash O
        ...                                ; more conditional logic
156745  cmp ebx,1
        ...                                ; selected immediate request ID
156753  cmp ebx,2
        ...
156767  mov ecx,[edi]                      ; reload P
156769  call AALib+0E5BF0                 ; native action-request helper
15676E  mov edx,[edi]                      ; reload P
156770  mov eax,[edx+3F50]                 ; selector, reread after helper
156776  mov ecx,[eax*4+06317664]          ; selector -> table index (.data)
15677D  mov edx,[ecx*4+008254E8]          ; index -> action ID (.rdata)
156784  mov eax,[ebp+08]                   ; restore O
156787  mov [eax+68],edx                   ; **positive deferred ID writer**
```

The consecutive instructions from `15676E` through `156787` contain **no conditional jump**; a normal return from the request helper and valid memory accesses reach this store. They are real decoded original opcodes, not a speculative search hit based on the displacement `+68`. The object pointer originates from `P+4`, matching the Stage67 object pointer relationship. The callback address is also used as a native function pointer at `156B84` (`push 005566B0`); the callback scheduler's runtime dispatch is not directly captured.

### Original-file selector mapping

| P+3F50 selector | `[06317664 + 4*selector]` | `[008254E8 + 4*index]` | immediate ID passed to native request helper |
|---:|---:|---:|---:|
| 0 | 1A | **24** | **2E** |
| 1 | 1B | **25** | **2F** |
| 2 | 1C | **26** | **33** |

The helper's immediate IDs come from original `15674A/156758/156760` reads of `0082557C/00825580/00825578` respectively; these addresses hold `2F/33/2E` in the original file. The verifier's 27 opcode assertions cover the key request-call and deferred-store chain; the three immediate-ID loads and table values are also checked by the verifier.


**Selector stability caveat:** the immediate request branches use EBX captured from `P+3F50` before the helper call, whereas the deferred writer **rereads P+3F50 after that helper returns**. If the helper changes the selector, the two values can diverge. Therefore 2F/25 and 33/26 are the original-file nominal pairings when the selector stays stable; Stage67 did not log both selector reads.

**Important limitation:** `06317664` is in **writable .data**, so these are initialized-file mappings, not observed runtime table contents. Also, `P+3F50` was not captured as a Stage67 field. Do not assert actual per-player selector or an exact store instruction hit on the basis of the file data alone.

## Direct connection to previously observed P1 run

The Stage67 report already recorded Stage65 **seq276** on P1 with **caller RVA `156769`** (original native call `0E5BF0`), writing the **2F request** at QPC `1467958862442–1467958862446`. This call occurs **inside the newly decoded `1566B0` callback**, immediately before the fall-through table lookup and `O+68` store at `156787`.

- Before this episode: Stage67 seq4412 POST QPC `1467958766094`: P1 `O+68=FFFFFFFF`.
- Stage65 seq276 QPC `1467958862442–2446`: same P1 request `2F`, native caller `156769`.
- Stage41 seq278 PRE QPC `1467958862984`: same P1 object `O=2A393020` **already has O+68=25**, while active/pending was still 22. Its POST commits pending `22→2F`.
- Stage67 seq4414 confirms activation of `2F` while deferred `25` remains.
- Stage67 seq4436 later sees `O+30=1`, promotes deferred `25` to pending, clears deferred, activates `25`.

**Interpretation revised vs Stage67/68:** Stage65's `156769` call writes the *immediate request* (`P+3F4C`), not deferred itself. However, the *same native callback* contains an immediate downstream instruction that writes deferred `O+68`. The previous wording that deferred `25` was present “before the latest request” should only be read as “before the Stage41 pending-action commit”; the earlier Stage65 request call was at QPC `...62442`, before the PRE snapshot at `...62984`. The newly identified callback provides a specific, highly consistent source for the intervening `25` assignment. **Because `156787` was not directly instrumented, this remains runtime-linked inference, not a measured writer IP.**

Independent Stage67 P2 seq3465 observed deferred `26` while `33` activated; the same original callback has a `33 / 26` selector mapping. No P2 writer IP or selector value was directly recorded.

## Separate arming and promotion remain unchanged

The source of byte `O+30: 0→1` remains candidate `AALib+1E8FEB` (`C6 46 30 01`) in function `1E8DC0`. The arming interval is Stage67 seq4434 POST `1467959867548` to seq4436 PRE `1467959966659`, not the earlier write interval.

Original native `1E9F46→1E9F4E→1E9F51` is the consumer that copies deferred into pending and clears it; its effects were directly recorded by Stage67 seq4436.

## Next: Stage70 narrowly scoped observational confirmation

The **positive-ID origin no longer needs a blind scan or generic memory watch**. If a next observer is justified after ABI review, prefer an opcode-guarded *single native site* at `156784` (`mov eax,[ebp+08]`, 3 bytes) and `156787` (`mov [eax+68],edx`, 3 bytes), with safe relocation/re-execution of exactly the six original bytes. No DR0/DR1 contention with Stage59 is required.

Minimum log fields: original RIP/EIP RVA, player pointer P and object O, side and TID, `P+3F50`, table-derived ID, pre/post `O+68`, QPC, and health/overflow counters; correlate Stage65, Stage41, Stage67 strictly within one session. Observe 2F/25 and 33/26 without forcing any selector or action. Gate on opcode SHA/bytes, avoid recursion, preserve all flags/GPR/x87/SSE and original call order; static differential ABI tests and Windows x86 build are **required before gameplay**. A 3-byte-only detour is invalid without handling the adjacent instruction safely.

If source-level ABI/handler coexistence is uncertain, **NO-GO** until reviewed. No Stage70 code/DLL was produced in Stage69.

## Reproduce

```powershell
py -3 .\Stage69_Analysis\tools\verify_stage69.py "D:\PATH\TO\AALib.exe"
```

Existing Stage67 DLL **1,887,232 bytes** remains known-good; keep Stage65 **1,879,552** and Stage59 r2 **1,153,024** rollback builds. Do not upload the private original EXE.
