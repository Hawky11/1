# Stage49 r2 — native linked-motion observer runtime VERIFIED (not a copy-branch hit)

## Inputs and integrity

2026-11-10 local Windows build 15:31:47; loaded game tracing 15:32:21–15:33:11.
Source: [stage49r2-relocation-aware-root-guard](https://github.com/Hawky11/1/tree/stage49r2-relocation-aware-root-guard). Exact fresh user log upload: [8317ae92](https://github.com/Hawky11/1/commit/8317ae92eb2adca8e82e38702ed1cd6deaf9b90f). New Build.log [de975e3](https://github.com/Hawky11/1/commit/de975e380cd5ec33d339bfd113059492f7c13292). This evidence branch starts at that exact commit and preserves its original log blobs.

**BUILD SUCCEEDED**, dinput8.dll **1,075,200 bytes**, one old nonfatal C4754 warning. Stage49 log contains `STAGE49_LINKED_MOTION_BUILD=2026-11-10-r2` and `STAGE49_INSTALLED 2/2 callsites=AF13D(listC8),AF1AA(listC0) native=1E9170`. This resolves the Stage49 r1 abort; the relocation-aware native guard passed in actual Windows x86 runtime. **Stage49 hook installation and data capture are CONFIRMED.**

Exact Git blob `ArcanaAI_Stage49LinkedMotionTrace.log` SHA **0c465c4eda656919bffe3bb6d1fada43addeede6**, 4,175,910 characters. 6,818 complete, contiguous Stage49 windows seq1..6818 (P1=3,409/P2=3,409), all `callRVA=AF13D`; AF1AA no P1/P2 occurrences (different conditional linked list). All 6,818 object snapshots pre/post mask `07FF`, all QPCBefore<=QPCAfter, no missing sequence or malformed records. Link pointer nonzero and all 3 linked fields readable in **5,512** records; 1,306 with link absent. Link pointer values seen: `0E5AF010` in 4,613, `063024F0` in 899, null in 1,306. Stage49's `staticGateCandidate` was **0 in all 6,818 windows**; **changedMask=0000 in all 6,818**. That is: the hooked original function executed; its guarded coordinate-copy path was **not observed for P1/P2** in this tested scenario. No change in the 11 captured fields *within* the native function, and no `linkChangedMask` observed.

32 `STAGE49_HEALTH` samples: calls **58,558**, otherObjects **51,740**, tracked P1/P2 **6,818**, every dropped/overflow/mismatch/badCaller **0**; no error/warning. Full buffer has no observed loss.

**Exact static gate explanation:** original PE32 SHA `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` examined at RVA 1E92BA..1E9301: `MOV EAX,[ESI+10]; SHR EAX,0F; TEST AL,1; JE 1E937C` before `MOV EDI,[ESI+2D0]` and linked ID compare `[EDI+60]==[ESI+280]`. Runtime Stage49 captured `O+10` (f7) as 0x00000000 (6,739), 0x00010000 (62), 0x00000080 (17), **none with bit15 set (0x8000)**. Therefore the tested P1/P2 branch never reaches linked copy under those observed snapshots, independently of linked pointer presence. This does not mean the path is dead globally or could not execute for other game objects/modes.

## Cross-stage integrity

Previous stages stayed installed:
| Trace | New P1/P2 windows | Other findings |
|---|---:|---|
| Stage35 | 13,528 | P1 6,764 P2 6,764, complete seq |
| Stage38 | 6,326 | P1 3,076 P2 3,250, complete seq |
| Stage44 | 1,901 | P1 981 P2 920; shared HEALTH loss=0 |
| Stage46 | 1,901 | mirrors Stage44 binding windows |
| Stage47 | 6,756 | P1 3,378 P2 3,378; 4,243 changedMask!=0; caller AE70C only; 32 health samples calls 58,066, other 51,310, dropped/overflow/mismatch/badCaller all 0 |
| Stage48 | 1,901 | P1 981 P2 920; caller 1E908F=1,533, 1E9E2D=368, 1E8DA8=0 |

[Full 6,818-row CSV](stage49_stage47_stage48_join.csv) joins Stage49 to nearest prior Stage47 and Stage48 completed events by (P side, object, QPC): **6,818/6,818** have both matching predecessors; stage48 POST activeID = Stage49 O+60 on **6,818/6,818**. Last Stage47 POST coordinate O+7C equals next Stage49 PRE O+7C in **5,617/6,818** cases; **1,201 differ**. O+80 agrees 6,816/6,818. These are **intervening numerical changes**, not proof of a specific writer or render movement. Stage49 does not write those fields in its observed windows.

## New static discovery for next stage

Original [physical AALib.exe](https://github.com/Hawky11/1/blob/main/BINARY_SOURCE_IN_LIBRARY.md) reread from the user's Library zip, **175,556,608 bytes**, matching SHA above. In caller loop over E2E02C8, RVA **AF136** has original `E8 C5 EF FF FF`, direct CALL to **native AE100**; then at AF13B `MOV ECX,ESI`; then AF13D calls Stage49's 1E9170. Importantly, native AE100 uses **ESI=object**, no arguments, `RET` at AE216; do NOT treat ECX as thiscall. Physical exact bytes verified at AF130, AF136, AF13B, AE100, AE114, AE1A6, AE1A9, AE1B8, AE1BE, AE216; original CALL rel32 target verified.

Native AE100 reads type at O+3C (compares 4) and O+1A4 (compares 0x32). Its guarded path has direct floating-point stores: `AE1A6 FST O+7C`, `AE1A9 FSTP O+AC`, `AE1B8 FST O+80`, `AE1BE FSTP O+B0`. **This is a strong candidate for at least some of the 1,201 Stage47→49 O+7C differences, but NOT yet proven by runtime.** Do not infer on-screen motion, animation, hit/damage, AI decisions or quality from numerical float state alone.

The same caller corridor also has original direct CALLs to native 1E9170 at **AF162** (different list E2E02BC) and **AF182** (list E2E02B0), in addition to AF13D and AF1AA. Earlier Stage49 static notes described two *selected* hook sites; do **not** interpret this as exhaustive count of original native CALLs. Stage49 watches P1/P2 in list C8 and conditional list C0; no P1/P2 events at AF1AA during this run.

## Handoff

[Stage50 source](https://github.com/Hawky11/1/tree/stage50-prelinked-native-coordinate-trace) adds one passive observer at AF136->AE100, with pre/post 11 object fields and exact gate/caller/QPC. It preserves the already working Stage49 r2, Stage48–35, and installs only if Stage49 works. [Installation guide](https://github.com/Hawky11/1/blob/stage50-prelinked-native-coordinate-trace/Stage50_Overlay/README_STAGE50_RU.md). **Stage50 Windows build and runtime PENDING.** After first Stage50 run, join Stage47 POST → Stage50 PRE/POST → Stage49 PRE on same object/QPC and test whether Stage50 explains the 1,201 X mismatches. Prior Stage49 r2 user-run does not need repeating separately.
