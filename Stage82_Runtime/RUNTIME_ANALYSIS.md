# Stage82 r1 same-run runtime — signature mismatch, fail-closed (2026-11-11)

**Evidence:** user-uploaded root `Build.log`, `ArcanaAI_Stage79SelectorFamilyTrace.log`, `ArcanaAI_Stage80F7150LateTrace.log`, `ArcanaAI_Stage81SelectorWriterTrace.log`, `ArcanaAI_Stage82NestedSelectorTrace.log` frozen at [commit de188f0](https://github.com/Hawky11/1/commit/de188f061689f5ed00e594413adec786a6735335).

**Build:** Windows MSVC x86 BUILD SUCCEEDED, DLL **1,942,528 bytes**, one warning C4754 (existing warning class). Build is not proof of Stage82 hook installation.

## Same-run evidence (16:22:09–16:22:53 local)

- Stage79 installed 1/1; **6,084** native EFFE8→E89E0 windows; full masks.
- Stage80 installed 1/1; **5,652** E8A8E→F7150 calls; full masks.
- Exactly **5,652/5,652** Stage80 calls have **one** matching containing Stage79 interval by same P/O/TID and QPC. Zero missing and zero ambiguous windows.
- **Five** matched selector (P+3F50) transitions, with same before/after values in both stages:
  - P2 0→1: Stage79 seq344 / Stage80 seq96;
  - P1 0→1: Stage79 seq1773 / Stage80 seq1493;
  - P2 1→2: Stage79 seq3682 / Stage80 seq3333;
  - P2 2→1: Stage79 seq4360 / Stage80 seq3984;
  - P2 0→1: Stage79 seq5932 / Stage80 seq5507.
- Across 36 Stage79 and 36 Stage80 HEALTH rows, **no nonzero** dropped, overflow, mismatch, readFail, or filtered counters.
- Stage81 r2 prerequisites and installation passed (3/3 exact MOV hooks). Two `STAGE81_NATIVE_WRITE` events: F7D38 at 16:22:40, F7D22 at 16:22:44, **both changed=0**. F7D43 had zero hits.
- Stage82 r1 emitted **`STAGE82_ERROR call signature; no patch`**, no `STAGE82_INSTALLED`, no nested-call evidence. Its 131-byte log does not identify which of the two sites differed.

## Verified original bytes and next discriminator

The original user-provided AALib.exe SHA256 is `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
The two Stage82 expected native E8 signatures were rechecked against its PE32 .text section:

| Native RVA | Original bytes | Direct target |
| --- | --- | --- |
| 0F79E4 | E8 77 C9 FF FF | 0F4360 |
| 0F7AD1 | E8 5A BE FF FF | 0F3930 |

Both **match the original PE**. Thus the r1 failure cannot be explained by an incorrect expected signature *for this original file*. The actual in-memory bytes, failure site, and underlying cause remain **unknown** (possible runtime patch/detour, image difference, or instrumentation/integration issue).

**Stage82 r2 SOURCE-ONLY diagnostic patch** is prepared separately: it changes only `Stage82NestedSelector.inl`, logs `STAGE82_SITE_AUDIT` for *both* sites (live and expected bytes, decoded E8/E9 target/owner, page protection/type), and still refuses installation on any mismatch. Its signature guard, mutex, thread suspension/recheck, rollback, and no-gameplay-edit design are preserved. It has **not** been compiled or runtime tested in Windows.

## Next run

Back up the verified Stage81 r2 1,934,336-byte DLL and current Stage82 r1 1,942,528-byte DLL. Apply only Stage82 r2 diagnostic source over exact r1 source, Build.bat, require BUILD SUCCEEDED, run one short COM-vs-COM, upload Build.log + Stage82 log (and same-run Stage79–81 if Stage82 installs). Do **not** force unknown live CALL bytes to match the original or claim the exact selector writer EIP without observed evidence. On errors, revert.

**No AI behavior improvement demonstrated.**
