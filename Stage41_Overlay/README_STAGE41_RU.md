# Stage41 — read-only native action object snapshots (source only; x86/runtime pending)

**Background.** Stage40 r2 verified [runtime upload](https://github.com/Hawky11/1/commit/e4d29fa639c74ab3e5345cf91fec729d43e43f78): Windows x86 BUILD SUCCEEDED (688128-byte DLL), STAGE40_INSTALLED 3/3. It produced 489 non-sentinel consumer windows, P1=271, P2=218. All 489 native 125920 selector results were FFFFFFFF, but that does not establish that the underlying action was discarded. 56/57 Stage39 output requests match the immediately next same-side Stage40 consumer with the same ID; one Stage39 tail is beyond the final Stage40 window.

## Static finding from original AALib.exe

E8480 calls original **1E9F00 at E84AB BEFORE 125920**. ECX is actual player object [P+4]. The native helper takes (rawRequest, 0, bit24(P+3B7C)), returns with RET 0Ch.

- When bit24=0 and [object+60]==request: writes [object+68]=FFFFFFFF, returns AL=0 (duplicate/current request).
- Otherwise: clears the low byte at object+30, writes [object+64]=request, sets bit0 of [object+08], returns AL=1 (new request state).
- The later 125920 table lookup is a separate, special mapping. Its original table has exactly 40 entries: mode 8:3, mode 9:28, mode 29:9. In this session both P+3978 and P+3F20 were 10 (P1) or 23 (P2), hence no matching table-mode entry. Its FFFFFFFF result does not roll back the earlier direct object write.
- Do not force any inputs, modes, IDs, actor pointers or table entries.

## One-file install

Replace only D:\INCUBATOR\ArcanaAIOverhaul\src\Stage40RequestConsumer.inl with the [Stage41 source file](src/Stage40RequestConsumer.inl).

Keep Stage38CallbackWindow.inl, Stage39ActionOutput.inl, Stage38 AIQueueTrace.cpp, Stage35/36 inls, Stage29 AITrace.cpp, ArcanaAI.ini and Build.bat from the already working Stage40 r2 project **unchanged**. No new INI option or native CALL hook. Stage41 piggybacks on existing Stage40 TLS windows / ring / worker, adding only read-only object snapshots. The original 3 Stage40 CALL wrappers and Stage40 log are retained.

New log: ArcanaAI_Stage41ObjectCommitTrace.log, one STAGE41_OBJECT_WINDOW per Stage40 observed non-sentinel request with identical stage40Seq, P1/P2, player, thread and QPC. objectBefore/objectAfter are independent P+4 pointers. Five-bit preMask/postMask validate object-relative fields (0x1F means all five readable). Compare fields only when object pointer unchanged:

- o0 = object+08, including bit0 pending state
- o1 = object+30 (low byte is relevant)
- o2 = object+60 (current raw ID)
- o3 = object+64 (pending raw ID)
- o4 = object+68 (duplicate/current state)
- request = P+3F4C on entry; mode = P+3F20; bit24Before = bit24 of P+3B7C.

Expected Stage41 log markers: STAGE41_DIRECT_OBJECT_BUILD=2026-11-10-r1 and STAGE41_ENABLED piggyback=Stage40 3/3; Stage40 existing marker remains STAGE40_ACTION_CONSUMER_BUILD=2026-11-10-r2.

## Run and acceptance

Archive old logs. Run the existing **x86 Build.bat**. If Build.log reports BUILD SUCCEEDED, install the newly built dinput8.dll and run natural COM-vs-COM without forced controls, IDs or state changes. Allow at least two seconds for the worker to flush. Upload new Build.log, Stage41, Stage40, Stage39, Stage38, Stage37, Stage36, EventDispatch, QueueTrace, ParentFlow, FinalCommandTrace and Stage35 logs to main.

Next analyst: check Stage40 INSTALLED 3/3 and Stage41 ENABLED, sequence integrity, 1:1 joins via stage40Seq/P/side/TID/QPC, pointer consistency, masks and dropped/overflow/mismatch. Separate new request writes (o3.post should equal raw request when bit24Before=1 or o2.pre differs) from duplicate path (when bit24Before=0 and o2.pre equals request, o4.post should become FFFFFFFF). Beware repeated IDs, changes by other native code and insufficient exit flush. Native command-state write is NOT proof of animation, hit, damage or AI improvement.

**Stage41 is SOURCE ONLY**. Windows x86 compilation and Stage41 runtime are not yet verified. SHA-pinned original AALib.exe is already in ChatGPT Library (SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba). Do not ask to re-upload it. Preserve all historical branches.
