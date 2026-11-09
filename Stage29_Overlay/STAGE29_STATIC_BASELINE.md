# Stage29 — same-run bit17 producer-vs-dispatch correlation

**Date:** 2026-11-09. **Source:** exact x86 original executable from the user's ChatGPT Library `/AALib.zip`, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, plus independently inspected same-run Stage15/21–24/queue logs captured **2026-11-09 17:28**. **Status:** Stage29 source overlay prepared, not yet Windows-built or in-game tested.

## Why this stage is needed

Prior Stages15/16 already identified `player+0x3B84` bit17 as the producer branch decision for `EDI=0x1E`. The Stage16 historical full-write capture logged 24,176 writes during a *different earlier 16:51 game session* and just four bit17 toggles. Stage26 ran another bit0 watcher at 17:28 and had bit17 mode disabled, so that run's four Stage15 producer attempts cannot be paired with *same-session bit17 setter/clearer write records*. This is the concrete evidence gap targeted by Stage29.

The source changes only the existing Stage16/Stage26 DR0/DR1 observation pipeline: watch bit17 and **record only genuine bit17 transitions**, while advancing the shadow on every observed write. Preserve same-run Stage17 callsite and Stage15 producer/Stage21 consumer traces. No gameplay or queue writes are added.

## Same-run Stage15 matrix — four actual attempts

All four records below are from the **same 17:28 session**. `STAGE15_BRANCH` 14 records grouped by side/tick; the `F6A68` branch snapshots and original `F6AD7` ALT_CALLSITE are:

| Side | Tick | F6A68 QPC | bit17 | Branch JE to F6A9D | ALT F6AD7 observed | Runtime outcome |
| --- | ---: | ---: | ---: | --- | --- | --- |
| P2 | 31450296 | 314503266298 | 0 | Yes | QPC 314503267012 | queue → dispatch |
| P1 | 31451546 | 314515681973 | 1 | No | not observed | bit17 alternate path → return |
| P1 | 31457515 | 314575463829 | 1 | No | not observed | bit17 alternate path → return |
| P2 | 31460765 | 314608004402 | 0 | Yes | QPC 314608005275 | queue → dispatch |

For both P1 attempts, the captured `[player+0x3B84]=0x00020000`; P1 `[player+0x3A34]=1`, `[player+0x3A36]=0x0A` or `0x22`, and local `[EBP-4]=1`. For P2, bit17=0 and local=0. These other fields are *correlated* with the branch in this tiny sample; their causal effects are not established.

All four attempts pass the earlier `F6A5D` `0xF2ED0` predicate (AL=0) and `F6A63` payload comparison. Two P2 attempts additionally reached `F6A9D` where `[EBP-4] != 2` branches to the common queue continuation. **F6A9D is not a queue-suppression gate.**

## Exact original binary decision tree

At original AALib RVA `0xF6A68`, confirmed by direct x86 disassembly from the pinned exe:

```asm
F6A68  mov eax,[esi+0x3B84]
F6A6E  shr eax,0x11
F6A71  test al,1
F6A73  je 0xF6A9D        ; bit17=0
; bit17=1: alternate dispatch, not ordinary queue insertion
F6A75  push 0
F6A79  call 0xE8560
F6A7E  lea ecx,[ebp-0x18]
F6A81  push esi
F6A82  push ecx
F6A83  call 0x156C00     ; snapshot / copy selected fighter references
F6A88  lea edx,[ebp-0x18]
F6A8B  push edx
F6A8C  call 0x130A10     ; large conditional alternative processing routine
F6A91  add esp,0x0C
F6A96  mov al,1
F6A9C  ret
; bit17=0: optional helpers, then the queue path
F6A9D  cmp [ebp-4],2
F6AA1  jne 0xF6AC8
...
F6AC8  push 0
F6ACC  call 0xE8560
F6AD7  call 0xF2DE0     ; actual event queue writer
```

`0x156C00` writes a **16-byte local snapshot** from the input fighter: `[snapshot+0]=player`, `[snapshot+4]=[player+4]`, `[snapshot+8]=[player]`, `[snapshot+0xC]=[player+0x3ED4]`. It does not enqueue anything. `0x130A10` reads bit17 and other flags and performs conditional gameplay work, including calls to routines elsewhere in the game; detailed meaning of this large function is not yet classified. **Do not describe the P1 alternate return as a dropped event.**

