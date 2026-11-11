# Stage70 r2 — original deferred-ID write RUNTIME VERIFIED

**Evidence date:** 2026-11-11. Exact user raw upload: [98ff59b871e90fb757c787b472d5d5322941ce07](https://github.com/Hawky11/1/commit/98ff59b871e90fb757c787b472d5d5322941ce07) on `main`. This evidence branch starts **at that exact uploaded commit**, preserving the full raw log files unmodified. **Source lineage:** [stage70-r2-relocation-aware-guard](https://github.com/Hawky11/1/tree/stage70-r2-relocation-aware-guard), which changed original previous-instruction RVA `15677D` guard to account for a PE HIGHLOW relocation at `156780`. Original private `AALib.exe` SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.

## 1. Build and installation verified

- `Build.log`: started **11.11.2026 02:53:34**, MSVC **14.51.36231 x86**, VS Community 18; **BUILD SUCCEEDED**, `dinput8.dll` **1,892,864 bytes**. Existing C4754 at AICommandSiteTrace.cpp:785, no compile failure. Original DLL itself not uploaded and not reproduced by the analyst; build provenance is from the user's log.
- All trace initialization headers **2026-11-11 02:54:29**, game thread **20812** for target events, P1=`0E4A1F78` with O=`2A3A3020`, P2=`0E4CCCA0` with O=`2A3A3C50`. No cross-session log mixing.
- Stage70:
```text
STAGE70_DEFERRED_BUILD=2026-11-11-r2 relocation-aware original signature; DRwrites=0.
STAGE70_GUARD x86=1 xmm=1 stage67=2 stage65=1 stage59Started=0 stage70=0 nativeOK=1 gameBase=00800000 tableRead=1 runtimeTableVA=00C254E8 expectedTableVA=00C254E8
STAGE70_INSTALLED 1/1 site=156784 store=156787 resume=15678A; source MOV executes once; DRwrites=0
```
The corrected relocated runtime table operand **matches** the expected `gameBase+4254E8`: r1 signature bug is resolved in this run. Stage65 1/1, Stage67 2/2, Stage59 WATCH_ARMED after initial worker wait.

## 2. Direct original-site positive writer observation

The newly emitted Stage70 log contains **exactly one** `STAGE70_DEFERRED_WRITE` row, at **02:55:06**:

```text
STAGE70_DEFERRED_WRITE seq=1 side=P1 player=0E4A1F78 object=2A3A3020 tid=20812
qpcBefore=1518396105821 qpcAfter=1518396105822
siteRVA=156784 originalStoreRVA=156787
valueEDX=00000025 eaxBefore=00000001 eaxAfter=2A3A3020 edxAfter=00000025
preMask=07 postMask=07
deferred=FFFFFFFF>00000025 selector=00000001>00000001
objectPtr=2A3A3020>2A3A3020
```

Stage70 HEALTH at 02:55:06: `calls=1 filtered=0 dropped=0 overflow=0 mismatch=0 readFail=0 ring=4096 depth=8`. All six PRE/POST individual reads succeeded, and the original source EDX value equals the resulting native object DWORD. Original native `156784: mov eax,[ebp+08]`, `156787: mov [eax+68],edx` are the displaced/replayed opcode-guarded instructions.

**Precise claim:** the instrumented execution of the *original* `156787` instruction is now **directly runtime verified by PRE/POST state and source register on the same native execution path**. While instrumented, its physical CPU EIP is inside the DLL replay code rather than the original EXE RVA, so `originalStoreRVA=156787` is an origin label **not an observed original EIP sample**. Source instrumentation and Windows behavior are supported by the installed marker and memory states; a separate independently reconstructed complete binary and differential emulator test are unavailable. There is no claim that gameplay AI was modified or that behavior is guaranteed unchanged in every possible future run.

## 3. Complete six-stage same-run causal sequence

Every event below is **P1/P=0E4A1F78, O=2A3A3020, TID=20812**. All QPC values are raw ticks, not milliseconds.

| Order | Trace row | Before→after QPC | Relevant observed state |
|---|---|---|---|
| 1 | Stage65 seq319 | 1518396105810→5813 | `callerCallRVA=00156769` in original callback 1566B0, `request=2F`, `P+3F4C: FFFFFFFF→2F`, full read mask 3F |
| 2 | Stage70 seq1 | 1518396105821→5822 | **`O+68: FFFFFFFF→25`**, EDX=25, `P+3F50:1→1`, P+4=O, full masks 07/07 |
| 3 | Stage40/41 seq320 | 1518396106226→6236 | Consume request `2F`; Stage41 `O+64:22→2F`, deferred `O+68:25→25`, full masks 1F/1F |
| 4 | Stage67 kind0 seq4539, Stage42 seq232 | 1518396106268→6303, inner 1518396106271→6300 | Native activation **`O+60:22→2F`**, `O+68` remains 25, `O+30=0` |
| 5 | Stage67 kind0 seq4561 | 1518397203768→3801 | **`O+30:1→0`, `O+64:2F→25`, `O+68:25→FFFFFFFF`, `O+60:2F→25`** |
| 6 | Stage42 seq234 nested in seq4561 | 1518397203770→3797 | `O+60:2F→25` and inner PRE pending25, deferredFFFFFFFF |

The Stage65 request helper finished **8 QPC ticks** before Stage70 PRE, and Stage41 began **404 QPC ticks** after Stage70 POST. Separate gate arming is bracketed: Stage67 seq4559 POST QPC 1518397103037 had `O+30=0`, deferred25; seq4561 PRE QPC 1518397203768 had `O+30=1`, deferred25. The original `1E8FEB` arming site was not specifically observed as an EIP hit.

This closes the particular Stage67/68/69 question of **where deferred `25` came from** for the new run and connects the original action request `2F` to deferred write and later promotion. One recorded positive write suffices to establish that this mechanism executes at least once, not its global uniqueness.

## 4. Full-session integrity

| Observer | Rows | Integrity |
|---|---:|---|
| Stage40 request consumption | 371 | 1..371, no duplicates/gaps |
| Stage41 object commit | 371 | 1..371, 1:1 same actor, seq and QPC as Stage40 |
| Stage42 activation | 276 | 1..276, all 276 uniquely nested in one same-actor/object Stage67 kind0 |
| Stage43 activation origin | 276 | 1..276, aligns with Stage42 |
| Stage65 action request helper | 369 | 1..369, all health error counters zero |
| Stage67 native hooks | 5,708 | 5,706 kind0 + 2 kind1; no gaps; all health error counters zero |
| Stage59 DR0/DR1 X write watcher | 16,947 | 1..16,947; WATCH_ARMED, all health error counters zero |
| Stage70 deferred positive writer | 1 | seq1; full masks, all health error counters zero |

Stage65's **369 unique native request-helper writes** plus Stage67's **2 E9E60 zero-request writes** explain **371/371 Stage40 request consumers** as the matching latest observed request-changing event for the same actor (no missed events within the observed source inventory). This is still a **bounded observer** claim, not a proof that there are no other possible request writers in the entire native binary. All **276/276 Stage42 activations** uniquely nested within Stage67 same-actor/object windows. All monitored errors (`dropped`, `overflow`, `mismatch`, `readFail`, Stage59 thread/install/ownership/drift counters) are zero at recorded HEALTH samples. QPC sequence analysis was performed on the exact raw uploaded blobs with the connected GitHub content and a read-only join; no new DLL instrumentation or user data was modified.

## 5. What is resolved, what remains

**RUNTIME VERIFIED:** Native callback at `1566B0` first calls immediate requester from original `156769`, then (in the same observed original code path) computes and records deferred action with native instruction `156787` into `O+68`; `O+30` later arms; `1E9F40` consumes deferred `25` by promoting into pending `O+64`, and `1E9C70` activates `25`. At event time selector=1, source EDX=25 and request2F. The earlier P2 2E/24 and P2 33/26 examples remain supportive historical evidence, not positive Stage70 per-IP traces in this particular capture.

**Still not established:** what *causes* COM to pick selector `P+3F50=1`; exact earlier writer/decision chain for selector; exact native EIP of `O+30` arming store in this run; general behavior, timing/throughput neutrality and AI quality outcomes. Stage71 should be **STATIC FIRST**: trace native writers/consumers of `P+3F50` and callback scheduling/selector provenance using the original SHA-pinned EXE and already archived logs; design a new read-only probe only after selecting one narrow and safe site. **No Stage71 DLL or additional game test is needed yet.**

## Provenance / no modification

User's new logs and Build.log remain in `stage70-r2-runtime-evidence` starting commit `98ff59b...`. This report is analysis-only. Stage70 r2 source maintained separately on `stage70-r2-relocation-aware-guard`; main branch and the original Stage70 r1 source not edited by the analyst. Preserve user's tested Stage70 r2 DLL **1,892,864 bytes** (known working in this capture), and older Stage67 **1,887,232**, Stage65 **1,879,552**, Stage59 r2 **1,153,024** rollback binaries.
