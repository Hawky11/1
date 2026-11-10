# Stage41 — first direct native object action-request state (static, source-only)

Date: 2026-11-10. Source parent: Stage40 r2 relocation-fixed branch (Windows runtime proven in upload e4d29fa, archived unchanged at stage40-r2-runtime-evidence). Original game EXE SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba (175556608 bytes; verified after extraction from Library /AALib.zip). Original PE32 i386 preferred base 00400000. All addresses below RVAs.

## Original native disassembly and data (objdump, read-only)

- E8480 native per-player request consumer: E8489 reads player P+3F4C, E848F compares FFFFFFFF. For a valid request E8498 reads bit24 of P+3B7C (low bit in its highest byte), E84A4 reads ECX=[P+4], E84A7 pushes that bit as arg3, E84A8 pushes zero as arg2 and E84AA pushes request as arg1, E84AB E8 50 1A 10 00 invokes native 1E9F00; its callee RET 0Ch maintains ESP. Only afterward E84E3 calls 125920.
- 1E9F00: E9F03 cmp byte [ebp+10],0; E9F07 mov eax,[ebp+08] (ID); if bit24=0 and [ECX+60]==ID, E9F11 writes [ECX+68]=FFFFFFFF and E9F18 clears AL, returning 0 at E9F1B. Otherwise E9F1E writes byte [ECX+30]=0, E9F22 writes [ECX+64]=ID, E9F25 sets bit0 in [ECX+08], E9F34 loads AL=1 and E9F37 returns with RET 0Ch. This is a native command-state write *before* optional selector remapping, not animation/hit evidence.
- 125920: reads six-byte records from original static table starting VA 05CF7260 (RVA 058F7260), until type byte FE. Exact original table contains 40 records of type 8 (3), 9 (28), and 29 (9). Mode is normally P+3F20, or constant 22 under the guarded P+3978==29 special case. Stage40 r2 observed P1 mode 10 for 271 calls and P2 mode 23 for 218 calls. No table entries for 10/23, explaining the 489 native selector FFFFFFFF returns; this does not negate the earlier 1E9F00 effects.
- The optional E8505->1357F0 actor path is reached only for successful 125920 lookup; it is not the general action-command transport. In Stage40 all actorSeen=0, expected for unmapped modes.

## Runtime evidence and exact correlations

Fresh Windows x86 Stage40 r2 Build.log: genuine BUILD SUCCEEDED, 688128-byte DLL. STAGE40_INSTALLED 3/3. Full Stage40 log 489 continuous windows, P1=271/P2=218, all p0 before non-sentinel and after FFFFFFFF, all selector=FFFFFFFF, all masks FF and no explicit Stage40 errors. Stage39=57 output windows; 56 match first subsequent same-player Stage40 raw ID before any later Stage39 output on that side, all 56 to distinct Stage40 sequences, no mismatched ID. Stage39 final seq6020 / P2 ID0303 occurs after last Stage40 window and is unobserved, not a confirmed dropped command. Stage37 59/59 exact EventDispatch type1 matches; Stage36 406 candidate observations 11 nonsentinel. See the evidence report in Stage40_R2_Runtime on stage40-r2-runtime-evidence.

## Stage41 instrumentation

Instead of adding a fourth CALL hook at E84AB (risking another ABI perturbation), this stage reuses existing tested Stage40 AD72F->E8480 before/after snapshots. It captures exactly five DWORDs with independent masks for **the actual object pointer O=[P+4]**, each before native E8480 and after it returns:

- o0 O+08 — native command-pending bit0
- o1 O+30 — relevant low-byte command reset
- o2 O+60 — current/ongoing native action ID
- o3 O+64 — pending/raw action request ID from 1E9F00
- o4 O+68 — duplicate/current-state field set to FFFFFFFF by the short-circuit path.

Snapshot pointer is stored separately both times, never silently treats a different O as the same object; data valid only when objectPreMask/objectPostMask indicate a successful read. Adds Stage41State fields to the existing TLS/ring Stage40 record and worker-only Stage41_OBJECT_WINDOW with stage40Seq/P/TID/QPC identical to corresponding Stage40 consumer row. No extra game hooks, no new worker, no INI, no native game calls or writes. Source checks: three and only three MakeRelativeCall invocations preserved, native 125920/1357F0 cdecl argument copying unchanged, PE ASLR r2 signature logic unchanged.

Stage41 expected raw checks:
- If bit24Before=1 OR o2.pre != request, require o3.post=request on readable same-pointer windows, o0.post bit0 set. If object state and request were already equal, changedMask may be zero while still consistent.
- If bit24Before=0 AND o2.pre=request, native 1E9F00 direct branch writes o4.post=FFFFFFFF, not necessarily o3. Do not classify duplicates as missing work.
- If both P+4 pointers differ, do not compare derived fields. Do not infer visuals, attacks, hit, damage or AI improvement from any state write.

**Caveat:** only static source and native opcode analysis completed. Stage41 Windows x86 build, hook persistence and object runtime records are NOT yet verified. User must copy one file Stage41_Overlay/src/Stage40RequestConsumer.inl to local src, build x86, run naturally, and upload logs. Keep Stage29 AITrace.cpp and every historical branch untouched. No original EXE re-upload required.