The generic bit113 writer at RVA `0xFAA0` writes `player+0x3B84` bit17 when passed index `0x71`; the unique immediate caller candidate at `0xC6502` was traced in Stage17. The original game bit17 clearer `0xED19F` clears bit17/18 conditionally when its helper returns AL0. Stage16 earlier 16:51 run saw two bit17 `0→1` writes (one P1 and one P2, post-EIP `0xFAD2`) and two `1→0` writes (post-EIP `0xED1A9`). **This different session is not QPC-correlatable to the 17:28 producer events.**

## Dispatcher result is deterministic for 0x1E, not a probe failure

Both P2 queue events reached type1 dispatcher via `0xEE200`; Stage23 resolver yielded record selector31 (0x1F) and Stage24 `0xF57D0` wrote output `(0x1F,0x00010001,0)` and observed the secondary OR on P1+0xF8.

Stage21 classifier at `0xEE426` invokes `0xE8CF0` with `EAX=0x1E`, selector1. Exact code:

```asm
E8D35 test al,al
E8D37 jne E8CFC  ; -> AL=0
E8D39 cmp edi,0x20
E8D3C jl E8D8D
E8D8D cmp edi,0x17
E8D90 setbe al
```

Payload `0x1E < 0x20`, but `0x1E > 0x17`, so `AL=0` in either branch of the earlier state-dependent check. **This is expected behavior** for the exact original code. Both Stage21 runtime logs indeed report `resultAL=0`.

This selects fallback at dispatcher `0xEE443`: `0xF7EB0(array=player+0x3A3C,index3)` clears the slot at `player+0x3A4C` followed by `AND [controller+8],0xFFFFFBFF`. In both P2 runtime events, slot3 was already `0→0`, and controller flag bit0x400 was already `0→0` (Stage22). Thus the fallback had **no observed change to either of those two fields**, not evidence of a failed hook. Do not assume slot3/bit0x400 reflect all possible downstream state.

## Stage29 observation design

A minimally changed Stage26 `AITrace.cpp` and INI:
- Enable `EnablePlayerBit17WriteTrace=1`, `EnableStage29Bit17TransitionsOnly=1`, `EnableStage26ObjectBit0WriteTrace=0` and `EnableControlFieldTrace=0`. Retain existing Stage14/15 producer, Stage17 setter, Stage21–24 dispatcher, Stage25 secondary OR and queue event traces.
- New `ArcanaAI_Stage29Bit17Transitions.log`; marker `STAGE29_BIT17_TRANSITIONS_BUILD=2026-11-09-r1`, config `STAGE29_CONFIG`, startup `STAGE29 active`, records `STAGE29_BIT17_TRANSITION`, counters `STAGE29_FILTER_STATUS`, and nonzero dropped warnings.
- DR0/DR1 hardware write watchpoints remain write-only; pass all existing thread/DR checks. Store a new record only for bit17 change; read the same field for every trap and **update shadow before filtering**. Keep QPC, per-side field, post-write EIP/RVA, code bytes, registers, and 8 DWORD stack snapshot for each retained transition.
- Fix unrelated source warning C4456: inner `reported` shadows outer variable; renamed to `filterReported`.
- This still cannot see writes on threads that never received the watchpoint, on the excluded COMvsCOM writer thread, or an initial transition before watchpoint install. Shadow-value old side is best effort; event order is not atomic across threads.
- User must compile with their working x86 `Build.bat`, verify the Stage29 marker and DR0/DR1 installs, then run a natural COM-vs-COM test and upload Build.log, Stage29 log, Stage15 producer log, Stage17 setter, queue/alternate/disptacher and Stage21 trace. No force or modification of game decisions.

## Decision criteria

- A Stage29 observed `P1 0→1` before a same-session `F6A68` P1 bit17=1 establishes contemporaneous setter direction; correlate its post-write EIP and Stage17 CALL at RVA0xC6502 to narrow why it was set.
- If no Stage29 P1 transition is captured but baseline has bit17=1 and P1 is suppressed, it was already set at installation or on an uninstrumented thread. **This does not prove the watcher failed**.
- If bit17=0 at F6A68 but no F6AD7 call appears, investigate missing Stage15/alternate logger coverage, caller return or an unseen precondition before claiming an alternate mechanism.
- Stage29 is **source-only and not yet MSVC/in-game validated**. Earlier source branches and archives remain immutable.
