# Stage89 r1 — native `F58E0` reader trace (SOURCE ONLY)

**Status: static validation PASS; Windows MSVC x86 build and game runtime NOT TESTED.** This is a passive diagnostic overlay on top of the previously **runtime-verified Stage88** DLL (1,975,296 bytes). There is **no AI/gameplay improvement claim**.

## Native model and exact locations

The original [SHA-pinned PE32](../Stage89_Analysis/STATIC_ANALYSIS.md) contains `F58E0(bitmapPtr,0x26)`: for bitmapPtr=`P+3A08`, it tests `P+3A0C & 0x200`, `&0x400`, `&0x800`, then (if all clear) generic bit 38 at `P+3A10 & 0x40`. AL is 1 when any applies, else 0. Four exact CALL sites:

| Site ID | CALL RVA | Original continuation | Conditional result |
|---|---|---|---|
| 0 | `9A870` | `9A875` | `9A87A JE` if AL=0 |
| 1 | `9AA20` | `9AA25` | `9AA2A JE` if AL=0 |
| 2 | `9AB00` | `9AB05` | `9AB0B JE` if AL=0 |
| 3 | `AAB55` | `AAB5A` | `AAB5F JNE` if AL!=0; AL=0 can reach `AAB65→E8560` |

## Files and safety

- [`src/Stage89F58E0Reader.inl`](src/Stage89F58E0Reader.inl) — four exact, individual CALL-site wrappers, never detours global `F58E0`, never adds another `EE306` hook. Each wrapper executes native `F58E0` **once** with two copied cdecl arguments and preserves original return registers, flags, SSE/x87. The wrappers do no file I/O. Stage81 worker drains the ring buffer.
- [`src/Stage81SelectorWriter.inl`](src/Stage81SelectorWriter.inl) — extends the existing Stage88 worker with Stage89 install/flush/health, retaining the previous overlays and order.
- [`ApplyStage89.ps1`](ApplyStage89.ps1) and [`RollbackStage89.ps1`](RollbackStage89.ps1) — SHA-gated installation and rollback, no other source edits. Baseline Stage88 `Stage81SelectorWriter.inl` SHA-256: `ede3d2dcc7218b82bc0d8d7f4967e1e2cd5e93eee985f9137d9c10d37fe25f3f`.
- [`tools/verify_stage89.py`](tools/verify_stage89.py): 22 original PE signature/target checks, **PASS_STATIC_SOURCE_ONLY** for original `AALib.exe` SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
- [`tools/correlate_stage89.py`](tools/correlate_stage89.py): offline Stage89 AL/bitmap/branch audit, optional same P/TID/QPC Stage88 **temporal context**, not causal action identity.

Worker-only `ArcanaAI_Stage89F58E0ReaderTrace.log` records `STAGE89_F58E0` with real AL/EAX/FLAGS, bitmap triples before/after, QPC, player, siteRVA, masks and computed branch. `predictionValid=1` means pre/post predicted AL agree; `2` means different and does not assert mismatch. `STAGE89_HEALTH` checks `dropped/mismatch/readFail/overflow`.

**No input injection, no game-data writes, no DR writes**, no guessed 0x800 hit. CALL-site hooking still changes timing and requires runtime verification.

## Install / test

1. Exit game; **back up Stage88 `dinput8.dll`** (1,975,296 bytes).
2. On the existing Stage88 source tree, run `powershell -ExecutionPolicy Bypass -File .\ApplyStage89.ps1`. This replaces only Stage81 inl and adds Stage89 inl, with SHA check and backup. Do NOT replace Stage88/Stage87 sources or AIQueueTrace.cpp.
3. In `[AIResearch]` keep:
   ```ini
   EnableStage24ResolverConsumerTrace=1
   EnableStage87=1
   EnableStage88=1
   EnableStage36ControlToCommandTrace=0
   EnableStage89F58E0ReaderTrace=1
   Stage89ReaderSiteMask=15
   ```
   Last two options default to 1 and 15 (binary site mask); for AAB55 only, set mask=8.
4. Run original MSVC x86 `Build.bat`, verify `BUILD SUCCEEDED` (size **unknown**, do not invent). Run 3–5 natural COM-vs-COM rounds and allow at least 2s worker flush.
5. Upload same-run `Build.log`, Stage89 reader, Stage88 join, Stage87 consumer, Stage24, EventDispatch and FinalCommand logs. Never mix QPC from different launches.

Expected `STAGE89_BUILD`, `STAGE89_INSTALLED mask=F sites=4/4`, `STAGE89_F58E0` records if relevant calls occur, `STAGE89_HEALTH mismatch=0 readFail=0 dropped=0 overflow=0`. If installed but no events, **natural path not sampled**, not proof of malfunction. If build, install, or health fails, stop and run `RollbackStage89.ps1` or restore Stage88 DLL.

**Not yet proven:** that any Stage89 reader executes after a particular Stage88 write, that a returned AL controls a particular COM decision, or that any movement/attack/hit occurred. Do not claim this before fresh runtime correlation.
