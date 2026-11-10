# Stage59 — guarded dynamic P1/P2 X writer watchpoints (source ready)

**Status 2026-11-10:** Stage59 source and narrow Stage38 adapter are published. The original PE and Stage58 control-flow signatures were statically inspected; the adapter text diff was compared exactly. **Windows MSVC x86 compilation and in-game execution of Stage59 have NOT been tested.** Treat this as an experimental passive diagnostic.

## Why

In the fresh Stage58 capture, 5,538 same-run ACDF0 windows contain **1,179 net raw-X changes**, partitioned into **776 ACE11 POST→ACFD6 PRE**, **434 within native 15CEC0**, with **31 overlapping windows**. Stage58 observed 411 physical calls to B86A0: 380 EAX=0 calls lead to an ACF2A FSTP instruction, while 31 EAX=1 calls skip ACF2A. None of the 380 monitored ACF2A operations changed tracked P1/P2 raw X this run. These function-level snapshots do not identify any writer responsible for the remaining changes.

Stage59 uses hardware **write-only DWORD** watchpoints on the *runtime* dereferenced player object pointers:
- P1: \`[g_playerP1+4] + 0x7C\`;
- P2: \`[g_playerP2+4] + 0x7C\`.

The two addresses are derived at runtime and validated before arming. This is **not** a fixed relocated RVA watchpoint. Because all Stage55–58 snapshot \`f1\` fields refer to original object+\`0x7C\`, the raw bits can be compared exactly within the same capture.

## Safety and ownership gates

- Stage59 requires **Stage58 installed 3/3** before creating a worker. It reads \`ArcanaAI.ini\` and refuses to arm if either Stage16 option \`EnableControlFieldTrace\` or \`EnablePlayerBit17WriteTrace\` is nonzero. Stage16 already uses DR0/DR1 for those modes.
- DR0/DR1 are installed on each accessible process thread only if both address slots AND their enable/RW/LEN fields are free. \`DR7=0x00DD0005\` (two local, write-only, four-byte watchpoints) is combined with the *existing* DR7 bits outside the DR0/DR1 mask. DR2/DR3 are not modified; unrelated debugger owners are skipped.
- The vectored handler handles only DR0/DR1 hits whose context matches Stage59's exact addresses/config; leaves unrelated DR2/DR3 and single-step causes to their existing handlers. It logs post-write EIP/RVA, thread ID, QPC, and raw DWORD into a preallocated ring. No file I/O occurs inside the exception handler.
- The **oldShadow** is a best-effort prior trapped value, **not** proven to be the value immediately before the instruction. Missing threads and racing writers may cause gaps. \`newRaw\` is the first safely read post-write value; \`readOK\` reports reliability.
- On P1/P2 object pointer changes Stage59 stops capture and attempts to disarm only DR0/DR1 configurations it still owns. If another owner replaced them, its state is never overwritten; the VEH remains process-lifetime for safe handling of any remaining watchpoint hits. Stage59 does not automatically rebind to a new object.
- Logs include \`STAGE59_THREAD_INSTALLED\`, \`STAGE59_WATCH_ARMED\`, \`STAGE59_X_WRITE\`, \`STAGE59_HEALTH\`. Nonzero \`dropped\`, \`readFail\`, \`ownershipConflicts\`, \`installFailures\`, \`teardownFailures\` or an absent thread-installed marker means incomplete/uncertain coverage.
- Hardware data breakpoints can slow gameplay or conflict with third-party debuggers. This is not a proof of zero runtime behavioral impact. Stop and roll back to the verified Stage58 DLL on a crash, major slowdown or installation mismatch.

## Original-binary and adapter preflight

The private original \`AALib.exe\` is **175,556,608 bytes**, SHA-256:

\`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba\`

Verified original instructions:
- ACF0F \`E8 8C B7 00 00\`: call B86A0.
- ACF17 \`84 C0\`; ACF19 \`75 12\`: TEST AL,AL / JNE ACF2D (skips the ACF2A writer if EAX nonzero).
- ACF2A and ACFA3 \`D9 58 7C\`: FSTP DWORD [EAX+0x7C].

\`Stage59_Analysis/tools/verify_stage59.py\` checks the original fingerprint, PE32/x86, those signatures, DR0/DR1 bit-field decoding, exact two-edit Stage38 adapter delta and required safety tokens. Run it locally; static checks **cannot** prove Windows compilation, debug-register safety, capture completeness or AI improvements.

## Files and limited installation

- Replace only \`D:\INCUBATOR\ArcanaAIOverhaul\src\Stage38CallbackWindow.inl\` with \`Stage59_Overlay/src/Stage38CallbackWindow.inl\`.
- Add \`D:\INCUBATOR\ArcanaAIOverhaul\src\Stage59XWriterWatch.inl\`.
- Keep Stage58ConditionalCalls, Stage57HelperBoundary, Stage56Writer, Stage55EarlierMotion, all older sources/INI/Build.bat unchanged **except the INI mode switch described in the installation guide**.

The adapter includes Stage59 just after Stage58, starts the worker only when Stage58 installs successfully, and preserves all existing FlushStage38 logging. It does not add new native CALL patches.

## Required test and interpretation

1. Back up the last verified Stage58 DLL (1,145,344 bytes); do not replace game DLL if compilation fails.
2. Disable the **two** Stage16 hardware breakpoint modes in \`[AIResearch]\` and enable Stage59. No runtime watchpoints will be installed if a Stage16 mode is still enabled.
3. Run Windows x86 \`Build.bat\`; require BUILD SUCCEEDED. After deploying, run a short natural COM-vs-COM test and let logs flush.
4. Upload **only** \`Build.log\`, Stage59 X Writer Watch log, and the **same-run** Stage55/56/57/58 logs. If Stage59 refuses to arm or crashes, send Build.log + Stage59 log instead.
5. Inspect thread coverage and dropped/error counters. Join each \`STAGE59_X_WRITE.qpcAfter\` to Stage55 ACDF0 and Stage57 early/tail and Stage58 observations by side/object address/thread/QPC; note other-thread writes separately. Decode **RVA_after** to the *prior complete instruction* in the private original PE (post-write EIP is not necessarily instruction start). Cross-check tracked value equality and duplicates before attributing any write.

**Neither Stage58 nor this Stage59 source demonstrates improved fighting AI, new combos, collision changes, damage or animation effects.**
