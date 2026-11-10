# Stage54 r2 runtime analysis

## Stage54 r2 runtime verified — 2026-11-10

Evidence: main upload commit a49661be904b7ef6427b8a885a852089f798801e (logs remain available at that immutable commit). Stage54 blob 413af4919f723351edfcafb7fb397514016ea479; Build.log blob 741cbe661dcba1e54d0ce46822838c19f1e28a25.

Build started 18:27:30 local log time, Windows x86 BUILD SUCCEEDED, DLL 1,113,600 bytes; existing C4754 warning only. Runtime r2 marker and STAGE54_INSTALLED 3/3 confirmed. ASLR check: load base 00800000, expected and actual relocated operand both 0E4A1B5C, readable=1.

Stage54: 19,734 unique contiguous rows (seq 1..19734), 6,578 per callsite A2B81/A2B86/A2B8B. All pre/post masks 1FFF and all changedMask=0000. 29 HEALTH records: missingObjects/dropped/overflow/mismatch/badCaller all zero. Thus no observed changes to any of the 13 watched DWORDs inside these three native calls in this run.

Stage47: 6,500 unique contiguous rows, full masks; 29 HEALTH with zero listed errors. Stage53: 6,572 unique contiguous rows, full masks; 29 HEALTH with zero listed errors. Only two initial Stage53 f7 O+18 changes 0→4; no X/Y change inside AF0A0.

Correlation method: first subsequent PRE after POST, same side/object/thread, QPC gap <=5,000 ticks; target sequence uniqueness checked.
- Stage47 POST → first Stage54 PRE: 6,500/6,500 distinct matches; gaps 252–1,726 ticks. Raw X differs in 504 (P1 266, P2 238); Y differs in zero. Stage47 POST X equals Stage54 PRE anchor O+AC in all 6,500.
- A1B80 POST → CFB80 PRE: 6,578 distinct matches, gaps 3–121; X/Y/AC/B0 identical.
- CFB80 POST → BAB60 PRE: 6,578 distinct matches, gaps 3–221; X/Y/AC/B0 identical.
- BAB60 POST → Stage53 PRE: 6,572 distinct matches, gaps 4–401; X/Y/AC/B0 identical.
- Exclude 78 first-sibling rows without a Stage47 partner and six terminal sibling rows without a Stage53 partner; do not join stale frames.

Conclusion: the observed X change occurs after the Stage47 snapshot and before A2B81→A1B80, not inside the three Stage54 siblings or Stage53 AF0A0. This does not identify an exact writer or establish visual/AI improvement.

Next: inspect original SHA-pinned AALib.exe control flow between Stage47 return sites AE711/AE943 and A2B81, identify the enclosing earlier routine/boundary, then prepare a focused passive Stage55 observer after verifying signatures, relocations and ABI. Stage55 source is NOT prepared in this update. Keep the working Stage54 r2 sources and DLL. No rebuild required for this analysis.
