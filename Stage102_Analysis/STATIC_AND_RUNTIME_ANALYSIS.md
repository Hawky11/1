# Stage102 — F34D0 can produce 0x27; original native F7B0A rejects all observed 0x27 (2026-11-12)

**Existing Stage98 r2 Windows runtime evidence, verified offline; no new game run/DLL/hook/game-state or DR writes.** Full private original game AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` verified. The original binary remains private in the user ChatGPT Library /AALib.zip; it is NOT committed to GitHub. Stage36 OFF.

## Exact immutable log provenance

User same-run upload [8919f8e](https://github.com/Hawky11/1/commit/8919f8ec242f1137c447d2eb4afc7ba0ed0c0e1e); TID 11260, frame 0E58FC2C. Full Git blobs read (not empty GitHub contents previews):

| Log | Exact Git blob SHA |
|---|---|
| Stage98 route | `fdac35245f47c3b19bee346aa6b53e964ab14cf7` |
| Stage96 ESI writer | `1e98d712ac78549e46e4e2e0719fc8995d1706fd` |
| Stage95 bypass | `3dba2624412b23842b9bd0a2a9a33647e256954c` |
| Stage94 producer/gate | `fba1df695c4fcac0af357c782cf521af24b91b93` |
| Stage93 original FinalCommand QPC | `4fa6b5dd3747e9c2abed7ea4daa11d77a637453b` |

## Three positive F34D0 returns at a different native site

F79B1 original CALL F34D0 / F79B6 ADD ESP,14h + MOV ESI,EAX is a DIFFERENT function invocation from Stage101 class5 F7A8A/F7A8F. **3/1813** Stage96 F79B6 events are real preEsi=FFFFFFFF → resultEax=nativeNextEsi=00000027. All three retain 0x27 through original F3280 caller result at F79D4, original F4360 caller result at F79F0, Stage98 class3, Stage95 F7A94 BYPASS, and Stage94 F7B0A GATE:

| Side | Stage96 F79B6 → F79D4 → F79F0 | Stage98 class3 | Stage95 bypass | Stage94 gate | gate ECX/forward |
|---|---|---:|---:|---:|---|
| P1 | 975 → 976 → 977 | 806 | 608 | 209 | 0 / reject |
| P2 | 6076 → 6077 → 6078 | 4630 | 4085 | 612 | 0 / reject |
| P2 | 15102 → 15103 → 15104 | 11658 | 10356 | 1468 | 0 / reject |

All stages matched by exact actor/TID/frame and increasing QPC in the next same-context Stage98 window. All three reach F7B0A with ESI=0x27 but ECX=0, so **all three reject** at original CMP ECX,1 / JNE F7C68. **None** reaches Stage93 original type1 FinalCommand. The 0x27 output may originate from one of several internal F34D0/F30B0 branches; the existing logs do not distinguish which.

## Wider population — fifteen native 0x27 gate rejections

Of **201 Stage94 gates**, **15** had chosenEsi=00000027. Strict latest same-context QPC source join has **15/15 unique immediate matching sources**: **12 F3930 PRODUCER** (Stage98 class1) and **3 Stage95 BYPASS** (the F79B6 F34D0 substitutions above, Stage98 class3). All 15 have **ECX=0 and forwardType1=0**; zero accepted. Among **167 original Stage93 FinalCommand type1 records**, **zero** carry payload 0x27. The 12 producer values are *not* attributed to F34D0; these are two separate native origins converging at the gate.

## Why class5 returned inherited ESI — resolved and unresolved conditions

Stage101 remains correct: F7A8F class5 invoked F34D0 **461 times** in this run and returned **unchanged** (459 FFFFFFFF; two 0x29). Stage102 independently verified full F34D0/F30B0 code hashes and opcode guards. New static evidence:

- F34EB reads global byte VA 0E2E66D4. If zero, F34FB calls E6960 and F3503 calls F5210, an exact integer signed quotient of `[E6960-result-pointer+0x18]/17000`. F350F requires quotient >=1 to enter extended processing, otherwise early ESI passthrough at F3663. E6960 may resolve another actor, so the value is not necessarily local P+3990.
- F34D0 next tests slot flags actor+264 and actor+265, helper CBF50 and the native action byte table. **Both allowed action indices for class5, 0 and 0x22, map to table byte 0x27** at VA 04C7AE51 (stride 20). Thus the action table itself does NOT block class5.
- Conditions F3579/F35F9 can rewrite local input ESI to 0x27; F30B0's own special return F325A requires nonnull output pointers, `*output==FFFFFFFF`, actor+1A9 nonzero and many additional native predicate checks. F30B0 is not pure; output-pointer changes may occur. Exact failed predicates **cannot** be assigned for the 461 class5 cases because global byte, resolved quantized value, slot flags, CBF50 results and F30B0 intermediate state were not logged. Unchanged EAX does not prove an early exit.
- Class5 argument distribution from logged P+3B70 bit2: action0 **234**, action0x22 **227**, both static table entries 0x27.

## Verifier and limitations

Portable offline Python package `ArcanaAI_Stage102_F34D0_0x27_Native_Gate_Audit.zip` delivered in ChatGPT; SHA256 `d9645e9eebce860a69db8875ec0b648e032ce288e362850576063af712c42b62`. Contains a private-original-EXE SHA-guarded Python script, optional **five-log same-run** correlator, complete report/JSON/RU guide and **19/19** synthetic/negative tests. Original PE full-code hashes, exact opcode/relative CALL checks, F5210 quantizer and two native table entries **35/35 PASS**. The actual runtime counts above were derived independently from full GitHub blob contents; test fixtures are not misrepresented as live evidence. No executable or raw logs bundled.

**Next Stage103, if worthwhile:** static analysis of the native **ECX=0** reason before F7B0A for class1 producer and class3 bypass `0x27` events. Do not force ECX, bypass the native gate, make new gameplay writes, allocate DR, or duplicate working Stage94/96/95 hooks. Preserve tested Stage98 r2 DLL **2,051,584 B** and Stage97 rollback **2,045,440 B**. No demonstrated AI/hit improvement.
