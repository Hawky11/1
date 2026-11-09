# Stage33 — type0/payload0 native consumer state snapshots

**Status:** original AALib.exe static analysis completed and verified; Stage33 passive source prepared. **MSVC x86 build and in-game Stage33 test are still pending.**

Source branch is a continuation of the tested Stage32+Stage31+Stage29 diagnostics. No new hook or intentional game-state write.

## Install into Windows working directory

Replace **only** these two files in `D:\INCUBATOR\ArcanaAIOverhaul`:

1. [Stage33_Overlay/src/AIQueueTrace.cpp](src/AIQueueTrace.cpp) → `src\AIQueueTrace.cpp`
2. [Stage33_Overlay/ArcanaAI.ini](ArcanaAI.ini) → `ArcanaAI.ini`

**KEEP `src\AITrace.cpp` from Stage29.** Do not replace any header, project file or other source. Historical Stage branches are not to be modified.

Run the usual **x86 `Build.bat`** and inspect complete `Build.log`. Install the new DLL+INI; archive any old `ArcanaAI*.log` before the fresh run. Run a natural COM-vs-COM session with both sides and multiple bit17 cycles. Let the worker flush for at least 2 seconds before exit.

Upload `Build.log` and fresh logs, especially `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage31AlternatePathTrace.log`, `ArcanaAI_Stage29Bit17Transitions.log`, `ArcanaAI_QueueTrace.log`, and `ArcanaAI_ProducerCallTrace.log`.

## Expected output

In `ArcanaAI_EventDispatchTrace.log`:

```text
STAGE32_TYPE0_DISPATCH_BUILD=2026-11-09-r1
STAGE33_TYPE0_CONSUMER_BUILD=2026-11-09-r1
STAGE33_CONFIG read-only pre/post EE200 snapshots for type0/payload0 ...
CPU event dispatch hook installed at RVA=0x000EE200 ...
EVENT_DISPATCH_DETAIL seq=... qpcBefore=... qpcAfter=... eventType=0x00000000 payload=0x00000000 ...
STAGE33_EFFECT_DETAIL seq=... qpcBefore=... qpcAfter=... side=P... tid=... player=... preMask=0xFFF postMask=0xFFF fields=0:...>...,1:...>...,
```

`STAGE33_EFFECT_DETAIL` uses the SAME `seq` and QPC pair as the corresponding `EVENT_DISPATCH_DETAIL`. Only type0/payload0 gets the new record; existing type1 details, Stage31 episodes and Stage29 bit17 watchpoints are preserved.

Field index map: 0=P+3B70, 1=P+3B84, 2=P+3F64, 3=P+3A08, 4=P+3A0C, 5=P+3A10, 6=object pointer [P+4], 7=O+08, 8=O+184, 9=P+3F54, 10=P+3F74, 11=P+3F78. `preMask`/`postMask` indicate which reads actually succeeded. False/zero masks invalidate assumptions based on values.

## Analyze the result

- Verify marker and hook installation; check missing seq, dropped/overflow, and Build.log errors.
- For each Stage31 nested episode, find same-side/TID subsequent Stage32 dispatcher; join its `seq` to Stage33 effect record.
- Check `P+3B84` bit17 clear, `P+3F64`, controller P+3A08..10 and object flags; *net change only*, not every transient write.
- Runtime net changes or entry into dispatcher do **NOT** establish a successful move/attack. No gameplay modifications or artificial type1/0x1E events.

[Stage33 static study](../Stage33_Analysis/STATIC_ANALYSIS.md). [Pinned verifier](../Stage33_Analysis/tools/verify_stage33.py):

```powershell
py -3 .\Stage33_Analysis\tools\verify_stage33.py --binary "G:\Arcana Heart 3 - LS XTEND\AALib.exe"
```

Original EXE already exists in ChatGPT Library `/AALib.zip` and does not need reupload.
