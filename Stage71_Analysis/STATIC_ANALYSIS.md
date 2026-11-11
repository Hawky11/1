# Stage71 — selector P+3F50 producer inventory and state-flow evidence

**Date:** 2026-11-11. **STATIC VERIFIED; exact producer EIP for the Stage70 selector=1 event NOT runtime verified.** Based on the user-frozen [Stage70 r2 runtime evidence](../Stage70_R2_Runtime/RUNTIME_ANALYSIS.md), original private `AALib.exe` SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. No new hook, DLL, game run, original binary modification, or AI behavior change. [Read-only SHA/opcode/call verifier](tools/verify_stage71.py).

## A. Explicit source of selector value 1 in the original engine

The native original function **`AALib+155B70`** has an exact literal-one setter:

```asm
155B79  mov edi,[ebp+08]           ; callback context
155B7C  mov esi,[edi]              ; P = current player
155B7E  mov eax,[esi+04]           ; current object O
...
155D1D  mov dword [esi+3F54],4     ; success branch A
155D27  push 21h                   ; engine event/action code
155D29  mov dword [esi+3F50],1     ; **direct selector=1 producer**
155D33  push 0
155D35  add esi,3A2Ch
155D3B  push esi
155D3C  call AALib+0F2DE0         ; downstream event dispatcher/queue
155D46  mov al,1                   ; function success
...
155D74  mov dword [esi+3F54],3     ; success branch B
155D7E  jmp AALib+155D27          ; joins same event+selector setter
```

The `155D7E` short jump target is verified from original bytes: **both state3 and state4 paths converge on the same literal-one store**. `P` pointer provenance is explicit at `155B7C`. Event `0x21` is a machine-code constant passed to the call `0F2DE0`; its game-level semantic name has not been independently verified.

This is a **true native positive producer** (not a caller of the deferred store, not a mere string/offset match, and not an injected value). It is not yet proven to be the exact source executed for the Stage70 run's P1 selector=1.

## B. Two direct caller paths into the literal-one setter

The original `155B70` function is directly called from both native callbacks:

```asm
1566F0  call AALib+155B70
1566F8  test al,al
1566FA  jne  AALib+156728       ; return path if selector-setting succeeded

1560B9  call AALib+155B70
1560C1  test al,al
1560C3  jne  AALib+156110       ; another return path on success
```

The Stage70-proven deferred writer `156787` is **inside the same `1566B0` callback** (which contains `1566F0`). This gives a concrete callback-level path from a selector producer to a later selector consumer. The immediate callback returns early after setter success in these paths; the captured `156769→156787` request+deferred sequence therefore cannot simply be interpreted as having traversed `1566F0→155B70` and **continued into** the request within the same successful setter call. The selector may persist to a later callback dispatch; that later dispatch was not captured by a dedicated selector-writer observer.

The original code loads selector `P+3F50` at `1566C5`, calls the immediate native requester from `156769`, then **rereads** selector at `156770` to derive deferred ID before the store at `156787`. Thus the selector is an actual part of the original request/deferred pair mechanism, not just an unrelated adjacent field. During the one Stage70 writer event the selector measured `1→1`, source EDX=`25`, and Stage65 immediately earlier requested `2F`.

## C. What gates the explicit-one selector path?

Original instruction-level flow into `155D29` includes more than a constant assignment:

- `155BD1/155BD7/155BE0` checks player `P+3B78` **bit2** and returns false if set.
- `155BE9` calls `AALib+0E5F70`; `155C06` calls `AALib+0E86F0` and the `155C0D` branch exits if the test returns AL=0.
- Reads of object floating-point fields `O+80` and `O+88` are compared with values loaded through `AALib+125490` and with object flag `O+24`. These are genuine conditional tests. Their high-level meaning (position, distance, attack range, or something else) is **not assigned** without evidence.
- The branch conditions also inspect bytes at `P+175` and indexed flags around `P+154` with helper predicates returned by `AALib+125470`. They determine whether branch A or B sets the `P+3F54` state before both set selector=1.

The precise reason why one particular in-game frame met those conditions remains **unobserved**. The static code constrains the possible conditions but does not show which executed.

