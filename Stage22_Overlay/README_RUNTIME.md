# Stage22 runtime evidence — 2026-11-08

This branch preserves the Stage22 source overlay and the user's 23:30:54 build transcript alongside the runtime logs uploaded to `main`.

Read `Stage22_Overlay/STAGE22_RUNTIME_ANALYSIS.md` for the complete QPC correlation and limitations.

**Result:** the Stage22 observer installed at RVA `0xEE451` and recorded three payload-`0x1E` post-AND states. All three were `bit400 0 -> 0`; no non-idempotent flag clear was observed. The run does, however, produce an 11/11 same-event correlation between Stage18 gate, Stage20 object guard and Stage17 setter calls. Stage21 r2's pre-call EDI values were found to be controller-array pointers, not action codes.

Build succeeded: Win32 x86, MSVC 14.51.36231, all eight C++ units compiled, DLL size **225,792 bytes**, with one non-fatal C4754 warning in `AICommandSiteTrace.cpp:785`.

The source/runtime snapshots and uploaded logs are historical evidence. Do not edit them; make the next research change on a new stage branch.
