# Stage40 static native-consumer analysis — source only (2026-11-10)

## Proven original binary / call graph

Exact original PE32/i386 AALib.exe (ChatGPT Library /AALib.zip, 175556608 bytes, SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba). PE preferred image base is 00400000; all addresses below are RVA. See tools/verify_stage40.py for deterministic validation of twelve opcode signatures and three direct E8 destinations.

- Native loop AD710 enumerates two sides. AD725 calls 9EEA0 and puts its returned per-player pointer in ECX at AD72D. Unique site AD72F: bytes E8 4C AD 03 00, calls E8480(P).
- E8480 is ECX=thiscall, with no stack arguments. E8489 loads raw request P+3F4C and E848F compares it with FFFFFFFF. Sentinel skips processing to E8554.
- Non-sentinel: E8498 loads bit from P+3B7F; E84AB calls 1E9F00 using raw request. E84B0/E84D4 selects a mode value using P+3978, P+3B86, P+3B88, P+3F20; E84E3 calls native 125920 with two stack arguments; E84ED checks EDI=returned EAX against FFFFFFFF.
- If rejected: E8535 clears bit24 of P+3B88; no actor helper call. Both nonempty-request branches reach E853F and reset P+3F4C=FFFFFFFF, then E8549 clears P+3B7C bit24.
- If valid: E84F7 calls 156C00 for context; E8505 calls 1357F0 with **four stack arguments**; EAX is the actual actor pointer. E850A sets actor+10 bit15. E8514 sets actor+2D0=P+4. E8523 writes actor+280=P+3F4C (raw request). E8529 sets P+3B88 bit24.

## Instrumentation

Three native five-byte CALL sites only: AD72F, E84E3, E8505. Exact five-byte expected instructions and three target entry prologues checked twice around the existing mutex/suspend thread patch transaction. No hook on E8523 store or any original instruction except the three CALLs. All original callees run once; no gameplay writes or new inputs.

- Outer wrapper checks P+3F4C=FFFFFFFF. For sentinel, it **tail jumps** directly to native E8480, leaving original return address on the stack (and flags restored); no emitted window. For other requests captures P snapshots before and after native E8480.
- Selector wrapper invokes original native 125920 with two **copied stack args in original order**, because simply calling the native target with the original return address under the stack would corrupt the ABI. After the native call it captures EAX and associated current P request in TLS.
- Actor wrapper similarly copies four stack arguments into the original 1357F0 invocation, obtains actual actor pointer from EAX and selector from EDI, then captures that actor's fields before native E850A/E8523. After E8480 returns it samples exactly the same actor pointer.
- All wrappers preserve native volatile/nonvolatile registers, EFLAGS, x87/SSE using PUSHFD/PUSHAD and FXSAVE/FXRSTOR. C++ observer helpers do read-only SafeReadDword probes.
- Stage38/Stage39 continue unchanged; same worker drains the Stage40 ring via Stage38's existing FlushStage38. No new INI key. Dependency on Stage38 is explicit. The Stage40 overlay carries byte-identical Stage39ActionOutput.inl.
- Logs: Stage40 seq, side/P/TID, four QPC boundaries, original native selector, original actor pointer, request before/after/at helper, eight independent P fields p0..p7, three actor fields a0..a2, read masks, dropped/overflow/mismatch warnings. Fast-path sentinel calls are intentionally uncounted.

## Verification and open questions

Source and original binaries were statically matched and instruction/displacement checks passed. **Windows x86 MSVC build, DLL installation, ABI execution in game, and Stage40 runtime records are NOT yet verified.** The user must first compile and check Build.log; do not install on compiler/linker errors. Run natural COM-vs-COM and then examine new logs, especially whether rejected selectors and successful actor writes both appear.

Do not infer that actor+280 is an animation ID, that rendering/hit/collision/damage happened, or that AI improved. Stage39 must correlate to Stage40 by same player and chronology, not merely matching repeated action IDs. Keep Stage29 AITrace.cpp and historical Stage branches untouched. No need to re-upload the original EXE.
