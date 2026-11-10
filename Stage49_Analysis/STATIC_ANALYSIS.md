# Stage49 — linked-object motion-state propagation (SOURCE READY)

2026-11-10. Previous Stage48 runtime [VERIFIED](https://github.com/Hawky11/1/blob/stage48-runtime-variant-binding-evidence/Stage48_Runtime/RUNTIME_ANALYSIS.md) and [2,317 native bindings](https://github.com/Hawky11/1/blob/stage48-runtime-variant-binding-evidence/Stage48_Runtime/stage48_variant_binding_join.csv). Stage49 has **not** been compiled under Windows x86 and has **no live runtime**.

## Why follow this path

Stage48 runtime proved 1,924/1,924 direct native binder `1E908F` variant steps map to 0x5C byte definition-record strides; 8,051/8,052 subsequent Stage47 windows match latest binding, with one isolated final P2 late variant step (+1/0x5C) without recorded Stage48 binder. Stage47 native function 1E66E0 itself updates floats O+7C/+80/+88/+8C (5,229 windows with float raw changes among 8,052). This is numerical internal motion state, not proof of on-screen translation/hits.

The original `AALib.exe` already in ChatGPT Library (175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`) was re-read as real PE32 by native objdump. The *different* original native function **RVA 1E9170** has thiscall ECX=object, no stack args, prologue
`55 8B EC 83 EC 08 56 8B F1`, and an ordinary `RET` at 1E9167.

This function contains a guarded linked-object data flow:
| RVA | Original op | Meaning |
|---|---|---|
| 1E92C8 | 8B BE D0 02 00 00 | load linked pointer L = [O+2D0] |
| 1E92CE | 8B 47 60 | load L+60 active ID |
| 1E92D1 | 3B 86 80 02 00 00 | compare against O+280 action ID |
| 1E92DD | D9 47 7C | FLD linked+7C |
| 1E92E6 | D9 56 7C | FST object+7C |
| 1E92E9 | D9 9E AC 00 00 00 | FSTP object+AC |
| 1E92EF | D9 87 80 00 00 00 | FLD linked+80 |
| 1E92FB | D9 96 80 00 00 00 | FST object+80 |
| 1E9301 | D9 9E B0 00 00 00 | FSTP object+B0 |

Additional branch gate at RVA 1E92BA checks bit15 of `O+10`, and the linked ID must match O+280. These are necessary for that static branch but may not be sufficient without other native branch conditions. It is evidence of possible position-state copying, **not yet** a proved game-visible coordinate transformation.

Two exact original CALLs target native 1E9170:
- **AF13D** CALL bytes `E8 2E A0 13 00`; preceded by `MOV ECX, ESI`; P1/P2 expected from **E2E02C8 main linked list** loaded at AF11E `8B 35 C8 02 2E 0E`.
- **AF1AA** CALL bytes `E8 C1 9F 13 00`; preceded by `MOV ECX, ESI`; **conditional E2E02C0 alternate linked list** loaded at AF19E `8B 35 C0 02 2E 0E`, **not** the same as AF13D. This call may have zero P1/P2 observations; do not treat as a failed hook.

Both original CALL target resolutions, root list loads, ECX moves, native prologue/return and 9 linked-field op bytes were compared to the pinned physical EXE. See [verifier](tools/verify_stage49.py) for repeatability.

## Passive instrumentation

[Stage49LinkedMotion.inl](../Stage49_Overlay/src/Stage49LinkedMotion.inl) adds exactly two strict redirect CALLs to a new single-purpose original-once native wrapper `S49LinkedMotionHook`, using the working Stage44-pattern x86 GPR/EFLAGS/FXSAVE/FXRSTOR preservation. Input ECX and native no-args return remain unchanged. Both CALL sites in one 4K page protected in one transaction, with mutex, other threads suspended and original bytes rechecked. If any guard fails no new redirects are applied.

New 4096-window read-only ring, existing Stage38 flush worker; no INI changes, extra thread, memory watches or gameplay input changes. P1/P2 filter uses their original pointer reference (player+4). Before and after original 1E9170, snapshots:
- Object `f0..f10`: O+7C, O+80, O+AC, O+B0, O+60, O+280, O+2D0, O+10, O+14, O+08, O+24 (11 DWORDs, mask 07FF).
- Linked object `l0..l2`: [O+2D0]+7C, +80, +60 (3 DWORDs, mask 7); linked pointer PRE/POST separately.
- Exact callRVA AF13D/AF1AA, side, object, TID, QPC before/after, raw changed bits and `staticGateCandidate` (O+10 bit15 and linked ID match O+280). Candidate is not a claim the native branch executed.
- New `ArcanaAI_Stage49LinkedMotionTrace.log` marker/build/install and independent HEALTH calls/otherObjects/dropped/overflow/mismatch/badCaller.

[Stage49 Stage38 adapter](../Stage49_Overlay/src/Stage38CallbackWindow.inl) is based on the working Stage48 file, with only Stage49 include, conditional installation *after* successful Stage48 and FlushStage49 next to existing Stage47 flush. All Stage44/45/46/47/48 sources must retain working previous versions.

## Next runtime

Archive old logs, compile x86 via existing Build.bat; require BUILD SUCCEEDED and STAGE49_INSTALLED 2/2. Natural COM-vs-COM fight with motion and at least two seconds for worker flush, upload fresh Build.log and Stage49 log alongside Stage48/47/46/45/44 etc.

Verify P1/P2 observations at AF13D and measure AF1AA (could be 0); expected masks, linked readability (not guaranteed for every window), `staticGateCandidate` counts, actual O+7C/+80/+AC/+B0 changed masks, equality with linked PRE raw coordinates under validated branch conditions, and health dropped/overflow/mismatch/badCaller=0. Correlate with Stage47 float update and Stage48 binding by QPC/object/ID. Explicitly flag any gaps; do not overclaim world-space movement, render, collision, damage or intelligent decisions.

No run was carried out in the user's Windows environment for Stage49 yet.