## D. Native direct-MOV writer inventory — uniqueness warning

Disassembly restricted to the original `.text` section found **20 explicit MOV DWORD [baseRegister+3F50], value** sites; all 20 exact sites and bytes are pinned by [verify_stage71.py](tools/verify_stage71.py):

| Kind | Count | Original RVA sites |
|---|---:|---|
| Literal **1** | **1** | `155D29` |
| Literal **0** | 9 | `0F3E0A`, `0F3F71`, `0F4690`, `0F48CA`, `0F7D43`, `124CFE`, `155669`, `1BDDDE`, `1BF38E` |
| Register-derived | 10 | `0F3DE9`, `0F3E02`, `0F3F50`, `0F3F69`, `0F466F`, `0F4688`, `0F48A9`, `0F48C2`, `0F7D22`, `0F7D38` |

**Critical non-uniqueness:** e.g. `0F3DE9` computes `ECX = 1 + (AL != 0)` and `0F3E02` computes `EDX = 1 + (AL == 0)`; both can also write **1 or 2** into the selector. The `0F466F/0F4688`, `0F48A9/0F48C2` and `0F7D22/0F7D38` families have analogous branching. Those paths depend on native flags/field checks (including `P+16E`, `P+170`, object+24 and P+3B78) and may represent other source situations.

Only **one direct literal-one MOV site** exists in this disassembly inventory, **not** only one possible writer of the numeric value 1. All sites are direct DWORD stores matched by base-register displacement; aliases, structure copy, different instruction encodings, or indirect helper writes are **not exhaustively ruled out**. This is a bounded static inventory, not a universal source proof.

## E. Relation to frozen Stage70 runtime evidence

The 2026-11-11 02:54:29 capture already proved:

1. Stage65 P1 seq319 caller `156769` requested `2F` (QPC `1518396105810–5813`).
2. Stage70 P1 seq1 wrote deferred `O+68:FFFFFFFF→25` with selector `P+3F50=1→1` (QPC `1518396105821–5822`).
3. Stage41/42 committed and activated immediate `2F`; Stage67 seq4561 later promoted deferred `25`, Stage42 seq234 activated `25`.

Stage70 logs selector at the **deferred writer**, not when it was first set. Therefore the above evidence proves **value 1 at the consumption time**, **not** that EIP=155D29 or any of the ten register-derived writers executed in this particular interval. There is no reason to rebuild Stage70 r2 or repeat that combat capture solely to demonstrate what is already known.

## F. Stage72 recommended scope — decide whether runtime writer attribution is worth instrumentation

First use existing Stage70 validated DLL `1,892,864` bytes as rollback. If direct selector-producer attribution is required, choose a safe **selector-write watcher scoped to player pointers** after reviewing Stage59's DR0/DR1 VEH ownership, possible DR2/DR3 owners and exact native six-byte stores. Direct site `155D29` is a ten-byte `MOV [ESI+3F50],1`, but **a single-site hook alone cannot distinguish competing register-derived writers** if it fails to fire.

Before any additional code, audit callback scheduling (path `1566F0→155B70`, alternate `1560B9→155B70`, event queue `155D3C→0F2DE0`) and decide whether existing trace contains a state3/4 or event21 indicator; no claim of direct selector writer until an EIP/site-linked event is obtained. A minimal observer should log exact site, old/new `P+3F50`, `P+3F54`, P/O, actor, TID, QPC, ownership/health and event context. Never force selector=1, 2F, 25, or any AI behavior. An alternative is **leave Stage71 as source-only**, since deferred ID origin is fully resolved for the captured run.

## Reproduce static analysis

```powershell
py -3 .\Stage71_Analysis\tools\verify_stage71.py "D:\PATH\TO\AALib.exe"
```

Original SHA, exact 20 sites, callback path opcodes, two direct `155B70` CALLs, native `156769` requester CALL, event dispatcher CALL, state3/state4 branch convergence and early-return jumps are verified. The exact original PE and 20+ critical opcodes/branches were also checked directly in this analysis environment; **no Stage71 DLL, Windows build or new runtime was produced**. Keep known-good Stage70 r2 and earlier rollback DLLs; don't publish the private original EXE.
