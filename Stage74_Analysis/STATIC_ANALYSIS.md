# Stage74 — existing callback-window selector snapshots; conflict-free source-only design

**Date:** 2026-11-11. **SOURCE READY / STATIC SOURCE-DIFF VERIFIED; WINDOWS x86 BUILD AND GAME RUNTIME PENDING.** New branch starts at the completed Stage73 branch. The Stage74 replacement is copied from the exact verified Stage70 r2 source blob `ed6065fd5877cb5e1ab634493c07b32f054510bf`; only **four** explicitly audited edits were made, with output blob `106dca56d662fe1c8e0b7083013a535b67cde3c8`. A source-only verifier reconstructs Stage70 r2 from Stage74 and checks both Git blobs. Original `AALib.exe` SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` was independently rechecked with its 20 selector MOV sites and two direct `155B70` CALLs. **No Stage74 DLL has been built or executed.**

## What was learned from the actual Stage70 r2 sources

- `Stage70_Overlay/src/Stage38CallbackWindow.inl` already passively wraps original native callback dispatch (two existing Stage38 hook boundaries); its PRE is **after** native `156C00`, and POST is **before** native `FC60`.
- Existing Stage38 has `kStage38Fields=30`, with indices 0..14 read from **player P**, and 15..29 from **object O**. The buffer and snapshot masks use fixed DWORDs (32 bits).
- Original Stage38 f3/f4 are `P+3ED0/P+3ED4`; f20 is `O+68`. It does **not** currently capture `P+3F50` or `P+3F54`.
- Stage59's verified `Stage59XWriterWatch.inl` installs DR0/DR1 **write-only DWORD** watchpoints on two dynamic X addresses and conditionally claims matching `DR6` bits. It explicitly leaves independent DR2/DR3 exceptions for other owners. The active Stage70 `ArcanaAI.ini` disables `EnableControlFieldTrace`, `EnablePlayerBit17WriteTrace` and `EnableCommandResolveTrace`; Stage59 is enabled. These settings do not prove foreign DR2/DR3 availability, so **do not take DR2/DR3** for the new observation.
- Stage70 uses an **already installed software hook**, no DR. The replacement leaves Stage70, Stage65, Stage67, Stage59 includes and install sequence, existing native hooks, ABI and context save/restore unchanged.

## Exactly four Stage74 source edits

File: [Stage74_Overlay/src/Stage38CallbackWindow.inl](../Stage74_Overlay/src/Stage38CallbackWindow.inl), ready to replace the currently installed Stage70 r2 `src/Stage38CallbackWindow.inl`.

1. Change `kStage38Fields = 30u` to `32u`. Existing `Stage38Record::Pre/Post` arrays and loops scale automatically; `1u<<31` is valid on 32-bit unsigned.
2. Append **`0x3F50u,0x3F54u`** to the offset array as fields `f30` and `f31` (preserves f0..f29 exactly).
3. Restrict the existing missing-object guard to `15 <= i < 30`; new player fields must be sampled even if O was missing.
4. Select address base `P` when `i<15 || i>=30`, and `O` only for fields 15..29. This is **critical**; blindly appending two offsets would incorrectly read `O+3F50/O+3F54`.

The original `STAGE38_CALLBACK_WINDOW` line now appends `f30=XXXXXXXX>YYYYYYYY f31=XXXXXXXX>YYYYYYYY`. `preMask`, `postMask` and `changedMask` bits 30/31 distinguish read failures and changes. All older column indices and Stage39's `Pre[28]` remain unchanged. Existing row capacity, log lines and field-saving ABI were not changed; each row gets 2 extra DWORDs in PRE and POST and two output fields. No new native hook site or debug register has been added.

## What a new capture can and cannot establish

For the same actor P/O/TID and QPC run, compare successive Stage38 callback PRE and POST samples:

- **Within a window**: if f30 `PRE != POST` with both mask bits valid, the selector was changed **somewhere during that native callback execution**. The logged callback RVA, f31 state transition, and exact QPC window narrow the candidate native source sites.
- **Across windows**: if the prior POST selector differs from the next PRE, the transition occurred **between sampled boundaries**; do not assign it to either callback.
- If the value was already 1 before the first observed callback, origin remains outside coverage. If f30 is always 1 and unchanged in the captured neighborhood, do not label `155D29` or any of the register-derived sites as executed.
- The known Stage38 seq4090 `158570→1566B0`, seq4092..4114 repeated `1566B0`, and seq4116 `1566B0` with nested Stage65 request 2F + Stage70 `O+68=25` are **from the previous frozen run**. New Stage74 sequences may differ; do not match by raw seq numbers across sessions. Correlate P/O/TID, QPC and action IDs **within one new run**.
- This is a passive **boundary correlation only**. Even a positive f30 transition **does not prove the writer EIP**. A subsequent Stage75 may target one restricted function or exact opcode only after the first-change window is observed. Full all-11-site instrumentation is postponed.

## Installation and rollback

Use the user-verified Stage70 r2 files and **replace only** `D:\\INCUBATOR\\ArcanaAIOverhaul\\src\\Stage38CallbackWindow.inl` with the complete Stage74 replacement. Keep Stage70's `Stage70DeferredWriter.inl`, all other source files, `ArcanaAI.ini`, and the known-good 1,892,864-byte DLL backed up. Run:

```powershell
py -3 .\Stage74_Analysis\tools\verify_stage74.py --exe "D:\PATH\TO\AALib.exe"
.\Build.bat
```

`verify_stage74.py` checks source blob identity, the four edits and optionally original EXE SHA. It cannot compile or prove runtime safety. After **BUILD SUCCEEDED**, the new DLL size is not preassigned and no install marker should be invented. Run one normal COM-vs-COM capture, verify Stage38 installation 2/2, Stage70 1/1, Stage59 WATCH_ARMED and zero relevant health errors. Upload from **one run**: `Build.log`, `ArcanaAI_Stage38CallbackWindowTrace.log`, `ArcanaAI_Stage65RequestOriginTrace.log`, `ArcanaAI_Stage70DeferredWriterTrace.log`, and `ArcanaAI_Stage59XWriterWatchTrace.log`. There is no need to upload every historical log. If build or run fails, restore backed-up Stage70 r2 adapter and DLL; **do not enable other DR owners** to compensate.

**Stage74 cannot claim runtime verification, selector writer IP, improved COM AI, or a restored immediate/deferred decision mechanism.** The only new observation is the proposed pair of read-only callback-window fields.
