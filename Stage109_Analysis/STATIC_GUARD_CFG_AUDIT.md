# Stage109 — Original CEBE0 guard CFG and four direct callers

**12 November 2026. Static only. No new hook, DLL, game run, DR or gameplay data writes. Stage36 OFF.**

## Provenance and completed verification

- Original AALib.exe user PE32/x86 SHA256: 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba.
- Portable offline verifier: **56/56 PASS**: 38 exact original opcode signatures, 7 rel32 call checks, 8 exact branch targets, one exhaustive raw-.text direct CEBE0 call-candidate scan, two table checks.
- Negative/positive unit tests: **26/26 PASS** with original binary available.
- Previous real [Stage108 runtime](https://github.com/Hawky11/1/blob/stage108-cea20-post-runtime-verified-read-unobserved/Stage108_Runtime/RUNTIME_ANALYSIS.md) confirmed Stage107 Windows build 2,065,920 B and INSTALLED 2/2, 13/13 Stage104 CEA20 PRE to Stage107 POST, but native CEC64 READ zero across 43 health samples. No Stage109 runtime observations.

## CEBE0 execution gates prior to CEC64

1. At CEBE6–CEC02, if 4th argument [EBP+14] nonzero and active byte [record+10] nonzero, original decrements active before later tests.
2. At CEC05, first argument ID == -1 clears active and returns zero.
3. At CEC18–CEC1D, unsigned ID > 0x1EE returns zero.
4. At CEC23–CEC51, original 17-byte-per-ID table at absolute VA 04339898 checks first byte; 0xFF branches to CED37 early return.
5. At CEC57–CEC5B, [record+10] == 0 branches to CECD6 skipping monitored CEC64, *not* necessarily immediately returning.
6. Otherwise original CEC5D/C61 load byte [record+11] and index [record+14], and CEC64 executes CMP EDX,[EDI+EAX*4+28020]. CEC6F and CEC86 access related state fields conditionally.

**Original ID 0x27 table row:** 0c 84 ff ff ff ff ff ff ff ff ff ff ff ff 0a 0a ff. Its first byte is 0x0C, not rejected by 0xFF sentinel. Stage104 ESI=0x27 (mode0 selection) **does not prove CEBE0 ID=0x27**. Stage107 POST field20=0x0A also does not prove a CEBE0 execution.

## Direct callers and source separation

Original raw .text includes exactly four candidate E8 rel32 direct calls to CEBE0, all exact targets verified:

| RVA | Fourth arg | Record bank | Iteration |
| --- | --- | --- | --- |
| 988EA | constant 0 | player+2CC | 64 entries |
| 98948 | constant 0 | player+1184 | dynamic count |
| CAD79 | caller [EBP+20] | player+2CC | 64 entries |
| CADDA | caller [EBP+20] | player+1184 | dynamic count |

Both record banks stride 0x24. Shared state ECX has original VA base 0433C138 and stride 0x2806C. The immediate F7DA3 CEA20 post-return path at F7DA8 checks [EBX+55] and either returns or invokes **CBDE0** via F7DB5/F7DC1; it does not directly invoke CEBE0. This direct scan cannot exclude indirect/dynamic CEBE0 calls.

CEA20 on a nonnull record clears byte [arg+10]. This suggests a plausible reason a later inactive record could skip CEC64, but **no runtime record identity/guard outcome has been confirmed** for the Stage108 events. Static overlap in player-relative address ranges is not identity proof.

## Artifacts and next stage

Offline chat archive: ArcanaAI_Stage109_CEBE0_CFG_Static_Audit.zip (SHA256 **2dd01072437eda1e82b77e1038b77865a6ab4f9cf8c6484cc4b34583120f1c05**), with exact verifier, 26 tests, full RU analysis, README and JSON; no private EXE, user runtime logs, DLL or hooks. The comprehensive RU report is in this ZIP.

**Stage110 if needed:** first review exact ABI/hook ownership and reversible guarded passive entry/guard observations for CEBE0. Distinguish no-call from ID/table/inactive-skips and prevent unbounded loop logging. No forced actions, DR or gameplay writes. Retain latest tested Stage107 DLL 2,065,920 B; fallback Stage104 r2 2,058,240 B, Stage98 r2 2,051,584 B, Stage97 2,045,440 B. No accepted move/animation/hit or AI quality proved.
