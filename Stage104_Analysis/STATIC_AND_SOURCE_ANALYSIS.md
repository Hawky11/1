# Stage104 — passive native F7CC0 mode-0 and F7DA3 CEA20 source-ready audit (2026-11-12)

**SOURCE-READY, STATIC VERIFIED, NOT WINDOWS BUILD OR GAME TESTED.** Private original AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` checked. No new executable, live hooks, DR registers, game state writes or AI improvement shown. Stage36 OFF.

## Verified original route

Stage102/103 same-run Stage98 r2 logs: 201 F7B0A original gates, 167 type-1 and 34 other statuses, including **15** zero-status `0x27` codes (12 F3930 producer, 3 F79B6 bypass). Original Stage93 FinalCommand observer covers only **F7C5D → F2DE0(mode 1)**. Thus its 167 type-1 records cannot prove loss of the zero-status `0x27` events.

Original opcode/CFG audit confirms:
- **F7CC0** exact original bytes `E8 1B B1 FF FF`: CALL **F2DE0**, return **F7CC5**. Stack before this call: `PUSH ESI` (payload); `PUSH 0` (mode); `ADD EDI,0x3A2C`; `PUSH EDI` (record ptr). Critically **EDI=player+0x3A2C at the call**, so new observer recovers player by subtracting this offset. Native F2DE0's original logic stores mode and code in the record when its guard permits. It does **not** establish hit/animation/AI effect.
- **F7DA3** exact original bytes `E8 78 6C FD FF`: CALL **CEA20**, return **F7DA8**. Original preceding `F7D96` status-zero test and `F7D9A` ESI=0x27 guard lead there; `ECX=[EBP-30]` and `PUSH EAX` directly precede the call.
- The 2 original callee prologues are checked in the private original and at runtime install preflight. Existing F7C5D remains untouched. F7CC0 can also be reached with other payload codes: non-0x27 mode-0 records must not be wrongly counted as errors.

## Prepared source-only package

User downloadable archive **`ArcanaAI_Stage104_Mode0_CEA20_Passive_SourceOnly.zip`**, SHA256 `d55f025ee38c4b066c37a076f086654d63006a75f16940d9702367fe8ed51b7e` (distributed in ChatGPT rather than GitHub; does not include private original exe/logs). Includes:

- `src/Stage104Mode0NativePath.inl`: two native 5-byte CALL site redirects. Every naked x86 hook snapshots PRE using PUSHFD/PUSHAD/FXSAVE (restored), bounded 8192 ring + worker flush, then **tail-JMP** to unchanged original native F2DE0 or CEA20. This intentionally preserves original return address, stack args, register/flags, once-only original callee. No game/DR writes, no synthetic AI actions or mode changes.
- `src/Stage81SelectorWriter.inl`: full Stage98 r2-compatible adapter, conditional Stage104 installation AFTER successful Stage98, plus Stage104 Flush/Health in existing worker.
- `ApplyStage104.ps1` and `RollbackStage104.ps1`: exact SHA guard and source backup/rollback. Requires existing Stage81 adapter SHA `12f2afbee4cf427f2b13cce7e9baeb7d525fc4e4b23bc42cc1c3d0a86463a192` and Stage98 r2 observer SHA `a56fe5692b3284fae37822e6f08438e3a32a800accbad5ff4c72d36ea97a1591`. New module SHA `1fd486068bdc9b023623fdd662eded12231e36e5cf7cf320ec7d6e0b24048905`. Updated adapter SHA `3b0f6e22194b0c6385a7030ea6aa101b3f8ba2e80f32feec2b93b6cf1ccb5ef7`.
- Standard-library-only `tools/verify_stage104.py`, `tools/test_stage104.py`, `tools/correlate_stage104.py`; RU instructions and machine JSON.

## Offline verification and limits

Full original PE SHA and **39/39 native byte/relative-edge/prologue checks PASS**; **25/25 source structure/adapter checks PASS**; **25/25 offline synthetic/negative tests PASS** (including strict actor/TID/frame/QPC and ambiguous-event rejection). These are **not** a MSVC build, an ABI equivalence proof at runtime or a game test.

Installer checks original fingerprint, Stage94+98 installation, both original 5-byte callsites, four surrounding signatures and two original callee prologues, and uses process-wide mutex, peer-thread suspension and patch rollback. Mode0 records will show `STAGE104_MODE0`, `special27`, `valid`, qpc, player, saved args; the special follow-up is `STAGE104_CEA20`, and HEALTH tracks dropped/mismatch/readFail.

## Windows validation handoff / next Stage105

Back up Stage98 r2 **2,051,584-byte tested DLL** and Stage97 **2,045,440-byte** fallback; apply via PowerShell against exact Stage98 r2 local source, build MSVC x86, review Build.log, short natural COM-vs-COM, and require `STAGE104_INSTALLED 2/2` and clean HEALTH. Upload **same-run** Build.log, Stage104, Stage94, Stage93, Stage98 and ArcanaAI.log. Only then establish strict Gate→MODE0→CEA20 with actor+TID+frame+QPC and original code. If build fails/signatures differ/instability occurs, rollback to tested Stage98 r2. Stage36 stays OFF, do not force `ECX=1` or synthetic commands. No performance or gameplay improvement can currently be claimed.
