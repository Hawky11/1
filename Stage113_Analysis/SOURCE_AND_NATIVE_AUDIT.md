# Stage113 — guarded parent CALL / native JE observer, SOURCE ONLY

**2026-11-12 · no Windows compile or live runtime claims.** Source ZIP generated in the ChatGPT session: `ArcanaAI_Stage113_ParentCall_Gates_SourceOnly.zip`, 29 KB, SHA256 `a1bf02dbe3afd1d34e851688dcd67a26e9c1eb7954dd51a76b9d3d5857f551ce`.

## Provenance
- Original user `AALib.exe` from Library `/AALib.zip`, 175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
- Portable original-byte/rel32/branch verifier **30/30 PASS**; offline native/source/log analyzer tests **23/23 PASS**.
- Strict incremental source patch expected exact Stage110 adapter SHA `fe8dece53058b7d36427aa01452aec13e246e73e9e31acfafb1c6acdeba744c3` and Stage110 module SHA `e183d8ba84ac07e9dcdcd8fd740604bb42e48bed1c52c36892354353bf8511b2`.
- New adapter SHA `d4026d45217188213aad701e278b00067815c4f7bae9ce9f5d88013e414de2d0`; new Stage113 module SHA `b982dbcf19b797937196bcf2cae2e8dcb6e403d2b4228066c5c7e362eba3943a`.

## Native sites
| Original RVA | Purpose | Original replay |
|---|---|---|
| `95CD6` | parent direct CALL `98780` | push original return `95CDB`; jump to real callee |
| `EFDD8` | parent direct CALL `CACE0` | push original return `EFDDD`; jump to real callee |
| `9888D` | original JE after `FA480` result TEST AL | original ZF→`98965` skip, else `98893` |
| `CACF1` | original JE for `[EBX+10CC]==-1` | original ZF→`CADF9` skip, else `CACF7` |

- Opt-in `EnableStage113ParentGateTrace=1`, Stage110 required ON, Stage36 `EnableStage36ControlToCommandTrace=0`.
- Four reversible in-memory **CODE** redirects only; no F2DE0 repatch, no DR writes, no player state/gameplay data writes, no synthetic AI commands.
- Snapshot EFLAGS, all GP and FPU/XMM registers; exact CALL return-stack and native JE semantics; bounded diagnostic ring/counters.
- Strict original PE/header/site signatures and Stage104/107/110 detour ownership; mutex, thread suspension, EIP checks, under-lock revalidation, rollback.
- Source-only `ApplyStage113.ps1` and `RollbackStage113.ps1` use exact SHA guards and preserve original Stage110 module.
- Source ZIP contains full `src/Stage81SelectorWriter.inl` replacement and new `src/Stage113ParentGateTrace.inl`, PowerShell installer/rollback, `tools/verify_stage113.py`, `tools/analyze_stage113.py`, tests, `analysis/native_results.json`, and Russian report.

## Live validation next — Stage114
User Windows x86 MSVC build, backup working Stage110 DLL **2,073,088 bytes**, short natural COM-vs-COM. Require `BUILD SUCCEEDED`, `STAGE113_INSTALLED 4/4`, clean health counters and **positive site hits** before classifying zero CEBE0 entry. Upload fresh same-session `Build.log`, `ArcanaAI.ini`, `ArcanaAI.log`, `ArcanaAI_Stage113ParentGateTrace.log`, Stage110, Stage107, Stage104, Stage94 logs.

A Stage113 `INSTALLED` marker alone does not prove parent functions were invoked. `gateApass` still permits later `[ESI+10CC]==-1` skip. Positive parent/gate observations alone do not establish accepted action, animation, hit or improved AI.

**No stage113 Windows build, DLL, game run or installed runtime result is claimed by this checkpoint.** `main` and older stage/evidence branches remain immutable here.
