# Stage56 static analysis — ACDF0 two FADD/FSTP writer observers

**Status: SOURCE READY, Windows MSVC build/link and runtime PENDING (2026-11-10).**

## Source provenance and verified binary

- Stage55 tested source parent: \`00614967dbc386822d75b4bb37954d4f6600a439\`.
- Stage55 runtime evidence: \`c27b6b516edcea4551de9e52e6a151acdc115279\`; 6,576 complete Stage47→Stage54 chains, 480 X-only differences all within ACDF0 as a *whole-function boundary*.
- Original private \`AALib.exe\`: 175,556,608 bytes, SHA256 \`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba\`. Independently re-extracted from the existing private AALib.zip, hash verified. PE32 x86 ImageBase 00400000.
- **The EXE is not, and must not be, included in the GitHub repository.**

## Original native bytes / control flow

GNU objdump's original x86 disassembly at ACDF0, plus raw on-disk .text bytes:

| Source instruction RVAs | Original instruction bytes | Effect |
|---|---|---|
| ACF23 | \`D9 44 8D A4\` | \`fld dword ptr [ebp+ecx*4-5Ch]\` |
| **ACF27** | **\`D8 40 7C\`** | \`fadd dword ptr [eax+7Ch]\` |
| **ACF2A** | **\`D9 58 7C\`** | \`fstp dword ptr [eax+7Ch]\` |
| ACF2D | \`8B 97 70 3B 00 00\` | continuation: \`mov edx,[edi+3B70h]\` |
| ACF99 | \`D9 84 8D 54 FF FF FF\` | \`fld dword ptr [ebp+ecx*4-ACh]\` |
| **ACFA0** | **\`D8 40 7C\`** | \`fadd dword ptr [eax+7Ch]\` |
| **ACFA3** | **\`D9 58 7C\`** | \`fstp dword ptr [eax+7Ch]\` |
| ACFA6 | \`8B 5D F4\` | continuation: \`mov ebx,[ebp-0Ch]\` |

Both candidate stores are **three bytes**, too short individually for a five-byte direct near detour. Each immediately preceding FADD plus following FSTP form a **six-byte instruction-aligned pair** with no branch entry into the overwritten interior in the inspected ACDF0 function. The original jump targets can bypass these pairs by landing at ACF2D or ACFA6.

The patch targets **ACF27 and ACFA0**. Each becomes \`JMP rel32\` + one NOP; a naked detour observes the original EAX target and raw [EAX+7C], executes the original FADD and FSTP once in the original order, records the resulting raw DWORD and jumps to **ACF2D / ACFA6** respectively. The original stack contains no extra detour CALL return address at the native pair. This instruments the **pair**, so its logged before/after X brackets the store but also the preceding FADD.

Full 16-byte signatures include each original preceding FLD, the six-byte pair and the next instruction. The original PE relocation directory was parsed: **zero HIGHLOW overlaps** in either signature, and the instruction bytes match the on-disk file. For this pair no ASLR fixup is needed, unlike the earlier Stage54 r1 prologue issue.

## Narrow source change

- **New** \`Stage56_Overlay/src/Stage56ACDF0Writer.inl\` defines two passive JMP detours, a thread-local depth-8 row pending stack, ring-8192 capture, log and health counters, signature checked two-site installer.
- **Replacement** \`Stage56_Overlay/src/Stage38CallbackWindow.inl\`: compared with Stage55 adapter, *only* adds the new include, invokes Stage56 after successful Stage55 installation and flushes Stage56 at the existing flush point.
- All earlier Stage47–55 sources, INI, Build.bat and game files stay unchanged. Stage55's native ACDF0 call still occurs once.

Each writer event records site/store RVA, side, player, **actual EAX target object**, thread ID, nested before/after QPC, raw X before/after and readable flags. Untracked objects contribute to \`otherObjects\`, not fake P1/P2 rows. Counters: calls, rows, otherObjects, dropped, overflow, mismatch, readFail. Every sample matches actual g_playerP1/g_playerP2 object pointer at the time of entry.

The detours preserve CPU GPRs/EFLAGS and FXSAVE/FXRSTOR state around their C++ observers; x87/SSE arithmetic is not intentionally changed. However, as in prior stages, FPU exception and instruction/data-pointer metadata behavior, timing/reentrancy and real-game stack unwinding are not established by these static checks. In particular, an exception *at the relocated x87 instruction* has a different code address. This is diagnostic code, not a byte-for-byte transparent CPU emulator or gameplay change.

## Validation performed and pending

Checks against private SHA-pinned original performed: actual file SHA/size, both 16-byte complete-instruction signatures, both 6-byte patch spans, original objdump instructions, and relocation overlap test. Original Stage55 adapter modification is intentionally confined to three insertions. Run the included stdlib verifier:

\`\`\`powershell
py -3 Stage56_Analysis\tools\verify_stage56.py --aalib "D:\path\to\AALib.exe"
\`\`\`

**Not yet validated:** local execution of the published verifier against a full Stage56 checkout, MSVC x86 compile/link, original-pair assembly differential tests, Windows installation, Windows runtime, and same-frame correlation. The verifier checks source tokens and adapter text; it is **not** an assembly semantic equivalence proof. Do not claim Stage56 installed or any new game behavior before actual logs.

## Runtime proof criteria / next analyst

Require: Build.log BUILD SUCCEEDED, Stage56 \`STAGE56_INSTALLED 2/2\` and retained Stage55 \`3/3\`, both signatures verified, no hook errors, no dropped/overflow/mismatch/readFail. Collect \`ArcanaAI_Stage56ACDF0WriterTrace.log\`, Stage55 and Stage47/54 **from the same run**.

For every Stage55 \`A11A0→ACDF0\` window, nest all Stage56 rows with **same object + same thread** whose QPC before/after both lie inside the ACDF0 window. Compare ordered Stage56 raw X transitions to Stage55 PRE/POST X, explicitly distinguish no writer events, one event, multiple events, unchanged raw writes and non-P1/P2 target events. Count matched and unmatched changed windows; do not assume a 480 count in a fresh capture (it was the previous run). If two native pairs do not explain all new Stage55 changes, inspect helper stores and unobserved paths.

Keep last-known-working Stage55 DLL available for rollback if Stage56 build, signature, or runtime checks fail.
