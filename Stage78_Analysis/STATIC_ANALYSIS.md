# Stage78 — original site and ABI source audit

**2026-11-11. STATIC ORIGINAL VERIFIED, SOURCE READY, WINDOWS BUILD/RUNTIME UNTESTED.**

Private original AALib.exe was examined locally from saved AALib.zip, not uploaded. SHA-256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba; i386 PE32, preferred ImageBase 00400000.

Verified original native opcodes (RVA : bytes):

| RVA | Original bytes | Meaning |
|---|---|---|
| 155D27 | 6A21 | PUSH 21h event code |
| 155D29 | C786503F000001000000 | MOV [ESI+3F50],1 |
| 155D33 | 6A00 | PUSH 0 |
| 155D74 | C786543F000003000000 | state=3 |
| 155D7E | EBA7 | join to 155D27 |
| 1560B9 | E8B2FAFFFF | CALL 155B70 |
| 1566F0 | E87BF4FFFF | CALL 155B70 |

Stage77 runtime already observed callback 1560B0 with selector 0→1, state 1→3 and adjacent Stage76 event code21. That is strong causal evidence for this native path but the exact writer EIP is still not directly observed.

New source Stage78_Overlay/src/Stage78LiteralOneWriter.inl installs only a ten-byte JMP+NOP detour at 155D29. Installation is serialized with the existing patch mutex, suspends peers through S76SuspendPeers, validates peer EIP against all ten overwritten bytes, checks native bytes before patching, and uses VirtualProtect/FlushInstructionCache and rollback. No new DR registers or VEH and no new gameplay write besides replaying the original MOV once.

Before/after original MOV, x86 PUSHFD/PUSHAD/FXSAVE, FNINIT/CLD, then FXRSTOR/POPAD/POPFD bracket diagnostic C++ calls. The original PUSH 21h is left on the native stack; ESI continues to point to P; a final JMP resumes original 155D33. TLS depth16 and 4096-record ring buffer store exact P/O/TID, QPC, return address EBP+4, selector/state, callback and event data with explicit read masks and health.

Adapter Stage78_Overlay/src/Stage38CallbackWindow.inl is Stage77 adapter plus THREE integration edits only: include new observer, install after successful Stage76 install and before Stage59 worker starts, and flush from the existing Stage38 periodic observer. Original Stage38 native 156C00 call remains exactly once.

Static original code and source structure are verified by Stage78_Analysis/tools/verify_stage78.py, which accepts original AALib.exe or AALib.zip with the exact SHA above. This verifier does not compile x86 inline assembly or prove runtime safety. The user still must build with MSVC x86 and test ordinary COM-vs-COM.

Positive STAGE78_LITERAL_ONE selector 0→1 at native site 155D29 proves the literal-one store executed and changed selector for that runtime event. A 1→1 hit proves only execution. No hit does NOT exclude the other 10 register-derived MOV sites, zero-writers, indirect or alias writes. P1 Stage77 inter-frame 0→1 remains independent and unproven.

Rollback: Stage77 1,905,664-byte DLL, previous source; Stage76 1,901,568-byte DLL. No smarter AI, new attacks or damage claimed.