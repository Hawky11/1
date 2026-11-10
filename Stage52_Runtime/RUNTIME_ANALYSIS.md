# Stage52 Windows x86 runtime — verified dual native boundaries; live X differences precede AEF10

Date: 2026-11-10. Exact user-uploaded commits: [7b760cf](https://github.com/Hawky11/1/commit/7b760cf270a27d16715e6e553e978acc3044a36f) (general logs) and [944b3b8](https://github.com/Hawky11/1/commit/944b3b83a499995808ab34bcce69f8bec973498c) (Stage36–52 plus Build.log). This evidence branch began at upload commit `944b3b83a499995808ab34bcce69f8bec973498c`, preserving original uploaded log blobs unchanged.

## Build and runtime integrity

- Build.log starts local **2026-11-10 16:53:14.29**, Windows MSVC x86 **BUILD SUCCEEDED**, `dinput8.dll` **1,097,728 bytes**. Existing C4754 warning only.
- COMvsCOM worker active 16:54:16, P1 and P2 control DWORDs forced to CPU.
- Stage52 `STAGE52_PRE_GLOBAL_COORD_BUILD=2026-11-10-r1`, **STAGE52_INSTALLED 2/2 AF10D→AEF10 and AF112→AED90**, native original invoked once at each. Stage51/50/49/47 continued to install.
- Stage52 **12,748** records, **6,374** for AEF10 and **6,374** for AED90, 3,187 per side per site. Sequential seq=1..12748 no gaps, all masks `1FFF/1FFF`, no error lines. **29 HEALTH** records; every missingObjects/dropped/overflow/mismatch/badCaller=0.
- Of all Stage52 events only **two** show changes: the FIRST AEF10 invocation has **P1 and P2 O+18 (f7) 00000000→00000004**, `changedMask=0080`. All later AEF10 rows and **all 6,374 AED90 rows** have changedMask=0000. Specifically **zero live X/Y/anchor writes inside either native observed call** for tracked P1/P2.
- Stage51: **6,370** P1/P2 rows unchanged, 29 zero-loss health; Stage50 **6,368** unchanged/gateCandidate0; Stage49 r2 **6,366** unchanged/staticGateCandidate0; Stage47 **6,244** rows with 4,317 nonzero changedMask. All sequences contiguous within logs; zero error/loss markers.

## Full original blob SHA references

| Stage | SHA of exact Git blob | Events |
|---|---|---:|
| Stage52 | `7c6dd41788605718aa6f26f7a012c5f8793159c5` | 12,748 |
| Stage51 | `a0bf9b8f69f8c259d1584b60c959176d10194b1e` | 6,370 |
| Stage50 | `f407f57ca2e77f92ea72f3175e9977837ca81575` | 6,368 |
| Stage49 | `6a7798df0d77dd446f8fa587498d3cb667eac70d` | 6,366 |
| Stage47 | `5511cf5abbd8cd088632b4088425d45f336064c9` | 6,244 |

Large file path fetch may return empty; retrieve exact Git blob by SHA, NOT a truncated search excerpt.

## Same-frame event correlation

Join events by tracked side AND object AND timestamp after previous call and before next call (monotonic QPC), with gap <=5,000 QPC ticks. Stage47 has 6,244 complete records, all match **one distinct** Stage52 AEF10 event (gap 367–1,162 ticks), its immediately subsequent AED90 same object (gap 3–72), then Stage51 same object (gap 4–71). [6,244-row exact CSV](stage47_stage52_stage51_sameframe_join.csv) includes IDs, QPC, side/object, PRE/POST values and flags. Additional Stage52 rows after Stage47 tail are not falsely interpreted as same-frame Stage47 matches.

| Boundary | P1/P2 pairs | X differences | Y differences | Anchors / other |
|---|---:|---:|---:|---|
| Stage47 X/Y POST → Stage52 AEF10 PRE | **6,244** | **868** | **0** | Stage47 X POST == Stage52 AEF10 PRE O+AC **6,244/6,244**. AnchorY also matches **6,244/6,244** |
| Stage52 AEF10 POST → Stage52 AED90 PRE | **6,374** | **0** | **0** | Anchor X/Y and O+18 all match 6,374/6,374 |
| Stage52 AED90 POST → Stage51 PRE | **6,370** | **0** | **0** | Anchors and O+18 match 6,370/6,370 |
| Stage51 POST → Stage50 PRE | **6,368** | **0** | **0** | Four X/Y/AC/B0 fields match 6,368/6,368 |
| Stage50 POST → Stage49 PRE | **6,366** | **0** | **0** | Four fields match 6,366/6,366 |

**Interpretation:** The observed **868 X-only divergences occur before AF10D→AEF10 is entered**, after the Stage47 observation. Therefore, despite static instructions at AED90 AEE01/AEE04/AEE16/AEE1C that write coordinates of SOME native object, AED90 does **not** write the two tracked object positions in observed events. Stage52 AEF10 modifies both objects' O+18 initial flag 0→4 only. Do not overclaim a globally inert routine; other objects/routes may use AED90.

## Next native boundary

Original SHA-pinned AALib.exe caller corridor disassembled:

```asm
A2B81 CALL A1B80
A2B86 CALL CFB80
A2B8B CALL BAB60
A2B90 CALL AF0A0   ; original bytes E8 0B C5 00 00
A2B95 CALL CFA90
; AF0A0 body:
AF0CD MOV [EBP-10],CX
AF0D1 CALL 1B26A0  ; native helper, caller cleans 2 dwords
AF10D CALL AEF10   ; Stage52 first hook
AF112 CALL AED90   ; Stage52 second hook
AF119 CALL AE220   ; Stage51
AF136 CALL AE100   ; Stage50
AF13D CALL 1E9170  ; Stage49
```

Call at A2B90 (native AF0A0) is a possible **Stage53 global AF0A0 entrance/exit boundary**. If Stage47→AF0A0 ENTER already differs in X, writer is further up the parent scheduler; otherwise identify the early AF0A0 prologue/helper region before AF10D. No Stage53 hooks/runtime yet at time of this report.

The user's Scharlachrot hover-dash and kick-cancel observation remains uncontrolled; raw X changes can represent native engine movement, not evidence of smarter AI. Use a controlled A/B playback before attributing behavioral changes.
