# Stage124 — Original FA480 caller roles versus the real AI command path (STATIC ONLY)

**2026-11-13.** Source is the user's original private `AALib.exe`, PE32/i386, preferred imagebase `0x00400000`, 175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.

**No new hooks, DLL, Windows build, game run, game-memory modification or accepted-command/AI-improvement claim.**

## Machine results and reproduced evidence

Original .text was scanned for E8 + signed rel32 targeting original RVA **0FA480**, cross-checked against linear GNU objdump instructions. **229/229** direct original E8 callsites match and have unique RVAs and verified opcode bytes; complete list and local disassembly are in the corresponding user ZIP. There are **20** 0x10000-size RVA ranges with such calls.

Verified usage forms immediately after CALL:
- **164** original `TEST AL,AL; JE`.
- **56** original `TEST AL,AL; JNE`.
- **3** delayed test after MOV immediate into another register.
- **1** immediate TEST without immediately following JE/JNE.
- **5** remaining non-immediate-test consumers (including other operations, stack restores and/or subsequent calls).

**220** immediate TEST-then-Jcc paths, **9** other usages. These describe local machine control flow, **not classification as AI acceptance/rejection**.

Independently confirmed **4/4** direct original E8 `CALL 0CEBE0` sites: `0988EA`, `098948`, `0CAD79`, `0CADDA`. Native checks cover **28** pinned opcode contexts, **9/9** JE/JNE branch destinations and relevant native direct calls `098886→0FA480`, `0F3B19→0FA480`, `0CE13C→0FA480`, `0F7AD1→0F3930`, `0F7DA3→0CEA20`. **45/45** stdlib positive/negative offline tests PASS, including native opcode, native edge, catalog and corruption negatives.

## FA480 gates at two different CEBE0 parent pathways

**A: Native function RVA 098780.** `098886 CALL 0FA480`, `09888B TEST AL,AL`, `09888D JE 098965`. When AL=0 the JE skips the **two** original CEBE0 call sites `0988EA` and `098948`. This is the direct original-code explanation of Stage123's real **16,490 / 16,490** Stage113 parent-A gate skips and Stage110 native CEBE0 entry=0 in this run. Stage115 separately captured **1,498** post-FA480 tuples `(G1,G2,G3,G4,G5)=(0,0,3,7,0)` with `observedAL=00,ZF=1`, each matching the sampled Stage113 gate.

**B: Native function RVA 0CACE0.** No direct FA480 call inside this function. At `0CACEA` original code checks `CMP DWORD PTR [EBX+0x10CC],-1`; `0CACF1 JE 0CADF9` skips processing if true. The two other direct CEBE0 sites `0CAD79`, `0CADDA` occur beyond that condition. Stage123 had `parentB=0` for the **particular observed parent-B callsite 0EFDD8**; that does not establish absence of any other callers or execution elsewhere.

Therefore a single global statement that FA480=0 blocks all CEBE0 processing is already too broad without identifying the active parent.

## Observed AI producer and alternate command handling

Stage94/104 known original chain: `0F7AD1 CALL 0F3930` (producer) and special-command original `0F7DA3 CALL 0CEA20` (Stage104 original call, Stage107 POST return). A direct FA480 call exists **inside producer F3930** at `0F3B19`:

```asm
0F3B19 call 0FA480
0F3B1E test al,al
0F3B20 je 0F3B32
0F3B22 cmp dword ptr [0E2D6718],3
0F3B29 jne 0F3B32
0F3B2B mov byte ptr [esi+0x252],1
0F3B32 ... producer logic continues ...
```

The two conditions therefore permit one original **byte-field store**. Failure to satisfy them falls into `0F3B32`, where other producer logic still executes; this is **not** itself proof of a command's native acceptance or rejection. F3930 has other branches and a dispatch table; that subpath is not guaranteed to execute on each invocation. The fresh Stage123 logs did not trace execution of F3B19 or this byte-field write.

In the **immediate native F7900–F7FFF** segment used by Stage104, **zero direct E8 FA480 callsites** occur. This is a *local* absence: the called producer F3930 itself does contain FA480. No inference of full route independence is made.

Same-run Stage123 **25/25** MODE0 special 0x27 → native CEA20 PRE → Stage107 POST events occurred, despite CEBE0 entry=0. That establishes a real executed command-processing **function call and return**, not an accepted command, hit, animation or improved AI.

## Other diverse predicates

- `0CE13C CALL FA480; 0CE141 TEST AL,AL; 0CE143 JE 0CE154`, followed by an extra flag check `[0E100EA8]` and alternate native control flow; some branches write that flag to zero.
- `0A6B33 CALL FA480; TEST AL,AL; JE 0A6C0B`, then another condition requires **G3==2** on the positive path. FA480=1 is not sufficient.
- `0A6C45 CALL FA480; TEST AL,AL; JE 0A6C85`, then another condition requires **G3==1**.
- `0A6CA6 CALL FA480; TEST AL,AL; JNE 0A6CBE` uses inverted jump polarity.

These are independently opcode-verified examples and **not yet named as gameplay command accept/reject functions**.

## User's newly supplied general runtime marker

The user separately supplied `[2026-11-12 23:48:01] COMvsCOM active: P1 and P2 control DWORDs are being forced to 1 (CPU).`. This follows the fresh Stage123 successful Build.log start at **23:47:30** and supports that test was COM-vs-COM. That exact line was **not present in the previous Stage123 GitHub upload**; an old root ArcanaAI.log must not be cited as the source of this new line.

## What is and isn't established

**Proven:** original callsite xrefs; multiple polarities and uses; one positively runtime-tested FA480/CEBE0 bypass; independent original CACE0 guard; a conditional byte-field write in producer F3930; and native CEA20 calls and POST returns in an observed game run.

**Not proven:** dynamic hits on any of 228 other FA480 direct callsites, their return values during Stage123, all-in-game action semantics, native command acceptance/commit, exclusive G4 last writer, animation/hit, or improved AI. The scan covers **direct E8 rel32 calls**, not indirect FF /2 calls, tail-jump paths or external runtime trampolines. Static original disassembly cannot rule out unknown runtime owners.

## Artifacts and next step

Chat file archive: `ArcanaAI_Stage124_FA480_Caller_AIFlow_StaticAudit.zip`, **47,993 bytes**, SHA256 **e938abc332278489b3d5f1efa99a0f611b39eed2671ebb13b33cf83a9a448b2d**, **10 files** (full Russian report, all-229 CSV, original machine disassembly context, native verifier, 45-test suite/results, per-region counts). No original EXE/DLL or game-modification scripts in package.

**Stage125** should remain **STATIC-FIRST**: follow the specific proven sequence `F7AD1→F3930→F7B0A producer gate→F7CC0 MODE0→F7DA3 CEA20→F7DA8 POST` into the actual original command acceptance/commit stage, including any causal relevance of F3B19's optional byte +0x252 store. Only consider a narrow passive, positively controlled runtime observer if concrete native acceptance evidence remains unavailable. Do not hook all 229 callers or force FA480 return values.

Preserve Windows-tested Stage122 DLL **2,089,984 B**, Stage115 **2,082,304 B**, Stage113 **2,079,744 B**, Stage110 **2,073,088 B**. Stage36 OFF.
