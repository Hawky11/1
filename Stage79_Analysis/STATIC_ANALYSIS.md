# Stage79 — the ten computed selector writers share one original native caller boundary

**2026-11-11: ORIGINAL PE STATIC VERIFIED, SOURCE READY; Windows x86 BUILD and same-run RUNTIME not yet tested.** Frozen Stage78 runtime evidence [8fae5d4](https://github.com/Hawky11/1/commit/8fae5d444ab55cb0cc25806499228eec7c7e563d) stays unchanged in [Stage78 evidence branch](https://github.com/Hawky11/1/tree/stage78-runtime-no-literal-hit-selector-alternative-evidence). Original private AALib.exe SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`; inspected locally from Library AALib.zip, never uploaded to GitHub. PE32/i386, ImageBase `00400000`, `.text` RVA `1000`, 2,875,392 bytes raw.

## Finding: four functions, all ten register-derived MOV sites, one common direct native ancestor

Original direct DWORD MOV instructions targeting `P+3F50`, previously inventoried by Stage71/73:

| Native function | Computed selector writer sites | Verified source logic |
|---|---|---|
| **0F3930** | `0F3DE9`, `0F3E02`, `0F3F50`, `0F3F69` | two pairs: `SETNE` yields `1+(AL!=0)`; `SETE` yields `1+(AL==0)` |
| **0F4360** | `0F466F`, `0F4688` | one SETNE/SETE pair |
| **0F46F0** | `0F48A9`, `0F48C2` | one SETNE/SETE pair |
| **0F7150** | `0F7D22`, `0F7D38` | descriptor-byte gated SETNE/SETE pair |
| **Total** | **10** | values **1 or 2**, subject to branch gates; zero-writer sites distinct |

The original direct `E8 rel32` edges form a converging call tree, all verified against the exact original bytes and targets:

```text
0EFFE8  CALL 0E89E0                       <- ONE common bounded direct call site
  0E89EE  CALL 0F6960
    0F6987 CALL 0F46F0                   -> 0F48A9 / 0F48C2
  0E8A03  CALL 0F7150                   -> 0F7D22 / 0F7D38
  0E8A8E  CALL 0F7150                   -> same two
    0F79E4 CALL 0F4360                  -> 0F466F / 0F4688
    0F7AD1 CALL 0F3930                  -> 0F3DE9 / 0F3E02 / 0F3F50 / 0F3F69
  0E8A28  CALL 0F70A0
    0F7106 CALL 0F3930                  -> same four
    0F70E4 CALL 0F7010
      0F705C CALL 0F3930                -> same four
```

**11** explicit CALL edges verified. An independent byte-wide direct-E8 target search of the original `.text` found the exact candidate caller sets for **all eight** target functions `0E89E0`, `0F6960`, `0F7150`, `0F70A0`, `0F7010`, `0F3930`, `0F4360`, `0F46F0` with no extra direct E8 candidate hits. The source is a bounded original direct-call graph: **it does not rule out indirect calls, aliases, alternate entrypoints, exception paths, or writes other than the known ten MOV sites.** The common call `0EFFE8` is only a *potential* enclosing interval, not proof it caused the five earlier runtime transitions.

Original `0EFFE8` site: bytes **`E8 F3 89 FF FF`**, exact continuation `0EFFED: 84 C0 74 16` (`TEST AL,AL; JE`); callee `0E89E0` starts `MOV EAX,[ESI+3B7C]`, uses `ESI=P`, and returns in AL. Surrounding original gates at `0E89EB` and `0E89F9` pick different branches and may bypass the writer functions. **A hit alone is insufficient: require P+3F50 before/after change for a positive family-level assignment.**

## Stage79 one-site implementation, risk envelope

- New [Stage79SelectorFamilyBoundary.inl](../Stage79_Overlay/src/Stage79SelectorFamilyBoundary.inl) patches **only one existing 5-byte native CALL** at `0EFFE8` with a same-length CALL to a diagnostic wrapper, preserving original function call exactly once. Unlike Stage78, it does **not** patch, replace or replay any of the ten selector-write instructions.
- Wrapper uses the established PUSHFD/PUSHAD/FXSAVE/FNINIT/CLD snapshot pattern, `call dword ptr [g_s79OriginalTarget]` **once**, restores all intended registers/flags/x87/XMM and returns to the original continuation `0EFFED`. The original callee sees a wrapper-local return address (not exactly the prior EFFED) and logging costs time; these need actual Windows runtime validation.
- Log `ArcanaAI_Stage79SelectorFamilyTrace.log`: `STAGE79_COMMON_WINDOW` with P/O/TID, QPC pair, native result EAX, pre/post mask and 13 player fields. `f0=P+3F50` selector, `f1=P+3F54` companion, `f2=P+4` object; `f3=P+16E` DWORD includes byte flags at +16E and +170; `f4=P+3B78`, `f5=P+3B7C`, `f6=P+3A30`, `f7=P+3A34`, `f8=P+3ED0`, `f9=P+3ED4`, `f10=P+3F4C`, `f11=P+3F7C`, `f12=P+3B90`.
- TLS depth16, exact original return `EFFED` checked, object stability checked, protected ring8192 with overflow/drop/read-failure counters; `STAGE79_HEALTH` records call totals and errors every installed flush, including zero-call periods.
- Installation checks exact native bytes, x86+SSE, Stage38=1 Stage76=3 Stage78=1 Stage59 worker not started. Uses the existing patch mutex, peer suspension + EIP conflict check, one-page protection change, instruction cache flush and best-effort rollback. Debug registers unchanged; no extra game-field write.
- The [Stage79 adapter](../Stage79_Overlay/src/Stage38CallbackWindow.inl) adds exactly 3 edits to the Stage78 adapter: one new include, install Stage79 after Stage78 successfully installs and before Stage59 worker, flush Stage79 alongside existing Stage78/76/38. Prior Stage38/77/76/78 source is preserved.
- **Windows x86 build, exception safety, stack/FPU/ABI equivalence, latency, effect on game and actual attribution have NOT been tested.** Do not describe this as a gameplay improvement.

## Stage78 runtime baseline and evidence criteria

[Stage78 runtime report](https://github.com/Hawky11/1/blob/stage78-runtime-no-literal-hit-selector-alternative-evidence/Stage78_Runtime/RUNTIME_ANALYSIS.md) proved Windows x86 BUILD SUCCEEDED and stage78 installed but *no literal-one site hits*; in one session Stage38 5,064 windows, Stage77 5,064 native 156C00 pairs, Stage76 9,704 phase records, five upstream 0/1/2 selector changes between Stage76 phase1 and next Stage77-before-156C00. One callback-local P2 154070 reset selector 1→0 with subsequent event 22. Exact writer EIP in those transitions is unknown.

Stage79's **positive** result would be a `STAGE79_COMMON_WINDOW` with `f0=00000000>00000001/00000002` (or `00000002>00000001` etc), QPC bracket and actor matching Stage38/76 gap. This attributes the **function-family interval**, **not** one of ten native site EIPs. If every Stage79 window leaves selector unchanged, it does not prove that the ten instructions never execute (transient writes might return to same value, and branch coverage differs from the Stage78 run), nor exclude another native/aliased writer outside this call interval. The next Stage80 probe should target the particular changed function branch if Stage79 is positive, avoiding ten simultaneous hooks.

Known-good rollback: user-tested Stage78 DLL **1,911,808 bytes**, Stage77 **1,905,664**, Stage76 **1,901,568**. Preserve all raw uploaded logs unchanged.