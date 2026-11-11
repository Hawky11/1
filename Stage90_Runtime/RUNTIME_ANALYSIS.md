# Stage90 runtime audit — F57D0 actively clears selector bitmap (2026-11-11)

**PASS (for the observed paths).** User-uploaded same-process logs [`3a6f9e9`](https://github.com/Hawky11/1/commit/3a6f9e908ab7e3bc1e18aca4085c5a6b3fdd044b) confirm that the **existing Stage24 EE306→F57D0 observer** can capture net changes to the controller bitmap and that three Stage89 `AL=0` reads have prior, concrete Stage90 net clears of the corresponding special bits. **Not yet proof that any attack or gameplay action was issued.**

Source-only package [Stage90 branch](https://github.com/Hawky11/1/tree/stage90-f57d0-bitmap-clear-source-only/Stage90_Overlay); original static findings [Stage90 analysis](../Stage90_Analysis/STATIC_ANALYSIS.md). The source's `SOURCE_ONLY` literal is its build identification string, **not an assertion that this Windows runtime did not occur**.

## Build and complete HEALTH totals

`Build.log`: Windows MSVC x86 **BUILD SUCCEEDED**, `dinput8.dll` **1,991,168 bytes**; the previous nonfatal `C4754` warning at `AICommandSiteTrace.cpp(785)` remains.

`ArcanaAI_Stage90AllF57D0Trace.log`: `STAGE90_READY` and **1,792** `STAGE90_F57D0` rows, P1 **910**, P2 **882**. Summed 55 `STAGE90_HEALTH` intervals:

| Metric | Count |
|---|---:|
| preCalls / tracked / completed | 1,792 / 1,792 / 1,792 |
| filtered | 0 |
| bitmap 3-DWORD net changes | 718 |
| bitmap 3-DWORD no change | 1,074 |
| special-bit net clears (nonzero clearMask) | **87** |
| special-bit net sets (nonzero setMask) | **88** |
| readMaskIncomplete / depthOverflow / mismatch / readFail / dropped | **0 / 0 / 0 / 0 / 0** |

All row read masks are complete `preMask=7`, `postMask=7` and `tableMask=1`. All descriptor flag-bit0 snapshots indicate the native clear-capable path, but **a descriptor flag does not equal an actual net clear**. Four F57D0 calls had both a net special-bit clear and a different net set in the same call.

| P+3A0C bit | Native net clears 1→0 | Native net sets 0→1 |
|---|---:|---:|
| `0x200` | **38** | **39** |
| `0x400` | **41** | **41** |
| `0x800` | **8** | **8** |
| **Total** | **87** | **88** |

Native selector codes resulting in a net clear: `0x06` (47 calls), `0x29` (13), `0x1C` (8), `0x00` (8), `0x0E` (3), `0x15` (4), `0x07` (3), `0x0D` (1). Example **Stage90 seq479** P1: selector `0x29`, dispatch type1/payload `0x49`, `P+3A08/0C/10 (0x10,0x800,0) → (0x29,0,0x10)`; `clearMask=0x800`.

## Old observer preservation and strict joins

Stage87: **280** `STAGE87_CONSUMER`; Stage88: **280** `STAGE88_F57D0`. Both attempted **1,792** call observations and filtered **1,512**, with zero `stale/mismatch/readFail/dropped`. **280/280 unique** Stage88 `stage87Seq` rows have precisely **one** Stage90 row matching the player, side, TID, selector argument, full bitmap `pre/post`, and enclosing QPC `[stage90Before,stage90After]`; 0 errors. Stage87 `seq` has the exact same player/TID/QPC as Stage88 `stage87Seq/qpc87`.

| Stage87 P+3F50 | Stage88 F57D0 arg | Target bit | Matched rows | Target bit 0→1 |
|---|---|---|---:|---:|
| 0 | `0x0E` | `0x200` | 140 | 10 |
| 1 | `0x0F` | `0x400` | 119 | 3 |
| 2 | `0x10` | `0x800` | 21 | 2 |
| **Total** | | | **280** | **15** |

Stage89 `F58E0`: exactly **3** records at `0xAAB55`, all P1/TID **6820**, all `AL=0` and bitmap masks complete, no HEALTH errors. The other three installed Stage89 CALL sites did not execute. All `AL=0` reads have both the special bits and the ordinary bit38 absent.

## Three same-player/TID/QPC Stage88→Stage90→Stage89 chains

For **each** of the three Stage89 reads, select the **latest preceding Stage88** event of the same player/TID, then identify a **subsequent Stage90** native call whose net `clearMask` actually removes that Stage88 target bit. The Stage90 call's own PRE snapshot contains the bit, POST snapshot lacks it. No subsequently **recorded F57D0 call** resets a special bit before the Stage89 read. The last Stage90 POST before each reader is identical to its Stage89 PRE bitmap. This is observed sequential native output state, **not a proven unique transaction identity across different dispatcher events**.

| Stage89 read | Latest prior Stage88 | Stage90 net clear | Clear call | Clear→read QPC ticks | AL |
|---|---|---|---|---:|---:|
| `seq1` P1 | `seq79` selector1, bit `0x400` | `seq540` removes `0x400` | arg `0x06`, type0/payload `0x12` | **17,224,012** | 0 |
| `seq2` P1 | `seq158` selector2, bit `0x800` | `seq1014` removes `0x800` | arg `0x06`, type0/payload `0x12` | **101,261** | 0 |
| `seq3` P1 | `seq205` selector0, bit `0x200` | `seq1445` removes `0x200` | arg `0x00`, type0/payload `0x00` | **11,008,773** | 0 |

**Detailed bit 0x800 chain** (Stage89 read `seq2`):

1. Stage88 `seq158` is a native `F57D0 arg=0x10` with POST `(0x10,0x800,0)`; its exactly matched Stage90 row is among the 280 strict joined events.
2. Stage90 `seq1011` changes bitmap `(0x10,0x800,0)→(0x24,0x8800,0)`, **preserving 0x800**.
3. Stage90 `seq1014` uses `argSelector=0x06`, `dispatchType=0,payload=0x12` and changes `(0x24,0x8800,0)→(0x06,0x1002,0)`, giving **`clearMask=0x800`**.
4. After this, there are **two** recorded F57D0 calls for the same player/TID and neither sets or clears special bits. Then Stage89 `seq2`, **101,261 QPC ticks** after the clear, reads exactly `(0x06,0x1002,0)` and returns `AL=0`.

**Other two chains:** before Stage89 `seq1`, Stage90 `seq536` preserved 0x400 while changing `(0x0F,0x400,0)→(0x24,0x8400,0)`, and Stage90 `seq540` then cleared it to `(0x06,0x1002,0)`. Eight subsequent F57D0 calls had no special-bit mutation, and the last POST was `(0,1,0)`, matching reader seq1. Before Stage89 `seq3`, Stage90 `seq1442` preserved 0x200 while changing `(0x14,0x40000200,0)→(0x24,0x8200,0)`, then Stage90 `seq1445` cleared it to `(0,1,0)`. Four subsequent F57D0 calls had no special-bit mutation, and reader seq3 saw the same `(0,1,0)`.

**Caution:** The per-instruction `F57FA` zero store is statically located; this trace measures **native call net PRE→POST**, not the exact machine instruction executing at each clear. Other code paths can write the bitmap between snapshots. QPC gaps are in raw ticks; do not convert to time without measuring the frequency. These chains establish the observed net-clear route and later zero AL, not an observed active-bit downstream read or actual movement/hit/AI-command issue.

## Stage91 next: active-state reader localization, not another clearing hook

**Stage90 answers the missing-bit question** for these three observed readers. The next question is who reads the bitmap **while** the special bit is still set, before Stage90's clearing F57D0 calls. Start with SHA-pinned **static** analysis of exact bitmap readers and native dispatch/queue predicates between the Stage88 set and Stage90 clear. Choose a small, noncolliding, read-only native observation boundary only if it can disambiguate causality. Keep the known-good Stage90 DLL **1,991,168 bytes**, Stage36 off, and avoid a second EE306 hook, global F58E0 entry detour, or gameplay writes.

[Machine-readable report](results.json).
