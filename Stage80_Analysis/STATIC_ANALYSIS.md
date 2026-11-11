# Stage80 — static audit of late original `0E8A8E → 0F7150` call

**2026-11-11. SOURCE READY, ORIGINAL PE STATIC VERIFIED. Windows MSVC x86 BUILD and COM-vs-COM RUNTIME NOT YET TESTED.**

## Source evidence motivating the boundary

Latest [Stage79 user capture](https://github.com/Hawky11/1/blob/stage79-runtime-common-selector-positive-evidence/Stage79_Runtime/RUNTIME_ANALYSIS.md), raw upload [1b2aada](https://github.com/Hawky11/1/commit/1b2aadaeabeebf959fa0575cf59023060e481323): 3,808 instrumented `EFFE8→E89E0` calls, **three** real source-family selector changes (P1 seq2901 0→1; P2 seq3404 0→1; P2 seq3740 1→0). Each shares event-code `P+3A30:FFFF→11`, unchanged companion state0, and independent same-player Stage38/Stage76 before/after linkage. Original `155D29` literal-one monitor Stage78 had zero hits. These are *family-window* observations, not exact writer EIPs.

The original `E89E0` function has three earlier conditional branches:
- `E89E9 TEST AL,1` (P+3B7C bit15), `E89EB JE E89F9` skips `E89EE CALL F6960` if clear.
- `E89F9 CMP BYTE [P+3F7C],0`, `E8A00 JE E8A1E` skips `E8A03 CALL F7150` if zero.
- `E8A1E TEST BYTE [P+3B90],10h`, `E8A25 JE E8A33` skips `E8A28 CALL F70A0` if bit4 clear.

The actual Stage79 record fields `f5=P+3B7C=0`, `f11=P+3F7C=0`, `f12=P+3B90=0/2/2` have those guards disabled in the three positive windows (and were stable before/after). Later conditional helpers `F69F0/F6B00/F6EE0` precede a direct `E8A8E CALL F7150`, with early returns possible. **Neither the original E8A8E CALL nor its internal MOV writer EIP has yet been instrumented; this is a candidate route, not proved execution.**

## Original opcode/ABI contract, SHA-pinned

The original user-owned private `AALib.exe` (read locally from AALib.zip; NOT uploaded) has SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. PE32 i386, ImageBase `00400000`. These exact original-byte signatures are checked in the [Stage80 read-only verifier](tools/verify_stage80.py) and/or installer:

| Original RVA | Opcode bytes | Meaning |
|---|---|---|
| `0E8A8D` | `56` | `PUSH ESI` original player argument |
| `0E8A8E` | `E8 BD E6 00 00` | 5-byte native `CALL F7150` |
| `0E8A93` | `83 C4 04 84 C0` | original `ADD ESP,4; TEST AL,AL` |
| `0F7150` | `55 8B EC 81 EC D4 00 00 00 53 56 57` | target prologue; reads P from `[EBP+8]` |
| `0F7D22` | `89 97 50 3F 00 00` | descriptor SETNE computed selector 1/2 |
| `0F7D38` | `89 8F 50 3F 00 00` | descriptor SETE computed selector 1/2 |
| `0F7D43` | `C7 87 50 3F 00 00 00 00 00 00` | selector 0 |
| `0EFFE8` | `E8 F3 89 FF FF` | independent original Stage79 common ancestor |

The original `F7150` enters with cdecl argument `P` on stack, and original caller **must** perform its own `ADD ESP,4` after return.

## Stage80 native boundary implementation

[Stage80F7150LateBoundary.inl](../Stage80_Overlay/src/Stage80F7150LateBoundary.inl) installs exactly **one** guarded, same-length native `CALL rel32` detour at `E8A8E` (not at any selector MOV), and retains Stage79 common outer boundary.

- Entered through original CALL, native stack has `[ESP]=E8A93`, `[ESP+4]=P` from original `PUSH ESI`.
- A paired read-only TLS depth16 / ring4096 wrapper reads original saved `ESI=P`, continuation, argument, P/O/TID/QPC, selector/state/event/flags. It verifies `saved[10]==saved[1]`, `saved[9]=base+E8A93`, object continuity and field masks. Logs `STAGE80_LATE_F7150`, `STAGE80_HEALTH`.
- It then duplicates original arg with **`PUSH DWORD PTR [ESP+4]`**, invokes **native F7150 exactly once**, removes **only the duplicate arg** with `ADD ESP,4`, restores intended native register/flag/FPU/XMM state around diagnostics and `RET` to E8A93, where original `ADD ESP,4` removes the still-existing original arg.
- Original F7150 sees a wrapper-local return address instead of the previous direct E8A93; the diagnostic adds nesting/time overhead and must be Windows/test-verified. C++ logging is done during periodic flush, not directly inside detoured native execution. No additional gameplay-field write or DR/VEH owner.
- Native patch installation checks original bytes and Stage79/78/76 installation state, x86+XMM and no Stage59 worker yet; uses existing named installation mutex, suspension/EIP conflict guard, VirtualProtect and cache flush with rollback attempt.
- [Stage80 adapter](../Stage80_Overlay/src/Stage38CallbackWindow.inl) differs from Stage79 adapter at exactly **three** integration points: new include, InstallStage80Hooks after installed Stage79 and before Stage59 worker, FlushStage80. All previous observers retained.

## What a positive outcome means

If `STAGE80_LATE_F7150 f0=0→1` or `1→0` occurs fully inside a matching changed `STAGE79_COMMON_WINDOW` with the same P/O/TID and QPC enclosure, it assigns the change to **original F7150 call interval**, narrowing potential sources to its branch writes `F7D22/F7D38/F7D43` and nested code. It does **NOT** individually prove one of these MOV EIPs until a more precise instruction-site observer runs.

If Stage79 changes selector but Stage80 does not, source might lie in F69F0/F6B00/F6EE0 or another earlier/later path. A zero-call Stage80 log is meaningful only if Stage79 positively changed selector in the same session; cross-session negative/positive comparisons cannot prove causality.

**Baseline/rollback:** user-tested Stage79 DLL **1,918,976 bytes**; Stage78 **1,911,808**, Stage77 **1,905,664**. Source-only Stage80 does not demonstrate AI enhancements, attacks, damage, movement or gameplay changes.
