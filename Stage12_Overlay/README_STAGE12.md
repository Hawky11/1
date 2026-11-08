# Stage 12 diagnostic overlay

Base source: ArcanaAIOverhaul_Stage11_Source.zip from the parent Stage 11 commit.

Apply this overlay to a fresh extraction of that archive:
1. Copy src/AIQueueTrace.cpp to the extracted src folder, replacing the original.
2. Copy Build.bat, ArcanaAIOverhaul.vcxproj and ArcanaAI.ini to the extracted root, replacing the originals.
3. Run Build.bat on Windows with the MSVC x86 toolchain and retain Build.log.
4. Back up the current working DLL/INI; install the new DLL only after a successful build.
5. Rename old logs, run one or two short COM vs COM rounds, then close the game normally.

New diagnostics:
- EVENT_DISPATCH_DETAIL: one type-1 record per invocation, including QPC/tick before/after the original function and queue snapshots.
- QUEUE_EVENT_DETAIL: explicit EBP/ESP, saved-frame chain and first-frame-vs-caller check, plus the existing RTL stack.

This is diagnostic-only. The original dispatcher is still called exactly once. AI decisions, helper arguments and command payloads are not modified.

The source has not been compiled or runtime-tested here. Do not replace a known-working DLL until the Windows build succeeds.
