# Stage128 — F30B0 fallback 0x27 provenance / positive-control three-site observer FEASIBILITY (STATIC ONLY)

**2026-11-13. No new DLL, detour, installer, gameplay run, data or DR writes.**

## Original binary and real source baselines
- Original private user AALib.exe, PE32 i386, **175,556,608 bytes**, original SHA256 **28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba**, preferred imagebase 0x400000. SHA and bytes independently reverified.
- Known source package Stage122 `Stage81SelectorWriter.inl` SHA256 **3a78a8b6b68fda0999d33a2d392dd781fffa4c5bfd67a31b7fa2cdb5b8fadd1d** and `Stage122ObjectWriterTrace.inl` SHA256 **4ae2a8f42774733f572b25d7f64df0c58ec005e52ae55080a6a718be4a75c1c1** verified exactly. These are the local delivered Stage122-package source files, not independently fetched current user Windows files. They remain unchanged.

## New critical disambiguation: 0x27 can appear from TWO different original exits

- **F325A** `B8 27 00 00 00` => native `MOV EAX,0x27`. This is the original **forced fallback** path after Stage127's prerequisites and no active record among 40+176, assuming no other exit. The immediately preceding original `F3254 MOV DWORD [ECX],0` has already run; immediately subsequent `F325F MOV DWORD [EDX],0` **has not** when observing pre-instruction at F325A.
- **F326A** `8B 45 0C 5F 5E` => `MOV EAX,[EBP+0x0C]; POP EDI; POP ESI`, returning the **input candidate**, which might itself be `0x27`, without executing F325A. A future detour must replay all **three** original instructions, including both POPs.
- Merely recording EAX=0x27 downstream (e.g. Stage123 MODE0) is **NOT proof** of the F325A fallback or of all 216 records inactive.

## Minimum proposed 3-site future observation, NOT implemented in Stage128

| Role | Original RVA | Whole-instruction bytes | Original behavior to replay once |
|---|---|---|---|
| Positive F30B0 entry | `0F30B0` | `55 8B EC 83 EC 08` (6) | `PUSH EBP; MOV EBP,ESP; SUB ESP,8` |
| Forced 0x27 branch | `0F325A` | `B8 27 00 00 00` (5) | `MOV EAX,0x27` |
| Input-candidate branch | `0F326A` | `8B 45 0C 5F 5E` (5) | `MOV EAX,[EBP+0x0C]; POP EDI; POP ESI` |

At F30B0 entry (before original prologue), `EAX` is object pointer, `[ESP]` return VA, `[ESP+8]` original candidate. Complete original E8 scan found **exactly two direct callers** of F30B0: `F3654 E8 57 FA FF FF` (return `F3659`) and `F3CBC E8 EF F3 FF FF` (return `F3CC1`, inside F3930). This permits caller discrimination *without an additional F3930 hook*, for those two direct callers; indirect call origins are not excluded.

For safe correlation across stack reuse, entry can derive `originalFrameEBP=entryESP-4` from the prologue; branch-exit hooks still have native `EBP` identifying the same frame. **But** a reliable final join requires a unique real per-entry call-scope ID in addition to sid/process, TID, frame, caller and QPC, ideally a reviewed per-thread scope stack. Mere QPC proximity or frame equality is insufficient. Unknown caller, nested scope, partial capture or mismatch => UNKNOWN. F3CBC's original post-call `F3CC1 ADD ESP,0x10; F3CC4 MOV [EBP+0x10],EAX` was verified.

## Exact independent offline checks
- **188/188** original opcode context anchors from Stage127 reverified at original RVAs.
- **30/30** native branch/Jcc targets re-decoded from the raw displacement bytes.
- **3** proposed sites + **12** original adjacent-context addresses = **15/15** verified signature sites.
- **5/5** direct E8 branch signatures and rel32 targets, including both F30B0 calls, both F30B0→CBE00 record checks, and F7AD1→F3930; full byte-scan independently confirms **F30B0=2** and **CBE00=2** direct callers.
- **17** known hook/external exclusion envelopes (Stage81, Stage104, Stage107, Stage110, Stage113, Stage122, external unknown F2DE0). **54/54** pairwise comparisons do not overlap the three proposed ranges, counting 3 internal comparisons. Stage104/107/110 and unknown F2DE0 use **conservative 16-byte exclusion envelopes**, not exact claimed patch lengths. This does NOT prove absence of unknown loaded third-party hooks.
- **294/294** Python stdlib offline tests PASS, including SHA, original bytes, branch/call target tampering, short/overlapping detours, wrong session/thread/frame/caller, duplicate/reordered scoped events, invalid read mask and nonzero health errors.
- User ChatGPT ZIP **`ArcanaAI_Stage128_F30B0_Fallback27_PositiveControl_DesignAudit.zip`**, **26,804 bytes**, SHA256 **35416a8c3c8086fa0921ef849d1271c7dc2393533280193861d5c7ab500ea142**, **19 files**, CRC PASS, full Russian audit + original-native catalogs + strict future-log analyzer + synthetic-only fixture + standalone verifier/test suite. No copyrighted original EXE, source detour, DLL or executable installer included.

## Future log contract: HYPOTHETICAL, not actual game evidence

The archive's `STAGE129_PRODUCER_EVENT_V1` design defines `entry` / `fallback27` / `returnInput` events with `sid,tid,scope,frame,caller,kind,seq,qpc,candidate,readMask,valid`, all-hit `STAGE129_HEALTH` counters for the same roles and explicit dropped/mismatch/readFail. Future matching must have unique ordered same sid/TID/scope/frame/caller events; otherwise mark UNKNOWN, do not confuse sampled EVENTs with all-hit counters. The included SYNTHETIC_ONLY log has two numerically equal 0x27 outcomes: one actual fallback hit and one retained input=0x27. **No Stage129 native runtime log exists.**

## Stage129 handoff and limits

If tracking the exact source of special numeric 0x27 materially advances the game AI diagnosis, implement **separate SHA-guarded, opt-in Stage129 source-only code** with positive entry and both exit hooks. Requirements: fail-closed loaded PE + original site signatures, coexistence checks for Stage81/104/107/110/113/122 and external F2DE0, suspended-thread EIP, mutex/atomic rollback, exact native replay of 6/5/5 bytes, preserved GP/EFLAGS/FPU/SSE, no new gameplay memory/DR writes, nonreentrant stable call-scope IDs, a real fresh MSVC x86 build and game log before any execution claim. **Stage128 itself is a completed source-free feasibility audit**, no Windows build or game run.

Stage123's **25/25** MODE0 0x27→CEA20→POST remain real observed events but their F30B0 branch origin still unknown. Neither proposed hook nor this study demonstrates accepted game command, animation, hit or improved AI. **Stage36 OFF.** Preserve Windows-tested Stage122 DLL **2,089,984 bytes**, fallback Stage115 **2,082,304**, Stage113 **2,079,744**, Stage110 **2,073,088**.

[Stage127 full producer analysis](https://github.com/Hawky11/1/blob/stage127-producer-recordflag-f30b0-cbf50-static-audit/Stage127_Analysis/FULL_STATIC_PRODUCER_AUDIT.md) · [Stage123 fresh COM-vs-COM evidence](https://github.com/Hawky11/1/blob/stage123-positive-g4-writer-windows-runtime-audit/Stage123_Runtime/FULL_G4_WRITER_SAME_SESSION_AUDIT.md).
