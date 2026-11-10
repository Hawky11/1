# Stage48 — native variant binding runtime VERIFIED

Date: 2026-11-10. Original user uploads were split into commits [7c32c4a](https://github.com/Hawky11/1/commit/7c32c4adc59b6570f62c3af30d2769c73c0d8b86), [e4230d9](https://github.com/Hawky11/1/commit/e4230d9a52d60c8b58e3304ee29d56f2b6bc05b6), and [72bd22d](https://github.com/Hawky11/1/commit/72bd22dcfffbb324e2ec8bd37409aed4060ac8b1). Evidence branch created **at exact final upload HEAD** 72bd22d before adding reports/CSV; original runtime logs retained unchanged.

Source [stage48-native-variant-binder-trace](https://github.com/Hawky11/1/tree/stage48-native-variant-binder-trace); [2,317-row Stage48 binding CSV](stage48_variant_binding_join.csv), [8,052-row Stage47 downstream join CSV](stage47_to_stage48_join.csv).

## Build / installation / data integrity

Windows x86 Build.log started 2026-11-10 13:54:23 local, `BUILD SUCCEEDED`, DLL **1,067,008 bytes**. Prior AICommandSiteTrace.cpp warning C4754 remains; no compiler/linker errors.
Stage48 log marker `STAGE48_VARIANT_BIND_BUILD=2026-11-10-r1`, confirmed `STAGE48_INSTALLED 2/2 callsites=1E8DA8,1E908F native=1E8490 wrapper=Stage44`. Working Stage44/45 hooks installed 1/1, Stage46 enabled, Stage47 INSTALLED 2/2.

All five relevant native logs read **in full via exact Git blob SHA**:
- Stage48 914,320 bytes: 2,317 contiguous records, seq1..2317 as shared Stage44 sequence; P1 and P2 observed; all 2,317 callerRead=1, Stage44 FF/FF, Stage46 1FFF/1FFF, resolverCount=1, QPC ordered, complete and newline terminated.
- Stage44 1,047,038 bytes: 2,317 records, Stage45 896,434 bytes: 2,317 records, Stage46 1,458,790 bytes: 2,317 records; exact 2,317/2,317 cross-stage binding join on stage44Seq, side, object, player, TID, QPC, native resolver EAX equals definition POST. No truncation.
- Stage47 4,593,812 bytes: **8,052** ordered P1/P2 consumer records; all from caller **000AE70C**, no AE93E P1/P2 events, all pre/post masks 1FFF, extraMask F, copied argument zero, changedMask matches raw IEEE754 differences, 0 malformed/sequence/QPC errors.
- Stage44 HEALTH: calls 64,369, otherObjects 62,052, matched P1/P2 2,317. Stage45 HEALTH same. All dropped/overflow/mismatch=0. Stage47 HEALTH calls 73,207, otherObjects 65,155, matched P1/P2 8,052, dropped/overflow/mismatch/badCaller=0. Stage48 has no separate queue/health; it uses Stage44 ring.
- Stage40/41 482/482 matched, Stage42/43 349/349 caller AD75C; 345 distinct Stage41 bit0 0→1 writes match 345 Stage42 activations, four extra Stage42 activations with known native caller AD75C but no newly observed Stage41 bit0 write: seq3,4,156,293. These are not proof of missing commands. Stage36/37 and Stage39–48 smaller logs checked as full UTF-8; old Stage35/38 very large logs preserved, not fully inspected as part of this pass.

## Native caller attribution: successful

Stage48 has **2,317** events distributed by exact caller:
| Caller RVA | P1 | P2 | Total | Interpretation |
|---|---:|---:|---:|---|
| `001E908F` | 898 | 1,026 | **1,924** | Native 1E8DC0 timing/variant-update path; newly hooked; runtime observed |
| `001E9E2D` | 200 | 193 | 393 | Existing Stage44 action-initial binding |
| `001E8DA8` | 0 | 0 | 0 | Installed successfully but this branch not observed for P1/P2 |

In all 2,317 captured windows, native action ID and O+6C **variant PRE/POST inside binder** are unchanged; native binder changes O+34 definition pointer in **2,313/2,317**, four unchanged. This is expected: at callsite 1E908F the native **caller** updates O+6C first, and the binder recalculates the corresponding definition, not the variant itself.

For **all 1,924 runtime 1E908F calls**, compare the previous Stage48 POST for the same side/object/ID to the current binder entry and exit. All 1,924 have matching ID, current binder PRE definition equals last previous POST definition, and **exactly 1,924/1,924** obey:

```text
newDefinitionPost - priorDefinitionPost
   = 0x5C * (newVariantAtBinderEntry - priorVariantPost)
```

Variant delta histogram: +1 **1,703**, -1 **109**, -3 **50**, -5 **32**, -2 **10**, +4 **6**, +2 **5**, +6 **3**, +3 **3**, -7 **1**, +5 **1**, -11 **1** (total 1,924). Both advancing and backward/reset jumps use one fixed 92-byte row stride, with zero exceptions. It is now verified at the **actual native binder instruction site** rather than extrapolated only from Stage47 snapshots.

## Stage48 → downstream Stage47 consumer

Each Stage47 window joined to nearest earlier Stage48 binder (same side/object, requiring binder qpcAfter strictly before consumer qpcBefore):
- All 8,052 have a preceding binder in this run and matching **active ID**.
- **8,051 / 8,052** also match the binder POST variant and definition pointer.
- All 2,284 first downstream consumers for distinct latest binders match **all 8** Stage46 POST float raw fields at their Stage47 PRE. This is strong producer→consumer value continuity.
- Of the 8,052 Stage47 calls **5,229** change ≥1 tracked float field (64.94%). Per-field overlapping changed counts O+7C=3,662, O+80=3,568, O+88=1,201, O+8C=4,299, O+90=0, O+94=16, O+150=105, O+154=18. All Stage47 callRVA AE70C.
- One isolated exception at the **very end**, Stage47 seq8051 P2, ID 0000050C, variant=4, definition=29C66E18. Nearest recorded Stage48 P2 seq2316 was variant=3 and definition POST=29C66DBC; difference +0x5C. Stage47 seq8051 QPC=1050761743422; last logged Stage48 P2 seq2316 QPC=1050760930174 (Stage48 seq2317 was P1). This could reflect an unrecorded/late-flushed binder, different native update path or end-of-capture ordering; insufficient to establish loss, runtime bug or invalid hook. Stage44 health counters are zero, but lack of a recorded binder in this one interval remains open.

## New static downstream direction (for Stage49)

SHA-pinned physical `AALib.exe` from ChatGPT Library AALib.zip, length 175,556,608, SHA256
`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
The original .text at **RVA 1E9170** has native thiscall ECX=O, prologue `55 8B EC 83 EC 08 56 8B F1`, no stack arguments. A guarded branch 1E92C8..1E9301 gets linked pointer `[O+2D0]`, compares `[linked+60]` to `[O+280]`, reads float raw `[linked+7C]` and `[linked+80]`, then writes current `O+7C`, `O+AC`, `O+80`, `O+B0`. This is concrete native state/position-field propagation but not yet proof of visible animation/collision.
Two verified native direct CALLs target that function: **AF13D → 1E9170** (bytes E8 2E A0 13 00), inside the main `E2E02C8` linked-list loop, and **AF1AA → 1E9170** (bytes E8 C1 9F 13 00), inside a *different*, conditionally enabled `E2E02C0` list. The root instructions are respectively AF11E `8B 35 C8 02 2E 0E` and AF19E `8B 35 C0 02 2E 0E`. Their native ECX is the respective list element; only AF13D is preselected as likely P1/P2. AF1AA may capture no P1/P2 at all; it can be included as a scoped branch-coverage control, not assumed to be the same list. Scope next passive Stage49 to these two verified CALL boundaries with PRE/POST O and linked pointer snapshots. Avoid unverified gameplay effect claims or behavior changes. No native EXE reupload.

## Conclusion

Native action variant progression → native definition resolver → native float-state update is now strongly supported by direct P1/P2 runtime evidence and precise 0x5C row arithmetic. Stage48 reaches the native variant transition caller with **no dropped/overflow/mismatch**. Stage49 should examine whether/when numeric coordinates are transferred into linked object state, and whether it is observable for both P1/P2. The project has not yet proven improved AI decisions, rendered animation, collisions or damage.
