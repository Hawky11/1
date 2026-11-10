# Stage48 — runtime attribution of native variant-index binder (SOURCE READY)

2026-11-10. Stage47 runtime evidence: [report](https://github.com/Hawky11/1/blob/stage47-runtime-motion-consumer-evidence/Stage47_Runtime/RUNTIME_ANALYSIS.md),
[5,614-row join](https://github.com/Hawky11/1/blob/stage47-runtime-motion-consumer-evidence/Stage47_Runtime/stage47_consumer_binding_join.csv).
Stage48 **Windows x86 build and in-game runtime are PENDING**.

## Why Stage48

Stage47 P1/P2 consumer, 5,614 complete records, 3,703 windows with actual native float-field differences; all Stage47 consumer O+34 pointers satisfy
`stage47_definition == last_stage46_binding_definition + 0x5C * (stage47_variant - stage46_variant)`.
All action IDs match; variants range 0..37, no violations.
The consumer function 1E66E0 itself did not change the definition pointer inside its PRE/POST window.
Stage45 resolver predicts definition based on O+60 action ID, O+6C variant and 0x5C-byte row stride.
Original SHA-pinned AALib.exe is already in ChatGPT Library (size 175,556,608 bytes; SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba).

Disassembly of original game .text identifies exactly **three direct** CALL sites to binding native 1E8490, of which Stage44 previously intercepted only 1E9E2D:
- `1E8DA8: E8 E3 F6 FF FF` to 1E8490 from native 1E8D60. Guard `1E8DA1: 39 7E 6C 74 07 8B CE`: compare O+6C with old EDI, skip binder if unchanged, otherwise native ECX=ESI.
- `1E908F: E8 FC F3 FF FF` to 1E8490 from native 1E8DC0. Guard `1E9088: 39 56 6C 74 07 8B CE`: compare O+6C with previously captured value, skip binder if unchanged, otherwise native ECX=ESI. Surrounding code includes timing O+78 and adjustments to O+6C.
- Existing Stage44 `1E9E2D: E8 5E E6 FF FF` remains installed.
Binder native prologue at 1E8490: `55 8B EC 83 EC 0C 53 56 57 8B F1` (ECX=O, thiscall, no stack args).
Original SHA, all six native signature locations and all three CALL targets were verified against physical AALib.exe. These are **static** facts; neither new site has been observed in game runtime yet.

## Instrumentation (only two new patches; NO new native wrapper)

1. [Stage48_Overlay/src/Stage44ActionDefinition.inl](../Stage48_Overlay/src/Stage44ActionDefinition.inl) is the exact working Stage46 file except two new DWORD provenance fields, read-only caller return RVA in S44Enter from PUSHAD-saved original ESP, and one extra S48FlushRecord invocation per existing Stage44 ring record. The Stage44 naked assembly wrapper text and original native once-only call remain byte-for-text unchanged.
2. [Stage48_Overlay/src/Stage48VariantBinder.inl](../Stage48_Overlay/src/Stage48VariantBinder.inl) installs **two** validated native CALL redirects (1E8DA8 and 1E908F) to already working `S44BindingHook`. Strict guard and target matching, rel32 validation, patch installation mutex, suspend other process threads, two separate 4KiB page protections (sites span distinct pages), revalidate both before writing. Install only if Stage44/45/47 already installed; otherwise leave prior native sites untouched.
3. [Stage48_Overlay/src/Stage38CallbackWindow.inl](../Stage48_Overlay/src/Stage38CallbackWindow.inl) changes the working Stage47 adapter only to include Stage48VariantBinder.inl and invoke InstallStage48Hooks after successful Stage47 installation. The existing Stage38 worker and Stage47 flush are unchanged; S48 logs piggyback on existing Stage44 flush.

The new passive [ArcanaAI_Stage48VariantBindingTrace.log] will carry each S44-recognized binding with stage44Seq, callRVA, P1/P2, O, TID, QPC, activeID, binding-time variant PRE/POST, definition PRE/POST, nested resolver EAX and shared masks. All three caller sources 1E9E2D, 1E8DA8, 1E908F are distinguished by return RVA. There is **no new ring/worker**: Stage44 HEALTH counters govern shared ring drops, mismatch and overflow; Stage45 and Stage47 health remain separately monitored. Stage48 captured PRE O+6C at native entry *after* caller updated the variant; compare against previous Stage47 O+6C or earlier Stage44 binds to prove changes before the binder, not inside it.

[Verifier](tools/verify_stage48.py) checks original SHA/PE and exact guard/CALL bytes, native targets, unchanged wrapper assembly, and only scoped overlay edits. Original physical binary byte assertions PASS; source structural checks PASS. No x86 Windows compile, dynamic execution, FPU numerical equivalence of modified runtime, performance benchmark or animation/damage improvement claim has been performed.

## Next runtime validation

Require successful Windows x86 Build.log and:
- STAGE48_VARIANT_BIND_BUILD=2026-11-10-r1
- STAGE48_INSTALLED 2/2 callsites=1E8DA8,1E908F native=1E8490
- STAGE48_VARIANT_BIND_WINDOW entries for P1/P2; histogram callRVA and correlating Stage44/45/46 binding by stage44Seq and time.
- For 1E8DA8 and 1E908F verify callerRead=1, full Stage44 FF/FF and Stage46 1FFF/1FFF masks, resolverCount=1, nested Stage45 predicted EAX equal Stage44 def POST, no fallback. PRE/POST definition update should correspond to new variant and original resolver table.
- Join most recent earlier Stage47 consumer and subsequent Stage47 consumer on same P/O/ID: when variant changes, binder should resolve new 0x5C-stride definition row. A caller with zero P1/P2 events is not proof of a bug, and the extra previous binding origins remain a separate question.
- Require Stage44/45/47 health dropped/overflow/mismatch/badCaller=0; check log truncation and file sizes.
No modifications to gameplay fields or forced controls. No claim of visible animation, hitboxes, damage or improved AI from this observation alone.
