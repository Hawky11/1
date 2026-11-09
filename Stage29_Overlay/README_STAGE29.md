# Stage29 — same-run bit17 setter vs producer and dispatcher

**Prepared source only; not yet built or run.** The aim is a short fresh COM-vs-COM run with both bit17 write transitions and producer/queue/consumer evidence from the *same game session*.

## Main discovery

The original AALib.exe and the Stage15/21–24 17:28 logs show two P1 `0x1E` attempts with `[player+3B84].bit17=1`; both follow an alternative non-queue path (`0x156C00` snapshot → `0x130A10`), while two P2 attempts with bit17=0 reach the `0xF6AD7` queue writer, dispatcher and fallback. The classifier returning AL=0 for payload0x1E is **statically required**, not a diagnostic failure. See `STAGE29_STATIC_BASELINE.md`.

The old Stage16 run at 16:51 had **24,176 records but just four bit17 changes** (two 0→1 and two 1→0). Stage29 reduces that diagnostic noise; bit17 transitions-only mode updates shadow on each observed write, retaining only changed bit17 snapshots and stack/QPC.

## Apply to working Stage26 project

From [stage29-bit17-producer-correlation](https://github.com/Hawky11/1/tree/stage29-bit17-producer-correlation):
1. Copy `Stage29_Overlay/src/AITrace.cpp` to `D:\INCUBATOR\ArcanaAIOverhaul\src\AITrace.cpp`.
2. Copy `Stage29_Overlay/ArcanaAI.ini` to `D:\INCUBATOR\ArcanaAIOverhaul\ArcanaAI.ini`.
3. **Do NOT replace `src/AIQueueTrace.cpp`**: keep the already-tested Stage25/Stage26 working variant with all Stage21–25 wrappers. Keep other C++ files and project files unchanged.
4. Run existing `Build.bat` from project root and confirm `BUILD SUCCEEDED`. Static source checks here do not replace an actual MSVC build.
5. In a fresh natural COM-vs-COM game, use existing P1/P2 CPU mode. Upload new Build.log and all same-session logs.

Required new marker `STAGE29_BIT17_TRANSITIONS_BUILD=2026-11-09-r1` in **`ArcanaAI_Stage29Bit17Transitions.log`**. Startup must say `STAGE29 active` with both watched addresses P1/P2+0x3B84 and DR7=0x00DD0005. The new events are `STAGE29_BIT17_TRANSITION`; no-change writes are suppressed. Filter counts appear every 250 suppressed writes.

Main same-run files: `ArcanaAI_Stage29Bit17Transitions.log`, `ArcanaAI_ProducerCallTrace.log` (Stage14/15), `ArcanaAI_AlternateCallTrace.log`, `ArcanaAI_QueueTrace.log`, `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage21PayloadConsumerTrace.log`, and `ArcanaAI_Bit113SetterTrace.log`. Also retain Stage22/23/24 and Build.log. **Do not substitute the old 16:51 `ArcanaAI_PlayerBit17WriteTrace.log`**.

`Stage26TransitionsOnly=1` remains in the INI for reversibility but Stage26 is **disabled**. `EnableControlFieldTrace=0`; DR0/DR1 cannot watch both bit17 and bit0 simultaneously. New mode does not change game state, producer decisions, helper return values, or queue contents.

### Offline checks

```powershell
py -3 .\Stage29_Overlay\tests\verify_stage29.py --source .\Stage29_Overlay\src\AITrace.cpp --ini .\Stage29_Overlay\ArcanaAI.ini
```

**Limitations:** write-only hardware watchpoints exclude the COMvsCOM worker and cannot cover threads that are not instrumented. Shadow values are best-effort; no transition in the log can mean P1 bit17 already had the value at initialization. The filtered log records game state at post-write boundaries, not arbitrary reads or unrelated bit changes.

Original EXE is already available in the user's ChatGPT File Library **`/AALib.zip`** — see the root `BINARY_SOURCE_IN_LIBRARY.md` on `main`; no user reupload required.
