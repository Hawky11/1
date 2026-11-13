# ArcanaAIOverhaul — Stage155 r1 external owner / epoch lease / raw ingress / ABA audit (2026-11-13)

**SOURCE_ONLY — no game DLL patch, no live game process access, no game DLL unload.** User Stage154 Windows MSVC x86 self-owned fixture previously confirmed PASS; the **Stage155 Windows x86 native fixture has NOT run yet**. `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, `origin(0x27)=UNKNOWN`.

## Verified artifacts / provenance

- Chat ZIP: `ArcanaAI_Stage155_SourceOnly_ExternalLease_RawIngress_ABAQuiescence_r1.zip`, **24,063 bytes**, SHA256 **`b7d20e46bd797ffba9757fcf121486d0e57d174bbb0bfc3147e75d8667555ef3`**, 12 ZIP entries CRC PASS, manifest 11/11 PASS after extracted-file recheck.
- Separate detailed Russian report: `ArcanaAI_Stage155_Audit_RU.md`, SHA256 `0d7ce1d9a7746bceebb01eaa1fe70b80d8d39e6381b0d1592aa4556be5b4dd33`.
- Stage146 pinned source evidence ZIP SHA256 `6d9c39d59b4883dab6cc4ea95f08deac56fe8c477fd6b4746bd1c764f96a207b`: 85/85 source blobs verified by size+SHA256 **in offline work environment**, 50/50 exact source anchors checked. 38 inherited Stage154 source anchors + 12 Stage155 anchors (Proxy export and start paths, Stage10 F34D0/F3930 native entry wrappers, Stage59 owned-watchpoint teardown). **These are not live DLL memory findings.**
- Stage154 previous artifact SHA256 `e6f6c38cdee7fb2c2596a73f2d818730cf82622218d1a2133a6e17f83d2f08ab` pins analysis baseline. See [Stage154 checkpoint](STAGE154_CHECKPOINT.md) for actual user Windows MSVC x86 results.
- Packaged **only** source-only model, self-owned Win32 x86 fixture source, Python tests, verification utilities, synthetic model JSON, metadata source hashes, PowerShell script and report; **no original user project source, INI, EXE, DLL, log, PDB, AALib or game data published**.

## Actual source-level blockers

- `src/Proxy.cpp:58–89`: exported DirectInput8Create calls eight worker `Start*` routines, but no verified external module-owner reference/lease gate. Closing this one export would not cover raw native detour entrances or retained foreign function pointers.
- `src/AIProducerContextTrace.cpp:161–165,838–852`: F34D0/F3930 hook declarations, C wrappers and `__declspec(naked)` entry; source-level example of native entry separate from exported gate. **Not proof of live hook ownership** or complete native entrypoint enumeration.
- `src/Stage81SelectorWriter.inl:556–558`: an additional worker Start path; eight reviewed worker starts `CloseHandle(thread)` immediately without proven retained handles for cooperative stop/join. Stage154 Windows self-owned fixture success does not modify them.
- `src/Stage59XWriterWatch.inl:292,330–333`: VEH registration, explicit keep-until-process-exit comment, owned DR uninstall. DR uninstall is not equivalent to `RemoveVectoredExceptionHandler`; real callback retirement/SEH/native unwind proof is missing.
- Live owner/caller census, ABI/SEH/unwind, workers shutdown, external raw pointers, Stage67/70 native transaction rollback, VEH quiescence, DR ownership remain **UNPROVEN**.

## Offline Stage155 checks — actually executed

- C++17 deterministic event-order test: **four actors**, two ordered events per actor, `8!/(2!)^4=2520` schedule interleavings per mode, **5,040 total**.
- `UNKNOWN` (foreign raw/native and future callback proof missing): **2,520/2,520 DENY**, zero ready, **1,020 states had empty counters but still DENY**. No after-close host lease and no invariant errors.
- `SYNTHETIC` (toy proof flags explicitly set by model): **1,560 MODEL_READY**, **960 DENY**; *not* a real-world game unload permission. No `FreeLibrary` call.
- 18/18 special adversarial model cases PASS: duplicate release, forged serial, stale epoch / ABA, after-close callback, raw bypass, zero-counter false readiness, owner loss, rearm, underflow, etc.
- GCC and Clang `-std=c++17 -Wall -Wextra -Werror -O2` both compiled successfully with **byte-identical stdout**; standalone local Python **44/44 PASS**. The final distributed ZIP was re-extracted and reran with **44/44 PASS**, manifest 11/11 PASS and pinned Stage146 audit 85/85 + 50/50 PASS. No Windows MSVC Stage155 result claimed.
- Win32 source fixture is a separate temporary **EXE** with host-style CRITICAL_SECTION lease, one host lease worker, one delayed simulated callback worker, deterministic manual-reset event sequencing, retained thread handles, post-close entry denial, raw unproven denial, epoch increment/stale token rejection. It does **not** call a real VEH, use raw game trampolines, load/unload DLL or access another process. Windows compile+runtime **PENDING USER TEST**.

## User Windows MSVC x86 handoff

Extract Stage155 ZIP and run from same PowerShell session:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch x86 -HostArch x86
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
cd 'D:\INCUBATOR\ArcanaAI_Stage155_SourceOnly_ExternalLease_RawIngress_ABAQuiescence_r1\Stage155_SourceOnly'
.\Invoke-Stage155Audit.ps1
```

Optional `-Stage146Zip 'D:\INCUBATOR\ArcanaAI_Stage146_Evidence_493fc20dff504bdabc3abc4924a9426f.zip'` for Windows read-only 50-anchor recheck; otherwise this additional Windows check is skipped. The runner compiles only isolated EXEs under `%TEMP%`, checks exact textual evidence markers, and cleans its temporary files. Python GCC/Clang-dependent tests can be **skipped** on Windows without those compilers; MSVC x86 executable tests are separate.

## Safety / next step

**NO production shutdown/unload approval.** Keep Stage129 working DLL **2,099,200 bytes** and Stage122 rollback **2,089,984 bytes** untouched. `Stage36 OFF`; origin(0x27)=UNKNOWN. Need real complete native entrypoint ownership, foreign pointer quiescence, external owner lease, worker/callback retirement and transaction rollback before any game DLL teardown. **Stage156 next:** review actual Windows Stage155 output, then continue source-only native-entrypoint admission coverage and callback retirement proof. Do not implement automatic game install/unload.
