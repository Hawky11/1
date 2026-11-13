# ArcanaAIOverhaul — Stage 152 Windows x86 validation checkpoint (2026-11-13)

**Result: USER-CONFIRMED `STAGE152_LOCAL_WINDOWS_TESTS_PASS`** on Windows 11, Visual Studio 2026 Community v18.9.1 / MSVC 14.51.36231, 32-bit host+target (`HostX86\x86\cl.exe`).

**Scope:** isolated self-owned Win32 test EXE/fixture, **not** a game DLL install, AALib patch, or proof of safe unloading from a live game.

## Artifacts

- Stage152 chat ZIP: `ArcanaAI_Stage152_SourceOnly_Win32X86_PrivateSandbox_FaultHarness_r1.zip`
- Stage152 Russian report: `ArcanaAI_Stage152_Audit_RU.md`
- Upstream Stage151 ZIP input: `ArcanaAI_Stage151_SourceOnly_TransactionRollback_QuarantineFaultModel_r1.zip`
- Stage151 ZIP SHA-256 **verified by user's Stage152 runner**: `7016b1f01d486c5ff27a3bb4017e7191473569d794735567e45ccb5f3cc2d4d1`; 9 upstream payload files verified.
- Stage152 package self-manifest: `STAGE152_PACKAGE_MANIFEST_OK 14`.

These are chat/Library artifacts; the ZIPs, user source tree, private INI, AALib.exe, DLL and logs are **not** committed into this GitHub repository by this documentation update.

## Windows invocation: reproducible working recipe

First try in an ordinary PowerShell failed with `PSSecurityException`: PowerShell execution policy blocked .ps1. Temporary session-only workaround:

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
```

After this the script failed at `Get-Command cl.exe` because MSVC environment was not initialized; compiler **was installed**. The user successfully initialized the existing PowerShell with:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch x86 -HostArch x86
Get-Command cl.exe
```

`Get-Command` found `C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\bin\HostX86\x86\cl.exe`. Use the **same** PowerShell session for subsequent commands, then:

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
Set-Location 'D:\INCUBATOR\ArcanaAI_Stage152_SourceOnly_Win32X86_PrivateSandbox_FaultHarness_r1\Stage152_SourceOnly'
.\Invoke-Stage152Audit.ps1 -Stage151Zip 'D:\INCUBATOR\ArcanaAI_Stage151_SourceOnly_TransactionRollback_QuarantineFaultModel_r1.zip'
```

The `-Scope Process` setting does not persist after that PowerShell process exits. The user could not find a Start-menu entry named "x86 Native Tools Command Prompt for VS 2026"; the PowerShell `Launch-VsDevShell.ps1` method **worked** and is the preferred recorded handoff.

## Exact key results from user console (not merely offline prediction)

```text
STAGE152_PACKAGE_MANIFEST_OK 14
[1/4] Pinned Stage151 SHA-256 and all ZIP members
{'upstream_stage': 151, 'verified_payload_files': 9, 'sha256': '7016b1f01d486c5ff27a3bb4017e7191473569d794735567e45ccb5f3cc2d4d1', 'upstream_model_cases': 173, 'upstream_source_anchors': 25, 'source_snapshot_mutated': False, 'safe_to_install': False, 'stage36': 'OFF'}
[2/4] Python unittest: Ran 27 tests in 0.077s, OK (skipped=2)
[3/4] MSVC Win32 x86 compilation: transaction_harness.cpp / win32_private_fixture.cpp / Creating code...
[4/4] Private Win32 allocation / API fault shim / cooperative worker / ABI / SEH
STAGE152_NATIVE_MODEL cases=149 active=2 clean=56 quarantined=91 dual=48 all_faults_reached=YES
STAGE152_WIN32_X86_SELF_OWNED stage67_memory=PASS stage70_memory=PASS cooperative_peer=PASS api_faults=48/48 stage67_injection=PASS stage70_injection=PASS abi_probe=PASS seh_probe=PASS external_process_access=NO
STAGE152_LOCAL_WINDOWS_TESTS_PASS (game/DLL install still forbidden)
```

Notes:
- Python: **25 passed + 2 skipped** (`test_26_compiles_gcc` and `test_27_compiles_clang`, because `g++` / `clang++` were absent); no Python test failed. The prior Linux/offline compiler checks were independent.
- MSVC Win32 x86 fixture **built and executed successfully**; Stage67+70 private sandbox memory checks, cooperative worker, 48/48 API fault cases, ABI and SEH probes PASS.
- 149 C++ model cases: 2 active, 56 clean, 91 quarantined, 48 dual faults, every intended fault reached.
- No external process access. None of these outcomes proves production detours, live game ownership or safe game DLL unloading.

## Outstanding blockers / Stage153 candidate

1. Current actual loaded hook site/owner map versus the source-only declarations; respect dormant Stage83/Stage84 and Stage36/Stage82 overlaps and Stage80→81 / Stage95→96 ordering.
2. Safe transactional rollback and page/thread state proof for the **real** Stage67/70 paths. Their source-level post-write failures remain **unfixed**; private harness does not fix them.
3. Native game ingress / complete ABI, non-local SEH/unwind, worker stop/join, FLS callback lifetime and safe DLL unload; test DLL/host gates from prior stages do not cover raw live game paths.
4. Strict `origin(0x27)=UNKNOWN` until uniquely attributed; no production patch just to attempt correlation.

**Keep `SAFE_TO_INSTALL=false`, `Stage36 OFF`, no gameplay writes.** Keep tested Stage129 2,099,200-byte working DLL and Stage122 2,089,984-byte rollback unchanged.

**Next Stage153:** evaluate Windows Stage152 evidence and design **source-only** preflight of real native ingress, FLS/worker shutdown and unload admission with no automatic installation.
