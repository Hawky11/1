# Stage39 — callback-to-native-action output, static analysis (source only)

Date: 2026-11-10. Base: [verified Stage38 runtime](../../Stage38_Runtime/RUNTIME_ANALYSIS.md), 4,916 paired Stage38 windows, including 6/6 positive Stage36 candidates reaching actual native callbacks. No Windows Stage39 build/runtime has been performed. Original PE32 i386 AALib.exe from ChatGPT Library `/AALib.zip`, 175,556,608 bytes, SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` (verified in this investigation); preferred image base 0x400000, runtime base 0x800000. Values below are **RVA**.

## Call graph confirmed against original PE

Three targets identified by Stage38 raw EDI for the six positive candidate chains:

1. **RVA 123AF0** (`AALib.exe+123AF0`): obtains a selector via native 1B84B0, reads player data via 125C90, clears P+3ED0 and, when selector != FFFFFFFF, calls **1BD6E0** at callsite **123B24**. The alternate path for sentinel calls 156DA0.
2. **RVA 1DA490**: obtains selector from 1B84B0, calls 125C90, clears P+3ED0, uses P+43B4 and a native switch; function **1DA440** writes actor-relative +280 (selected value), +284 (P+3A36), +1AC (payload minus 0x20), and +278 (copied O+278). Then it calls **1BD6E0** at **1DA592** when selector is valid; fallback 156DA0 when sentinel. These are object state writes, not identified animations.
3. **RVA 12EB10**: reads callback context[3], clears P+3ED0 and checks native guards, then branches through a 32-entry jump table at **12EB74**. Some branches call **131CA0/131D00** to dispatch an action selector. In 131CA0, callsite **131CC5** invokes **E5BF0**; direct opcode E5BF0 writes `P+3F4C = selected raw ID` and may set bit24 in P+3B7C, then callsite **131CDA** invokes F7E70 to install the *next callback* at P+3ED0. The 131D00 variant has same destinations at 131D1C and 131D31. These are state/action-request outputs, not visual hits.

Shared routine **1BD6E0** takes the callback context, compares context[1] to `[P+4]`, and chooses primary action state `P+3EE4` or alternate `P+40AC`. It calls **1B8050** at 1BD7B0, or **1B80F0** at 1BD789. Both write native action-state descriptors: pointer +0x28, index +0x0C, state +0x0/4/8, +0x18/+0x24 and possibly +0x30/+0x34. On return 1BD6E0 invokes 1BD590. **This is a native state/request boundary**, but the producer/consumer of the selected descriptors and rendered animation remain unproven. Avoid assigning animation IDs to any raw fields.

All 10 exact opcode ranges and eight E8 CALL edges are checked using the provided stdlib verifier (and were checked against the SHA-pinned original EXE during preparation).

## Stage39 instrumentation

No new hooks, no wrapper changes, no new trampoline or debugger. Reuses tested Stage38 prepare/finish wrappers at EE7AE and EE7C7, original guarded EE7BE CALL EDI remains untouched. Replace only Stage38CallbackWindow.inl and add Stage39ActionOutput.inl; keep AIQueueTrace.cpp, Stage35ConsumerTrace.inl, Stage36ControlPredicate.inl (r2), Stage29 AITrace.cpp, and ArcanaAI.ini unchanged. Stage38 continues to write its original log and indices.

In the same pending Stage38 TLS record and worker ring, read 30 additional raw DWORDs **only when the saved native EDI is exactly one of the three verified callback RVAs**. Snapshot once after native 156C00 / before indirect CALL, and once after call / before native FC60. Log `STAGE39_OUTPUT_WINDOW` to ArcanaAI_Stage39ActionOutputTrace.log using exactly the corresponding Stage38 `stage38Seq`, player, TID, type/payload, callback, original Stage38 QPC pair and object generation. No action fields are modified; fields are read with existing SafeReadDword and independent masks. `changedMask` describes only readable pairs. If actor pointer P+43B4 or object P+4 changes, their derived fields must **not** be compared as the same structure. Net unchanged does not mean no transient output.

| a index | Native location | Caveat |
|---|---|---|
| 0 | P+3F4C | Raw ID stored by E5BF0 |
| 1 | P+3B7C | Bit24 may be set by E5BF0 |
| 2–3 | P+3ED0, P+3ED4 | Next callback pointer/arg, NOT actual cached EDI |
| 4–10 | P+3EE4 + 0,4,8,C,28,30,34 | Primary action state from 1BD6E0/1B8050 |
| 11–17 | P+40AC + 0,4,8,C,28,30,34 | Alternate action state from 1BD6E0/1B80F0 |
| 18 | P+43B4 | Pointer to optional actor |
| 19–23 | [P+43B4]+14,1AC,278,280,284 | Actor-relative output, only if pointer readable |
| 24–26 | P+3F44, P+3F3C, P+3F20 | Player state/timing selector controls |
| 27–29 | [P+4]+184,278,280 | Object-relative state/lifecycle control |

Full 30-bit mask is `3FFFFFFF` *if all addresses exist*; actor may be null and missing actor bits are normal. This data is **not** a renderer/hitbox hook.

## Runtime acceptance / next step

Require Windows x86 BUILD SUCCEEDED, Stage38 r1 `STAGE38_INSTALLED 2/2`, Stage39 r1 build/ENABLED messages. Every Stage39 row must have matching unique Stage38 row by stage38Seq/side/player/tid/callback/QPC; callbacks exclusively RVAs 12EB10,123AF0,1DA490. Recompute readable changedMask and field count. Preserve zero/unchanged records separately, check Stage38 dropped/overflow/pairMismatch, avoid blaming logger tail flush. Correlate all six natural positive Stage36 chains to exact Stage39 rows and record which native action-state fields change. Then reverse-trace **consumers** of these changed fields toward animation/effect or hit resolution; only instrument a new native consumer after this evidence. No forced input, no bit17/0x1E injection. This source is **NOT** a compiled/installed or live-verified Stage39 result.
