# Stage54 r1 — Windows x86 BUILD VERIFIED; hook installation REFUSED (2026-11-10)

## Grounded evidence

- User uploads: [bd0716d](https://github.com/Hawky11/1/commit/bd0716d9c270c7f4ca19f8051f26b7e448bb5541), [47675c5](https://github.com/Hawky11/1/commit/47675c5a6262acf9ed470fc443192746a265b8b6).
- `Build.log` at 18:13:10: **BUILD SUCCEEDED** x86, `dinput8.dll` **1,113,088 bytes**, preexisting warning C4754 only.
- `ArcanaAI_Stage54PreParentCoordinateTrace.log` at 18:13:36: `STAGE54_PRE_PARENT_COORD_BUILD=2026-11-10-r1` followed immediately by `ERROR: Stage54 original sibling CALL/signature mismatch; no patches.`. No Stage54 windows or installation.
- Stage53 still installed: `STAGE53_INSTALLED 1/1 A2B90->AF0A0`; **3,918** Stage53 windows; 18 zero-error HEALTH, only two initial O+18 flag changes, zero X/Y within AF0A0. Stage47 3,848 windows, Stage52 7,832 rows; no errors from those monitors.
- This immutable branch includes Stage54 r1 source and exact untouched root log blobs from user commit 47675c5.

## Verified source root cause

Original AALib.exe size 175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, PE32 ImageBase `0x00400000`. At function A1B80 the original first instruction is `80 3D 5C 1B 0A 0E 00` (CMP BYTE PTR [0x0E0A1B5C],0). At **RVA A1B82** the 4-byte absolute operand has an **IMAGE_REL_BASED_HIGHLOW (type 3)** relocation in the PE base-relocation directory.

Stage54 r1 incorrectly compared the whole *unrelocated on-disk seven-byte prologue* to the already relocated in-memory code. In the user run `g_gameBase=0x00800000` is supported by previous absolute EIP-vs-RVA observations; expected relocated operand is `0x0E4A1B5C`, not `0x0E0A1B5C`. The runtime failure is consistent with this mismatch. Since r1 returns on the first failed test, the upload cannot individually confirm each remaining comparison; do not claim more than the identified deterministic relocation defect.

**Next:** Stage54 r2 preserve all original callsite/rel32 checks and additional helper signatures, but validate A1B80 using relocation-aware expected DWORD = original operand + (actual loaded gameBase - PE preferred ImageBase). Keep read-only wrappers. Windows x86 build/game runtime r2 still pending.
