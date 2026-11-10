# Stage49 r1 — Windows x86 BUILD PASSED; native install BLOCKED by signature guard

Date: 2026-11-10 14:37–14:38 local. **This is an unsuccessful Stage49 instrumentation run, not a completed Stage49 runtime result.**

## Immutable input snapshot

This evidence branch starts at uploaded main commit [b7e0fbedf493a2939ecf2ef96e9098ff80836db5](https://github.com/Hawky11/1/commit/b7e0fbedf493a2939ecf2ef96e9098ff80836db5), which includes [all fresh user logs from 1dde176](https://github.com/Hawky11/1/commit/1dde176a9cf02285cea12de7f6cbd2252807690e) and Build.log. Original r1 source remains frozen in [stage49-linked-motion-state-propagation](https://github.com/Hawky11/1/tree/stage49-linked-motion-state-propagation).

Build.log: `Started: 10.11.2026 14:37:28,80`, `BUILD SUCCEEDED`, dinput8.dll **1,074,176 bytes**, only the pre-existing C4754 warning in AICommandSiteTrace.cpp.

**Direct Stage49 evidence** (`ArcanaAI_Stage49LinkedMotionTrace.log` is 162 bytes):

```text
[2026-11-10 14:37:59] STAGE49_LINKED_MOTION_BUILD=2026-11-10-r1
[2026-11-10 14:37:59] ERROR: Stage49 root/CALL/linked-field/ABI signature mismatch; no patches.
```

No `STAGE49_INSTALLED`, `STAGE49_LINKED_WINDOW` or `STAGE49_HEALTH`. The Stage49 fail-closed guard intentionally did **not** install either new redirect; there is no Stage49 motion-transfer runtime evidence. A successful link is not a substitute for native installation.

## Other trace integrity / safety

Direct Git blob content inspected (large files are not empty even when a path-oriented fetch returns no text):

| Log | Window records | Evidence |
|---|---:|---|
| Stage35 | 9,336 | balanced P1=4,668 P2=4,668, contiguous seq 1..9336 |
| Stage38 | 4,277 | P1=2,110 P2=2,167, contiguous seq 1..4277 |
| Stage44 | 1,318 | P1=679 P2=639, contiguous seq 1..1318; shared HEALTH no losses |
| Stage47 | 4,664 | P1=2,332 P2=2,332, contiguous seq 1..4664, all from native caller AE70C; 2,899 changedMask != 0 |
| Stage48 | 1,318 | P1=679 P2=639; caller 1E908F=1040, caller 1E9E2D=278, caller 1E8DA8=0 for P1/P2 |

Stage47 `STAGE47_INSTALLED 2/2`; 23 HEALTH samples total calls=40,268, otherObjects=35,604, P1/P2=4,664; **dropped=overflow=mismatch=badCaller=0** throughout. Stage44 `STAGE44_INSTALLED 1/1`; its 29 HEALTH samples total calls=42,736, other=41,418, P1/P2=1,318; **dropped=overflow=mismatch=0**. Stage48 `STAGE48_INSTALLED 2/2`. No new fail markers in the sampled Stage35/38/44/47/48 logs; large file readers used blob SHA and content, not the truncated fetch_file response.

## Likely failing guard: PE32 relocations, requires r2 runtime confirmation

The SHA-pinned EXE's preferred ImageBase is **0x00400000**. The *new r1 run* Stage13 line `ret=0x008F6ADC` at return RVA `0x000F6ADC` demonstrates loaded base **0x00800000**. The original Stage49 `valid()` compares disk-exact instruction bytes at AF11E (`8B 35 C8 02 2E 0E`) and AF19E (`8B 35 C0 02 2E 0E`), each containing a 32-bit **absolute pointer**. These operands are expected to be rebased by the Windows loader; with delta +0x00400000 expected addresses are **0x0E6E02C8** and **0x0E6E02C0**, rather than original file values 0x0E2E02C8 and 0x0E2E02C0. Conversely original rel32 CALL bytes do not require that relocation.

r1 emits only one combined error for all root/CALL/linked-field/ABI guards. Therefore root relocation is a **strong hypothesis**, not proven exact failing site. Do not infer anything about real AI quality, on-screen movement, animation or damage from this run.

## Next

[Stage49 r2 source](https://github.com/Hawky11/1/tree/stage49r2-relocation-aware-root-guard) adjusts only the two root checks to verify expected relocated absolute operands and adds per-signature diagnostics while keeping a fail-closed guard. [r2 Windows instructions](https://github.com/Hawky11/1/blob/stage49r2-relocation-aware-root-guard/Stage49_Overlay/README_STAGE49_R2_RU.md) replace just Stage49LinkedMotion.inl (keep all other working source files). **New Windows x86 build/runtime pending.**
