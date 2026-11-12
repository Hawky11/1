# Stage118 — original AALib.exe G3=3 / G4=7 indirect-writer CFG (STATIC ONLY)

**2026-11-12 · no Windows build, no new DLL/hook/game-state modification.**

## Provenance and validation

Original user's `AALib.exe` in private `AALib.zip`: **175,556,608 bytes**, PE32 x86, imagebase `0x00400000`, original SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` — reverified independently.

Local stdlib verifier: **35/35 exact native instruction signatures**, **9/9 PE HIGHLOW relocated pointer operands**, **3/3 E8 rel32 CALL edges**, **2/2 native short JNE targets**, and **31/31 Python unittest PASS** (including corrupt-byte and branch-failure cases). Fresh reproducible ZIP `ArcanaAI_Stage118_G3_3_G4_7_IndirectWriter_StaticAudit.zip` **17,227 B**, SHA256 `c6ee16c20a0cf0b16bb45d43b032ca371bae82a668fddffe7f3885f612a058dd`, 8 text/source/test files (no original EXE/DLL). Available as ChatGPT file attachment.

## Discovery: *original* indirect G4/G5 writer RVA 19F2AD

In Stage117, 34 `G1..G5` direct stores omitted writes where the base is computed. Stage118 shows a concrete native case:

```asm
19F223 mov al,byte ptr [esi+2Ah]
19F226 test al,al
19F22A cmp byte ptr [esi+2Bh],al
19F22D je 19F409              ; if both flags zero -> leave this path
19F243 setne bl               ; mapIndex = ([ESI+2Ah] != 0)
19F24A sete cl                ; slotIndex = ([ESI+2Ah] == 0)
19F250 mov eax,[0E10BEDCh]   ; additional original selector, not sampled in Stage116
19F255 imul ecx,ecx,278h
19F25B add ecx,0E10B640h
19F261 mov edi,ecx            ; G4 + 0x278*slotIndex
19F289 cmp eax,8
19F28C jne 19F2D1
19F295 mov dword ptr [esi+10h],3
19F2A0 call F9900             ; original map lookup, argument = EBX
19F2A8 cmp eax,-1
19F2AB jne 19F2B3
19F2AD mov dword ptr [edi],7  ; proven indirect store to selected G4 or G5
```

- If `[ESI+2A]!=0`: `BL=1`, `CL=0`, `EDI=G4=0x0E10B640`. For `modeWord[0x0E10BEDC]==8` and `F9900(1)==-1`, **G4 is assigned 7** and object `[ESI+10]` is assigned 3.
- If `[ESI+2A]==0` **and** `[ESI+2B]!=0`: `BL=0`, `CL=1`, `EDI=G5=0x0E10B8B8`. If mode=8 and `F9900(0)==-1`, **G5 is assigned 7**.
- If both flag bytes are zero, the code branches to `19F409` before selecting the slot.

`F9900` at `0F9900..0F991C` only accepts index 0/1 and returns `map[index]` from `0x05146280 + index*4` (or -1 for invalid index). Here index always 0/1, hence a returned -1 arises from the **map entry**. For the first-slot route this is `DWORD[0x05146284]==0xFFFFFFFF`.

Original map reset `F9920` writes four `0xFFFFFFFF` entries at `0x05146280..0x0514628C` (1 direct E8 caller `1A2D13`); map indexed writer `F9940` writes to `[ECX*4+0x05146280]` (23 original direct E8 callers). Raw E8 callsite scan: `F9900` 91 sites, `F9920` 1, `F9940` 23, `19EF30` 1. These counts are *original static*, not execution counters.

## G3 needs an additional synchronization step

`19F295` writes **object state** `[ESI+10]=3`, **not G3**. Separate original global sync sites:
- `19F96D MOV EAX,[EDI+10]` → `19F973 MOV [G3],EAX`;
- `1A0C76 MOV ECX,[EDI+10]` → `1A0C79 MOV [G3],ECX`, followed by `1A0C81 CALL 19EF30` (the copy is **before** this call).

Other object-state-3 stores exist at `19EFDD` and `19F768`. Later bulk resets or slots swaps can change G3/G4/G5. Thus we have **proved a static possible writer/provenance chain**, not uniquely identified which writer ran last during the user's game.

## Relation to Stage116: do NOT overstate

Previous, separate Windows runtime: **1,455** complete post-FA480 snapshots all showed `(G1,G2,G3,G4,G5)=(0,0,3,7,0)`, predicted=observed AL0/ZF1. Stage113 full-hit HEALTH had **16,760/16,760** original parentA JE bypasses.

These data **are consistent** with the first-slot path `selector=8; [ESI+2A]!=0; map[1]=-1; G4=7` followed by global G3 synchronization; they **do not prove that path executed**. Original runtime did not observe `[0x0E10BEDC]`, `map[1]`, object flags, or write-site hits. No semantic mode names, forced gameplay behavior, attack/hit/AI quality, or last-writer claim.

## Handoff Stage119

STATIC-first trace **writers and owners of `[0x0E10BEDC]`, `map[0/1]`, and object `[ESI+2A/2B]`**, then decide whether any narrow, guarded read-only runtime observation is essential. Do not force global state, patch FA480, add a conflicting detour, or write DR/gameplay data. Stage36 OFF. Preserve last Windows-tested Stage115 DLL **2,082,304 B**; Stage113 **2,079,744 B** and Stage110 **2,073,088 B** as rollbacks.

**This is static-only; there was no Stage118 Windows build or game run.**
