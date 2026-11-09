# Stage29 runtime evidence — 2026-11-09 20:35 COM-vs-COM

## Provenance and limitations
- User's fresh GitHub upload to `main` commit `c5d49fec910f96e621e7502fb2a2cc5d5f1fb57d`, 20 changed/new runtime logs, including 8,902,658-byte ProducerCallTrace.
- This evidence branch is based on **immutable Stage29 source** `ad78d4668bd4ca14fc471289037bd6ef9ed62f0b`, with exact log blobs copied from that main upload. No old Stage branch was changed.
- `Build.log` was NOT present in the new main upload; cannot independently verify MSVC transcript or claim `BUILD SUCCEEDED`. Actual in-game Stage29 marker, active DR0/DR1 watchpoints and events confirm live Stage29 instrumentation executed.
- Original binary pinned SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, available in ChatGPT Library `/AALib.zip`; see `BINARY_SOURCE_IN_LIBRARY.md`.

## Runtime setup
- `STAGE29_BIT17_TRANSITIONS_BUILD=2026-11-09-r1`; INI: bit17 watch on, Stage29 transitions-only on, Stage26 bit0 watch off, control watcher off.
- P1 player `0x0E251F78`, field `0x0E255AFC`; P2 player `0x0E27CCA0`, field `0x0E280824`; baseline both `0x00000000`. DR7 `0x00DD0005`.
- Watchpoint install started with 55 new game threads, then +1, +1 and +2 batches. COMvsCOM writer TID 17960 explicitly excluded; all recorded transitions and relevant F6A68 probes have TID 17912.
- Eight contiguous `STAGE29_BIT17_TRANSITION seq=1..8`: four sets 0→1 at post-RVA `0xFAD2`; four clears 1→0 at post-RVA `0xED1A9`, two cycles each player. Last periodic filter status: `suppressedNoBit17Transition=21456 recorded=8`. This is a last-observed counter, not a final shutdown summary.
- Stage17 observed eight `BIT113_SET_CALL` records at callsite `0xC6502`, helper `0xFAA0`, index `0x71`, two calls within each of the four cycles. All four first calls precede the transition 0→1; second calls occur with bit17 already set.
- No explicit dropped/overflow/error lines found in Stage29 and producer logs examined. Logs lack an explicit final clean-stop/flush marker: do not assert perfect capture of all game activity.

## QPC correlation: producer 0x1E gate, setter and clearer
All QPC values are from *the same 20:35 game run*. For stage29 hex QPC we converted to decimal. The table covers all **five** `STAGE15_BRANCH probe=F6A68_PLAYER_BIT17_BRANCH` events with `EDI=0x1E` (not all Stage14 probes).

| Side | tick | QPC @ F6A68 | bit17 at gate | set QPC 0→1 | clear QPC 1→0 | bit17 branch taken | F6AD7 / queue / dispatch |
|---|---:|---:|---:|---:|---:|---:|---|
| P1 | 42674656 | 426746875823 | 0 | none before gate | none | 1 | YES, once |
| P2 | 42674953 | 426749783359 | 1 | 426749780481 | 426749785223 | 0 | NO |
| P1 | 42682546 | 426825784546 | 1 | 426825782070 | 426825785807 | 0 | NO |
| P2 | 42689218 | 426892469824 | 1 | 426892467506 | 426892471294 | 0 | NO |
| P1 | 42691156 | 426911894005 | 1 | 426911891787 | 426911895279 | 0 | NO |

For **every one of the four bit17=1 attempts**, the same-player sequence is
`Stage17 C6502 setter → Stage29 post-FAD2 set → Stage15 F6A68 gate with bit17=1 / JE not taken → Stage29 post-ED1A9 clear`.
The gate falls **strictly inside the set→clear window**, QPC distances from set to gate 2878/2476/2318/2218; gate to clear 1864/1261/1470/1274, respectively. This independently confirms the bit17 path selection; no need to assume stale snapshots or combine different runs.

For the initial P1 attempt at tick 42674656, Stage15 shows bit17=0, JE taken and subsequent `F6A9D` probe, then `ALT_CALLSITE F6AD7` QPC 426746876512, `QUEUE_EVENT_DETAIL` seq12 QPC 426746876587 (type1/payload0x1E accepted), `EVENT_DISPATCH_DETAIL` seq12 QPC 426746876599→426746883745. Stage21 classifier `E8CF0 AL=0` at 426746881809 and slot3 fallback `0→0` at 426746882807. Stage22 controller bit400 `0→0` at 426746883239; Stage23 resolver produced record selector31 and Stage24 output mask `0x00010001`, Stage25 ORed target P2 object bit0 `0→1`. The dispatcher/consumer path is distinct from producer's bit17 gating.

## Interpretation
- **New stronger result:** outcome is determined by **the player's bit17 at the observed F6A68 gate**, not by being P1 or P2. Both sides now have observed bit17=1 suppressed queue attempts; P1 has observed bit17=0 queued attempts (prior Stage26 captured P2 bit17=0 queued attempts). The native bit17=1 code path performs separate processing (Stage29 static baseline: 0xE8560/0x156C00/0x130A10), so “not queued” is not equivalent to “event lost”.
- Stage29 supplies temporal causation for the **gate decision**: a real bit113 setter and hardware-observed set occur shortly before each `0x1E` attempt and the native clearer shortly after. It **does not prove** that setter timing or the non-queue branch is a gameplay bug, nor that the branch omits a required action.
- AL0 from classifier `E8CF0` and the later no-op fallback apply only to the queued `bit17=0` case; do not conflate them with suppression at `F6A68`.
- QPC should be compared within a single run only. Stage16 old 16:51 and Stage26 old 17:28 are independent sessions.
- Hardware watchpoints don't cover excluded/uninstrumented threads. No logged transition does not prove no write; the accepted initial P1 event has no associated set preceding its gate.

## Next research action (Stage30)
Focus on the native alternative path after `F6A68` at bit17=1: inspect and instrument **outcome/side effects** of `0x156C00` and `0x130A10`, its return `AL=1`, and whether P1/P2 actions advance similarly to queued events. Prioritize read-only static analysis against pinned AALib.exe before adding invasive hooks. Any new runtime instrumentation should be a separate Stage30 branch, preserve all register/EFLAGS/stack semantics, and compare both branch outcomes in the same run. Do not forcibly clear bit17 or inject `0x1E` into the queue; that would change the game and invalidate causal observations.
