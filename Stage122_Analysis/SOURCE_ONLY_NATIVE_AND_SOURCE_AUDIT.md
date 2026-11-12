# Stage122 — SHA-guarded positive-control object writer observer, SOURCE ONLY

**2026-11-12. Implemented native Win32/x86 C++ source overlay; no Windows build or game-run success claimed.** Source ZIP delivered in this conversation: **`ArcanaAI_Stage122_PositiveControl_ObjectWriterTrace_SourceOnly.zip`**, **48,883 bytes**, SHA256 **`5d0abe6f5bab87aad38a766509ec6e67baa45bf34d16cbb52d74d5aee65426c6`**, **17 files**; ZIP CRC verified. Source, exact SHA-guarded PowerShell Apply/Rollback, RU docs, native-original EXE verifier, strict log correlator, 59 tests, machine results, and explicitly SYNTHETIC fixture. No private original EXE or built DLL is distributed.

## Original source and native provenance

- User original `AALib.exe` in Library `/AALib.zip`: **175,556,608 B**, PE32 x86, preferred base `0x00400000`, SHA256 **`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`** checked in this turn.
- Exact current Stage115 adapter `src/Stage81SelectorWriter.inl` SHA256 **`d4026d45217188213aad701e278b00067815c4f7bae9ce9f5d88013e414de2d0`** → updated Stage122 adapter SHA256 **`3a78a8b6b68fda0999d33a2d392dd781fffa4c5bfd67a31b7fa2cdb5b8fadd1d`**. The incremental edit inserts Stage122 prototype, calls `Install` after **successful Stage113::Install**, appends `Flush/Health` to existing worker loop and includes new module `Stage122ObjectWriterTrace.inl` after Stage113; all unrelated adapter bytes match.
- New module `src/Stage122ObjectWriterTrace.inl` SHA256 **`4ae2a8f42774733f572b25d7f64df0c58ec005e52ae55080a6a718be4a75c1c1`**. Existing Stage115 `Stage113ParentGateTrace.inl` SHA256 **`273059538a37cb1cfb82ab9c6eb891d0e535c59bcae1e41c42f06c27ff336d3a`**, and Stage110 `Stage110Cebe0GuardTrace.inl` SHA256 **`e183d8ba84ac07e9dcdcd8fd740604bb42e48bed1c52c36892354353bf8511b2`** are unchanged.
- Native verification against SHA-pinned original EXE: **9/9** complete instruction/context site signatures, **2/2** original rel32 CALL targets, **33/33** no-overlap comparisons against known reserved hook ranges, exact adapter delta, source installer and rollback hash guards. **59/59 offline Python unit tests PASS** including negative corrupt-bytes, session/TID/object mismatch, duplicate/ambiguous joins, missing sampled events, invalid slots, partial reads and health error flags.
- No Windows compiler/simulator available in this session; **source-only checks do not prove DLL compilation, install or gameplay stability**.

## Three native original instruction sites implemented

| RVA | Original bytes | Hook role | Native replay, exactly once |
|---|---|---|---|
| `1A0C81` | `E8 AA E2 FF FF` (5 B) | `kind=1` positive control | push original return `1A0C86`; tail-jmp to original `19EF30` |
| `19F295` | `C7 46 10 03 00 00 00` (7 B) | `kind=2` object-state write | `MOV DWORD PTR [ESI+10],3`, resume `19F29C` |
| `19F2AD` | `C7 07 07 00 00 00` (6 B) | `kind=3` slot writer | `MOV DWORD PTR [EDI],7`, resume `19F2B3` |

New patch bytes are E9 rel32 plus correct 0/2/1 NOP suffix. Original GP/FLAGS/FPU/SSE preserved across C++ observations using PUSHFD/PUSHAD, 16-byte aligned FXSAVE/FXRSTOR; no extra game data or DR writes, no F2DE0 redetour or synthetic commands. Logger is worker-flushed rather than direct file writes inside hooks.

Install requires `EnableStage122ObjectWriterTrace=1`, Stage113=1, Stage115=1, Stage36 `EnableStage36ControlToCommandTrace=0`; checks loaded EXE fingerprint, exact native bytes + neighbors, original Stage115 helper signatures/relocated globals, and correct Stage104/107/110/113 detour ownership. Serial mutex, thread suspension+original-site EIP check, under-lock signature revalidation, post-patch ownership checks and exact-original rollback on failure. Unknown third-party detours outside known sites cannot be excluded.

## Logging and evidence contract

New `ArcanaAI_Stage122ObjectWriterTrace.log`: `STAGE122_BUILD`, `STAGE122_INSTALLED 3/3`, `STAGE122_EVENT`, `STAGE122_HEALTH`. Logs carry process-session token `sid`, TID, object pointer, QPC, EDI, ESI/ECX, readMask, selector, map0/1, G3/4/5, object flags/state. Actual selected slot determined from **EDI** and ASLR-rebased G4/G5: slot1=G4, slot2=G5, slot0=neither.

HEALTH counts **every** native hit per site before sampling. Detailed EVENT is intentionally sampled (first 32, then powers-of-two per health window); an absent detailed join is not proof of missing execution. Ring=4096, drop/mismatch/readFail counters explicit. Python log correlator only joins uniquely ordered **same sid, same TID, same object** event triples inside configurable QPC window, labels incomplete/ambiguous events UNKNOWN; its included fixture is marked **SYNTHETIC_ONLY**, not real user gameplay.

**A positive new-run G4 writer hit proves only that this specific original instruction was reached and replayed; it does not prove the *last*, *only*, or historically observed Stage116 G4 writer** because alternative direct, indirect and bulk writers exist. Parent CALL count=0 does not prove the virtual method never entered (early exits above call site). No evidence here of accepted native move, attack, hit or improved AI.

## Installation / Stage123 Windows handoff

1. **Close AALib.exe**; backup current Windows-tested Stage115 `build/dinput8.dll` **2,082,304 bytes**. Stage113 **2,079,744 bytes** and Stage110 **2,073,088 bytes** remain older rollbacks.
2. In extracted `Stage122_Overlay` run `powershell.exe -ExecutionPolicy Bypass -File ".\\ApplyStage122.ps1" -Root "D:\\INCUBATOR\\ArcanaAIOverhaul"`. The PS script requires exact original Stage115 adapter/module and Stage110 checksums, Stage113 + Stage115 backup manifests; creates `.stage122_backup`, changes only adapter and adds Stage122 .inl. No executable or gameplay patch is made by the PowerShell script.
3. Set INI in `[AIResearch]`: `EnableStage113ParentGateTrace=1`, `EnableStage115Fa480Trace=1`, `EnableStage122ObjectWriterTrace=1`, `EnableStage36ControlToCommandTrace=0`; preserve Stage110/107 required observer flags.
4. Run user's **MSVC x86 Build.bat**; only a fresh Build.log can certify compilation. Short **natural** COM-vs-COM test, no forced globals or game writes. Upload Build.log, fresh INI/general log, Stage122 plus Stage115/113/110/107/104 logs.
5. To rollback sources, game closed: `RollbackStage122.ps1 -Root ...`; then disable Stage122 in INI and **rebuild DLL or restore backed-up Stage115 DLL**. Source rollback alone does not modify already compiled DLL.

**Stage122 is SOURCE ONLY. The Stage123 target is first real Windows compile, 3/3 install and positive per-site runtime hit test.**

[Stage121 review](https://github.com/Hawky11/1/blob/stage121-passive-writer-probe-feasibility-design/Stage121_Design/FEASIBILITY_AND_POSITIVE_CONTROL_AUDIT.md).
