# Stage90 — locating bitmap clearing via existing native F57D0

**Status:** source-only, original EXE verified; not Windows-built or game-runtime-tested. The last real validated DLL is Stage89 (1,984,000 bytes). This file records *static evidence*, not a new runtime observation.

- AALib.exe SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`; PE32 `.text` RVA mapping verified, no raw offset confusion.
- Original existing Stage24 call `EE306 E8 C5 74 00 00 -> F57D0` (cdecl four arguments). Stage24 already invokes the Stage88 Pre/Post bridge around the original helper exactly **once**; Stage90 reuses it.
- F57D0 receives `EDI = P+3A08`; `ESI = argSelector` (the code 0x0E/0x0F/0x10 for Stage87 selector 0/1/2), and reads `table[ESI*3]` at RVA `0x049DF188`.
- `F57EC` tests bit0 of `table[ESI*3]`. If set, `F57F8 XOR EAX,EAX`, `F57FA MOV [EDI+4],EAX` **zeros P+3A0C**, and `F57FD MOV [EDI+8],EAX` zeros P+3A10. It then stores code with `F5800 MOV [EDI],ESI`.
- F57D0 can subsequently **OR a new bit** into the bitmap using `F5856` and `F58AC`; therefore a pre/post observer sees **net changed bits**, not internal intermediate zeroing. A bit that is cleared then re-set in the same call will not appear in `clearMask`.
- Original descriptor triplets for codes `0x0E`, `0x0F`, `0x10` all equal `01 26 FF`; bit0 set. F57D0 special handling of descriptor index `0x26` uses `P+3F50` to pick bit0x200/0x400/0x800. Stage89 reader `F58E0` tests those bits at `F58F2`, `F58FF` and `F5907`.
- There are two verified direct callsites to generic nine-DWORD zero initializer `F57A0`, including init `EAEB6` for `P+3A08`; this is a **separate possible clearing mechanism** and Stage90 does not claim exhaustive coverage of every writer.

**Stage90 design:** No patch at EE306, F57D0 or F58E0. Modify just the C++ instrumentation bridge from Stage88 to add `arcana_stage90::Pre(frame)` before the original Stage88 pre callback and `arcana_stage90::Post(saved)` after its post callback. Preserve Stage87 same-thread handoff and Stage89 reader hooks. Original Stage24 assembly/call/arguments remain unchanged. Record all native F57D0 P1/P2 calls with full three-DWORD bitmap pre/post, table descriptor, input code and dispatcher metadata, same-thread QPC, and special bit clear/set masks. Filter non-player calls only. Use bounded 8,192-record ring and worker-only disk logging. A more compact mode is available with `Stage90LogAllF57D0=0`, but full mode is required for exhaustive audit.

**Hypothesis to test:** Between Stage88 special-bit set and later Stage89 AAB55 AL=0, an unlogged ordinary F57D0 call clears or replaces the special bit. The Stage90 data will verify or refute it only for calls through EE306 and cannot rule out other writers. If no intervening F57D0 change is found, investigate other writers before asserting consumer behavior.
