# Stage113 Win32 BuildFix — C2668 SuspendAll overload ambiguity

User Windows MSVC 14.51.36231 x86 build on 2026-11-12 20:26:14 failed at `src/Stage113ParentGateTrace.inl:256` with C2668, **before link**, in Proxy.cpp.

Root cause: Stage113 `SuspendAll(arcana_stage110::Suspended&)` is called unqualified from the `arcana_stage113` namespace. Argument-dependent lookup adds `arcana_stage110::SuspendAll(arcana_stage110::Suspended&)`; both are equally viable. No gameplay/hook/runtime issue is established from this compiler error.

**Single-line source fix:**
```diff
- bool ok=SuspendAll(suspended),rollback=true;DWORD touched=0u;
+ bool ok=arcana_stage113::SuspendAll(suspended),rollback=true;DWORD touched=0u;
```

Original Stage113 module SHA256 `b982dbcf19b797937196bcf2cae2e8dcb6e403d2b4228066c5c7e362eba3943a`; one-line corrected module SHA256 `a5c8edecfceadd60716cf3a51f70d06619eb6d2ec8f9269ff9c5a0d0096a755b`.

Packaged local chat artifact: `ArcanaAI_Stage113_C2668_BuildFix.zip`, containing an exact-SHA-guarded `FixStage113C2668.ps1`, fixed module and README. Installation:
```powershell
powershell.exe -ExecutionPolicy Bypass -File ".\FixStage113C2668.ps1" -Root "D:\INCUBATOR\ArcanaAIOverhaul"
```
The script creates a one-time `.stage113-c2668.bak` copy, verifies both source and replacement hashes, changes only one source file, and is idempotent if already fixed. **Do not run ApplyStage113.ps1 again** after the fix; rerun only `Build.bat`.

This checkpoint **does not claim a successful Windows build, runtime installation, stage113 hook hits, or game results**; request fresh Build.log and in-game Stage113 and Stage110 logs after rebuild. Preserve Stage110 rollback DLL 2,073,088 bytes. Stage36 OFF.