# Stage121 — original writer probe feasibility, STATIC DESIGN ONLY

**2026-11-12. No new DLL, runtime hook, Windows compile, game run or gameplay writes.**

## Decision

The historical Windows Stage116 logs measured **1,455 full post-FA480 global tuples (0,0,3,7,0)** and **16,760/16,760** FA480 AL=0/ZF=1 bypasses on the observed parent-A path. Original-native static Stages118–120 established one *possible* object-state=3 / G4-or-G5=7 writer chain, but Stage116 did not record when that writer executed. **No static audit can retroactively identify the actual historical last writer.** Even a new positive candidate-writer hit will not prove it is globally the only or last writer because other direct, indirect and bulk writers exist.

If the precise object provenance materially improves the original AI investigation, a future **3-site passive** positive-controlled diagnostic is the smallest defensible initial test:

| Original RVA | Complete original bytes | Bytes | Purpose |
|---|---|---:|---|
| 1A0C81 | E8 AA E2 FF FF | 5 | Parent CALL to 19EF30; ECX points to object; positive reachability control |
| 19F295 | C7 46 10 03 00 00 00 | 7 | Native object.state=3 store; intermediate hit |
| 19F2AD | C7 07 07 00 00 00 | 6 | Native selected G4/G5 slot-head=7 store; specific candidate-writer hit |

The parent at 19F410 has pre-child G3 copy 1A0C79. parentControl=0 **does not rule out** that the virtual method was entered but returned earlier. At a future 19F2AD hit, EDI must be observed and classified as actual G4 or G5, not inferred from a stale flag snapshot.

## Verified independent local checks

- Original user's private AALib.exe: **175,556,608 bytes**, original SHA256 **28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba**, PE32 x86 preferred base 0x00400000.
- **30/30 fixed original x86 instruction signatures** (3 hook candidates plus 27 context instructions); native direct rel32 CALLs 1A0C81→19EF30 and 19F2A0→F9900 verified.
- Site boundaries align with original instruction ends (5/7/6). Three site ranges do not overlap known Stage104/107/110/113/115/F2DE0 native reservations. This is not a claim that any unknown runtime third-party patches are absent.
- Exact Stage115 baseline source SHA256 **273059538a37cb1cfb82ab9c6eb891d0e535c59bcae1e41c42f06c27ff336d3a** checked, including Stage113 C2668 fix. Existing source NOT modified by Stage121.
- **55/55 offline stdlib unit tests PASS**, covering corrupted byte sites, unsafe instruction extents, false cross-session/object/TID matches, reordered/ambiguous QPC events, partial/invalid records and health error flags.

Reproducible user-facing ZIP: **ArcanaAI_Stage121_Passive_WriterTrace_DesignAudit.zip**, **18,862 bytes**, SHA256 **6f8a23e52b4b04d5d5f886cd0bbcc0fffd0e950ac5d60aa564375b3ac97ce082**, **11 files** (full Russian report, exact-source/EXE verifier, prospective offline correlator, synthetic-only log fixture, 55 tests, results). NO EXE, DLL, patch installer, hook implementation or real Stage122 runtime logs are included.

## Hypothetical Stage122 acceptance contract — NOT IMPLEMENTED

Records would need a unique session identifier, TID, exact object pointer, QPC, site kind (parent/state3/slot7), actual EDI and selected G4/G5, readMask, validity, original-once replay, all-hit counts and queue drop/worker status. Only unambiguous ordered joins within the same session, thread and object may count; missing or repeated matches are UNKNOWN. Positive parent count is mandatory before interpreting writer=0. Do not use stale Stage116 timestamps to join future rows. An explicit nested-call ID is preferable to QPC alone.

Any future source-only hook implementation must fail closed on original SHA, loaded code signatures and prior hook ownership; mutex, suspension, EIP-region checks, full rollback and exact replay-once are required. Preserve flags, GP/FPU/XMM, original CALL return behavior. Original native stores should execute once, with **no extra data writes, DR operations, F2DE0 redetour, forced AI commands or synthetic actions**. Stage36 OFF. No Windows build or runtime installation is claimed.

## Recommendation

For the existing AI diagnosis, the FA480=0 branch has already been explained by observed global values; a G4 writer defect is **not proven**. Defer risky detours unless the user specifically needs current-run object writer provenance, then implement/review a guarded positive-controlled Stage122 source-only overlay before Windows testing.

Preserve rollback DLLs: **Stage115 2,082,304 B**, **Stage113 2,079,744 B**, **Stage110 2,073,088 B**.

[Stage120 original lifecycle audit](https://github.com/Hawky11/1/blob/stage120-object-lifecycle-vtable-g3-before-g4-static-audit/Stage120_Analysis/FULL_STATIC_OBJECT_LIFECYCLE_AUDIT.md) · [Stage116 actual Windows evidence](https://github.com/Hawky11/1/blob/stage116-fa480-five-globals-runtime-verified/Stage116_Runtime/FULL_SAME_RUN_AUDIT.md).
