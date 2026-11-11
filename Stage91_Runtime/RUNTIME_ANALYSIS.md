# Stage91 runtime — direct F58E0 readers SEE the active bits, 1,825 native AL=1 events

**Status:** Windows MSVC x86 **BUILD SUCCEEDED**; Stage91 `INSTALLED mask=7 sites=3/3`, source-only r1 is now **tested at runtime**. The three exact native CALL sites at `F5A4D`, `F5A5C`, `F5A76` produced **34,790** recorded `F58E0` reads, including **1,825 AL=1** with the matching special bit present in `P+3A0C`. **No demonstrated final COM command or attack** yet.

**Provenance:** fresh user upload [`4e56101`](https://github.com/Hawky11/1/commit/4e56101e9095d6f3b04fd4dcedf2ca5d5314ed57), Stage91 source [`0516e76`](https://github.com/Hawky11/1/commit/0516e767be5657118d6950466c5c3dfdfb632a5c) and [source-only branch](https://github.com/Hawky11/1/tree/stage91-f58e0-direct-bit-readers-source-only). Raw logs in root: `Build.log`, `ArcanaAI_Stage91F58E0ReaderTrace.log`, Stage90/88/87/89, FinalCommand/EventDispatch.

**Important file retrieval pitfall:** `ArcanaAI_Stage91F58E0ReaderTrace.log` is **15,480,170 bytes** (Git blob SHA `84f4f78cb0eeeb68425caea347bb300e4c308e2f`). The GitHub connector `fetch_file` returned **empty content** for this large file; this is **not an empty runtime log**. Fetching the **exact Git blob by SHA** recovered the whole 15.48-MB text in memory; no truncation was used for any counts below.

## 1. Windows build / native installation

- `Build.log`: **BUILD SUCCEEDED**, `build/dinput8.dll` **1,999,872 bytes**. Pre-existing warning `C4754` at `AICommandSiteTrace.cpp:785` remains nonfatal.
- `STAGE91_BUILD=2026-11-11-r1 SOURCE_ONLY ...` and `STAGE91_INSTALLED mask=7 sites=3/3 rvas=0F5A4D,0F5A5C,0F5A76 nativeTarget=0F58E0 originalOnce=1 noNewEE306=1 DRwrites=0 gameWrites=0`. `SOURCE_ONLY` is a compile-time source identifier, not a claim that runtime did not happen.
- All three sites executed, and all 34,790 events have accurate site/index, player and complete PRE/POST read masks. **34,790/34,790** `predictionMatch=1`; **34,790/34,790** native `pre=post` bitmap; sequential `seq=1..34790` with no gaps or duplicates.

## 2. Direct native predicate results

| CALL site | bitIndex | Bit in P+3A0C | Events | AL=0 | AL=1 | P1 AL=1 | P2 AL=1 |
|---|---:|---|---:|---:|---:|---:|---:|
| `F5A4D` | 9 | `0x200` | **12,571** | 11,388 | **1,183** | 265 | 918 |
| `F5A5C` | 10 | `0x400` | **11,388** | 10,831 | **557** | 245 | 312 |
| `F5A76` | 11 | `0x800` | **10,831** | 10,746 | **85** | 60 | 25 |
| **Total** | | | **34,790** | **32,965** | **1,825** | **570** | **1,255** |

Positive results always had the relevant bit set in the actual PRE bitmap. Positive `F58E0` returned `EAX=1` / `AL=1` with `EFLAGS=0x213`; negative returned `AL=0` with `EFLAGS=0x246`. These are the **actual callee values** saved by the wrappers, not an inferred result from a calculation.

Stage91 HEALTH sums across **58** periodic intervals: `calls0=12253,calls1=11209,calls2=10652,events=34114,filtered=0,overflow=0,dropped=0,mismatch=0,readFail=0`. **The interval counters are reset periodically.** There are **676 additional valid event rows after the last HEALTH row**, so 34,114 is a **partial health sum**, **not a mismatch with the 34,790 real event rows**.

## 3. Stage90→Stage91 strict same-P/TID/QPC state consistency

For all **34,790** Stage91 reads, find the most recent **completed** Stage90 `F57D0` call with the **same player and thread**, `stage90.qpcAfter < stage91.qpcBefore`. There is **always** such a row, and its 3-DWORD POST bitmap exactly equals the reader's PRE bitmap in **34,790/34,790** observations. Zero discrepancies. This establishes strong chronological, same-actor bitmap consistency across native writers and direct readers (within the observed F57D0 coverage); it is not proof that arbitrary other writers don't exist.

Stage90 same-run: **2,069** `STAGE90_F57D0` rows (periodic 58-health sums cover 2,060; nine rows after last HEALTH), **99 net special clears and 100 sets** through those interval boundaries; **853** triple-DWORD changes, **1,207** no change, all HEALTH errors zero. Stage87 and Stage88 each have **261** matched selector events (selector0 **129**, selector1 **85**, selector2 **47**); Stage88 `joined/completed=261/261`, `mismatch/stale/readFail/dropped=0`.

There are **49 AL=1** Stage91 readings whose latest preceding Stage90 `F57D0` call was **itself one of the precisely paired Stage87→Stage88 calls** (no intervening Stage90 call). Of these 49, **12** have Stage88 target-bit `bitBefore=0,bitAfter=1` and thus establish an actual **0→1 bit set followed by a positive downstream native read**: 11 for bit `0x200`, 1 for `0x400`. Stage91 `0x800` has actual AL=1 positives and a Stage88→Stage91 direct state join, but not a new `0→1` transition in the subset of immediate joins.

The remaining AL=1 events are often after intervening `F57D0` calls which **preserve an already-set bit**; do not attribute all 1,825 positive reads to a single Stage88 selector event.

## 4. Three actual source→positive-reader→clear examples

These examples use the same player and TID; QPC is raw ticks, **not seconds**. They demonstrate native state chronology and branch-predicate result, **not** proof of the final game action.

**Bit `0x200` new-set → positive read**:
- Stage88 `seq22` P2 selector0: `(0x29,0,0x10)→(0x0E,0x200,0)`, target bit **0→1**. Same-call Stage90 `seq129` has `stage87Seq=22`, POST `(0x0E,0x200,0)`.
- Stage91 `seq2469` P2 at `F5A4D` / index9: bitmap `(0x0E,0x200,0)`, **AL=1**, QPC **96,902 ticks** after Stage88 POST. No intervening F57D0 call.

**Bit `0x400` new-set → positive read**:
- Stage88 `seq239` P1 selector1: `(0x29,0,0x10)→(0x0F,0x400,0)`, target **0→1**; matching Stage90 `seq1936`.
- Stage91 `seq32679` P1 `F5A5C` / index10: same active bitmap and **AL=1** only **146 QPC ticks** after Stage90 POST, no intervening F57D0.

**Bit `0x800` already-set → positive read → clearing**:
- Stage88 `seq98` P1 selector2: POST `(0x10,0x800,0)`, bit already set; matching Stage90 `seq678`.
- Stage91 `seq12415` P1 `F5A76` / index11: `(0x10,0x800,0)`, **AL=1**.
- Stage90 `seq680` subsequently changes that bitmap to `(0x06,0x1002,0)`, **clearMask=0x800**, **621 raw QPC ticks after reader completion**. No claim that this is a newly-set Stage88 bit; positive read and subsequent clear are independently observed.

A total of **1,685 AL=1 readings** have a later recorded same-player/TID `F57D0` net-clear of their bit somewhere in the remainder of this session; the other 140 have no later same-bit clear recorded. These are **read events**, many repeated within an active interval, not 1,685 unique command activations.

## 5. Still not proven

- Stage91 records `nextBranchPredictedTaken` based on disassembled conditional logic, **not an independently traced branch target**. The previous static analysis puts AL=1 for indices9/10 on the path toward `F5A68` / `P+3B70 |= 8`, and index11 on the `9ED20` branch rather than `9ED60`. These branch executions and field writes need independent tracing for a full claim.
- Stage89's old special-index `0x26` reader at AAB55 still shows **four AL=0**; this is not contradictory: it samples a different stage/bitmap state and its OR-of-3 special logic.
- Nothing here proves that a particular COM attack was queued, animated, or hit an opponent, or that AI is improved. Code policy remains read-only; no synthetic input/game writes.

## Stage92 next

Start static/native guarded observation of the **post-F5990 branch effects**: actual `P+3B70` bit0x8 transitions and the **9ED20 vs 9ED60** selected helper path, ideally with a bounded correlation to Stage91 call/AL and existing queue/command trace by same player/TID/QPC. Do **not** add another EE306 hook or global F58E0 detour or enable Stage36. Preserve **Stage91 tested DLL 1,999,872 bytes** for rollback.

[Machine-readable results](results.json).
