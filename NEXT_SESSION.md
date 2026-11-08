# Next session handoff

**Read this file first when opening the repository in a new chat.** The user intends to provide only the repository link, so this file must contain the current working state and the next concrete task.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- Latest completed evidence branch: `stage15-postguard-flow-diagnostics`
- Stage15 HEAD at the last review: `4d1e2a049c835a991647cb04565d31d330aeac95`
- Stage15 overlay README: [Stage15_Overlay/README_STAGE15.md](https://github.com/Hawky11/1/blob/stage15-postguard-flow-diagnostics/Stage15_Overlay/README_STAGE15.md)
- Full chronology: [PROJECT_HISTORY.md](PROJECT_HISTORY.md)

Always re-check the remote branch HEAD before beginning. If it has moved, inspect the new commit and update this handoff rather than relying blindly on the SHA above.

## What the investigation established

The event of interest is type `1`, payload `0x1E`, produced in the parent routine at `AALib.exe+0xF69F0`.

Stage15 instrumented five branch sites. In the latest reviewed run:

1. Four invocations returned `0x11E` from F2F50.
2. F2ED0 returned `AL=0` in all four.
3. `EDI=0x1E` passed in all four.
4. Three events had bit17=1 in `[player+0x3B84]` and did not reach F6AD7.
5. One event (P2 at 12:16:45.020) had bit17=0, passed through F6A9D and reached F6AD7.
6. The P2 event was accepted by QueueTrace and subsequently observed by EventDispatchTrace with the same `(type=1,payload=0x1E)` pair.

This makes bit17 the current primary lead. It does **not** yet tell us which routine writes the field, nor prove that the four-event pattern is universal.

Stage15 installation was reported as 5/5 probes; 13 Stage15 records had `fieldsMask=0x0FFF` and `localRead=(1,1)`. The uploaded Stage15 set did not include `Build.log`, so do not claim the compiler output was reviewed.

## Next task: Stage16 — trace writers of player bit17

Create a new branch, preferably named:

`stage16-player-bit17-writer-trace`

Base it on the exact latest Stage15 branch HEAD (re-check first). Do not modify the Stage12, Stage13, Stage14, or Stage15 branches.

### Research steps

1. Inspect the Stage15 overlay and the actual source archive used to build the DLL. Preserve the staged overlay application order documented in `Stage15_Overlay/README_STAGE15.md`: clean Stage11 source base, then Stage12, Stage13, Stage14, Stage15. Add the new Stage16 overlay last.
2. Disassemble the relevant code and find all credible writes to `[player+0x3B84]` (including partial-width writes, bit operations, and writes through computed pointers). Do not assume the field is written only as a full DWORD.
3. Determine which write sites can be safely instrumented. Validate original instruction bytes, effective destination, and the semantics of each instruction before patching.
4. Implement passive Stage16 probes around validated writer sites. Capture old/new value and bit17 transition, player pointer, writer RVA, caller/return RVA, thread/QPC/time, and useful registers/call context. If a reliable broad write observer is not feasible, use validated per-site hooks rather than a speculative global patch.
5. Preserve the original write exactly once and do not force the flag, alter AI decisions, event type/payload, or queue contents.
6. Keep Stage12–Stage15 tracing enabled for correlation, especially Stage15 branch records, alternate callsite, QueueTrace, and EventDispatchTrace.
7. Add `Stage16_Overlay/README_STAGE16.md` with exact hook sites, byte checks, install behavior, overlay/build steps, required uploads, and limitations.
8. Build using the project's MSVC x86 toolchain. Retain the complete `Build.log`. Back up the known-working DLL and INI; do not install a failed build.
9. Run a short COM-vs-COM test, close the game normally, and upload the fresh logs to the Stage16 branch.

### Required files after the Stage16 run

- `Build.log`
- `ArcanaAI_ProducerCallTrace.log` (Stage14/15 and any Stage16 records configured there)
- `ArcanaAI_AlternateCallTrace.log`
- `ArcanaAI_QueueTrace.log`
- `ArcanaAI_EventDispatchTrace.log`
- `ArcanaAI_FinalCommandTrace.log`

Keep ParentFlow and ProducerContext logs when available; they may be useful to connect flag writes to parent/helper activity.

## How to review the next upload

- Check the Stage16 branch HEAD and list changed/new files first.
- A source archive, overlay, README, or DLL by itself is not a new runtime run. Look for fresh runtime logs, and especially `Build.log`.
- Compare the new run against Stage15 and earlier traces. Do not just summarize the new log in isolation.
- Confirm exact hook-install counts and any rejected byte fingerprints.
- Correlate writer events to Stage15 using QPC/timestamps, player pointer, thread ID, and P1/P2 context.
- Build a timeline for every observed bit17 transition and determine whether it precedes the relevant F6A68 branch.
- Distinguish facts from hypotheses. In particular, do not say the bit17 writer is identified until the Stage16 evidence identifies an actual write site and shows its old/new value.
- Report missing logs explicitly; never imply that a missing Build.log or runtime trace was checked.
- After completing the analysis, append a dated Stage16 entry to `PROJECT_HISTORY.md`, update this file with the new checkpoint and next task, and add the new links/commit SHA to the root README if appropriate.

## Working rules for future chats

- The user expects direct repository inspection and log analysis, not a request to re-upload files that are already present in the linked branch.
- Do not change Stage12–Stage15 historical branches.
- Never treat an uploaded source archive or README as runtime evidence.
- Do not invent symbol names, addresses, hook counts, build success, or log contents.
- Keep all conclusions traceable to a branch, commit, file, and preferably specific record/time/QPC.
- The user's next chat may contain only this repository URL: https://github.com/Hawky11/1 . Start by reading this handoff and the project history.
