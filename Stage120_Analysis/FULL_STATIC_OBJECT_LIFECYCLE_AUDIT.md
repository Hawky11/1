# Stage120 — original AALib.exe object lifecycle / vtable / G3-before-G4 CFG audit

**2026-11-12 · ORIGINAL-NATIVE STATIC ONLY.** Reproducible ChatGPT ZIP: `ArcanaAI_Stage120_ObjectLifecycle_G3G4_CFG_StaticAudit.zip` **21,834 bytes**, SHA256 **`c9927f2cabdaa4ca6fc1ba8d836800f88fb6cd7783b914bd0b152283172d9675`**, **10 files**: full Russian report, exact machine catalog, independent verifier, 55 negative/positive regression tests and test output, original GNU objdump excerpts, temporal witness graph and JSON results. **No EXE or DLL** inside.

## Original binary verification

- Original private user `AALib.exe`, PE32 i386, preferred imagebase `0x00400000`, size **175,556,608 bytes**, SHA256 **`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`** reverified via the private Library ZIP.
- **122/122** exact x86 instruction signatures, **6/6** original relocated vtable entries, **11/11** native Jcc/JMP destinations, **4/4** E8 rel32 CALL targets, **28/28** additional hardcoded independent opcode anchors, **55/55** Python stdlib tests. The negative suite detects altered original bytes, vtable, branch, direct CALL, G3 sync, G4=7 writer, flag-clear and catalog mismatches.
- Exact source/instructions and test fixtures are in the distributed ZIP, not the GitHub branch. All addresses below are *original RVAs* unless explicitly labeled preferred VAs. The game EXE may be rebased at runtime.

## Real object layout / virtual owner

Original constructor `19EAB0` stores preferred vtable VA `0x00810A18` at `19EAD3: MOV [ESI],0x00810A18`. All six function pointers are backed by relocated `.rdata`:

| vtable offset | preferred function VA | RVA |
|---|---|---|
| `+00` | `0x0059EDA0` | `19EDA0` |
| `+04` | `0x0059F410` | `19F410` |
| `+08` | `0x0059EE70` | `19EE70` |
| `+0C` | `0x0059EE00` | `19EE00` |
| `+10` | `0x0042B270` | `02B270` |
| `+14` | `0x0059EF00` | `19EF00` |

`19F410` is a **vtable+4** method; no original *direct E8 CALL* to `19F410` was observed. Its actual indirect dispatch count/conditions in Stage116 remain unknown. The slot0 function `19EDA0` explicitly zeros `[ESI+0x10]` and `WORD[ESI+0x2A]`, sets global `G3=[0x0E10B37C]=0`, and refreshes the inner `[ESI+0x24]` pointer. This proves native reset capability, NOT when reset ran in the earlier live COM-vs-COM.

## Same-object propagation and strict instruction order

Original `19F410` receives object pointer `ECX` and preserves it as `EDI` at `19F433`. Its reachable common suffix:

```asm
1A0C76  mov ecx,[edi+10h]      ; old (pre-child) state
1A0C79  mov [0E10B37Ch],ecx    ; G3 = old object state
1A0C7F  mov ecx,edi            ; exact same object to child
1A0C81  call 19EF30           ; sole literal E8 call to this native function
1A0C86  call 10B30            ; after child returns
```

Child `19EF30` receives `ESI=ECX` at `19EF3E`. Hence the parent's `EDI+0x10` and child's `ESI+0x10` address the **same object in that call**. A separate state-change path inside the *same parent* also stores to G3 at `19F96D MOV EAX,[EDI+0x10]` → `19F973 MOV [G3],EAX`. **G3 is not guaranteed to be written exactly once per invocation.**

## Early returns before the Stage118 conditional writer

- Entry checks at `19EF36` and `19EF46` test global bytes `0x0E10AFAA` and `0x0E10AFA9`, jumping directly to return `19F40B` if set.
- After `19EFB4 CALL 16F4D0`, both relevant paths reject a nonzero existing object state `[ESI+0x10]` at `19EFBD/19EFC1` or `19F003/19F007`. Other virtual-call results and global/mode conditions also gate reachability. Thus a later pass with object state=3 can bypass the writer.
- The timer check `19F208 CMP [ESI+0x1C],0xFF;19F20F JL 19F409` gates progression **when BL != 0**. The threshold does not apply unconditionally when BL=0.
- At `19F223` both object flags `[ESI+0x2A/0x2B]` zero cause `19F22D JE 19F409`. Further selected-slot predicates remain those verified in Stage118/119: selector=8; map[selected index]=-1.

## Normal continuation after the conditional selected-slot write

When all upstream gates pass: `19F250` loads selector; `19F255/19F25B` compute `G4+0x278*slotIndex`; `19F289 CMP EAX,8`; `19F295 MOV [ESI+0x10],3`; `19F2A0 CALL F9900` returns map entry; `19F2AB JNE 19F2B3` skips write unless -1; `19F2AD MOV DWORD [EDI],7` sets G4 or G5. The normal fallthrough reaches a second `F9900`, `19F2C4 CALL 74600`, `19F2CC JMP 19F3D0`, and **`19F3D0 MOV WORD [ESI+0x2A],0` clears both flags**.

The static continuation proves **normal CFG order assuming called helpers return**. It does **not** prove the helpers have no memory side effects, that a recorded G4 head remains 7 forever, or that the call was executed in Stage116. Notably, `19F3D0` does **not** itself copy the new object state to G3.

## Example of a POSSIBLE two-invocation history, not a measured trace

| hypothetical moment | object.state | G3 | G4 |
|---|---:|---:|---:|
| T0 just before child, old object.state=0 | 0 | 0 | previous |
| T0 after 19F295 / 19F2AD conditions satisfied | 3 | 0 | 7 |
| T0 normal child exit clears flags | 3 | 0 | 7 |
| T1, IF same object is visited again | 3 | 3 | 7 |

On T1, `1A0C79` can copy the *previously updated* state 3 to G3 while the child takes an early nonzero-state return, explaining a stable global pair without repeated writes at 19F2AD. This temporal *witness* is permitted by the decoded code. The actual same-object lifetime and T0/T1 execution have **not** been observed or reconstructed from Stage116.

## Evidence boundary and next step

Previous Stage116 runtime contained **1,455** full 5-DWORD `(G1,G2,G3,G4,G5)=(0,0,3,7,0)` snapshots post FA480; its parent-A native gate counter recorded **16,760/16,760 AL0/ZF1 skips**. Those logs did NOT record writer-site `19F295/19F2AD/19F3D0` hits, virtual dispatch, object pointer lifetimes, selector/map/flags *at write time*. Stage120 **does not establish the last G4 writer, active native mode name, successful FA480 in this state, accepted attack, hit or improved AI**.

**Stage121 recommendation:** Decide whether last-writer provenance materially advances original AI question; if needed, design one narrowly scoped read-only positively-controlled observer on native object/writer flow only after strict source/hook coexistence checks. Do not force state values or rehook F2DE0, write DR registers, synthesize commands or edit gameplay memory. Preserve Stage115 Windows-tested DLL **2,082,304 bytes**, Stage113 **2,079,744 bytes**, Stage110 **2,073,088 bytes**; Stage36 OFF. **No new DLL was built in Stage120.**
