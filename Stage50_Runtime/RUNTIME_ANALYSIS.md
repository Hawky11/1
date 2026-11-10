# Stage50 runtime — AE100 observer installed, no P1/P2 coordinate writes

## Original user artifacts (frozen)

2026-11-10 Windows x86 Build.log started 15:55:39; [user runtime upload e21cf58](https://github.com/Hawky11/1/commit/e21cf58ff4db752a5b0ef5dfbab88d6ed5def912), [Build.log upload 2ddee8a](https://github.com/Hawky11/1/commit/2ddee8a8b025ede1c799edf1ee11fde9d0ff7056). This evidence branch is based directly on **2ddee8a**; original full log blobs are unchanged.

**Windows x86 BUILD SUCCEEDED**, `build/dinput8.dll` **1,081,344 bytes**, prior non-fatal C4754 warning only. Original loaded DLL wrote `STAGE50_PRELINKED_COORD_BUILD=2026-11-10-r1` and `STAGE50_INSTALLED 1/1 callsite=AF136 native=AE100; ESI object; native once`. Stage49 r2 `STAGE49_INSTALLED 2/2` remained present. Stage50 build AND installed runtime are therefore **VERIFIED**; this does *not* mean AE100 copied or changed coordinates.

Exact Git blob `ArcanaAI_Stage50PreLinkedCoordinateTrace.log` SHA `d3ef8bf4e2248817bc6c4666f61cc380fdccee01`: 2,759,720 chars; **5,792** P1/P2 windows (2,896 each), seq1..5792 contiguous, all caller AF136, mask `07FF` for both PRE/POST. All `changedMask=0000` and `gateCandidate=0` in this sample. **O+3C=0x00000000 and O+1A4=0xFFFFFFFF in 5,792/5,792**, explaining why the guarded native AE100 coordinate store was not entered for either P1/P2 object. Stage50 has **no observed changes** in O+7C, O+80, O+AC or O+B0; do not equate this with global inactivity.

28 Stage50 `STAGE50_HEALTH` samples: calls **49,214**, otherObjects **43,422**, tracked 5,792; dropped/overflow/mismatch/badCaller **all zero**. No Stage50 ERROR/WARNING.

Stage49 new exact Git blob SHA `9aea63cf9161a2932e2a50210d3c4f25efa03f41`: **5,790** P1/P2 windows (2,895 each), all AF13D, all `changedMask=0000` and `staticGateCandidate=0`; linked pointer nonzero in 5,405; 28 HEALTH samples calls 49,198, otherObjects 43,408, dropped/overflow/mismatch/badCaller all 0. Stage47 new Git blob SHA `778051f07b1165d7da4caf24fd5a4cb632a07721`: **5,668** P1/P2 windows (2,834 each), 3,489 with nonzero changedMask, 28 HEALTH samples calls 48,034, otherObjects 42,366, lost/corrupt all 0. Stage48 1,553 P1/P2 binds; Stage44/46 likewise 1,553; Stage38 5,331 and Stage35 11,356 P1/P2 windows, all contiguous.

## Exact same-event pipeline proof

[Complete 5,790-row join CSV](stage47_stage50_stage49_join.csv) matches completed Stage47 consumer → Stage50 native AE100 → Stage49 native 1E9170 by (side, object, QPC). Every Stage49 window matches its immediately preceding Stage50 sequence **5,790/5,790** (Stage50 has two extra final samples due capture boundary). Stage50 POST exactly equals Stage49 PRE for raw **O+7C, O+80, O+AC, O+B0** on **all 5,790**, with original-call timing order. So the observed coordinates do not change within AE100 or within the very next linked-motion 1E9170.

Before the Stage50 entry, however, last Stage47 POST O+7C differs from Stage50 PRE O+7C in **1,257/5,792** windows, while O+80 remains equal in all 5,792 and Stage47 POST O+7C equals Stage50 PRE O+AC in **5,792/5,792**. Thus 1,257 events exhibit an intervening **O+7C-only** change, with **O+AC preserved**; no new Stage46 binder was observed between the paired Stage47 and Stage50 QPCs. This is proof of an additional numerical write somewhere in this gap, **not yet proof of its actual writer** nor a visible game movement.

## Static next step: a broader original per-frame update preceding Stage50

Original SHA-pinned Library AALib.exe is PE32 175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Same caller function at `AF0A0` runs several native steps **before** the per-object `AF136` loop:
- `AF10D E8 FE FD FF FF` -> native **AED10?** (verify by rel32: AF112 + (-0x202) = AEF10, i.e. function AEF10)
- `AF112 E8 79 FC FF FF` -> native **AED90**
- `AF117 PUSH 1`, **`AF119 E8 02 F1 FF FF`** -> native **AE220** (single DWORD argument, caller cleanup at AF124). AE220 contains broader player/object update calls and may change object state indirectly.
- `AF11E MOV ESI,[E2E02C8]`, then per-object AF136 -> AE100 (Stage50), AF13D -> 1E9170 (Stage49).

Stage51 will first **bracket AF119 → AE220 read-only** (snapshot both player objects pre/post) to decide whether this native per-frame step accounts for those 1,257 X-only differences. No engine state patches, no movement/AI-quality claims. New Windows x86 build/runtime is pending.
