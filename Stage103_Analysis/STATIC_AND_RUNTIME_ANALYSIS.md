# Stage103 — native ECX=0 for 0x27 is a separate mode-0 dispatch path (2026-11-12)

## Status / provenance

Offline SHA-verified original AALib.exe analysis; no Windows build or new runtime capture, no new hook, DR ownership or gameplay-state modification; Stage36 OFF. Original EXE SHA256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` (private in user's Library, NOT committed). Reproducible source-only archive `ArcanaAI_Stage103_NativeStatus0_0x27_StaticAudit.zip` delivered in ChatGPT, SHA256 `1c65ddd6f62be935d07b2738b6fe4075f0f9dc4e030a74e75c4f16151f1ac6c4`; it includes `verify_stage103.py`, `test_stage103.py`, expanded analysis and machine results. Native source checks **37/37** PASS; offline synthetic tests **12/12** PASS.

## Important correction of Stage102 wording

Stage102's 15/15 `chosenEsi=0x27`, `ECX=0`, `forwardType1=0` at F7B0A is valid **for the type-1 route**: none appeared among the 167 original Stage93 type-1 FinalCommand records at F7C5D. But `ECX=0` is **not proof that 0x27 was dropped by the original game function**. Stage103 disassembly proves a dedicated zero-status 0x27 **mode-0** route immediately after F7C68. It is not observed by the existing Stage93 F7C5D trace.

## Byte-verified original control-flow facts (RVA)

- **F7ADB**: `MOV ECX,[EBP-8]`; **F7B07**: `CMP ECX,1`; **F7B0A**: `JNE F7C68`. ECX is loaded from caller-local output **status**, not an independently computed gate predicate.
- If status **1**: **F7C59 PUSH ESI; F7C5A PUSH 1; F7C5D CALL F2DE0**. This is the monitored Stage93 type-1 call.
- If status **0**: **F7C68 CMP [EBP-8],0**; **F7C72 CMP ESI,27h**, and `ESI=27h` enters the distinct special branch. It executes original call **F7CAE→E9800**, **F7CB6 PUSH ESI; F7CB7 PUSH 0; F7CC0 CALL F2DE0**. Thus the same helper is called with different **mode 0** versus mode 1 from different native CALL sites.
- At the later status check **F7D55/F7D96/F7D9A**, zero status and ESI=0x27 converge on **F7DA3 CALL CEA20**. All these RVA/opcode and native relative branch targets were independently checked in the unmodified SHA-pinned EXE. Exact downstream behavior in live game was not recorded.
- Original **F7AD1 CALL F3930** receives a status output pointer to the caller's `[EBP-8]`; **F3C3E** explicitly stores `*status=0` on one producer path, **F3C44** stores context, contrasting **F3CE1** `*status=FFFFFFFF` on another. F3930 also calls F30B0 at F3CBC. Which exact producer-internal site generated each of 12 runtime 0x27 outputs was not observed.
- Original **F7A8A CALL F34D0** also receives the status output pointer; **F358E** and **F360B** explicitly write zero when the candidate 0x27 action conditions hold. **F3654 CALL F30B0**; special F30B0 result **F325A returns EAX=0x27** coupled to **F3254 `*status=0`** and **F325F `*context=0`**. For 3 actual F79B6 return substitutions, the exact inner branch is not logged.

## Full existing Stage98 r2 runtime gate classification

Original same-run Stage94 Git blob `fba1df695c4fcac0af357c782cf521af24b91b93` from [8919f8e](https://github.com/Hawky11/1/commit/8919f8ec242f1137c447d2eb4afc7ba0ed0c0e1e), parsed in full: **201 gates**: **167** status1/type1; **34** status0/non-type1. Of those 34, **15** are ESI=0x27 (12 Stage94/F3930 producers plus 3 Stage95/F34D0 bypass), and remaining **19**: 0x0E=5, 0x10=8, 0x20=1, 0x25=4, 0x26=1. All 15 0x27 gates have `originalFlags=00000297` at F7B0A. Stage102's source-event pairing remains valid; Stage103 clarifies its **semantic scope**. No other ECX values were observed at the gate.

## Limits and next step

The native **mode-0 path exists and is statically reachable** from logged inputs in the unmodified original function. Stage103 did **not** prove by a new runtime hook that 15 mode-0 events executed at F7CC0/F7DA3 or caused visible motion/hits/AI improvements. For Stage104, check whether an existing trace observes the F7CC0/CEA20 sequence; otherwise audit a narrow passive source-only observer with original-once/ABI guarantees and strict rollback. **Do not force ECX=1, bypass F7B0A, patch game fields, add DR, or claim improved AI**. Last Windows-tested DLL: Stage98 r2 **2,051,584 bytes**, fallback Stage97 **2,045,440 bytes**.
