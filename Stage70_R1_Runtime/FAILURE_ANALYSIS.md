# Stage70 r1 failed installation — confirmed relocation-unsafe guard

Date: 2026-11-11. New user upload in main: [afb6692a](https://github.com/Hawky11/1/commit/afb6692a7044b0bde487de39b44f9120d7a4f6a1) (UTC 2026-11-10 23:41:31); same 2026-11-11 02:38:39–02:39:15 local session. This is **NOT** Stage70 runtime evidence: the new hook never installed.

## New build and installation

- `Build.log`: 11.11.2026 02:38:02, MSVC 14.51.36231 x86, **BUILD SUCCEEDED**, DLL **1,892,864 bytes**. Existing C4754 warning in AICommandSiteTrace.cpp:785; no build failure.
- Stage70 log (171 bytes): `STAGE70_DEFERRED_BUILD=2026-11-11-r1`; then `ERROR: Stage70 prerequisites or signature mismatch; no patch.`; **no** `STAGE70_INSTALLED`, **no** `STAGE70_DEFERRED_WRITE`.
- Stage65 installed 1/1, **218** writer events, 18 HEALTH records, no nonzero dropped/overflow/mismatch/readFail.
- Stage67 installed 2/2, **4,128** native windows (4,126 kind=0, 2 kind=1), 18 HEALTH records, no nonzero dropped/overflow/mismatch/readFail.
- Stage59 started after Stage70 install attempt, then WATCH_ARMED (DR0/DR1), **12,614** X_WRITE events, 19 HEALTH records, no nonzero relevant counters. Stage41 has 220 object commit windows. All share same init at 02:38:39 or 02:38:40 and same P1 player `0E4A1F78` and P2 player `0E4CCCA0`, P1 O=`2A449020`, P2 O=`2A449C50`, main TID=7320. Session did not demonstrate a Stage70 write.
- Root blob provenance: Stage70 `93302c793c0337238890d6b7f3062c35374a6b6f`; Build `696914d0dcb7d177fd609c4908263c7300cd18e9`; Stage65 `e6bc0a8adc3e2b4251c8e9c84e58b3c7ff754fcc`; Stage67 `0d902e959a90a05bd6df926d3695c75cb82e55eb`; Stage59 `6592b2af7216629792c0b3a6f8adafefe73e9202`; Stage41 `cdc0420aedacf3db4ded48f2919946e3e6cb9804`.

## Root cause: an original-file absolute VA was compared against relocated runtime bytes

In Stage70 r1 `S70OriginalOK()` the `prev[]` guard at original RVA `15677D` was:

```text
8B 14 8D E8 54 82 00    mov edx,[ecx*4+008254E8]
```

Original `AALib.exe`: SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`; PE32 image base `00400000`. The PE relocation table contains **IMAGE_REL_BASED_HIGHLOW (type 3)** at exact original RVA `156780` (32-bit operand), as well as at `0E9E5C` (the earlier Stage67 E9E60 hook's already relocation-aware operand). The Stage67 live return address `008AD761` establishes the runtime module base `00800000` for this run (RVA `AD761`), a delta of `+00400000`. Accordingly, the expected actual memory bytes of that instruction are:

```text
8B 14 8D E8 54 C2 00    mov edx,[ecx*4+00C254E8]
```

The Stage70 r1 comparison expected `...54 82 00` (the **preferred-base file bytes**) at the **runtime relocated** instruction. This guarantees the guard rejects normal rebasing. The other Stage70 site instructions are position-independent and match exact original bytes:

```asm
156784  8B 45 08  mov eax,[ebp+08]
156787  89 50 68  mov [eax+68],edx
15678A  8B 0F     mov ecx,[edi]
```

This is a sufficient, verified explanation for the r1 install failure; the r1 log does not independently print each prerequisite, so it does not prove all other checks succeeded. The Stage67/65 installed markers and Stage59 worker ordering support expected gating.

## Stage70 r2 correction

[Branch stage70-r2-relocation-aware-guard](https://github.com/Hawky11/1/tree/stage70-r2-relocation-aware-guard):

- In `Stage70_Overlay/src/Stage70DeferredWriter.inl`, replace the seven-byte, relocation-unsafe `prev[]` match with a three-byte invariant prefix `8B 14 8D` and **read** the relocated operand at `g_gameBase+0x156780` with `SafeReadDword`. Require operand to equal `g_gameBase+0x4254E8`.
- Keep both original target instructions and continuation guard unchanged. Keep hook/movement behavior unchanged; no new DR/VEH.
- Add one `STAGE70_GUARD` diagnostic with x86, xmm, Stage67/65/59 status, relocated operand, runtime expected VA, and signature result. This makes any further install refusal independently diagnosable without ungrounded speculation.
- Updated `verify_stage70.py` checks original PE relocation entries of type HIGHLOW at both RVAs and computes expected absolute addresses under multiple image bases. Private EXE was inspected offline; both relocation entries existed, and the source's new guard logic structurally reflects that.
- **No r2 build/test yet**. r1 DLL must not be called Stage70 runtime verified.

## Additional actual same-session P2 2E/24 chain (not Stage70 instrumentation)

A separate current-run observation supports Stage69's third nominal immediate/deferred mapping:

1. Stage67 P2 seq1131 POST QPC `1508731492971`: O+68=`FFFFFFFF`.
2. Stage65 P2 seq73 callerCallRVA `00156769`, request `0000002E`, same O=`2A449C50`, TID=7320, QPC `1508731593057–1508731593060`.
3. Stage41 P2 seq73 PRE QPC `1508731593137` already has O+68=`00000024`, pending `21`; POST pending `21→2E`, deferred stays 24.
4. Stage67 P2 seq1133 PRE QPC `1508731593179`: deferred `00000024`, gate O+30=0; the same window activates immediate `2E` while deferred 24 persists.

This is strong **same-session static/native-caller runtime linkage**, not a direct observed `156787` CPU/store hit. Do not infer actual selector `P+3F50` values, which were not logged.

## Next

No additional user log upload or gameplay test is needed for the failed r1. Apply the **two r2 overlay files** on the same working Stage70/67 tree (Stage70DeferredWriter.inl updated; Stage38CallbackWindow.inl is unchanged from r1), run `verify_stage70.py` and `test_stage70.py`, complete the still-pending compiled-wrapper x86 differential/ABI checks, then MSVC x86 Build.bat. Only after validation may a short COM-vs-COM Stage70 r2 capture prove the positive O+68 store. Preserve Stage67 good DLL 1,887,232 bytes, Stage65 1,879,552 and Stage59 r2 1,153,024 rollback builds. Do not overwrite main or past archived evidence.
