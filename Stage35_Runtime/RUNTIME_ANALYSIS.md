# Stage35 — downstream controller consumer runtime verified

**Review date:** 2026-11-10. Source: 2026-11-10 natural COM-vs-COM run 00:23:45–00:24:25 local. User pushed fresh Build.log and 22 changed/new `.log` files to `main` commits [6000b14](https://github.com/Hawky11/1/commit/6000b1411ca1becdc05c5105f40528f696756277) and [1a6f431](https://github.com/Hawky11/1/commit/1a6f431f1ba426ecc5a45509ddcae2136972ada8). **All 23 inputs are preserved as unchanged original Git blobs** in this branch's `Stage35_Runtime/`. The two pre-existing root logs `ArcanaAI_PlayerBit17WriteTrace.log` and `ArcanaAI_Stage26ObjectBit0WriteTrace.log` had **unchanged blob SHA** and were intentionally **not** mislabeled as fresh evidence.

Original **source-only Stage35** branch `stage35-controller-consumer-trace` @ `3ce68f5a5c49fbda11ffd6ad514ea0c229a790e3` is unchanged. This evidence branch derives from that exact source. Stage34 runtime baseline remains immutable at `stage34-type0-variant-runtime-evidence` @ `ad801c2278f81cdb997d5773207efecb4211af65`.

## Build and hook integrity

- User's Windows **x86 `Build.bat` SUCCEEDED** at 2026-11-10 **00:22:58**, VS18 Community, MSVC **14.51.36231**, `vcvarsall x86` exit 0. Linked `dinput8.dll`, **257024 bytes**; no compiler or linker errors. Single previously known **nonfatal C4754** in `src/AICommandSiteTrace.cpp:785` referring to line 782, identical warning category to earlier stages.
- Stage35 log has exact `STAGE35_CONTROLLER_CONSUMER_BUILD=2026-11-10-r1`, `STAGE35_CONFIG`, and **`STAGE35_INSTALLED callsite=ACABE->F60C0 phase=TIMER callsite=ACAC4->F5BD0 phase=FLAGS`**. The prior original `EE200` dispatcher, Stage31 `F6A8C→130A10`, Stage29 bit17 hardware observer and Stage32–34 traces remained active. This is observed installation, not proof of zero instrumentation overhead.
- Stage35 has exactly **9280 `STAGE35_CONSUMER` rows, seq 1..9280**, no gaps, duplicates, malformed rows or truncated terminal line; **4640 TIMER + 4640 FLAGS**. Per side **P1 4640 and P2 4640** (both phases included); all observed records run on game TID **8204**. **4640/4640** adjacent matching same-side/player/TID TIMER→FLAGS pairs, positive QPC separation **2..249 ticks**.
- Every one of **9280/9280** Stage35 `preMask=postMask=0xFFF`: all 12 raw fields were readable at both snapshots. No explicit ERROR, WARNING, dropped or overflow in Stage35, EventDispatch, Stage31, Stage29 and Queue core logs. As always, uninstrumented code paths/threads and between-snapshot writes cannot be excluded by these logs alone.

## EE200 baseline preserved in this run

`EVENT_DISPATCH_DETAIL` seq **1..125** without gaps: **85 type0** (`payload0=69`, `payload1=13`, `payload4=3`, `payload5=0`) and **40 type1**. Stage33 payload0 effects **69/69**, Stage34 payload1/4 effects **16/16**; **all 85/85** match exact unique dispatcher `seq/QPC/side`, and all 85 have primary `preMask=postMask=0xFFF`. The 16 Stage34 effect records have additional `slot0BMask=0x3`. No natural payload5 was observed, so its runtime behavior remains unconfirmed.

Stage31 emitted **32 events**: four `BEGIN`, four `END`, and **24 type0 queue** `kind=3` rows (`payload0=18`, `payload1=5`, `payload4=1`). Every one of **24/24** queue returns maps to exactly one subsequent same-side/player/TID/payload `EVENT_DISPATCH_DETAIL` with QPC delta **5..299 ticks**; see the [full per-event CSV](stage31_to_stage35_run_dispatch_pairs.csv). Four scoped episodes were P2 payload1, P1 payload0, P2 payload0 and P1 payload0.

Stage29 captured **8 bit17 transitions (4 complete set/clear cycles)** from both players: all four sets post-RVA `0xFAD2`, all four clears post-RVA `0xED1A9`. The four clear QPCs occur **inside actual matched EE200 calls**: dispatch seq **14** (P2 payload1), **54** (P1 payload0), **95** (P2 payload0), **121** (P1 payload0). This supports the Stage29–34 state path surviving Stage35 instrumentation; it is not an observed move.

## Main Stage35 result: real slot11 timer consumption

Stage34's native payload4 branch (`EE360→991C0`) writes `P+3A6C` slot11. This run includes **3 payload4 EE200 dispatches**, all on **P1**, with **2 positive slot writes** and one already-at-target:

| EE200 seq | Player | Slot11 before→after |
|---:|---|---|
| 90 | P1 | `0x00000000 → 0x00000F00` |
| 92 | P1 | `0x00000F00 → 0x00000F00` |
| 93 | P1 | `0x00000E88 → 0x00000F00` |

Native `ACABE→F60C0` phase TIMER **actually consumes** these values:

- First relevant TIMER Stage35 **seq 7957**, after dispatch seq92, reads `f3=P+3A6C: 0xF00→0xE88`; observed `f8=P+3A3C=0x78` (120).
- A subsequent payload4 EE200 seq93 writes `0xE88→0xF00`; the next TIMER **seq7961** observes a second `0xF00→0xE88` (also f8=`0x78`).
- Thereafter the consecutive native TIMER passes observe `0xE88→0xE10→0xD98→0xD20→...`, ending **`0x00000078→0x00000000`**. There are **33** Stage35 TIMER records with changed nonzero slot11; **33/33 decrement by exactly 120** and every associated header `f8` is **120**. All this observed slot activity is on P1. [All 33 transitions with QPC, masks and adjacent FLAGS](stage35_slot11_timer_transitions.csv).

**Important distinction:** the writer's `0xF00=3840` is 120×32 raw units, and the consumer indeed subtracts 120 per **observed call** under this game state. That does **not** by itself prove an action duration, 32 displayed animation frames, damage, movement, or why the slot was set. The second payload4 reset interrupts the first countdown.

## FLAGS consumer and controller-to-action limit

For **4640** matched FLAGS calls the observed `F5BD0` routine was executed after TIMER. Stage35 `f4=P+3B78` **does change** across some FLAGS calls while the upstream 12 raw fields remain readable; Stage35 demonstrates downstream flag recomputation.

Among the **33** TIMER/FLAGS pairs with positive slot11 on TIMER entry, subsequent `FLAGS` `P+3B78 bit7` is 1 in **32** and 0 in **one** (the final `0x78→0` case). However, in the **4607** pairs that began with a zero slot11, bit7 is also 1 in **3989**. Therefore, **bit7 is not an exclusive marker of payload4 or a nonzero timer** and these boundary snapshots alone **cannot prove** that the exact conditional `F5F4C/F5F55` branch was taken on a particular call; other flags/state conditions matter.

No meaningful per-call changes were observed in Stage35's direct controller map fields `f0/f1/f2` during either TIMER or FLAGS, consistent with these being updated by other routines (not proof of unchanged all frame). `f10/f11` object fields also show no net changes inside these two wrappers in this run. **None of this establishes a concrete rendered attack, hit, guard, motion or AI bug**.

## Integrity, limitations and Stage36 handoff

- All 23 fresh artifacts were reused as **identical Git blob SHA**; original uploads were not reserialized, truncated, or replaced. The Stage35 field indices and 12-bit masks are defined in [Stage35 source/install guide](../Stage35_Overlay/README_STAGE35_RU.md).
- Confirmed: full downstream path at instrumented call boundaries, exact P1/P2 order, two naturally triggered payload4 slot resets, and slot11 countdown with dynamic header `P+3A3C=0x78` in each observed nonzero tick; parallel queue/dispatcher/bit17 diagnostics remain consistent.
- **Not confirmed:** causal attribution of an exact individual `F5F55` OR to a given row, input semantics of `F58E0` during actual gameplay, a move/action boundary, rendered animation, damage, attack/hit success, or any AI malfunction. Correct attribution requires *specific downstream branch/output observation* and, if possible, a matched in-game visible outcome.
- Suggested next **Stage36**: first statically map and verify the **actual command/animation/action-issuance function**, using the original SHA-pinned `AALib.exe` already in ChatGPT Library `/AALib.zip`. Consider a **bounded passive** read of `F5F4C` branch result and `F58E0` controller-bitmap predicate (known caller `EB7D9`) only if it demonstrably narrows the path to such an action boundary. Preserve all tested Stage35 and earlier branches, original Stage29 `AITrace.cpp`; never force payload5, type1/0x1E, bit17 or controller fields.

**Status: Stage35 x86 build + passive runtime technically successful for the captured game path; gameplay effect still unverified.**
