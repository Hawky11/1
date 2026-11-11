# Stage85 r1 long-run positive after-join selector path (2026-11-11)

**STATUS: NEW ALTERNATE NATIVE SELECTOR PATH VERIFIED BY SAME-RUN QPC; EXACT WRITER EIP STILL UNKNOWN.**
Frozen raw user upload [commit e1fd008f1ed8695f9f7cb53b005112f41f9605b7](https://github.com/Hawky11/1/commit/e1fd008f1ed8695f9f7cb53b005112f41f9605b7), containing updated Stage79/80/81/82/84/85 logs captured **2026-11-11 18:19:12–18:21:06 local**. Existing Build.log from **18:03:23** is unchanged; this is the already Windows-tested Stage85 r1 DLL **1,957,376 bytes**, with one existing C4754 warning. Do **not** mix QPC with the previous 18:04 run or Stage84's earlier run.

## Runtime and strict containment

| Stage | Event count | Exact unique parent QPC matches |
| --- | ---: | ---: |
| Stage79 COMMON_WINDOW | 16,747 | — |
| Stage80 F7150 | 15,218 | **15,218/15,218** inside Stage79 |
| Stage82 nested CALL | 3,812 | **3,812/3,812** inside Stage80 |
| Stage84 exact native MOV | 16 | **16/16** inside Stage82 |
| Stage85 F7C68 JOIN | 228 | **228/228** inside Stage80 |

Matching requires same P1/P2 side, player address, object address, thread ID and **integer QPC**. All logged Stages79/80/81/82/84/85 HEALTH error counters (dropped, overflow, mismatch, readFail, filtered) are zero. Stage81 installed **3/3** at F7D22/F7D38/F7D43 but all native MOV counters stayed **zero**. Stage84 installed 6/6, recorded 16 exact writes, **9 actual selector changes** and 7 idempotent writes.

Ten Stage80 net selector changes occurred. **Nine** align with a matching Stage84 native writer; the **tenth** proves another path, Stage80 seq5640. **227/228** Stage85 JOIN snapshots match both their Stage80 PRE and POST selector value; Stage85 seq82 is the exception and demonstrates the selector changing after the join. This is a **P2 0→1** instance, different from the separate earlier Stage84 P2 1→2 event, whose exact writer EIP is also unknown.

## Frozen positive window

| Boundary | Seq | QPC | selector P+3F50 | companion P+3F54 |
| --- | ---: | ---: | --- | --- |
| Stage80 F7150 PRE | **5640** | 2073365374279 | 0 | 1 |
| Stage82 F3930 | **1453** | 2073365374294–2073365374315 | 0→0 | 1→2 |
| Stage85 JOIN at F7C68 | **82** | 2073365374319 | **0** | **2** |
| Stage80 F7150 POST | **5640** | 2073365374386 | **1** | **2** |

All have **side P2**, **player 0DF4CCA0**, **object 2A3CFC50**, **TID 2516**; Stage85 captured **ESI=0x20, local [EBP-8]=0, EBX=FFFFFFFF**. No Stage84 exact writer event occurs in Stage80 seq5640. Original Stage80 native EAX return is **00000001**.

**Established:** P2 selector 0→1 occurs after F3930 return **and after the F7C68 checkpoint**, but before the original F7150 returns. The precise instruction that wrote memory has **NOT** been observed, and there is no demonstrated improved COM AI/gameplay.

## Native static check and next Stage86 source-only probe

SHA-pinned private original AALib.exe (SHA256 **28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba**) shows:
- F7C68 original `cmp DWORD PTR [EBP-8],0; jne F7D52`; observed 0 goes via F7C72.
- Observed ESI=0x20 selects F7CD0 path to F7CDA; F7CE6 sets ECX=EDI=P, then F7CEC or F7CF0 pushes exactly one 0/1 argument.
- Native call at **F7CF2**, exact five bytes **`E8 69 08 FF FF`**, resolves to **E8560**; return F7CF7.
- Native E8560 uses `ret 4` at verified three exit RVAs **E858E, E8636, E86CA**. Original subsequent F7D22/F7D38/F7D43 MOV instructions exist, but Stage81 did not observe their execution.

**Stage86 next:** passively observe original E8560 call boundary at F7CF2 (original once, same ECX and copied arg, matching RET4 ABI); compare P+3F50 before and after E8560 in a new positive Stage80 window. This determines whether to inspect E8560 internal callees or later F7150 instructions. One guarded CALL hook; no game-value writes, no DR register ownership or gameplay changes. Source prepared and statically checked, **Windows x86 BUILD and game runtime Stage86 NOT TESTED**. Preserve current 1,957,376-byte Stage85 DLL, all frozen logs, and `EnableStage36ControlToCommandTrace=0` because of Stage82 F79E4 collision.
