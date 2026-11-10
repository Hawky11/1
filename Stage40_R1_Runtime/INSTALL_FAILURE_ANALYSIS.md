# Stage40 r1 Windows x86 install failed closed — original uploaded runtime evidence (2026-11-10)

Source branch: stage40-action-request-consumer-trace at 43821a5. Original user upload in main: commit 4faf4e0774a9b8b9a8e2990048b53b2120a4328a (05:37:30 UTC); this branch starts at that exact commit so the Build.log and all 27 runtime log blobs are preserved unchanged. Earlier Stage39 evidence ae6074d and all Stage29-39 historical branches remain untouched.

## Build and Stage40

Build.log: Windows MSVC x86, started 2026-11-10 08:34:12 local; BUILD SUCCEEDED, DLL 687104 bytes. No compile/link errors; one old C4754 warning from AICommandSiteTrace.cpp. The new ArcanaAI_Stage40ActionConsumerTrace.log is exactly one newline-terminated line:

    [2026-11-10 08:36:20] ERROR: Stage40 signatures/call targets mismatch; no patches.

No STAGE40_INSTALLED marker and no STAGE40_CONSUMER_WINDOW record. This is the expected fail-closed behavior; it means no Stage40 hooks were installed. Therefore this run gives NO runtime evidence for E8480 consumption, actor+280 transfer or action execution.

## Existing observer health (available checked streams)

Stage39ActionOutputTrace: 65 STAGE39_OUTPUT_WINDOW rows and its BUILD/ENABLED headers; no explicit warning/error in that file. Stage37Type1CommitTrace: 69 STAGE37_TYPE1_EFFECT rows with BUILD/CONFIG/ENABLED headers; no explicit warning/error. Stage36ControlToCommandTrace: 900 sequential CANDIDATE results, 883 nativeEAX=FFFFFFFF and 17 non-sentinel; all 17 show the native output flag set to 1. Stage36 r2 3/3 installation markers present, no explicit warning/error in that file. EventDispatchTrace has 215 EVENT_DISPATCH_DETAIL and QueueTrace has 68 QUEUE_EVENT_DETAIL rows. Stage38CallbackWindowTrace and Stage35ControllerConsumerTrace are large; the current GitHub text retrieval returned no contents for them, so their entire integrity and warning counters were not independently checked here. The exact uploaded original blobs are preserved through the branch HEAD.

## Root cause confirmed with original EXE, not guessed from output

SHA-pinned AALib.exe in ChatGPT Library AALib.zip: size 175556608 bytes, SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba, PE32 i386, preferred ImageBase 00400000. Exact bytes for all three Stage40 CALL sites and the three target entry instructions match the original PE. The original source validation incorrectly compared all nine native bytes in 125920 prologue against the in-memory executable. At RVA 125925, the 32-bit absolute operand is 05CF7260 and there is an IMAGE_REL_BASED_HIGHLOW relocation (type 3) at exactly RVA 125925. The observed running AALib.exe base is 00800000, hence Windows relocates this DWORD to 060F7260. That valid difference causes Stage40 r1 memcmp of the 125920 prologue to fail, and no hooks get installed.

## Corrective stage

Stage40 r2 source branch: stage40-action-consumer-r2-relocation-fix. Only Stage40_Overlay/src/Stage40RequestConsumer.inl needs replacing in the local project. It now checks the first five prologue opcode bytes and the precise DWORD relocated by loadedBase minus preferredBase; preserves checks of all three CALL sites, their targets, and other two prologues. Adds explicit per-site signature error reporting to prevent another ambiguous failure. An updated original-PE verifier tests the HIGHLOW entry and expected relocated values at several bases.

Static PE/call/relocation tests PASS; Stage40 r2 has not been compiled or run on Windows yet. Avoid rerunning r1 or making any gameplay/AI outcome claims. No request for AALib.exe re-upload.
