# Stage92 native branch and helper provenance

**Original AALib.exe SHA256:** 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba. Private original analyzed without publishing its bytes.

Original native update ACAB8 calls F5990(actor), which invokes Stage91 reader sites F5A4D (bit9), F5A5C (bit10) and F5A76 (bit11). Index9 AL1 branches at F5A57 to F5A68; index10 AL1 also reaches F5A68 (OR [ESI+3B70],8). Index11 AL1 leads at F5A86 to CALL 9ED20, and AL0 at F5A8F to CALL 9ED60.

At 9ED50 OR [EAX],EDX sets P+3B70 bit3; at 9ED92 AND [EAX],EDX clears bit3. Both helpers accept this=ECX and bit index3 on the stack; both return with RET4. A pre-existing bit3 may be unchanged even after AL1; before/after and actual branch selection are separate facts.

Stage92 adds three original five-byte CALL-site observers ACAB8, F5A86 and F5A8F. F5990 invoked once with a copy of the original cdecl arg; caller original argument remains on stack. Helper wrappers log state then tail-jump to native helper once to preserve RET4 ABI. Guard exact code bytes/relative targets, original PE fingerprint, peer EIP and rollback in a transaction. Does not install any EE306 hook or global F58E0 detour; no game-state writes, no DR writes.

**Static-only checks:** 67/67 exact PE32 and source checks PASS; 2 positive plus 3 negative synthetic correlator cases PASS. **MSVC x86 compilation, installation, in-game observations and final COM/AI behavior all NOT VERIFIED.** Stage91 DLL 1,999,872 bytes is the tested fallback.

Log correlation must match P1/P2 player+TID+QPC and verified source state: do not treat Stage91 AL1 as a queued command without independently observed same-event EventDispatch/FinalCommand evidence.
