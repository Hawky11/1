# Next session handoff

**Read this file first in a new chat.** The user may provide only the repository link. Continue from the exact active diagnostic branch and its README; do not ask for files already present in the repository or Library.

## Repository and current checkpoint

- Repository: https://github.com/Hawky11/1
- Documentation branch: `main`
- Latest runtime-tested stage: **Stage15**
- Current diagnostic implementation branch: `stage16-player-bit17-writer-trace`
- Stage16 HEAD at last check: `590675c3db91ea0b4def838bce91908b7b97c667`
- Stage15 evidence branch / unchanged HEAD: `stage15-postguard-flow-diagnostics` / `4d1e2a049c835a991647cb04565d31d330aeac95`
- Stage16 instructions: [Stage16_Overlay/README_STAGE16.md](https://github.com/Hawky11/1/blob/stage16-player-bit17-writer-trace/Stage16_Overlay/README_STAGE16.md)
- Stage16 static review: [STATIC_XREF_REVIEW.md](https://github.com/Hawky11/1/blob/stage16-player-bit17-writer-trace/Stage16_Overlay/STATIC_XREF_REVIEW.md)
- Full history: [PROJECT_HISTORY.md](PROJECT_HISTORY.md)

Always re-check the remote branch HEAD and changed files first. Stage16 HEAD may have moved if the user uploads build/runtime logs or further source fixes.

## Stage15 evidence to retain

The event under investigation is type `1`, payload `0x1E`, produced in the parent routine at `AALib.exe+0xF69F0`.

In the latest reviewed Stage15 run:
1. Four invocations returned `0x11E` from F2F50.
2. F2ED0 returned `AL=0` in all four.
3. `EDI=0x1E` passed in all four.
4. Three events had bit17=1 in `[player+0x3B84]` and did not reach F6AD7.
5. One event (P2 at 12:16:45.020) had bit17=0, passed through F6A9D and reached F6AD7.
6. The P2 event was accepted by QueueTrace and observed by EventDispatchTrace with `(type=1,payload=0x1E)`.

Stage15 installed its five probes (5/5); 13 Stage15 records had `fieldsMask=0x0FFF` and `localRead=(1,1)`. The Stage15 upload did not include Build.log. The four-event pattern is a lead, not a universal rule.

## Stage16: source-only writer tracing prepared

Stage16 was created from the exact Stage15 HEAD above. Historical Stages12–15 remain unchanged.

### Static binary finding

The user's Library file `AALib.zip` was used; SHA-256 for `AALib.exe` is `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. It is PE32 i386, ImageBase `0x00400000`, SizeOfImage `0x0E0EB000`.

The static xref scan found 107 direct references with displacement `+0x3B84`, including 60 write-like RMW operations. The most relevant direct clear is VA `0x004ED19F` / RVA `0x000ED19F`:

```asm
and dword ptr [esi+3B84h], 0xFFF9FFFF
```

This clears bits 17 and 18. The same routine immediately before it calls `AALib.exe+0x4F2ED0` with type=1 and payload=`0x1E`; the clear occurs only when the helper returns `AL=0`. This is only a candidate until Stage16 confirms the effective address is the P1/P2 player field and the instruction runs during the scenario.

No immediate `OR [reg+0x3B84],0x00020000` was found. Contextual checks resolved `0x004F7BD4` as OR with EAX=1 (bit 0) and `0x005BF48C/0x005BF495` as OR with EAX=`0x00800000` (bit 23). See the static xref review for the full writer list.

### Stage16 implementation

The overlay modifies the existing Stage11 `src/AITrace.cpp` module. It uses write-only 4-byte DR0/DR1 data breakpoints on each exact runtime P1/P2 `[player+0x3B84]` DWORD:
- DR7 configuration `0x00DD0005` (local DR0/DR1 enable, RW=01 write-only, LEN=11 four bytes).
- Player object bases derive from the confirmed P1/P2 COM control addresses minus `0x3F2C`; the watched DWORD is then at +`0x3B84`.
- Only threads with DR0 and DR1 address registers both zero and all DR0/DR1 enable and RW/LEN fields clear (`DR7 & 0x00FF000F == 0`) are eligible. Existing DR2/DR3 are preserved. COMvsCOM writer thread is excluded. Threads skipped for occupied slots are logged and not retried.
- The legacy Stage2 `EnableControlFieldTrace` mode shares DR0/DR1 and must stay disabled. Supplied INI does so and enables `EnablePlayerBit17WriteTrace=1`.
- The dedicated log is `ArcanaAI_PlayerBit17WriteTrace.log`; it captures writes from any user-mode module in the target process, absolute EIP-after, `inAALib=0/1`, AALib RVA when applicable, 24 code bytes, thread/context, QPC/time, new DWORD and a best-effort shadow old DWORD/bit17.
- The old-value shadow is best-effort. `shadowInitialized=1` only means the startup baseline read succeeded; it does not prove every write was seen. A skipped thread or dropped record may leave it stale. Check all warning and drop records.

### Critical current status

**Stage16 has not been compiled, installed, or runtime-tested in this environment.** There is no Stage16 Build.log or fresh Stage16 runtime log yet. Do not claim the watchpoints installed or that the candidate writer fired.

## Next concrete task: obtain and analyze the Stage16 run

The Stage16 overlay instructions are authoritative: [README_STAGE16.md](https://github.com/Hawky11/1/blob/stage16-player-bit17-writer-trace/Stage16_Overlay/README_STAGE16.md).

User-side steps:
1. Start with a clean extraction of `ArcanaAIOverhaul_Stage11_Source.zip`.
2. Apply Stages12, 13, 14, and 15 in the exact order described in the existing READMEs.
3. Apply Stage16 last: replace `src/AITrace.cpp` and `ArcanaAI.ini`.
4. Build with the Stage12 Build.bat and MSVC x86. Retain the full `Build.log`; do not replace the known-working DLL if the build fails.
5. Start with clean runtime logs, run a short COM-vs-COM match, and close normally so buffers flush.
6. Upload fresh files to the Stage16 branch.

Required Stage16 upload:
- `Build.log`
- `ArcanaAI_PlayerBit17WriteTrace.log`
- `ArcanaAI_ProducerCallTrace.log`
- `ArcanaAI_AlternateCallTrace.log`
- `ArcanaAI_QueueTrace.log`
- `ArcanaAI_EventDispatchTrace.log`
- `ArcanaAI_FinalCommandTrace.log`

Keep `ArcanaAI_ParentFlowTrace.log` and `ArcanaAI_ProducerContextTrace.log` when available.

### How to review the uploaded run

1. Re-check Stage16 branch HEAD and list the changed files. A source/README/INI-only upload is not a runtime test; require Build.log and fresh runtime logs.
2. Read Build.log for successful x86 build and warnings/errors.
3. In `ArcanaAI_PlayerBit17WriteTrace.log`, confirm Stage16 startup addresses, initial values, `DR7=0x00DD0005`, and one or more `Hardware watchpoint install batch` records.
4. Inspect every skipped-thread warning and dropped-record warning. If any relevant writer thread was skipped, do not claim complete write coverage.
5. Analyze `PLAYER_BIT17_WRITE` entries. The logged EIP is **after** the write; disassemble the 24-byte code window to identify the writer instruction. If `inAALib=0`, use the absolute EIP and code bytes to identify the external in-process module. Correlate side, address, thread ID, QPC, timestamp, registers, full DWORD and bit17 transitions.
6. Check whether VA `0x004ED19F` / RVA `0x000ED19F` actually fires for P1/P2. Do not assume ESI is a player base until the watched effective address and player object match the logged target.
7. Correlate bit17 writes with Stage15 F6A68/F6A9D/F6AD7, AlternateCallTrace, QueueTrace and EventDispatchTrace. Establish whether a transition precedes or follows the parent branch and event insertion.
8. Separate observed facts from hypotheses. The one clear path at 0xED19F does not prove it is the only writer.
9. After analysis, append a dated Stage16 runtime entry to `PROJECT_HISTORY.md`, update this handoff, and refresh `README.md` with the current Stage16 HEAD. Create a Stage17 branch only if evidence justifies a new diagnostic question.

## Rules for future sessions

- Inspect repository files and existing Library uploads before requesting anything from the user.
- Never modify Stage12–Stage15 historical branches.
- Never infer a runtime test from source code or README uploads.
- Never invent build results, hook installation, or log contents.
- Keep the user's next action clear and avoid asking for information already present in the repo or conversation.
