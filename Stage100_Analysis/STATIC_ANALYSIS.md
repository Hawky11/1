# Stage100 — F34D0 / F30B0 native EAX→ESI return provenance (2026-11-12)

**Source status: STATIC VERIFIED only.** Original user Library `AALib.zip` containing `AALib.exe`, 175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Original executable remains private and is not committed to GitHub. PE32 x86 and `.text` mapping verified.

## Exact original call and return flow

- Stage99 established class5 control always reaches F7A94 BYPASS, with F34D0 called for mode !=2 or mode=2 and helper525CD0 return AL=0. Stage98 r2 mode distribution for 461 class5 events is **mode0 208 / mode1 253**. The original code therefore requires F34D0 on each such path, but the Stage98 log is **PRE-branch**, not a new post-call execution measurement.
- At F7A8A native `CALL F34D0`, five pushed args are player EDI, argument 0 or 0x22 (derived from `P+3B70 &4`), inherited ESI, `&[caller EBP-8]`, and `&[caller EBP-4]`. At F7A8F `ADD ESP,14h; MOV ESI,EAX` passes the actual return to ESI. **Stage96** already owns F7A8F, **Stage95** already owns F7A94. They provide matching TID/frame/QPC and preEsi/resultEax/nextEsi/chosenEsi fields.
- **F34D0 full region** F34D0..F366B (412 bytes, SHA256 `e5ce599737ed5b5b00750a4db10656f1d7d4e3dddd938075339076d97d7cba3e`). Its early return F3663 returns `[EBP+10]`, initially inherited ESI. Its guarded extended path can replace `[EBP+10]` with **0x27** at F3579 or F35F9; it calls F30B0 at F3654 and returns F30B0 EAX unmodified.
- **F30B0 full region** F30B0..F3273 (452 bytes, SHA256 `c888bf02c0f64a9af31044d857b8d05f9c1f6d68be2631eb6d3945099b403eb0`). It has a guarded special return F325A `EAX=0x27` (and zeroes two pointed-to outputs), or ordinary F326A `MOV EAX,[EBP+0C]` returning the argument passed from F34D0 (inherited ESI, possibly rewritten to 0x27).
- Thus the original **F34D0 return domain is `{incoming ESI, 0x27}`** on these code paths. This does **not** establish a particular live 0x27 occurrence count. If incoming ESI was already 0x27, return=0x27 is ambiguous.
- **Not side-effect-free:** F34D0/F30B0 call native helpers and can update caller output pointers and possibly player/game state. EAX provenance alone cannot prove AI action execution, hit, safety of forcing branches, or gameplay improvements.

## Verification, evidence limits and next checkpoint

The self-contained downloadable `ArcanaAI_Stage100_F34D0_ReturnProvenance_StaticAudit.zip` supplied in the Stage100 chat turn contains the original-EXE SHA/PE checker, **15/15 native region/opcode/call target checks**, 8/8 offline tests (including **120** synthesized class5 joins and **270** return-domain combinations), a strict four-log Stage98/96/95/94 QPC joiner, Russian guide and machine JSON. The scripts require no debugger, no new DLL and no gameplay memory writes. The ZIP SHA256 is `0377ee968afd78152a76490fd0b3c7c064a04039e4413a10b6ea98cf7bb889ca`. The original EXE and large raw logs are intentionally not bundled.

**Important limitation:** The latest Stage98 r2 aggregate results were read, but the 3–5MB Stage98/96/95 raw logs could not be retrieved in full text via this session's GitHub connector. **No new real same-session return-value join or count of ESI→0x27 substitutions is claimed.** An offline join utility has been prepared and tested on synthetic data only. Test it against the user's already-existing full homogeneous Stage98 r2 logs before publishing measured counts. Do not mix runs.

**NEXT Stage101**, if needed: run the strict offline Stage98→Stage96 F7A8F→Stage95 F7A94 joins, count changed-to-0x27 vs unchanged and analyze game-state/output effects separately. No redundant hooks F7A8F/F7A94, no forced commands, no DR writes; **Stage36 OFF**. Preserve Windows-tested Stage98 r2 DLL 2,051,584 bytes and Stage97 rollback DLL 2,045,440 bytes.
