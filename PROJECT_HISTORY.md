## Stage153 — current-source native ingress / worker / VEH unload preflight: FIVE BLOCKERS (2026-11-13)

[Stage153 checkpoint and Windows handoff](STAGE153_CHECKPOINT.md). Pinned user's Stage146 ZIP `6d9c39d59b4883dab6cc4ea95f08deac56fe8c477fd6b4746bd1c764f96a207b`: 85/85 copied sources reverified, 8 MSBuild compile roots / 66 literal-include reachable files. **8 workers** launched by `Proxy.cpp DirectInput8Create`: each start uses CreateThread and promptly CloseHandle(thread), with worker `for(;;)` and no verified cooperative stop/join protocol. `Proxy.cpp` DllMain handles ATTACH only, no DETACH/shutdown; do **not** block/join inside DllMain (loader lock). **3 reachable VEH registrations** (AITrace.cpp:2738, AICommandTrace.cpp:805, Stage59XWriterWatch.inl:292), **0** reachable RemoveVectoredExceptionHandler calls; Stage59 explicitly keeps VEH for process lifetime. **122 reachable naked hook declarations** are *not* installed-owner evidence. **0 current FLS API allocations**, do not transpose earlier Stage142/143 test DLL FLS to production game DLL. **No game DLL unload** until native owners/foreign callers/worker quiescence/VEH retirement/ABI/SEH/unwind verified. Synthetic pure C++17 16-condition preflight exhaustive **65,536** cases = 65,535 denied + 1 hypothetical all-true (NOT game permission); 16/16 single-missing denied. **24/24 offline tests**, GCC/Clang matching output, 12/12 payload hashes; ZIP 23,611B SHA256 `031d1a58229021768923da530d7e9bc95e338f4ed6c2c2f6fee08fc99edb1e0b`; **Stage153 Windows MSVC x86 NOT YET RUN**, unlike user-confirmed Stage152 Win32 x86 PASS. `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, origin(0x27)=UNKNOWN. No DLL/INI/project writes. NEXT Stage154 source-only real shutdown/admission design and rollback barriers, no game install.

---

## Stage152 — user-confirmed VS2026 MSVC x86 isolated Win32 sandbox PASS (2026-11-13)

[Stage152 full checkpoint, Windows command sequence, exact user console and safety gates](STAGE152_CHECKPOINT.md). User ran `Invoke-Stage152Audit.ps1` with the Stage151 ZIP in **Visual Studio 2026 Developer PowerShell v18.9.1**, initialized with `Launch-VsDevShell.ps1 -Arch x86 -HostArch x86`; `cl.exe` resolved to MSVC 14.51.36231 `HostX86\\x86`. Two initial setup issues resolved: execution policy (`Set-ExecutionPolicy -Scope Process Bypass -Force`) and missing developer environment (`Get-Command cl.exe` failing before VsDevShell). Stage152 package manifest 14 entries OK; pinned Stage151 archive SHA256 `7016b1f01d486c5ff27a3bb4017e7191473569d794735567e45ccb5f3cc2d4d1`, nine payload members verified. Python **25 PASS, 2 SKIP** (no GCC/Clang on user's Windows system; zero failures). Native MSVC Win32 x86 `transaction_harness.cpp` + `win32_private_fixture.cpp` compiled and executed: `STAGE152_NATIVE_MODEL cases=149 active=2 clean=56 quarantined=91 dual=48 all_faults_reached=YES`; `STAGE152_WIN32_X86_SELF_OWNED stage67_memory=PASS stage70_memory=PASS cooperative_peer=PASS api_faults=48/48 stage67_injection=PASS stage70_injection=PASS abi_probe=PASS seh_probe=PASS external_process_access=NO`; **`STAGE152_LOCAL_WINDOWS_TESTS_PASS`**. This is a **self-owned private test sandbox**, NOT a game DLL test/installation or live owner/ABI/unload proof. `SAFE_TO_INSTALL=false`, Stage36 OFF, origin(0x27)=UNKNOWN, working Stage129 DLL 2,099,200 B and Stage122 rollback 2,089,984 B preserved. **NEXT Stage153:** source-only real native ingress, shutdown, FLS, worker and safe unload admission audit; no automatic game injection.

---

## Stage148–151 — current-source hook/write provenance and Stage67/70 rollback/quarantine source-only progression (2026-11-13)

[Full Stage148–151 checkpoint](STAGE148_151_CHECKPOINT.md). Stage148: 131 source-declared intervals across 53 source files, 181 unclassified calls; 5 conditional interval overlaps (4 dormant Stage83↔Stage84, 1 Stage36↔Stage82 at F79E4), 6/6 tests. Stage149: 181/181 call candidates classified, 217 potential memcmp signature checks, 30 dependency edges, 11/11 tests; Stage67 post-write failure risk. Stage150: 11 bounded address→write source witnesses, Stage67 dual-site / Stage70 single-site fault and rollback gap, 17/17 tests; installed flags can precede success verification. Stage151: isolated transactional fault model reported 173 reached cases = 2 model active + 70 verified clean rollback + 101 quarantine, 30/30 tests; unknown thread resume or rollback is **QUARANTINED**, never falsely clean. These are offline, source-only artifacts; **none installed into game**, live patch owners/production rollback not proven. `SAFE_TO_INSTALL=false`, Stage36 OFF, origin(0x27)=UNKNOWN. Details and package names in checkpoint.

---

## Stage147 — actual Stage146 evidence ZIP inspected; 485 RVA reference classification / dormant overlap (2026-11-13)

[Full verified Stage147 checkpoint](STAGE147_CHECKPOINT.md). Corrected the earlier mistake that Stage146 evidence ZIP was not uploaded: user's `ArcanaAI_Stage146_Evidence_493fc20dff504bdabc3abc4924a9426f.zip` WAS received (489,054 B SHA256 `6d9c39d59b4883dab6cc4ea95f08deac56fe8c477fd6b4746bd1c764f96a207b`, 94 entries CRC PASS). Independently validated **85/85 source copies SHA+size** against included current_sources.json, while 70/70 original Stage134 matches are Windows collector metadata (Build.bat/config not all re-copied). 37 paths new against archival manifest: **31 historical backup/source+manifest files**, **6 config/installation state metadata**. Reconstructed exact current .vcxproj eight compile roots and 66-file literal include graph: `Stage83F3930Writer.inl` **not reachable**, `Stage84F3930Writer.inl` reachable. Classified all **485** Stage146 heuristic RVA rows: 132 backup, 4 dormant Stage83, **12 false-positive error log lines** (not actual patch writes), 337 reachable source literals with ownership UNPROVEN. Added Stage10 parent F3280/F4030/F4360 and Stage81 F7D22/F7D38/F7D43 to prior Stage10 producer F34D0/F3930 + Stage129 F30B0/F325A/F326A = **11 bounded sites, 55/55 pair comparisons zero overlap**. Four 6-byte sites at F3DE9/F3E02/F3F50/F3F69 are declared in both dormant Stage83 and active-chain Stage84, **conditional source conflict only if Stage83 reintroduced**, no proof of live double-patching. Delivered metadata-only chat ZIP `ArcanaAI_Stage147_CurrentHookOwner_CollisionAudit_MetadataOnly_r2.zip` 15,749 B SHA256 `4f90a72b02f54b08f022553850fefc77185bf2852b4e93f81d278fb227858fe3` 11 files CRC + 10/10 manifest PASS, **21/21** Python tests, no source/INI/binaries; full Russian report included. Other source patch-writer-to-site ownership and current runtime byte owner remain unverified. **SAFE_TO_INSTALL=false**, Stage36 OFF, origin(0x27)=UNKNOWN; Stage129 2,099,200 B DLL and Stage122 2,089,984 B rollback untouched. Next Stage148 source-only comprehensive reachable patch-writer provenance.

---

## Stage146 — user Windows current-source 70/70 SHA MATCH, 37 additional paths; evidence ZIP pending (2026-11-13)

[Stage146 exact console transcript and audit](STAGE146_CHECKPOINT.md). User executed `Capture-Stage146CurrentSources.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul' -IncludeSourceSnapshot` in PowerShell 5.1. Reported `STAGE146_CAPTURE_COMPLETE`; `source=85 config=22 skippedGenerated=11 skippedSensitive=0 readErrors=0`; **archived 70 expected / 70 current SHA MATCH / 0 drift / 0 missing / 37 NEW relative to Stage134**. 24 `.vcxproj` items, 409 include edges, 46 names-only INI keys. Five historical Stage10/Stage129 hook sites matched reference source hashes and had 0 mutual interval overlaps; NO live owner proof. 485 **heuristic RVA text references** cannot yet be assigned patch ownership. Generated user-side temp evidence ZIP `ArcanaAI_Stage146_Evidence_493fc20dff504bdabc3abc4924a9426f.zip` **not uploaded, not inspected**, hence Stage147 exact source-level collision/caller mapping awaits file. Stage146 offline 104/104 static + 24/24 unit tests, chat-only source tool ZIP 60,443 B SHA256 `b9bc8317aae43a29a024f65f3581b61d4bb4d277a46eeea63642e064bff9f9db`. Read-only, no running game opened; `STAGE146_NO_LIVE_OWNER_PROOF`. **SAFE_TO_INSTALL=false**, Stage36 OFF, origin(0x27)=UNKNOWN, Stage129 DLL 2,099,200 B and Stage122 rollback 2,089,984 B unchanged. Next Stage147 review after ZIP upload.

---

## Stage146 — read-only full current project source/config census with historical owner-site classification, source-only (2026-11-13)

[Detailed Stage146 checkpoint and Windows handoff](STAGE146_CHECKPOINT.md). Stage145 user-confirmed source validation covered only five current files (11 archive refs); it explicitly did not verify the current full 70-file Stage134 list or live owners. Stage146 builds a standalone **Windows PowerShell 5.1 read-only** capture that enumerates current eligible sources/config, SHA256-comparisons for all **70 Stage134 archival paths** (MATCH/DRIFT/MISSING, plus additional NEW paths), .vcxproj compile/include text, C++ `#include` edges, and names-only INI key inventory. It marks five known archived hook intervals Stage10 F34D0 5B / F3930 6B; Stage129 F30B0 6B / F325A 5B / F326A 5B with no overlaps among these five, **not** any claim of complete live hook census. Heuristic RVA references explicitly unverified as owners. Local capture writes solely under %TEMP%, has opt-in `-IncludeSourceSnapshot` (source/project files only, excludes .ini and binaries, requires user privacy review); game project and DLL untouched. Offline checks **104/104 static**, **24/24 Python model/unit PASS**; chat-only `ArcanaAI_Stage146_FullCurrentSource_OwnerInventory_ReadOnly_SourceOnly.zip` **60,443 B**, SHA256 `b9bc8317aae43a29a024f65f3581b61d4bb4d277a46eeea63642e064bff9f9db`, 22 files CRC and 21/21 manifest PASS. **Windows Stage146 full current 70-file inventory NOT RUN** and no live installed-owner/ABI/worker/FLS evidence. NEXT user runs `Stage146_SourceOnly/tests/Capture-Stage146CurrentSources.ps1` read-only and sends its generated evidence ZIP; next Stage147 audits current-owner conflicts using actual source, never automatic native installation. **SAFE_TO_INSTALL=false**, Stage36 OFF, origin(0x27)=UNKNOWN; preserve Stage129 2,099,200 B `dinput8.dll` / Stage122 2,089,984 B fallback.

---

## Stage145 — user-confirmed Windows current/source reference SHA and ledger integrity PASS (2026-11-13)

[Stage145 checkpoint with complete Windows PowerShell console log](STAGE145_CHECKPOINT.md). User ran `Test-Stage145LocalEvidence.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'`: 11/11 archived reference hashes MATCH (5 original, 2 Stage139, 2 Stage141, 2 Stage143), 5/5 **current** Windows source hashes MATCH (AIProducerContextTrace.cpp, ArcanaAIOverhaul.vcxproj, Proxy.cpp, Stage129ProducerProbe.inl, Stage81SelectorWriter.inl), 38/38 selected Stage144 surface records mapped and **17/17 obligations present in static ledger**, final `STAGE145_LOCAL_EVIDENCE_PASS refs=11 current=5 obligations=17`. Script explicitly says `STAGE145_NO_LIVE_OWNER_PROOF`: 17 obligations are **NOT** proven satisfied and no loaded hook/worker/FLS/ABI state inspected. Stage145 offline 92/92 static + Stage144 102/102 regression + 14/14 Python adversarial model + 65,536 synthetic assertion masks PASS. Corrected source-only ZIP `ArcanaAI_Stage145_ShutdownObligations_AdversarialNativeIngress_SourceAudit_r2.zip` 90,872 B SHA256 `d59c55d30524263994bf9e2e2aa432cb352a4afb1a540196790ec3374d5a208f`, 34 files CRC and 33/33 manifest PASS. **SAFE_TO_INSTALL=false**, Stage36 OFF, origin(0x27)=UNKNOWN. Preserve tested Stage129 2,099,200 B DLL and Stage122 fallback 2,089,984 B. Next Stage146: full read-only current project/source/config owner map, not live install.


---

## Stage144 — current Windows source snapshot verification 5/5 MATCH (2026-11-13)

[Full Stage144 source audit and exact Windows transcript](STAGE144_CHECKPOINT.md). After Stage144 static 102/102 and synthetic 6/6 (4096 boolean states), user ran `Test-Stage144LocalSnapshot.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'` in PowerShell 5.1: **5 SOURCE_MATCH, 0 drift/missing**, final `STAGE144_SNAPSHOT_MATCH files=5; snapshot-only, SAFE_TO_INSTALL=false`. Matched `src/AIProducerContextTrace.cpp`, `ArcanaAIOverhaul.vcxproj`, `src/Proxy.cpp`, `src/Stage129ProducerProbe.inl`, `src/Stage81SelectorWriter.inl` to November 12 source-capture hashes. The original captured 70/70 archival-file checks were not an independent current 70-file Windows-tree check. No running game opened; `STAGE144_NO_LIVE_OWNER_PROOF` is explicitly reported, so loaded hook ownership/ABI/unwind, raw game entry quiescence, FLS and worker teardown remain unverified. The bounded 38-surface mapping and source-only ZIP `ArcanaAI_Stage144_RealEntrypoint_Coverage_AdmissionGap_SourceAudit.zip`, 75,660 B SHA256 `5adfe52843106df2a1832f6e7b70fdceaf645620b35d93d1df2ceadd696b32fc` remain the supporting static record. **SAFE_TO_INSTALL=false**, Stage36 OFF, origin(0x27)=UNKNOWN; preserve Stage129 2,099,200 B DLL / Stage122 2,089,984 B rollback. NEXT Stage145 source-only shutdown obligations and bypass-case audit.

---

## Stage143 r2 — user-confirmed Windows x86 native test DLL/host 16/16 PASS (2026-11-13)

[Full Stage143 checkpoint and native Windows transcript](STAGE143_CHECKPOINT.md). Stage143 r1 PowerShell runner failed before compilation due to read-only automatic `$Host` collision (`$host` assignment). r2 changed **four** uses to `$hostExe`, left C++ unchanged and passed rechecked **66/66 static**, **526/526 GCC** and **526/526 Clang** policy tests. User reran r2 on VS2026 Community MSVC x86: compiled/linked disposable `Stage143FlsProbe.dll` + `Stage143Host.exe`, `STAGE143_NATIVE_BUILD PASS`, all **16/16** admission/FLS registry/unload scenarios PASS. Results: `orphan_host_lease` PASS, `denied_after_host_close` hostDenied=20, `race_gate` hostDenied=1, `parallel` callbacks=8/manual=512/live=registry=inFlight=registryErrors=0, `registry_audit` blocked=1/errors=0. R2 source ZIP `ArcanaAI_Stage143_IsolatedAdmission_Registry_UnloadBarrier_SourceOnly_r2.zip` 31,171 B SHA256 `6e3c1c77525421ed7f156144c8910f924fc8ff98209b83c1272bbbb5bfe95cf4`, 18 files CRC PASS. MSVC produced garbled Cyrillic linker status but no error. These are only **test DLL/host** results, NOT complete owner/caller registry or safe game DLL unload. **SAFE_TO_INSTALL=false**, Stage36 OFF, origin(0x27)=UNKNOWN; preserve Stage129 tested 2,099,200 B DLL and Stage122 rollback 2,089,984 B. **NEXT Stage144:** source-only complete entrypoint coverage and missing-admission audit.

---

## Stage143 — two-level admission gate / atomic FLS registry source-only isolated experiment (2026-11-13)

[Stage143 isolated admission + FLS registry checkpoint](STAGE143_CHECKPOINT.md) is the full source/test/safety record. Stage142 prior **13/13 Windows x86 isolated DLL lifecycle PASS** does not prove safe unload when outside callers retain raw exports. Stage143 introduces cooperative host-side lease (lease++ before testing admission, close gate, await lease zero) and per-DLL inFlight tracking plus **512-entry bounded atomic CAS FLS registry**, callback removal and registry consistency checks before experimental `FlsFree`. Host `FreeLibrary` occurs only after local quiescence; foreign or bypassing callers not accounted, real game safety not established. Tests planned (16 scenarios) cover orphan host lease with DLL inFlight=0, active DLL call, live FLS slot, fibers, races, parallel callbacks, preflight flags and two reload cycles. **Static audit 64/64**, **portable model 526/526 GCC** and **526/526 Clang** PASS, all 256x2 bitmask combinations. **Stage143 MSVC Windows native test NOT RUN**. Chat ZIP `ArcanaAI_Stage143_IsolatedAdmission_Registry_UnloadBarrier_SourceOnly.zip` 30,386 bytes SHA256 `1842562f03488399e1170a80767faa2a229d2b35c5496af9f84cd1546cf2d01a`, 18 files CRC+SHA manifest PASS, no EXE/DLL. **SAFE_TO_INSTALL=false**, Stage36 OFF, candidate27 origin UNKNOWN, Stage129 DLL 2,099,200 B / Stage122 backup 2,089,984 B untouched. Next Windows 16/16 test output, then Stage144 real module entrypoint coverage analysis; no auto install.

---

## Stage142 — user-confirmed Windows x86 isolated FLS DLL/host 13/13 PASS (2026-11-13)

[Full Stage142 audit and exact MSVC x86 / test console output](STAGE142_CHECKPOINT.md). User executed `Test-Stage142NativeHarness.ps1` under PowerShell 5.1: VS2026 Community x86 compiled `Stage142FlsProbe.cpp` into **disposable test DLL** and `Stage142Host.cpp` into **test host EXE**, `STAGE142_NATIVE_BUILD PASS`, `STAGE142_NATIVE_HARNESS PASS: 13/13 isolated DLL lifecycle scenarios`. Real test callbacks: `thread_exit`=1, `fiber_delete`=1, `fiber_switch`=1 with manualFrees=1. `parallel_exit` callbacks=8, manualFrees=512, live=0, inFlight=0; `live_slot_block` blocked=1, `inflight_block` denied=1 blocked=1, `preflight_missing` blocked=8, `denied_after_stop` denied=2, `reload_twice` PASS. Garbled linker-generated .lib/.exp message is console mojibake; final build passed. Portable Stage142 audit 44/44 static and 291/291 GCC/Clang checks; chat-only source ZIP `ArcanaAI_Stage142_IsolatedFLS_DllUnload_Lifecycle_SourceOnly.zip`, 23,093 B SHA256 `2fa2d5c4f7d778079925f130b19d57998d1ea97f8797d1f8d717d13c3553461b`, 15 files CRC PASS. Controlled FlsFree/FreeLibrary in **test DLL** is not proof of safe game-bridge unload or native detour ownership, ABI, scope/caller registry or real quiescence. **SAFE_TO_INSTALL=false**, Stage36 OFF, `origin(0x27)=UNKNOWN`; retain Stage129 DLL 2,099,200 B / Stage122 rollback 2,089,984 B. **Next Stage143:** source-only admission/callback registry and unload barrier research in isolated host.

---

## Stage141 — Windows MSVC x86 native isolated harness 16/16 PASS, FIFO race fixed in source-only candidate (2026-11-13)

[Stage141 detailed checkpoint and exact Windows test output](STAGE141_CHECKPOINT.md). User ran `Test-Stage141NativeHarness.ps1` in PowerShell 5.1 on VS2026 Community x86: isolated `Stage141Win32Harness.cpp` test EXE **BUILD/LINK PASS** and **16/16 deterministic scenarios PASS**. `parallel` 400 queued, 400 finalized, 400 ACK, `sinkTainted=0`; `order_fixed` 2 queued/2 ACK, asserts repaired monotonic FIFO sequence. Stage140 had intentionally reproduced old `sequence`-before-ring-lock bug. `fls_exit` (1 seen, 0 finalized, 0 ACK, tainted=1) and `write_fail`, `partial`, `flush_fail`, `open_fail` (ACK=0, tainted=1) behaved as expected. `fls_preflight` PASS only evaluates a gate — **no `FlsFree`, no real DLL unload, no live game ABI proof**. Prior Stage141 offline evidence: 52/52 static, 19/19 GCC and Clang, 256 FLS combinations. ZIP `ArcanaAI_Stage141_SequenceFIFO_FLS_UnloadPreflight_SourceOnly.zip`, 36,059 B SHA256 `8097b0e05a3ee308824b33f79cd129cea56c63283fac346f098a6d55f9c860f5`; 19 files CRC PASS. **SAFE_TO_INSTALL=false**, Stage36 OFF, candidate origin of `0x27` UNKNOWN; preserve Stage129 tested DLL 2,099,200 B and Stage122 rollback 2,089,984 B. **Next Stage142:** isolate safe FLS shutdown and unload lifecycle in separate test DLL/host.

---

## Stage140 — isolated Windows x86 native harness 15/15 PASS, sequence-order bug REPRODUCED (2026-11-13)

[Full Stage140 checkpoint with exact Windows console output](STAGE140_CHECKPOINT.md). User ran `Test-Stage140NativeHarness.ps1` under Windows PowerShell 5.1; MSVC selected VS2026 Community x86, `Stage140Win32Harness.cpp` built/linked into **isolated test EXE**, `STAGE140_NATIVE_BUILD PASS` and `STAGE140_NATIVE_HARNESS PASS: 15/15 deterministic scenarios`. `parallel` 400/400 queued/ACK/finalized taint=0. `fls_exit` 1 observed/0 finalized/0 ACK taint=1. `write_fail`, `partial`, `flush_fail`, `open_fail` each ACK=0 with taint=1. `order_repro` 2 ACK/2 queued, **demonstrates** real Stage139 race: seq assigned before SRW publication lock, FIFO can invert numbering; no fix yet. Portable checks 40/40 static, 20/20 sink model, 39/39 GCC+Clang regression. Chat-only Stage140 ZIP 30,026 bytes SHA256 `30e55725ee79266c1d00bd4528554391c36c57478286ac155a79e32c897f2cf2`. Synthetic native harness PASS is NOT game hook ABI/unwind/quiescence/unload proof. Stage141 next: source-only sequence-order lock fix + deterministic test, FLS teardown gate. **SAFE_TO_INSTALL=false**, Stage36 OFF, `origin=UNKNOWN`; Stage129 DLL 2,099,200 B / Stage122 rollback 2,089,984 B unchanged.

---

## Stage136–139 — owner-integrated `0x27` bridge development, source-only (2026-11-13)

[Full Stage136–139 evidence checkpoint](STAGE136_139_CHECKPOINT.md) archives source packages, hashes, tests and open gates. Stage136 unsampled candidate27 model: 20/20 GCC/Clang. Stage137 r2 guarded FLS/SEH wrapper: 22/22 GCC/Clang, 32/32 static, **Windows MSVC x86 /Zs PASS** (VS selection initially misidentified SSMS; fixed by C++ component requirement). Stage138 ABI/SEH/completion and queue-pop-before-write review: 31/31 static and model. Stage139 pending child entry + exit-verified parent linkage and ACK-after-WriteFile/FlushFileBuffers sink candidate: 39/39 GCC/Clang, 20/20 sink model, 51/51 static, regressions PASS. **NEW user-confirmed Stage139 Windows MSVC x86 `/Zs` PASS:** `Stage139Win32Bridge.cpp`, `AIProducerContextTrace.cpp`, `Proxy.cpp`, final marker `STAGE139_MSVC_SYNTAX PASS: /Zs only, no object/DLL emitted; original project untouched.` This checks syntax, **not** ABI, SEH/FLS, linking or gameplay. `VerifiedParent` does not establish origin: `0x27` source **UNKNOWN**. SOURCE ONLY, **SAFE_TO_INSTALL=false**, no new JMP, no Arm(), no DLL; Stage36 OFF. Next Stage140 isolated Windows native ABI/SEH/FLS and sink failure harness; preserve Stage129 DLL 2,099,200 B / Stage122 backup 2,089,984 B.

---

## Stage135 — full current source capture resolved real F34D0/F3930 owner conflict (2026-11-13)

[Full current-source audit](https://github.com/Hawky11/1/blob/stage135-current-source-stage10-owner-collision-audit/Stage135_Audit/CURRENT_SOURCE_STAGE10_OWNERSHIP_REVIEW.md) · [JSON](https://github.com/Hawky11/1/blob/stage135-current-source-stage10-owner-collision-audit/Stage135_Audit/results.json). Captured user working tree `ArcanaAI_Stage134_CurrentSources.zip` (source snapshot UTC 2026-11-12T23:48:42Z), 70/70 SHA256+sizes verified, source ZIP SHA256 e1ae1b0bb74b508a2d47ce600c37f67f730b64306c52fc3d4e6d16cf2077a84f; original AALib.exe 175,556,608 B SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba separately verified. All 21 archived Stage134 source module hashes match this captured current tree, anchoring 52 Stage81–129 patch-site records. Additional source inspection found **Stage10 AIProducerContextTrace.cpp** already installs trampoline owner JMP at **F34D0/5B** and **F3930/6B** when enabled (default 1), so future Stage133 proposed parent detours F34D0/6B and F3930/6B would overlap by **5 and 6 bytes**. 10-site design must be replaced; use existing Stage10 `CaptureAndCall` pre/post original normal-return boundary to attach unique scope, with independent unsampled Stage129 all-input27 evidence and proven exceptional unwind. Stage10 `SuspendOtherProcessThreads` currently returns count 0 on snapshot failure and skips OpenThread/SuspendThread failures, without full EIP/in-flight parent proof; unsafe to reuse as installation primitive. Stage81 starts after Stage80 detour, not proven idle. Limited source registry 88 declared sites (52+2+34); Stage36/Stage82 both mention F79E4 5B, Stage36 OFF. Chat ZIP `ArcanaAI_Stage135_CurrentSource_Owner_Collision_Audit.zip` 10,134 bytes SHA256 4ce2541d6b1df71ce6ac3d662667608025e62d8d96273458c765704152e66910, 5 files ZIP CRC PASS, synthetic tests 12/12. No new patches or DLL. Next Stage136 owner-integrated read-only static design, fail-closed / SEH / loss accounting before opt-in native code. Preserve working Stage129 DLL 2,099,200 B, Stage122 rollback 2,089,984 B, Stage36 OFF.

---

## Stage134 — archive-source owner inventory improved 26→52, native quiescence/unwind STILL BLOCKED (2026-11-13)

[Full Stage134 report](https://github.com/Hawky11/1/blob/stage134-archival-hook-owner-quiescence-audit/Stage134_Audit/STATIC_OWNER_AND_DEPLOYMENT_GATE_REVIEW.md) · [machine verification](https://github.com/Hawky11/1/blob/stage134-archival-hook-owner-quiescence-audit/Stage134_Audit/results.json). Immutable original user AALib.exe 175,556,608 B SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba rehashed. From different user Library historical source overlay archives, retrieved 22/22 named Stage129 adapter includes, identified **21 owner stages and 52 patched RVA byte ranges** with explicit source literal RVA, source SHA and original native bytes. **1,891** disjoint checks (1,326 historic owner-owner, 520 future 10 vs historic owner, 45 future mutual), **0 collisions**, 62/62 original-byte spans, 19/19 Python tests PASS. This is NOT currently compiled integrated owner proof: full current eight C++ translation units, Stage80 E8A8E prerequisite and unknown external detours not verified. Stage81 starts own thread and installs after Stage80, not guaranteed before game parent execution; Stage129 EIP-only suspended-thread check misses parent frame in callee and creation after snapshot. Stage133 five normal RETs cannot guarantee exceptional unwind cleanup; Stage129 sampling lacks all-27 child bridge. Future F3C11 ABI/flag and tail caller scope still not implemented. **Safe_to_install FALSE**, no new native patch, no gameplay writes. Chat ZIP `ArcanaAI_Stage134_OwnerMap_Quiescence_Unwind_StaticAudit.zip` 17,878 bytes SHA256 96f4988a99e67d5bc596dc700d5d385e5daf4fae8902081863c963b24af77211, 11 files CRC PASS, includes a separate read-only *user-run* source collector for Stage135; collector not yet Windows-tested. Keep Stage129 DLL 2,099,200 B, Stage122 rollback 2,089,984 B, Stage36 OFF.

---

## Stage133 — initial five-site provenance design found incomplete: five exit paths + safer F3C11 (2026-11-13)

[Full original native source/ABI audit](https://github.com/Hawky11/1/blob/stage133-parent-scope-exit-abi-static-audit/Stage133_SourceABI/SOURCE_ABI_REVIEW.md) · [machine results](https://github.com/Hawky11/1/blob/stage133-parent-scope-exit-abi-static-audit/Stage133_SourceABI/results.json). User original x86 AALib.exe 175,556,608 B verified SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba. Disassembled F34D0/F3930 and found 5 normal RET: F3662, F366B, F3E4A, F3E5D, F4017. To observe parent exits BEFORE EBP destruction, 5-byte whole-instruction epilogues F365C, F3666, F3E42, F3E55, F4011 were identified. Early F34D0 RET F366B occurs without F30B0 call. Old F3C15 5B hook would steal fault-capable MOV EAX,[EBX]; alternate **F3C11 7B CMP+MOV** keeps that memory-read at original F3C18 but requires exact native CMP EFLAGS preservation for F3C27 JE and post-write confirmation. So revised future minimum normal-return sites **10**, plus optional F3B71 read. Verified 21 opcode byte spans, 2 F30B0 native CALLs, 5 return sites, 150 local direct branches, 2-way indirect F3B12 jump table, 305 nonoverlaps against 26 known nonexhaustive owner envelopes, 39 synthetic state tests. Chat ZIP 11 files SHA256 e2fa357a573324d7111d9618a5c5c11a864451bac1cc707bb8dacf7829c6587a. **NO new patch/DLL**; install remains BLOCKED: no full current integrated source inventory; EIP checks miss live parents inside callees; SEH/nonlocal stack-unwind may skip RET; Stage129 does not log all 0x27 candidate events (sampled only). Next Stage134 owner/quiescence/unwind safety proof before source patch. Preserve Windows Stage129 2,099,200 B and Stage122 2,089,984 B; Stage36 OFF.

---

## Stage132 — why three upstream probes cannot uniquely attribute F30B0 candidate 0x27 (2026-11-13)

[Full audit](https://github.com/Hawky11/1/blob/stage132-upstream27-exact-provenance-feasibility-design/Stage132_Design/FULL_FEASIBILITY_AUDIT_RU.md) · [machine results](https://github.com/Hawky11/1/blob/stage132-upstream27-exact-provenance-feasibility-design/Stage132_Design/results.json), branch `stage132-upstream27-exact-provenance-feasibility-design`. Exact private original AALib.exe SHA verified; **6** full native instruction signatures, **8** CALL targets, Stage131 baseline **162/13/20** native opcode/call/branch checks, **171** hook/site non-overlap comparisons (26 known *nonexhaustive* owner envelopes and mutual), **27** synthetic/offline unit tests PASS. New critical distinction: **F3B71 MOVZX EDI,[mapping] is a READ** and may fail downstream candidate selection; candidate is only written at **F3C15**, which contains a 3B MOV followed by 2B MOV EAX,[EBX], both inside a possible 5B E9 detour. Literal F3579 and F35F9 may both hit; F3C15 may hit repeatedly; caller-supplied ESI can contain inherited 0x27. Correct future design needs parent entries F34D0/F3930, two literal sites F3579/F35F9, and post-transform commit F3C15; optional F3B71; unique invocation IDs, parent stack/unwind cleanup and Stage129 every-candidate27 join. Stage84 six hooks F3DE9/F3E02/F3F50/F3F69/F3E0A/F3F71, Stage129 three hooks disjoint, but complete live ownership **not verified**. Chat-only source ZIP 23,537 B, SHA256 b6b23572c01b3be2513a7f547c55ef859e3ea47e09cf6198b3a7b3bc9dd983dd (14 files). **DESIGN ONLY — no new hooks, MSVC build or game writes.** Stage133 ABI/parent exits first; preserve Stage129 DLL 2,099,200 B, Stage122 rollback 2,089,984 B.

---

## Stage131 — on-disk original producers of the input-candidate `0x27` before F30B0 (2026-11-13)

[Full native audit](https://github.com/Hawky11/1/blob/stage131-original-producer-input27-static-audit/Stage131_Analysis/FULL_NATIVE_INPUT27_PROVENANCE_AUDIT.md) · [machine results](https://github.com/Hawky11/1/blob/stage131-original-producer-input27-static-audit/Stage131_Analysis/results.json). Exact user original AALib.exe SHA verified. **162/162 original opcodes, 13/13 E8 targets, 20/20 Jcc edges, 5 full .text E8 census targets (16 callers), 11/11 group metadata, 256/256 selector rows and 496/496 offline tests PASS.** On original F34D0 path native literal stores F3579/F35F9 assign `[EBP+10]=0x27` before F3654 CALL F30B0, after flag+264/+265, CBF50, and mapping byte==27 guards; F79B1/F7A8A caller selectors are 0/0x22 and both have original column17=0x27. On original F3930 path F3B71 loads table[selector*20+groupColumn] where group 0x24/0x25 uses column17; 26 original selector indexes yield 0x27, F38D0 does not convert 0x27, F3C15 stores candidate, then F3CBC optionally calls F30B0. Both paths can inherit earlier ESI candidate 0x27; runtime Stage129 input-0x27 scope43/56 cannot be uniquely attributed to one original source site without new data. Table .data is not verified live. Chat ZIP `ArcanaAI_Stage131_F30B0_UpstreamCandidate27_StaticAudit.zip` **21,750 B**, SHA256 ded1de041852fa49656bc631f4b9ef6152aa8fe061aa1c0b4a07fdd6edaf12c8 (12 text/Python files, ZIP CRC PASS). **STATIC ONLY**: no new DLL, hooks, gameplay writes or Win32 run. Next Stage132 design-first minimal provenance feasibility or further inherited-ESI static audit; Stage36 OFF, preserve Stage129 Windows-tested DLL 2,099,200 B.

---

## Stage130 — Stage129 F30B0 original-producer branch observed in Windows COMvsCOM (2026-11-13)

[Full runtime audit](https://github.com/Hawky11/1/blob/stage130-stage129-producer-runtime-audit/Stage130_Runtime/RUNTIME_ANALYSIS.md) and [machine results](https://github.com/Hawky11/1/blob/stage130-stage129-producer-runtime-audit/Stage130_Runtime/results.json); frozen evidence branch from freshly uploaded root logs at `19290b916e0f7ece195beab9067915cf28272250`. MSVC x86 BUILD SUCCEEDED, DLL size **2,099,200 B**, old C4754 only. Stage129 3/3 installed, sid `01DD72F704247563`; **53 HEALTH**, `entry=3000`, `fallback27=0`, `returnInput=3000`, `paired=3000`, zero lost/mismatched/orphan/unknown/read-fail. **87** sampled entry/return pairs all valid. Scope43 (F3659) and scope56 (F3CC1) sampled `candidate=result=0x27` preceding P2 Stage104 MODE0 `0x27` by **94/33 QPC ticks**, same TID/player. Stage104 36 MODE0, 17 special27, 17 CEA20; Stage107 17 POST, native CEC64 reads0. Stage122 real state3 then G4 writer in same session, Stage113 gateA skip 14510, Stage115 all-false full snapshots1282, Stage110 CEBE0 entry0. Old Stage93/94 root logs not same session and excluded. **No gameplay action-commit proof or global rejection inference**. **Next Stage131:** static review of original F30B0 input candidate 0x27 supplied at F3654/F3CBC; hooks only if justified. Preserve Windows-tested Stage129 and Stage122 rollback DLLs.

---

## Stage129 — F30B0 forced-0x27 vs retained input source-only observer (2026-11-13)

**SOURCE-ONLY delivered** in conversation as `ArcanaAI_Stage129_F30B0_Forced_vs_Input_SourceOnly.zip` (30,781 bytes, SHA256 `186c587dffa51e216cd212d8b23f6bb8857b31882125477dd4a14ffbbd44d541`; 13 files, CRC PASS). This is **not a GitHub source branch**; the package is attached to the user's conversation. Baseline Stage122 adapter/module source SHA verified. The Stage129 source adds a three-site read-only opt-in provenance observer at F30B0 (6B prologue), F325A (5B true forced EAX=0x27) and F326A (5B return original candidate). Correlates entry/exit by sid/TID/unique TLS scope/EBP frame/caller/QPC; all-hit HEALTH counters and sampled atomic event pairs. Preserves original code operations once, GP/EFLAGS/FPU/SSE; refuses patched/mismatched context and in-flight F30B0 execution at install, with rollback. Stage36 OFF, no extra gameplay writes, FA480 override, F2DE0 rehook or DR writes. Original game SHA `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` verified; **13/13** original signatures, **5/5** direct relative CALLs, **54/54** nonoverlaps, **53/53** portable regression tests PASS. SYNTHETIC_ONLY fixture is **not runtime evidence**. **No MSVC compile or actual game test yet**. Next Stage130: apply guarded source to exact Windows-tested Stage122 and compile MSVC x86; then inspect fresh natural COMvsCOM health/event log. Keep existing Stage122 DLL 2,089,984 B as rollback.

---

## Stage128 — original producer F30B0 two distinct ways to return 0x27; three-site observer feasibility (2026-11-13)

[Static audit](https://github.com/Hawky11/1/blob/stage128-f30b0-fallback27-three-point-probe-design/Stage128_Design/FEASIBILITY_AUDIT.md) · [results](https://github.com/Hawky11/1/blob/stage128-f30b0-fallback27-three-point-probe-design/Stage128_Design/results.json). Original private AALib.exe SHA verified; **188/188** native anchors, **30/30** jumps, **5/5** direct relative calls, **15/15** site signatures, **54/54** non-overlap comparisons against 17 known/conservative detour envelopes, **294/294** standalone offline regressions PASS. **F325A** forces EAX=0x27, but **F326A** returns original input [EBP+0x0C], which can itself be 0x27; so prior Stage123 MODE0=0x27 does not prove the fallback ran. Stage128 proposes only 3 future **NOT INSTALLED** probes: F30B0 entry (6-byte original prologue) + F325A (5-byte MOV) + F326A (5-byte MOV/POP/POP). Two direct callers F3654 (return F3659) and F3CBC inside F3930 (return F3CC1); entry [ESP] distinguishes, and future joins require per-invocation scope/TID/frame/QPC. Complete ZIP `ArcanaAI_Stage128_F30B0_Fallback27_PositiveControl_DesignAudit.zip` **26,804 B**, SHA256 **35416a8c3c8086fa0921ef849d1271c7dc2393533280193861d5c7ab500ea142**, 19 files, CRC PASS, synthetic log only, no original EXE/DLL/patch. **STATIC DESIGN ONLY.** Next Stage129 source-only opt-in guarded producer observer if justified; Windows game run not done. Stage36 OFF, Stage122 Windows DLL 2,089,984 B preserved.

---

## Stage127 — F30B0 conditional 0x27 and F3930 CBF50 fast path (2026-11-13)

[Full original-native audit](https://github.com/Hawky11/1/blob/stage127-producer-recordflag-f30b0-cbf50-static-audit/Stage127_Analysis/FULL_STATIC_PRODUCER_AUDIT.md) · [results](https://github.com/Hawky11/1/blob/stage127-producer-recordflag-f30b0-cbf50-static-audit/Stage127_Analysis/results.json). Private original PE32 AALib.exe SHA verified; **752/752** pinned x86 instructions, **154/154** Jcc/JMP branches, **31/31** direct E8 sites (independent full .text scan vs objdump), **1,096/1,096** offline tests. F30B0 returns `0x27` at F325A **only after** its strict pointer/flag/table/subfunction prerequisites and **40+176** inactive records (stride 0x24); other paths at F326A return the input candidate, not necessarily 0x27. Producer F3930 uses `CBF50` at F3B55 with mode=1; when context+8==0, CBF50 returns AL1 on fast JNE CBF5F→CBFB2 **without testing candidate record+10**, so an AL1 is a ranking predicate, not attack accepted. F3930 only optionally calls F30B0 at F3CBC. Stage123's 25 MODE0 0x27 events **do not establish F325A ran**. ZIP `ArcanaAI_Stage127_Producer_RecordFlag_Gates_StaticAudit.zip`, 32,935 B, SHA256 eadce91cdcb8b91db701ba738d2c34438438779ec128223c3ccbb4384c42efa1 (13 files, CRC PASS). **STATIC ONLY no new hooks/DLL.** Next Stage128 positive-controlled producer branch observer feasibility only if source of 0x27 still material. Stage36 OFF, tested Stage122 DLL 2,089,984 B preserved.

---

## Stage126 — handler dispatch 236, record activity and F30B0 fallback 0x27 (2026-11-13)

[Full original-native static audit](https://github.com/Hawky11/1/blob/stage126-dispatch236-recordflag-fallback27-static-audit/Stage126_Analysis/FULL_STATIC_NATIVE_AUDIT.md) · [results](https://github.com/Hawky11/1/blob/stage126-dispatch236-recordflag-fallback27-static-audit/Stage126_Analysis/results.json). Original private AALib.exe SHA pinned. **236/236** relocatable handler table `0x7F8380` entries, **59** unique handlers; **31/31** native table xrefs, **146/146** opcode sites, **38/38** Jcc/JMP edges, **41/41** selected direct calls, **532/532** offline tests PASS. Numeric ID `0x27` maps to **CC5F0**, a bitmask predicate; `CEAD0/CEB4A` produces a per-ID predicate array, `CEBE0` may update `record+0x10` at `CF0FB` (not game hit/attack acceptance); Stage123 CEBE0 entry remained 0. **New upstream relation:** producer `F3930→F30B0`; conditional `F30B0` scans **40+176** records via `CBE00(record[+10])` and returns **EAX=0x27** at F325A if all inactive (and earlier guards pass). **Static possible**, no runtime same-invocation join to Stage123's 25 MODE0 0x27 events. ZIP `ArcanaAI_Stage126_DispatchTable_RecordActivation_StaticAudit.zip` **31,158 B**, SHA256 **4ad1dd44e7d4dedc2edd32d201210d13b711c0329b979507db25b19a9b9eee62**, 16 source/data/test files. **STATIC ONLY; no DLL, hooks or game run.** Next Stage127 original record activity / F3930/F30B0 fallback and actual gameplay consumer; preserve Windows-tested Stage122 DLL 2,089,984 B, Stage36 OFF.

---

## Stage125 — AI producer/record writer/CEA20 state sync != final action accept (2026-11-13)

[Original-native static audit](https://github.com/Hawky11/1/blob/stage125-native-command-packet-cea20-consumer-static-audit/Stage125_Analysis/FULL_NATIVE_COMMAND_STATIC_AUDIT.md) · [machine results](https://github.com/Hawky11/1/blob/stage125-native-command-packet-cea20-consumer-static-audit/Stage125_Analysis/results.json). Original AALib.exe SHA-256 verified; **149/149** anchored original instructions, **16/16** branches, **173/173** E8 calls (F2DE0 137, CEA20 18, CBDE0 12, CBF20 1, CBE60 5), **50/50** six-field xref contexts and **50/50** stdlib offline tests PASS. Source-only chat ZIP `ArcanaAI_Stage125_NativeCommandPacket_CEA20_Consumer_StaticAudit.zip` **23,966 B**, SHA256 c2627add99b5c32debffcf255128614f94fc0b6c45ab3964a403b65721a888a9 (12 files; no EXE/DLL). **F7CC0 CALL F2DE0** fills packet, F7CC5 overwrites callee EAX (no result check); original loaded F2DE0 has an unknown external E9 detour reported by Stage104. **CEA20** uses input record/null and `record+0x18` to update/reset context arrays +0x28010/+0x28018/+0x28020, clears record+0x10 and returns via RET4 without an accept boolean. **F7DA8 Stage107 POST happens BEFORE CMP [EBX+55]**, and both original special-mode0 0x27 suffix branches return AL0 (one does extra CBDE0 resets). Therefore prior Stage123 25/25 MODE0→CEA20→POST **does NOT prove native action commit/hit or rejection**. Original CEC57/CEC64 and CF0BF read the state arrays, but Stage123 native CEC64 read=0 and CF0BF is uninstrumented. Stage126 static handler dispatch VA0x7F8380, record+10 activation and actual action consumer. **STATIC ONLY, no hooks, no build**; preserve Stage122 DLL 2,089,984 B, Stage36 OFF.

---

## Stage124 — original FA480 229 callers classified, AI command-path scope verified (2026-11-13)

[Stage124 original native report](https://github.com/Hawky11/1/blob/stage124-fa480-callers-ai-action-route-static-audit/Stage124_Analysis/FULL_STATIC_CALLER_AUDIT.md) · [results](https://github.com/Hawky11/1/blob/stage124-fa480-callers-ai-action-route-static-audit/Stage124_Analysis/results.json). Reverified SHA-pinned original AALib.exe. **229** original direct FA480 callsites: **164** immediate TEST→JE, **56** immediate TEST→JNE, **9** other; **4** direct CEBE0 original sites: 988EA/98948 behind **FA480** gate 9888D, CAD79/CADDA behind **independent count -1 guard** CACF1 in CACE0. Stage94 `F7AD1 CALL F3930` producer's conditional internal `F3B19 CALL FA480` can set `[ESI+0x252]=1` only if AL1 and another global=3, but F3930 continues at false branch: **NOT native command rejection proof**. Immediate F7900-F7FFF original code has no direct FA480, although called producer does. Stage123 25/25 native CEA20 PRE→POST were executed with Stage110 CEBE0 ENTRY0. **28/28** native signature anchors, **9/9** branches and **45/45** offline tests PASS. Chat ZIP `ArcanaAI_Stage124_FA480_Caller_AIFlow_StaticAudit.zip` SHA256 **e938abc332278489b3d5f1efa99a0f611b39eed2671ebb13b33cf83a9a448b2d**, **47,993 B** / 10 files with 229-call CSV and verifier. New user-provided general log confirms Stage123 COMvsCOM at **23:48:01**, not included in prior 11-file upload. **STATIC ONLY**, no new hooks/DLL. Next Stage125 concrete AI command acceptance/commit native flow. Stage36 OFF; preserve tested Stage122 DLL 2,089,984 B and rollbacks.

---

## Stage124 — 229 verified FA480 callers; one AL0 bypass is not the entire AI (2026-11-13)

[Full original native audit](https://github.com/Hawky11/1/blob/stage124-fa480-229-callers-static-classification/Stage124_Analysis/FULL_STATIC_CALLER_AUDIT.md) · [results](https://github.com/Hawky11/1/blob/stage124-fa480-229-callers-static-classification/Stage124_Analysis/results.json). User provided fresh COMvsCOM log **2026-11-12 23:48:01**, fixing Stage123 general-log freshness. Original private AALib.exe SHA verified, independent full .text E8 scan and objdump matched **229/229** direct FA480 calls (0 discrepancies), with **164** immediate AL-zero `JE`, **56** immediate AL-nonzero `JNE`, **9** other/deferred usages; **25/25** regression tests PASS. Zero direct FA480 calls within ±0x400 of `0F7CC0` command route; site `098886` (Stage113 observed) is just one of 229, so `FA480=0` there cannot demonstrate global AI command rejection. Statically separate `0A6B33`/ `0A6C45` sites additionally check G3==2 / G3==1. User ZIP `ArcanaAI_Stage124_FA480_229Caller_OriginalNative_StaticAudit.zip` **29,473 B**, SHA256 `9508fb0f7799ef468c982fea8a93832e1b96373db8e00d49b44b6afa3e623c23`, full 229-site CSV / verifier / offline tests. **STATIC ONLY, no DLL or hooks.** Next Stage125 native action-acceptance/commit CFG. Preserve Stage122 working DLL 2,089,984 B; Stage36 OFF.

---

## Stage123 — Windows Stage122 positively observed real G4=7 writer (2026-11-12)

[Full fresh runtime audit](https://github.com/Hawky11/1/blob/stage123-positive-g4-writer-windows-runtime-audit/Stage123_Runtime/FULL_G4_WRITER_SAME_SESSION_AUDIT.md) · [machine results](https://github.com/Hawky11/1/blob/stage123-positive-g4-writer-windows-runtime-audit/Stage123_Runtime/results.json) · [11-file user upload](https://github.com/Hawky11/1/commit/4f1cc5b47d9c138d5f5bdea82bd843f7112d9e0f). Windows x86 Build.log **BUILD SUCCEEDED** DLL **2,089,984 B**. Stage122 **INSTALLED 3/3**. In one sid/TID/object, **1** original state3 writer hit `19F295` then **1** original **G4=7** slot writer hit `19F2AD` just **4 QPC ticks later**, live `selector=8, map0=1, map1=-1, f2A=1`, before G4=0; next same-object parent snapshot shows **state3, G3=3, G4=7, f2A=f2B=0**. 51 Stage122 HEALTH windows contain 10,978 parent positive-control hits, state3=1,slotG4=1,zero drops/errors; 1768 sampled parents, not 10978. Stage115 1498/1498 full snapshots `(0,0,3,7,0)` predictedAL=observedAL=0; Stage113 16490 gateA skips, CEBE0 entry0. Same-session **25/25** Stage104 MODE0 0x27→CEA20 PRE and **25/25** POST. Historic last/only writer not proven, parent same-invocation join not uniquely established due sampling. Root general ArcanaAI.log and Stage93/94 were **not freshly uploaded**, do not correlate. **Next Stage124** static FA480 caller relevance to genuine AI move resolution; Stage36 OFF; preserve Stage122 DLL 2,089,984 B, Stage115 fallback 2,082,304 B.

---

## Stage122 — guarded 3-point positive-control object writer SOURCE ONLY (2026-11-12)

[Stage122 static/source audit](https://github.com/Hawky11/1/blob/stage122-positive-controlled-object-writer-source-only/Stage122_Analysis/SOURCE_ONLY_NATIVE_AND_SOURCE_AUDIT.md) · [results](https://github.com/Hawky11/1/blob/stage122-positive-controlled-object-writer-source-only/Stage122_Analysis/results.json). Download ZIP `ArcanaAI_Stage122_PositiveControl_ObjectWriterTrace_SourceOnly.zip`, **48,883 B**, SHA256 **5d0abe6f5bab87aad38a766509ec6e67baa45bf34d16cbb52d74d5aee65426c6**, 17 files. Source overlay adds three SHA/original-byte guarded native code-only E9 detours for 1A0C81 (positive child-call control), 19F295 (object.state=3 original writer), 19F2AD (G4/G5 selected slot=7 original writer), original CALL/store instructions once, and distinct Stage122 all-hit HEALTH vs sampled EVENT logs. Exact Stage115 adapter+observer/Stage110 checksums and manifests required by PowerShell Apply/Rollback, no F2DE0 repatch/gameplay/DR writes intended, Stage36 OFF. Original private EXE SHA verified, **9/9** original signatures, **2/2** native CALL targets, **33/33** no-overlap checks, **59/59 offline regression tests** PASS. **Not Windows-built or tested live**. Future new-run hit only establishes one candidate writer, not historical or globally last writer. Next Stage123 MSVC x86 build & natural gameplay positive-control test; backup known-good Stage115 DLL **2,082,304 B**, older Stage113 2,079,744 B, Stage110 2,073,088 B.

---

## Stage121 — passive G4 writer-provenance probe feasibility DESIGN ONLY (2026-11-12)

[Audit](https://github.com/Hawky11/1/blob/stage121-passive-writer-probe-feasibility-design/Stage121_Design/FEASIBILITY_AND_POSITIVE_CONTROL_AUDIT.md) · [machine results](https://github.com/Hawky11/1/blob/stage121-passive-writer-probe-feasibility-design/Stage121_Design/results.json). Original private AALib.exe and Stage115 source SHA-verified. **30/30** exact native opcodes, **55/55** offline tests, three non-overlapping whole-instruction proposed sites: `1A0C81` positive parent-to-child CALL, `19F295` object.state=3, `19F2AD` selected G4/G5=7 store; existing hooks unchanged. Past Stage116 snapshots cannot identify historical G4 last writer; a future hit can prove a concrete writer, not exclusive last-writer coverage. Download ZIP `ArcanaAI_Stage121_Passive_WriterTrace_DesignAudit.zip` SHA256 `6f8a23e52b4b04d5d5f886cd0bbcc0fffd0e950ac5d60aa564375b3ac97ce082`, 18,862 B / 11 files. **STATIC DESIGN ONLY: no new hooks, source overlay, DLL or game run.** Stage122 guarded observer only if provenance materially advances AI diagnosis. Stage36 OFF; preserve known-good Stage115 DLL 2,082,304 B.

---

## Stage120 — original object lifecycle reveals G3 is copied BEFORE child G4=7 writer (2026-11-12)

[Stage120 full static audit](https://github.com/Hawky11/1/blob/stage120-object-lifecycle-vtable-g3-before-g4-static-audit/Stage120_Analysis/FULL_STATIC_OBJECT_LIFECYCLE_AUDIT.md) · [results](https://github.com/Hawky11/1/blob/stage120-object-lifecycle-vtable-g3-before-g4-static-audit/Stage120_Analysis/results.json). Original user private AALib.exe SHA verified. **122/122 exact native opcode sites**, **6/6** original vtable pointers, **11/11** native branches, **4/4** E8 calls, **28/28** independent opcode anchors, **55/55** stdlib regressions PASS. Original constructor `19EAD3` sets vtable VA `0x810A18` (slot+4 native `19F410` parent); `19F410` carries object in EDI, then `1A0C76/1A0C79` copies **old** `object+0x10` into **G3** BEFORE `1A0C81 CALL 19EF30` with that same object, which may set object.state=3 and G4/G5=7. The normal writer's `19F2CC→19F3D0` continuation clears both temporary flags+2A/+2B; child may early-exit when object state is already nonzero. Static possible two-step scenario: T0 object.state 0→3, G4 7 while G3 old0; T1 next same-object pre-copy produces G3=3. No observed T0/T1, no Stage116 last-writer proof. ZIP `ArcanaAI_Stage120_ObjectLifecycle_G3G4_CFG_StaticAudit.zip`, **21,834 B**, SHA256 c9927f2cabdaa4ca6fc1ba8d836800f88fb6cd7783b914bd0b152283172d9675. **STATIC ONLY**, no DLL, no hook, no game data write. Next Stage121 decide whether a narrow passive provenance probe is actually needed. Stage36 OFF; backups Stage115 2,082,304 B; Stage113 2,079,744 B; Stage110 2,073,088 B.

---

## Stage119 — FA480 selector/map/flag state producers STATIC ONLY (2026-11-12)

[Original-native source audit](https://github.com/Hawky11/1/blob/stage119-selector-map-object-flags-static-provenance/Stage119_Analysis/FULL_STATIC_PROVENANCE.md) · [results](https://github.com/Hawky11/1/blob/stage119-selector-map-object-flags-static-provenance/Stage119_Analysis/results.json). SHA-pinned user AALib.exe: **45/45** PE HIGHLOW relocated refs (selector `0E10BEDC`:29; four map slots:7/5/2/2), **42/42** unique opcode sites, **135/135** direct calls to map getter/reset/setter/swap and four-entry search, **7/7** Jcc edges, **46/46** offline tests. Native selector can reach **8** through guarded `[ESI+50]` 0..9 wrap counter (INC 7→8 or DEC 9→8); map[1] can become -1 from full-table reset, indexed `F9940(1,-1)` or swap from map0, and can later be set to 1. Object flag `[ESI+2A]=1` or `[ESI+2B]=1` comes from successful 4-entry `F565D20` search at FDF70()+0x5104, with original reset sites. **This is static possible CFG, not dynamic proof** that Stage116's G3=3/G4=7 arose at `19F2AD`. Chat ZIP `ArcanaAI_Stage119_SelectorMapFlag_StaticAudit_Clean.zip`, **22,631 B**, SHA256 **024a1259c9f95195276b3a9d90a29a4c20b276106ce3280c87f819d0c28f7e37**, 10 data/code files; no EXE/DLL. Next Stage120 object lifetime/parent CFG, then decide whether a narrow passive hit-controlled observer is warranted. Stage36 OFF. Stage115 Windows DLL 2,082,304 B preserved.

---

## Stage118 — G4=7 original indirect writer, Stage116 tuple explained as native possibility (2026-11-12)

[Detailed static CFG audit](https://github.com/Hawky11/1/blob/stage118-g3-g4-indirect-writer-cfg-static-audit/Stage118_Analysis/FULL_STATIC_CFG_AUDIT.md) · [results](https://github.com/Hawky11/1/blob/stage118-g3-g4-indirect-writer-cfg-static-audit/Stage118_Analysis/results.json). Original user AALib.exe SHA verified. **35/35** native signatures, **9/9** relocated operands, **3/3** native CALLs, **2/2** short JNEs, **31/31** offline tests PASS. Previously missing writer found: `19F2AD MOV [EDI],7` with `EDI=G4 + 0x278*([ESI+2A]==0)`. With `[ESI+2A]!=0`, native `[0x0E10BEDC]==8` and `map[1] at [0x05146284]==-1`, native code sets object `[ESI+10]=3` (RVA19F295) and **G4=7**; opposite slot can become G5=7 when 2A=0,2B!=0 and map[0]=-1. Separate `19F973` or `1A0C79` copy object state to global G3; no single instant G3+G4 write. F9900 lookup and F9920 table reset/F9940 writer verified. Stage116 sampled tuple `(0,0,3,7,0)` is **consistent**, NOT a proof this exact original writer executed; mode/map/flags were not recorded in live logs. Chat ZIP `ArcanaAI_Stage118_G3_3_G4_7_IndirectWriter_StaticAudit.zip` SHA256 c6ee16c20a0cf0b16bb45d43b032ca371bae82a668fddffe7f3885f612a058dd, 17,227 B / 8 source/docs/test files. **STATIC ONLY, no new hook/DLL/game run.** Next Stage119 native selector/map/flag writer CFG. Preserve Stage115 DLL 2,082,304 B; Stage36 OFF.

---

## Stage117 — original FA480 state writer/static xref provenance (2026-11-12)

[Stage117 full static audit](https://github.com/Hawky11/1/blob/stage117-fa480-five-globals-native-writer-static-audit/Stage117_Analysis/FULL_STATIC_NATIVE_AUDIT.md) · [machine results](https://github.com/Hawky11/1/blob/stage117-fa480-five-globals-native-writer-static-audit/Stage117_Analysis/results.json). Original private AALib.exe SHA verified; **371/371 PE HIGHLOW relocated absolute .text references** to the five globals, **34 direct store addresses**, **18 original opcode signatures**, **21/21 stdlib tests** PASS. Xrefs G1/G2/G3/G4/G5: **29/9/148/136/49**, direct stores **13/2/11/7/1**; not exhaustive of computed-pointer/memset/rep-mov writes. **G4/G5 are adjacent 0x278-byte record heads**; F97E0 resets both (0x4F0), F9830 resets indexed record, F99A0 block-swaps and sets G3=1/2. `1F44D0` can produce G1=8 or 14, while native `19FA42` positive helper branch sets G3=1 then G4=4, one valid native FA480-positive combination. **Observed Stage116 G3=3/G4=7 still lack precise last-writer proof**; no native mode name assigned. **229** FA480 direct callers, Stage113 watched only one. Chat ZIP `ArcanaAI_Stage117_FA480_GlobalWriter_StaticAudit.zip` SHA256 bf9cef7f76c877f20e150892134f27ff41043290d1165833798d891b561e8fd8 (22,541 B, 8 files). **STATIC ONLY; no new DLL or hooks.** Next Stage118 source CFG/indirect writer provenance. Stage36 OFF; backups Stage115 2,082,304 B, Stage113 2,079,744 B.

---

## Stage116 — real FA480 global predicates captured (2026-11-12)

[Full Stage116 audit](https://github.com/Hawky11/1/blob/stage116-fa480-five-globals-runtime-verified/Stage116_Runtime/FULL_SAME_RUN_AUDIT.md) · [machine results](https://github.com/Hawky11/1/blob/stage116-fa480-five-globals-runtime-verified/Stage116_Runtime/results.json) · [fresh eight-file upload](https://github.com/Hawky11/1/commit/ad3a541b9eeced167901d6f82e2617d0485c67fa). MSVC x86 **BUILD SUCCEEDED** (Stage115 DLL **2,082,304 bytes**); Stage115 piggyback installed with no added hooks; Stage113 **4/4**. All **1,455/1,455** sampled post-FA480 5-DWORD snapshots full/readMask=1F, same tuple **(0,0,3,7,0)** on 670 P1 / 785 P2, all native pred1/2/3 false, real AL=0/ZF=1, zero prediction differences; exact QPC matches to 1,455 Stage113 gate records. Across **52** clean Stage113 HEALTH snapshots, **16,760/16,760** parentA+gateA calls skip toward 98965, gatePass=0; parentB=0, Stage110 CEBE0=0, Stage107 READ=0. Same-run **24/24** Stage104 MODE0 0x27→CEA20 and **24/24** CEA20→POST, zero errors. Root Stage94/Stage93 are older sessions (TID5480/12204), **NOT** same-run Stage116 (TID10288): no fresh joins claimed. Static meanings of global values remain unknown. Next Stage117 static writer/xref analysis of five globals; preserve Stage115 DLL **2,082,304 B**, Stage113 fallback 2,079,744 B; Stage36 OFF.

---

## Stage115 — FA480 global predicates piggyback observer SOURCE ONLY (2026-11-12)

[Stage115 audit](https://github.com/Hawky11/1/blob/stage115-fa480-global-snapshot-source-only/Stage115_Analysis/SOURCE_ONLY_AUDIT.md) · [results](https://github.com/Hawky11/1/blob/stage115-fa480-global-snapshot-source-only/Stage115_Analysis/results.json). Chat ZIP `ArcanaAI_Stage115_FA480_Globals_SourceOnly.zip`, SHA256 e77ad4f0a54774439c3b687423d2d1b947dfdee2d8ec269a120acfdab8328efe. Based on Windows-built Stage113 with C2668 fix, extends **existing** hook 9888D only: five ASLR-relocated original global DWORD snapshots, prediction versus real AL/ZF, separate sampled Stage115 log, original Stage113 four hook assembly untouched. **42/42 static/source checks**, **27/27 offline tests** PASS, **no Windows build or game-run**. Exact SHA-guarded Apply/Rollback modifies one source file; does not patch F2DE0, create new hook or write DR/gameplay data. Stage36 OFF. Next Stage116 Windows x86 build and natural match, require positive Stage115 snapshots and clean health. Preserve Stage113 DLL 2,079,744 B and Stage110 DLL 2,073,088 B.

---

## Stage114 — Win32 Stage113 observed 15,726 native FA480 gate skips (2026-11-12)

[Full audit](https://github.com/Hawky11/1/blob/stage114-fa480-zero-parent-gate-runtime-audit/Stage114_Runtime/FULL_SAME_RUN_AUDIT.md) · [machine counts](https://github.com/Hawky11/1/blob/stage114-fa480-zero-parent-gate-runtime-audit/Stage114_Runtime/results.json) · [raw upload](https://github.com/Hawky11/1/commit/0a990a8801bd5538c482aa4f6572c633217f05ca). User fixed Stage113 C2668 and Windows x86 **BUILD SUCCEEDED**, DLL **2,079,744 B**. Two runs appended; first Stage113 disabled, second **INSTALLED 4/4**, TID12204, 46 clean HEALTH windows. Observed **15,726 parentA+gateA**, all 15,726 **FA480=0/JE skip**, no pass; parentB/gateB=0. 2,734 event rows are deliberately sampled, not missing hits. Stage113 `P0` label derives from nested actor pointer base+0x154, not an actual third side. Same-run Stage94 212 gates: 173/173 FinalCommand, 39/39 MODE0; 18/18 MODE0 0x27 → CEA20 → Stage107 POST, all unique. CEBE0 entry/CEC64 read still 0. Original FA480 boolean gates traced statically to 1F4700 and five global fields, but actual runtime globals **not recorded**. No Stage114 DLL or code change, no gameplay/hit/AI improvement proved. Stage36 OFF; next Stage115 bounded read-only global snapshot piggybacking Stage113 9888D. Preserve Stage113 DLL and older rollbacks.

---

## Stage111 — Windows Stage110 installed, full 214/214 gate joins, CEBE0 ENTRY zero (2026-11-12)

[Detailed audit](https://github.com/Hawky11/1/blob/stage111-cebe0-entry-zero-full-runtime-audit/Stage111_Runtime/FULL_SAME_RUN_AUDIT.md) · [machine results](https://github.com/Hawky11/1/blob/stage111-cebe0-entry-zero-full-runtime-audit/Stage111_Runtime/results.json) · [fresh 11-file user upload](https://github.com/Hawky11/1/commit/3ed706cb9be5e9d13f9202c0606080f304ba5070). Win32 MSVC build succeeded DLL 2,073,088 bytes, Stage104/107/110 installed 2/2, 47 clean HEALTH snapshots per Stage94/104/107/110. Stage94 2044 producer events and 214 gate decisions; all 171 forwardType1 match Stage93 FinalCommand and all 43 nonforward match Stage104 MODE0 uniquely in the same session. 20 chosen ESI=0x27 gates match 20 MODE0 0x27, 20 CEA20 PRE and all **20 Stage107 POST** uniquely; P1 7/P2 13, 4–15 QPC ticks median7, all valid index0, v10=v18, v20=0xA. 16/20 gate 0x27 immediate F3930 producer candidate matches, four not attributable from supplied logs. Stage110 entry=0, guard=0 and Stage107 native CEC64 READ=0 throughout; no evidence of CEBE0 invocation nor of a specific early guard. Hook installed≠positive entry hit. Five ancillary stage traces empty; fresh INI/general ArcanaAI.log not part of upload. No data/DR writes, forced actions, accepted attack/hit/AI improvement evidence. Next Stage112 original CBDE0 CFG/downstream and CEBE0 direct caller reachability (988EA/98948/CAD79/CADDA), with optional passive guarded diagnostics only after static check. Stage36 OFF; preserve 2,073,088-byte Stage110 DLL.

---

## Stage110 — CEBE0 entry and CEC57 activity gate read-only source ready (2026-11-12)

[Stage110 report](https://github.com/Hawky11/1/blob/stage110-cebe0-entry-active-guard-source-only/Stage110_Analysis/STATIC_AND_SOURCE_ANALYSIS.md) · [results](https://github.com/Hawky11/1/blob/stage110-cebe0-entry-active-guard-source-only/Stage110_Analysis/results.json). User original AALib.exe SHA-verified, 56/56 prior native +4/4 additional native, 69/69 source contracts, 42/42 offline tests PASS. Two new passive code redirects at CEBE0 entry and native CEC57 guard preserve original instructions once and branch flags, require installed exact Stage107/104 ownership, opt-in disabled by default, no preexisting F2DE0 detour changes. Log counter classes actual entry and actual CEC57 guard, bounded sampled ring; early entry classification only predicts branches, not proven execution. Source ZIP `ArcanaAI_Stage110_CEBE0_Entry_Guard_SourceOnly.zip` SHA256 `3dbde3bf24fe10140e0948f1263a06d8be876870da9ccf147f0b4b61989ccecd` (45,460 B, 15 files), exact SHA-guarded Apply/Rollback, Russian instructions and Python tests delivered in chat. **NO Windows Stage110 build, DLL, new runtime events, gameplay/DR writes, AI/hit improvements proven.** Stage36 OFF. Next Stage111 Windows x86 compile + same-run Stage104/107/110 logs; tested fallback Stage107 2,065,920-byte DLL unchanged.

---

## Stage109 — CEBE0 native control-flow guard and caller STATIC verified (2026-11-12)

[Stage109 report](https://github.com/Hawky11/1/blob/stage109-cebe0-guard-cfg-static-audit/Stage109_Analysis/STATIC_GUARD_CFG_AUDIT.md) · [results](https://github.com/Hawky11/1/blob/stage109-cebe0-guard-cfg-static-audit/Stage109_Analysis/results.json). Original private AALib.exe SHA verified, **56/56 opcode/call/rel-edge/table checks** and **26/26 offline guard tests PASS**. CEBE0 pre-CEC64 may early-exit for ID -1, unsigned >0x1EE, first table byte 0xFF; may skip CEC64 when active [record+0x10]==0, including after optional active decrement via fourth argument. Native 0x27 table record starts byte0=0x0C (valid), but Stage104 ESI 0x27 != proven CEBE0 ID. Exactly four original direct E8 rel32 callsites 988EA/98948/CAD79/CADDA; reader called from other functions, while immediate F7DA3 CEA20 continuation invokes CBDE0 or returns, not CEBE0. Stage108 13/13 CEA20 PRE→POST and 0 native CEC64 READ preserved; true runtime CEBE0 entry/guard unknown. Read-only archive Stage109 SHA256 2dd01072437eda1e82b77e1038b77865a6ab4f9cf8c6484cc4b34583120f1c05 delivered in chat, no private executable/log/DLL. **No new Windows build, game run, hook, DR, game-data writes, force commands, accepted action/hit/AI improvement proof**. Stage36 OFF; Stage107 Windows DLL 2,065,920 B unchanged. Next Stage110 only after passive entry observer ownership/ABI/rollback audit.

---

## Stage108 — CEA20 PRE→POST 13/13 runtime confirmed, CEC64 reads absent (2026-11-12)

[Full Stage108 analysis](https://github.com/Hawky11/1/tree/stage108-cea20-post-runtime-verified-read-unobserved/Stage108_Runtime) · [raw user upload db7a996](https://github.com/Hawky11/1/commit/db7a9967a8f3ac6c85d0fc2072c96638a2a46e65). Windows Stage107 build succeeded, DLL 2,065,920 bytes, known C4754. Stage107 installed 2/2, Stage104 installed 2/2, Stage36 OFF. Fresh same-run Stage104 29 MODE0, 13 CEA20; Stage107 13 POST, 0 nativeRead/READ_CEC64. All 13 CEA20 PRE match unique POST by actor/TID/frame/ESI/state/QPC, 4–58 ticks (median5), P1=5 P2=8. 43 HEALTH each clean: dropped/mismatch/readFail/filtered=0. Reader counter increments before filters so no native CEC64 execution observed. POST state index0, `v10=v18`, `v20=0xA`; POST `record=0` unpopulated field, PRE arg0 nonzero. 12 PRE arg0+0x930, one **P1** +0x954, unrelated to previous-run P2 class5 absent same-session producer evidence. No demonstrated CEBE0 entry/accepted action/animation/hit/AI quality. Next Stage109 static entry/branch/caller audit, no new writes/hook yet. Keep tested Stage107 DLL 2,065,920 B and Stage104 r2 fallback 2,058,240 B.

---

## Stage107 — read-only CEA20 post-return + CEBE0 actual indexed read SOURCE ONLY (2026-11-12)

[Stage107 immutable branch](https://github.com/Hawky11/1/tree/stage107-cea20-post-cebe0-read-source-only/Stage107_Analysis) · [report](https://github.com/Hawky11/1/blob/stage107-cea20-post-cebe0-read-source-only/Stage107_Analysis/STATIC_AND_SOURCE_ANALYSIS.md). Exact original SHA checked, 23/23 PE/native and 23/23 source guards PASS, 21/21 offline negative/parser tests PASS. Read-only observation at **F7DA8** (native CMP/JE replay one time; state after CEA20) and **CEC64** (actual native field20 CMP read; original flags kept for branch). New hook opt-in (default off) installed only after Stage104 r2 exact ownership; F2DE0 preexisting detour untouched. Stage104 PRE→POST shares F7150 EBP, while nested CEBE0 has distinct stack EBP; reader joins on state/index/TID/QPC/parentPost, not identical frame. Source archive `ArcanaAI_Stage107_CEA20_Post_CEBE0_Read_SourceOnly.zip` SHA256 `fec9d0597d72aa7947686dee7e44cdd9f7f1f3d37efc745555845dda70e032ee` delivered in chat with exact SHA installer/rollback; no EXE, DLL or raw logs. **NO Windows Stage107 build/game run or CEBE0 consumption observed**, no forced actions, no game data/DR writes, Stage36 OFF. Next Stage108 user Windows build and natural COM-vs-COM runtime logs; keep latest verified Stage104 r2 DLL 2,058,240 B.

---

## Stage106 — native CEA20 shared-state writer → CEBE0 reader STATIC verified (2026-11-12)

[Audit and results](https://github.com/Hawky11/1/tree/stage106-cea20-cebe0-static-consumer-audit/Stage106_Analysis). Original AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`; 37/37 exact native checks and 8/8 offline tests PASS. F7DA3 → CEA20 writes state+0x28010/18/20, CEBE0 reads corresponding arrays, with exact global base 0433C138/stride 0x2806C shared by verified callers. Stage105 Stage104 log has 17 valid CEA20 0x27 PRE-call records: 16 arg0=player+0x930, one P2 seq44 arg0=player+0x954 (natural class5 special 0x27). **No proof of same-run CEBE0 consumption, return-time store, attack animation/hit, AI improvement, or game-wide stability.** No new DLL/hook/Windows run, game/DR writes; Stage36 OFF. Source-only audit ZIP SHA256 `c1a889b2b1ba3b148ac9d73810c656b08966fb727c5830d88633204190d2ef62` available in ChatGPT. Next Stage107 only if a guarded read-only post-call consumer witness is truly needed.

---

## Stage105 — native 0x27 Mode0/CEA20 runtime proof + first class5 F34D0 `FFFFFFFF→0x27` (2026-11-12)

[Stage105 independent evidence snapshot](https://github.com/Hawky11/1/tree/stage105-mode0-27-runtime-confirmed/Stage105_Runtime) · [complete same-run analysis](https://github.com/Hawky11/1/blob/stage105-mode0-27-runtime-confirmed/Stage105_Runtime/RUNTIME_ANALYSIS.md) · [JSON](https://github.com/Hawky11/1/blob/stage105-mode0-27-runtime-confirmed/Stage105_Runtime/results.json), inherits untouched new user [main upload 24885c9](https://github.com/Hawky11/1/commit/24885c90922251cd065ae812c7f1d30b000c7b66). User Windows MSVC x86 14.51.36231 build **SUCCEEDED**, dinput8.dll 2,058,240 B, single preexisting C4754. Stage104 r2 preserved preexisting detour `F2DE0 E9_REL32_NOP`, targetVA `78E3F920` and `STAGE104_INSTALLED 2/2`. Exact new runtime: **245 Stage94 gates**, 199 ECX1 + 46 ECX0. Exact actor/TID/frame/QPC/payload one-to-one join: **199/199 mode1 Stage93 FinalCommands**, **46/46 mode0 Stage104 F7CC0**; no ambiguity/missing/reuse. 46 mode0 contain **17 ESI=0x27**, all **17/17** followed original F7DA3→CEA20 with 7–82 QPC ticks (median13) and no invalid record. Other 29 mode0 payloads: 0E12/10 8/26 4/25 2/20 3. 17 x 0x27 origins: 15 Stage94 F3930 producer, two Stage95 bypass. First bypass native Stage96 F79B6 P1 `FFFFFFFF→0x27` followed class3 route1095, bypass826, gate287, MODE0 seq5, CEA20 seq6. Second breakthrough: Stage96 **F7A8F class5** returned `FFFFFFFF→0x27` for P2 **once in 479 class5** (other returns FFFFFFFF476, inherited 0x60 once, inherited 0x22 once). Full chain Stage98 route12768 → Stage96 writer14812 → Stage95 bypass11195 → Stage94 gate1755 → Stage104 MODE0 seq43 → CEA20 seq44. Original Stage101 461 unchanged still describes DIFFERENT prior run; no contradiction. Six HEALTH streams Stage104, Stage94/95/96/97/98 each have 55 clean snapshots (dropped/mismatch/readFail=0). Source file hashes, provenance, limits in report; larger Git blobs were fetched complete, not interpreted as empty. **No analyst new DLL/hook or gameplay/DR writes, Stage36 OFF; no demonstrated attack hit, animation, combat improvement or game-wide stability.** Stage104 r2 latest runtime-tested binary 2058240 B; Stage98 r2 rollback 2051584 B, Stage97 2045440 B. **Next Stage106:** consider original CEA20 downstream effects/consumer, only passive/no game writes.

---

## Stage104 r2 — r1 Windows build success, fail-closed runtime preflight on preexisting F2DE0 detour (2026-11-12)

[Stage104 r2 immutable analysis](https://github.com/Hawky11/1/tree/stage104-r2-f2de0-detour-compat-source/Stage104_R2_Analysis) · [report](https://github.com/Hawky11/1/blob/stage104-r2-f2de0-detour-compat-source/Stage104_R2_Analysis/R1_FAILURE_AND_R2_SOURCE_FIX.md) · [JSON](https://github.com/Hawky11/1/blob/stage104-r2-f2de0-detour-compat-source/Stage104_R2_Analysis/results.json). User's fresh 2026-11-12 17:19 Windows MSVC x86 `Build(1).log` shows **BUILD SUCCEEDED**, 2,057,728-byte dinput8.dll and previous C4754. Stage104 r1 trace contains `STAGE104_BUILD ... r1`, `STAGE104_SIGFAIL region=CALLEE_F2DE0 expected=558BEC8B450880780F00752F actual=E94BC9C4789080780F00752F`, `STAGE104_ERROR ... no patch` and **no installed/Mode0/CEA20 records**. The first 6 original F2DE0 bytes were replaced with **E9 rel32+NOP**, original bytes 6..11 unchanged. Prior detour owner cannot be identified from limited logs. R1 correctly refused to patch. Stage104 r2 **source-only one-file fix** permits original entry or narrowly recognized E9+NOP prior detour with original tail and resolved committed executable target (`VirtualQuery`, no guard/noaccess), leaves all original callsite/CEA20/Stage94/98 signatures strictly guarded, preserves existing callee detour by tail-JMP to unchanged F2DE0 entry. SHA guarded incremental apply/rollback only on exact r1 installation, game closed; r1 source/evidence unchanged. Original private AALib.exe SHA `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`: native checks **39/39**, r2 source **36/36**, offline tests **35/35** PASS. Chat archive `ArcanaAI_Stage104_r2_F2DE0_Existing_Detour_Compatibility_SourceOnly.zip` SHA256 `8a0c83f49117eccabc815102000959f22a22ef3aa2b707579136e00f0d89b7a0`, no executable or raw logs inside. **No r2 Windows build, installed hook, game run or AI improvement proven**. Tested rollback Stage98 r2 DLL 2,051,584 B, Stage97 2,045,440 B; Stage36 OFF. **NEXT Stage105:** build and normal capture verifying `STAGE104_INSTALLED 2/2` and real F7CC0/F7DA3 events, then same-run strict QPC correlation.

---

## Stage104 — original mode-0 and CEA20 passive observer prepared SOURCE ONLY (2026-11-12)

[Stage104 independent analysis branch](https://github.com/Hawky11/1/tree/stage104-mode0-f2de0-cea20-source-ready/Stage104_Analysis) · [report](https://github.com/Hawky11/1/blob/stage104-mode0-f2de0-cea20-source-ready/Stage104_Analysis/STATIC_AND_SOURCE_ANALYSIS.md) · [machine JSON](https://github.com/Hawky11/1/blob/stage104-mode0-f2de0-cea20-source-ready/Stage104_Analysis/results.json). Full original AALib.exe SHA `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` checked; **39/39** native opcode/call/CFG/prologue guards PASS; **25/25** source contracts and **25/25** offline tests PASS. Stage102 real 15 0x27/ECX0 gate samples are not represented by Stage93 F7C5D mode-1 FinalCommand. Two passive original 5-byte native CALL redirects prepared at **F7CC0→F2DE0(mode0)** and **F7DA3→CEA20**. At F7CC0 the original code has shifted EDI by +3A2C; observer recovers original P by subtracting this offset. Native F2DE0 writes a mode/code record, not proof of attack/hit. Trampolines preserve EFLAGS/GPR/FPU, original stack/return via native tail-JMP original-once, with bounded ring and existing worker flush. Exact source SHA-fail-closed, game fingerprint, original target prologues and byte signatures, thread-suspend/mutex/rollback, no F7C5D collision or DR owner, no game/AI writes. Chat deliverable `ArcanaAI_Stage104_Mode0_CEA20_Passive_SourceOnly.zip` SHA256 `d55f025ee38c4b066c37a076f086654d63006a75f16940d9702367fe8ed51b7e`; original game binary excluded. **NOT COMPILED ON WINDOWS, NOT INSTALLED/TESTED IN GAME, NO NEW LIVE LOGS, NO CLAIMED AI/HIT IMPROVEMENT.** Stage36 OFF. Last tested Stage98 r2 DLL **2,051,584 B**, fallback Stage97 **2,045,440 B**. **NEXT Stage105:** build and short standard COM-vs-COM test on user Windows, same-run Stage104/94/93/98 + Build.log; only then strict native mode0 evidence analysis. Preserve all prior stage branches unchanged.

---

## Stage103 — original status-0 0x27 route uses F2DE0 mode 0, not Stage93 type-1 FinalCommand (2026-11-12)

[Immutable Stage103 analysis](https://github.com/Hawky11/1/tree/stage103-native-27-status0-mode0-route-analysis/Stage103_Analysis) · [native CFG report](https://github.com/Hawky11/1/blob/stage103-native-27-status0-mode0-route-analysis/Stage103_Analysis/STATIC_AND_RUNTIME_ANALYSIS.md) · [results JSON](https://github.com/Hawky11/1/blob/stage103-native-27-status0-mode0-route-analysis/Stage103_Analysis/results.json). **Corrective finding relative to Stage102:** its observed 15/15 ECX0/forwardType1=0 “rejected” gates are **only rejected from the mode-1 F7C5D CALL F2DE0 path**. Original SHA-pinned AALib.exe disassembly proves F7ADB reads caller `[EBP-8]` output status into ECX, F7B07 CMP 1 and F7B0A JNE F7C68. For **status0 + ESI=0x27** the native F7C72/F7C75 special case reaches F7CAE→E9800, F7CB6 PUSH ESI / F7CB7 PUSH **0** / **F7CC0 CALL F2DE0**, then status0/ESI27 path at F7D96/F7D9A reaches **F7DA3 CALL CEA20**. The Stage93 observer at F7C5D monitors the mode-**1** CALL only; zero 0x27 FinalCommand events do not prove no original native handling. F3930 directly writes *status=0 at F3C3E (contrasted with -1 at F3CE1); F34D0 writes *status=0 at F358E/F360B; F30B0 0x27 return F325A writes status0 at F3254. Specific internal F3930/F34D0 branches for all observed events remain unlogged. Re-read complete Stage94 blob `fba1df695c4fcac0af357c782cf521af24b91b93`: **201 gates, 167 status1/type1, 34 status0/no-type1**, of which **15 ESI27** (12 producers, 3 bypass), remaining **19** (0E=5,10=8,20=1,25=4,26=1). Original AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` independently checked; **37/37 byte+branch contracts**, **12/12 offline tests** PASS. ZIP `ArcanaAI_Stage103_NativeStatus0_0x27_StaticAudit.zip` SHA256 `1c65ddd6f62be935d07b2738b6fe4075f0f9dc4e030a74e75c4f16151f1ac6c4`, includes verifier/report/tests, excludes EXE/logs. **No new game/Windows build, no runtime confirmation of F7CC0/CEA20 execution, no new hooks/DR/game writes, no AI improvement. Stage36 OFF.** Last tested Stage98 r2 DLL 2,051,584 bytes; rollback Stage97 2,045,440. **NEXT Stage104:** seek mode0 native runtime evidence from existing tracing before designing minimal passive observation.

---

## Stage102 — natural special ESI 0x27 exists, but 15 native gates reject (2026-11-12)

[Immutable Stage102 branch](https://github.com/Hawky11/1/tree/stage102-f34d0-0x27-native-gate-evidence/Stage102_Analysis) · [native/runtime analysis](https://github.com/Hawky11/1/blob/stage102-f34d0-0x27-native-gate-evidence/Stage102_Analysis/STATIC_AND_RUNTIME_ANALYSIS.md) · [results JSON](https://github.com/Hawky11/1/blob/stage102-f34d0-0x27-native-gate-evidence/Stage102_Analysis/results.json). Exact private original EXE SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` checked; full function SHA and opcode/branch/relative E8 calls, quantizer F5210 (/17000), action table VA 04C7AE51 indexes 0 and 0x22 both 0x27: **35/35 PASS**. Offline synthetic tests **19/19 PASS**. Full five original GitHub blobs from user Stage98 r2 [8919f8e](https://github.com/Hawky11/1/commit/8919f8ec242f1137c447d2eb4afc7ba0ed0c0e1e) strictly joined by actor/TID/frame/QPC. Stage96 F79B6 early F34D0 return wrote **three** `FFFFFFFF→0x27` results among 1813 calls, unlike Stage101 461 class5 F7A8F all unchanged. Three fully correlated chains **975→976→977→806→608→209** (P1), **6076→6077→6078→4630→4085→612** (P2), **15102→15103→15104→11658→10356→1468** (P2). All retained code0x27 through F79D4/F79F0, Stage98 class3, Stage95 bypass and Stage94 gate but were **rejected** by original gate ECX0/forwardType1=0. Exhaustive Stage94 201-gate review revealed **15** chosenESI0x27: **12** unique immediate F3930 producer sources, **3** bypass sources, all 15 ECX0 and rejected. All 167 original Stage93 FinalCommand payloads exclude 0x27. Static native F34D0 still has global guard, E6960/F5210 reciprocal integer quantizer, actor+264/+265 special slots, CBF50 and F30B0 helper conditions; **exact reason 461 class5 returns unchanged UNKNOWN** because intermediate states were not logged. Action table entries 0/0x22 are 0x27 and cannot be claimed as gate failure. No new DLL, native probe, run, game/DR write, Stage36 stays OFF. Stage102 chat ZIP `ArcanaAI_Stage102_F34D0_0x27_Native_Gate_Audit.zip` SHA256 `d9645e9eebce860a69db8875ec0b648e032ce288e362850576063af712c42b62` (offline verifier, five-log correlator, 19 tests; original EXE excluded). Preserve tested Stage98 r2 DLL 2051584 B and Stage97 fallback 2045440 B. **NEXT Stage103:** native ECX source upstream of F7B0A without forcing game commands or injecting redundant hooks. No gameplay hit/AI improvement proven.

---

## Stage101 — LIVE-source full-log evidence closes F34D0 class5 return provenance (2026-11-12)

[Stage101 immutable branch](https://github.com/Hawky11/1/tree/stage101-real-class5-return-full-log-analysis/Stage101_Analysis), [full offline analysis](https://github.com/Hawky11/1/blob/stage101-real-class5-return-full-log-analysis/Stage101_Analysis/RUNTIME_LOG_ANALYSIS.md) and [machine data](https://github.com/Hawky11/1/blob/stage101-real-class5-return-full-log-analysis/Stage101_Analysis/results.json), using **existing** Stage98 r2 Windows raw logs [user upload 8919f8e](https://github.com/Hawky11/1/commit/8919f8ec242f1137c447d2eb4afc7ba0ed0c0e1e). Previous large GitHub `fetch_file` limitation was resolved by reading all five full text blobs using exact Git blob SHA. Stage98 route 14,107, Stage96 post-F34D0 F7A8F writer 461, Stage95 bypass 12,474, Stage94 producer 1,633, gates 201, Stage93 real final 167. Strict actor/TID/frame/QPC and next-route bounding matched **461/461** class5 routes uniquely to Stage96 F7A8F and Stage95 F7A94 (no missing/ambiguous/reused results), with **all 461 returns unchanged** `preEsi=resultEax=nativeNextEsi=chosenEsi`: 459 FFFFFFFF and two finite 0x29. **No observed 0x27 replacement.** All class5 modes 0 (208) or 1 (253), not mode2. P1 finite `0x29` chains `Stage98 7833→Stage96 10228→Stage95 6897→Stage94 1050→Stage93 93` and `8665→11036→7668→1122→103`, both real forward type1 payload 0x29, not generated by F34D0 changing ESI. 459 other class5 bypasses did not reach the observed Stage94 gate. QPC lag route→writer 4–75 (median14), writer→bypass 1–168 (median2). The Stage100 original-code return domain {inherited ESI,0x27} is still structurally possible; the special result was not exercised in this run. Stage101 chat ZIP contains portable 5-log correlator + 10/10 synthetic tests, original binary and raw logs omitted. No new game run or hook/DLL/game/DR write, Stage36 OFF, no causal hit or improved-AI claim; Windows-tested Stage98 r2 2,051,584 B remains working. Next Stage102 only a specific new natural changed-EAX/mode2 or guarded-static question, not another redundant F7A8F/F7A94 hook.

---

## Stage100 — original F34D0/F30B0 return-value domain proved STATIC ONLY (2026-11-12)

[New stage100-f34d0-return-provenance-static branch](https://github.com/Hawky11/1/tree/stage100-f34d0-return-provenance-static/Stage100_Analysis) · [full analysis](https://github.com/Hawky11/1/blob/stage100-f34d0-return-provenance-static/Stage100_Analysis/STATIC_ANALYSIS.md) · [results](https://github.com/Hawky11/1/blob/stage100-f34d0-return-provenance-static/Stage100_Analysis/results.json). Original user Library AALib.zip opened and AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` verified. Native F7A8A→F34D0, F34D0→F30B0 and F7A8F `MOV ESI,EAX` checked. F34D0 either returns its incoming ESI unchanged at F3663 or, through F3579/F35F9/F30B0, can return `0x27`; F30B0 has a special `0x27` return F325A and input pass-through at F326A. Output parameters and other native helpers may have gameplay side effects; only the return-value *domain* is established. PE/full-native SHA checks **15/15**, synthetic tests **8/8**, 120 synthetic class5 event joins and 270 model cases PASS. No Stage100 Windows DLL/build/run/native hook, no new game/DR writes or forced AI; Stage36 OFF. The downloadable ZIP `ArcanaAI_Stage100_F34D0_ReturnProvenance_StaticAudit.zip` includes Python stdlib-only original binary audit and strict Stage98/96/95/94 correlator; ZIP SHA `0377ee968afd78152a76490fd0b3c7c064a04039e4413a10b6ea98cf7bb889ca`. Full raw logs (3–5MB each) visible in GitHub could not be read via the connector in this session, so no real Stage100 return-to-0x27 counts are asserted. Stage98 r2 **461 class5 bypass mode0/1** still only establishes that F34D0 was required by original control flow; final exact returns await Stage101 real log joins. Preserve Stage98 r2 tested DLL 2,051,584 bytes, Stage97 fallback 2,045,440 bytes, history branches untouched.

---

## Stage99 — class5 late routing resolved by original-code static proof (2026-11-12)

**New immutable Stage99 branch:** [stage99-class5-static-route-proof](https://github.com/Hawky11/1/tree/stage99-class5-static-route-proof/Stage99_Analysis), [technical report](https://github.com/Hawky11/1/blob/stage99-class5-static-route-proof/Stage99_Analysis/STATIC_ANALYSIS.md) and [machine summary](https://github.com/Hawky11/1/blob/stage99-class5-static-route-proof/Stage99_Analysis/results.json). Recovered original user-Library AALib.zip, verified AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, independently decoded native F7A49..F7A97 and 525CD0. **10 native byte-region/direct-edge checks passed; 100 synthetic tests passed**. The entire class5 subgraph terminates at Stage95 F7A94 bypass (never F7A99 producer). `mode3A34==2` is sole entry to helper 525CD0; its AL=0 iff signed code in {0,1,2,9,10}. With any other mode or zero helper result, F34D0 is called, replacing ESI via Stage96's existing F7A8F observer. F34D0 argument is 0x22 iff P+3B70 bit2, else zero. Existing Stage98 r2 class5 461/461 bypasses had modes 0=208, 1=253, so all **461** statically imply F34D0 call / helper skipped; those are inferences from static flow plus Stage98 *pre*-branch logs, not new execution measurements. Prior Stage98 complete **14,107/14,107** route joins remain unchanged; existing live DLL **2,051,584 B** remains the last Windows build. No Stage99 DLL, hook, gameplay change, synthetic command, or proof of hit/AI improvement; Stage36 OFF. Next Stage100 only if justified: inspect F34D0 return ESI / natural mode2 using already-installed Stage96 and Stage95 observers, not broad instrumentation. Stage99 offline scripts and Russian guide are packaged in a ZIP delivered in ChatGPT.

---

## Stage98 r2 LIVE — TID logger fixed, all early native routes matched (2026-11-12)

[New user log upload `8919f8e`](https://github.com/Hawky11/1/commit/8919f8ec242f1137c447d2eb4afc7ba0ed0c0e1e) · [full report](Stage98_R2_Runtime/RUNTIME_ANALYSIS.md) · [machine counts](Stage98_R2_Runtime/results.json) · [r2 source/rollback](https://github.com/Hawky11/1/tree/stage98-r2-f7a00-tid-logger-fix-source-only/Stage98_R2_TID_Fix). MSVC x86 BUILD SUCCEEDED, DLL **2,051,584 B**, Stage98 1/1 alongside Stage97 1/1, Stage96 5/5, Stage95 2/2, Stage94 2/2; all stages have **49** HEALTH snapshots with zero drops/errors. Source fix successfully restores real Stage98 TID **11260 on 14,107/14,107** routes, matching all companion native events. Strict player/TID/frame/QPC join: **14,107/14,107** unique outcomes (1,633 producer, 12,474 bypass). Class1 producer 1,633/1,633; class3 bypass 12,013/12,013; **13,646/13,646** early predicted routes accurate; class5 461 observed bypass but helper/predicate remains unresolved. Stage94 201 gates =151 producer +50 finite bypass, 167 forward /34 reject; **167/167** forward gates match same-session Stage93 FinalCommand. Last tested Windows DLL now Stage98 r2, Stage97 fallback; no AI improvement claim, Stage36 OFF. Next focus Stage99 class5 late predicates and `525CD0`.

---

## Stage98 r1 LIVE tested / exact F7A00 routing; r2 TID hotfix prepared (2026-11-12)

[User Stage98 runtime upload](https://github.com/Hawky11/1/commit/aace9cd5b4eb001ddddfdb338fd8416471e3b0a9) · [report](Stage98_Runtime/RUNTIME_ANALYSIS.md) · [results JSON](Stage98_Runtime/results.json) · [r2 corrective source and rollback](https://github.com/Hawky11/1/tree/stage98-r2-f7a00-tid-logger-fix-source-only/Stage98_R2_TID_Fix). Stage98 r1 built MSVC x86 2,051,584 B; F7A00 hook **INSTALLED 1/1**, Stage94/95/96/97 coexist, 49 HEALTH snapshots each stage report no dropped/mismatch/readFail. **13,926 ROUTE** exact one-to-one outcomes against **1,894 Stage94 PRODUCER** and **12,032 Stage95 BYPASS** when reconstructing unique single-thread QPC context. class1 → producer 1,894/1,894, class3 → bypass 11,335/11,335; class5 697 → bypass observed but native late predicate unresolved. **13,229/13,229 predicted early routes match**, no conflicts. Stage98 r1 **all tid=0** because assignment was commented out, while all companion logs have sole TID 11956; strict stock correlator cannot be run correctly on this r1 capture. Stage98 r2 restores only assignment, exact SHA-gated updater/rollback; original native replay, hook and loader unchanged, 72/72 offline fix tests and original PE 86/86 checks pass. **R2 is source-only pending Windows build/live test**. Stage93FinalCommandQpc remains old session, so no same-run Stage98→FinalCommand claim. Preserve tested r1 DLL 2,051,584 B and Stage97 fallback 2,045,440 B, Stage36 OFF. Next: r2 Windows rebuild, verify real TID and strict joined results.

---

## Stage98 r1 — source-only F7A00 native command-route predicate witness (2026-11-12)

[Stage98 branch](https://github.com/Hawky11/1/tree/stage98-f7a00-command-route-source-only/Stage98_Overlay) · [source commit](https://github.com/Hawky11/1/commit/46be94c8428eab83af284ae16dc4a7bf733991d0) · [native static audit](https://github.com/Hawky11/1/tree/stage98-f7a00-command-route-source-only/Stage98_Overlay/analysis/STATIC_ANALYSIS.md). Following Stage97 LIVE success (MSVC x86 DLL 2,045,440 B), identified original shared boundary **F7A00 MOV EAX,[EDI+3B78]**, early branch to F7A99/F3930 when state bit8 is clear or flag3B70 bit18 is set, and later bypass branches at status 0x816–0x81A / bit15 / bit17. Stage98 adds precisely **one passive guarded 6-byte hook at F7A00**, original MOV once, records real native predicate state, current command ESI and QPC; existing Stage94/95 logs determine actual next route. Early decision classes 1–4 have deterministic predicted destinations; class 5 **unknown** because dynamic helper `525CD0` and additional checks still apply. Full SHA-gated installer and rollback for **exact Stage97 r1 source**; no other stage source or native site changes, Stage36 OFF, no gameplay or DR writes and no synthetic commands. **Offline 86/86 PE/source/synthetic tests pass. Windows build and game runtime NOT tested yet.** Preserve tested Stage97 DLL and Stage96 fallback; next upload Build.log + full Stage98/97/96/95/94/93 same-run logs. No claim of improved attacks or hit causality.

---

## Stage97 r1 LIVE runtime validation and native ESI source resolution (2026-11-12)

[User logs commit 61e62b6](https://github.com/Hawky11/1/commit/61e62b6de57b47fdeb1767a71f931e5463ca782a) · [complete report](Stage97_Runtime/RUNTIME_ANALYSIS.md) · [machine results](Stage97_Runtime/results.json) · [source-only installation package](https://github.com/Hawky11/1/tree/stage97-inherited-esi-source-only/Stage97_Overlay). Windows x86 BUILD SUCCEEDED; DLL 2,045,440 B. F77D4 Stage97 **INSTALLED 1/1**; Stage94 2/2 + Stage95 2/2 + Stage96 5/5 coexist; each has 46 clean HEALTH snapshots. Same-run counts: Stage97 INHERITED 12,997 (66 finite), Stage96 WRITER 14,071, Stage95 BYPASS 11,563 (61 finite), Stage94 GATE 200 (162 forward/38 reject), Stage93 FINAL_QPC 162. Exact latest-writer Stage95 finite-bypass provenance **61/61**: Stage96 59 (F79F0=39, F7924=13, F7A8F=7), Stage97 inherited ESI 2. Stage97 seq3471/8905 both `localD4=0x2C` and JNE predicted true; same-run 68/72 QPC ticks to Stage95 seq3140/7987, then Stage94 gate386/1058 forward, Stage93 FinalCommand seq46/114 payload 0x2C. No Stage96 0x2C writers. All 200 gates have 139 producer +61 bypass routes and 162/162 forwarded match true FinalCommand. This **reproduces the route** suggested for earlier Stage96 bypass seq9461; cannot claim an exact cross-session join. No gameplay improvement proven. Preserve Stage97 and Stage96 DLL, Stage36 OFF. Historical source-only notes below remain history.

---

## Stage97 r1 — inherited ESI candidate witness (SOURCE ONLY; 2026-11-12)

**Code:** [Stage97 branch](https://github.com/Hawky11/1/tree/stage97-inherited-esi-source-only/Stage97_Overlay); [static audit](https://github.com/Hawky11/1/tree/stage97-inherited-esi-source-only/Stage97_Overlay/analysis/STATIC_ANALYSIS.md); [Stage96 evidence](https://github.com/Hawky11/1/blob/main/Stage96_Runtime/RUNTIME_ANALYSIS.md). From user Library original SHA256-pinned AALib.exe, statically reconstructed F7756 writer to `[EBP-2C]`, F77D8 `MOV ESI,[EBP-2C]`, F77DB conditional jump, and branch to F789E/F7A94 bypassing five Stage96 writers. A single exact-signature guarded hook at F77D4 replays original CMP/MOV once and logs pre/local/post ESI context. Includes SHA-gated ApplyStage97, rollback, source/tests and Stage95/96 join correlator. Stage36 OFF, no extra game-data/DR writes or synthetic commands. **Offline 66/66 PASS. Windows compilation and live installation NOT performed.** Stage96 remains last confirmed build (2,039,808 B); runtime writer attribution remains **55/56**, bypass→gate **56/56**. Stage95 bypass seq9461, ESI=0x2C remains unexplained until Stage97 live evidence. Preserve project history and source branches. Next: source update → x86 Build.bat → short test and upload complete same-run logs.

---

## 2026-11-12 — Stage96 r1 LIVE verified; source provenance resolved 55/56, one ESI gap remains

**Evidence:** [runtime upload 7922935](https://github.com/Hawky11/1/commit/7922935697265f370148fa3fadf06b710e88e395) and [full Stage96 analysis](Stage96_Runtime/RUNTIME_ANALYSIS.md). This supersedes the older Stage94 source-only pending status below, without removing historical text.

- MSVC x86 **BUILD SUCCEEDED**, dinput8.dll **2,039,808 bytes** (only pre-existing nonfatal C4754 warning). Stage94 **2/2**, Stage95 **2/2**, Stage96 **5/5** native hook installation verified in the same run; 51 clean HEALTH records for each stage, no drops/mismatches/read failures. Stage36 OFF; no extra game-state/DR writes reported.
- Stage96 **17,558 writer events** across F7924 11,804 / F79B6 1,693 / F79D4 1,270 / F79F0 2,286 / F7A8F 505. Stage95 12,484 bypass events; Stage94 208 gates (163 forwarded, 45 rejected); Stage93 163 real FinalCommand QPC rows.
- **163/163** forwarded gate → real FinalCommand events match player, TID, side, command payload and order (2–62 QPC-tick lag). **152** gate events match finite F3930 result. The other **56/56** gates uniquely match Stage95 finite ESI bypass events (2–58 QPC ticks).
- Of those 56 bypass-gate events, **55/56** have an exact-value latest Stage96 writer (1–16 QPC ticks before bypass): F79F0 **39**, F7924 **12**, F7A8F **4**. The exceptional P1 bypass seq 9461 → gate seq 1484 forwarded ESI **0x2C**, but latest Stage96 writer returned **0x2B** about 2,606,909 QPC ticks before: provenance UNKNOWN; never claim 56/56 writer attribution.
- **Stage94 heuristic false-positive corrected:** gate seq 1487 ESI **0x23** had `sourceCandidateSeq=1486` F3930 EAX=FFFFFFFF and `sourceValueMatches=0`; actual measured route Stage96 F7924 → Stage95 bypass seq 9707 → Stage94 gate. Stage94 heuristics alone do not prove source.
- Next Stage97: disassemble original game control flow into F7A94 and investigate inherited/unobserved ESI writer that yields 0x2C. Keep passive, signature-gated, original native behavior, Stage36 OFF. The complete case-by-case limitations are in `Stage96_Runtime/RUNTIME_ANALYSIS.md`.

---

## Stage94 r3 exact Stage82-callchain compatibility — source only, Windows test pending (2026-11-12)

Stage94 r2 diagnostics from user's fresh game log: `STAGE94_SIGFAIL region=F3930_CALL rva=0F7AD1 expected=E85ABEFFFF actual=E87A679B50 gameBase=00100000`, and `STAGE94_SIGFAIL_TARGET callF3930Expected=0F3930 actualRVA=50AAE250 jneExpected=0F7C68 actualRVA=0F7C68`. **The actual CALL target VA is 0x50BAE250**, not 0x50AAE250: old diagnostic printed target relative to gameBase. The previously installed **Stage82 r2** is the identified owner: its source `Stage82NestedSelector.inl` (user Library source archive `ArcanaAI_Stage82_r2_SignatureAudit_SourceOnly.zip`) installs an exact CALL detour at F7AD1 targeting its local `Stage82Call1`. That wrapper invokes the original F3930 with five copied args, returns native EAX, and returns to F7AD6. This was a **false rejection of an authorized earlier patch** by Stage94's original-only producer-call guard, not evidence of corrupt game code.

**Source-only r3 fix:** [Stage94 r3 hook source](https://github.com/Hawky11/1/blob/stage94-f3930-producer-type1-gate-source-only/Stage94_Overlay/src/Stage94Type1ProducerGate.inl), [SHA-safe r1/r2 upgrader](https://github.com/Hawky11/1/blob/stage94-f3930-producer-type1-gate-source-only/Stage94_Overlay/UpdateStage94r3.ps1), [stage94 README](https://github.com/Hawky11/1/blob/stage94-f3930-producer-type1-gate-source-only/Stage94_Overlay/README_STAGE94_RU.md). `ProducerCallChainOK` permits only exact original CALL F3930 OR verified Stage82 redirect **to the runtime exact address `&Stage82Call1`**, with `arcana_stage82::g_installed=1` and `g_Stage82Target1=gameBase+F3930`. No accept-any-call-into-DLL condition. All other native bytes and branch targets must still match, including recheck after threads suspended, with original rollback and two Stage94 hook sites unchanged. No loader API under suspended-peer signature check. Stage36 remains OFF. Source/PE model **38/38** offline tests PASS incl actual E8 rel32 target 0x50BAE250, wrong actor/handler variants, original AALib.exe original bytes. r3 ZIP complete and SHA verified: `ArcanaAI_Stage94_r3_Stage82_CallChain_Compatibility_SourceOnly.zip`.

**NEXT ACTION:** close AALib; preserve tested Stage93 2,019,840-byte DLL; extract r3 ZIP; invoke `powershell -ExecutionPolicy Bypass -File .\UpdateStage94r3.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'`; then x86 Build.bat, one short game run. Require `STAGE94_PRODUCER_CHAIN_OK viaStage82=1`, `STAGE94_INSTALLED 2/2`, healthy Stage94 diagnostics and ongoing Stage82/93/92. Upload Build.log + complete `ArcanaAI_Stage94ProducerGateTrace.log`; if installed then same-run Stage82, Stage93FinalCommandQpc, FinalCommand for correlation. If new error or instability, retain Stage93 DLL/rollback; **r3 is not Windows-built/game-tested yet**.

---

## Stage94 r1 runtime signature preflight refused; diagnostic r2 prepared (2026-11-12)

User's game log: `STAGE94_BUILD=2026-11-12-r1 SOURCE_ONLY F7AD6 after-F3930 producer+F7B0A native JNE; originalOnce=1 gameWrites=0 DRwrites=0`, then `STAGE94_ERROR native signature already modified/not exact; no patch`. This means r1 did **not install any Stage94 native hook**, and the active **runtime bytes** at one or more of seven guarded sites or the two relative targets disagree with the expected original bytes. Re-examined exact user original AALib.exe: **7/7 guard byte sequences plus two relative call/JNE destinations pass (9/9)**. No evidence currently identifying the responsible prior hook; don't force/skip the signature test.

**Latest Stage94 r2 diagnostic**: [source branch](https://github.com/Hawky11/1/tree/stage94-f3930-producer-type1-gate-source-only/Stage94_Overlay) now contains revised `src/Stage94Type1ProducerGate.inl` with `DescribeSignatureFailure()` executed before refusing the hook: `STAGE94_SIGFAIL region=... rva=... expected=... actual=...`, optional `STAGE94_SIGFAIL_JMP` with native JMP targetVA and owner UNKNOWN, optional `STAGE94_SIGFAIL_TARGET`. Live capture still refuses all patches until exact signature matches; Stage93 native observer unchanged. SHA-gated `UpdateStage94Diagnostic.ps1` upgrades **existing installed Stage94 r1 source only** (Stage94 observer and backup manifest's `installed_stage94_sha`), preserving Stage81/92/93 and fully functional `RollbackStage94.ps1`. Separate ZIP `ArcanaAI_Stage94_r2_RuntimeSignatureDiagnostic_SourceOnly.zip` provided in ChatGPT, verified all files SHA. **15/15** offline checks PASS; Windows r2 build/runtime pending. Backup original 2,019,840-byte Stage93 DLL; extract r2 and run `powershell -ExecutionPolicy Bypass -File .\UpdateStage94Diagnostic.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'`, then Build.bat, one game launch, upload Build.log + full `ArcanaAI_Stage94ProducerGateTrace.log`. Do not try full Stage94 installer again on an already installed Stage94 source; that installer only accepts exact Stage93 input. New clean-install script in GitHub expects r2 observer SHA `80f5d540387c2e0df30898a0cb32d60050a8917104e816e4a63bdd135bec772e`.

Stage36 stays OFF. Do not infer final-command causality or claim native Stage94 hook is working until `STAGE94_INSTALLED 2/2` and clean same-session trace are observed.

---

## 2026-11-12 — Stage94 r1 produced SOURCE-ONLY (not Windows built)

- Goal: advance beyond Stage93's 133/133 temporal state-to-FinalCommand joins toward same-event native origin of final type1 payload. Original AALib.exe verified SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba, x86 PE32.
- Static native route: CALL F3930 at F7AD1 -> F7AD6 ADD ESP,14h / MOV ESI,EAX (result becomes ESI); branches may override ESI at F7AEE; F7B07 CMP ECX,1 -> F7B0A JNE F7C68 gate; downstream F7C59 PUSH ESI / PUSH 1 / PUSH EBX -> original F7C5D CALL F2DE0. Not all commands originate at F3930.
- Prepared reversible SHA-gated Stage94 observer overlay on exact Stage93 r3: F7AD6 5-byte native result replay, F7B0A 6-byte JNE replay with original branch targets; 8192 bounded worker-only log, GPR/FLAGS/FPU restoration, install mutex, peer EIP guard and failure rollback. Stage94 requires Stage93 installed. No changed AI decisions, synthetic events, DR watchpoints, duplicate F7C5D/EE306 hooks, Stage36 enabled or extra game-state writes.
- [Stage94 source branch](https://github.com/Hawky11/1/tree/stage94-f3930-producer-type1-gate-source-only/Stage94_Overlay) and ChatGPT ZIP `ArcanaAI_Stage94_F3930_Producer_Type1_Gate_SourceOnly.zip` preserve exact source, installer, rollback, original PE/static analysis and offline correlator. Source/PE 38/38 PASS, synthetic/installer 13/13 PASS, raw native encodings verified. **Not tested in Windows MSVC or game.**
- Next runtime evidence required: new Build.log, Stage94 2/2 installation and clean Health, `STAGE94_PRODUCER`/ `STAGE94_GATE`, existing `STAGE93_FINAL_QPC` and original FinalCommand same-session. Backup last confirmed Stage93 DLL 2,019,840 bytes. Strong temporally bounded native ESI agreement may demonstrate a path but does not alone prove unique gameplay causes.

---

## 2026-11-12 — Stage93 r3 Windows runtime success, direct F5A68 native setter established

- User upload [c623b607](https://github.com/Hawky11/1/commit/c623b607a) brought real MSVC x86 Build.log, 2,019,840-byte `dinput8.dll` BUILD SUCCEEDED (only prior C4754), new Stage93 direct-bit3 and final-command QPC logs, and fresh Stage92/91/90 data. Stage93 `INSTALLED 1/1`, Stage92 3/3 with Stage35 compatibility, 47 clean health intervals. Report: `Stage93_Runtime/RUNTIME_ANALYSIS.md`; counters `results.json`.
- Exactly 15,814 Stage92 F5990 ticks; 25,898 Stage91 bit-index reads (all predictionMatch=1); 7,907 direct F5A68 OR events and 7,907 helper events (SET10/CLEAR7897). Direct OR branch distribution: 6,526 early bit2 bypass, 585 positive bit9, 796 positive bit10. Exact unique OR-to-F5990 joins, zero errors.
- 81 Stage92 P+3B70 bit3 0→1 transitions: 77 directly captured native F5A68 OR 0→1 (44 P1/33 P2), four on original 9ED20 SET helper ticks, zero unexplained. This gives the previously missing direct-writer evidence, without claiming all historical Stage92 events can be joined across sessions or no other native writers exist.
- Added QPC to existing FinalCommand recorder without another native detour: 133/133 exact original FinalCommand matches (P1 70/P2 63, 43 unique payloads). 46 command snapshots bit3=1; 87 bit3=0. 133/133 last completed Stage92 same actor/TID snapshots agree bit3, QPC delay 533–2,303 raw ticks. EventDispatch has 430 detailed rows (287 type0,143 type1); bit3→specific payload causality not yet established.
- Stage90 has 1,392 fresh native F57D0 calls; Stage87/88 logs in root came from earlier run and are **not** joined. Keep tested DLL and rollback, Stage36 OFF. Next stage should target action-selection cause with a narrowly guarded passive native observation, not assume temporal correlation implies control.

---

## 2026-11-12 — Stage93 r1 prepared, source-only (not Windows tested)

- Confirmed Stage92 r3 live report: 17,788 F5990 ticks, 9,786 helper selections, 31,605 exact Stage91 reader joins; 127 bit3 rises, of which 119 without observed 9ED20 SET. Report in `Stage92_Runtime/RUNTIME_ANALYSIS.md`.
- Retrieved original AALib.exe from user ChatGPT File Library; verified SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Disassembled F5990; at RVA **F5A68** native opcode `83 8E 70 3B 00 00 08` ORs P+3B70 bit3. Branches bit2 and positive bit9/bit10 can enter this direct OR. Bit11 follows the already traced helper path. This explains a candidate for Stage92 no-helper rises but does **not** establish 119/119 attribution.
- Created immutable [Stage93 source-only branch](https://github.com/Hawky11/1/tree/stage93-direct-bit3-final-qpc-source-only): exact byte-gated passive F5A68 OR wrapper + Stage81 install/flush/health; SHA-gated PowerShell Apply/Rollback. Existing Stage92 r3 and Stage91 DLLs remain untouched.
- QPC source-only bridge captures game-thread QPC in original FinalCommand F7C5D observer, logs worker-only companion `STAGE93_FINAL_QPC` without second CALL detour. Direct native OR observer logs `STAGE93_OR` with side, player, TID, Stage92 tick, before/after field, QPC and health.
- Offline verification: original-EXE/source verifier **32/32 PASS**, native x86 OR independently encoded exact 7 bytes, synthetic correlator **2 tests PASS**. **NOT built with Windows MSVC, installed or runtime-tested**. The complete patch/test/analysis ZIP is provided through ChatGPT. Stage36 must remain disabled; preserve Stage92 r3 2,011,648-byte DLL.
- Next: user applies ZIP against exact Stage92 r3 source, compiles MSVC x86, verifies Stage93 1/1 and Stage92 3/3, zero health errors, uploads same-run Build.log and new/old trace files. Only live direct-writer and final-command timestamp joins may support further conclusions; no visual attack/hit interpretation yet.

---

# Project history

## Stage92 r3 runtime PROVEN — 31,605 reader calls -> 17,788 ticks -> 9,786 helpers (2026-11-12)

- [User upload 7e1df728](https://github.com/Hawky11/1/commit/7e1df728bbfb85b797c06785cd8cf7e407c53a35) x86 **BUILD SUCCEEDED 2,011,648 bytes**, Stage35 compatible pair verified with no guard bypass, `STAGE92_INSTALLED 3/3` and 48/48 health rows clean (drops/readFail/mismatch/overflow zero).
- **All 17,788 Stage92 ticks** joined by exact Stage91 sequence range, side/player/TID and enclosing QPC to **31,605 Stage91 direct F58E0 bit9/10/11 reads**, 0 mismatches. Branch choices: no reader 6,502; bit9 AL1 753; bit10 AL1 747; bit11 AL1/SET_9ED20 154; bit11 AL0/CLEAR_9ED60 9,632. All 9,786 Stage92 helper events matched parent tick/actor, P1 only SET. Stage90 F57D0 1,742 native calls, P1 bit11 up/down 10/10; all 154 positive bit11 reads have preceding last Stage90 bitmap bit11 set. Exact event: Stage90 seq252 -> Stage91 seq4429–4431 -> Stage92 tick3815/helper3816: P+3B70 0x0→0x8.
- Stage92 P+3B70 bit3 rose **127** calls (8 with SET helper, **119 without captured helper**), fell **123** (all with CLEAR). So alternative flag setters exist inside F5990, unique SET origin unproven. There were 174 real FinalCommand type1 records and 602 EventDispatch details; 55 FINAL_COMMAND snapshots had bit3 on, but current final-command log lacks QPC and **causal command selection NOT established**.
- Full [runtime analysis](https://github.com/Hawky11/1/blob/main/Stage92_Runtime/RUNTIME_ANALYSIS.md) / [machine stats](https://github.com/Hawky11/1/blob/main/Stage92_Runtime/results.json). **Next Stage93**: source-only native alternate P+3B70 bit3 writer origin and QPC timestamp at existing F7C62 final-command observer, then strict joins to actor+TID+event. Preserve Stage36 OFF, no synthetic commands or game-state writes.

---

## Stage92 r2 face-control refusal → r3 Stage35-aware fail-closed check (2026-11-12)

- User r2 Build.log: **BUILD SUCCEEDED** with x86 MSVC, `dinput8.dll` **2,010,112 bytes** and old C4754. Live audit **14/15 region signatures matched**; only `ACAB8_AFTER 0ACABD` mismatched, expected `56E8FD95040056`, live `56E81D8FB17856`. All Stage92 target CALLs and native 9ED20/9ED60 opcodes passed. R2 refused patch, no tick/helper evidence.
- Neighbor `ACABE` belongs to already installed **Stage35 Controller Consumer**, original TIMER helper F60C0; paired Stage35 FLAGS hook `ACAC4→F5BD0`. The modified CALL live points to `0x78FC59E0`; INI has Stage35 enabled. This is a signature collision with an earlier observer, not proof of action changes.
- ChatGPT generated **Stage92 r3 ZIP**, SHA source+adapter guard, r2 rollback, accepts either pristine native ACABE or precisely checked Stage35 *pair* in same dinput8 executable image, no generic bypass and no game data write. Original EXE static 67/67; synthetic **10** cases and **17** source structural guards passed; **r3 Windows compile/runtime untested**. [Stage92 r3 analysis handoff](https://github.com/Hawky11/1/blob/stage92-r3-stage35-compatible-guard-handoff/Stage92_R3_Analysis/README_RU.md). Next test: one launch and r3 Stage92 log; expect `STAGE92_STAGE35_COMPAT` and `STAGE92_INSTALLED 3/3`; only then play 3–5 COM-vs-COM rounds to test bit3 branches.

---

## Stage92 r1 Windows runtime audit — guard blocks installation (2026-11-11)

- [Latest user upload c8530d5](https://github.com/Hawky11/1/commit/c8530d5fd389088bb214010a0b99d19d1f1e1c6f): `Build.log` x86 **BUILD SUCCEEDED**, DLL **2,008,576 bytes**, old C4754. `ArcanaAI_Stage92F5990BranchTrace.log` only `STAGE92_BUILD` + `STAGE92_ERROR native signature mismatch/already installed; no patch`. No hook, helper selection, bit3 transitions or final COM action evidenced for Stage92.
- Stage87/88/89/90 trace files present with active records and no startup critical warnings visible; do not conflate those prior stage events with Stage92 hook installation. SHA-pinned original AALib passes 67/67 static Stage92 checks, but actual loaded EXE (after earlier detours) failed one of r1 live checks. r1 did not log the first failing byte, therefore exact conflict unknown.
- **Next r2:** separate [audit note](https://github.com/Hawky11/1/blob/stage92-r2-live-signature-audit-source-only/Stage92_R2_Audit/README_RU.md); ChatGPT ZIP provides r1→r2 source-only SHA-gated upgrade and byte-reporting `STAGE92_SIGAUDIT` without bypassing checks. Windows r2 build/runtime UNTESTED. Upload Build.log and Stage92 SIGAUDIT after single launch; investigate failed RVA then consider a minimal noncolliding new branch. Preserve verified Stage91 DLL 1,999,872 bytes, Stage36 OFF.

---

## Stage92 source-only native F5990 branch/helper attribution (2026-11-11)

- Prepared [Stage92 branch](https://github.com/Hawky11/1/tree/stage92-f5990-branch-consequence-source-only), [RU installation](https://github.com/Hawky11/1/blob/stage92-f5990-branch-consequence-source-only/Stage92_Overlay/README_STAGE92_RU.md), and [native static report](https://github.com/Hawky11/1/blob/stage92-f5990-branch-consequence-source-only/Stage92_Analysis/STATIC_ANALYSIS.md). Based on exact Stage91 source-only commit `0516e767be5657118d6950466c5c3dfdfb632a5c` and preserved prior historical branches. Tested Stage91 DLL **1,999,872 bytes** unchanged.
- Original SHA-pinned AALib.exe disassembly maps `ACAB8→F5990`; AL=1 bit9/10 direct path OR at `F5A68` sets `P+3B70 & 0x8`. Index11 AL1 chooses `F5A86→9ED20` native bit3 setter; AL0 chooses `F5A8F→9ED60` native clearer; both native `RET4` with index3. **Static mapping not yet actual runtime selected branch/hit evidence.**
- Three read-only exact CALL-site wrappers capture F5990 PRE/POST and actual helper call selection while replaying original function and helpers exactly once. Helper **tail JMP** preserves original callee arg cleanup. Native PE/opcode guards, peer suspension, mutex and rollback. No EE306/F58E0 conflicts, no gameplay/DR changes. Stage91 remains source preserved.
- **67/67 PE32/source checks PASS; 2 positive and 3 negative synthetic correlator tests PASS. SOURCE ONLY: not compiled with MSVC, no Stage92 runtime installed/tested, no AI behavior improvement.** Next: user installs overlay onto exact Stage91 sources and builds x86; 3–5 natural rounds; upload same-session Build.log and Stage92/91/90/88/87/Dispatch/FinalCommand, check installation and zero errors; correlate same actor/TID/QPC; rollback on failure.

---

## Stage91 direct special-bit native reads VERIFIED at runtime — 1,825 positive ALs, 2026-11-11

- [Fresh uploaded commit `4e56101`](https://github.com/Hawky11/1/commit/4e56101e9095d6f3b04fd4dcedf2ca5d5314ed57) · [Stage91 full runtime analysis](Stage91_Runtime/RUNTIME_ANALYSIS.md) · [results.json](Stage91_Runtime/results.json) · [source code branch](https://github.com/Hawky11/1/tree/stage91-f58e0-direct-bit-readers-source-only/Stage91_Overlay). **Windows MSVC x86 BUILD SUCCEEDED**, DLL **1,999,872 bytes**, old nonfatal C4754. `STAGE91_INSTALLED mask=7 sites=3/3`, all three direct `F58E0` bit9/10/11 readers called.
- **Critical large-file retrieval trap:** Stage91 log actually **15,480,170 bytes** (blob SHA `84f4f78cb0eeeb68425caea347bb300e4c308e2f`). `fetch_file` yields empty on oversized file; `fetch_blob` returns full data. Parsed **34,790** Stage91 events: `F5A4D/index9/0x200` 12,571 (AL1 **1,183**), `F5A5C/index10/0x400` 11,388 (**557**), `F5A76/index11/0x800` 10,831 (**85**). Total native predicate **AL1=1,825**, AL0=32,965. All 34,790 predicted results match actual AL, bit present for each AL1, before/after bitmap equal, full masks, last same-player/TID F57D0 POST exactly matches reader PRE. No errors or dropped events. Periodic health sums **34,114**, with 676 further events past last health row, explaining difference.
- **Causal native-state sequences:** 49 AL1 readings directly followed a Stage88 joined native F57D0 call; 12 had actual target bit 0→1 (11×0x200, 1×0x400). Examples Stage88 seq22 P2 (0x29,0,0x10)→(0x0E,0x200,0) → Stage91 seq2469 AL1; Stage88 seq239 P1 (0x29,0,0x10)→(0x0F,0x400,0) → Stage91 seq32679 AL1; Stage88 seq98 P1 already-set 0x800 → Stage91 seq12415 AL1 → Stage90 seq680 clears 0x800. This confirms real downstream **predicate consumption**, not a particular action/hit.
- **Stage92 next:** verify the executed post-F5990 branch and P+3B70 field changes, 9ED20 versus 9ED60 selection, and ultimately same-event FinalCommand/queue provenance with read-only guarded hooks. Preserve Stage91 DLL, Stage36 off. Do not claim improved COM fighting performance from AL1 alone.

---

## Stage91 source-only: native direct F58E0 bitmap-bit readers 9/10/11 localized — 2026-11-11

- [Stage91 original SHA-pinned static disassembly](Stage91_Analysis/STATIC_ANALYSIS.md) · [68/68 static verification](Stage91_Analysis/static_results.json) · [11-file code/installer/test branch](https://github.com/Hawky11/1/tree/stage91-f58e0-direct-bit-readers-source-only/Stage91_Overlay). Native helper `F58E0` generic indices **9/10/11** check special bits of `P+3A0C` 0x200/0x400/0x800, respectively. Reader callsites **F5A4D/F5A5C/F5A76** inside F5990; upstream update call at `ACAB8`. AL1 in first two callers goes to F5A68 setting `P+3B70 |= 8`; third caller selects `9ED20` vs `9ED60`. These are native control-flow candidates **not yet observed by Stage91 runtime**.
- New Stage91 source-only r1: three individually SHA/exact-signature protected F58E0 CALL site wrappers, preserving original callee once and context/stack; minimal extension to Stage90-tested Stage81 worker, bounded worker-only ring. No global F58E0 hook, duplicate EE306, writes to game fields, or synthetic inputs. Apply/rollback pinned exact Stage90 source. **68/68 static PASS**, synthetic positive and three negatives PASS, 11 Git blobs match archive. **NOT Windows-built nor runtime tested.**
- Preserve Stage90 DLL **1,991,168 bytes**, Stage36 off. Next action: user Windows x86 build and same-run COM-vs-COM logs for Stage91/90/88/87/89/FinalCommand/EventDispatch. A Stage91 `AL=1` would prove the specific native predicate saw the active bit, not proof of queued COM attack, hit or improved AI.

---

## Stage90 runtime proves net F57D0 special-bit clears and all three later Stage89 AL0 paths — 2026-11-11

- [Frozen user run `3a6f9e9`](https://github.com/Hawky11/1/commit/3a6f9e908ab7e3bc1e18aca4085c5a6b3fdd044b) · [detailed Stage90 audit](Stage90_Runtime/RUNTIME_ANALYSIS.md) · [machine results](Stage90_Runtime/results.json) · [Stage90 source branch](https://github.com/Hawky11/1/tree/stage90-f57d0-bitmap-clear-source-only/Stage90_Overlay). Windows MSVC x86 **BUILD SUCCEEDED**, DLL **1,991,168 bytes**, one familiar nonfatal C4754. Stage90 existing Stage24 EE306→F57D0 bridge **1,792/1,792** completed P1/P2 calls with 718 output changes and 1,074 no change; **87 special-bit net clears** [38×0x200, 41×0x400, 8×0x800], **88 net sets** [39/41/8]. All Stage90 HEALTH readMaskIncomplete/depthOverflow/mismatch/readFail/dropped zero.
- **280/280 unique Stage87→Stage88→Stage90** exact QPC-containing player/TID/selector/bitmap PRE+POST joins; selector0/1/2 140/119/21 and 15 Stage88 target-bit 0→1 changes. No evidence of hook collisions, Stage36 remains disabled.
- **Three full-state native chronology chains before Stage89 AL0:** (1) P1 Stage88 seq79 selector1/0x400, Stage90 seq536 preserves bit, Stage90 seq540 arg6/type0/payload0x12 clears bit, Stage89 seq1 returns AL0; (2) Stage88 seq158 selector2/0x800, Stage90 seq1011 preserves bit, Stage90 seq1014 arg6/type0/payload0x12 clears bit, Stage89 seq2 AL0 **101,261 QPC ticks** after clear; (3) Stage88 seq205 selector0/0x200, Stage90 seq1442 preserves bit, Stage90 seq1445 arg0/type0/payload0 clears bit, Stage89 seq3 AL0. All same player P1/TID 6820; last Stage90 POST before each read matches Stage89 PRE, no intervening **recorded F57D0** new special-bit set. Exact F57FA store EIP is STATIC, not independently observed; other code may write bitmap.
- The question “why AAB55 always sees AL0?” is **resolved for these three sampled sequences** as native net clears before reader. **NEXT Stage91:** find genuine consumers during active-bit windows before the clear; do not make AI improvement/hit claim, install another EE306 hook or introduce gameplay writes.

---

## Stage90 source-only: original F57D0 zero/reset path pinned; full-call read-only observer ready — 2026-11-11

- [Full Stage90 source-only 12-file branch](https://github.com/Hawky11/1/tree/stage90-f57d0-bitmap-clear-source-only/Stage90_Overlay) · [static disassembly](Stage90_Analysis/STATIC_ANALYSIS.md) · preceding [Stage89 runtime](Stage89_Runtime/RUNTIME_ANALYSIS.md). Original exe SHA pinned; **30/30 static checks PASS**, synthetic correlator positive and negative tests PASS, 12 Git blobs verified identical to local ZIP contents. **Stage90 Windows x86 build/runtime NOT YET TESTED**; preserve tested Stage89 1,984,000-byte DLL.
- **Native potential clear:** `F57EC TEST table[selector*3],1`; if true, `F57FA MOV [EDI+4],0` zeros `P+3A0C`, `F57FD MOV [EDI+8],0` zeros `P+3A10`. `F5800` stores new code and `F5856/F58AC` can OR bit back. Descriptor for selector codes 0x0E/0x0F/0x10 = `01 26 FF`; bit0 set. Net pre/post clearing is not identical to internal zeroing where bits are re-added.
- **Stage90 hook policy:** **no new native patch**, no second EE306 or global F58E0 detour. Existing Stage24 EE306→F57D0 wrapper and Stage88 C++ Pre/Post bridge extended to observe all P1/P2 native calls; record triple bitmap PRE/POST, `clearMask/setMask=0xE00`, descriptor, selector code, dispatch, QPC, and exact same-thread Stage87 seq if available. Bounded 8192 ring, worker-only writing, SHA-gated two-file install/rollback; prior Stage87/88/89 instrumentation preserved.
- Next: Windows MSVC x86 build, COM-vs-COM capture with Stage90 `logAll=1`, strict Stage88-within-Stage90 same-player/TID/QPC matching, identify any F57D0 call between Stage88 set and Stage89 AL=0 that **actually clears** the bit. If none, study other native writers. No COM action/attack inference yet; keep Stage36 disabled.

---

## Stage89 live F58E0 reader AL=0; Stage88 all three bit outputs runtime-verified — 2026-11-11

- [Fresh user run `7475572`](https://github.com/Hawky11/1/commit/7475572ea25d04edb01f383367af83ceb2ad7711) · [full Stage89 audit](Stage89_Runtime/RUNTIME_ANALYSIS.md) · [results JSON](Stage89_Runtime/results.json) · [source `7ede68b`](https://github.com/Hawky11/1/commit/7ede68b825faadea5caed2a59b9afa4881cc24a0). `Build.log` **BUILD SUCCEEDED**, Windows x86 DLL **1,984,000 bytes**, earlier nonfatal C4754.
- Stage89 installed **all four** targeted `F58E0` CALL-site hooks (mask=F), but live counts `9A870=0,9AA20=0,9AB00=0,AAB55=4` (P1 three, P2 one). All four `AL=0`, native bitmap read mask=7 pre/post, no bitmap mutation, predicted AL agrees. All Stage89 HEALTH mismatch/readFail/dropped/overflow = 0. The `AAB55` predicted JNE-not-taken is not proof of traversal into `E8560`.
- **NEW Stage88 selector2 positive in SAME RUN:** 170/170 strict Stage87↔Stage88 same-player/TID/QPC links. Selector0→0x0E/bit0x200 81 events (4 actual bit0→1); selector1→0x0F/bit0x400 62 (0); **selector2→0x10/bit0x800 27 (1)**. Stage88 seq31 P1 `0x29,0,0x10 → 0x10,0x800,0`. 54 3-DWORD changes; all HEALTH errors zero. Earlier Stage88 run had zero selector2 only as a time-scoped fact, **now superseded as a testing gap**.
- No Stage89 sample contained any of the special bits set; nearest prior same-P/TID Stage88 events are separated by QPC gaps, and all sampled bitmap states lack those bits. Thus no **immediate/causal** Stage88→reader→command link; FinalCommand's 167 type1 records must not be matched by mere presence.
- **Stage90 next:** static localization of actual active-bitmap reader and clear/reset writers, then decide on a scoped passive exact-site trace; preserve Stage89 DLL, keep Stage36 off, no gameplay writes or extra EE306/global F58E0 hook.

---

## Stage89 r1 targeted F58E0 reader SOURCE PREPARED — 2026-11-11

- [Stage89 isolated source branch](https://github.com/Hawky11/1/tree/stage89-f58e0-four-call-readers-source-only), [README/install](https://github.com/Hawky11/1/blob/stage89-f58e0-four-call-readers-source-only/Stage89_Overlay/README_STAGE89_RU.md), [verification JSON](https://github.com/Hawky11/1/blob/stage89-f58e0-four-call-readers-source-only/Stage89_Analysis/source_only_results.json). **22 SHA-pinned native signature/destination checks PASS; x86 MSVC build and gameplay runtime NOT PERFORMED.**
- Four separate exact five-byte native CALL-site wrappers (`9A870/9AA20/9AB00/AAB55`) pass original `F58E0(P+3A08,0x26)` args exactly once. No F58E0 global detour, no additional EE306 hook, no synthetic inputs, no writes to game/DR; preserves original results/registers/flags/FPU/SSE. Stage81 worker reused for bounded ring and health logs; SHA-guarded apply/rollback supplied.
- Stage89 captures actual reader AL and before/after P+3A08/0C/10, with fallback bit38 accounted for; `nextBranchPredictedTaken` is **not** an observed EIP. Offline QPC nearest prior Stage88 link remains time context, **not causal proof**. Stage88 remains last runtime-verified stage: 175 exact joins, 13 target-bit sets.
- Next: Windows x86 Build.bat, installation audit, 3–5 natural COM-vs-COM rounds and same-session Stage89/88/87/24/Dispatch/FinalCommand logs. Keep Stage36 off and back up Stage88 DLL (1,975,296 bytes). No AI/gameplay improvement claimed.

---

## Stage89 static localization of genuine downstream F58E0 0x26 reader — 2026-11-11

- [Exact original SHA-pinned disassembly and callsite table](Stage89_Analysis/STATIC_ANALYSIS.md): F58E0 special `0x26` predicate reads `[P+3A08]+4=P+3A0C`, tests `0x200`, `0x400`, `0x800`, and returns `AL=1` if any is set; generic fallback also examines `P+3A10` bit `0x40` (bitIndex 38). **Four literal-0x26 callsites** `9A870/9AA20/9AB00/AAB55`, each guarded; at `AAB55`, AL=0 can lead to `AAB65→E8560`.
- **STATIC ONLY**: no Stage89 runtime observations/build/AI changes yet. Stage88's 175 mapped bitmap writes do not establish these readers fired. Candidate next source task is a guarded exact CALL-site passive observer plus same-session correlation, not a global F58E0 detour or second EE306 hook.

---

## Stage88 Windows x86 runtime: selector→F57D0→controller bitmap established — 2026-11-11

- [User's 21:03–21:05 upload](https://github.com/Hawky11/1/commit/ccbbf6a41cd7ed5ba0fdf194f557a4bbf112b3ca) · [Stage88 source-only stage](https://github.com/Hawky11/1/commit/eb92db82bae6afb155a58bef8e8429ef3abf7af3) · [full audited runtime analysis](Stage88_Runtime/RUNTIME_ANALYSIS.md) · [JSON](Stage88_Runtime/results.json).
- **BUILD SUCCEEDED**, dinput8.dll **1,975,296 bytes**, existing nonfatal C4754; Stage87 EE2F0 and Stage24 EE306→F57D0 installed. Stage88 has no second EE306 hook. **175/175 strict one-to-one** Stage87→Stage88 same-run seq, side, pointer, TID and QPC; Stage88 1482 pre calls/1307 normal filtered/175 joined and completed; stale/mismatch/readFail/dropped all zero.
- **Confirmed native controller output:** selector0→0x0E→P+3A0C bit0x200 (110 calls, **12** bit 0→1); selector1→0x0F→bit0x400 (65 calls, **1** bit0→1). 58 three-DWORD pre/post changes and 117 unchanged; both COM players. Previous Stage87 had selector2 observations in a **different run**; Stage88 selector2/0x10/bit0x800 **not observed** here.
- Stage88 dispatch metadata: 171 type0/payload0x22, two type0/payload0x23, two type2/payload0x04. Existing Stage24 legacy has five separate type1/payload0x1E samples; FinalCommand logged 143 type1 events, **not proved causally joined** to Stage88. Bitmap write is **not action animation or hit evidence**.
- Next Stage89: SHA-pinned static mapping of actual readers of output P+3A08/0C/10, find one safe non-colliding downstream observation boundary, then read-only same-run test. No AI/gameplay changes, synthetic events, or second EE306 hook.

---

## Stage85 long-run AFTER-JOIN positive alternate selector path / Stage86 source-only — 2026-11-11

- [Frozen user log commit](https://github.com/Hawky11/1/commit/e1fd008f1ed8695f9f7cb53b005112f41f9605b7) and [full analysis](Stage85_Runtime/LONG_RUN_ANALYSIS.md): Stage85 r1 unchanged DLL **1,957,376 bytes** from previous Windows x86 build. New 18:19–18:21 capture 16,747 Stage79, 15,218 Stage80, 3,812 Stage82, 16 Stage84 and 228 Stage85 events. All strict actor/object/TID/QPC parent windows uniquely matched (15,218/15,218, 3,812/3,812, 16/16 and 228/228). All HEALTH errors zero.
- **New rare positive:** Stage80 P2 seq5640 selector 0→1, companion 1→2; Stage82 F3930 seq1453 selector 0→0, companion 1→2; Stage85 F7C68 seq82 selector=0, companion=2, ESI=20h and localF8=0; F7150 returns selector=1. No Stage84 exact MOV hit in that interval. Nine other selector net changes matched Stage84; Stage81 direct MOV hooks zero throughout. **Writer occurred after F7C68, exact writer EIP still not identified.**
- **Next Stage86 prepared SOURCE ONLY** as a two-file optional local overlay in chat (not committed to main, not Windows-built): original SHA-pinned F7CF2 E8 call to E8560 and verified callee `RET 4`, capture before/after selector with original callee once, preserve register/stack/EFLAGS and existing 79/80/81/82/84/85. Requires Windows compile/gameplay test and an uploaded same-session Stage86 log. Keep tested Stage85 DLL as rollback, Stage36 off, no AI/gameplay edit.

---

## Stage85 r1 verified join (unmatched rare writer remains open) — 2026-11-11

- [Stage85 immutable capture](https://github.com/Hawky11/1/commit/428b6b27895b1b1fb3015ab225169370e541050c), [full report](Stage85_Runtime/RUNTIME_ANALYSIS.md), [results](Stage85_Runtime/results.json): Windows x86 BUILD SUCCEEDED, new DLL **1,957,376 bytes**, one existing C4754 warning. Stage85 guard exact signature F7C68, installed 1/1, 109 join samples, all health errors zero.
- One-session Stage79 8,598; Stage80 7,830; Stage82 2,100; Stage84 12 exact stores; Stage85 109. Unique exact QPC containment: 7,830/7,830 (80⊂79), 2,100/2,100 (82⊂80), 12/12 (84⊂82), 109/109 (85⊂80). Of the 109 joins, 103 share a preceding Stage82 call inside the same Stage80 and exactly match that call's selector POST; the other six have no preceding nested call.
- All **seven** Stage80 net selector changes were attributed to actual F3930 Stage84 writers. The **earlier unmatched P2 1→2** in separate [Stage84 long run](Stage84_Runtime/RUNTIME_ANALYSIS.md) was **not reproduced**; do not conflate different runs. Stage81 direct F7150 writer hooks remained zero, so exact native writer of the rare route is still unknown.
- Next: **longer capture with unchanged Stage85 DLL**, not more speculative instrumentation. Preserve original logs and DLL backups; Stage36 remains disabled due to Stage82 F79E4 collision. No gameplay modification or improved AI demonstrated.

---

## Stage83–84 exact F3930 selector writers and remaining F7150 alternative path — VERIFIED (2026-11-11)

- Stage83 r1 [Windows capture](https://github.com/Hawky11/1/commit/394d68c567b41012d41a49007229cb4bf1594080): successful x86 build (DLL **1,952,256 bytes**), four exact F3930 store hooks installed 4/4. Five `STAGE83_EXACT_WRITE` events; real selector writes located at **F3F50** (from EDX) and **F3F69** (from ECX). The observed P2 1→0 net change was not captured by its four hooks.
- Stage84 r1 [first 6/6 capture](https://github.com/Hawky11/1/commit/abef4114a210ac7dac014a9fdd016233b833f0d5): successful build, DLL **1,952,768 bytes**, six F3930 native exact-store hooks (four existing + F3E0A and F3F71 immediate-zero stores), all native signatures verified. First run: six exact-write rows, three changed.
- Stage84 r1 [extended same-run capture](https://github.com/Hawky11/1/commit/364429c76dbfbfecd7c1679dd3e4dacd90e4897c): 15,116 Stage79, 13,849 Stage80, 3,784 Stage82, 21 Stage84 events; all 80⊂79, 82⊂80, 84⊂82 strict same-actor/object/TID/QPC containment is 100% unique; no dropped/overflow/mismatch/readFail. Stage81 installed 3/3 but zero writer hits. **F3F71 confirmed exact instruction performing 1→0 twice and 2→0 once**. F3F50 also directly writes 2→1, and F3F69 writes 1→2. 15/16 F7150 net P+3F50 changes attributed to exact F3930 writer EIP.
- **Remaining gap:** P2 1→2 in Stage79 seq2478 / Stage80 seq2160 / Stage82 F3930 seq513. In F3930, P+3F54 changed 1→2 but P+3F50 remained 1; by enclosing F7150 return P+3F50 changed 1→2, with no Stage84 exact store. Do not assign a writer EIP without new evidence. Stage85 should statically inspect F7150 after the F3930 return, before deciding on a new hook.
- Stage36 must remain disabled to avoid the F79E4 conflict with Stage82; backup known-good Stage84 1,952,768-byte DLL. **No observed AI behavior improvement.** [Detailed report](Stage84_Runtime/RUNTIME_ANALYSIS.md) · [JSON](Stage84_Runtime/results.json).

Append-only record of the ArcanaAI COM-vs-COM diagnostic investigation. Add a new dated entry after each completed stage; do not rewrite earlier conclusions except to correct a documented factual error.

## Project objective

Trace how the COM-vs-COM AI path produces and queues the type-1 event with candidate payload `0x1E`, and identify why the P1/P2 event sometimes does not reach the alternate queue insertion at `AALib.exe+0xF6AD7`.

The staged instrumentation is intended to be observational: it should preserve original arguments, return values, queue state, branch decisions, and AI behavior.

## Stage branch index

| Stage | Branch | Commit at last recorded check | Scope |
|---|---|---|---|
| 29r | [stage29-bit17-runtime-evidence](https://github.com/Hawky11/1/tree/stage29-bit17-runtime-evidence) | `d5f2cd0266759aa95f8617a3eb3e3770040d73d3` | Full same-run bit17 setter/gate/queue correlation and exact uploaded logs |
| 10 | [stage10-diagnostics](https://github.com/Hawky11/1/tree/stage10-diagnostics) | `ee06d43ad63f9806e3fce5ce0c66cfac91210cf1` | Stage10 source archive and baseline traces |
| 11 | [stage11-diagnostics](https://github.com/Hawky11/1/tree/stage11-diagnostics) | `9ae08714655744ceb692494d07ab4eac13a8f8b7` | Stage11 source archive and baseline traces |
| 12 | [stage12-eventdispatch-diagnostics](https://github.com/Hawky11/1/tree/stage12-eventdispatch-diagnostics) | `4552ac45e5d7dd68b592bd82a312ae75e0139851` | Queue and event-dispatch diagnostics |
| 13 | [stage13-alternate-callsite-diagnostics](https://github.com/Hawky11/1/tree/stage13-alternate-callsite-diagnostics) | `19b0e6fc40d9b7626cb2557aa844123eb3419ec1` | Alternate queue callsite at F6AD7 |
| 14 | [stage14-producer-call-diagnostics](https://github.com/Hawky11/1/tree/stage14-producer-call-diagnostics) | `dcd036d1b3cead45f8400fd8545dff41d0d59131` | Parent/helper call context and return values |
| 15 | [stage15-postguard-flow-diagnostics](https://github.com/Hawky11/1/tree/stage15-postguard-flow-diagnostics) | `4d1e2a049c835a991647cb04565d31d330aeac95` | Branch-by-branch flow between F2ED0 and F6AD7 |
| 16 | [stage16-player-bit17-writer-trace](https://github.com/Hawky11/1/tree/stage16-player-bit17-writer-trace) | `58313d8c0f6071bc8099fc5ec04ae04d2974dafd` | Runtime-confirmed bit17 setter/clearer; Stage17 caller verification remains |
| 17 | [stage17-bit113-setter-caller-trace](https://github.com/Hawky11/1/tree/stage17-bit113-setter-caller-trace) | `6b04b0a9ad272aecf8dcb481b5c52edfe5acfe1e` | Source-only setter-entry observer for bit index 0x71; awaiting build/runtime test |
| 17b | [stage17-callsite-init-diagnostics](https://github.com/Hawky11/1/tree/stage17-callsite-init-diagnostics) | `c33317c9b2fa28c339c38f3aaebdcb9a378b73a1` | Diagnostic hardening for the Stage17 direct-callsite observer; build/runtime pending |
| 18 | [stage18-bit113-gate-condition-trace](https://github.com/Hawky11/1/tree/stage18-bit113-gate-condition-trace) | `520eec4cd216795e8ffbaa8738b3602b5e40d39a` | Passive observer of the 0x4F2FD0 state-table gate result; source-only, build/runtime pending |
| 18r | [stage18-bit113-gate-runtime-evidence](https://github.com/Hawky11/1/tree/stage18-bit113-gate-runtime-evidence) | 0139152b9eeb778a23ec4f3197c57faff749a78a | Uploaded Stage18/17/16 runtime logs plus the matching Stage18 overlay and runtime analysis; Build.log not uploaded |
| 19 | [stage19-bit113-state-map-trace](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-trace) | `841142895a976419a9bda197be286e7434993067` | State-code conversion observer; runtime evidence preserved separately |
| 19r | [stage19-bit113-state-map-runtime-evidence](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-runtime-evidence) | `3e75c07c7bdbd0827112f6a488936587a41a1b4b` | Stage19/18/17/16 runtime evidence and matching overlay |
| 20 | [stage20-bit113-object-bit2-guard-trace](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-guard-trace) | `11dde281a08a3481517d0cb60d4752393fa0e578` | Build-verified Stage20 source; real object-bit-2 helper wrapper |
| 20r | [stage20-bit113-object-bit2-runtime-evidence](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-runtime-evidence) | `08125627bfbb588aff8e735168c51a1083cb06e6` | 33 positive object-guard samples and three payload-0x1E dispatch events |
| 21 | [stage21-payload-1e-consumer-trace](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-trace) | `d210c3540ddd29fff1795ef8da6d3605958d481a` | Exact r1 source snapshot used for the successful build and runtime test |
| 21r | [stage21-payload-1e-consumer-runtime-evidence](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-runtime-evidence) | `150a666f6f61e58aa67766010e6af4300a51d901` | Archived Build.log and runtime logs; P1 object-guard AL=0 → setter → bit17 set/clear chain |
| 21r2 | [stage21-payload-1e-consumer-trace-r2](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-trace-r2) | `28fd26b76c22b9070eabaa33a85c713876234aaf` | Source correction captures pre-call EDI; Stage22 runtime shows it is a controller-array pointer, not decoded action code |
| 22 | [stage22-payload-1e-post-and-trace](https://github.com/Hawky11/1/tree/stage22-payload-1e-post-and-trace) | `27b711d4b3ee8079060bfbab8f8ecc22713ac917` | Build-tested post-AND controller bit-0x400 observer; Stage21 r2-derived consumer overlay |
| 23 | [stage23-resolver-result-trace](https://github.com/Hawky11/1/tree/stage23-resolver-result-trace) | `abaf5bb48d0920632f7b9e1d7c0316eeda3c330d` | Source-only resolver-result observer at dispatcher CALL 0xEE273 -> 0x52EAC0; corrected EDI dataflow |
| 22r | [stage22-payload-1e-runtime-evidence](https://github.com/Hawky11/1/tree/stage22-payload-1e-runtime-evidence) | `5b9d1bd83d1324d899f4c5f7f17dcaca136e7453` | Stage22 Build.log and runtime logs; 3 post-AND samples and 11/11 Stage18→20→17 correlations |
| 23r | — | — | Pending: Stage23 build and runtime evidence not yet supplied |

Commit dates for these snapshots are 2026-11-08 (GitHub commit metadata, UTC). Verify branch HEADs again before relying on the index.

## Stage 12 — queue and event-dispatch diagnostics

Overlay and rationale: `Stage12_Overlay/README_STAGE12.md` on the Stage12 branch.

- Added detailed type-1 event-dispatch records with QPC/tick and before/after queue snapshots.
- Added queue-side diagnostics for EBP/ESP, saved-frame chain, caller/frame checks, and stack context.
- The dispatcher is still called exactly once; the overlay is diagnostic-only.
- Source README explicitly says the overlay was source-only and required a successful Windows/MSVC x86 build plus runtime verification.

## Stage 13 — alternate queue callsite

Overlay and rationale: `Stage13_Overlay/README_STAGE13.md` on the Stage13/15 branches.

- Added a passive observer at `AALib.exe+0xF6AD7`, expected to be a near CALL to `AALib.exe+0xF2DE0`.
- Captures registers, EFLAGS, return address, original stack arguments, stack/frame memory, and a code window.
- The hook validates the original instruction bytes and decoded CALL target before installation; it restores registers and EFLAGS before tail-jumping.
- The Stage13 README explains that Stage12 runtime traces had shown type-1 insertions from caller return RVA `0xF6ADC`, P1/P2, payload `0x1E`.

## Stage 14 — producer and guard-call diagnostics

Overlay and rationale: `Stage14_Overlay/README_STAGE14.md`.

- Added probes around the parent routine at `AALib.exe+0xF69F0` and helper calls/returns:
  - `F6A14` / `F6A19`: call to / return from `F2F50`.
  - `F6A53` / `F6A58`: call to / return from `F2ED0`.
- The Stage14 README records four observed cases in which F2F50 returned `0x11E`, F2ED0 returned `AL=0`, but only one case proceeded to the alternate queue writer. Stage15 was created to identify the intervening branch.
- Probes validate original bytes/targets and use buffered logging. They are observational and do not alter arguments, payloads, queue state, or helper return values.

## Stage 15 — post-guard branch-flow diagnostics

Overlay and rationale: `Stage15_Overlay/README_STAGE15.md`.

Five probes were added between the F2ED0 result and the alternate insertion:

| Probe | RVA | Observed decision |
|---|---:|---|
| `F6A5D_F2ED0_BRANCH` | `0xF6A5D` | Original JNE after `test al,al` |
| `F6A63_PAYLOAD_BRANCH` | `0xF6A63` | `cmp edi,1Eh; jne F6AC8` |
| `F6A68_PLAYER_BIT17_BRANCH` | `0xF6A68` | Bit 17 of `[player+0x3B84]`; `je F6A9D` |
| `F6A9D_LOCAL_GUARD_BRANCH` | `0xF6A9D` | `cmp [ebp-4],2; jne F6AC8` |
| `F6AAC_SECONDARY_HELPER_BRANCH` | `0xF6AAC` | Secondary-helper return test |

### Stage15 runtime result (latest reviewed run)

Source for this result: the Stage15 runtime-log analysis completed in the previous chat; branch/commit reference above. The current Stage15 branch contains the corresponding `ArcanaAI_ProducerCallTrace.log`, queue, dispatch, alternate-call, final-command, and related logs.

- Installation: all five Stage15 probes reported installed (`5/5`). Stage14 also reported `5/5`.
- Stage15 branch trace: 13 Stage15 records; all had `fieldsMask=0x0FFF`; local variables were readable (`localRead=(1,1)`).
- Four relevant invocations returned `0x11E` from F2F50. In all four F2ED0 returned `AL=0`; `EDI=0x1E` passed the payload comparison.
- The differentiating observation was bit 17 at `[player+0x3B84]`:

| Time | Player | `[player+0x3B84]` | Bit 17 | Reached alternate writer F6AD7 |
|---|---|---:|---:|---|
| 12:16:34.696 | P1 | `0x00020000` | 1 | No |
| 12:16:36.871 | P2 | `0x00020000` | 1 | No |
| 12:16:45.020 | P2 | `0x00000000` | 0 | Yes |
| 12:16:47.433 | P1 | `0x00020000` | 1 | No |

- In the successful P2 case at 12:16:45.020, the ordered trace was F6A5D (AL=0; JNE not taken), F6A63 (EDI=0x1E; mismatch branch not taken), F6A68 (bit17=0; JE F6A9D taken), F6A9D (`localFC=0`; JNE F6AC8 taken), and then F6AD7 with `type=1`, `payload=0x1E`.
- QueueTrace then recorded `ACCEPTED_BY_QUEUE`; the saved queue pair was `(0001,001E)`. EventDispatchTrace received the same arguments and reported `beforeMatches=1`, `afterMatches=1`.
- `F6AAC_SECONDARY_HELPER_BRANCH` had no records in this run. That matches the observed successful route, which passed through the common F6AC8 path to F6AD7 and bypassed F6AAC.
- The absence of payload `0x1E` in `FinalCommandTrace` is not evidence of event loss: that trace watches a different callsite (F7C62), whereas this event was inserted via F6AD7.
- The reviewed sequence numbers were continuous from `seq=1` through `seq=10833`, with no gaps reported in the prior analysis.
- **Build.log was not present in the uploaded Stage15 set.** Runtime installation was observed, but compiler warnings/build output were not independently reviewed.

### Stage15 interpretation and limits

The observed samples strongly associate the alternate insertion with bit17 being clear. In this run, all three sampled invocations with bit17 set bypassed F6AD7, while the one with bit17 clear reached it. This is a four-event observation, not yet proof of the complete game-wide rule or of who controls the flag.

The Stage15 branch itself does not identify the writer(s) of `[player+0x3B84]`. That is the next research target.

## Current hypothesis and next investigation

**Primary lead:** bit 17 of `[player+0x3B84]` gates whether the type-1 / payload-`0x1E` event reaches the alternate queue callsite at `F6AD7`.

Next, identify every code path that sets, clears, or otherwise changes that bit. Start with static disassembly/references to the player field and enumerate candidate writers. Then add a Stage16 diagnostic observer at validated write sites, capturing at minimum:

- player pointer and effective target address;
- old and new `[player+0x3B84]` values (or the affected byte/word and reconstructed DWORD);
- whether bit 17 changed and its old/new state;
- write instruction RVA and caller/return RVA;
- thread ID, QPC/timestamp, registers, and enough call context to distinguish P1 from P2.

Do not guess writer addresses or install speculative hooks. Validate instruction bytes and write semantics first. Keep Stage12–Stage15 branches intact and correlate Stage16 writer events with the existing Stage15 branch and queue traces.


## Stage 16 — player bit-17 writer trace (implementation prepared; runtime pending)

Branch: [stage16-player-bit17-writer-trace](https://github.com/Hawky11/1/tree/stage16-player-bit17-writer-trace). Base: exact Stage15 HEAD `4d1e2a049c835a991647cb04565d31d330aeac95`. Stage16 HEAD at the last documentation update: `590675c3db91ea0b4def838bce91908b7b97c667`.

### Static review

The previously supplied `AALib.zip` was materialized from the Library and its executable fingerprint was verified:

- SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
- PE32 i386, ImageBase `0x00400000`, SizeOfImage `0x0E0EB000`, EntryPoint RVA `0x0020EA6C`.

An Intel-syntax objdump scan found 107 direct references with displacement `+0x3B84`, including 60 write-like RMW instructions. The most relevant explicit clear is at VA `0x004ED19F` / RVA `0x000ED19F`:

```asm
and dword ptr [esi+3B84h], 0xFFF9FFFF
```

It clears bits 17 and 18. The path immediately before it pushes payload `0x1E`, type `1`, and a player subobject pointer into `AALib.exe+0x4F2ED0`; it runs the clear only when the helper returns `AL=0`. This is a static candidate path; runtime evidence must establish that ESI points to the same P1/P2 object addresses and that this path executes during the Stage15 scenario.

No immediate `OR [reg+0x3B84],0x00020000` was found. Two dynamic OR sites were resolved in context: `0x004F7BD4` ORs EAX=1 (bit 0), while `0x005BF48C` and `0x005BF495` OR EAX=0x00800000 (bit 23). Several dynamic AND masks were also decoded as affecting other bits. Full review: `Stage16_Overlay/STATIC_XREF_REVIEW.md` and `Stage16_Overlay/WRITE_XREFS_3B84.txt`.

### Stage16 instrumentation prepared

`Stage16_Overlay/src/AITrace.cpp` extends the already-compiled Stage11 `AITrace.cpp` module; no new project source or Build.bat changes are required.

- Derives each player object from the previously confirmed P1/P2 control DWORD RVAs by subtracting `0x3F2C`, then watches player+ `0x3B84`.
- Uses DR0/DR1 write-only 4-byte data watchpoints, DR7 configuration `0x00DD0005`.
- Requires DR0 and DR1 address registers to be zero and all DR0/DR1 enable and RW/LEN bits to be clear (`DR7 & 0x00FF000F == 0`) before installing in a game thread. It preserves DR2/DR3, excludes the COMvsCOM worker thread, and does not overwrite configured DR0/DR1 slots.
- Refuses to run this mode together with the legacy `EnableControlFieldTrace` mode, since both share DR0/DR1. Supplied Stage16 INI sets the legacy key to 0 and `EnablePlayerBit17WriteTrace=1`.
- Records writes from any user-mode module in the target process, not only AALib.exe. Logs absolute post-write EIP, `inAALib=0/1`, AALib RVA when applicable, side, TID, local time, tick, QPC, old/new DWORD (old as a best-effort shadow), bit17 status, registers, EFLAGS, DR6, and 24 code bytes around EIP. A worker flushes a 4096-record buffer to `ArcanaAI_PlayerBit17WriteTrace.log`.
- Logs skipped threads and dropped records. If a thread is skipped or a write is missed before a watchpoint is installed, the old-value shadow may be stale; the new value and post-write code context remain the primary evidence.

### Current status / next operation

Stage16 is **source-only**. It has not been compiled with MSVC and has not been installed or runtime-tested here. No Stage16 `Build.log` or runtime log has been uploaded. Do not report Stage16 as successful until the user supplies the build and runtime evidence.

Next: follow `Stage16_Overlay/README_STAGE16.md` to assemble the Stage11 base plus Stage12–Stage16 overlays, build with MSVC x86, run a short COM-vs-COM session, close normally, and upload the fresh logs. Compare each write event to Stage15 F6A68/F6A9D/F6AD7, QueueTrace, and EventDispatchTrace. Do not infer the flag setter from the static candidate alone.


## Stage 16 runtime results — 2026-11-08

Run window: `13:02:27.719`–`13:03:18.651`. Runtime analysis: [Stage16_Overlay/RUNTIME_ANALYSIS.md](https://github.com/Hawky11/1/blob/stage16-player-bit17-writer-trace/Stage16_Overlay/RUNTIME_ANALYSIS.md).

The Stage16 player-field log contains 30,114 `PLAYER_BIT17_WRITE` records. It starts with exact P1/P2 player and field addresses, initial DWORD values, and `DR7=0x00DD0005`. Eight installation batches report 60 thread watchpoint installations; there are no skipped-thread, dropped-record, or warning/error entries in the Stage16 log. The other Stage15/12 queue, dispatcher, producer and alternate-callsite logs are also present.

### Confirmed setter

The five observed 0→1 transitions all execute the same generic bitfield setter:
- Instruction at VA `0x0040FAD0` / RVA `0x0000FAD0`: `or dword ptr [eax],edx`.
- In each bit17 transition, EAX equals the exact watched P1/P2 field address and EDX=`0x00020000`.
- The instruction belongs to helper VA `0x0040FAA0` / RVA `0x0000FAA0`, which accepts a bit index and uses player bitfield base `+0x3B78`.
- Index `0x71` (113) maps to DWORD offset `+0x3B84`, bit remainder 17, mask `0x00020000`.
- The helper log also records 75 writes with EDX=`0x00400000` (bit 22), which do not set bit17.

A static search of 129 direct references to the generic helper found one literal-index caller at VA `0x004C6502`. It performs a generic test of bit index 2 at `0x00495B00`; when AL=0 it calls the bitfield helper with index `0x71` and player in ESI. This is a strong static candidate, but Stage16 did not record the helper's caller return address, so this callsite is not yet runtime-proven.

### Confirmed clearer

All five observed 1→0 transitions came from VA `0x004ED19F` / RVA `0x000ED19F`:

```asm
and dword ptr [esi+3B84h], 0xFFF9FFFF
```

For each transition ESI was the exact watched P1/P2 player object, and the preceding `0x004F2ED0` helper returned AL=0. The mask clears bits 17 and 18. The write watchpoint saw 597 executions at the clearer, but only five changed bit17 from 1 to 0.

### Correlation with the event path

Six matching Stage15 parent events were observed in this run. For all six, F2F50 returned EAX=`0x11E`, F2ED0 returned AL=0, and EDI=`0x1E` passed the payload compare.

Five events had bit17 set when F6A68 ran. In each case the generic setter set bit17 before the branch, F6A68's original JZ was not taken, and the event did not reach F6A9D/F6AD7. The clearer subsequently reset bit17.

The sixth event (P1 at `13:03:12.258`) arrived at F6A68 with bit17=0; the JZ was taken, F6A9D was reached, and F6AD7 inserted `type=1,payload=0x1E`. QueueTrace reported `ACCEPTED_BY_QUEUE`; EventDispatchTrace confirmed the same pair. FinalCommandTrace correctly has no matching payload-`0x1E` record because that event used the alternate writer at F6AD7, not F7C5D.

This run therefore confirms the bit17 set/clear instructions and strongly correlates the temporary flag with suppression at F6A68. The sample is six parent events; do not overgeneralize it to all matches or all AI events.

### Build-log limitation and next target

No `Build.log` was uploaded. Runtime logs prove that the module was active and its watchpoints installed, but compiler output and build warnings were not independently reviewed.

Next diagnostic question: which runtime caller supplies bit index `0x71` to `0x0040FAA0`? Stage17 should observe entry to the generic setter, record its caller return address, ECX/player, stack bit-index argument and relevant surrounding state, and verify or reject the static candidate at `0x004C6502`. Preserve the Stage16 and Stage12–Stage15 branches as evidence snapshots.


## Stage 17 — bit-113 setter caller trace (source prepared; runtime pending)

Branch: [stage17-bit113-setter-caller-trace](https://github.com/Hawky11/1/tree/stage17-bit113-setter-caller-trace), created from the Stage16 runtime-evidence branch at HEAD `58313d8c0f6071bc8099fc5ec04ae04d2974dafd`. Stage17 HEAD at this documentation update: `6b04b0a9ad272aecf8dcb481b5c52edfe5acfe1e`.

### Why Stage17 is needed

Stage16 confirmed that all five observed 0→1 changes to player bit17 executed the final OR at VA `0x0040FAD0` with EAX equal to the exact watched P1/P2 `player+0x3B84` field and EDX=`0x00020000`. The instruction belongs to a generic bitfield setter at VA `0x0040FAA0` / RVA `0x0000FAA0`; its input bit index `0x71` (113) maps to player+0x3B84, bit 17.

The strongest static caller candidate is VA `0x004C6502` / RVA `0x000C6502`. It performs a helper test and, on AL=0, pushes `0x71`, moves the player into ECX, and calls `0x0040FAA0`. The expected return address is VA `0x004C6507` / RVA `0x000C6507`. The call-site evidence is documented in `Stage17_Overlay/STATIC_CALLER_REVIEW.md`. This candidate was not runtime-proven by Stage16 because Stage16 did not capture the setter's caller return address.

### Prepared instrumentation

`Stage17_Overlay/src/AICommandSiteTrace.cpp` adds a narrowly filtered entry hook to the existing Stage14/15 probe module:

- Validates the exact seven-byte prologue at RVA `0xFAA0`: `55 8B EC 56 8B 75 08`. Refuses to patch if bytes differ.
- At entry, PUSHFD occurs before checking the bit-index argument. Only calls with stack argument `0x71` invoke probe 11.
- Preserves EFLAGS/registers, replays the original seven-byte prologue and continues at RVA `0xFAA7`.
- Interprets ECX as the player object; records player fields, CPU control, registers, EFLAGS/ZF, caller frame, original return address and bit-index argument.
- Reads the preceding five bytes as the caller CALL, decodes the E8 rel32 destination and emits `callTargetVerified=1` only when the destination equals the verified setter entry.
- Installs the setter hook before enabling the Stage14/15 parent-path probes to reduce the chance of missing the first relevant call.
- Reuses the existing bounded Stage14 probe ring; Stage17 entries route to `ArcanaAI_Bit113SetterTrace.log`. The worker flushes this ring even when Stage14 itself is disabled and reports ring drops to the Stage17 log.

The candidate should produce call RVA `0x000C6502`, return RVA `0x000C6507`, decoded target `0x0040FAA0`, bit index `0x71`, and entry ZF=1 if the preceding AL=0 branch condition is the one that led to the setter. Do not assume that candidate is the only caller.

### Current status / next operation

Stage17 is **source-only**. It has not been compiled with MSVC, installed, or runtime-tested. The Stage16 runtime logs are still the latest evidence; no Stage17 `Build.log` or setter-caller trace exists yet.

Use `Stage17_Overlay/README_STAGE17.md` for the exact overlay order and install procedure. Build with the existing x86 toolchain, retain `Build.log`, then run a short COM-vs-COM session and upload the fresh `ArcanaAI_Bit113SetterTrace.log` together with Stage16/Stage15 correlation logs. Only claim the static candidate is confirmed if a runtime record reports the expected return RVA and `callTargetVerified=1`.


## Stage 17 build attempt — 2026-11-08 (compile blocked)

The first reported MSVC x86 build used Visual Studio 2026 18.9.1 / MSVC 14.51.36231 and stopped while compiling `src/AITrace.cpp`. It emitted repeated C3861 errors stating that `ReadDwordAtAddress` was not declared, at 14 call sites (reported around source lines 465–484). Compilation did not reach a successful link; no Stage17 DLL should be installed from this attempt.

### Source comparison

The current Stage17 branch's `Stage16_Overlay/src/AITrace.cpp` is byte-for-byte the same Git blob as the Stage16 evidence branch's corresponding file (blob `2a40bfabc2cfe6b3a39ba67ccbf5257deac389a2`). That tracked file contains both the `ReadDwordAtAddress(DWORD, DWORD*)` implementation and its call sites, with the implementation in the same anonymous namespace before the uses. Therefore the pasted diagnostics do not match compilation of the tracked Stage16 overlay as currently published; a stale, partially merged, or otherwise different local `src/AITrace.cpp` is the leading explanation. This is a source comparison, not proof of the exact local mismatch because the local file was not included with the log.

### Corrective source revision

Stage17 was updated without modifying Stage16 or any earlier evidence branch:

- Stage17 branch HEAD: `6b04b0a9ad272aecf8dcb481b5c52edfe5acfe1e`.
- Added an explicit forward declaration for `ReadDwordAtAddress(DWORD, DWORD*)` near the start of the anonymous namespace in `Stage16_Overlay/src/AITrace.cpp`; the existing implementation remains in place.
- Expanded `Stage17_Overlay/README_STAGE17.md` with the exact overlay-copy requirement and a PowerShell check that verifies the local `src/AITrace.cpp` contains both the declaration and implementation.

This is a defensive source fix and build-path clarification, **not a verified build fix**. Stage17 remains uncompiled and untested until a fresh MSVC x86 Build.log shows a successful compile/link. If the same C3861 errors remain after copying the current Stage16 overlay, inspect the exact local `src/AITrace.cpp` rather than changing hook logic blindly.


## Stage 17 callsite build and runtime follow-up — 2026-11-08 (caller log pending)

Evidence branch: [stage17-bit113-setter-callsite-trace](https://github.com/Hawky11/1/tree/stage17-bit113-setter-callsite-trace), HEAD `0c2411187c47e56a6cb675b597eeb8a754f8f8a6`.

This is a separate Stage17 implementation snapshot from the earlier `stage17-bit113-setter-caller-trace` branch. The callsite variant replaces the five-byte CALL at RVA `0x000C6502` with a passive observer that tail-jumps to the original generic bitset setter at RVA `0x0000FAA0`. Its supplied INI enables `EnableStage17Bit113SetterCallTrace=1`. The expected original CALL bytes are `E8 99 95 F4 FF`, resolving to the setter; expected return RVA is `0x000C6507`.

### Build result

The user supplied the complete Build.log in chat. It reports:
- Visual Studio 2026 18.9.1, MSVC 14.51.36231, x86 environment initialized successfully.
- All eight translation units compiled: Proxy.cpp, ComVsCom.cpp, AITrace.cpp, AICommandTrace.cpp, AIQueueTrace.cpp, AICommandSiteTrace.cpp, AIProducerContextTrace.cpp, AIParentFlowTrace.cpp.
- Linking succeeded: `build\\dinput8.dll`, 190,976 bytes.
- One non-fatal warning C4754 in `AICommandSiteTrace.cpp` lines 782–785. It concerns the mixed-width intermediate in the stack snapshot address calculation:
  `static_cast<std::uint64_t>(stackBase) + i * sizeof(DWORD)`, followed by a 32-bit address bound check. The compiler suggests widening the `i * sizeof(DWORD)` subexpression. This warning did not prevent compilation or linking and is not evidence of a Stage17 hook failure.
- The Build.log was shared in conversation but was not included in the uploaded branch tree.

### Runtime run: 2026-11-08, approximately 15:40:27–15:41:09

The new branch contains Stage16/15/14/12 correlation logs from this session:
- `ArcanaAI_PlayerBit17WriteTrace.log`: 22,739 `PLAYER_BIT17_WRITE` records; 60 game-thread watchpoint installations; no warning/error/drop lines in the file.
- `ArcanaAI_ProducerCallTrace.log`: Stage14 and Stage15 probes; Stage15 branch counts are six F6A5D, six F6A63, six F6A68, three F6A9D, and zero F6AAC records.
- `ArcanaAI_AlternateCallTrace.log`: three relevant alternate writer calls at F6AD7.
- `ArcanaAI_QueueTrace.log`: all three corresponding type-1 / payload-`0x1E` inserts report `ACCEPTED_BY_QUEUE`.
- `ArcanaAI_EventDispatchTrace.log`: all three matching type-1 / payload-`0x1E` events are seen by the dispatcher with `beforeMatches=1` and `afterMatches=1`.
- The reviewed PlayerBit17Write, ProducerCall, Queue and EventDispatch logs contain no warning/error/drop reports.

### Reconfirmed writer/clearer sequence

Three actual 0→1 bit17 changes were captured, each immediately associated by QPC with an F6A68 sample for the same player:
| Time | Player | Setter post-EIP / RVA | Setter EAX | Setter EDX | Clear post-EIP / RVA |
|---|---|---|---|---|---|
| 15:40:46.482 | P2 | `0x00D2FAD2 / 0xFAD2` | `0x0E9F0824` | `0x00020000` | `0x00E0D1A9 / 0xED1A9` |
| 15:40:53.020 | P2 | `0x00D2FAD2 / 0xFAD2` | `0x0E9F0824` | `0x00020000` | `0x00E0D1A9 / 0xED1A9` |
| 15:41:01.431 | P1 | `0x00D2FAD2 / 0xFAD2` | `0x0E9C5AFC` | `0x00020000` | `0x00E0D1A9 / 0xED1A9` |

The setter instruction itself remains VA `0x0040FAD0` (`or [eax],edx`); the logged EIP is after it. The clearer instruction itself remains VA `0x004ED19F` (`and [esi+3B84h],0xFFF9FFFF`); the logged EIP is after it. Each clear changed the watched DWORD from `0x00030000` to `0x00010000`, clearing bit17.

### Six event outcomes

All six matching parent events had F2F50 return EAX=`0x11E`, F2ED0 return AL=0, and EDI=`0x1E` at the relevant payload/branch check.

| Time | Player | bit17 at F6A68 | Original JZ | Result |
|---|---|---:|---|---|
| 15:40:44.850 | P1 | 0 | Taken | F6AD7; accepted and dispatched |
| 15:40:46.482 | P2 | 1 | Not taken | Suppressed before F6A9D/F6AD7 |
| 15:40:53.020 | P2 | 1 | Not taken | Suppressed before F6A9D/F6AD7 |
| 15:40:53.351 | P1 | 0 | Taken | F6AD7; accepted and dispatched |
| 15:41:00.061 | P2 | 0 | Taken | F6AD7; accepted and dispatched |
| 15:41:01.431 | P1 | 1 | Not taken | Suppressed before F6A9D/F6AD7 |

The three bit17=1 cases each have a matching 0→1 setter write before F6A68 and a 1→0 clear afterward. The three bit17=0 cases each reach the alternate queue and EventDispatch. This is a second independent six-event sample supporting the bit17/F6A68 relationship; it still does not establish the full semantic meaning of bit113.

### Important missing Stage17 evidence

The uploaded branch tree does **not** contain `ArcanaAI_Bit113SetterTrace.log`. The callsite variant writes its own log and should record either:
- an installation line for the passive observer at RVA `0x000C6502`, or
- a validation/patch failure explaining why it refused to install,
followed by `BIT113_SET_CALL` records if the callsite executes.

The expected records expose `callRVA=0x000C6502`, `returnRVA=0x000C6507`, `index=0x00000071`, P1/P2 player object, registers, related object fields, stack and QPC. Without this file, the newly tested runtime still does **not** confirm that the three setter calls came from the static candidate at `0x004C6502`; the candidate remains unproven.

Next operation: obtain/upload only `ArcanaAI_Bit113SetterTrace.log` from the same game/output folder as the other ArcanaAI logs. Do not request another build or match unless the file was never created. If it was not created, check the active game-folder `ArcanaAI.ini` for `EnableStage17Bit113SetterCallTrace=1` and confirm the just-built DLL was installed in the game directory before arranging a targeted rerun. Keep this evidence branch immutable.


## Stage17 callsite missing-log diagnosis — 2026-11-08

The 2026-11-08 15:40:27–15:41:09 run produced the Stage16 write/branch/queue/dispatch logs but did not produce `ArcanaAI_Bit113SetterTrace.log`. The successful Build.log was pasted in chat (eight translation units, linked DLL size 190,976 bytes), but it is not stored in the evidence branch, and the log does not prove which exact `AITrace.cpp` contents were compiled.

Review of the original callsite-variant source at `stage17-bit113-setter-callsite-trace` HEAD `0c2411187c47e56a6cb675b597eeb8a754f8f8a6` found:

- The Stage17 logger itself was present and the supplied Stage17 INI set `EnableStage17Bit113SetterCallTrace=1`.
- However, several early exits in `FieldTraceThread` (conflicting DR0/DR1 modes, missing `AALib.exe`, invalid image/ranges, or failed initial P1/P2 reads) returned before writing to the Stage17-specific log. Therefore, absence of the file did not distinguish a stale DLL/source from an initialization failure.
- `AppendStage17Bit113Log` used a double-escaped `\\r\\n` format sequence, which wrote literal backslash characters instead of normal CRLF separators. This did not explain the missing file, but it was corrected.

A new source-only diagnostic branch was created from that evidence snapshot; the original run branch remains unchanged:

- Branch: [stage17-callsite-init-diagnostics](https://github.com/Hawky11/1/tree/stage17-callsite-init-diagnostics)
- HEAD: `c33317c9b2fa28c339c38f3aaebdcb9a378b73a1`
- Source change commit: `fbf466a151ea327a498b19741cda92787ffff46d`
- README/checklist commit: `c33317c9b2fa28c339c38f3aaebdcb9a378b73a1`

The revised source writes `STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r1`, a worker-entry marker, and the three relevant INI values before validation. It also records Stage17-specific errors on the former early-return paths and fixes CRLF formatting. The README adds a PowerShell source-marker check before build and requires installing the new DLL and matching INI beside the actual game executable.

**No new MSVC build or runtime test has been performed for this revision.** This change is an observability fix, not evidence that the callsite hook installed. Next, apply the diagnostic overlay after Stage16, verify the marker in the assembled local `src\\AITrace.cpp`, rebuild x86, install the new DLL/INI, and launch the game only long enough to confirm the Stage17 marker/config/install result. If the marker appears and reports the hook installed, run a short COM-vs-COM session and compare `BIT113_SET_CALL` QPCs with Stage16 bit17 writes. If no marker file appears while other ArcanaAI logs are written, stop and verify which proxy DLL the game actually loaded; do not repeat a match blindly.


## Stage17 diagnostic build error and source correction — 2026-11-08

The user verified the r1 source marker in `D:\\INCUBATOR\\ArcanaAIOverhaul\\src\\AITrace.cpp`, proving the correct diagnostic source had been copied into the assembled project. The subsequent MSVC x86 build stopped in `AITrace.cpp` with 14 C3861 errors: `ReadDwordAtAddress` was called at lines 466–485 before any declaration was visible.

The exact r1 source inspection confirmed:
- first uses were inside `QueueStage17Bit113SetterRecord` (around line 466);
- the actual definition was later in the same anonymous namespace (around line 603);
- the implementation signature is `bool ReadDwordAtAddress(DWORD address, DWORD* value)`.

A minimal source-only correction was committed to `stage17-callsite-init-diagnostics`:
- forward declaration `bool ReadDwordAtAddress(DWORD address, DWORD* value);` inserted immediately before `QueueStage17Bit113SetterRecord`, inside the existing anonymous namespace;
- existing implementation left unchanged;
- no hook logic, callsite bytes, logger behavior, or runtime tracing semantics changed.

Current branch HEAD: `34fbf14975b3e5d1b9dd6bdec0c3d6c8756bc25f` (source fix commit `1da019d543be29df58f158cf9aecc8ecfc7eb85e`; README clarification commit `34fbf14975b3e5d1b9dd6bdec0c3d6c8756bc25f`). Source inspection confirms declaration at line 422 and implementation at line 607, with all earlier calls now following the declaration.

The user's reported build failed before link. Do not install any DLL from this attempt and do not treat it as a Stage17 build. The local build script deletes its project-local `build\\dinput8.dll` before compilation; the previously installed game-folder DLL is not thereby replaced.

**Next:** copy the corrected `Stage17_Overlay/src/AITrace.cpp` from this branch into the assembled project, verify the existing marker again, rerun Build.bat, and provide the new Build.log. Do not run another game test until a successful link. The current corrected source has not yet been compiled in MSVC in this environment.


### Stage17 corrected-source marker revision r2 — 2026-11-08

After inserting the forward declaration, the diagnostic build marker was bumped from `STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r1` to the distinct `STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r2`. This prevents a stale r1 DLL from being mistaken for the corrected r2 source. The Stage17 README's PowerShell check and expected runtime marker were updated accordingly.

Current source branch: `stage17-callsite-init-diagnostics`; latest HEAD after README update: `ebabe7faa0521115425e56d9b6880b76e1a48806`. This revision is still source-only; no MSVC build result is available yet.


## Stage17 diagnostic r2 build succeeded — 2026-11-08 16:32

The user supplied the next MSVC x86 Build.log from `D:\\INCUBATOR\\ArcanaAIOverhaul`, started 2026-11-08 at 16:32:02. The local source marker check had already returned the r1 marker before the compile-fix update; the corrected r2 source has since been verified in GitHub at branch `stage17-callsite-init-diagnostics`, HEAD `ebabe7faa0521115425e56d9b6880b76e1a48806`.

Build results in the supplied log:
- Visual Studio 2026 18.9.1 / MSVC 14.51.36231, x86 environment initialized successfully.
- All eight C++ translation units compiled, including `AITrace.cpp` and the existing Stage14/15 `AICommandSiteTrace.cpp`.
- Link succeeded and created `build\\dinput8.dll`.
- DLL size: **198,144 bytes**.
- The previous 14 C3861 errors for `ReadDwordAtAddress` are gone.
- One non-fatal C4754 warning remains at `AICommandSiteTrace.cpp` lines 782–785, related to mixed-width arithmetic in the stack-snapshot address bound calculation. It did not prevent compile or link.

This is the first successful build of the corrected Stage17 callsite diagnostic revision. It proves compilation/linking, not that the DLL has been installed, loaded, or that the RVA `0xC6502` hook installed.

### Next runtime verification

1. Copy the new 198,144-byte `build\\dinput8.dll` and the matching Stage17 `ArcanaAI.ini` beside the actual game executable / `AALib.exe`, keeping a backup of the prior DLL.
2. Launch the game and inspect `ArcanaAI_Bit113SetterTrace.log` before running another full COM-vs-COM test.
3. Its first lines must contain `STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r2`, `STAGE17_INIT`, and `STAGE17_CONFIG` with `EnableStage17Bit113SetterCallTrace=1`.
4. Inspect the hook installation result. If it reports installation at RVA `0x000C6502`, then conduct a short COM-vs-COM session to obtain `BIT113_SET_CALL` entries and correlate their QPC/player pointers with Stage16 bit17 writes.
5. If other ArcanaAI logs appear but the Stage17 marker does not, stop and determine which DLL/INI pair the game actually loaded. Do not repeat a full test blindly.

The Stage17 callsite candidate remains runtime-unconfirmed until the log proves successful hook installation and matching `BIT113_SET_CALL` records.


## Stage 17 runtime result — direct bit-113 caller confirmed (2026-11-08)

Evidence branch: [stage17-callsite-init-diagnostics](https://github.com/Hawky11/1/tree/stage17-callsite-init-diagnostics), uploaded runtime-log HEAD `dcfd1efbe221438b1f2b3fae800ca76b1fa87e7d`. This contains the corrected r2 source plus the fresh 16:34 runtime logs. The earlier `stage17-bit113-setter-callsite-trace` branch remains an unchanged pre-fix snapshot.

### Build and initialization

The user supplied Build.log from 2026-11-08 16:32:02:
- Visual Studio 2026 18.9.1 / MSVC 14.51.36231, x86.
- All eight C++ translation units compiled.
- Link succeeded; `build\\dinput8.dll` size **198,144 bytes**.
- Non-fatal C4754 remains in `AICommandSiteTrace.cpp` lines 782–785; it did not prevent linking.

Runtime initialization at 16:34:05:
- `STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r2`
- `FieldTraceThread` entered.
- INI values: `EnableControlFieldTrace=0`, `EnablePlayerBit17WriteTrace=1`, `EnableStage17Bit113SetterCallTrace=1`.
- Installed the passive observer at RVA `0x000C6502`, return RVA `0x000C6507`, original helper RVA `0x0000FAA0`; 34 process threads were suspended during the patch.
- No Stage17 error, warning, or ring-drop line was present.

### Direct runtime attribution

`ArcanaAI_Bit113SetterTrace.log` contains nine `BIT113_SET_CALL` records. All report:
- `callRVA=0x000C6502`
- `returnRVA=0x000C6507`
- `index=0x00000071`
- The exact P1/P2 object pointer in both `player` and `ECX/ESI`.

The Stage16 player-field log contains 20,456 `PLAYER_BIT17_WRITE` records and eight actual bit17 transitions: four 0→1 sets and four 1→0 clears. The Stage16 worker installed 62 write-watchpoint instances across 10 installation batches. No warning/error/drop/skip report was present in the reviewed PlayerBit17WriteTrace.

Every one of the four observed 0→1 writes at RVA `0x0000FAD2` has a preceding Stage17 call from the candidate site with the same side and player object. The Stage17 call observed `player+0x3B84=0`; the Stage16 write immediately followed from the original helper and set that field to `0x00020000`.

| Player | Time | Stage17 call seq | Stage17 QPC | Stage16 setter QPC | Delta |
|---|---|---:|---:|---:|---:|
| P2 | 16:34:29.538 | 1 | 7940066152498 | 7940066152865 | 367 |
| P1 | 16:34:30.940 | 3 | 7940080167261 | 7940080167781 | 520 |
| P1 | 16:34:36.957 | 5 | 7940140346097 | 7940140346493 | 396 |
| P2 | 16:34:43.566 | 7 | 7940206436096 | 7940206436449 | 353 |

Thus the static candidate at `AALib.exe+0xC6502` is **runtime-confirmed as the caller that set bit17 in all four observed transitions in this run**. Five additional Stage17 calls (seq 2, 4, 6, 8, 9) saw `player+0x3B84=0x00020000` already set; these are repeated/idempotent calls and did not correspond to additional 0→1 transitions. The callsite is confirmed for the observed scenario, not proven to be the only possible setter path in every game state.

All four corresponding clears were again observed at RVA `0x000ED1A9` (post-EIP for instruction `0x000ED19F`), with the watched DWORD transitioning from `0x00030000` to `0x00010000`.

### Relationship to the event suppression

The new ProducerCallTrace has four Stage15 records at each of:
- `F6A5D_F2ED0_BRANCH`
- `F6A63_PAYLOAD_BRANCH`
- `F6A68_PLAYER_BIT17_BRANCH`

All four F6A68 records saw bit17=1 and recorded `branchTaken=0` (the original JZ was not taken). There were zero F6A9D local-guard records and zero F6AAC secondary-helper records. The alternate-callsite hook at F6AD7 installed successfully but recorded no actual calls. QueueTrace and EventDispatchTrace contain no type-1 / payload-`0x1E` insert/dispatch in this run. The sample therefore confirms the suppression side of the bit17 gate; it does not add a new successful type-1 / payload-`0x1E` insertion sample.

### Caller context and remaining question

At the Stage17 callsite, the captured `otherObject` pointer equals the player pointer, and `other3B70` was `0x650` or `0x640`; bit index 2 is clear in both values, which is consistent with the final `0x495B00` guard allowing the setter call.

The captured low words at player `+0x3A36` were `0x42`, `0x22`, `0x20`, and `0x5E`. These differ from the `0x1A/0x18/0x19` values in the documented straight-line comparison sequence preceding the call. This does not invalidate the runtime callsite match; the hook is installed at the exact validated CALL and the setter transitions follow it. It means the next static/dynamic review should reconstruct which earlier branch paths enter the final block, and whether any helper between that comparison and the call changes these fields. Do not infer the semantic meaning of the state fields yet.

### Conclusion

Stage17 has completed its primary goal: it proves that `0x004C6502` passes bit index 113 to the generic bitset setter and initiates all four captured bit17 set transitions. Together with Stage16's runtime-confirmed setter at `0x0040FAD0`, clearer at `0x004ED19F`, and Stage15's F6A68 branch, the origin and suppression chain are now linked by runtime evidence.

Next investigation should focus on the conditions and branch paths inside caller function RVA `0x000C63B0`, particularly the earlier helper/branch gates and the repeated calls while bit17 is already set. Preserve this runtime branch as an evidence snapshot; put any new instrumentation in a new Stage18 branch.


## Stage 18 source overlay — bit-113 state-table gate result observer (2026-11-08)

Source branch: [stage18-bit113-gate-condition-trace](https://github.com/Hawky11/1/tree/stage18-bit113-gate-condition-trace), latest HEAD \`520eec4cd216795e8ffbaa8738b3602b5e40d39a\`. Stage17 runtime evidence branch \`stage17-callsite-init-diagnostics\` remains unchanged and remains the latest completed runtime checkpoint.

### Static reconstruction

The uploaded AALib.exe was rechecked directly (PE32, ImageBase \`0x00400000\`, SizeOfImage \`0x0E0EB000\`, SHA-256 \`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba\`).

At caller RVA \`0x000C63B0\`, the branch relevant to Stage17's confirmed setter call is:

- RVA \`0x000C64A0\`: CALL to helper RVA \`0x000F2FD0\`.
- The call arguments are pushed as \`push 0x19; push EAX; push ESI\`, where EAX came from \`0x000F6ED0\).
- RVA \`0x000C64A5\`: \`83 C4 0C 84 C0\` = \`add esp,0x0c; test al,al\`.
- RVA \`0x000C64AA\`: \`75 41\` = \`jne 0x000C64ED\`.

The four Stage17 calls recorded low words at player \`+0x3A36\` of \`0x42\`, \`0x22\`, \`0x20\`, and \`0x5E\`, not the direct-branch accepted values \`0x1A\`, \`0x18\`, and \`0x19\`. The disassembly shows that the \`0x4F2FD0\` result is the alternate incoming gate: AL=1 takes the JNE to \`0x4C64ED\`, bypassing the direct state comparisons. Stage17 did not log AL, so this remained a strong branch inference rather than a direct runtime observation.

Helper \`0x004F2FD0\`:
- chooses its table through selector helper \`0x004E6E30\`, player field \`+0x3F20\), and optionally a secondary table;
- reads a byte at \`table + stateIndex*0x1A + 0x19\`;
- returns AL=1 iff that byte is not \`0xFF\`.

The special selector helper \`0x004E6E30\` returns true when all three conditions hold:
1. DWORD \`[player+0x3978] == 0x1D\`;
2. bit 0 of byte \`[player+0x3B86]\` is set (bit 16 of DWORD \`[player+0x3B84]\`);
3. bit 28 of DWORD \`[player+0x3B88]\` is set.

It then selects index \`0x16\); otherwise the selector is DWORD \`[player+0x3F20]\`. The primary pointer array is at VA \`0x004C82518\` / RVA \`0x04882518\`. When bit 25 of DWORD \`[player+0x3B80]\` is set, a nonzero matching secondary pointer from VA \`0x004C82C20\` / RVA \`0x04882C20\` overrides the primary pointer.

### Stage18 source changes

The new overlay is under \`Stage18_Overlay/\`; the previous \`Stage17_Overlay/\` snapshot is preserved unchanged in this branch.

- \`src/AITrace.cpp\` adds a passive observer at RVA \`0x000C64A5\`, immediately after the original \`0x4F2FD0\` helper returns.
- The installer validates the argument push sequence \`6A 19 50 56\`, helper CALL bytes/target \`E8 2B CB 02 00\` → \`0x4F2FD0\`, overwritten bytes \`83 C4 0C 84 C0\`, and following branch \`75 41\`.
- The hook captures helper AL/EAX, player/state-index/key arguments, parent frame, selector-driving fields, table pointers and the calculated table byte.
- It removes the observer CALL return plus the original three stack arguments (\`add esp,0x10\`), recreates \`test al,al\`, and resumes at the untouched original JNE. It does not replace the helper result or change the branch decision.
- Table reads use \`ReadProcessMemory\` and bounded selectors/state indices; the actual helper AL is retained as the authoritative result. The direct comparison route at \`0x4C64B1\` bypasses this observer and remains outside Stage18's coverage.
- INI keeps Stage16 and Stage17 active and enables \`EnableStage18Bit113GateTrace=1\`. The new output is \`ArcanaAI_Bit113GateTrace.log\`.

### Verification status and next action

This is a **source-only diagnostic stage**. The branch contains the source, matching INI, README and static disassembly notes. No MSVC build, DLL link, hook installation or runtime capture has been performed for Stage18. Do not treat source presence as proof of installation.

Next:
1. Apply \`Stage18_Overlay/src/AITrace.cpp\` and \`Stage18_Overlay/ArcanaAI.ini\` over the already assembled Stage17 project.
2. Verify marker \`STAGE18_BIT113_GATE_BUILD=2026-11-08-r1\` in the local source, then build \`Release | Win32\` with MSVC and keep Build.log.
3. Only after a successful link, back up the currently working game-folder DLL and install the new DLL with the matching INI.
4. First confirm the Stage18 marker/config/install line in \`ArcanaAI_Bit113GateTrace.log\`; then run a short COM-vs-COM scenario and collect the Stage18, Stage17 and Stage16 logs plus related ProducerCall/AlternateCall/Queue/Dispatch traces.
5. Correlate Stage18 \`resultAL\` with the derived table byte and Stage17 setter calls/QPCs. Distinguish the helper-gated route from the direct comparison path.

Keep the Stage17 runtime branch immutable. If the Stage18 build fails, fix the source before any runtime test; if the build succeeds but the hook does not install, stop and diagnose from the Stage18-specific log rather than repeating the scenario.


## Stage18 runtime result — state-table gate and payload-0x1E suppression (2026-11-08)

Runtime evidence branch: [stage18-bit113-gate-runtime-evidence](https://github.com/Hawky11/1/tree/stage18-bit113-gate-runtime-evidence), HEAD 0139152b9eeb778a23ec4f3197c57faff749a78a. The earlier source-only branch [stage18-bit113-gate-condition-trace](https://github.com/Hawky11/1/tree/stage18-bit113-gate-condition-trace), HEAD 520eec4cd216795e8ffbaa8738b3602b5e40d39a, is preserved as the source snapshot. The runtime branch contains the uploaded logs and the exact Stage18 overlay. The separate analysis is in Stage18_Overlay/STAGE18_RUNTIME_ANALYSIS.md.

### Startup and hook installation

Runtime started on 2026-11-08 at 18:00:15. The logs show:
- Stage18 marker STAGE18_BIT113_GATE_BUILD=2026-11-08-r1;
- EnableStage18Bit113GateTrace=1, EnableStage17Bit113SetterCallTrace=1, and EnablePlayerBit17WriteTrace=1;
- successful Stage18 hook installation at RVA 0x000C64A5, after the helper CALL at 0x000C64A0 to 0x000F2FD0, with 44 threads suspended;
- successful Stage17 hook installation at RVA 0x000C6502, targeting generic SetBit helper 0x0000FAA0, also with 44 threads suspended.

**Build.log was not included in the uploaded files.** The startup markers and install messages prove the loaded DLL ran both observers, but the exact MSVC compiler/linker output and DLL size are not available for this run.

### Gate result and correlation

Stage18 logged six BIT113_GATE_RESULT records. All six returned AL=1; all six post-return table readbacks were 0xFA (not sentinel 0xFF), with key 0x19. The three unique state/table cases were:

| Player | State index | Selector | Byte address | Readback |
|---|---:|---:|---:|---:|
| P1 | 0x0E | 0x0A | 0x0559DBDD | 0xFA |
| P2 | 0x19 | 0x17 | 0x055A073B | 0xFA |
| P1 | 0x18 | 0x0A | 0x0559DCE1 | 0xFA |

The table byte is reread after the helper returns, so it is corroborative; the helper's actual AL is authoritative. No AL=0 example was captured in this run.

Stage17 contains six setter calls, all from caller RVA 0x000C6502, return RVA 0x000C6507, bit index 0x71, and the matching player object. Stage16 contains 15,094 PLAYER_BIT17_WRITE records and six bit17 transitions: three 0→1 sets at RVA 0x0000FAD2 with EDX=0x00020000, and three clears at RVA 0x000ED1A9.

| Episode | Player | Gate→setter | Setter→bit17 set | Bit17 set→F6A68 | F6A68→clear |
|---:|---|---:|---:|---:|---:|
| 1 | P1 | 136 QPC ticks | 342 | 2,015 | 1,572 |
| 2 | P2 | 135 QPC ticks | 343 | 1,974 | 1,767 |
| 3 | P1 | 169 QPC ticks | 441 | 2,537 | 1,989 |

Each episode also contains a second helper/setter call after bit17 is already set. These later calls are idempotent OR operations, not additional 0→1 transitions.

### Why the setter is called twice per episode

The Stage18 caller-frame records split into two parent return addresses:
- 0x00DB6801 (RVA 0x00096801, static VA 0x00496801) follows a direct CALL to 0x004C63B0 at 0x004967FC.
- 0x00DEB9FA (RVA 0x000CB9FA, static VA 0x004CB9FA) follows an indirect CALL EDX at 0x004CB9F8 inside a dispatcher beginning at 0x004CB9C0.

The dispatcher reads a function pointer from table VA 0x0428AEF0 + index*4. Static data inspection confirms entry index 0x0A at VA 0x0428AF18 contains pointer 0x004C63B0. Therefore, both paths invoke the same caller: once directly and once through callback/function-table slot 10. This explains the paired Stage18 gate and Stage17 setter records.

### Target payload-0x1E suppression

The three Stage15 F6A68 records have EDI=0x0000001E, bit17=1, ZF=1, and branchTaken=0. The target attempts were observed at 18:00:35.343 (P1), 18:00:37.836 (P2), and 18:00:42.083 (P1).

There are no later F6A9D or F6AAC branch records, and the F6AD7 alternate queue callsite hook installed but recorded no invocation. QueueTrace, EventDispatchTrace, and FinalCommandTrace each contain 27 type-1 records for other payloads; none has payload 0x1E.

**Conclusion:** Stage18 confirms that the 0x4F2FD0 state-table gate allowed all three observed target episodes (AL=1, readback 0xFA). Stage17/16 then show bit17 being set, and Stage15 confirms that bit17 is the decisive suppression condition for the target payload-0x1E attempts in this run. No successful payload-0x1E insertion or AL=0 table-gate sample was captured. The semantic meaning of byte 0xFA remains unassigned.

All reviewed Stage18, Stage17 setter, Stage16 writer, Stage15 producer, queue, dispatch, and alternate-callsite logs contain no warning/error/drop report.


## Stage19 source overlay — bit-113 state-code mapping (2026-11-08)

Source branch: [stage19-bit113-state-map-trace](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-trace), current HEAD `841142895a976419a9bda197be286e7434993067`. It was branched from the original Stage18 source snapshot [stage18-bit113-gate-condition-trace](https://github.com/Hawky11/1/tree/stage18-bit113-gate-condition-trace), HEAD `520eec4cd216795e8ffbaa8738b3602b5e40d39a`. The Stage18 runtime-evidence branch remains separate and unchanged.

### New findings from matching AALib.exe

The original `AALib.exe` from the user's Library archive has SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, matching the binary identity used by the Stage18 static analysis.

The three Stage18 table-byte readbacks of `0xFA` already exist in the on-disk PE's initialized `.data` image:

| Selector | State index | Preferred VA | Stage18 runtime VA | PE byte |
|---:|---:|---:|---:|---:|
| `0x0A` | `0x0E` | `0x04C7DBDD` | `0x0559DBDD` | `0xFA` |
| `0x17` | `0x19` | `0x04C8073B` | `0x055A073B` | `0xFA` |
| `0x0A` | `0x18` | `0x04C7DCE1` | `0x0559DCE1` | `0xFA` |

The runtime module base inferred from Stage18's return address is `0x00D20000`, a relocation delta of `+0x00920000` from the preferred `0x00400000` base. The relocation reproduces all three observed runtime addresses exactly. This establishes that the observed initial `0xFA` values are embedded in the PE; it does **not** establish that runtime code can never mutate the writable table.

The primary table pointer array at preferred VA `0x04C82518` maps selector `0x0A` to `0x04C7DA58` and selector `0x17` to `0x04C80498`. Each state row has stride `0x1A`; the gate reads key offset `0x19`.

### State-index conversion path

The wrapper `0x4F6ED0` calls `0x4F6BA0`, which reads `WORD [player+0x3A34]` and the signed raw code `WORD [player+0x3A36]`.

- Raw values `<=0x17` are returned as the state index directly.
- Raw values `0x18..0x1F` are rejected.
- Other mapped values pass through `0x4F02C0`, which computes `rawIndex=rawCode-0x20`, checks it against `DWORD [player+0x1220]`, and reads a WORD from `player+0x2B9E+rawIndex*0x14`.
- Flag priority in the helper is `0x0400→0`, `0x0800→1`, `0x1000→2`, `0x0200→5`, `0x2000→3`, `0x4000→7`, `0x8000→4`, `0x0008→6`; otherwise it returns `0xFFFFFFFF`.
- `0x4F6BA0` maps helper results `0..4` to indexes `0x18..0x1C`, `5→0x1A`, `6→0x1B`, `7→0x1C`, and `0xFFFFFFFF→-1`. The later caller rejects indexes `>=0x1C`.

This explains the Stage18 observations at the conversion level: P1 raw code `0x20` / stateIndex `0x18` implies helper result 0; P2 raw code `0x5C` / stateIndex `0x19` implies helper result 1. Stage19 is designed to capture these values directly.

For key offset `0x19`, the on-disk primary table's valid `0xFF` rows are:

| Selector | `0xFF` state indexes below `0x1C` |
|---:|---|
| P1 selector `0x0A` | `01, 03, 09, 0F, 14, 15, 16, 17` |
| P2 selector `0x17` | `01, 03, 07, 09, 0D, 0F` |

The mapped rows `0x18..0x1B` are all `0xFA` for these selectors. Thus a naturally reached direct state in the listed rows is the clearest current candidate for Stage18 to record actual `AL=0`. Those direct returns bypass Stage19's converter hook, but they remain covered by the Stage18 gate observer. Stage19 must not force-edit table or player state to manufacture such a sample.

### Stage19 source implementation

The new source overlay adds:
- `Stage19_Overlay/src/AITrace.cpp`;
- `Stage19_Overlay/ArcanaAI.ini`;
- `Stage19_Overlay/README_STAGE19.md`;
- `Stage19_Overlay/STATIC_STATE_MAP_ANALYSIS.md`.

It installs a passive inline observer at RVA `0x000F6C08`, immediately after the `0x4F02C0` CALL at `0x000F6C03`. The observer replaces six bytes (`ADD ESP,8; CMP EAX,5`) with CALL+NOP, logs the original helper EAX/raw code/WORD flags/derived index/QPC, accounts for its synthetic CALL return address plus the two original cdecl arguments, replays `CMP EAX,5`, and resumes at the untouched `JNE` at `0x000F6C0E`. It does not use DR0–DR3 and does not change the helper result or branch condition. Output is `ArcanaAI_StateMapTrace.log`.

### Verification status and next action

This is a **source-only** stage. Git comparison confirms that the Stage19 branch is six commits ahead of the Stage18 source snapshot, contains only the four new Stage19 overlay files, and has no changes to the Stage18 source or evidence branch. Stage19 has **not** been compiled with MSVC, linked, installed, or runtime-tested.

Next:
1. Apply Stage19 `AITrace.cpp` and `ArcanaAI.ini` over the assembled Stage18 source.
2. Build `Release | Win32` with MSVC and upload the complete `Build.log`. Do not install if compilation or linking fails.
3. Back up the working DLL, install only after a successful build, and confirm the Stage19 build marker/config plus successful hook installation at RVA `0x000F6C08`.
4. Run a short natural COM-vs-COM session and upload `ArcanaAI_StateMapTrace.log`, `ArcanaAI_Bit113GateTrace.log`, `ArcanaAI_Bit113SetterTrace.log`, `ArcanaAI_PlayerBit17WriteTrace.log`, plus relevant queue/dispatch/producer logs.
5. Verify the known conversions `0x20→0→0x18` and `0x5C→1→0x19`, compare the helper result with the WORD flags, correlate by player pointer/QPC, and check whether Stage18 observes an actual `AL=0` on a valid `0xFF` row.

Preserve Stage18 runtime evidence and all earlier historical branches unchanged.


## Stage19 runtime result — state-code mapping confirmed (2026-11-08)

Runtime evidence branch: [stage19-bit113-state-map-runtime-evidence](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-runtime-evidence), HEAD `3e75c07c7bdbd0827112f6a488936587a41a1b4b`. The user-uploaded log files were initially placed at the root of `main`; this companion evidence branch preserves the complete log set, the exact Stage19 overlay, and `Stage19_Overlay/STAGE19_RUNTIME_ANALYSIS.md`. The Stage19 source branch [stage19-bit113-state-map-trace](https://github.com/Hawky11/1/tree/stage19-bit113-state-map-trace), HEAD `841142895a976419a9bda197be286e7434993067`, remains separate and unchanged.

### Installation and limitations

Run initialization: 2026-11-08 19:05:37.

- Stage17 observer at RVA `0x000C6502): installed, 44 threads suspended.
- Stage18 gate observer at RVA `0x000C64A5): installed, 44 threads suspended.
- Stage19 state-map observer at RVA `0x000F6C08): installed with six-byte patch, 44 threads suspended.
- Stage19 marker/config: `STAGE19_BIT113_STATE_MAP_BUILD=2026-11-08-r1`, `EnableStage19StateMapTrace=1`.

**Build.log was not uploaded.** Runtime marker and hook-install line prove the Stage19 DLL code loaded and executed, but do not provide compiler/linker output or final DLL size.

### Stage19 results

`ArcanaAI_StateMapTrace.log` contains **1,235** `STATE_MAP_CALL` records, all with `readMask=0x0F`, and no Stage19 warning/error/drop record.

| Side | Raw code | Helper EAX | WORD flags | Derived index | Count |
|---|---:|---:|---:|---:|---:|
| P1 | `0x20` | 0 | `0x0400` | `0x18` | 200 |
| P1 | `0x22` | 0 | `0x0400` | `0x18` | 190 |
| P1 | `0x24` | 0 | `0x0400` | `0x18` | 130 |
| P1 | `0x29` | 0 | `0x0400` | `0x18` | 12 |
| P1 | `0x4A` | 3 | `0x2000` | `0x1B` | 152 |
| P2 | `0x58` | 1 | `0x0802` | `0x19` | 82 |
| P2 | `0x59` | 1 | `0x0982` | `0x19` | 297 |
| P2 | `0x5C` | 1 | `0x0802` | `0x19` | 56 |
| P2 | `0x5E` | 3 | `0x2002` | `0x1B` | 116 |
| **Total** | | | | | **1,235** |

Helper-result totals: 532×0, 435×1, 268×3. The observed flag values agree with the helper's priority checks. Stage18 QPC correlation confirms:
- 31 P2 raw `0x59) / helper EAX 1 / derived index `0x19) mappings, QPC gaps 67–108 ticks;
- two P1 raw `0x22) / EAX 0 / index `0x18) mappings, gaps 64–70 ticks;
- two P1 raw `0x20) / EAX 0 / index `0x18) mappings, gaps 63–71 ticks.

Mapped index `0x1B) from raw P1 `0x4A` and P2 `0x5E` did not appear in Stage18 gate records in this capture; not all calls to the state mapper necessarily enter the bit-113 caller. Direct raw indexes `<=0x17) bypass the Stage19 conversion helper.

### Stage18–Stage15 correlation

- Stage18: 42 gate results, all `AL=1), all post-return byte reads `0xFA). Still no actual `AL=0) sample. Parent returns: 38 direct path (`0x00DB6801`) and four callback path (`0x00DEB9FA`).
- Stage17: eight setter calls, two in each of four episodes, all from `0x000C6502), index `0x71`.
- Stage16: 17,361 writes; exactly four bit17 sets at RVA `0x0000FAD2` and four clears at RVA `0x000ED1A9`.
- Stage15: four payload-`0x1E` attempts; all saw bit17=1 and `branchTaken=0` at F6A68. No F6A9D/F6AAC/F6AD7 call followed, and no payload-`0x1E` record appeared in Queue, EventDispatch or FinalCommand.
- No diagnostic warning/error/drop reports were found in the reviewed Stage19–Stage15 logs.

**New important observation:** 31 P2 table-gate calls at index `0x19` returned `AL=1` but had no subsequent setter call. Therefore the Stage18 table gate is not a sufficient condition for reaching the bit-113 setter.

## Stage20 source overlay — trace the downstream object bit-2 guard (2026-11-08)

Source branch: [stage20-bit113-object-bit2-guard-trace](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-guard-trace), latest HEAD `c46738caf098fe187fdb2dae16310efc4da13390`. Git comparison with `stage19-bit113-state-map-trace` shows the Stage20 branch is ahead with exactly four new files under `Stage20_Overlay/`; it does not modify the Stage19 source or evidence branches.

### Static finding

The matching `AALib.exe` (SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`) has this post-table-gate code:

```asm
004C64ED  mov ecx,[ebx]
004C64EF  push 2
004C64F1  call 00495B00
004C64F6  test al,al
004C64F8  jne 004C640B
004C64FE  push 71h
004C6500  mov ecx,esi
004C6502  call 0040FAA0
```

Helper `0x00495B00` uses argument 2 to make mask `0x4`, reads `[object+0x3B70]), returns AL=1 if bit 2 is set and AL=0 if clear. The subsequent JNE skips the setter on AL=1. This is the evidence-backed next guard to test: 31 P2 state-index-`0x19` table-allowed calls had no setter, while the four setter episodes had Stage17 `OtherObject3B70` values `0x440`, `0x640`, `0x460`, `0x640` (bit 2 clear). The reason for the 31 gate-only calls remains a hypothesis until the helper's actual AL and checked object are captured.

### Stage20 instrumentation contract

The new overlay wraps only the five-byte CALL at RVA `0x000C64F1). It saves original ECX, duplicates argument 2 and executes the real helper. It logs the actual AL/EAX, original ECX object pointer, EBX slot pointer, `[EBX]), `[object+0x3B70]), bit-2 state, player fields, QPC and EBP parent return. It restores helper registers/EFLAGS, discards the saved ECX via flag-neutral `LEA ESP,[ESP+4]), then `RET 4` consumes the original caller argument and returns to `0x000C64F6`. The original `TEST/JNE` remains untouched. No hardware watchpoints are added. Output: `ArcanaAI_Bit113ObjectGuardTrace.log`.

### Verification status and next action

Stage20 is **source-only**; it has not been built with MSVC or runtime-tested. The source includes startup/config markers, signature checks for the callsite and following TEST/JNE, and a wrapper-specific installation log.

Next:
1. Apply Stage20 `AITrace.cpp` and `ArcanaAI.ini` over the assembled Stage19 source.
2. Build `Release | Win32` with MSVC and upload the complete `Build.log`. Do not install unless compilation and linking both succeed.
3. Back up the working DLL, install only after successful build, and verify Stage20 marker/config plus the wrapper installation at RVA `0x000C64F1).
4. Run a short natural COM-vs-COM test and upload `ArcanaAI_Bit113ObjectGuardTrace.log`, Stage18/17/16/19 logs and Stage15/Queue/Dispatch/FinalCommand logs.
5. Test whether the 31 P2 index-`0x19` gate-only calls show guard AL=1 and bit 2 set, while the four setter episodes show AL=0 and bit 2 clear. Continue to seek a natural Stage18 AL=0 table-sentinel sample.

Preserve Stage19 source/evidence and all Stage12–Stage19 historical branches unchanged.

## Stage20 MSVC build result — successful (2026-11-08 19:55:42)

Source branch: [stage20-bit113-object-bit2-guard-trace](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-guard-trace), build-verified source snapshot HEAD 11dde281a08a3481517d0cb60d4752393fa0e578. The Stage20 overlay is based directly on the immutable Stage19 source branch stage19-bit113-state-map-trace (HEAD 841142895a976419a9bda197be286e7434993067). Stage19 runtime evidence remains separate in stage19-bit113-state-map-runtime-evidence (HEAD 3e75c07c7bdbd0827112f6a488936587a41a1b4b).

The user supplied the build transcript in chat:
- Build started: 2026-11-08 19:55:42,91.
- Visual Studio: 18 Community.
- MSVC toolset: 14.51.36231.
- Architecture: x86 / Win32.
- All eight C++ sources compiled, including AITrace.cpp.
- Link step succeeded and produced build\\dinput8.dll.
- DLL size: **216,576 bytes**.
- One non-fatal C4754 warning remains in AICommandSiteTrace.cpp lines 782–785; the compiler notes an arithmetic comparison branch that cannot execute and suggests a 64-bit cast. The build succeeded despite this warning.

The Build.log was pasted in chat and is not present as a file in the repository. Do not claim to have an archived Build.log.

### Current status

**Stage20 is build-verified, runtime-pending.** The successful build does not prove that the Stage20 wrapper installed in the game process. The next run must first confirm the STAGE20_BIT113_OBJECT_BIT2_GUARD_BUILD=2026-11-08-r1 marker, config value EnableStage20Bit113ObjectGuardTrace=1, and the successful observational wrapper installation at RVA 0x000C64F1, targeting original helper RVA 0x00095B00. Stop before gameplay if validation or installation fails.

After installation is confirmed, run a short natural COM-vs-COM test and upload ArcanaAI_Bit113ObjectGuardTrace.log plus the Stage18/17/16/19 and relevant Stage15/queue/dispatch logs. The target correlation remains: do the 31 P2 state-index-0x19 gate-only calls show object-helper AL=1 and bit 2 set, while the four known setter episodes show AL=0 and bit 2 clear? This remains a hypothesis, not a runtime conclusion.

Preserve all Stage12–Stage19 source and evidence branches unchanged.
## Stage20 runtime result — object guard confirmed; alternate payload path observed (2026-11-08 19:58:47)

Runtime evidence branch: [stage20-bit113-object-bit2-runtime-evidence](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-runtime-evidence). It starts from uploaded runtime snapshot commit `3bd72139fa658bb60e0a3a3a30b63af7cd7519b8` and preserves all logs plus the exact Stage20 overlay. The Stage20 source branch remains [stage20-bit113-object-bit2-guard-trace](https://github.com/Hawky11/1/tree/stage20-bit113-object-bit2-guard-trace), HEAD `11dde281a08a3481517d0cb60d4752393fa0e578`.

### Stage20 wrapper installation

Runtime log confirms marker `STAGE20_BIT113_OBJECT_BIT2_GUARD_BUILD=2026-11-08-r1`, config `EnableStage20Bit113ObjectGuardTrace=1`, and successful wrapper installation at CALL RVA `0x000C64F1` to original helper RVA `0x00095B00`, resume RVA `0x000C64F6`, with 40 suspended threads. The original TEST/JNE were left untouched.

### Direct object-guard result

Stage20 recorded 33 object-helper calls, all P2. All 33 returned actual helper `AL=1` / `EAX=1`; the checked DWORD `[object+0x3B70]` was `0x0000004C` and bit `0x4` was set in every record. All 33 had `readMask=0x3F`. All matched the 33 Stage18 gate records one-to-one by side, player pointer and QPC, with Stage20 following Stage18 by 129–173 QPC ticks. All Stage18 records returned AL=1 and used table key `0x19`; the separate `stateIndex` values were 0x0A (4), 0x04 (2), 0x14 (25), and 0x10 (2).

Per the statically verified caller, this nonzero helper result makes the original JNE skip the direct setter call at `0x000C6502`. The Stage17 passive observer at that site installed but logged zero setter calls in this run. Stage16 produced 28,043 writer snapshots; every one had `oldBit17=0` and `newBit17=0`, with zero bit17 transitions. This is consistent with the object-guard result. No AL=0 / bit-2-clear control sample was obtained, so the opposite case remains unverified.

Stage19 mapping recorded 2,974 helper calls, including 240 P2 mappings rawCode `0x5C` -> expected index `0x19` (mapResult 1), 570 rawCode `0x60` -> `0x1B` (result 3), 158 rawCode `0x63` -> `0x1C` (result 7), and P2 calls mapping to `0x18`. These are not a direct one-to-one match for the 33 Stage18 table-gate records; keep the mapper's `expectedStateIndex`, Stage18's `stateIndex`, and its table `key` distinct.

### Alternate payload path

Three alternate callsite events at RVA `0x000F6AD7` carried payload `0x1E` (P2 once, P1 twice). The matching queue details show type 1/payload `0x1E` accepted and stored `(0001,001E)`; the matching EventDispatch details show the same type/payload with `beforeMatches=1` and `afterMatches=1`. All three episodes have Stage15 bit17=0 and a taken F6A68 branch; F6AD7 follows the corresponding F6A9D probe by 311–426 QPC ticks and F6A68 by 620–851 ticks, in the same thread/parent frame. This demonstrates that the observed alternate queue/dispatch path runs while bit17=0; it is distinct from the direct setter branch at `0xC6502`.

FinalCommandTrace captured 60 records and no payload `0x1E` at its monitored callsite RVA `0x000F7C5D`. Treat this as a scope-limited no-hit, not proof the dispatched event was unhandled.

No explicit warning/error/drop/failure records were found in the reviewed runtime logs. The user's successful Build.log was pasted in chat before the test and is not present as a file in the runtime upload.

### Next research questions

1. Obtain a natural object-bit-2-clear Stage20 call and correlate actual AL=0 with Stage18 and the direct Stage17 setter/Stage16 bit17 transition.
2. Statically locate the consumer after dispatch of type 1 / payload `0x1E` at caller RVA `0x000EE5CB`; do not assume the `0x000F7C5D` FinalCommand hook observes this path.
3. Resolve why F6A68/F6A9D report taken branches with bit17 zero while F6AD7 is still reached. Avoid assigning branch semantics from probe labels alone.

See `Stage20_Overlay/STAGE20_RUNTIME_ANALYSIS.md` on the evidence branch for detailed counts and correlations. Preserve all Stage12–Stage19 historical branches unchanged.

Documentation sync on `main`: latest runtime evidence branch HEAD `08125627bfbb588aff8e735168c51a1083cb06e6`; current Stage20 source remains `11dde281a08a3481517d0cb60d4752393fa0e578`.


## Stage21 source stage — locate the payload-0x1E consumer (2026-11-08)

Source branch: [stage21-payload-1e-consumer-trace](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-trace), HEAD `d210c3540ddd29fff1795ef8da6d3605958d481a`. It branches from the immutable Stage20 source snapshot `stage20-bit113-object-bit2-guard-trace`, HEAD `11dde281a08a3481517d0cb60d4752393fa0e578`. Stage20 runtime evidence remains separately preserved in `stage20-bit113-object-bit2-runtime-evidence`, HEAD `08125627bfbb588aff8e735168c51a1083cb06e6`.

### Static consumer identification

- Corrected the Stage12 address interpretation: dispatcher entry is RVA `0x000EE200`; `callerRVA=0x000EE5CB` is the return address after the dispatcher CALL.
- In the type-1 branch at `0x004EE200`, the dispatcher calls `0x004E8CF0` at RVA `0x000EE426` with payload in EAX, player in ESI and a literal selector argument 1 on the stack. This selector is distinct from the original dispatch event type in `[dispatcher EBP+8]`.
- The resolver's byte at result-structure offset `+3` is decoded into EDI; `cmp edi,1` at `0xEE3E1` therefore tests decoded action code 1, not original dispatch event type 1. For payload `0x1E`, helper `0x004E8CF0` is statically constrained to return `AL=0`: if the intermediate `0x00525860` lookup returns nonzero, the helper's early exit is AL=0; otherwise `0x1E < 0x20`, but `0x1E <= 0x17` is false at the final `SETBE`.
- The subsequent `JE 0x004EE443` selects the fallback call `0x004F7EB0` at RVA `0x000EE446`. The helper writes zero to `DWORD [controllerArray + index*4 + 4]`; the caller supplies index 3 and pointer `player+0x3A3C`, so the exact slot is `[controllerArray+0x10]`. The next instruction clears bit `0x400` in `[controller+8]`.
- This locates a real consumer-side effect downstream of dispatcher and explains why the unrelated monitored FinalCommand callsite `0xF7C5D` had no payload-`0x1E` record.

### F6A68 / F6A9D / F6AAC control-flow clarification

- At `F6A68`, bit17=0 makes `JE F6A9D` taken; bit17=1 takes the other path and returns at `F6A9C` rather than entering the alternate queue insertion.
- On the bit17-clear path, `F6A9D` checks `[ebp-4] != 2`; its taken branch jumps to `F6AC8`. If it falls through, the secondary helper at `F6AA5` runs.
- At `F6AAC`, the secondary helper's nonzero result also jumps to `F6AC8`; zero allows the extra `0x72` setter sequence before joining at the same `F6AC8`.
- `F6AC8` is a common continuation, not a return. The queue insertion call at `F6AD7` follows it. Therefore the F6A9D and F6AAC taken branches do not prevent queueing payload `0x1E`; they skip only the extra side-effect sequence. This resolves the previously reported “branch taken but still reaches F6AD7” observation by static disassembly.
- The Stage20 evidence bundle did not contain the Stage15 per-probe runtime records needed for a fresh QPC-level reconstruction of each of the three individual branch episodes. The conclusion above is control-flow proof from the matching static binary, not a newly claimed per-event runtime trace.

### Stage21 source overlay

- `Stage21_Overlay/src/AIQueueTrace.cpp` adds two observational CALL wrappers:
  - `0xEE426 -> 0x4E8CF0`: logs real post-call EAX/AL plus original dispatch type/payload, decoded action code and helper selector argument.
  - `0xEE446 -> 0x4F7EB0`: logs array pointer, index, and slot-3 DWORD before/after the original clear, plus controller flags immediately before the following AND instruction.
- Both wrappers duplicate the original cdecl arguments, invoke the original helper exactly once, preserve its result/register effects for the original caller, and leave original caller stack cleanup instructions intact. Install-time validation checks the surrounding instruction signatures and decoded helper destinations; installation uses the shared patch mutex and suspends other process threads.
- `Stage21_Overlay/ArcanaAI.ini` enables `EnableStage21PayloadConsumerTrace=1`. New log: `ArcanaAI_Stage21PayloadConsumerTrace.log`.
- `Stage21_Overlay/STATIC_PAYLOAD_1E_CONSUMER_ANALYSIS.md` contains the full disassembly and trace contract. `Stage21_Overlay/README_STAGE21.md` contains application/build steps.

### Status and next action

Stage21 is **source-only**. No MSVC build, DLL, runtime hook installation or Stage21 runtime evidence is claimed in this commit.

1. Apply the Stage21 `AIQueueTrace.cpp` and INI overlays to the assembled Stage20 project, keeping Stage20 `AITrace.cpp` and all earlier source units intact.
2. Build `Release | Win32` and preserve the complete Build.log. Do not install if compilation/linking fails.
3. After a successful build, confirm the Stage21 marker and both CALL-wrapper installation entries; then capture `ArcanaAI_Stage21PayloadConsumerTrace.log` with the matching Stage12 EventDispatch and Stage20 object-guard logs.
4. Confirm runtime classifier `AL=0` and slot-`3` pre/post clear. Separately continue the natural Stage20 object-bit-2-clear experiment to obtain the missing object-guard `AL=0) sample. Never force the object bit or helper return value.

Stage21 is a source snapshot only; Stage20 source/evidence and all earlier historical branches remain unchanged.


## Stage21 runtime result — consumer fallback confirmed; object-guard negative control captured (2026-11-08 21:43–21:45)

Runtime evidence branch: [stage21-payload-1e-consumer-runtime-evidence](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-runtime-evidence), HEAD `274a58e2b514766ea2ef067c83f3ae321a0908ce`. This branch starts from the uploaded runtime snapshot commit `e7f3faf33c5e37f49896ed4f0375b12c2b853daa` and preserves the runtime logs, the exact Stage21 r1 overlay and the user-supplied Build.log transcript. Full analysis: `Stage21_Overlay/STAGE21_RUNTIME_ANALYSIS.md`.

Tested source branch: [stage21-payload-1e-consumer-trace](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-trace), HEAD `d210c3540ddd29fff1795ef8da6d3605958d481a`. It is the exact source version built and loaded in this run.

### Build and installation

- Build started 2026-11-08 21:43:30,49 using Visual Studio 18 Community / MSVC 14.51.36231 x86.
- All eight C++ translation units compiled and `build\\dinput8.dll` linked successfully at **222,208 bytes**.
- One non-fatal C4754 warning remains in `AICommandSiteTrace.cpp` lines 782–785.
- Stage21 marker `STAGE21_PAYLOAD_1E_CONSUMER_BUILD=2026-11-08-r1` is present.
- Both Stage21 CALL wrappers installed: `0xEE426 -> 0x4E8CF0` and `0xEE446 -> 0x4F7EB0`, with 40 process threads suspended during installation.
- Stage20 object guard at `0xC64F1`, Stage17 setter observer at `0xC6502`, and Stage18 observer at `0xC64A5` also installed.

### Payload-0x1E consumer runtime

Two events were traced, one P1 and one P2, on thread 23660:
- The alternate producer at `0xF6AD7` queued type 1 / payload `0x1E`; QueueTrace reported `ACCEPTED_BY_QUEUE` and stored `(0001,001E)`.
- EventDispatch at `0xEE200` recorded the same arguments with `beforeMatches=1` and `afterMatches=1`.
- Stage21 classifier CALL at `0xEE426` returned real `EAX=0 / AL=0` for both P1 and P2.
- Both then reached fallback CALL `0xEE446` to helper `0x4F7EB0`, index 3.
- The slot DWORD was zero before and after both fallback helper calls. The controller flags read before the subsequent AND were also zero. Thus fallback execution is confirmed, but neither sample proves an actual nonzero→zero slot change or a set bit-`0x400` clear.

### Stage20 object-bit-2 negative control and Stage17 setter

This same run finally recorded a natural P1 object guard with bit 2 clear:
- `[object+0x3B70]=0x00010440), `bit2Set=0), helper argument 2, real `AL=0`, `readMask=0x3F`.
- 79 QPC ticks later, Stage17 recorded the direct setter CALL at `0xC6502`, index `0x71`, same P1 pointer `0x0E9C1F78), same thread 23660, and the same `other3B70=0x00010440`.

This confirms the object helper's negative result is followed by the direct bit-113 setter call in this sample; the original JNE is not taken and execution falls through to `0xC6502`.

### Remaining limits and r1 logger defect

- `ArcanaAI_PlayerBit17WriteTrace.log` is 7,565,612 bytes and contains 14,081 writer records. The blob reinspection correction appended below finds exactly two bit17 transitions: one P1 set at RVA `0xFAD2` and one later P1 clear at RVA `0xED1A9`. The P1 set follows the Stage17 setter by 361 QPC ticks; the full transition data and correlation are recorded in the correction section below.
- Stage18 observer installed but `ArcanaAI_Bit113GateTrace.log` contains no gate result entries. No same-event Stage18 state-table correlation is available for this P1 object-guard sample.
- Stage19 had 1,320 `STATE_MAP_CALL` entries, but none was correlated to the object guard/setter QPC at 21:45:03.002.
- The Stage21 r1 field `decodedActionCode` is invalid: it sampled EDI after helper `0x4E8CF0`; the observed values (`0x0E9C59B4` / `0x0E9F06DC`) equal the controller-array pointers in the following slot-clear records. The actual AL and dispatch type/payload fields are valid.

### Corrected Stage21 r2 source candidate

A separate source branch preserves this correction without rewriting r1: [stage21-payload-1e-consumer-trace-r2](https://github.com/Hawky11/1/tree/stage21-payload-1e-consumer-trace-r2), HEAD `28fd26b76c22b9070eabaa33a85c713876234aaf`. It saves EDI on the stack before calling the original classifier helper and logs `decodedActionCodePreCall`; it uses marker `STAGE21_PAYLOAD_1E_CONSUMER_BUILD=2026-11-08-r2`. This branch is **not yet built or runtime-tested**. The Stage21 r1 source/evidence branches remain immutable.

### Next research questions

1. Capture a natural fallback where slot 3 is nonzero before `0x4F7EB0`, or controller bit `0x400` is set before the following AND, to confirm an actual state transition.
2. The direct setter → bit17 write chain is confirmed for this P1 episode. Remaining gaps are a natural fallback with nonzero slot 3 or bit `0x400` set before the helper/AND, and same-event Stage18 gate correlation.
3. If another Stage21 payload capture is needed, build from r2 to obtain a valid pre-call decoded action code. Do not rewrite or overwrite the r1 evidence branch.


## Stage21 Stage16 blob reinspection correction — bit17 transitions confirmed (2026-11-08)

This correction supersedes the Stage21 runtime paragraph above that described `ArcanaAI_PlayerBit17WriteTrace.log` as zero bytes. That statement came from a normal file-read result that returned no text for an oversized blob. The file is present in the upload and its Git blob was successfully retrieved and parsed.

- Blob size: **7,565,612 bytes**.
- Lines: **14,089**.
- `PLAYER_BIT17_WRITE` records: **14,081**.
- Exactly **two** bit17 transitions were found in the entire run; both are P1: one set and one later clear.

### Set transition correlated to the object guard and setter

All three records use P1 player `0x0E9C1F78` and thread 23660:
- Stage20 object-guard record: qpc `0x00000764133B9CC6`, `[object+0x3B70]=0x00010440`, bit2 clear, real helper `AL=0`.
- Stage17 setter record: qpc `0x00000764133B9D15`, CALL RVA `0x000C6502`, index `0x71`; gap from guard is **79 QPC ticks**.
- Stage16 set record: sequence 11981, qpc `0x00000764133B9E7E`, RVA `0x0000FAD2`, watched field `player+0x3B84`; old DWORD `0x01000000`, new DWORD `0x01020000`, `oldBit17=0`, `newBit17=1`, `shadowBit17Changed=1`. It occurs **361 QPC ticks after the setter** and **440 ticks after the object guard**.

This confirms the tested P1 path end-to-end: object bit2 clear causes the original helper to return AL=0; the caller reaches the direct bit-113 setter; the player's bit17 is actually set by the write at RVA `0xFAD2`.

### Later clear

Stage16 sequence 12178 records qpc `0x000007641364D88E`, RVA `0x000ED1A9`, old DWORD `0x00020000`, new DWORD `0x00000000`, `oldBit17=1`, `newBit17=0`. This is **2,701,840 QPC ticks after the set** and confirms the later clear path in the same capture.

### Corrected Stage21 r1 conclusions

- The previous statement that Stage16 writer evidence was unavailable is withdrawn.
- The P1 object-bit-2-clear → actual AL=0 → index-0x71 setter → actual bit17 0→1 write chain is now confirmed with QPC-level evidence.
- Stage16 observed exactly one set and one clear transition in this run; the P2 side had no bit17 transition.
- Stage18 still has no gate-result entries, so no same-event Stage18 table-key/state-index correlation has been established.
- Stage21 r1 fallback slot 3 and controller flag 0x400 were already zero before the respective P1/P2 fallback calls; a nonzero-to-zero mutation remains unobserved.
- The r1 `decodedActionCode` value remains invalid because EDI was sampled after the classifier helper. The separately preserved r2 source saves EDI before that helper but is not built or runtime-tested.

Updated detailed analysis: [Stage21 runtime analysis](https://github.com/Hawky11/1/blob/stage21-payload-1e-consumer-runtime-evidence/Stage21_Overlay/STAGE21_RUNTIME_ANALYSIS.md). Updated evidence-branch HEAD: `150a666f6f61e58aa67766010e6af4300a51d901`.


## Stage22 source stage — payload-0x1E controller post-AND trace (2026-11-08)

Source branch: [stage22-payload-1e-post-and-trace](https://github.com/Hawky11/1/tree/stage22-payload-1e-post-and-trace), HEAD `27b711d4b3ee8079060bfbab8f8ecc22713ac917`. It branches from the corrected Stage21 r2 source branch `stage21-payload-1e-consumer-trace-r2`, HEAD `28fd26b76c22b9070eabaa33a85c713876234aaf`. No earlier source or evidence branch was modified.

### Scope

Stage21 r1 captured the controller flags immediately before the following AND, but both P1/P2 samples already had bit `0x400` clear. Stage22 adds a post-instruction observation at dispatcher RVA `0x000EE451`:

```asm
004EE44B mov esi, [esi+4]
004EE44E add esp, 8
004EE451 and dword ptr [esi+8], 0xFFFFFBFF
```

- Expected seven-byte AND encoding: `81 66 08 FF FB FF FF`; the six-byte preceding context is also validated.
- The seven-byte instruction is replaced with a five-byte relative CALL plus two NOPs.
- The wrapper captures the controller DWORD before the operation, executes the exact original AND once, captures the value after, and restores the post-AND EFLAGS and all general registers.
- Installation uses the shared patch mutex, thread suspension, byte revalidation, instruction-cache flush and original page-protection restoration. Any fingerprint/signature mismatch refuses the patch.
- The new log is `ArcanaAI_Stage22PayloadPostAndTrace.log`; records include dispatcher type/payload, controller pointer, side when resolvable, before/after DWORDs and bit-`0x400` values.
- The inherited Stage21 r2 classifier wrapper logs `decodedActionCodePreCall`, fixing the r1 post-helper EDI sampling defect. The Stage21 slot-3 before/after helper capture is retained.

### Follow-up source review

A post-commit manual assembly/stack review corrected two details in the same Stage22 branch (latest HEAD `27b711d4b3ee8079060bfbab8f8ecc22713ac917`):
- After restoring the EFLAGS generated by the original AND, stack cleanup uses flag-neutral `LEA ESP,[ESP+8]`, not `ADD ESP,8`, so the original EFLAGS survive to the original continuation.
- The CALL replacement returns first to `0x000EE456`; its two trailing NOPs at `0xEE456–0xEE457` then continue at `0xEE458`. The static analysis and code comments reflect this.

At the time of this source-stage checkpoint, build/runtime were still pending; the subsequent Stage22 runtime result is appended later in this history and supersedes that interim status.

A further stack/register review (same source branch, current HEAD `27b711d4b3ee8079060bfbab8f8ecc22713ac917`) also saves the incoming EAX before using it to read the pre-AND DWORD, restores that EAX after logging, and adjusts logger offsets for the additional local. The logger reads the pre-AND DWORD from `savedEsp+12` and CALL return address from `savedEsp+16`. This remains source inspection only; no build or runtime result is claimed.

### Status at source-stage commit

At the moment this source branch was first committed, it was source-only and had no build/runtime result. The later Stage22 runtime section below records the successful 225,792-byte build and confirmed runtime installation; it supersedes this interim status. The exact expected instruction bytes remain checked before patching.

### Next test

Apply `Stage22_Overlay/src/AIQueueTrace.cpp` and `Stage22_Overlay/ArcanaAI.ini` to the assembled Stage20/21 project, then build Release | Win32. After successful build, verify both Stage21 r2 and Stage22 markers plus installation at RVA `0x000EE451`. Capture the Stage22 post-AND log and matching Stage21, dispatch, queue, Stage20, Stage17, Stage16, Stage18 and Stage19 logs.

Interpretation: `bit400Before=1 bit400After=0` proves the controller flag actually changed. `0 -> 0` proves execution only. A natural nonzero slot-3 entry is still required to prove the independent slot clear; same-event Stage18 gate correlation also remains open. Preserve all Stage21 historical branches and the Stage21 r1 runtime evidence unchanged.


## Stage22 runtime result — controller AND executed; Stage18/20/17 chain correlated (2026-11-08 23:30–23:32)

Source branch: [stage22-payload-1e-post-and-trace](https://github.com/Hawky11/1/tree/stage22-payload-1e-post-and-trace), HEAD `27b711d4b3ee8079060bfbab8f8ecc22713ac917`.  
Runtime evidence branch: [stage22-payload-1e-runtime-evidence](https://github.com/Hawky11/1/tree/stage22-payload-1e-runtime-evidence), HEAD `5b9d1bd83d1324d899f4c5f7f17dcaca136e7453`. The evidence branch preserves the exact tested source/INI, the user-supplied Build.log, uploaded logs, and `Stage22_Overlay/STAGE22_RUNTIME_ANALYSIS.md`.

### Build and installation

- Build started 2026-11-08 23:30:54 using VS 18 Community / MSVC 14.51.36231 x86.
- All eight C++ units compiled; `build\\dinput8.dll` linked successfully at **225,792 bytes**.
- One non-fatal C4754 warning remains at `AICommandSiteTrace.cpp:785`.
- Stage21 r2 and Stage22 markers are present. Both Stage21 callsite wrappers and the Stage22 observer at RVA `0xEE451` installed successfully.
- Queue, EventDispatch, Stage20 object guard, Stage17 setter and Stage18 gate observers also installed.

### Payload-0x1E consumer path

Three events (P2, P2, P1), all thread 10804, are matched across QueueTrace, EventDispatchTrace, Stage21 consumer and Stage22 post-AND logs:

| Side | Queue QPC | Dispatch-before QPC | Classifier QPC | Fallback QPC | Post-AND QPC |
|---|---:|---:|---:|---:|---:|
| P2 | 8190556018120 | 8190556018144 | 8190556019441 | 8190556022852 | 8190556023516 |
| P2 | 8190773497452 | 8190773497465 | 8190773498477 | 8190773500276 | 8190773500737 |
| P1 | 8190826674362 | 8190826674384 | 8190826675468 | 8190826677753 | 8190826678325 |

Each queue event was accepted, dispatch history matched type 1/payload `0x1E`, classifier helper `0xE8CF0` returned actual `AL=0`, and fallback helper `0xF7EB0` ran with index 3. Stage22 post-AND records follow fallback by 664, 461 and 572 QPC ticks.

### Post-AND controller state

All three post-AND readbacks succeeded. Bit `0x400` was `0 → 0` for P2 twice and P1 once. P1's complete controller DWORD was `0x4` both before and after; P2's was `0`. Stage21 slot-3 DWORD was also `0` before and after all three fallback helper calls.

Conclusion: Stage22 proves the original AND executed and preserves its post-AND observation, but this run did not capture a nonzero-to-zero mutation of either slot 3 or controller bit `0x400`.

### 11/11 Stage18 → Stage20 → Stage17 runtime matches

The Stage18 log has 11 gate-result records; Stage20 has 11 object-guard records; Stage17 has 11 setter-call records. Every indexed triplet matches by side, player pointer, thread 10804, game tick and parent return address. In each triplet:
- Stage18 returned `AL=1`, key `0x19`, state index `0x18` or `0x19`, and table byte `0xFA`.
- Stage20 read object DWORD `0x00000640`, bit 2 clear, and the original object helper returned `AL=0`.
- Stage17 then observed setter CALL RVA `0xC6502`, bit index `0x71`, 71–96 QPC ticks after the object guard (gate-to-guard gaps were 122–178 QPC ticks).

This resolves the previous same-event Stage18 gate-result correlation gap for this guard/setter path. It is not a direct mapping of Stage19 raw codes: `ArcanaAI_StateMapTrace.log` is zero bytes in this run.

Five two-call episodes show Stage17's first entry observing `[player+0x3B84]=0` and the second observing `0x20000`. This is consistent with bit17 becoming set between the calls. However, `ArcanaAI_PlayerBit17WriteTrace.log` is zero bytes, so the exact write instruction/QPC is not independently observed in this run.

### Correction: Stage21 r2 pre-call EDI is not an action code

The Stage21 r2 wrapper successfully captures EDI before helper `0xE8CF0`, but the runtime values prove that EDI has already been repurposed by CALL `0xEE426`:
- P2: decimal 245303004 = `0x0E9F06DC`, identical to the P2 controller-array pointer recorded by the following fallback call.
- P1: decimal 245127604 = `0x0E9C59B4`, identical to the P1 controller-array pointer recorded by the following fallback call.

Therefore `decodedActionCodePreCall` is a misleading field name and must not be interpreted as the decoded action. The timing correction is real; the assumed register dataflow was not. Future action-code instrumentation must capture the resolver result at the actual decode/compare point, or correct the static disassembly from resolver output to CALL `0xEE426`.

### Missing evidence and next task

- `ArcanaAI_PlayerBit17WriteTrace.log`: zero bytes.
- `ArcanaAI_StateMapTrace.log`: zero bytes.
- No natural precondition with slot 3 nonzero or controller bit `0x400` set before the operation was observed.
- Next priority: audit the EDI dataflow from resolver output through `cmp edi,1` at `0xEE3E1` to CALL `0xEE426`; add a passive observer at the true decode/compare location only if action-code identity is still required. Preserve all Stage21/22 evidence branches.



## Stage23 source stage — capture the type-1 resolver result (2026-11-09)

Source branch: [stage23-resolver-result-trace](https://github.com/Hawky11/1/tree/stage23-resolver-result-trace), HEAD `abaf5bb48d0920632f7b9e1d7c0316eeda3c330d`. It branches from the completed Stage22 source. The exact Stage22 source snapshot remains preserved in `Stage22_Overlay/src/AIQueueTrace.cpp` on this branch; the new complete replacement is in `Stage23_Overlay/src/AIQueueTrace.cpp`.

### Corrected static dataflow

The archived `AALib.exe` was extracted from the previously uploaded `AALib.zip`; its SHA-256 exactly matches the analyzed executable. Direct disassembly confirms:

- Type-1 resolver CALL: RVA `0x000EE273` -> helper RVA `0x0012EAC0`.
- The helper returns a pointer to a 12-byte record in EAX. The dispatcher reads bytes `+0x0B`, `+0x04`, `+0x02`, and `+0x03`. Byte `+0x03` is loaded into EDI at `0xEE28D`, then used through the subsequent `MOV EAX,EDI` and helper call at `0xEE306`.
- At `0xEE30B`, `MOV EDI,[EBP+8]` overwrites EDI with the dispatcher's original event-type argument. The callee `0x4EC4F0` preserves EDI. Therefore `CMP EDI,1` at `0xEE3E1` compares dispatcher event type, not the earlier resolver byte.
- In the later type-1/payload-`0x1E` path, `LEA EDI,[ESI+0x3A3C]` at `0xEE3FC` repurposes EDI as the controller-array pointer. This explains why Stage21 r2's pre-call EDI sample was the controller-array pointer.
- The earlier Stage21 static assertion that EDI at `0xEE3E1` still carried the resolver byte is explicitly superseded by this Stage23 binary review.

### Instrumentation

Stage23 wraps the five-byte CALL at `0xEE273`. It makes private copies of the four existing cdecl stack arguments, calls the original resolver exactly once with identical argument values, then records the returned record pointer, all 12 raw bytes, selected byte fields and input arguments. It preserves the original returned EAX, post-call registers and EFLAGS; it restores the private stack area with flag-neutral LEA, returning to the original `0xEE278` continuation with the original caller-owned arguments still in place.

The installer validates:
- the four-byte prefix `52 53 51 50`;
- the exact CALL bytes `E8 48 08 04 00`;
- the seven-byte suffix `0F B6 48 0B 8B 56 04`;
- resolver prologue `55 8B EC 8B 45 10 83 F8`;
- the decoded target equals `0x0012EAC0`;
- PE image size/timestamp.

It uses the shared mutex, suspends other process threads, revalidates the bytes immediately before patching, flushes the instruction cache and restores page protection. The log filters to exact P1/P2 type-1/payload-`0x1E` traffic. Its new output field is deliberately named `resolverByte03`, not an assumed action/attack ID. The inherited Stage21 field has been renamed to raw `preCallEdi` in the Stage23 overlay to avoid repeating the prior semantic mistake.

### Status and next test

**Source-only; not yet compiled or runtime-tested.** No DLL size, hook installation or resolver result is claimed. The expected marker is `STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r1`; the new file is `ArcanaAI_Stage23ResolverResultTrace.log`.

Next: apply the complete Stage23 overlay over the assembled Stage22 project, build Release | Win32, review the full Build.log, then run normally and correlate Stage23 resolver records with Stage21 classifier/fallback and Stage22 post-AND logs. Do not interpret `resolverByte03` as an attack ID without runtime evidence.


## Stage23 r2 compiler correction — resolver pointer casts (2026-11-09 00:33)

The first user-side Stage23 build attempt stopped in `src/AIQueueTrace.cpp`. Visual Studio 2026 Developer Command Prompt 18.9.1 initialized MSVC 14.51.36231 for x86 successfully. Project checks and the compiler/linker path checks passed; `Proxy.cpp`, `ComVsCom.cpp`, `AITrace.cpp`, and `AICommandTrace.cpp` compiled before the build stopped.

MSVC reported two C2440 errors:
- line 1515: `static_cast` could not convert `BYTE*` to `intptr_t`;
- line 1567: the same invalid pointer-to-integer conversion.

No final link occurred, and no Stage23 DLL was produced by this attempt. The cause was the same expression in two resolver-target comparisons: `static_cast<std::intptr_t>(resolver)`. The code already treats the neighboring callsite address as an integer through `reinterpret_cast<std::intptr_t>`, so the two helper-pointer conversions have now been changed to `reinterpret_cast<std::intptr_t>(resolver)`. This changes only the pointer-to-address representation conversion; it does not change the target RVA, signature checks, resolver arguments, wrapper execution, or hook placement.

Corrected source branch: [stage23-resolver-result-trace](https://github.com/Hawky11/1/tree/stage23-resolver-result-trace), current HEAD `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`. The Stage23 build marker is now `STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r2`. The updated source is `Stage23_Overlay/src/AIQueueTrace.cpp`; the Stage23 README and static analysis record the same correction.

**Status:** corrected source committed, rebuild pending. Do not claim successful compilation, DLL size, hook installation, or runtime records until a fresh Release | Win32 Build.log and same-run installation log are reviewed. The next user action is to reapply the Stage23 r2 source and INI over the assembled Stage22 project, rebuild, and upload the complete Build.log.


## Stage23 runtime result — resolver mapping captured (2026-11-09 00:39–00:40)

Source branch: [stage23-resolver-result-trace](https://github.com/Hawky11/1/tree/stage23-resolver-result-trace), HEAD `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`.  
Runtime evidence branch: [stage23-resolver-result-runtime-evidence](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence), HEAD `8e2d025ca60def5103206250a71929fa5d9329bd`. It preserves the exact uploaded run logs, the user-pasted Build.log, and `Stage23_Overlay/STAGE23_RUNTIME_ANALYSIS.md`.

### Build and hook installation

- Build began 2026-11-09 00:39:38,25 with Visual Studio 18 Community / MSVC 14.51.36231 x86.
- All eight translation units compiled: Proxy.cpp, ComVsCom.cpp, AITrace.cpp, AICommandTrace.cpp, AIQueueTrace.cpp, AICommandSiteTrace.cpp, AIProducerContextTrace.cpp and AIParentFlowTrace.cpp.
- `build\\dinput8.dll` linked successfully at **229,376 bytes**.
- Only warning: non-fatal C4754 in `AICommandSiteTrace.cpp:785`, associated with the bounds calculation around line 782. No compile/link errors.
- Runtime initialization was 2026-11-09 00:40:04. The marker `STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r2` is present.
- Stage23 observer installed at resolver CALL RVA `0x000EE273` -> helper RVA `0x0012EAC0`; installation reported 40 suspended threads. Three readable resolver records were captured.

### Resolver output for payload 0x1E

All three records are thread 11360, dispatcher type 1 / payload `0x1E`:

| Side | Tick | Resolver QPC | Record pointer | Args (arg1,arg2,arg3,arg4) |
|---|---:|---:|---|---|
| P2 | 823151515 | 8231589249280 | `0x063FDCA0` | `(0x17,0x17,0x1E,0x38)` |
| P1 | 823158406 | 8231658248588 | `0x05FDF490` | `(0x0A,0x0A,0x1E,0x26)` |
| P1 | 823166531 | 8231739431884 | `0x05FDF490` | `(0x0A,0x0A,0x1E,0x26)` |

Every result was readable and all three contained the same twelve bytes:

`60 FF 03 1F 12 00 00 05 13 24 03 00`

DWORD view: `(0x1F03FF60, 0x05000012, 0x00032413)`. Selected bytes: +2=`0x03`, +3=`0x1F`, +4=`0x12`, +0x0B=`0x00`. The P2 args agree with player fields +0x3978 / +0x3F20 = `0x17`, +0x1224 = `0x38`; the two P1 records agree with corresponding values `0x0A`, `0x0A`, `0x26`. The P1 pointer is reused for both P1 events; P2 has a distinct pointer, but the returned bytes match.

### End-to-end event correlation

All three Stage23 resolver records match Stage13 ALT_CALLSITE, QueueTrace, EventDispatchTrace, Stage21 and Stage22 using side/player/thread/tick/QPC:

| Side | ALT_CALLSITE QPC | Resolver QPC | Classifier QPC | Fallback QPC | Post-AND QPC |
|---|---:|---:|---:|---:|---:|
| P2 | 8231589249194 | 8231589249280 | 8231589253347 | 8231589256114 | 8231589257163 |
| P1 | 8231658248530 | 8231658248588 | 8231658251710 | 8231658252447 | 8231658252837 |
| P1 | 8231739431782 | 8231739431884 | 8231739434850 | 8231739435766 | 8231739436176 |

- ALT_CALLSITE is the alternate producer call at RVA `0xF6AD7`, type 1 / payload `0x1E`.
- QueueTrace logged `ACCEPTED_BY_QUEUE` at return RVA `0xF6ADC`; all three stored type `0x0001` / payload `0x001E`.
- EventDispatch logged all three at caller RVA `0xEE5CB`; queue history matched the dispatch args.
- Stage21 classifier helper `0xE8CF0` returned `AL=0` for all three.
- Stage21 fallback helper `0xF7EB0` ran with index 3 for all three.
- Stage22 post-AND observer at RVA `0xEE451` ran for all three.

Slot 3 was readable and zero before and after all three fallback helper calls. Controller flags before the AND were zero; Stage22 post-AND confirmed bit `0x400` remained `0 -> 0` in all three. The run proves the fallback and AND execute, but does not capture an actual nonzero-to-zero clear.

Stage21's `preCallEdi` values still equal the controller-array pointer (P2 `0x0E9F06DC`, P1 `0x0E9C59B4`); keep that field as raw EDI, not an action-code interpretation.

### Stage18 / Stage20 / Stage17 cross-check

- Stage18 and Stage20 each logged 27 records. Pairing by side, player, thread, tick and parent return produced **27/27** matches; each matched object-guard QPC followed its gate result by 124–204 ticks.
- All 27 Stage18 results returned `AL=1`, key `0x19`, table byte `0xFA`.
- Stage20 object helper returned `AL=1` in 25 records. Two P1 records at tick 823152734 had `AL=0`, object DWORD `0x58`, and bit2 clear.
- Stage17 logged two P1 setter calls at RVA `0xC6502`, index `0x71`, 76 and 73 QPC ticks after the two corresponding AL=0 guard results. First snapshot saw `player+0x3B84=0`; second saw `0x20000`. This is consistent with the bit becoming set between calls, but the exact write is not independently confirmed because the Stage16 writer log is zero bytes.
- StateMapTrace and ProducerCallTrace are also zero bytes. FinalCommandTrace has no payload-`0x1E` record at its monitored RVA `0xF7C5D`; this does not negate the confirmed separate alternate queue/dispatch route.
- No ERROR/WARNING/DROP/OVERFLOW/FAILED markers were found in the nonempty uploaded runtime logs.

### Resolver byte +3: downstream role established, semantic name still open

The runtime establishes byte +3 = `0x1F` in both P1 and P2 samples. Static disassembly of the same AALib binary shows it is passed as arg2 to helper `0x004F57D0` at CALL RVA `0x000EE306`. At that callsite:
- arg1 = pointer to `player+0x3A08`;
- arg2 = resolver byte +3 (selector `0x1F`);
- arg3 = resolver byte +2 (`0x03`);
- arg4 = resolver byte +4 (`0x12`).

The helper uses arg2/ESI as an index into a three-byte-per-entry mapping table at `0x04DDF188 + selector*3`: one flag byte and two mapped selector bytes. It stores the index in the output structure's first DWORD and uses the mapped bytes to set bits in the following DWORDs; flag bits may change initialization and trigger a secondary helper. The table is in writable `.data` and may be initialized at runtime. Therefore the strongest supported current description is **resolver mapping selector/index 0x1F**. Its high-level semantic category (attack, command, animation, etc.) is not established.

### Remaining evidence gaps and next stage

- `ArcanaAI_PlayerBit17WriteTrace.log`, `ArcanaAI_StateMapTrace.log`, and `ArcanaAI_ProducerCallTrace.log` were zero bytes.
- No actual nonzero-to-zero transition of fallback slot 3 or controller bit `0x400` was captured.
- Exact instruction that changes player bit17 is still absent from this run.

Next: Stage24 should add a passive observer at CALL RVA `0x000EE306` -> helper RVA `0x000F57D0`, without changing Stage23. Capture the four incoming args, the three runtime table bytes at `0x04DDF188 + 0x1F*3`, and the three DWORDs at `player+0x3A08` before/after the helper. Confirm whether the flag byte selects a secondary helper path. Preserve original args, return registers/EFLAGS and caller cleanup; never write table/player state.


## Stage23 runtime result — second r2 run (2026-11-09 09:04–09:07)

This is a second run of the existing Stage23 r2 binary, not a Stage24 run. The uploaded marker remains `STAGE23_RESOLVER_RESULT_BUILD=2026-11-09-r2`, the installed resolver hook is still at CALL RVA `0x000EE273 -> 0x0012EAC0`, and the queue/dispatch trace header requests Stages 12/21/22/23 only. No Stage24 marker, hook at `0x000EE306`, or Stage24 log is present.

Evidence branch: [stage23-resolver-result-runtime-evidence-run2](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence-run2), HEAD `0e6ed619a4e812687e615b00535ef17e54118b5b`. It preserves all 18 current uploaded runtime logs, the Build.log supplied in chat, and `Stage23_Overlay/STAGE23_RUNTIME_ANALYSIS_RUN2.md`. The first Stage23 capture remains unchanged at [stage23-resolver-result-runtime-evidence](https://github.com/Hawky11/1/tree/stage23-resolver-result-runtime-evidence), HEAD `306b006a2230c1226a49ab93b9b8b74722fd1683`.

### Build and hook installation

- Build began 2026-11-09 09:04:32,60 with Visual Studio 18 Community / MSVC 14.51.36231 x86.
- All eight translation units compiled; `build\\dinput8.dll` linked successfully at **229,376 bytes**.
- Only diagnostic was the known non-fatal C4754 warning at `AICommandSiteTrace.cpp:785`, associated with the bounds calculation beginning at line 782.
- Runtime initialized at 09:06:20. Stage23 r2 hook installed at `0x000EE273 -> 0x0012EAC0`, reporting 39 suspended threads. Runtime image base was `0x005B0000`.

### Resolver and payload-0x1E path

Three type-1 / payload-`0x1E` resolver records were captured, all readable and all on thread 16252:

| Side | Tick | ALT QPC | Resolver QPC | Δ | Args (arg1,arg2,arg3,arg4) | Record pointer |
|---|---:|---:|---:|---:|---|---|
| P2 | 1331046 | 13310812900 | 13310813095 | 195 | `(0x17,0x17,0x1E,0x38)` | `0x05C8DCA0` |
| P1 | 1332312 | 13323424185 | 13323424234 | 49 | `(0x0A,0x0A,0x1E,0x26)` | `0x0586F490` |
| P1 | 1350156 | 13501860518 | 13501860595 | 77 | `(0x0A,0x0A,0x1E,0x26)` | `0x0586F490` |

All returned the same 12 bytes as the first Stage23 capture: `60 FF 03 1F 12 00 00 05 13 24 03 00` (DWORDs `0x1F03FF60, 0x05000012, 0x00032413`). Byte +2=`0x03`, +3=`0x1F`, +4=`0x12`, +0x0B=`0x00`.

Each record correlates to Stage13 ALT_CALLSITE at RVA `0xF6AD7`, QueueTrace accepted insertion at return RVA `0xF6ADC`, EventDispatch at RVA `0xEE5CB`, Stage21 classifier/fallback and Stage22 post-AND:

| Side | Resolver QPC | Classifier QPC | Fallback QPC | Post-AND QPC |
|---|---:|---:|---:|---:|
| P2 | 13310813095 | 13310818324 | 13310820264 | 13310820882 |
| P1 | 13323424234 | 13323426193 | 13323426682 | 13323427028 |
| P1 | 13501860595 | 13501862845 | 13501863381 | 13501863736 |

All three classifier calls to `0xE8CF0` returned AL=0; fallback `0xF7EB0` ran with index 3; the Stage22 post-AND observer then executed. Slot 3 remained `0 -> 0`, and controller bit `0x400` remained `0 -> 0` in every sample. No actual nonzero-to-zero clear was observed. Stage21 `preCallEdi` remains the controller-array pointer, not the resolver mapping byte.

### Stage18 / Stage20 / Stage17 bit17 setter correlation

- Stage18 and Stage20 each recorded eight P2 entries. Sequential pairing by side/player/thread/tick, stack frame and QPC gives **8/8 matches**; object-guard checks followed their gate entries by 121–175 QPC ticks.
- All eight Stage18 gate results returned AL=1, key `0x19`, table byte `0xFA`.
- First six Stage20 object-guard results returned AL=1 with `object+0x3B70=0x4C`.
- Last two object-guard results returned AL=0 with `object+0x3B70=0x58` (bit2 clear), both at tick `1353484`. Their parent returns are `0x00646801` and `0x0067B9FA`, respectively.
- Stage17 logged exactly two P2 setter calls at CALL RVA `0x000C6502`, index `0x71`, same player `0x0E27CCA0`, with matching EBP/stack frames:
  - guard #7 QPC `0x326C2BD8E` → setter #1 `0x326C2BDD6` (**72 ticks**); `player+0x3B84` was 0.
  - guard #8 QPC `0x326C2C03B` → setter #2 `0x326C2C081` (**70 ticks**); `player+0x3B84` was `0x00020000`.
- Therefore the field was zero at the first AL=0 guard/setter entry and already `0x20000` at the second. The first setter call lies between the two observations. This is strong temporal and stack attribution consistent with the first original setter call setting bit17, but the exact memory-store instruction is not directly captured because `ArcanaAI_PlayerBit17WriteTrace.log` is zero bytes.

### Empty logs, scope-limited no-hits and diagnostics

- `ArcanaAI_PlayerBit17WriteTrace.log`, `ArcanaAI_StateMapTrace.log`, and `ArcanaAI_ProducerCallTrace.log` were zero bytes.
- `ArcanaAI_FinalCommandTrace.log` contains 45 records but none at its monitored callsite carries payload `0x1E`; this does not contradict the confirmed alternate queue insertion/dispatch route at `0xF6AD7`.
- No ERROR/WARNING/DROP/OVERFLOW/FAILED markers were found in nonempty uploaded runtime logs.

### Next task remains Stage24

The new run strengthens the Stage23 resolver result and the Stage18/20/17 setter attribution, but does not inspect the downstream mapping consumer. Stage24 must add a separate passive observer at CALL RVA `0x000EE306 -> 0x000F57D0`, capture its four arguments, the three runtime table bytes at `0x04DDF188 + selector*3`, and the three DWORDs at `player+0x3A08` before/after the helper. Use a distinct Stage24 marker and log filename. Keep both Stage23 evidence branches immutable. Read/write diagnostics only; do not alter mapping-table or player state.


## Current checkpoint — Stage24 runtime, 2026-11-09 16:35

- Tested source: [stage24-resolver-consumer-trace](https://github.com/Hawky11/1/tree/stage24-resolver-consumer-trace), commit `0bca8a6d8e851990563b281c517f01bd1a0c81a2`.
- Evidence: [stage24-resolver-consumer-runtime-evidence](https://github.com/Hawky11/1/tree/stage24-resolver-consumer-runtime-evidence), commit `983a7f4f205c99cd1c8f44814c58cc09ed76f0c8`. See [full analysis](https://github.com/Hawky11/1/blob/stage24-resolver-consumer-runtime-evidence/Stage24_Overlay/STAGE24_RUNTIME_ANALYSIS.md).
- User uploaded logs to Stage23 branch at `96464289b7c41c4f62402b75ad1081d191ae999b`; this is a Stage24 r1 run despite the branch name. Do not treat that branch HEAD as the old Stage23 source snapshot; old Stage23 r2 source remains at `6e5f53da1e6959db840a84bbb9bebaad8894bd4b`.
- Build 16:35:12: eight units compile, DLL233984 bytes, only C4754 at AICommandSiteTrace.cpp:785. Marker and hook EE306->F57D0 confirmed.
- Two P1 events: selector0x1F, actual arg3=0/arg4=0, table pre-entry bytes03,00,10; output (6,0x1002,0)->(0x1F,0x10001,0) and (4,0x11,0)->(0x1F,0x10001,0). All reads successful. Both correlate to Stage23 and queue/dispatch/classifier/fallback/AND. Slot3 and bit0x400 stay zero.
- **Correction:** previous predictions arg3=3/arg4=18 were not confirmed. Resolver bytes remain3/18; full intervening dispatcher dataflow needs review. Wrapper argument-copy/replay/log offsets were reviewed without finding an overwrite.
- Flags3 suggest both static flag paths; secondary-helper execution is not directly captured. Higher-level selector semantics remain unknown.
- All19 log blobs and pasted Build.log preserved. Read17 logs fully without explicit error/drop/overflow markers. Two large logs (PlayerBit17WriteTrace and ProducerCallTrace) could not be read with available connector transport; do not describe them as empty or claim their write transitions were verified.
- Next: retrieve those two large logs; inspect full EE278..EE306 disassembly to explain zero arguments before selecting Stage25 instrumentation. No Stage25 code has been created.



## Current checkpoint — Stage25 prepared, Stage24 follow-up complete

- Source-only Stage25: [stage25-secondary-object-flag-trace](https://github.com/Hawky11/1/tree/stage25-secondary-object-flag-trace), commit `3f034550f9d1254cc136452dde363657f913a91c`. Apply `Stage25_Overlay/src/AIQueueTrace.cpp` and `Stage25_Overlay/ArcanaAI.ini` to the working Stage24 project. No project-file change.
- [Follow-up analysis](https://github.com/Hawky11/1/blob/stage25-secondary-object-flag-trace/Stage25_Overlay/STAGE24_FOLLOWUP_AND_STAGE25_STATIC.md) completes the two previously unread large Stage24 logs and corrects the argument dataflow using the fingerprinted EXE.
- Both large files fully retrieved and exact Git blob hashes verified. Write trace:21886 records, no missing/duplicate sequence IDs, blocks can appear out of order. Producer trace:10463 ordered records without gaps. No explicit errors/drops/overflow markers.
- P2 setter->bit17 OR confirmed: Stage17 QPC0x41E9BA1075 -> write seq15206 QPC0x41E9BA11CE, delta345. Clear seq15215 at ED19F uses **0xFFF9FFFF**, not the previously stated0xFFFDFFFF; it clears bits17/18, with observed0x30000->0x10000.
- **Argument issue resolved:** EE2F0 reloads ECX from player+3F34 (arg4); EE2F6 reloads EDX from player+3F30 (arg3). Resolver bytes3/18 are not the final arguments. Zero/zero in Stage24 is correct for those fields.
- **Secondary-path correction:** F58C1 calls9EF00(primary arg3,-1), not arg4. With arg3=0 it selects slot1; Stage24 returned EAX matches P2. Stage25 observes the original OR[target+F8],1 at F58C9 directly.
- Stage25 log:`ArcanaAI_Stage25SecondaryObjectFlagTrace.log`; marker:`STAGE25_SECONDARY_OBJECT_FLAG_BUILD=2026-11-09-r1`. Records source/target player, selector/args, before/after field and QPC. Original OR executes once; no extra game-state writes.
- Validation: exact PE SHA256 and patch signatures; extracted x86 wrapper assembled and compared with original OR in128 randomized Unicorn cases (registers, ESP, EFLAGS, output, logger context). **Not compiled with MSVC and not tested in-game yet.**
- Next: inspect user's new Build.log, then Stage25 marker/hook installation and records. Correlate Stage25 QPC between Stage23 and Stage24 records. Establish whether target bit0 transitions0->1 or is already1; gameplay semantics remain unknown.
- Stage24 evidence remains immutable at `983a7f4f205c99cd1c8f44814c58cc09ed76f0c8`. Its initial transport limitation is superseded by this follow-up; no old logs or historical source branches were rewritten.



---

## Stage25 — runtime-verified secondary-object OR, 2026-11-09

- Original source snapshot: [stage25-secondary-object-flag-trace](https://github.com/Hawky11/1/tree/stage25-secondary-object-flag-trace), `3f034550f9d1254cc136452dde363657f913a91c`.
- New immutable runtime evidence: [stage25-secondary-object-flag-runtime-evidence](https://github.com/Hawky11/1/tree/stage25-secondary-object-flag-runtime-evidence), `b426a8f74764b7606ce3b5da075bb3a61d3698b`, preserving 20 uploaded logs from main upload commit `b9ba7472ccaeea110b8828098d04b23a264527df` by identical Git blob SHA, source/INI, hand-pasted chat Build.log normalized as `Stage25_Overlay/Build_log_from_chat.txt`, and `Stage25_Overlay/STAGE25_RUNTIME_ANALYSIS.md`. Original Build.log was not uploaded.
- Build from chat started 09.11.2026 16:50:57.87: VS 18 Community, MSVC 14.51.36231 x86, 8 C++ files compile, `dinput8.dll` successfully linked at 237,568 bytes. The existing single C4754 warning in `AICommandSiteTrace.cpp:785` persists; no compiler error.
- `STAGE25_SECONDARY_OBJECT_FLAG_BUILD=2026-11-09-r1` at 16:51:44; validated 7-byte OR replacement installed at RVA `0xF58C9`. One 16:52:07 runtime sample P1/tid6128/tick29260265 captures source player `0x0E251F78`, target `0x0E27CCA0` (P2) and target DWORD `0x0E27CD98` before and after the original `OR [EAX+0xF8],1`: both `0x00000001`. Both reads succeeded; bit0 was already 1.
- Same-event QPC: alternative call `292602968311`; queue insertion `292602968399` (type1/payload0x1E, accepted); dispatcher start `292602968416`; Stage23 resolver `292602968420` (selector31); Stage25 OR `292602971617`; Stage24 helper completion `292602973136` (mapping bytes03/00/10, output `(0,1,0)→(0x1F,0x10001,0)`, helper EAX same as P2); Stage21 classifier `292602975570` AL0; fallback slot3 `0→0` at `292602976845`; Stage22 controller bit0x400 `0→0` at `292602977325`; dispatcher end `292602977883`. The Stage25 event is precisely inside the Stage24 helper, not a separate action.
- Full large-log inspection: Stage16 write trace 12,948,453 bytes with seq1..24176 all present/unique; Stage14 producer trace 8,833,751 bytes with seq1..11383 all present/unique. Both newline-terminated, neither has explicit error/warning/drop/overflow markers. One bit17 set+clear cycle for P1 (seq3119/3128) and one for P2 (seq13050/13059); these touch player+`0x3B84`, not target+`0xF8`. Stage15 has 10 branch records, including an accepted P1 payload0x1E event with bit17=0 and later P2 suppressed bit17=1 event.
- **Limit:** only one filtered Stage25 OR was observed; a 1→1 write does not show the initial writer. No gameplay-semantic attribution of +0xF8 bit0. Proposed Stage26: static writer xrefs and then (if needed) protected, noninterfering P1/P2 +0xF8 write observation with unique marker/log, while preserving Stage16 DR0/DR1.


---

## Stage26 — source-only P1/P2 +0xF8 bit0 write trace prepared, 2026-11-09

- Source branch: [stage26-object-bit0-write-trace](https://github.com/Hawky11/1/tree/stage26-object-bit0-write-trace), HEAD `a249f452353c340fe8f1dab23375a8a932b079b4`. Parent source snapshot is Stage25 `3f034550f9d1254cc136452dde363657f913a91c`. All prior source/evidence branches preserved.
- Motivating Stage25 result: P1/type1/payload0x1E selected P2; original `OR DWORD PTR [EAX+0xF8],1` at F58C9 observed target P2+`0xF8` `1→1`. Previous setter and gameplay semantics unknown.
- Static approach: adapt version-controlled Stage20 `Stage20_Overlay/src/AITrace.cpp` hardware write-watchpoint pipeline for a new Stage26 mode. New field offset `0xF8`, mask0x1, derived watchpoint RVAs P1 `0x0DCA2070` / P2 `0x0DCCCD98`. Preserve known DR0/DR1 write-only configuration `0x00DD0005`, DR2/DR3, external DR owners and original game writes.
- New INI `EnableStage26ObjectBit0WriteTrace=1`, `Stage26TransitionsOnly=1`; legacy `EnableControlFieldTrace=0` and Stage16 `EnablePlayerBit17WriteTrace=0`, checked mutually exclusive. Existing Stage25 other tracing remains enabled. Stage26 marker `STAGE26_OBJECT_BIT0_WRITE_BUILD=2026-11-09-r1`, log `ArcanaAI_Stage26ObjectBit0WriteTrace.log`.
- Existing Stage16 record buffer is reused: capture side, thread, tick/QPC, post-write EIP/RVA, registers/DR6, code window and eight guarded stack DWORDs, old/new shadow bit0, plus baseline P1/P2 values. Default suppress unchanged bit0 writes after updating shadow; log filtered counts periodically and report dropped records. For unfiltered traffic use `Stage26TransitionsOnly=0`.
- Static source/INI invariants checked in the creation session; the bundled verifier performs equivalent source/INI address and toggle checks. **No successful MSVC x86 build or in-game runtime is claimed.** An entire AALib.exe binary was not part of the source repository, so a complete static search of writer sites remains unavailable. A P1/P2 value already 1 at startup would not identify the original setter.
- Next: user applies Stage26 `AITrace.cpp` and INI over working Stage25 project (retaining Stage25 `AIQueueTrace.cpp`), builds and uploads a new game run. Only then analyze write attribution and archive evidence.


---

## Stage26 — full runtime evidence of player+0xF8 bit0 lifecycle (2026-11-09)

- Original source branch [stage26-object-bit0-write-trace](https://github.com/Hawky11/1/tree/stage26-object-bit0-write-trace), HEAD `a249f452353c340fe8f1dab23375a8a932b079b4` (immutable). Evidence branch [stage26-object-bit0-write-runtime-evidence](https://github.com/Hawky11/1/tree/stage26-object-bit0-write-runtime-evidence), HEAD `4b7564ff71664fedd5745e3bcfc4210d74540075`, includes exact root-uploaded 21 Git log blobs, chat Build.log transcript and detailed `Stage26_Overlay/STAGE26_RUNTIME_ANALYSIS.md`. Root upload commit `8c6edc82d3146c11b3dd23d964471c31ba3fe816`.
- Build: 2026-11-09 17:27:39, VS18 Community MSVC14.51.36231 x86, eight sources successful, DLL size 240,640 bytes. Warnings: newly introduced nonfatal C4456 at `AITrace.cpp:1671` (inner `reported` hides outer `reported`), existing unrelated nonfatal C4754 at `AICommandSiteTrace.cpp:785`. Loaded Stage26 marker and startup installation of write-only P1/P2 DR0/DR1 proved in runtime; configurations `Stage26TransitionsOnly=1`, Stage16 disabled. Initial P1 and P2 +0xF8 values both 0. Six watchpoint installation batches 38+1+16+1+1+2 =59 game threads; COM writer thread excluded, existing owners untouched.
- Trace `ArcanaAI_Stage26ObjectBit0WriteTrace.log` 89,929 bytes, 150 newline-terminated lines including 140 seq1..140 event records, no gaps/duplicates. All 140 events are real bit0 transitions with shadowInitialized=1, all on thread18044. P1 82 events (41 `0→1`,41 `1→0`); P2 58 (29/29). No explicit warnings/error/dropped/overflow. The shadow is best-effort and uninstrumented-thread writes remain a limitation.
- Exactly 70 observed `0→1` transitions execute Stage25's relocation of original game `OR DWORD PTR [EAX+0xF8],1` at RVA `0xF58C9`; hardware post-EIP points into DLL `0x50B0C267` (do not assign `AALib RVA=0` as game writer). Original seven instruction bytes `83 88 F8 00 00 00 01` confirmed in Stage26 code window. The evidence covers the observed run, not an assertion of all possible paths.
- 70 `1→0` clear transitions break down by post-write RVA: `0x9787A` 54 writes from `0x97873: AND [EDI+0xC8],0xFFFFFFF2`; `0x975AC` 7 from `0x975A6: MOV [ESI+0xC8],EAX` after `AND EAX,0xFFFFFFFE`; `0x95F5B` 8 from `0x95F55: MOV [EDI+0xC8],EAX` observed EAX=0; `0x97037` 1 from `0x97031: MOV [ESI+0xC8],EBX` observed EBX=0. The EDI/ESI base equals player+0x30, so +0xC8 is player+0xF8. Some paths reset additional bits or whole DWORD. Gameplay meaning not proven.
- Two exact P2/type1/payload0x1E events: ALT/queue dispatch accepted; Stage23 resolver selector31; Stage24 arg3=1, output mask0x10001 and helperResultEAX=P1; Stage25 target P1 0→1. Stage26 trap seq1 QPC314503270177 precedes Stage25 QPC314503270399 by222 ticks; seq87 QPC314608006662 precedes Stage25 QPC314608006859 by197 ticks. Later clears are Stage26 seq2 and seq88 at post-RVA0x9787A. Stage21 AL=0, fallback slot3 0→0, Stage22 bit400 0→0 in each episode.
- Fully read fresh Stage14 `ProducerCallTrace` 7,563,025 bytes / 9,730 ordered unique seq IDs and Stage19 `StateMapTrace` 1,268,525 bytes / 2,266 unique IDs; both newline terminated and without explicit errors/drops/overflow. Stage15 includes fourteen branch records with two accepted P2/bit17=0 events. Current `ArcanaAI_PlayerBit17WriteTrace.log` file is **identical stale Stage25 blob SHA `befd639d44731fd9cb5b35de44d49878406d716c`**, not current Stage26 evidence because Stage16 was disabled.
- Stage26 answers the concrete setter/clearer question. Stage27 candidate: inspect clear site parent functions/callers and state conditions without new instrumentation unless evidence demands it. Keep all Stage25 and Stage26 source/evidence branches untouched.


---

## Stage27 — static lifecycle and dynamic clear-parent reconstruction (2026-11-09)

- [Stage27 branch](https://github.com/Hawky11/1/tree/stage27-bit0-lifecycle-callers), commit `b490f87892791d45f0b0841a47e70d60329ec457`, created as an immutable extension of Stage26 evidence branch `stage26-object-bit0-write-runtime-evidence` at `4b7564ff71664fedd5745e3bcfc4210d74540075`. [Full analysis](https://github.com/Hawky11/1/blob/stage27-bit0-lifecycle-callers/Stage27_Analysis/STAGE27_LIFECYCLE_ANALYSIS.md). No new game hook, DLL, MSVC build, or in-game evidence in Stage27: this is **offline analysis** of the already verified Stage26 data.
- Fully re-paired Stage26 `ArcanaAI_Stage26ObjectBit0WriteTrace.log` (original blob SHA `eeb87824a4cd10e573ef84ca124f93c9cd9b684c`): 140 sequential transitions, 70 matching set→clear cycles; no orphan, overlapping or open sets. Per-side: P1 41 set/clear pairs and P2 29. Derived complete `Stage27_Analysis/cycles_stage26.csv` contains 70 rows; aggregate `stage27_summary.json` cross-checked for 70/70 counts.
- Primary clearer at instruction RVA`0x97873`, post-EIP `0x9787A`, 54 clears (P1 28, P2 26). Stage26 code bytes show apparent helper entry RVA`0x97870`: `57 8B F8 83 A7 C8 00 00 00 F2` = `push edi; mov edi,eax; and [edi+0xC8],0xFFFFFFF2`. With EDI=`player+0x30`, this clears bits0,2,3 at `player+0xF8`.
- **New dynamic parent evidence:** because only PUSH EDI modifies ESP before trapped AND, `stack[1]` credibly records helper return. P1/P2 combined: return RVA`0x979D1` observed34 times (P1 16, P2 18), `0x9760E` 20 times (P1 12, P2 8). All 34 from first group additionally show `0x9760E` deeper at stack[4], consistent with nested helper path. Candidate E8 CALL instruction starts `0x979CC` and `0x97609` are inferred, NOT verified static xrefs without the pinned PE.
- Other three clear paths: instruction RVA`0x975A6` 7 P1 (MOV after AND EAX,FE); RVA`0x95F55` 8 (MOV EAX=0, P1 five/P2 three); RVA`0x97031` 1 P1 (MOV EBX=0 alongside other contiguous zero writes). Lifetimes by clear post-RVA median QPC ticks: `0x9787A` 97,068; `0x975AC` 99,990; `0x95F5B` 3,704,260; `0x97037` 20,731,060. Raw ticks, not clock-time conversions.
- All 70 sets execute original `OR [EAX+0xF8],1` from RVA`0xF58C9` via Stage25 DLL wrapper; in each setter EAX targets a player object, ESI the other player. Setter EBX is variable; exactly two cases EBX=`0x1E`, matching the two Stage25/P2 payload0x1E events (Stage26 setSeq1→clearSeq2, and setSeq87→clearSeq88). Both clear via primary function with nested-return RVA`0x979D1`. Wider semantics of flag and EBX not proven.
- Produced stdlib-only Python `tools/analyze_bit0_lifecycle.py` and five `tests/test_stage27.py` regression tests, which were **not run on the user machine**. Optional `--binary` validates exact SHA256 of AALib.exe, all four site signatures and putative rel32 E8 CALLs, emits short static report; Capstone disassembly optional. Full PE is not stored in GitHub, so complete static parent xrefs remain pending.
- Next: user runs optional script locally against **exact original AALib.exe** and uploads only `Stage27_AALib_StaticReport.txt`; examine real preceding branches and parent call graph. Do not equate bit0 to suppression of the type1/payload0x1E event without causal evidence. Preserve Stage25/26 historical branches untouched.


---

## Stage27 verified static PE report — corrected CALL graph, 2026-11-09

- User uploaded `Stage27_AALib_StaticReport.txt` after running the original Stage27 Python `--binary` check on `G:\Arcana Heart 3 - LS XTEND\AALib.exe`. Exact CRLF 2,389 bytes were archived **byte-for-byte** as Git blob `7e3dc3a77f602fde1e82ca9fe70903d4d29dd9c7` in new [stage27-verified-static-callgraph](https://github.com/Hawky11/1/tree/stage27-verified-static-callgraph), commit `2e08b356fa9923e57b0c78c08c70adbf8e1015c1`. Parent Stage27 branch `b490f87892791d45f0b0841a47e70d60329ec457` preserved unchanged. The report confirms AALib SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` and all four writer signatures (0x95F55,0x97031,0x975A6,0x97873), plus 0x97870 helper prologue.
- The initial Stage27 prediction was **partly wrong**: CALL at RVA0x979CC is E8→RVA0x97870, return0x979D1 (**verified**); CALL at RVA0x97609 is E8→RVA0x97970, return0x9760E (**verified; does NOT target RVA0x97870**). Stage26 dynamic 34+20 stack-return clusters remain true. For 20 stack[1]=0x9760E captures inside 0x97870, transfer via 0x97970 may be a tail jump/trampoline or another control-flow mechanism; the local report lacked 0x97970 bytes, so **do not assert mechanism yet**.
- Expanded decoding from verified report's 0x97853 hex window shows full 0x97870 helper until RET0x978AA: clears bits0,2,3 at [player+0xF8], writes [player+0x104]=2, calls 0xF7EB0,0x976D0,0x99150 and sets bit0x400 at [player+0xF4]. Distinct from Stage22 controller+8 flag. Window 0x97586 confirms conditional clear at RVA0x975A6 after `CMP WORD [EBX+0x3A34],1` and `TEST AL,1`; other 0x95F55/0x97031 clear routines reset several fields.
- Added self-contained read-only `Stage27_VerifiedStatic/tools/inspect_call_flow.py`, which pins SHA256, checks both real E8 destinations and exports windows around `0x97970` (which was not included in original report), plus 0x97609 and clearers. Its five synthetic tests passed locally; **the follow-up original PE report was not yet produced**. Next: user runs script and uploads only `Stage27_CallFlow_97970.txt`; no DLL rebuild or new game session. See [VERIFIED_CALL_GRAPH.md](https://github.com/Hawky11/1/blob/stage27-verified-static-callgraph/Stage27_VerifiedStatic/VERIFIED_CALL_GRAPH.md).


---

## Stage27 completed — exact 0x97970 dual path to bit0 clearer (2026-11-09)

- After Stage27 initial archive `b490f87892791d45f0b0841a47e70d60329ec457` and static direct-CALL verification `2e08b356fa9923e57b0c78c08c70adbf8e1015c1`, the user ran the new offline original-PE inspector and uploaded `Stage27_CallFlow_97970.txt`, 4,257 bytes CRLF, SHA1 Git blob `d5d8c9f90665baf1c625bb65515910c2984b7a64`. Archived byte-for-byte in **[stage27-bit0-tailjump-verified](https://github.com/Hawky11/1/tree/stage27-bit0-tailjump-verified)**, commit `78e48cdcef7a67fcdf64a50dfa95228357eedbe6`; detailed x86 reconstruction `Stage27_CompleteCallGraph/VERIFIED_97970_BRANCH.md`.
- Original pinned PE SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` verified in report. Decoded the contiguous 0x97970–0x979DF bytes with standard GNU objdump on extracted original x86 bytes; no runtime modifications.
- **Proven state switch:** `0x97970` reads `[ESI+0xD4]` (captured ESI=player+0x30, so player+0x104). State0 calls `0x978B0` without clearing. State1, after guards `[esi+C4]&1==0; [esi+CC]&7==0; [esi+C8]&1==1`, uses `0x979CC CALL 0x97870`; after return at `0x979D1`, calls `0x978B0`. **34** Stage26 clear records with stack[1] RVA0x979D1 (P1 16,P2 18). State2, with same guards, uses `0x979AC JMP 0x97870` *after removing its own ESI/EBP frame*, preserving external caller return `0x9760E`. **20** Stage26 clear records (P1 12,P2 8). Nonmatching states/failed guards return. This resolves both prior dynamic caller-return groups with exact static control flow.
- Original clear helper at `0x97870` masks `[player+0xF8]` with `0xFFFFFFF2`, writes `[player+0x104]=2`, calls `0xF7EB0`, `0x976D0`, `0x99150`, and sets `[player+0xF4]|=0x400`. Do not confuse this with Stage22 controller +8 bit400. All Stage26 54 primary clears are explained by state1/state2 routes. Both captured P2 source `payload0x1E` events followed state1 and nested CALL; no proof all payload1E events must do so.
- Open: `0x978B0` helper side effects, deeper state semantics, and any link to original COM-vs-COM suppression behavior. No Stage28 new hook or build has been implemented. Preserve prior snapshots.


---

## Stage28 — original PE state rearm/bit1 branch verified (2026-11-09)

- Source of exact binary: user's ChatGPT Library `/AALib.zip` (13MB compressed). Extracted `AALib.exe` 175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, same as earlier fingerprint. Original executable used **only locally** for read-only PE analysis and was not uploaded to GitHub. .text x86 RVA0x1000, 0x2BDEBE virtual bytes; disassembled full .text with GNU objdump; signatures and direct rel32 transfers validated.
- Created [stage28-original-pe-state-lifecycle](https://github.com/Hawky11/1/tree/stage28-original-pe-state-lifecycle), HEAD `6a210f4be510ab264ddefcadd8d740bfe923cfa4`, parent preserved Stage27 `stage27-bit0-tailjump-verified` `78e48cdcef7a67fcdf64a50dfa95228357eedbe6`. `Stage28_Analysis/STATIC_STAGE28_ANALYSIS.md` documents every branch, register, pointer offset and uncertainty; `tools/verify_stage28.py` implements pinned-PE signature checks. Local verifier passed 26 instruction signatures, 16 direct CALL and one JMP targets, plus 6 stdlib tests (including real pinned original EXE); no C++ changes/runtime run.
- Main finding: `0x978B0` (called by `0x979D2`, after Stage27 state1 clear and on state0) first uses `0x9EEA0` to select a fighter, then reads bit1 of object field +0xC8 (player+0xF8). If bit1 set, clears bit1 and compares selected fighter's `[fighter+0x3A34]` and `[fighter+0x3A36]` against `(0,0x13)`, `(0,0x14)`, `(0,0x15)` via `0xF2ED0`. If all comparisons fail, it bypasses state/countdown gates, calls `0x97830` and returns AL=1. `0xF2ED0` only compares two fields and returns 0/1, **not** queue-insertion `0xF2DE0`.
- For ordinary route `0x978B0` checks object+0xC4 bit0 clear, object+0xCC bit1 clear, state `[object+0xD4]==0`; invokes `0x991E0(object+0x10,0)`, which returns old slot0 counter, decrements it by base stride and clamps negative to zero; if old counter `<=0`, `0x97830` writes state1. The helper `0x97830` writes state1, `0x97870` writes state2 (and clears object+0xC8 bits0,2,3 while preserving bit1); both call 0xF7EB0/0x976D0(mode0 or1)/0x99150, and set object+0xC4 bit0x400.
- Stage26 old/new **full DWORD** recheck: 140 transition-only records with exactly 70 0x0→0x1 and 70 0x1→0x0; bit1 not present at those sampled boundaries. Because `Stage26TransitionsOnly=1`, non-bit0-change writes are suppressed; cannot exclude transient bit1 writes or infer branch runtime usage. Static semantics are technical, not known gameplay meanings. No demonstrated causal bridge to Stage15 bit17 producer suppression nor Stage21 classifier fallback.
- Handoff Stage29: return to COM-vs-COM original 0x1E production/dispatch problem. Prior Stage15/16/21/23/24 data establish P1/P2 branch divergence and classifier AL0/fallback, while Stage28 gives state lifecycle context but not proof of suppression. Preserve all branches; check existing evidence before deciding on a new passive instrument. Library AALib.zip can be located anew and materialized in future sessions without asking user to reupload.


---

## Permanent source location — original AALib.exe in ChatGPT Library (2026-11-09)

- At the user's explicit request, recorded a stable access guide in [BINARY_SOURCE_IN_LIBRARY.md](BINARY_SOURCE_IN_LIBRARY.md), linked prominently from README.md and NEXT_SESSION.md. The original game binary is already in the user's **ChatGPT File Library** at **`/AALib.zip`**, contains `AALib.exe`, size 175,556,608 bytes. SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. ZIP size 12,865,549 bytes.
- Retrieve in future sessions through `files__list(surface=library)` then `files__materialize` with that session's returned file_id; verify SHA256. The file is NOT checked into GitHub. Future assistants must use the Library before asking the user for an executable.


---

## Stage29 — same-session bit17 producer/dispatcher diagnostics prepared (2026-11-09)

- **Immutable Stage29 source branch** [stage29-bit17-producer-correlation](https://github.com/Hawky11/1/tree/stage29-bit17-producer-correlation), HEAD `ad78d4668bd4ca14fc471289037bd6ef9ed62f0b`, base Stage28 `6a210f4be510ab264ddefcadd8d740bfe923cfa4`. Contains `Stage29_Overlay/src/AITrace.cpp` and INI plus `README_STAGE29.md`, `STAGE29_STATIC_BASELINE.md`, Python static verifier, and persistent Library EXE location guide. The branch contains no game binary, no new runtime log and **has not been MSVC compiled/tested**; static configuration checks only.
- Using the verified original executable `AALib.exe` already in the user's persistent ChatGPT Library `/AALib.zip`, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, plus full Stage15 and Stage21/22/23/24 runtime logs from **the same 17:28 game session**, established four parent `payload=0x1E` attempts: P1 at ticks31451546,31457515 (bit17=1, Stage15 F6A68 JE not taken, follow native `0xF6A75` alternate branch, no F6AD7 observed); P2 ticks31450296,31460765 (bit17=0, JE taken and reached `0xF6AD7` queue insertion and dispatcher). This does not prove the P1 alternate branch is defective: it is a real original game action path that calls `0xE8560`, `0x156C00` and `0x130A10` then returns.
- P2 `type1/payload0x1E` enters classifier `0xE8CF0`, for which `AL=0` is a deterministic outcome because 0x1E is below 0x20 but above 0x17, regardless of intervening state check. The fallback at `0xEE443` clears Stage21 array slot3 already0→0 and Stage22 controller bit0x400 already0→0. These are two distinct behaviors: **P1 no queue path because bit17=1** vs **P2 expected classifier AL0/idle state cleanup**. Do not treat AL0 as malfunction without comparing gameplay behavior and actual expected action.
- Earlier Stage16 *different game session* 16:51 produced 24,176 write records against `player+3B84` with exactly four bit17 toggles, 2 from0→1 at setter post-EIP RVA0xFAD2 and 2 from1→0 at clearer post-EIP0xED1A9. Cannot QPC-align these with the Stage15 17:28 branch events. This is why Stage29 uses write-only DR0/DR1 with a **transition-only filter** (shadow updated on every trap) so setter/clearer and Stage15 producer can be caught together in a new natural run. New marker `STAGE29_BIT17_TRANSITIONS_BUILD=2026-11-09-r1`, new `ArcanaAI_Stage29Bit17Transitions.log`, Stage29-only filter status and stack snapshot.
- Copied only the Stage26 `AITrace.cpp` with small edits for Stage29 + INI. Stage25 working `AIQueueTrace.cpp` is retained in user project. DR0/DR1 conflicts guarded, no writes to game state; older Stage26 mode remains available by toggles if needed. Fixed the prior nonfatal C4456 variable shadow in the Stage29 source, while preserving tested Stage26 source history.
- **Next action:** user applies Stage29 AITrace.cpp/INI, runs MSVC x86 Build.bat, tests short COM-vs-COM and uploads new Build.log and same-run Stage29/Stage15/Stage17/queue/dispatch/Stage21 logs. Inspect any build errors then QPC align with previously known gate and queue path, distinguish initial bit17=1 from a new 0→1 captured transition. Archive any runtime evidence in its own branch before further changes.

---

## Stage29 runtime — bit17 setter/gate same-run confirmation (2026-11-09 20:35)

- Evidence archived without rewriting source or older branches: [stage29-bit17-runtime-evidence](https://github.com/Hawky11/1/tree/stage29-bit17-runtime-evidence), commit `d5f2cd0266759aa95f8617a3eb3e3770040d73d3`, parent source branch `ad78d4668bd4ca14fc471289037bd6ef9ed62f0b`. Exact 20 newly changed/added logs copied by blob SHA from user upload on `main` commit `c5d49fec910f96e621e7502fb2a2cc5d5f1fb57d`; detailed machine-readable results in [runtime analysis](https://github.com/Hawky11/1/blob/stage29-bit17-runtime-evidence/Stage29_Runtime/RUNTIME_ANALYSIS.md) and [CSV](https://github.com/Hawky11/1/blob/stage29-bit17-runtime-evidence/Stage29_Runtime/payload_1E_correlation.csv). New upload omitted `Build.log`; runtime marker and actual trap events verify in-game Stage29 operation but not Windows build transcript.
- Stage29 `STAGE29_BIT17_TRANSITIONS_BUILD=2026-11-09-r1`, DR7 `0x00DD0005`, observed P1/P2 field addresses match expected `player+0x3B84`. Eight contiguous transitions (P1: 2 set/2 clear; P2: 2 set/2 clear); set post-RVA `0xFAD2`, clear post-RVA `0xED1A9`. Stage17 `0xC6502` had eight setter calls, index 0x71 (two per set-clear pair). Last periodic filtered count 21456 suppressed, 8 recorded; this is not a shutdown total. No explicit overflow/drop/error seen in Stage29 or Stage14 producer logs.
- Same run **five** Stage15 `F6A68` checks with EDI=0x1E: P1 tick42674656 bit17=0 branchTaken=1 → F6A9D and F6AD7 queue insertion at QPC426746876512/426746876587 → dispatch. Four bit17=1 checks: P2 tick42674953, P1 tick42682546, P2 tick42689218, P1 tick42691156; all bit17 branchTaken=0, no queue. The Stage29 hardware-observed set strictly precedes *each* of these four F6A68 gates and clear strictly follows within same tick (QPC table in report). This resolves the missing same-run bit17 writer correlation. The different queue outcomes follow native bit17 state, **not player side**.
- Queued P1 0x1E dispatch: Stage21 classifier AL0, slot3 0→0, Stage22 bit400 0→0; Stage23 selector31, Stage24 output `0x00010001`, Stage25 secondary target P2 bit0 0→1. This does **not** imply a classifier fault. For bit17=1, prior exact original-PE static analysis identifies independent alternative processing calls (0xE8560,0x156C00,0x130A10) instead of queue; not proven a gameplay malfunction.
- Next Stage30 investigation: offline verify alternative-path observable effects and semantics before introducing new passive hooks. Full original binary remains in Library `/AALib.zip`; preserve immutable historical evidence.

## Stage30 — native alternate path statically verified (2026-11-09)

- **Completed offline analysis:** [stage30-native-alternate-path-analysis](https://github.com/Hawky11/1/tree/stage30-native-alternate-path-analysis), commit `78c2358d57d7b31814cea61eeb94aa49bb996473`, parent Stage29 evidence `d5f2cd0266759aa95f8617a3eb3e3770040d73d3`. [Detailed findings](https://github.com/Hawky11/1/blob/stage30-native-alternate-path-analysis/Stage30_Analysis/STATIC_ANALYSIS.md), 12 bounded disassembly windows and reproducible SHA-pinned Python/objdump verifier. Exact original AALib.exe verified; 16 direct calls, 9 signatures and 5 constants passed. Wrong-binary rejection also checked. No DLL changes/build/new runtime.
- **New finding:** bit17 alternate route has real state writes and conditional chain `130A10 -> 12F3B0 -> 156DA0 -> F2DE0`, final CALL `156E47` / return `156E4C`, type0 and payload0/1/4/5. “Bypasses queue” means bypasses **type1/0x1E at F6AD7**, not necessarily all queue activity. The producer forces AL1 after 130A10; that is not a downstream success result.
- E8560 is shared by both producer routes; conditionally changes object direction bit3 and negates two floats. 156C00 copies four DWORD references. 130A10 clears counters, changes object flags/state block, and invokes E6C60 (which sets P+3B84 bit16); branch priorities depend on bits20/21 and entry bits17/19/20/21. Details distinguish confirmed operations from inferred gameplay meaning.
- Stage29 seq2/4/6/8 all show `0x30000 -> 0x10000`: bit16 present before bit17 clear, consistent with E6C60 but **not proof of its writer site**, because unchanged-bit17 writes were filtered. Existing QueueTrace contains accepted type0 records at 156E4C, but aggregation lacks per-event QPC/parent context: do not pair them with the four episodes.
- **Next Stage31 (not implemented):** bounded passive wrapper at F6A8C -> 130A10, entry/post QPC and relevant state snapshots; extend existing queue observer for per-event 156E4C type0 with same-thread scoped parent context. First inspect source filters/aggregation. Preserve state, args, registers/EFLAGS/FPU semantics. No forced bit17 or duplicate queue insertion. Gameplay defect remains unproven.

## Stage31 — alternate-path observer source prepared (2026-11-09)

- **Source-only branch:** [stage31-alternate-path-runtime-trace](https://github.com/Hawky11/1/tree/stage31-alternate-path-runtime-trace), commit `e905316353b844f276a40dff33516d48aa1e4d99`, based on Stage30 `78c2358d57d7b31814cea61eeb94aa49bb996473`. [Installation, log schema, validation and limitations](https://github.com/Hawky11/1/blob/stage31-alternate-path-runtime-trace/Stage31_Overlay/README_STAGE31_RU.md).
- Apply **only** `Stage31_Overlay/src/AIQueueTrace.cpp` and `Stage31_Overlay/ArcanaAI.ini` to the working project. **Retain Stage29 AITrace.cpp and all other files.** New queue source is based on working Stage25 queue source, retaining Stage21–25 hooks. No project/header changes. Keep historical branches unchanged.
- New five-byte CALL wrapper at F6A8C -> 130A10 captures BEGIN/END with per-thread nested episode IDs and 32 raw DWORD fields/read mask. A separate per-event queue record observes type0 returnRVA156E4C, with before/after snapshots, QPC, current episode (0 when unscoped), and parent player. It does not change the old type1 detail filter or aggregate logger.
- Marker `STAGE31_ALTERNATE_PATH_BUILD=2026-11-09-r1`; dedicated `ArcanaAI_Stage31AlternatePathTrace.log`; INI `EnableStage31AlternatePathTrace=1`. Requires installed queue observer. Ring capacity1024; depth8; explicit dropped/overflow warnings. Formatting/I/O stays on worker. The alternate wrapper preserves GPR/EFLAGS and uses aligned FXSAVE/FXRSTOR around diagnostic calls; original helper invoked once.
- **Validation completed:** original EXE SHA and four source byte signatures, both call targets and INI; extracted C++ logic tests for read masks, nesting/parent IDs, exact-once mock call, filtering, TLS isolation and buffer overflow; 12 Unicorn stack/flags/FX-data comparisons. Emulator FIP/FDP metadata is excluded because Unicorn does not restore it; full native IA32 test explicitly skipped (host cannot execute IA32 ELF). **No MSVC build or in-game Stage31 validation yet.**
- Limitations: non-atomic snapshots; normal-return TLS lifetime (abnormal unwind may leave unclosed scope); buffered tail can be lost on exit; local state-block candidate is not the resolved redirected E6960 result. No gameplay fix, forced bit, duplicate event or boolean result inference.
- **Next user action:** x86 Build.bat, save Build.log, install DLL/INI, archive old logs, natural COM-vs-COM run, allow >=2s final flush, upload Build.log + all fresh logs (especially Stage31, Stage29, ProducerCallTrace, QueueTrace, EventDispatchTrace).
- **Next assistant action:** require marker + installation line + complete BEGIN/END pairs; reject absence conclusions with drops/unclosed scopes. Correlate Stage29 set -> Stage15 bit17=1 -> Stage31 BEGIN -> same-episode kind3 type0 (if selected) -> END -> clear. Evaluate bit19/20/21, object+20 and special-state alternatives before calling missing kind3 a defect. Archive exact tested source/INI and runtime separately after evidence arrives.


## Stage31 — реальный alternate-path runtime подтверждён (2026-11-09)

- Неизменяемая ветка исходника `stage31-alternate-path-runtime-trace` @ `e905316353b844f276a40dff33516d48aa1e4d99`; независимая evidence-ветка [stage31-alternate-path-runtime-evidence](https://github.com/Hawky11/1/tree/stage31-alternate-path-runtime-evidence) @ `0a9ae75cd952f9ec6e2d8588a3c44ce0974cb466`. В `Stage31_Runtime/` находятся 21 оригинальный лог (blob SHA равны загруженным в `main` @ `6e3c9a8`), [полный анализ](https://github.com/Hawky11/1/blob/stage31-alternate-path-runtime-evidence/Stage31_Runtime/RUNTIME_ANALYSIS.md), [CSV](https://github.com/Hawky11/1/blob/stage31-alternate-path-runtime-evidence/Stage31_Runtime/correlation.csv).
- `STAGE31_ALTERNATE_PATH_BUILD=2026-11-09-r1`, установлен hook RVA `0xF6A8C→0x130A10` с 60 остановленными потоками. Всего 64 непрерывных записей seq1–64: 6 BEGIN, 6 END, 52 TYPE0_QUEUE; из них 6 вложенных совпадают по episode, side, player, thread, caller `0x156E4C`; остальные 46 без scope.
- Шесть последовательностей Stage29 set (3 P1, 3 P2) → Stage15 bit17=1 branchTaken=0 → Stage31 BEGIN → одно type0/payload0 → END → Stage29 clear соответствуют по QPC. Все полные snapshot-маски и прочитанные before/after. За время вызова P+3B84 изменился `00020000→00030000` (установка bit16), потом после END сбросился bit17 до `00010000`. Очередь при kind3 сохраняет `active=0`, несмотря на обновление storedType/Payload до 0. **Не утверждать, что это доказывает выполненную gameplay-команду**.
- В том же сеансе контрольный P1 bit17=0 → F6AD7 type1/payload0x1E → QueueTrace active0→1 → Stage21 classifier AL0; подтверждает выбор пути состоянием, а не фиксированной стороной P1/P2.
- В проверенных Stage31, Stage29, ProducerCall и QueueTrace явных drops/warnings/errors не найдено; seq Stage31 без дыр. Оговорки: неинструментированные потоки, snapshot-неатомарность, возможная потеря хвоста буфера. В upload `Build.log` отсутствует: MSVC transcript не подтверждён независимо, но загруженный в игре DLL с Stage31 hook выполнялся.
- **Дальше:** Stage32 статическая проверка meaning/fate type0/payload0 (особенно active=0), bit16 и downstream dispatch. Новые хуки только пассивные и целевые; сначала анализ исходного pinned AALib.exe из Library `/AALib.zip`, не просить EXE снова. Сохранять старые Stage-ветки.


## Stage32 — original type0/payload0 reactivation route verified (2026-11-09)

- Source-only [stage32-type0-dispatch-correlation](https://github.com/Hawky11/1/tree/stage32-type0-dispatch-correlation) @ `44b23d1634139fe15928b1ff50bbddda056f633d`, based on Stage31 evidence `0a9ae75`. [Static analysis](https://github.com/Hawky11/1/blob/stage32-type0-dispatch-correlation/Stage32_Analysis/STATIC_ANALYSIS.md), [verifier](https://github.com/Hawky11/1/blob/stage32-type0-dispatch-correlation/Stage32_Analysis/tools/verify_stage32.py) and [install guide](https://github.com/Hawky11/1/blob/stage32-type0-dispatch-correlation/Stage32_Overlay/README_STAGE32_RU.md).
- Native `F2DE0` only activates queue for type1/2 or type0/payload0x1E; type0/payload0 merely writes current tuple with active0. This is **not** a dropped event conclusion. `F2E20` checks current vs history; on the changed branch `EE480` uses `F2E50` to move the tuple to history, sets active1 at `EE589`, calls `EE200` at `EE5C6` (return `EE5CB`). Stage31 six episodes had type0/payload0 current and type1 nonzero history; would pass comparison if not overwritten. Existing aggregate `EVENT_DISPATCH` confirms both sides dispatch type0/payload0 with active1 in same run, but does not prove per-episode provenance.
- Original bit16 write at `E6C66` and clear at `E9E66` verified; Stage31 captures bit16 present by END but does not assign gameplay meaning. Read-only SHA-pinned verification against Library original AALib.exe passed 21 signatures and five CALL targets. No published full executable or full disassembly.
- Source changes are small: additional independent type0/payload0 detail mode `EnableStage32Type0DispatchDetail=1` via the existing working EventDispatchHook and `EVENT_DISPATCH_DETAIL` buffer; marker `STAGE32_TYPE0_DISPATCH_BUILD=2026-11-09-r1` in existing EventDispatchTrace.log. No new hook, altered gameplay flag, duplicated event, or bypass. Stage29 AITrace.cpp, Stage31 source and all historic branches unchanged. **MSVC build/runtime remains pending.**
- Next: x86 build, natural COM-vs-COM runtime and per-event QPC correlation against Stage31. Warn about grouped counters and temporal coincidences; no bug fix unless causality/gameplay failure is observed.


## Stage32 — real type0/payload0 dispatcher correlation confirmed (2026-11-09)

- Immutable evidence: [stage32-type0-dispatch-runtime-evidence](https://github.com/Hawky11/1/tree/stage32-type0-dispatch-runtime-evidence) commit `a6ed15cce970d92e690f349a8dd7a2cf2e5e5f97`; original Stage32 source-only `stage32-type0-dispatch-correlation` @ `44b23d1634139fe15928b1ff50bbddda056f633d`. All 21 uploaded runtime logs from `main` @ `a7a99cf` archived as unchanged Git blobs; [report](https://github.com/Hawky11/1/blob/stage32-type0-dispatch-runtime-evidence/Stage32_Runtime/RUNTIME_ANALYSIS.md), [21-pair CSV](https://github.com/Hawky11/1/blob/stage32-type0-dispatch-runtime-evidence/Stage32_Runtime/stage31_to_stage32_type0_dispatch.csv). The uploaded commit does not include `Build.log`; no independent MSVC transcript despite successfully running Stage32 instrumentation in game.
- Runtime: Stage32 marker and EE200 hook installed; Stage31 F6A8C→130A10 and Stage29 bit17 DR0/DR1 observer installed. Stage31 seq1..35 = 3 BEGIN/3 END/29 kind3, fully nested three complete episodes. EventDispatch detail seq1..131 = 77 type0/payload0 and 54 type1; Stage29 seq1..8 = 4 set/4 clear transitions. In key traces no explicit warnings/overflows/dropped, all sequences contiguous.
- For all **three** bit17=1 Stage15 alternative episodes, Stage31 END is followed by **same-thread, same-player type0/payload0 EE200** entry after 11,11,9 QPC ticks (dispatch #43/#60/#120). Dispatcher sees active=1/current=FFFF,FFFF/history=0,0. Stage29 bit17 is cleared **inside** each matched EE200 QPC interval; Stage31 P+3B84 changes 0x20000→0x30000 and the clear leaves 0x10000. Thus original type0 event is processed through EE200, not merely inserted with active=0.
- All 21 Stage31 type0/payload0 kind3 rows (3 scoped, 18 unscoped) have unique type0/payload0 EE200 detail matches in same TID/side within 7..230 QPC ticks. This is strong same-event evidence, not an explicit parent-ID in EE200. The fourth P2 bit17 set/clear episode occurs without any F6A68/Stage31 BEGIN, and clear lies inside an **unscoped** type0 dispatch #87; bit16 was absent in that clear shadow.
- Direct P1 bit17=0 control still invokes F6AD7 with type1/payload0x1E. This analysis **does not establish gameplay effect success**; do not force events or change bits. Next Stage33: statically inspect native type0/payload0 actions inside EE200 and downstream effect paths, using already-available Library `/AALib.zip`. Preserve all Stage31/32 source and evidence branches.


## Stage33 — native type0/payload0 consumer path proven statically (2026-11-09)

- Stage33 source-only branch [stage33-type0-consumer-state-trace](https://github.com/Hawky11/1/tree/stage33-type0-consumer-state-trace) @ `774ddbbf3a8fec3d6ed09afc31232ade3731c2da`, based on Stage32 completed runtime `a6ed15cce970d92e690f349a8dd7a2cf2e5e5f97` and current `main` `9723345`. [Static report](https://github.com/Hawky11/1/tree/stage33-type0-consumer-state-trace/Stage33_Analysis/STATIC_ANALYSIS.md), [reproducible 24+5+3 verifier](https://github.com/Hawky11/1/tree/stage33-type0-consumer-state-trace/Stage33_Analysis/tools/verify_stage33.py), [RU install guide](https://github.com/Hawky11/1/tree/stage33-type0-consumer-state-trace/Stage33_Overlay/README_STAGE33_RU.md).
- Static original AALib.exe pinned SHA verified; type0/payload0 at EE200 selects `158560` table index0 (VA 6521F80, bytes `0F 00 00 00 00 00 00 00`, fields +2/+3/+4 zero), calls `158EC0` with flag-id0 (table VA 652222C first byte 0x01, sets P+3B70 bit10), `F57D0` with descriptor0 (VA 4DDF188 `01 00 FF`) and `EC4F0` state/coordinator. Last branch clears selected object+8 bit10 at EE352. Do not call these a proven attack/move.
- `EC4F0` contains precise Stage29 bit17 clear: `ED193` calls `F2ED0(queue, type1,payload1E)` to check current **history**; `ED19F` clears P+3B84 bit17/18 with mask `FFF9FFFF` if AL0. Stage32's three matching dispatch entries had history=(0,0) and bit17 0x30000→0x10000 happened during the dispatcher calls at post-RVA ED1A9. Consistent with this native direct writer; not a Boolean success result.
- New Stage33 observer extends the **existing** EE200 EventDispatchHook only for type0/payload0 with read-only pre/post snapshots of P+3B70/3B84/3F64, controller P+3A08/0C/10, O pointer/O+08/O+184, P+3F54/3F74/3F78. Writes `STAGE33_EFFECT_DETAIL` with same seq/QPC as Stage32, 12-bit successful-read masks and raw values. No code patch site, no artificial event, original dispatcher once. Stage31 and Stage29 preserved.
- Original exe SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`; verification against Library `/AALib.zip` passed 24 exact signatures, 5 E8 calls, 3 data tables, wrong archive rejected. **No MSVC build or in-game Stage33 validation**. Next: Windows x86 Build.bat, install, natural COM-vs-COM and fresh detailed logs, then correlate net state changes vs native paths. Gameplay defect remains unproven; historical branches immutable.


## Stage33 — x86 Build and runtime confirmed; scope-limited type0 payload0 snapshots (2026-11-09)

- Source-only Stage33 `stage33-type0-consumer-state-trace` @ `774ddbbf3a8fec3d6ed09afc31232ade3731c2da` unchanged; archive [stage33-type0-consumer-runtime-evidence](https://github.com/Hawky11/1/tree/stage33-type0-consumer-runtime-evidence/Stage33_Runtime) @ `5eb0d96cd4a20cb713f4add43516857e6ba74389`, [full report](https://github.com/Hawky11/1/blob/stage33-type0-consumer-runtime-evidence/Stage33_Runtime/RUNTIME_ANALYSIS.md), [104 effect records CSV](https://github.com/Hawky11/1/blob/stage33-type0-consumer-runtime-evidence/Stage33_Runtime/stage33_effect_records.csv), [3-episode CSV](https://github.com/Hawky11/1/blob/stage33-type0-consumer-runtime-evidence/Stage33_Runtime/episode_correlation.csv); all 21 user's fresh logs plus Build.log copied using their original Git blob SHA.
- Windows `Build.bat` succeeded on x86 VS18/MSVC14.51, `dinput8.dll` size 249856. Existing nonfatal MSVC **C4754** `AICommandSiteTrace.cpp:785` (line782 referenced) is present. Stage33/Stage32 markers and native hooks installed; no explicit dropped/overflow in core traces.
- Stage32 detail seq1..176: 104 type0/payload0, 72 type1; Stage33 104 complete `STAGE33_EFFECT_DETAIL` records join exactly by seq, QPC and player, all preMask/postMask **0xFFF**, all 12 DWORD reads valid. 37/104 have net field changes, 67/104 unchanged at boundary. Field change counts indices 0..11: **[29,4,25,34,33,16,0,1,3,4,0,0]**. No net changes does not establish absence of transient writes/action.
- Stage31 seq1..54 = 3 BEGIN, 3 END, 48 queue observations, type0 payload distribution: 0:31,1:9,4:5,5:3. All **31/31** type0/payload0 queue returns map uniquely to per-event Stage32+Stage33 records (6..392 QPC tick gaps).
- Scoped episode1 P1 type0/payload0 maps to detail/effect **seq47**; episode3 P2 type0/payload0 maps to **seq161**; Stage31 END→EE200 12 and 9 ticks. Both show `P+3B84 30000→10000` with Stage29 bit17 clear **inside** EE200, `P+3F64 0x15/0x16→0`, `P+3A08 0x29→0`, `P+3A0C 0→1`, `P+3A10 0x10→0`. P2 also `O+184 0x1E→0` and O+08 mask changes.
- **Episode2 P2 is type0/payload1**, so no Stage32/Stage33 per-event record by current filter. Stage29 bit17 is nevertheless cleared at ED1A9 after Stage31 END and aggregated dispatch includes type0/payload1, but cannot establish one-to-one dispatch QPC or net effects. This is a diagnostic gap, **not** a lost event. Stage15 7 F6A68 checks: 3 alternate bit17=1, 4 direct bit17=0 type1/0x1E; separate control route.
- Stage34 task: statically differentiate native payload0/1/4/5 paths and extend existing passive EE200 detail filter to all four payloads, maintaining Stage29/31/32/33 context and no gameplay writes. Preserve history, never force bit17 or insert synthetic events. Original `AALib.exe` already in Library `/AALib.zip`.


## Stage34 — source-only variant consumer trace prepared (2026-11-09)

- Branch [stage34-type0-variant-state-trace](https://github.com/Hawky11/1/tree/stage34-type0-variant-state-trace) commit `803e3ae15f43370dc0fe7969b0ec6242320124d7`. Based on Stage33 tested source `774ddbb`; immutable Stage33 evidence `5eb0d96`. [Detailed native report](https://github.com/Hawky11/1/tree/stage34-type0-variant-state-trace/Stage34_Analysis/STATIC_ANALYSIS.md) · [Russian build/install guide](https://github.com/Hawky11/1/tree/stage34-type0-variant-state-trace/Stage34_Overlay/README_STAGE34_RU.md) · [exact verifier](https://github.com/Hawky11/1/tree/stage34-type0-variant-state-trace/Stage34_Analysis/tools/verify_stage34.py). Preserve all previous Stage branches.
- Original AALib.exe already stored in ChatGPT Library `/AALib.zip` with SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. The exact committed `verify_stage34.py` was run against the original EXE and **PASSED** SHA, PE32, 15 byte signatures, 6 E8 target checks, 4 payload table rows, 8 derived flag/descriptor reads. The actual Python script blob SHA matches the executed local file's `git hash-object` `b1261d857316c260625579e1de10b0ecbbd156f5`.
- EE200 native type0/payload0/1/4/5 table: via `158560` at preferred VA 6521F80 + payload×8, uses bytes at +2/+3/+4. Rows 0 and 4 select flagsId0/controllerId0/output0; rows 1 and 5 select flagsId1/controllerId1/output1. `158EC0` flag byte id0=01, id1=03; P+3B70 bit10 is set in both, bit5 is set only in id1. `F57D0` descriptors id0=01,00,FF and id1=01,01,FF select controller P+3A08 id0/id1 and P+3A0C bit0/bit1. Payload4/5 uniquely take `EE360→991C0` and write index0B of P+3A3C controller slots = **P+3A6C** (scaled input×120). No semantic interpretation of these slots/flags as a successful game move is proven.
- Stage34 overlay modifies **only** existing `EE200` EventDispatchHook capture predicate to include type0/payload1,4,5, and when selected logs 12 field before/after pairs plus one extra P+3A6C slot with explicit pre/post read mask. Uses existing worker ring for `EVENT_DISPATCH_DETAIL` + `STAGE34_EFFECT_DETAIL`, retains original `STAGE33_EFFECT_DETAIL` schema for payload0 and type1 detail behavior. Native original called once; no added hook, no inserted event and no game writes. Stage29 AITrace.cpp unchanged, Stage31–33 markers retained. INI key `EnableStage34Type0VariantsTrace=1` requires dispatch/Stage32/Stage33 enabled.
- **No Windows x86 MSVC build or Stage34 runtime yet.** User next: copy Stage34_Overlay AIQueueTrace.cpp and ArcanaAI.ini only, build x86, install, natural COM-vs-COM, upload Build.log + fresh logs. Analyst must explicitly correlate Stage31 type0/payload1 alternate episodes with Stage34 per-event EE200/QPC and compare state changes to static tables. Avoid guessing missing effects, keep original EXE Library reference and GitHub histories.


## Stage34 — real Windows x86 runtime: payload1 correlation and payload4 controller slot proven (2026-11-09)

- Archived **21 new runtime logs plus Build.log** from user uploads `main` @ `4e03cab8949a91bc3583ad1eadf7cb20d5c3a14a` and `1039963cfb2593000632352acb1dfb68e2641952`, reusing exact Git blob SHA in [stage34-type0-variant-runtime-evidence](https://github.com/Hawky11/1/tree/stage34-type0-variant-runtime-evidence) @ `ad801c2278f81cdb997d5773207efecb4211af65`; source-only `stage34-type0-variant-state-trace` @ `803e3ae15f43370dc0fe7969b0ec6242320124d7` untouched. [Complete analysis](https://github.com/Hawky11/1/blob/stage34-type0-variant-runtime-evidence/Stage34_Runtime/RUNTIME_ANALYSIS.md), [109 effects CSV](https://github.com/Hawky11/1/blob/stage34-type0-variant-runtime-evidence/Stage34_Runtime/stage34_all_effect_records.csv), [33 queue/dispatch CSV](https://github.com/Hawky11/1/blob/stage34-type0-variant-runtime-evidence/Stage34_Runtime/stage31_to_stage34_variant_pairs.csv), [variant summary](https://github.com/Hawky11/1/blob/stage34-type0-variant-runtime-evidence/Stage34_Runtime/stage34_variant_summary.csv).
- Original user x86 Build.bat succeeded at 23:46:47 on 2026-11-09: VS18/MSVC14.51, `dinput8.dll` **250880** bytes. Single nonfatal C4754 warning AICommandSiteTrace.cpp:785 (source line782 referenced). Stage34 marker r1, EE200 hook, Stage31 F6A8C observer, Stage29 bit17 watcher, F2DE0 hook all verified.
- Stage32 details seq1..168: **90 type0/payload0; 7 payload1; 12 payload4; 0 payload5; 59 type1**. Stage33 effects 90, Stage34 effects 19; **109/109 effects** exact join with same seq/QPC/side/TID/player/payload, all 12 pre/post DWORD reads succeed (FFF/FFF); extra slot P+3A6C read succeeds on all 19 variant rows (mask3). Stage31 seq1..35 = 1 BEGIN,1 END,33 queue rows distribution 26×payload0,3×payload1,4×payload4. All **33/33** uniquely match individual EE200+effect in 6..488 QPC ticks (conditional on same thread/side/payload; not a persistent parent ID).
- **Critical formerly missing payload1 alternate found:** Stage31 episode1 P2 type0/payload1 (queue seq12, END seq13) → Stage32/34 EE200 seq56 only 12 QPC ticks after END; Stage29 bit17 clear `0x30000→0x10000` at QPC542085815861 occurs *inside* that exact EE200 call. P+3F64 0x15→1, P+3A08 0x22→1, P+3A0C 0x10002→2, P+3B70 0x20460→0x20420, consistent with native id1/bit5 table.
- **Payload4 extra branch confirmed:** 12 individual type0/payload4 effect records, 8 change P+3A6C and all 8 end `0xF00`; values initially 0, E88 or E73. Static `EE360→991C0` writes this controller slot, but no downstream gameplay success inferred. Net field/slot changes: payload0 27/90, payload1 3/7, payload4 8/12. Stage29 4 bit17 transitions = 2 set/clear P2, one unscoped payload0 dispatch #13 and one scoped payload1 #56; no missing Stage31 parent fabricated.
- **Payload5 NOT observed in this run.** Do not claim complete runtime coverage for variant5; native payload5 route statically verified. Next Stage35 should focus on static read-only tracing of *downstream consumers of controller IDs/slots* to bridge state changes and actual action; optional future natural payload5 run, no forced injection, no gameplay modification. AALib.exe original already in ChatGPT Library `/AALib.zip`; never ask to reupload. Preserve all older source and evidence stage branches.


## Stage35 — native downstream slot/bitmap consumers and passive source-only observer (2026-11-10)

- Branch: [stage35-controller-consumer-trace](https://github.com/Hawky11/1/tree/stage35-controller-consumer-trace), based on the **tested Stage34 runtime evidence** branch `ad801c2278f81cdb997d5773207efecb4211af65`; no historical branches were rewritten. See `Stage35_Analysis/STATIC_ANALYSIS.md`, `Stage35_Analysis/tools/verify_stage35.py` and `Stage35_Overlay/README_STAGE35_RU.md`.
- Original verified SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` from persistent ChatGPT Library `/AALib.zip`: read-only PE32 static check PASS, 19 opcode signatures, 7 E8 CALL target checks; executable itself not committed.
- Direct native reads found beyond Stage34 `EE200` writer: `A1106→ACA90` per-side loop, `ACABE→F60C0` timer-slot consumption (`F611C→F7ED0` with dynamic `P+3A3C` subtraction, conditional `F7EB0` clear), `ACAC4→F5BD0` player/object flag derivation (`F5F4C` compare `P+3A6C`; `F5F55` OR `P+3B78` bit7), plus `EB7D9→F58E0` bitmap check on `P+3A0C/10`. Actual gameplay-action semantics remain unknown.
- Source-only Stage35 extends Stage34 via an `AIQueueTrace.cpp` include of `Stage35ConsumerTrace.inl`: opt-in bounded two CALL-site wrappers that snapshot 12 controller/player/object DWORDs with QPC, mask and side before/after native helper calls. Original helper arguments/return state preserved; no game writes beyond installing validated observer CALL redirections. Includes a dedicated worker-flushed log and dropped/overflow warnings.
- **No Windows MSVC x86 compilation, real installation, natural COM-vs-COM runtime trace, or behavioral result has yet been supplied for Stage35.** No attack/hit/movement or AI bug proven. Next verify user's Build.log/hook markers and QPC-correlated Stage35 TIMER/FLAGS records, preserve Stage29 AITrace.cpp, and avoid forced events/flag changes.

## Stage35 — Windows x86 build and native controller-consumer runtime evidence (2026-11-10)

- Input on main commits `6000b1411ca1becdc05c5105f40528f696756277` and `1a6f431f1ba426ecc5a45509ddcae2136972ada8`: exactly 23 newly added/changed files (22 logs + Build.log) compared with the previous documentation commit `5658e3a`. Two unchanged old logs (unfiltered Stage16/Stage26) were excluded from the fresh evidence archive. All 23 fresh blobs preserved byte-for-byte in [stage35-controller-consumer-runtime-evidence](https://github.com/Hawky11/1/tree/stage35-controller-consumer-runtime-evidence), initial archive commit `4044426a57b6bd8c0872d2afacd1ea7388cce8e8`. Complete findings: `Stage35_Runtime/RUNTIME_ANALYSIS.md` and reproducible CSVs.
- Build 2026-11-10 00:22:58 succeeded, Windows MSVC x86; `dinput8.dll` 257024 bytes. One pre-existing C4754 warning in `AICommandSiteTrace.cpp:785`, zero errors. Stage35 build marker and both native callsite installations verified.
- `STAGE35_CONSUMER` has **9280 consecutive seq** = 4640 TIMER and 4640 FLAGS, every record 12/12 pre/post reads good, **4640 matching adjacent pairs**, P1/P2 and TID intact, no dropped/overflow/error. Stage32–34 has 125 dispatches, 85 complete type0 effect matches (69 payload0, 13 payload1, 3 payload4; zero payload5), four Stage31 scoped episodes and 24/24 queue→dispatch links. Stage29 observes four bit17 cycles, clear inside four matching EE200 calls.
- **Proven downstream timer:** two naturally written/reset P1 payload4 slot11 0xF00 values, then a complete native TIMER chain of **33 decreases of exactly 0x78** with header `P+3A3C=0x78`. Per-row evidence preserved in `Stage35_Runtime/stage35_slot11_timer_transitions.csv`. Confirms original game's read/update behavior at F60C0, not a visible attack/guard/move.
- **Limit:** P+3B78 bit7 in post-FLAGS state is 1 for 32/33 positive-slot and 3989/4607 zero-slot pairs; it is not an exclusive marker of successful command or a particular F5F55 branch execution. Source-only Stage35 previous branch `stage35-controller-consumer-trace` @ `3ce68f5` remains unmodified. No AI gameplay bug proven. Next Stage36: read-only static trace into genuine action/command issuance boundary, then bounded passive runtime probe only if exact signature and semantics validated. Keep Stage29 AITrace.cpp unchanged.

## Stage36 — original game action selection / queue output mapping; source-only (2026-11-10)

- Branch [stage36-control-to-command-trace](https://github.com/Hawky11/1/tree/stage36-control-to-command-trace), based on *completed* Stage35 evidence `stage35-controller-consumer-runtime-evidence` @ `091fa2692b33b2e97d19406a330139796e4fa7af`. Stage35 source/evidence and Stage29–34 historical branches remain immutable. See `Stage36_Analysis/STATIC_ANALYSIS.md`, `Stage36_Analysis/tools/verify_stage36.py` and `Stage36_Overlay/README_STAGE36_RU.md`.
- Original exact `AALib.exe` from ChatGPT Library `/AALib.zip`, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Read-only independent opcode/CALL verification PASS for **22 signatures and 8 direct calls**.
- Static dataflow: `EB718→F58E0(P+3A08,0x1F)` / `EB7D9→F58E0(P+3A08,0x1E)` return bitmap AL used by native branch checks; in a separate command-selection flow `F79CF→F3280` result enters `F79E4→F4360` (five cdecl args, returned EAX candidate enters ESI), then additional branches may revise choice before the already traced native `F7C5D→F2DE0(queue=P+3A2C,type=1,payload=ESI)` command-event proposal. The existing `QUEUE_EVENT_DETAIL` has QPC and callerRVA F7C62; `FINAL_COMMAND` captures payload and flags.
- New Stage36 source-only overlay inherits Stage35 and adds opt-in **three validated, all-or-none, original-function-once** x86 CALL wrappers on EB718, EB7D9 and F79E4, logging raw predicates AL, filter EAX, candidate output pointers, 10 raw before/after DWORDs and QPC for exact P1/P2 with masks and buffered worker. No game input or state writes beyond validated diagnostic CALL patches.
- **Neither new Stage36 DLL build nor real Stage36 in-game runtime is verified.** No animation/movement/hit or AI bug has been confirmed. Next x86 build/install/natural COM-vs-COM and inspect logs; keep Stage29 AITrace.cpp, avoid synthetic type1/0x1E and payload5; never conflate a candidate with final queued payload.

## Stage36 r1 Windows runtime failed closed; Stage36 r2 Stage10 hook compatibility (2026-11-10)

- User added Stage36 r1 artifacts to GitHub main commits `1cdbbee9ca2b5df500eb581e7db3b6eb83c0aca0`, `aeee2f8e4c27ffe7969ce8fcc67257f437cf8044`. Compared to previous main Stage36 docs, exactly **24 newly changed/uploaded files** (23 .log + Build.log), archived by exact Git blob SHA under `Stage36_FailedRuntime/` in new branch `stage36-control-to-command-r2-stage10-compat`, first archive commit `b43c09ee4965e09fdd82fefa570d2176c0114be2`. Two unchanged old logs not counted as new.
- Windows x86 `Build.bat` at 2026-11-10 00:55:02 **SUCCEEDED**: 264192-byte DLL, zero build errors, only previous C4754 warning. But Stage36 r1 logger recorded error `callsite, caller context, native prologue or CALL target mismatch; no patches` at 00:55:43. No Stage36 predicates/candidates or `STAGE36_RESULT` observed. ***Do not treat Stage36 r1 as a verified runtime stage.***
- Cause confirmed by previous observer: Stage10 ParentFlow installed a **six-byte F4360 native inline hook** at 00:55:43.096, preceding Stage36 validation and replacing the original prologue that r1 expected. Native original SHA-256 checked `AALib.exe` and exact r1 CALL signatures all match; failure is Stage36's assumption about coexistence with previous Stage10 hook, not a wrong native signature. Stage10 logged 66 F4360 results.
- Despite failed Stage36 observer, old monitors worked: **13172 Stage35 records** (6586 TIMER, 6586 FLAGS, complete FFF/FFF masks), 202 slot11 TIMER changes, 199 EventDispatch (104 payload0, 13 payload1, 6 payload4, **6 payload5**, 70 type1), 129 valid type0 effect records, 49 Stage31 events (39 type0 queue), ten Stage29 bit17 transitions. For the first time natural **payload5** on P2 seq98–100 and P1 seq102–104 wrote **slot11 `0→F00`, `F00→F00`, `E88→F00`** for both sides; see `Stage36_FailedRuntime/type0_payload5_first_natural_evidence.csv`.
- Stage10 F4360 caller `F79E9` recorded 66 filtered EAX results. Exactly 19 unique temporally adjacent `QUEUE_EVENT_DETAIL callerRVA=F7C62,type1` have identical raw EAX and payload at 9..69 QPC tick separation. Remaining 47 lack such unique adjacency within 10000 ticks (46 F4360 sentinel `FFFFFFFF`). No proven on-screen animation or hit. Full per-candidate CSV and [r1 report](https://github.com/Hawky11/1/blob/stage36-control-to-command-r2-stage10-compat/Stage36_FailedRuntime/RUNTIME_ANALYSIS.md) saved.
- New source-only Stage36 **r2** branch `stage36-control-to-command-r2-stage10-compat`: only two changed user files `src/Stage36ControlPredicate.inl` (accept only a validated existing six-byte JMP to executable code within own dinput8.dll, all callsites still exact) and `src/AIQueueTrace.cpp` (build marker r2). Diagnostic `STAGE36_R2_ENTRY` and `STAGE36_R2_INSTALLED` added, unknown/foreign patch fails closed. Stage35 original .inl and INI byte-identical, Stage29 AITrace.cpp untouched. **r2 Windows x86 compile and live game runtime not yet verified**. Await next Build.log and new Stage36/Stage35/ParentFlow/Queue/Dispatch logs. Preserve r1 branch and all past stages.

## Stage36 r2 — native filter→type1 queue→EE200 runtime fully verified (2026-11-10)

- User uploaded fresh Stage36 r2 run to GitHub main commit `fc6e14016371c39809f1d803fa84d3116ce9038c`. Relative to previous handoff main `e40f028`: **23 changed/new artifacts** (22 .log + Build.log; exact original SHA blob copies) archived as `Stage36_R2_Runtime/` under [stage36-control-to-command-r2-runtime-evidence](https://github.com/Hawky11/1/tree/stage36-control-to-command-r2-runtime-evidence), archive commit `8a657de36b66ab8de5aad9bbee4c0784c6e5e0f2`. Three unchanged old root log blobs excluded from fresh count. Full [report](https://github.com/Hawky11/1/blob/stage36-control-to-command-r2-runtime-evidence/Stage36_R2_Runtime/RUNTIME_ANALYSIS.md), [412-row raw CSV](https://github.com/Hawky11/1/blob/stage36-control-to-command-r2-runtime-evidence/Stage36_R2_Runtime/stage36_all_412_candidate_records.csv), [7-row correlated native pipeline CSV](https://github.com/Hawky11/1/blob/stage36-control-to-command-r2-runtime-evidence/Stage36_R2_Runtime/seven_candidate_to_queue_to_dispatch_matches.csv).
- `Build.bat` 2026-11-10 01:17:20 Windows MSVC x86 **BUILD SUCCEEDED**: 266240-byte DLL, no compiler/link errors, single previous C4754 warning. Native Stage36 r2 explicitly logged `STAGE36_R2_ENTRY F4360=0x006A4360 bytes=E98B37365090 mode=STAGE10_OWN_DLL detourShape=1 target=0x50A07AF0` and **`STAGE36_R2_INSTALLED ... 3/3 sites`**. Stage10 six-byte F4360 hook collision solved without abandoning strict ownership verification. Source-only r2 branch `stage36-control-to-command-r2-stage10-compat` @ `4918328` unchanged.
- **412 Stage36 results, seq1–412 no gaps, 293 P1 / 119 P2, all `CANDIDATE`**, read masks 3FF/3FF, args1F, outputs3>3, QPC before<after, no positive dropped/overflow. **405 EAX=FFFFFFFF sentinel**, **7 accepted return values 42,43,10,66,19,8,41**, all 412 EAX==incoming third argument and no net field or out-pointer DWORD changes. There were **0 runtime INPUT_1E/INPUT_1F** records (installed callsites not naturally seen or not accepted by filters), so their branch results remain unverified.
- **Seven strong native command pipeline matches:** 7/7 non-sentinel Stage36 F4360 returns match a unique nested Stage10 F4360 observation, unique subsequent `QUEUE_EVENT_DETAIL callerRVA=F7C62,type1` 11–17 QPC ticks after, and unique subsequent `EE200 EVENT_DISPATCH_DETAIL type1` 14–50 QPC ticks after queue, each same player/TID/payload. Stage10 F4360 now sees `callerRVA=FFFFFFFF` because Stage36 calls it from own DLL wrapper; this is instrument context, original caller `F79E9` preserved by Stage36. Stage10 sampled 27 FFFFFFFF failures but recorded all 7 successes (34 F4360 records total), consistent with 1/16 failure sampling. **No on-screen attack, animation, hit, actual gameplay success or bug proven**.
- Old Stage35 has 8935 complete single records (4467 complete TIMER→FLAGS pairs + final dangling P2 TIMER), masks full and 197 slot11 changes; 137 EE200 dispatches (95 type0 = 70 payload0, 12 payload1, 10 payload4, 3 payload5; 42 type1), **95/95 effect snapshots** match dispatch. 37 FinalCommand/queue F7C62 type1 proposal records; only 7 uniquely linked to non-sentinel Stage36 filter returns. Stage31 29 total, Stage29 four bit17 transitions. Core logs have no explicit dropped/overflow errors. Keep all historical branches intact.
- Stage36 r2 now **completed as live native candidate/command-pipeline diagnostic**. Next Stage37 identify actual action/animation/object execution boundary downstream of EE200 type1, or an independently verifiable predicate path. Reuse existing Library AALib.exe and Stage29 AITrace.cpp unchanged; do not infer AI malfunction from sentinels or absent sampled predicates.

## Stage37 — original type1 EE200 controller/object write path mapped, source-only (2026-11-10)

- New branch [stage37-type1-controller-object-commit-trace](https://github.com/Hawky11/1/tree/stage37-type1-controller-object-commit-trace), based on fully verified Stage36 r2 runtime evidence `stage36-control-to-command-r2-runtime-evidence` @ `b3c879c610fb88c15de4519a4fdd6a13f7bc2948`. Previous Stage36 source-only, Stage35 and all earlier historical stages remain unchanged. Full `Stage37_Analysis/STATIC_ANALYSIS.md`, `Stage37_Analysis/tools/verify_stage37.py`, `Stage37_Overlay/README_STAGE37_RU.md`.
- **Original EXE re-used from Library**: `AALib.zip` SHA-pinned EXE `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, PE32/i386, 175,556,608 bytes. Full original PE .text disassembly and independent **22 exact signatures + 7 E8 CALL target checks PASS**.
- Confirmed exact downstream static path for **type1 EE200**: `EE273→12EAC0` produces table description for native payload; `EE27F` writes `[P+4]+0x184`, `EE2C0` writes `P+3F64`, `EE2C6→158EC0` derives bitflags, `EE306→F57D0` maps command-controlled input bits into `P+3A08/0C/10`, `EE315→EC4F0` resets native player/object lifecycle/flags including `[P+4]+08/0C/10/278/27C`. This demonstrates static native *controller/object state commit* downstream of queue dispatch, **not an on-screen attack, animation, hit or AI failure**.
- Created passive Stage37 overlay on existing tested Stage36 code: only replacement `Stage37_Overlay/src/AIQueueTrace.cpp` plus `Stage37_Overlay/ArcanaAI.ini`. New opt-in `EnableStage37Type1CommitTrace=1`, requires EventDispatch and Detail options 1. **No extra game CALL patch**, no extra thread, no extra game state write: the original EE200 hook is called once, and for precise P1/P2/type1, records 30 raw controller/player/object DWORD fields before/after, independent read masks `0x3FFFFFFF` and `changedMask`; uses same bounded 1024-record event detail ring and worker-only `ArcanaAI_Stage37Type1CommitTrace.log` output. `dispatchSeq`, P/TID and QPC are exactly identical to existing `EVENT_DISPATCH_DETAIL` record, enabling unambiguous one-to-one join.
- Old Stage35ConsumerTrace.inl / Stage36ControlPredicate.inl unchanged, Stage29 AITrace.cpp unchanged. **New Stage37 Windows x86 compile, native hook persistence and runtime effect records not yet tested**. User to copy two overlay files, compile existing x86 Build.bat, archive old logs, natural COM-vs-COM, upload fresh Build.log and Stage37/Stage36/EE200/queue logs. Next analysis should establish actual state writes for seven confirmed candidate→type1 commands, then seek a truly downstream action/animation executor if needed.


## Stage37 runtime verified — 2026-11-10

Upload: `0a87d4c5e93b5115e9288cbde78acf491f18bf4f` (07:10:17 Moscow), evidence branch: [stage37-type1-runtime-evidence](https://github.com/Hawky11/1/tree/stage37-type1-runtime-evidence). Full report: [Stage37_Runtime/RUNTIME_ANALYSIS.md](https://github.com/Hawky11/1/blob/stage37-type1-runtime-evidence/Stage37_Runtime/RUNTIME_ANALYSIS.md).

- Windows x86 build at 07:07:57: BUILD SUCCEEDED, DLL 268288 bytes; only old C4754 warning. Stage37 r1 marker and ENABLED present, EE200 installed; Stage36 r2 3/3 installed with STAGE10_OWN_DLL.
- 173 consecutive EventDispatch records, 52 type1 and 121 type0. All 52 type1 have exact Stage37 join on dispatchSeq, side, player, tid, payload and both QPC stamps. P1=22/P2=30. All 30 fields readable before/after (3FFFFFFF), all changed masks recomputed correctly; every row changes state.
- O+278 increments by one in 52/52; P+3A08 and P+3A0C change 51/52, P+3A10 23/52, P+3F64 51/52, P+3B70 49/52. O+184 changes only 3/52 (0->1E); do NOT label it animation ID.
- 478 consecutive Stage36 CANDIDATE records: 472 FFFFFFFF, 6 positive IDs 6,12,82,20,42,89. All six uniquely match nested ParentFlow F4360, subsequent same P/TID/payload F7C62 queue and EE200/Stage37. INPUT_1E/1F remain unobserved. ID42/dispatch164 changes only P+3F64 and O+278; unchanged bitmap is not a lost command.
- Stage35 10812 consecutive full-mask rows, 5406 TIMER/FLAGS pairs. No explicit runtime ERROR/WARNING/dropped/overflow in successfully retrieved logs. ProducerCallTrace could not be retrieved (transport closed); its exact blob is archived unchanged, do not claim it was empty or checked. All other 24 uploaded files read, nonempty and newline terminated; no malformed Stage37 rows or sequence gaps in checked streams. Tail flush completeness cannot be proved solely from logs.
- Next Stage38: trace readers of changed object lifecycle fields (especially O+278 and player P+3F64) to a precise native per-frame action/animation consumer using original SHA-pinned AALib.exe already in Library; no need to request EXE. This proves controller/object state changes, not visible attacks/hits or an AI bug. No new DLL needed for this completed review. Preserve Stage37 source and all older branches, keep Stage29 AITrace.cpp.


## Current handoff — Stage38 native callback window source ready (2026-11-10)

New source: [stage38-native-callback-window](https://github.com/Hawky11/1/tree/stage38-native-callback-window); [install instructions](https://github.com/Hawky11/1/blob/stage38-native-callback-window/Stage38_Overlay/README_STAGE38_RU.md); [static analysis and verification](https://github.com/Hawky11/1/blob/stage38-native-callback-window/Stage38_Analysis/STATIC_ANALYSIS.md). **Windows x86 compilation/runtime NOT yet performed.** Preserve Stage37 evidence `d9808dbf483524c720f7085c754fcddd74824d11` and all historical branches.

- Original SHA-pinned AALib.exe reused, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Do not request EXE again. P+3F64 copies to P+3A14 and other-player P+3A18; O+278 behaves as a lifecycle generation, neither is proven animation ID.
- New direct native path: EE5E0 calls EE480/EE200, then type1 EE63B->12EEE0 selects callback through F7E70 into P+3ED0 and argument P+3ED4. EE72A caches callback in EDI, bit7 of P+3B74 may suppress it, EE7AE->156C00 prepares context, **EE7BE CALL EDI actually invokes it**, then EE7C7->FC60 may enqueue later events.
- Two passive five-byte CALL wrappers at EE7AE and EE7C7; snapshots AFTER native context preparation and BEFORE FC60. Original guarded indirect CALL remains untouched. Exact P1/P2, TLS frame/context pairs, actual callback/after, QPC, 30 fields and read/change masks, 4096-record ring with dropped/overflow/mismatch warnings. Includes zero-target controls and repeated callbacks. No game-state writes or synthetic inputs. Do not claim a rendered hit/animation from callback execution alone.
- Verification PASS: SHA/PE32, 11 exact signature ranges + 5 CALL targets + FF D7; 6 x86 wrapper ABI executions under Unicorn (FIP excluded because emulator FXRSTOR does not restore it); actual C++ pairing/snapshot helper tests with stubbed OS APIs (nested, overflow, masks, mismatches, drops). **Not MSVC build or live runtime validation.**
- USER: replace `src/AIQueueTrace.cpp`, add `src/Stage38CallbackWindow.inl`, replace `ArcanaAI.ini` from Stage38_Overlay. Keep Stage35ConsumerTrace.inl, Stage36ControlPredicate.inl r2 and Stage29 AITrace.cpp. Existing x86 Build.bat, install DLL/INI, archive old logs, natural COM-vs-COM, allow >=2s flush; upload Build.log and fresh Stage38/37/EventDispatch/36/Queue/ParentFlow/FinalCommand/35 logs.
- NEXT ANALYST: require BUILD SUCCEEDED, STAGE38_CALLBACK_WINDOW_BUILD=2026-11-10-r1 and STAGE38_INSTALLED 2/2; validate 30 fields/full masks, changed masks, per-window QPC, exact callback==callbackAfter and warning counters. Correlate Stage37 post object/generation with Stage38 pre f0/f28 and f1 type/payload, player/TID/QPC; require unique associations and inspect intervening dispatches. Record repeated same-generation callbacks and zero-target controls separately. A completed window is not a new command. Follow actual callback targets toward native outputs only after runtime is available.

## 2026-11-10 — Stage38 runtime verified: guarded native callbacks, 6 candidate-to-callback chains

- Source-only commit `4963dcdccfaaa190d5ce2438510ff24c0ba2a51d` and `stage38-native-callback-window` preserved. Original user runtime upload `def14ecc8a3354b96a2564d0280a088684a68ae6` (07:28–07:30 logs). Evidence archived without modifying 26 original log/build blobs in [stage38-native-callback-runtime-evidence](https://github.com/Hawky11/1/tree/stage38-native-callback-runtime-evidence), commit `12dd482bdb3070131016f9fc234ba9574580094c`; [complete analysis](https://github.com/Hawky11/1/blob/stage38-native-callback-runtime-evidence/Stage38_Runtime/RUNTIME_ANALYSIS.md).
- x86 build succeeded, 278528-byte DLL, only historic C4754 warning. Runtime Stage38 marker `2026-11-10-r1`, both CALL wrappers installed (`STAGE38_INSTALLED 2/2`). 4916 contiguous fully readable 30-field callback windows; no QPC inversion, broken mask, wrong callback/after, or explicit dropped/overflow/pairMismatch warning. 4195 nonzero EDI guarded calls (P1 2119, P2 2076); 721 zero EDI controls (P1 250, P2 471); 367 windows changed some selected fields, 4549 net unchanged. All records newline terminated.
- Stage37 56/56 exact type1 join with 176 EventDispatch events (120 type0,56 type1). 55/56 next Stage38 windows match Stage37 post object+generation, packed type1/payload, side/TID and chronology before next same-side dispatch. Exception P2 dispatch112 repeated same-payload dispatch113, and only after 113 Stage38 seq2169 matches the incremented generation. Avoid treating 112 as proven lost work.
- Stage36 r2 367 candidates (361 sentinel; 6 positives payload IDs 33,6,12,92,10,37); 6/6 positive results uniquely match F7C62 queue -> type1 dispatch -> Stage37 -> nonzero Stage38 callback. Example callback VA 0x0092EB10 (RVA 12EB10). Stage35 10592 records, 2648 complete TIMER/FLAGS pairs for each player, all FFF masks; ProducerCall 9,001,773 bytes read, no explicit warnings. No Stage36 INPUT_1E or INPUT_1F samples.
- **Scope:** proves naturally selected type1 candidate -> queue -> controller/object commit -> actual guarded native callback invocation/return for six positive candidates; not visual animation, actual hit, damage, or improvement of AI. Next Stage39 research must find first definite downstream action/animation/effect output from the real callback RVA targets, using already-held SHA-pinned EXE, then plan minimal passive instrumentation. Historical branches and Stage29 AITrace.cpp unchanged.

## 2026-11-10 — Stage39 static native action-output analysis and passive source ready

- Parent verified evidence: [Stage38 runtime](https://github.com/Hawky11/1/tree/stage38-native-callback-runtime-evidence), commit `12dd482bdb3070131016f9fc234ba9574580094c`. New source-only branch [stage39-native-action-output-trace](https://github.com/Hawky11/1/tree/stage39-native-action-output-trace) at `0fa652958df7f6ed4f4a6055e90df40c1b533d56`; [full disassembly and field semantics](https://github.com/Hawky11/1/blob/stage39-native-action-output-trace/Stage39_Analysis/STATIC_ANALYSIS.md), [installation](https://github.com/Hawky11/1/blob/stage39-native-action-output-trace/Stage39_Overlay/README_STAGE39_RU.md). No Windows Stage39 DLL compilation, installation, or runtime claimed.
- Original `AALib.exe` SHA256 matched existing Library `/AALib.zip`: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, 175556608 bytes, PE32/i386. **10 exact original opcode signatures + 8 direct E8 CALL target validations PASS**. Positive Stage36 callback set from Stage38: RVA 12EB10, 123AF0, 1DA490; RVA 123AF0 / 1DA490 both call 1BD6E0, which writes primary/alternate player action-state through 1B8050 / 1B80F0. RVA 12EB10 dispatches by a jump table and reaches output writers E5BF0 (P+3F4C, possibly P+3B7C bit24) and next-callback setter F7E70 via 131CA0/131D00.
- Stage39 does not patch a single additional native instruction: it reuses unchanged Stage38 wrapped guarded CALL context. Modified Stage38CallbackWindow.inl includes new Stage39ActionOutput.inl, samples 30 new native field values pre/post only for three real callback targets, stores samples in existing pending/ring, logs exact Stage38 sequence and QPC on existing worker. Actor P+43B4 derived fields must be regarded as invalid when pointer absent or different. Masked net writes are not proof of completed animation/hit/damage.
- Next user task: replace Stage38CallbackWindow.inl and add Stage39ActionOutput.inl, keep working Stage38 AIQueueTrace.cpp, Stage35/36 inls, Stage29 AITrace.cpp and ArcanaAI.ini unchanged, x86 Build.bat and natural COM-vs-COM; upload Stage39/38/37/36/EventDispatch and related logs. Preserve prior branches. No original EXE re-upload required.

## 2026-11-10 — Stage39 Windows x86 runtime verified, next Stage40 static output consumer found

- New user upload `fe8b2207e73af6dc5f6e89cba453369afe0e7828`; source [stage39-native-action-output-trace](https://github.com/Hawky11/1/tree/stage39-native-action-output-trace) at `0fa6529`. All 27 newly uploaded original Git blobs archived verbatim to [stage39-native-action-runtime-evidence](https://github.com/Hawky11/1/tree/stage39-native-action-runtime-evidence) at `ae6074d0aa9a06203dff9fb15a1eb4bee88755e6`, plus [full report](https://github.com/Hawky11/1/blob/stage39-native-action-runtime-evidence/Stage39_Runtime/RUNTIME_ANALYSIS.md). Preserve older branches.
- MSVC x86 build 2026-11-10 07:57:55 `BUILD SUCCEEDED`, 285184-byte DLL, only historical C4754 warning. Runtime Stage39 r1 marker and Stage38 hook 2/2 present. **51 Stage39 windows match 51/51 exact Stage38 rows**, 5272 Stage38 total (all sequential); 39 callback RVA12EB10 / 4 RVA123AF0 / 8 RVA1DA490. Every Stage39 window sets `P+3F4C` from FFFFFFFF to non-sentinel and updates `P+3ED0/3ED4`; 12 update P+3EE4 primary action state. No invalid masks or warning/drop/overflow. Actor P+43B4 absent for 35 rows; masks intentionally omit unreadable actor-relative fields.
- Stage36: 348 consecutive `CANDIDATE` calls, 336 sentinel plus 12 non-sentinel EAX. **10 have out4=1** and uniquely join native type1 queue, EE200, Stage37 and Stage38. Eight hit Stage39 target callbacks, two target **RVA122800** outside Stage39 filter (not lost). **2 EAX=39 have out4=0**, P1 seq254 and P2 seq306; ParentFlow confirms exact values but no same-interval F7C62 type1 queue/dispatch. They are not evidence of discarded actions or errors. Stage37 57/57 exact EventDispatch type1; Stage35 11176 complete records, 2794 TIMER/FLAGS pairs per side; old observers healthy.
- New original binary consumer: single callsite `AD72F -> E8480` in native per-player loop; it reads `P+3F4C`, calls action helpers, in successful branch writes **actor+280 = ID** and stores player object pointer in actor+2D0; then resets `P+3F4C` to FFFFFFFF. Static only: no Stage40 code or runtime claims. Stage40 should observe this consumer passively and then identify actual animation/hit output. Old AITrace.cpp and experimental branches untouched.


## 2026-11-10 — Stage40: passive E8480 request consumer source ready (Windows x86/runtime pending)

- Branch [stage40-action-request-consumer-trace](https://github.com/Hawky11/1/tree/stage40-action-request-consumer-trace), source HEAD [43821a5](https://github.com/Hawky11/1/commit/43821a5c5575a590dfd959e838247ca25f8bf9ea). Based on main 84e917a; Stage39 evidence ae6074d and all historical branches preserved. [Install notes](https://github.com/Hawky11/1/blob/stage40-action-request-consumer-trace/Stage40_Overlay/README_STAGE40_RU.md) and [static report](https://github.com/Hawky11/1/blob/stage40-action-request-consumer-trace/Stage40_Analysis/STATIC_ANALYSIS.md).
- SHA-pinned Library AALib.exe (SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba; size 175556608) directly checked: twelve exact original instruction signatures and three CALL targets passed, AD72F->E8480, E84E3->125920, E8505->1357F0. E8480 checks P+3F4C, 125920 selector, passes valid raw ID into actual actor+280, stores P+4 into actor+2D0, then clears P+3F4C. This path is STATIC until new runtime evidence arrives.
- Stage40 overlay: new Stage40RequestConsumer.inl, integrated Stage38CallbackWindow.inl, unchanged Stage39ActionOutput.inl, verifier, installation notes. Reuses unchanged Stage38 AIQueueTrace.cpp and Stage29 AITrace.cpp, Stage35/36 inls and existing ArcanaAI.ini. No new worker/INI key. Three passive CALL wrappers preserve registers/flags/FPU; original helper stack arguments replayed in correct order (125920: two; 1357F0: four). Sentinel entry tail-jumps without tracing. Strict signatures and guarded patch installation.
- New log ArcanaAI_Stage40ActionConsumerTrace.log holds per-player P+3F4C before/after, native selector, exact native actor returned by 1357F0 and actor+280/+2D0 before/after, flags, QPC, independent read masks and health counters. Correlate with Stage39 by player/QPC chronology rather than repeated ID alone; no synthesized inputs/game-state writes.
- Windows x86 MSVC compile/link, live hook installation and Stage40 runtime have NOT been verified. Install new .inls according to README only for a fresh x86 BUILD SUCCEEDED, run natural COM-vs-COM, upload fresh Build.log and Stage40/39/38/37/36 and queue/dispatcher logs. Check Stage38 2/2 and Stage40 3/3 and dropped/overflow/mismatch. Actor+280 is native state binding, not proven animation, hit or AI improvement.


## 2026-11-10 — Stage40 r1 x86 build passed, hook installation failed closed; Stage40 r2 ASLR fix ready

- User runtime upload in main [4faf4e0](https://github.com/Hawky11/1/commit/4faf4e0774a9b8b9a8e2990048b53b2120a4328a); all 28 original build/runtime log Git blobs retained by [stage40-r1-install-failure-evidence](https://github.com/Hawky11/1/tree/stage40-r1-install-failure-evidence), report [Stage40_R1_Runtime/INSTALL_FAILURE_ANALYSIS.md](https://github.com/Hawky11/1/blob/stage40-r1-install-failure-evidence/Stage40_R1_Runtime/INSTALL_FAILURE_ANALYSIS.md), evidence report commit 64afd5b. Stage40 r1 source-only branch stays unchanged at 43821a5. Previous Stage39 and earlier evidence preserved.
- Build.log at 08:34:12 Windows x86 BUILD SUCCEEDED; DLL 687104 bytes, only old C4754. Actual Stage40 runtime log at 08:36:20 has a SINGLE ERROR: Stage40 signatures/call targets mismatch; no patches. NO Stage40 installed marker or consumer windows. This is successful fail-closed guard action, NOT a verified E8480 runtime observation. Stage39 65 output windows, Stage37 69 type1 effects, Stage36 r2 900 candidate observations (883 sentinel and 17 non-sentinel, all 17 native forwarding flag=1). Stage38 and Stage35 large full-file integrity not verified through text connector; original uploaded bytes remain in evidence branch.
- Root cause found against exact original Library AALib.exe SHA 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba. PE32 base 00400000, actual runtime base 00800000. Original 125920 prologue contains absolute DWORD VA 05CF7260, with type-3 IMAGE_REL_BASED_HIGHLOW relocation at RVA 125925. Windows converts it to 060F7260 at runtime. R1 bytewise comparison of unrelocated prologue rejected correct loaded instructions, while all original CALL target bytes match.
- Fix branch [stage40-action-consumer-r2-relocation-fix](https://github.com/Hawky11/1/tree/stage40-action-consumer-r2-relocation-fix), [revised inl](https://github.com/Hawky11/1/blob/stage40-action-consumer-r2-relocation-fix/Stage40_Overlay/src/Stage40RequestConsumer.inl). R2 validates opcode bytes, explicit relocated absolute operand, other two prologues and all three relative CALLs; includes per-check failure message and marker STAGE40_ACTION_CONSUMER_BUILD=2026-11-10-r2. The static test of PE relocation under imagebase 00400000/00800000/00C00000 and three CALL targets PASSED.
- **Next user:** replace only src/Stage40RequestConsumer.inl with r2 overlay, keep working other sources/INI/Build.bat. Run existing x86 build, install only after BUILD SUCCEEDED, natural COM-vs-COM, upload new Stage40/39/38/37/36 + Build.log. Stage40 r2 Windows build/hook install/actor+280 transfer NOT YET VERIFIED. No game-state injection and no reason to request original EXE.


## 2026-11-10 — Stage40 r2 native request consumer RUNTIME VERIFIED; Stage41 direct object source ready

- Fresh user upload [e4d29fa](https://github.com/Hawky11/1/commit/e4d29fa639c74ab3e5345cf91fec729d43e43f78), fresh x86 Build.log and runtime blobs preserved unchanged at [stage40-r2-runtime-evidence](https://github.com/Hawky11/1/tree/stage40-r2-runtime-evidence) with [full analysis](https://github.com/Hawky11/1/blob/stage40-r2-runtime-evidence/Stage40_R2_Runtime/RUNTIME_ANALYSIS.md), and [56 exact chronological joins](https://github.com/Hawky11/1/blob/stage40-r2-runtime-evidence/Stage40_R2_Runtime/stage39_to_stage40_next_consumer.csv). Source branch stage40-action-consumer-r2-relocation-fix untouched, all historical branches preserved.
- Real Windows x86 BUILD SUCCEEDED at 08:47:58, DLL 688128 bytes (only old C4754 warning). Stage40 runtime marker STAGE40_ACTION_CONSUMER_BUILD=2026-11-10-r2 and STAGE40_INSTALLED 3/3 installed; 489 continuous and valid Stage40 windows (P1 271 / P2 218, 95 unique raw request IDs), 489 full FF player masks, no malformed QPC and no explicit dropped/overflow/mismatch errors. 489/489 raw P+3F4C before non-sentinel and after FFFFFFFF; 125920 selector results all FFFFFFFF, no actor helper calls, actor+280 never observed. This DOES prove E8480 consumes/clears requests but not that it drops attacks.
- Stage39 57 windows, 56/57 join the next same-player Stage40 before any later Stage39 on that side with identical ID, 56 unique next consumers and QPC lag 34..1292 ticks. Last Stage39 P2 stage38Seq6020 ID0303 occurs after last Stage40 window and is simply unobserved, not proven lost. EventDispatch 188 details (129 type0/59 type1) exactly joins Stage37 59/59; Stage36 406 CANDIDATE results (395 sentinel, 11 non-sentinel), Queue 57 type1 detail records. Stage38/Stage35 large logs could not be fully text-read by GitHub connector and full integrity was NOT asserted; raw blobs preserved in evidence branch.
- SHA-pinned original Library AALib.exe re-disassembled (SHA 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba). Actual native 125920 static table has exactly 40 six-byte entries, with modes 8:3, 9:28 and 29:9, terminated FE. Stage40 runtime had P+3978/P+3F20 10 (P1) or 23 (P2) in every window; lookup thus has no matching mode and correctly returns FFFFFFFF. The optional actor+280 branch is NOT the core generic action path.
- Crucial earlier E8480 CALL at E84AB->1E9F00 (ECX=[P+4], args request/zero/bit24) is the true direct player-object request setter. Native 1E9F00 writes object+64=raw ID, object+08 bit0=1, object+30 byte=0 when request is new; if bit24=0 and object+60=ID, it writes object+68=FFFFFFFF and returns 0 (duplicate/current path). This is a static disassembly finding; the Stage40 observer did not snapshot these fields, so no live direct object+64 write yet proven.
- Stage41 SOURCE-ONLY branch [stage41-direct-object-commit-window](https://github.com/Hawky11/1/tree/stage41-direct-object-commit-window), [install notes](https://github.com/Hawky11/1/blob/stage41-direct-object-commit-window/Stage41_Overlay/README_STAGE41_RU.md), [static report](https://github.com/Hawky11/1/blob/stage41-direct-object-commit-window/Stage41_Analysis/STATIC_ANALYSIS.md). Adds five masked native object O=[P+4] DWORD snapshots (O+08, +30, +60, +64, +68) to already working Stage40 TLS/ring/worker, logs matching stage40Seq to ArcanaAI_Stage41ObjectCommitTrace.log; no new native CALL patch or gameplay write. Preserve existing Stage29 AITrace.cpp, Stage35/36, Stage38/39, INI and Build.bat. One-file replacement: src/Stage40RequestConsumer.inl from Stage41_Overlay. Stage41 Windows x86 compilation and runtime are NOT YET VERIFIED; natural COM-vs-COM build/run and fresh Stage41/40/39 logs needed before any stronger claim.


## 2026-11-10 — Stage41 object request runtime VERIFIED; Stage42 native activation source prepared

- Latest user runtime log upload [c39c966](https://github.com/Hawky11/1/commit/c39c9663eeba7e0a91b24c43f83661a54bedf598) (2026-11-10 06:09:11 UTC). Original uploaded Build.log and all log blobs kept unchanged at [stage41-runtime-object-commit-evidence](https://github.com/Hawky11/1/tree/stage41-runtime-object-commit-evidence); [detailed analysis](https://github.com/Hawky11/1/blob/stage41-runtime-object-commit-evidence/Stage41_Runtime/RUNTIME_ANALYSIS.md), [306 classified cases](https://github.com/Hawky11/1/blob/stage41-runtime-object-commit-evidence/Stage41_Runtime/stage41_object_commit_classified.csv). Old source/evidence stage branches remain intact.
- Actual Windows x86 Build.log 09:07:12 local: BUILD SUCCEEDED, dinput8.dll 805376 bytes, only preexisting C4754 in AICommandSiteTrace.cpp, no compiler/link failures. Stage40 r2 INSTALLED 3/3; Stage41 DIRECT_OBJECT_BUILD r1 ENABLED with no additional hooks. Stage40 306 consecutive windows and Stage41 306 consecutive windows match exactly by stage40Seq, P1/P2, player pointer, TID, request, mode, both QPCs and O=[P+4] before/after. All 306 have read masks 1F/1F, stable O, no Stage40/41 explicit errors. P1=133, P2=173, 98 distinct raw request IDs.
- Native 1E9F00 direct request branch classifier based on original EXE: 257 NEW/REQUEUE (P1 133 / P2 124), ALL 257 end with O+64=raw ID, O+08 bit0 set, and low byte O+30=0. 254 observable changes in O+64 (3 already equaled ID); 255 bit0 0->1 (2 were already 1). This **runtime confirms** pending action request propagation into original native object state.
- 49 CURRENT/DUPLICATE shortcut cases (ALL P2): bit24 before=0 and O+60 before equals request, 49/49 O+68 after=FFFFFFFF, 49/49 O+64 unchanged. IMPORTANT: O+68 was already FFFFFFFF BEFORE in all 49, so do NOT claim direct observed writes to O+68; the branch is only confirmed as condition/field-consistent with static logic.
- Across all Stage41 calls, O+08 DWORD changed 255, O+30 changed 107, O+64 changed 254, while O+60 (current action ID) changed ZERO and O+68 changed ZERO. Stage41 alone does NOT prove active action (O+60) was committed. Stage39 50 raw action outputs match first subsequent same-side Stage40 request in 50/50 cases. EventDispatch type1 55 matched Stage37 55/55; Stage36 r2 882 sequential candidates with 13 non-sentinel native EAX; QueueTrace updated. Huge Stage38/Stage35 source logs were not fully accessible via connector text and are preserved unmodified; do not claim their full-file integrity.
- Next original native boundary disassembled directly from SHA-pinned Library AALib.exe SHA 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba: 1E9F40 tests O+08 bit0 at RVA 1E9F5C, calls 1E9C70 from **RVA 1E9F62** when set (original bytes E8 09 FD FF FF). Native 1E9C70 clears O+08 bit0 at 1E9C73, then reads O+64 at 1E9E0B and writes O+60 at 1E9E10. Four linked-object consumer calls from AD710 at AD75C/AD778/AD797/AD7BC confirmed. This is a natural pending-to-active handoff; no forced writes.
- **Stage42 SOURCE-ONLY READY**, branch [stage42-native-activation-consumer-trace](https://github.com/Hawky11/1/tree/stage42-native-activation-consumer-trace), [two-file installation](https://github.com/Hawky11/1/blob/stage42-native-activation-consumer-trace/Stage42_Overlay/README_STAGE42_RU.md), [native static analysis](https://github.com/Hawky11/1/blob/stage42-native-activation-consumer-trace/Stage42_Analysis/STATIC_ANALYSIS.md). One extra strict read-only CALL wrapper at 1E9F62->1E9C70 with native replay once, x87/SSE/GPR/flags preservation, P1/P2 object matching, five read-only object fields and health counters in ArcanaAI_Stage42ActionActivationTrace.log. Stage38 worker reused. Install ONLY (1) Stage42_Overlay/src/Stage38CallbackWindow.inl replacing local src/Stage38CallbackWindow.inl and (2) new Stage42_Overlay/src/Stage42ActivationHook.inl in local src; preserve working Stage41 src/Stage40RequestConsumer.inl, Stage39ActionOutput.inl, AIQueueTrace.cpp, Stage35/36, Stage29 AITrace.cpp, INI, Build.bat.
- Stage42 Windows x86 compilation, hook install and runtime activation are **NOT YET VERIFIED**. Next user natural COM-vs-COM + fresh Stage42/41/40/39/38/37/36 and Build.log to main, with ≥2s worker flush. Do NOT infer hit/damage/AI quality from O+60 alone or ask for another original EXE.


## 2026-11-10 — Stage42 activation RUNTIME VERIFIED, Stage43 native-caller provenance source ready

- Latest user upload in main [a1096e9](https://github.com/Hawky11/1/commit/a1096e980b6d40c290741693b6e8bc9116af01c8), SHA-pinned raw uploaded log blobs retained unmodified in [stage42-runtime-activation-evidence](https://github.com/Hawky11/1/tree/stage42-runtime-activation-evidence), with [full analysis](https://github.com/Hawky11/1/blob/stage42-runtime-activation-evidence/Stage42_Runtime/RUNTIME_ANALYSIS.md) and [356 exact Stage41-to-Stage42 row joins](https://github.com/Hawky11/1/blob/stage42-runtime-activation-evidence/Stage42_Runtime/stage41_to_stage42_activation_join.csv). All old stage branches untouched.
- Real Windows MSVC x86 BUILD SUCCEEDED (2026-11-10 09:30:43 local), dinput8.dll 810496 bytes, only pre-existing C4754. Stage42_ACTION_ACTIVATION_BUILD r1 and Stage42 INSTALLED 1/1 at original native 1E9F62->1E9C70. Stage40 r2 INSTALLED 3/3, Stage41 r1 ENABLED. Stage42 356 sequential full-mask (1F/1F) activation windows, P1=154/P2=202, native bit0 O+08 1->0 in ALL, O+60 POST equals O+64 PRE in ALL. O+60 actually changed in 354/356; in two it already equaled the new ID. 94 distinct activated IDs. This proves the pending ID is promoted to current/active **native command state**; it is NOT a visual animation, hitbox, damage or AI quality result.
- 36 Stage42_HEALTH records: 3492 intercepted original native calls, 3136 unrelated native objects, 356 P1/P2 activations, dropped=overflow=mismatch=0 throughout. Valid, monotonic QPC and contiguous sequences. Existing Stage41 426/426 match corresponding Stage40 (P1=166, P2=260), full masks and stable object. Stage41 new/requeue 350 records; duplicate/current 76. **350 of 356 Stage42** are exact last preceding same-object Stage41 pending-ID matches; **350 distinct Stage41 sequence IDs**, exhausting **all 350 new/requeue** events, one activation each. Lags 2..376 QPC ticks, median 6. Six additional Stage42 activations are legitimate promotions not matching last Stage41 pending raw ID.
- Six extras: P1 seq=204/229/292 activate 00000025, while latest Stage41 ID=0000002F and previous O+68=00000025, Stage42 pre O+30 lowbyte=1; consistent with native 1E9F40 alternate O+68->O+64 branch, intermediate write not directly observed. P2 seq=83/285/286 activate 0000024C, 0000024D, 0000024C, whereas latest Stage41 ID=00000248 and Stage42 pre O+30=0 and O+68=FFFFFFFF; upstream writer unknown, NOT an AI failure. Stage39 58/58 outputs joined first next same-side Stage40 exact ID, Stage37 66/66 exact EventDispatch type1 match, Stage36 495 candidate rows with 12 non-sentinel native return values. Large Stage38/Stage35 logs not completely readable through GitHub text fetch; raw original blobs preserved and no full health claim made.
- New SOURCE-ONLY [stage43-native-activation-origin-trace](https://github.com/Hawky11/1/tree/stage43-native-activation-origin-trace), [one-file install instructions](https://github.com/Hawky11/1/blob/stage43-native-activation-origin-trace/Stage43_Overlay/README_STAGE43_RU.md), [static analysis](https://github.com/Hawky11/1/blob/stage43-native-activation-origin-trace/Stage43_Analysis/STATIC_ANALYSIS.md). Original Library AALib.exe SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba was verified; 14/14 original direct native CALL sites into RVA 1E9F40 matched. Existing Stage42 wrapper can read original leaf-function caller return address from saved PUSHAD ESP+8 (and native return from ESP+4), logging parentCallerRVA in ArcanaAI_Stage43ActivationOriginTrace.log 1:1 with Stage42 seq/QPC. Source retains exactly ONE previously proven Stage42 CALL patch at 1E9F62, one native replay, original Stage42/41/40 behavior and worker; no added hooks, injected inputs, game-state writes, threads or INI.
- **NEXT USER:** Replace ONLY local D:\INCUBATOR\ArcanaAIOverhaul\src\Stage42ActivationHook.inl with [Stage43_Overlay/src/Stage42ActivationHook.inl](https://github.com/Hawky11/1/blob/stage43-native-activation-origin-trace/Stage43_Overlay/src/Stage42ActivationHook.inl); preserve existing Stage38CallbackWindow.inl (including Stage42), Stage41 Stage40RequestConsumer.inl, Stage39ActionOutput.inl, AIQueueTrace.cpp, Stage35/36, Stage29 AITrace.cpp, INI and Build.bat. Archive old logs; x86 Build.bat -> require BUILD SUCCEEDED; install new dinput8.dll; natural COM-vs-COM, >=2s flush; upload fresh Build.log, Stage43/42/41/40/39/38/37/36 and related logs to main. Stage43 Windows x86 compilation and runtime are NOT YET VERIFIED. No original EXE re-upload required.


## 2026-11-10 — Stage43 runtime VERIFIED, 232 native activation callers at AD75C; Stage44 source ready

- User uploaded fresh runtime in [main 09526f3](https://github.com/Hawky11/1/commit/09526f3edebd3057bad4a524cbaa75509a7fbfd6); immutable original user log blobs frozen in [stage43-runtime-activation-origin-evidence](https://github.com/Hawky11/1/tree/stage43-runtime-activation-origin-evidence), [full Stage43 report](https://github.com/Hawky11/1/blob/stage43-runtime-activation-origin-evidence/Stage43_Runtime/RUNTIME_ANALYSIS.md) and [232-row exact join CSV](https://github.com/Hawky11/1/blob/stage43-runtime-activation-origin-evidence/Stage43_Runtime/stage43_native_origin_join.csv). All earlier source/evidence branches retained.
- Real Windows x86 Build.log 09:55:43 local: BUILD SUCCEEDED, dinput8.dll 1041408 bytes; only old AICommandSiteTrace.cpp C4754 warning. Stage43 ACTION_ORIGIN_BUILD r1 ENABLED, Stage42 INSTALLED 1/1, Stage41 ENABLED, Stage40 INSTALLED 3/3. Stage43 232 records sequence 1..232, exact 232/232 joins to Stage42 by side/P/TID/O/QPC/pendingID/activeID and fields O+30/O+68; all nativeReturnRVA=1E9F67, callerRead=knownCaller=1 and **callerRVA=000AD75C for every P1/P2 activation (P1 114, P2 118)**. This confirms 1E9F40 was invoked by the AD710 object sweep at AD75C *in this run*; no assertion that all possible game callers are only AD75C or that upstream O+64 writer was identified.
- Stage42 232/232 activate O+60 POST=O+64 PRE, bit0 O+08 1>0, 230/232 actual current-ID changes and 2 already-equal; masks 1F/1F and QPC ordered. 30 health rows total calls=2090, otherObjects=1858, P1/P2=232 and dropped=overflow=mismatch=0. Stage41 295/295 matching Stage40 snapshots, NEW/REQUEUE 232 and CURRENT/DUPLICATE 63; all 232 NEW queued IDs correspond to exactly 232 distinct Stage42 activations, zero mismatched new requests, zero extra activations in this run (the earlier six extra events did NOT recur, so their origins remain unknown). Stage39 39/39 exact next same-side Stage40 request ID; Stage37 45/45 exact EventDispatch type1; Stage36 276 candidates (7 non-sentinel native EAX). Large Stage38 and Stage35 full-log integrity not verified because GitHub text connector returned empty text; original blobs preserved.
- New SHA-pinned original Library AALib.exe static disassembly: after 1E9C70 writes O+60 active ID at RVA 1E9E10, original CALL **1E9E2D -> 1E8490** (bytes E8 5E E6 FF FF) resolves the native action definition. At 1E84B3, 1E8490 calls 1DF2D0 with current O+60 ID and O+6C variant, returns action-definition pointer or fallback; then 1E84B8 stores pointer into **O+34**, 1E84BB sets byte **O+26D=1**, 1E84D5 writes metadata byte from definition+2C into **O+178**. All target prologue, CALL bytes/target and these writes verified against original executable SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba. These are static, NOT runtime Stage44 results.
- **Stage44 SOURCE ONLY** in [stage44-action-definition-binding-trace](https://github.com/Hawky11/1/tree/stage44-action-definition-binding-trace), [install instructions](https://github.com/Hawky11/1/blob/stage44-action-definition-binding-trace/Stage44_Overlay/README_STAGE44_RU.md), [static report](https://github.com/Hawky11/1/blob/stage44-action-definition-binding-trace/Stage44_Analysis/STATIC_ANALYSIS.md), [verifier](https://github.com/Hawky11/1/blob/stage44-action-definition-binding-trace/Stage44_Analysis/tools/verify_stage44.py). Add exactly one validated passive original CALL wrapper 1E9E2D->1E8490, original native once, preserve GPR/EFLAGS/x87/SSE, exact P1/P2 O snapshots O+60/+34/+26C (ready-byte +26D)/+178/+6C/+08/+0C/+48 plus definition+20/+2C/+48, mask/health/ring, existing Stage38 worker. No gameplay writes, forced inputs/IDs, extra threads/INI; existing Stage43 origin logs retained.
- **NEXT USER:** Replace ONLY local D:\INCUBATOR\ArcanaAIOverhaul\src\Stage38CallbackWindow.inl from Stage44_Overlay/src and ADD new local src\Stage44ActionDefinition.inl. KEEP current working Stage43 src\Stage42ActivationHook.inl, Stage41 src\Stage40RequestConsumer.inl, Stage39ActionOutput.inl, AIQueueTrace.cpp, Stage35/36, Stage29 AITrace.cpp, ArcanaAI.ini, Build.bat. Archive logs; Windows x86 Build.bat, install DLL only if BUILD SUCCEEDED, natural COM-vs-COM, >=2sec flush, upload fresh Build.log, ArcanaAI_Stage44ActionDefinitionTrace.log and Stage43/42/41/40/39/38/37/36 plus related logs to main. **Stage44 Windows build/runtime NOT YET VERIFIED.** Do not infer animations, hit, damage or actual AI improvement from metadata binding alone.


---

## Current — Stage44 runtime VERIFIED (2026-11-10)

Upload [73a93ed](https://github.com/Hawky11/1/commit/73a93ede99755e063896c78b510a2acee70311ba). [Evidence and report](https://github.com/Hawky11/1/blob/stage44-runtime-action-definition-evidence/Stage44_Runtime/RUNTIME_ANALYSIS.md), evidence commit b7c72b83b4b382751cedd9310809788943113fc0; original logs preserved plus Stage44 source overlay and 321-row CSV.

Windows x86 BUILD SUCCEEDED (1,047,552-byte DLL), existing C4754 only. Stage44 installed 1/1. 321 binding windows (P1 166/P2 155), FF/FF object masks and post metadata mask 7; all ready-byte=1 and metadata type consistent. 292/292 Stage42 activations each contain one matching Stage44 binding. 29 bindings outside Stage42 windows require separate caller attribution, not proof of lost actions. Stage42/44 health dropped/overflow/mismatch all zero. Stage41 291 NEW/REQUEUE -> 291 distinct activations, 64 duplicates; one extra Stage42 P1 seq160 ID26, original caller AD75C. All 292 Stage43 origins AD75C. Fallback lookup versus normal action definition remains unresolved; no animation/hit/damage or AI-quality claim. Stage35/38 large-log full-text verification unavailable, original blobs preserved.

**NEXT:** Stage45 static inspection of 1E84B3->1DF2D0 normal/fallback resolver and possible caller attribution of 29 other bindings. Stage45 source/DLL NOT prepared. No need to repeat Stage44. Original EXE already in Library. Preserve historical branches.



---

## Current — Stage44 runtime VERIFIED; Stage45 resolver SOURCE READY (2026-11-10)

[Stage45 source](https://github.com/Hawky11/1/tree/stage45-definition-resolver-trace), commit fe91fb09a31579f71e58d211670839838bdd33b1; [installation](https://github.com/Hawky11/1/blob/stage45-definition-resolver-trace/Stage45_Overlay/README_STAGE45_RU.md), [static analysis](https://github.com/Hawky11/1/blob/stage45-definition-resolver-trace/Stage45_Analysis/STATIC_ANALYSIS.md), [extracted-wrapper test](https://github.com/Hawky11/1/blob/stage45-definition-resolver-trace/Stage45_Analysis/tools/test_stage45.py).

**New original-EXE finding:** SHA-pinned AALib.exe verified again. Resolver 1DF2D0 checks context+ID+B0000 gate then signed WORD at context+((ID<<7)+variant)*2; positive index yields [context+B0B0C]+(index-1)*5C, otherwise fallback. Original fallback 0E2E49E0 has HIGHLOW relocation at RVA 1DF2FB, hence gameBase+0DEE49E0. At Stage44 runtime base 00800000 fallback=0E6E49E0; **none of 321 Stage44 post-definition pointers equals fallback**. This resolves the prior specific fallback uncertainty for that capture, not action semantics.

**Stage45:** one new passive strict CALL wrapper 1E84B3->1DF2D0, two original args copied and native called once, original/native stack cleanup retained, GPR/EFLAGS/x87/SSE saved around diagnostics. Captures only matching active Stage44 parent windows via TLS; logs context/ID/variant/gate/signed index/table base/prediction/EAX/fallback and bindingQpcBefore, dedicated ArcanaAI_Stage45DefinitionResolverTrace.log, existing worker. Full native signature plus relocation-aware immediate checks. 128 extracted-assembly/native Unicorn equivalence cases PASS (arguments, once-only native call, GPR/ESP/EFLAGS, XMM0/7, FPCW/MXCSR), SHA/CALL/relocation/integration PASS. **Windows x86 compile and game runtime remain PENDING.**

**NEXT USER:** in D:\\INCUBATOR\\ArcanaAIOverhaul\\src replace Stage38CallbackWindow.inl from Stage45_Overlay/src and add Stage45DefinitionResolver.inl. Keep working Stage44ActionDefinition.inl, Stage43 Stage42ActivationHook.inl, Stage41 Stage40RequestConsumer.inl, Stage39, AIQueueTrace.cpp, Stage35/36, Stage29 AITrace.cpp, INI and Build.bat. Archive logs, x86 Build.bat, require BUILD SUCCEEDED, install fresh DLL, natural COM-vs-COM and >=2s flush. Upload Build.log, Stage45/44/43/42/41/40/39/38/37/36, Queue/EventDispatch/ParentFlow/FinalCommand/Stage35 to main.

**NEXT ANALYST:** require STAGE45_DEFINITION_RESOLVER_BUILD r1 and INSTALLED 1/1; join bindingQpcBefore to Stage44 by side/P/O/TID, nested QPC, matching ID/variant and EAX=Stage44 POST O+34. Normal mask 1F/path3, fallback paths 1/2 use applicable bits, unknown path0 is not proof of invalid game data. Check predictions, actual fallback equality and zero loss counters. otherObjects includes calls outside captured Stage44 windows. 29 prior outside-Stage42 bindings still lack upstream caller attribution; Stage45 does NOT identify their producer. Stage44 evidence remains b7c72b83b4b382751cedd9310809788943113fc0; preserve all historical branches. Original EXE already in Library.



---

## Current — Stage45 runtime VERIFIED (2026-11-10)

Fresh upload [4f8d227](https://github.com/Hawky11/1/commit/4f8d227d01cf624526569aa89999e092ca65ef9d). Exact uploaded log blobs preserved at [stage45-runtime-definition-resolver-evidence](https://github.com/Hawky11/1/tree/stage45-runtime-definition-resolver-evidence), evidence commit 3fdb8647fae45cc517281c7f74ffaa4a9789c203; [report](https://github.com/Hawky11/1/blob/stage45-runtime-definition-resolver-evidence/Stage45_Runtime/RUNTIME_ANALYSIS.md), [231-row CSV](https://github.com/Hawky11/1/blob/stage45-runtime-definition-resolver-evidence/Stage45_Runtime/stage45_resolver_binding_join.csv). Source fe91fb09a31579f71e58d211670839838bdd33b1 now has successful Windows x86 build/runtime evidence: DLL 1,052,672 bytes, only existing C4754; Stage45 INSTALLED 1/1, Stage44 1/1, Stage42 1/1, Stage43 enabled, Stage40 3/3.

**Proven:** 231 resolver records (P1 106/P2 125), all mask1F/path3/predictionMatches1/resultIsFallback0; independently calculated table pointer equals actual EAX. All 231 join distinct Stage44 bindings by parent QPC/P/O/TID, nested time, ID/variant and EAX=POST O+34. All variants zero; no fallback/nonzero variant sample. All binding fields/masks/ready byte/metadata type consistent. 209 Stage42 activations nested into this binding chain, all 209 NEW Stage41 requests -> distinct activations; 305 Stage41 total, 96 duplicates; no extra Stage42 activation. All 209 origins AD75C. 22 binding/resolver pairs outside Stage42 (including initial two ID0) remain unassigned upstream but are also normal table results. No animation/hit/damage or improved-AI claim.

Stage42 HEALTH 2525 calls/2316 excluded; Stage44 9069/8838; Stage45 43539/43308; all dropped/overflow/mismatch zero. Full reviewed Stage36/37/39–45, Queue/EventDispatch sizes match Git blobs. Stage38 text adapter returned empty; Stage35 and remaining historical logs not fully verified, original blobs preserved. Stage39 31/31 next-consumer raw ID matches. See report for scope.

**NEXT Stage46:** inspect native downstream use of bound definition pointer and choose first concrete execution-state/animation-related consumer for passive observation. Stage46 source/DLL NOT prepared; no need to repeat Stage45. 22 outside-Stage42 bindings remain independent caller-attribution question. Keep working Stage45 sources and all prior historical branches. Original AALib.exe already in Library; do not request again.



---

## Current — Stage45 runtime VERIFIED; Stage46 motion-state SOURCE READY (2026-11-10)

[Stage46 source](https://github.com/Hawky11/1/tree/stage46-definition-motion-state-trace), commit 2365dec68429d8cf404bdfdca58e0ee171b109e7; [installation](https://github.com/Hawky11/1/blob/stage46-definition-motion-state-trace/Stage46_Overlay/README_STAGE46_RU.md), [static report](https://github.com/Hawky11/1/blob/stage46-definition-motion-state-trace/Stage46_Analysis/STATIC_ANALYSIS.md), [verification](https://github.com/Hawky11/1/blob/stage46-definition-motion-state-trace/Stage46_Analysis/tools/verify_stage46.py).

Original SHA-pinned AALib.exe verified. Native 1E8490 after binding: 1E8A1F requires signed O+70 < O+6C; 1E8A2D requires O+08 bit9 clear; byte definition+30 dispatches 1..9 through 1E8D30. Verified float writes/zeroing at O+7C/+80/+88/+8C/+90/+94/+150/+154, signed metadata WORD conversion /100, conditional sign inversion, common tail ground clamp and O+70 advancement. These are motion-related raw object-state operations, NOT proof of rendered animation/hit/damage or AI quality.

**Stage46 source only:** no new hooks/asm/worker/INI. Extends Stage44 record/ring with 13-field PRE (after resolver return, before O+34 native store) and POST (after binding native return) plus definition +20/+30/+34/+38 DWORDs. Stage45 leave calls S46Resolved; Stage44 flush emits ArcanaAI_Stage46DefinitionMotionTrace.log with stage44Seq/P/O/TID/parent QPC/resolvedQpc, resolverCount, definition/definitionAfter and masks. Keep first resolution, count repeats. Stage44 HEALTH governs shared ring, also check Stage45. Native assembly wrappers unchanged. 64 original-fragment Unicorn cases PASS plus original SHA/guard/dispatch/tail signatures and constants; helper bit query stubbed false, modes 2/7/8 and whole-function runtime not tested. **Windows x86 build and game runtime PENDING.**

**NEXT USER:** replace EXACTLY TWO files in D:\\INCUBATOR\\ArcanaAIOverhaul\\src from Stage46_Overlay/src: Stage44ActionDefinition.inl and Stage45DefinitionResolver.inl. KEEP Stage45 Stage38CallbackWindow.inl, working Stage43 Stage42ActivationHook.inl, Stage41 Stage40RequestConsumer.inl, Stage39, AIQueueTrace.cpp, Stage35/36, Stage29 AITrace.cpp, INI/Build.bat. Archive logs, x86 Build.bat; only install fresh DLL after BUILD SUCCEEDED. Natural COM-vs-COM, >=2s flush. Upload Build.log, Stage46/45/44/43/42/41/40/39/38/37/36, Queue/EventDispatch/ParentFlow/FinalCommand/Stage35 to main.

**NEXT ANALYST:** verify STAGE46_DEFINITION_MOTION_BUILD r1 and ENABLED, Stage44/45 installations. Join Stage46 to Stage44 by seq/P/O/TID/QPC and to Stage45 EAX/parent QPC. Expected resolverCount=1, object masks 1FFF/1FFF, metadata F; definitionAfter must match resolved definition. Decode f5..f12 as IEEE754. Analyze command byte +30 and guards against raw-field changes; enclosing function may have other writes/helper conditions, so endpoint mismatches are not automatically bugs or lost commands. Check shared health counters. Treat 22 prior outside-Stage42 bindings separately; caller attribution still unknown. Stage45 evidence 3fdb8647fae45cc517281c7f74ffaa4a9789c203 remains immutable. Original EXE already in Library.


---

## 2026-11-10 — Stage46 runtime VERIFIED, native motion-field initialization

- User upload [main 3e668d7](https://github.com/Hawky11/1/commit/3e668d74ec1181ee6adf5952c7665e8948b767df) frozen verbatim in [Stage46 runtime evidence](https://github.com/Hawky11/1/tree/stage46-runtime-definition-motion-evidence); source 2365dec unchanged, report [RUNTIME_ANALYSIS.md](https://github.com/Hawky11/1/blob/stage46-runtime-definition-motion-evidence/Stage46_Runtime/RUNTIME_ANALYSIS.md) and [297-row CSV](https://github.com/Hawky11/1/blob/stage46-runtime-definition-motion-evidence/Stage46_Runtime/stage46_motion_binding_join.csv) in evidence branch, report commit 4cf28b4.
- Windows x86 BUILD SUCCEEDED, DLL 1,056,256 bytes, old AICommandSiteTrace.cpp C4754 only. Stage46 enabled, Stage44/45 installed. 297 Stage46 windows (P1 152/P2 145), mask1FFF/1FFF/F, resolverCount1; 297/297 matched Stage44 and Stage45 EAX/ID/variant/parent QPC. Stage45 resolver QPC lag 0 (221), 1 (75), 9 (1) ticks; all within parent. 297/297 normal path3, prediction matches, no fallback.
- Guard (signed O+70<O+6C and bit9 clear) passes 290/297; seven guard-false mode0. Mode counts 0:208, 1:9, 2:2, 3:51, 4:19, 5:8. Float fields change in 68 windows: O+7C in 1, O+80 in 11, O+88 in 44, O+8C in 23, O+90 in 32, O+94 in 4, O+150 in 0, O+154 in 3 (overlap and possible negative zero). Seq7 mode3 O+88 0 to 3.86/O+90 0 to 0.64; seq3 mode2 O+7C 890 to 982/O+80 0 to 196. This is concrete native definition-driven object-state initialization, NOT proof of animated movement, hit, damage, improved AI.
- Stage42/43 269/269 origins AD75C and nested bindings. Other 28 Stage44 bindings outside Stage42 not explained upstream. Stage40/41 294/294; 266 new Stage41 bit0 writes each match a distinct Stage42 activation. Extra Stage42 seq3 P2 0506 to 0, seq4 P1 0506 to 0, seq209 P1 002F to 0025, all AD75C but no new Stage41 write. Stage42 health 2943 calls /2674 other; Stage44 9706/9409; Stage45 52979/52682, all dropped/overflow/mismatch 0. Stage36/37 and Stage39-46 read in full and file sizes/newlines checked. Stage35/38 huge text not fully validated.
- NEXT Stage47 (SOURCE NOT PREPARED): use original AALib.exe already in Library, trace actual consumers of O+88/+8C/+90/+94 and O+7C/+80, then only passive x86 instrumentation at verified native boundary for consumer/position or animation-state correlation. Do not repeat Stage46 or request original EXE. Preserve all historical branches.


---

## 2026-11-10 — Stage47 passive native motion-consumer SOURCE READY, runtime PENDING

- Branch [stage47-native-motion-consumer-trace](https://github.com/Hawky11/1/tree/stage47-native-motion-consumer-trace), source commit [726c6c7](https://github.com/Hawky11/1/commit/726c6c73f128de36f927d7d534c1b566a2491788), [installation](https://github.com/Hawky11/1/blob/stage47-native-motion-consumer-trace/Stage47_Overlay/README_STAGE47_RU.md), [static analysis](https://github.com/Hawky11/1/blob/stage47-native-motion-consumer-trace/Stage47_Analysis/STATIC_ANALYSIS.md), [SHA-pinned verifier](https://github.com/Hawky11/1/blob/stage47-native-motion-consumer-trace/Stage47_Analysis/tools/verify_stage47.py). Branched from unchanged Stage46 source commit 2365dec; Stage46 runtime 3e668d7 and evidence branch preserved.
- Original Library AALib.exe 175556608 SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba used locally; verified all six direct CALLs with original arguments to 1E66E0, two object-list root instructions, original prologue/two RET 4 sites and 14 field-reading/writing native signatures. Consumers read O+88/+8C/+90/+94 and update O+7C/+80/+88/+8C; still cannot label visible movement/animation or hit/damage.
- Calls AE70C and AE93E enumerate E2E02C8 (same list as Stage43 P1/P2 AD75C); four calls from other lists not patched. New passive Stage47MotionConsumer.inl preserves GPR/flags/x87/SSE around diagnostics, copies single native stack DWORD, original native once, RET4 wrapper, strict checks, two-site installation under mutex/thread suspension, one page VirtualProtect, existing Stage38 worker/ring 4096, P1/P2 13-field PRE/POST snapshots and definition+30, new Stage47 log. Only modifies adapter Stage38CallbackWindow.inl (include/conditional install/flush); retains tested Stage46 Stage44/45 code.
- Static local SHA and native CALL/field verification PASS, structural source checks PASS. **Windows x86 compilation, wrapper execution equivalence, actual game runtime and P1/P2 branch coverage NOT TESTED.** Future runtime must check exact installed 2/2, records, qpc, masks and health. Preserve all history and do not repeat Stage46 or ask for EXE.

---

## 2026-11-10 — Stage47 runtime verified; Stage48 two-variant-binder source ready

- User uploaded exact new run [main e517667](https://github.com/Hawky11/1/commit/e5176670e76396174cf58986fe3bf62c50c47178); original Build.log and runtime log blobs frozen in [stage47-runtime-motion-consumer-evidence](https://github.com/Hawky11/1/tree/stage47-runtime-motion-consumer-evidence), detailed report [Stage47_Runtime/RUNTIME_ANALYSIS.md](https://github.com/Hawky11/1/blob/stage47-runtime-motion-consumer-evidence/Stage47_Runtime/RUNTIME_ANALYSIS.md), [5,614-row joined CSV](https://github.com/Hawky11/1/blob/stage47-runtime-motion-consumer-evidence/Stage47_Runtime/stage47_consumer_binding_join.csv), [336-binding summary CSV](https://github.com/Hawky11/1/blob/stage47-runtime-motion-consumer-evidence/Stage47_Runtime/stage47_binding_summary.csv). Evidence report commit 447557349924279cae7fa8a9b1c83f90d9ba5e76. Source Stage47 remains immutable. Windows x86 BUILD SUCCEEDED, DLL 1,063,424 bytes, old C4754 warning only, Stage47 INSTALLED 2/2 AE70C/AE93E native 1E66E0.
- Full Stage47 blob 3,202,722 bytes, 5,614 sequential P1/P2 windows (2,807/2,807), all actual P1/P2 observations caller AE70C; AE93E not observed. Every argument 0, full masks 1FFF/1FFF, extraMask F, all changedMask/QPC checks consistent. Stage47 health 49,315 calls, 43,701 otherObjects, 5,614 P1/P2, dropped/overflow/mismatch/badCaller zero. 3,703 windows show raw IEEE754 changes (O+7C 2248, O+80 2415, O+88 795, O+8C 3173, O+90 0, O+94 11, O+150 88, O+154 20), overlapping counts.
- All 5,614 stage47 events joined nearest preceding Stage46 binding on side/object and SAME active ID; exact 0x5C variant/definition stride invariant 5,614/5,614: Stage47 O+34 pointer = last Stage46 definitionAfter + (Stage47 O+6C - Stage46 O+6C)*0x5C. Variant range 0..37, 1,581 base rows/4,033 positive multiples; zero mismatches. No Stage47 consumer window itself writes O+34. 339/339 Stage46↔44↔45 joined; 336 bindings followed by Stage47 consumers, 320 first matching previous mode/def full eight floats. Stage42/43 298 origin AD75C; all nested Stage44; 41 additional Stage44 outside. Stage40/41 363/363, 296 new bit0 writes match distinct activations, two extra Stage42 seq3/4 no new Stage41, known caller AD75C. All Stage42/44/45 health loss counters zero. Stage36/37/39–47 text checked; Stage35/38 large logs not fully verified.
- Static original physical EXE disassembly (SHA 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba) reveals 3 direct native binder CALLs 1E9E2D (Stage44), **1E8DA8** (1E8D60 variant change), **1E908F** (1E8DC0 timing/variant update). New branch [stage48-native-variant-binder-trace](https://github.com/Hawky11/1/tree/stage48-native-variant-binder-trace), [installation](https://github.com/Hawky11/1/blob/stage48-native-variant-binder-trace/Stage48_Overlay/README_STAGE48_RU.md), [static report](https://github.com/Hawky11/1/blob/stage48-native-variant-binder-trace/Stage48_Analysis/STATIC_ANALYSIS.md), [verifier](https://github.com/Hawky11/1/blob/stage48-native-variant-binder-trace/Stage48_Analysis/tools/verify_stage48.py), source commit 2a96d06b072852e4e82a2a8ccd7edb6d8cbe8d5c. Reuses proven S44BindingHook at 2 additional CALL sites and existing Stage44 ring with callsite attribution; no new ASM wrapper, threads or gameplay changes. Static native byte and minimal-overlay structural checks PASSED. Stage48 Windows x86 compile/game runtime not tested. NEXT user replaces exactly Stage38CallbackWindow.inl and Stage44ActionDefinition.inl and adds Stage48VariantBinder.inl, preserves working Stage45 Stage46 .inl and Stage47 source/INI etc. New Stage48 log and Build.log needed. No original EXE reupload, no Stage47 repeat. No rendered movement/animation/hit/damage or AI-quality claim.


---

## 2026-11-10 — Stage48 runtime VERIFIED, Stage49 linked-state consumer SOURCE READY

- Fresh user upload split across 7c32c4a / e4230d9 / [72bd22d](https://github.com/Hawky11/1/commit/72bd22dcfffbb324e2ec8bd37409aed4060ac8b1). Original logs retained exactly in [stage48-runtime-variant-binding-evidence](https://github.com/Hawky11/1/tree/stage48-runtime-variant-binding-evidence); [technical report](https://github.com/Hawky11/1/blob/stage48-runtime-variant-binding-evidence/Stage48_Runtime/RUNTIME_ANALYSIS.md), [2,317-row Stage48 table](https://github.com/Hawky11/1/blob/stage48-runtime-variant-binding-evidence/Stage48_Runtime/stage48_variant_binding_join.csv), [8,052-row Stage47 continuity](https://github.com/Hawky11/1/blob/stage48-runtime-variant-binding-evidence/Stage48_Runtime/stage47_to_stage48_join.csv). Windows x86 BUILD SUCCEEDED, DLL 1,067,008 bytes, old C4754 warning. Stage48 INSTALLED 2/2 callsites 1E8DA8/1E908F native 1E8490. 2,317 valid Stage48 PRE/POST windows 2317/2317 match Stage44/45/46 native resolver. Callers 1E908F=1,924 (P1 898/P2 1026); 1E9E2D=393; 1E8DA8=0 observed P1/P2.
- All **1,924** native 1E908F variant transitions obey delta(pointer)=0x5C*delta(variant) versus previous same-ID binder; all current native PRE pointers equal earlier post and return POST pointers match Stage45 resolver. Variation includes 1,703 +1; 109 -1; 50 -3; 32 -5 and other jumps. Stage47 has 8,052 complete consumer windows (same AE70C caller), 5,229 change native float state. Nearest preceding Stage48 binder matches 8,051/8,052 consumers on ID+variant+definition; single last-file P2 seq8051 ID050C variant4 pointer29C66E18 after last P2 binder variant3 pointer29C66DBC, +0x5C, not yet explained and NOT proven loss. Stage44/45/47 health dropped/overflow/mismatch/badCaller zero; Stage40/41 482/482; Stage42/43 349/349, all caller AD75C, 345 new bit0 writes matched, 4 other activations remain valid. Large Stage35/38 not fully text-verified.
- Physical SHA-pinned original AALib.exe static analysis finds native 1E9170 with guarded link O+2D0, comparison linked+60 vs O+280, and linked+7C/+80 floated into O+7C/+80/+AC/+B0. Direct native CALLs AF13D in linked list E2E02C8 and AF1AA in **different conditional list E2E02C0**. Created [stage49-linked-motion-state-propagation](https://github.com/Hawky11/1/tree/stage49-linked-motion-state-propagation) with passive 2-site x86 wrapper preserving original-call once and GPR/FLAGS/x87/SSE, strict signatures/transactional patch, P1/P2 and linked-object snapshots, 4096 ring using existing worker. Static physical original SHA/17 signatures and native call destinations verified; prior Stage48 Stage38 adapter preserved exactly except include/install/flush; new wrapper assembly matches proven Stage44 shape. [Guide](https://github.com/Hawky11/1/blob/stage49-linked-motion-state-propagation/Stage49_Overlay/README_STAGE49_RU.md), [analysis](https://github.com/Hawky11/1/blob/stage49-linked-motion-state-propagation/Stage49_Analysis/STATIC_ANALYSIS.md), [verifier](https://github.com/Hawky11/1/blob/stage49-linked-motion-state-propagation/Stage49_Analysis/tools/verify_stage49.py). Windows x86 build and game runtime Stage49 **PENDING**. Replace exactly Stage38CallbackWindow.inl and add Stage49LinkedMotion.inl; keep working Stage48 Stage44/Stage48 binder, Stage47 motion, Stage46 resolver. No EXE request, no old stage repeat, no claim of actual on-screen animation/hit/damage or improved AI.


## 2026-11-10 — Stage49 r1 build success but runtime hooks blocked; r2 relocation-aware source prepared

- User main [upload 1dde176](https://github.com/Hawky11/1/commit/1dde176a9cf02285cea12de7f6cbd2252807690e) and [Build.log b7e0fbe](https://github.com/Hawky11/1/commit/b7e0fbedf493a2939ecf2ef96e9098ff80836db5), 14:37 local. Windows x86 `BUILD SUCCEEDED`, dinput8.dll **1,074,176 bytes**, previous C4754 warning only. Stage49 log marker r1 then `ERROR: Stage49 root/CALL/linked-field/ABI signature mismatch; no patches.`. **No Stage49 hook installed, no STAGE49_LINKED_WINDOW.** Preserve this distinction from successful build and earlier-stage traces.
- Stage47 direct Git blob contained 4,664 P1/P2 consumer records (2,332 each) and 2,899 windows with nonzero changedMask; 23 HEALTH samples: 40,268 calls, 35,604 other, **dropped/overflow/mismatch/badCaller=0**. Stage48 had 1,318 P1/P2 binding windows (679 P1 / 639 P2; 1E908F 1,040, 1E9E2D 278, 1E8DA8 zero). Stage44 1,318 with zero loss; Stage38 4,277, Stage35 9,336. Large blobs fetched via Git blob SHA; zero content returned by path-based fetch for very large logs must not be interpreted as empty logs.
- [Frozen Stage49 r1 failed-run evidence and exact uploaded logs](https://github.com/Hawky11/1/tree/stage49r1-guard-failure-evidence), [analysis](https://github.com/Hawky11/1/blob/stage49r1-guard-failure-evidence/Stage49_Runtime/RUNTIME_ANALYSIS.md); original [Stage49 r1 source](https://github.com/Hawky11/1/tree/stage49-linked-motion-state-propagation) preserved unchanged.
- Original EXE PE32 preferred ImageBase **0x00400000**; new Stage13 alternate-call log has `ret=0x008F6ADC` at RVA `0xF6ADC`, indicating live game image at **0x00800000**. Original Stage49 r1 root checks compare static absolute address operands `8B 35 C8 02 2E 0E` and `8B 35 C0 02 2E 0E` with runtime instruction bytes, probably mismatching loader's relocation (+0x00400000). This is a strong causal hypothesis, **not proven exact failing guard** because r1 reported aggregate failure.
- New immutable source branch [stage49r2-relocation-aware-root-guard](https://github.com/Hawky11/1/tree/stage49r2-relocation-aware-root-guard); code [25cbae8](https://github.com/Hawky11/1/commit/25cbae814e7c8c2baac002f7355a425b695f3a82), static verifier assertions [f326efb](https://github.com/Hawky11/1/commit/f326efbfe9944e807912d2d4a0a31de64c5b09ec). Only Stage49LinkedMotion.inl native source changes: root guard checks unrelocated opcode and exact `preferredOperand + (g_gameBase - 0x00400000)`, with detailed errors by RVA and first failed byte. All other original CALL/linked/ABI guards and fail-closed install retained; no AI/gameplay writes added. [Single-file r2 Windows installation guide](https://github.com/Hawky11/1/blob/stage49r2-relocation-aware-root-guard/Stage49_Overlay/README_STAGE49_R2_RU.md). **Stage49 r2 x86 build/runtime PENDING**.


## 2026-11-10 — Stage49 r2 verified; native linked-copy branch inactive for P1/P2; Stage50 source prepared

- Fresh user [runtime upload 8317ae92](https://github.com/Hawky11/1/commit/8317ae92eb2adca8e82e38702ed1cd6deaf9b90f) and [Build.log de975e3](https://github.com/Hawky11/1/commit/de975e380cd5ec33d339bfd113059492f7c13292). Windows x86 **BUILD SUCCEEDED**, dinput8.dll **1,075,200 bytes**, previous C4754 warning only; Stage49 `STAGE49_LINKED_MOTION_BUILD=2026-11-10-r2`, **STAGE49_INSTALLED 2/2**. r1 guard failure resolved. Exact logs and analysis frozen in [stage49r2-runtime-linked-motion-evidence](https://github.com/Hawky11/1/tree/stage49r2-runtime-linked-motion-evidence), [report](https://github.com/Hawky11/1/blob/stage49r2-runtime-linked-motion-evidence/Stage49_Runtime/RUNTIME_ANALYSIS.md), [cross-stage 6,818-row CSV](https://github.com/Hawky11/1/blob/stage49r2-runtime-linked-motion-evidence/Stage49_Runtime/stage49_stage47_stage48_join.csv).
- Complete Stage49 blob 4,175,910 characters, 6,818 contiguous P1/P2 linked windows (3,409 each), AF13D=6,818, AF1AA=0, full O snapshot mask 07FF in all; 5,512 have nonzero link and full linked mask 7. Every single `changedMask=0000` and `staticGateCandidate=0`. Native function 1E9170 did not copy linked coordinates within any observed P1/P2 window. O+10 values: 6,739 × 0x00000000, 62 × 0x00010000, 17 × 0x00000080; zero with bit15 0x00008000. 32 HEALTH samples calls=58,558, otherObjects=51,740, tracked=6,818, all dropped/overflow/mismatch/badCaller=0, no truncation/sequence gap.
- Stage47 6,756 P1/P2 windows, 4,243 changed (all source AE70C), 32 HEALTH samples calls 58,066 otherObjects 51,310, all loss/mismatch zero. Stage48 1,901 native binding windows (P1 981/P2 920), caller 1E908F 1,533, 1E9E2D 368, 1E8DA8 zero. Stage44/46 each 1,901, Stage38 6,326, Stage35 13,528. 6,818 Stage49 events match prior Stage47 and Stage48 by QPC/side/object; 6,818/6,818 Stage48 POST ID matches Stage49 active ID. 1,201 Stage47 last POST X `O+7C` values differ from Stage49 PRE X, two Y differences. This is evidence of intervening writer(s), not necessarily a particular function or visible world movement.
- SHA-verified original Library AALib.exe inspected as x86 PE32 physical bytes. Native CALL **AF136 E8 C5 EF FF FF → AE100** precedes **AF13D → 1E9170** in same E2E02C8 loop. Native AE100 receives **ESI=object**, is no-arg RET at AE216, condition `[O+3C]==4 || [O+1A4]==0x32`, and actual original instructions `AE1A6 FST O+7C; AE1A9 FSTP O+AC; AE1B8 FST O+80; AE1BE FSTP O+B0`. All 10 native signatures and rel32 target checked. AE100 is candidate to explain 1,201 intermediate X changes; runtime attribution **not yet tested**. Additional other-list 1E9170 direct CALL sites AF162 and AF182 also exist in this caller corridor; Stage49 only selected AF13D and AF1AA.
- New source branch [stage50-prelinked-native-coordinate-trace](https://github.com/Hawky11/1/tree/stage50-prelinked-native-coordinate-trace), [Stage50 adapter](https://github.com/Hawky11/1/blob/stage50-prelinked-native-coordinate-trace/Stage50_Overlay/src/Stage38CallbackWindow.inl), [new passive wrapper](https://github.com/Hawky11/1/blob/stage50-prelinked-native-coordinate-trace/Stage50_Overlay/src/Stage50PreLinkedCoordinate.inl), [static report](https://github.com/Hawky11/1/blob/stage50-prelinked-native-coordinate-trace/Stage50_Analysis/STATIC_ANALYSIS.md), [SHA-pinned verifier](https://github.com/Hawky11/1/blob/stage50-prelinked-native-coordinate-trace/Stage50_Analysis/tools/verify_stage50.py) and [Russian install guide](https://github.com/Hawky11/1/blob/stage50-prelinked-native-coordinate-trace/Stage50_Overlay/README_STAGE50_RU.md). One new passive CALL patch; no gameplay writes, original called once; 16 structural checks PASS. **Stage50 Windows x86 Build/runtime PENDING**; replace only Stage38 adapter and add Stage50 source; keep Stage49 r2. No claim of on-screen movement, hits/damage or improved AI.


## 2026-11-10 — Stage50 Windows x86 runtime VERIFIED, native AE100 inactive for P1/P2; Stage51 source READY

- Fresh runtime [e21cf58](https://github.com/Hawky11/1/commit/e21cf58ff4db752a5b0ef5dfbab88d6ed5def912), [Build.log 2ddee8a](https://github.com/Hawky11/1/commit/2ddee8a8b025ede1c799edf1ee11fde9d0ff7056): Windows x86 **BUILD SUCCEEDED** 15:55:39, DLL 1,081,344 bytes, existing C4754 warning only; **STAGE50_INSTALLED 1/1** at AF136→native AE100, Stage49 r2 remains installed. Frozen exact uploads and [RUNTIME_ANALYSIS.md](https://github.com/Hawky11/1/blob/stage50-runtime-prelinked-coordinate-evidence/Stage50_Runtime/RUNTIME_ANALYSIS.md) in [stage50-runtime-prelinked-coordinate-evidence](https://github.com/Hawky11/1/tree/stage50-runtime-prelinked-coordinate-evidence), [5,790-row exact QPC CSV](https://github.com/Hawky11/1/blob/stage50-runtime-prelinked-coordinate-evidence/Stage50_Runtime/stage47_stage50_stage49_join.csv).
- Stage50 exact Git blob 2,759,720 chars, 5,792 complete P1/P2 windows (2,896 each), pre/post masks 07FF, contiguous seq 1..5792, all caller AF136; `changedMask=0`, `gateCandidate=0` all. Runtime O+3C=0 and O+1A4=FFFFFFFF in 5,792/5,792, so native AE100 coordinate copying is never exercised for tracked objects. 28 HEALTH samples: calls 49,214 otherObjects 43,422 tracked 5,792; dropped/overflow/mismatch/badCaller all zero. No errors.
- Stage49 r2 5,790 P1/P2 windows (2,895 each), all caller AF13D, zero changedMask/gate, 5,405 linked pointers nonzero; health 49,198 total, 43,408 other, tracked 5,790, all loss counters zero. Stage47 5,668 (2,834 each) with 3,489 changed, health 48,034 total, 42,366 other, loss zero. Stage48/44/46 1,553 binds, Stage38 5,331 and Stage35 11,356 records.
- **5,790/5,790** Stage50 POST values O+7C, O+80, O+AC and O+B0 equal immediately subsequent Stage49 PRE values with exact same sequential index and same-side/object QPC. Stage47 previous POST X differs from Stage50 PRE X in **1,257/5,792**, Stage47 POST Y matches Stage50 PRE Y 5,792/5,792. **5,792/5,792** Stage47 POST X equals Stage50 PRE anchor X O+AC; the intervening writer changes live O+7C without changing anchor O+AC. No new Stage46 binder between those Stage47→Stage50 events. This is *not yet* attributed to particular native writer, visual animation, hit or AI quality.
- Original Library SHA-pinned AALib.exe physical static disassembly confirms AF10D CALL AEF10, AF112 CALL AED90, **AF117 PUSH 1; AF119 CALL AE220**, AF124 original caller ADD ESP,4, then per-object AF136 AE100 and AF13D 1E9170. AE220 is broader player/object update with one cdecl arg1; candidate for 1,257 X-only differences. [Stage51 source](https://github.com/Hawky11/1/tree/stage51-global-update-coordinate-boundary) adds original-call-once read-only pre/post both P1/P2 observer at AF119, preserving copied arg/return stack, register/flag/x87/SSE, and existing worker. 7 machine signatures and source/adapter structural checks PASS; [Russian install guide](https://github.com/Hawky11/1/blob/stage51-global-update-coordinate-boundary/Stage51_Overlay/README_STAGE51_RU.md), [static analysis](https://github.com/Hawky11/1/blob/stage51-global-update-coordinate-boundary/Stage51_Analysis/STATIC_ANALYSIS.md), [verifier](https://github.com/Hawky11/1/blob/stage51-global-update-coordinate-boundary/Stage51_Analysis/tools/verify_stage51.py). **Stage51 Windows x86 compile/runtime PENDING**; user replaces ONLY Stage38CallbackWindow.inl and ADDS Stage51GlobalCoordinate.inl, retains Stage49 r2 and Stage50.

## 2026-11-10 — Stage51 Windows x86 runtime VERIFIED — 435 same-frame pre-AE220 X changes

- Fresh uploads: [runtime logs b411d18](https://github.com/Hawky11/1/commit/b411d18a1c22a3f1f2ddd5c27b08cfb03f1f4894), [latest log continuation + Build.log 0341856](https://github.com/Hawky11/1/commit/034185644dd587f5e3fc1eaf725e24d0f8b93c6f). Windows x86 **BUILD SUCCEEDED**, 2026-11-10 16:14:35, `dinput8.dll` **1,089,024 bytes**, old C4754 warning only; game COMvsCOM active.
- Stage51 marker `STAGE51_GLOBAL_COORD_BUILD=2026-11-10-r1`; **STAGE51_INSTALLED 1/1 AF119→AE220**, original caller-clean arg=1 and native called once. **6,560** P1/P2 windows (3,280 each), seq 1..6560 continuous, all `preMask/postMask=1FFF`, all 13-field `changedMask=0000`. **31 HEALTH** lines, all missingObjects/dropped/overflow/mismatch/badCaller/badArg = 0. No errors.
- Stage50 r1 **6,558** windows (all unchanged, gateCandidate=0); Stage49 r2 **6,556** windows (all unchanged, staticGateCandidate=0); Stage47 **6,494** windows with **4,037** changedMask!=0; Stage48/46/44 each **1,783** native windows. Earlier hooks remain installed.
- Match by same side/object/QPC, restrict Stage47→Stage51 to same-frame gap <=5,000 QPC ticks: **6,494** exact matches; **435** Stage47 X POST O+7C differ from Stage51 PRE O+7C, **0** Y changes; Stage47 X POST equals Stage51 PRE anchor O+AC **6,494/6,494**. Stage51 POST = Stage50 PRE on all four raw X/Y/anchor fields **6,558/6,558**; Stage50 POST = Stage49 PRE **6,556/6,556**. The remaining 66 Stage51 tail rows do not have same-frame Stage47 samples and must not inflate the X/Y differences.
- Conclusion: tested native AE220 at AF119 does **not** directly change P1/P2 observed coordinates, while the **435** same-frame X discrepancies arise **before AF119** and after Stage47. Native sequence preceding AF119: **AF10D→AEF10**, **AF112→AED90**. Next **Stage52 source PENDING**: passive snapshots before/after both calls, verify original physical call signatures + ABI before any hook, no game-state writes. No inference of movement, combat hits/damage or smarter AI.
- Immutable evidence: [stage51-runtime-global-coordinate-evidence](https://github.com/Hawky11/1/tree/stage51-runtime-global-coordinate-evidence), [full report](https://github.com/Hawky11/1/blob/stage51-runtime-global-coordinate-evidence/Stage51_Runtime/RUNTIME_ANALYSIS.md), [6,494-row same-frame CSV](https://github.com/Hawky11/1/blob/stage51-runtime-global-coordinate-evidence/Stage51_Runtime/stage47_stage51_sameframe_join.csv). Original blobs frozen at HEAD derived from `0341856`. Keep Stage49 r2 / Stage50 r1 / Stage51 r1 source intact.
## 2026-11-10 — Scharlachrot behavior observation and Stage52 dual-native passive probes SOURCE READY

- User observed Scharlachrot, after Arcana activation, hover-dash toward opponent and cancel into a kick. **Uncontrolled anecdotal observation**, not evidence of enhanced AI, new action selection, or causal effect of any specific hook. The installed COMvsCOM DLL already forces both CPU control DWORDs to 1; diagnostic callsite hooks may affect timing but do not intentionally change attacks.
- Stage51 Windows x86 VERIFIED: 6,560 AF119→AE220 P1/P2 events, zero internal raw changes. Stage47→Stage51 same-frame join has 6,494 matches, 435 X-only divergences and 6,494 matches between prior Stage47 POST X and Stage51 anchor O+AC; Stage50/49 also inactive on tested P1/P2. Frozen Stage51 report/CSV already recorded above.
- New SHA-pinned original AALib.exe disassembly: `AF10D E8 FE FD FF FF → AEF10`, `AF112 E8 79 FC FF FF → AED90` (no stack args, both native RET C3), immediately before existing Stage51 AF119→AE220. **AED90 contains actual FPU stores** `AEE01 FST [ESI+7C]; AEE04 FSTP [ESI+AC]; AEE16 FST [ESI+80]; AEE1C FSTP [ESI+B0]`. These are object writes in some execution paths, **not yet attributed** to tracked P1/P2 or 435 drifts.
- **Stage52 SOURCE READY** branch [stage52-pre-global-dual-native-coordinate-boundary](https://github.com/Hawky11/1/tree/stage52-pre-global-dual-native-coordinate-boundary): [new passive observers](https://github.com/Hawky11/1/blob/stage52-pre-global-dual-native-coordinate-boundary/Stage52_Overlay/src/Stage52PreGlobalCoordinate.inl), [Stage38 adapter](https://github.com/Hawky11/1/blob/stage52-pre-global-dual-native-coordinate-boundary/Stage52_Overlay/src/Stage38CallbackWindow.inl), [static analysis](https://github.com/Hawky11/1/blob/stage52-pre-global-dual-native-coordinate-boundary/Stage52_Analysis/STATIC_ANALYSIS.md), [stdlib verifier](https://github.com/Hawky11/1/blob/stage52-pre-global-dual-native-coordinate-boundary/Stage52_Analysis/tools/verify_stage52.py), [RU install](https://github.com/Hawky11/1/blob/stage52-pre-global-dual-native-coordinate-boundary/Stage52_Overlay/README_STAGE52_RU.md). Two adjacent original CALLs hooked together only after Stage51 installation, both native originals invoked once with GPR/EFLAGS/FXSAVE restore, 13-field P1/P2 before/after snapshots, QPC, health, 8192 ring. No artificial inputs/AI decisions/coordinate writes. Previous Stage49 r2, Stage50 r1, Stage51 r1 retained.
- **Stage52 Windows x86 BUILD/GAME RUNTIME PENDING.** Do not assert successful compilation, hook install, improved animations, hits/damage, or AI quality. Only replace Stage38CallbackWindow.inl with Stage52 adapter and add Stage52PreGlobalCoordinate.inl; keep all others as installed. In runtime join Stage47→Stage52 AEF10→AED90→Stage51 by exact side/object/QPC, excluding stale tail rows. Compare controlled COMvsCOM baseline before attributing movement.
## 2026-11-10 — Stage52 Windows x86 VERIFIED, 868 pre-AEF10 X divergences; Stage53 source ready

- New user uploads: [7b760cf](https://github.com/Hawky11/1/commit/7b760cf270a27d16715e6e553e978acc3044a36f) and [944b3b8](https://github.com/Hawky11/1/commit/944b3b83a499995808ab34bcce69f8bec973498c). Windows x86 **BUILD SUCCEEDED** started 16:53:14, DLL **1,097,728 bytes**, nonfatal preexisting C4754 warning; game COMvsCOM active.
- Stage52 **STAGE52_INSTALLED 2/2** AF10D→AEF10 and AF112→AED90; full Git blob 6,546,113 chars, **12,748** P1/P2 rows (6,374 AEF10 + 6,374 AED90; 3,187 each side per native), seq 1..12748 contiguous, 13-field masks all 1FFF; **29 HEALTH**, all zero missing/dropped/overflow/mismatch/badCaller, no errors. **Only 2 changes** among all Stage52 windows: initial AEF10 P1/P2 **O+18 0→4**, no X/Y/anchor write in either routine for tracked objects; AED90 6,374 windows all unchanged.
- Stage47 **6,244** windows (4,317 intra-native changes). Same-frame side/object/QPC correlation **6,244 distinct** Stage47 POST→Stage52 AEF10 PRE (gap 367–1,162 QPC ticks) contains **868 X-only differences, zero Y changes**; Stage47 POST X equals Stage52 PRE anchor O+AC **6,244/6,244**. AEF10 POST→AED90 PRE **6,374/6,374** exact X/Y/anchors; AED90 POST→Stage51 PRE **6,370/6,370** exact; Stage51→Stage50 **6,368/6,368** and Stage50→Stage49 **6,366/6,366** exact. Stage51/50/49 raw fields all unchanged in the new run. Tail matches excluded rather than using stale Stage47 values.
- New immutable [Stage52 runtime evidence branch](https://github.com/Hawky11/1/tree/stage52-runtime-dual-native-evidence), [full runtime report](https://github.com/Hawky11/1/blob/stage52-runtime-dual-native-evidence/Stage52_Runtime/RUNTIME_ANALYSIS.md), [6,244-row CSV](https://github.com/Hawky11/1/blob/stage52-runtime-dual-native-evidence/Stage52_Runtime/stage47_stage52_stage51_sameframe_join.csv). The previously recorded 435 drift count refers to earlier Stage51 run, not to this Stage52 session.
- Original SHA-verified AALib.exe reveals one direct parent callsite **A2B90 E8 0B C5 00 00→AF0A0**, directly after A2B81→A1B80, A2B86→CFB80, A2B8B→BAB60, before A2B95→CFA90. AF0A0 contains both Stage52 natives. AF0A0 no-stack-arg original entry bytes `55 8B EC B8 6C 14 00 00 E8 23 C4 15 00`, native epilogue AF34C `5F 5E 5B 8B E5 5D C3`. Exact call rel32, original bytes and SHA all checked.
- **Stage53 source READY** [stage53-parent-af0a0-coordinate-boundary](https://github.com/Hawky11/1/tree/stage53-parent-af0a0-coordinate-boundary): [one passive native-once AF0A0 parent CALL wrapper](https://github.com/Hawky11/1/blob/stage53-parent-af0a0-coordinate-boundary/Stage53_Overlay/src/Stage53ParentCoordinate.inl), [Stage38 adapter](https://github.com/Hawky11/1/blob/stage53-parent-af0a0-coordinate-boundary/Stage53_Overlay/src/Stage38CallbackWindow.inl), [original PE verifier](https://github.com/Hawky11/1/blob/stage53-parent-af0a0-coordinate-boundary/Stage53_Analysis/tools/verify_stage53.py), [static analysis](https://github.com/Hawky11/1/blob/stage53-parent-af0a0-coordinate-boundary/Stage53_Analysis/STATIC_ANALYSIS.md), [RU installation guide](https://github.com/Hawky11/1/blob/stage53-parent-af0a0-coordinate-boundary/Stage53_Overlay/README_STAGE53_RU.md). No gameplay writes, preserves Stage52/51/50/49 and installed earlier. Compare Stage47→Stage53 PRE, Stage53 PRE→Stage52 AEF10 PRE to isolate X divergence before or inside AF0A0 early region. **Stage53 Windows x86 BUILD and runtime PENDING**. No evidence yet of better combat AI/animations/hits from raw data.


## 2026-11-10 — Stage53 Windows x86 runtime VERIFIED; Stage54 pre-parent three-sibling source READY

- New user commits [ac8b4ba](https://github.com/Hawky11/1/commit/ac8b4baca7479be511a9137af6b1cffbdac24998), [2bb84f0](https://github.com/Hawky11/1/commit/2bb84f00f6f8702ba8334a9958ae41f0fa381a74). Build.log confirms **BUILD SUCCEEDED** (17:18:05) 1,105,408-byte x86 DLL, old C4754 only, COMvsCOM active. **STAGE53_INSTALLED 1/1 A2B90→AF0A0**.
- Stage53 log **6,024 P1/P2 windows** (3,012 per side), full pre/post masks, 28 HEALTH all missing/dropped/overflow/mismatch/badCaller=0. **6,022 unchanged** and only the initial two P1/P2 `O+18 0→4` changes; **no raw X/Y/anchor writes** by AF0A0 in sampled tracked windows.
- Same run Stage47 5,904 windows. Side/object/strict QPC join Stage47 POST→Stage53 PRE **5,904/5,904** (gap 355–1742 ticks), **1,091 live X differences** (403 P1, 688 P2), **zero Y differences**; Stage47 POST X equals Stage53 PRE anchor X **5,904/5,904**. Stage53 PRE→Stage52 AEF10 PRE 6,024 matches, zero X/Y difference. Remaining 120 Stage53 tail rows lack Stage47 and were excluded. Earlier Stage52's 868 drift rows are from a different run and must not be merged.
- Exact source and original log blob SHAs frozen in [stage53-runtime-parent-af0a0-evidence](https://github.com/Hawky11/1/tree/stage53-runtime-parent-af0a0-evidence), [RUNTIME_ANALYSIS.md](https://github.com/Hawky11/1/blob/stage53-runtime-parent-af0a0-evidence/Stage53_Runtime/RUNTIME_ANALYSIS.md). Root `main` logs remain unchanged.
- Verified original AALib.exe physical sequence before AF0A0: **A2B81→A1B80**, **A2B86→CFB80**, **A2B8B→BAB60**, followed by Stage53 A2B90. [Stage54 source branch](https://github.com/Hawky11/1/tree/stage54-pre-parent-three-call-boundary), [new 3-site passive wrappers](https://github.com/Hawky11/1/blob/stage54-pre-parent-three-call-boundary/Stage54_Overlay/src/Stage54PreParentCoordinate.inl), [2-file install guide](https://github.com/Hawky11/1/blob/stage54-pre-parent-three-call-boundary/Stage54_Overlay/README_STAGE54_RU.md), verifier/static explanation. All native originals invoked once; Stage53–49 preserved; no new AI commands/coordinate writes. **Stage54 Windows x86 BUILD/runtime PENDING**, no claim of improved animations, hits, damage or AI quality.

## 2026-11-10 — Stage54 r1 x86 build SUCCESS, runtime hook blocked by ASLR signature validation; r2 source READY

- New user log commits [bd0716d](https://github.com/Hawky11/1/commit/bd0716d9c270c7f4ca19f8051f26b7e448bb5541), [47675c5](https://github.com/Hawky11/1/commit/47675c5a6262acf9ed470fc443192746a265b8b6). **BUILD SUCCEEDED** at 18:13:10, x86 `dinput8.dll` 1,113,088 bytes, existing C4754 warning only. Stage54 log reports marker r1 then **ERROR: original sibling CALL/signature mismatch; no patches** at 18:13:36; **Stage54 not installed**. Stage53 3,918 windows, 18 HEALTH zero-loss, only initial two O+18 writes and zero X/Y within AF0A0. Stage47 3,848 windows, Stage52 7,832 boundary rows. No successful Stage54 coordinate sample.
- Independent original `AALib.exe` SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` PE32 check: verified relocation-table entry **RVA A1B82 type 3 HIGHLOW**, within native A1B80 original bytes `80 3D 5C 1B 0A 0E 00` (original abs operand `0x0E0A1B5C`). Actual load base `0x00800000` shifts operand to `0x0E4A1B5C`, incompatible with r1's literal seven-byte comparison. Original static CALL bytes and destinations A2B81/A2B86/A2B8B all verified. Since r1 short-circuited on first mismatch, other runtime comparisons were not individually observed.
- [Frozen Stage54 r1 failure evidence](https://github.com/Hawky11/1/tree/stage54-r1-signature-mismatch-runtime-evidence) with exact uploaded root log blob SHAs and [analysis](https://github.com/Hawky11/1/blob/stage54-r1-signature-mismatch-runtime-evidence/Stage54_Runtime/R1_INSTALL_FAILURE_ANALYSIS.md). Preserve immutable.
- New [Stage54 r2 source](https://github.com/Hawky11/1/tree/stage54r2-relocation-aware-sibling-boundaries) [relocation-aware inl](https://github.com/Hawky11/1/blob/stage54r2-relocation-aware-sibling-boundaries/Stage54_Overlay/src/Stage54PreParentCoordinate.inl), [verifier](https://github.com/Hawky11/1/blob/stage54r2-relocation-aware-sibling-boundaries/Stage54_Analysis/tools/verify_stage54.py), [RU guide](https://github.com/Hawky11/1/blob/stage54r2-relocation-aware-sibling-boundaries/Stage54_Overlay/README_STAGE54_RU.md). Compensates absolute operand by loaded base delta while retaining strict original three CALL-site/target checks, plus status logs. All original functions called once, no coordinate or game AI writes. Python independent original PE relocation/signature tests PASS at three simulated image bases. **Stage54 r2 Windows x86 build/runtime PENDING**; replace only existing Stage54PreParentCoordinate.inl, keep installed r1 adapter and previous stages untouched.


## Stage54 r2 runtime verified — 2026-11-10

Evidence: main upload commit a49661be904b7ef6427b8a885a852089f798801e (logs remain available at that immutable commit). Stage54 blob 413af4919f723351edfcafb7fb397514016ea479; Build.log blob 741cbe661dcba1e54d0ce46822838c19f1e28a25.

Build started 18:27:30 local log time, Windows x86 BUILD SUCCEEDED, DLL 1,113,600 bytes; existing C4754 warning only. Runtime r2 marker and STAGE54_INSTALLED 3/3 confirmed. ASLR check: load base 00800000, expected and actual relocated operand both 0E4A1B5C, readable=1.

Stage54: 19,734 unique contiguous rows (seq 1..19734), 6,578 per callsite A2B81/A2B86/A2B8B. All pre/post masks 1FFF and all changedMask=0000. 29 HEALTH records: missingObjects/dropped/overflow/mismatch/badCaller all zero. Thus no observed changes to any of the 13 watched DWORDs inside these three native calls in this run.

Stage47: 6,500 unique contiguous rows, full masks; 29 HEALTH with zero listed errors. Stage53: 6,572 unique contiguous rows, full masks; 29 HEALTH with zero listed errors. Only two initial Stage53 f7 O+18 changes 0→4; no X/Y change inside AF0A0.

Correlation method: first subsequent PRE after POST, same side/object/thread, QPC gap <=5,000 ticks; target sequence uniqueness checked.
- Stage47 POST → first Stage54 PRE: 6,500/6,500 distinct matches; gaps 252–1,726 ticks. Raw X differs in 504 (P1 266, P2 238); Y differs in zero. Stage47 POST X equals Stage54 PRE anchor O+AC in all 6,500.
- A1B80 POST → CFB80 PRE: 6,578 distinct matches, gaps 3–121; X/Y/AC/B0 identical.
- CFB80 POST → BAB60 PRE: 6,578 distinct matches, gaps 3–221; X/Y/AC/B0 identical.
- BAB60 POST → Stage53 PRE: 6,572 distinct matches, gaps 4–401; X/Y/AC/B0 identical.
- Exclude 78 first-sibling rows without a Stage47 partner and six terminal sibling rows without a Stage53 partner; do not join stale frames.

Conclusion: the observed X change occurs after the Stage47 snapshot and before A2B81→A1B80, not inside the three Stage54 siblings or Stage53 AF0A0. This does not identify an exact writer or establish visual/AI improvement.

Next: inspect original SHA-pinned AALib.exe control flow between Stage47 return sites AE711/AE943 and A2B81, identify the enclosing earlier routine/boundary, then prepare a focused passive Stage55 observer after verifying signatures, relocations and ABI. Stage55 source is NOT prepared in this update. Keep the working Stage54 r2 sources and DLL. No rebuild required for this analysis.


## Current — Stage55 earlier-motion boundaries SOURCE READY (2026-11-10)

Stage54 r2 runtime is verified: 19,734 unchanged sibling windows, 6,500 distinct Stage47→Stage54 pairs with 504 X differences already before A2B81. See [Stage54 report](https://github.com/Hawky11/1/blob/main/Stage54_Runtime/R2_RUNTIME_ANALYSIS.md).

**Stage55 source:** [stage55-earlier-motion-boundary](https://github.com/Hawky11/1/tree/stage55-earlier-motion-boundary), based on verified Stage54 r2 source commit 4661af49287e106cb9f9d911f63829dbd5f2fae4. [Installation](https://github.com/Hawky11/1/blob/stage55-earlier-motion-boundary/Stage55_Overlay/README_STAGE55_RU.md), [static analysis and validation limits](https://github.com/Hawky11/1/blob/stage55-earlier-motion-boundary/Stage55_Analysis/STATIC_ANALYSIS.md).

Three original-once no-stack-argument CALL boundaries: **A119B→AE6E0**, **A11A0→ACDF0**, **A11A5→AC8A0**. AE6E0 contains Stage47 AE70C. ACDF0 has verified native X stores at ACF2A/ACFA3 (FSTP [EAX+7C]), making it a candidate; runtime attribution is not yet established. Stage47 AE93E belongs to AE880, not AE6E0; do not force its containment. ABCB0 and later corridor remain outside Stage55.

Checks passed: exact original EXE SHA/PE, six full-instruction signatures with no HIGHLOW relocation overlap, decoded calls, reachable CFG with plain RET and no EBP stack argument access; exact narrow adapter change; **384 synthetic assembly differential tests** (128 per wrapper). x87 instruction/data-pointer metadata excluded from emulator comparison. **Windows MSVC build/link and game runtime PENDING.** This is passive diagnostics, not a demonstrated AI improvement.

**Next user action:** replace ONLY src/Stage38CallbackWindow.inl from Stage55_Overlay and ADD src/Stage55EarlierMotionCoordinate.inl. Keep working Stage54PreParentCoordinate.inl r2 and all prior source/INI/Build.bat. Build; install DLL only after BUILD SUCCEEDED. Natural COM-vs-COM 30–60 seconds, allow flush, upload Build.log, ArcanaAI_Stage55EarlierMotionCoordinateTrace.log and same-run Stage47–54 logs.

**Next analyst:** require Stage55 3/3, Stage54 3/3, full masks, unique seq, zero missingObjects/dropped/overflow/mismatch/badCaller. Match Stage47 AE70C by same side/object/thread and containment inside AE6E0; compare last contained Stage47 POST with parent POST, report multiplicity. Then join parent POST→ACDF0 PRE/POST→AC8A0 PRE/POST→Stage54 first PRE using strict QPC and unique targets, excluding stale/unmatched tail. A whole-call X delta does not prove a specific store or visible movement.


## 2026-11-10 — Stage55 same-run ACDF0 whole-function X attribution VERIFIED

- Immutable original evidence [upload c27b6b5](https://github.com/Hawky11/1/commit/c27b6b516edcea4551de9e52e6a151acdc115279), tested Stage55 source [00614967](https://github.com/Hawky11/1/commit/00614967dbc386822d75b4bb37954d4f6600a439). [Published runtime report](https://github.com/Hawky11/1/blob/main/Stage55_Runtime/RUNTIME_ANALYSIS.md); [frozen branch](https://github.com/Hawky11/1/tree/stage55-runtime-acdf0-verified-evidence). Windows x86 Build.log started 18:51:03 local, BUILD SUCCEEDED, 1,122,816-byte DLL, only existing C4754. Stage55 3/3, Stage54/53/52/51/50/49/47 hooks retained.
- Capture integrity: Stage55 19,764 unique contiguous rows 1..19764, 9,882 per side and 6,588 per each A119B/A11A0/A11A5 call. Stage47 6,576 AE70C rows; Stage49 6,638, Stage50 6,640, Stage51 6,642, Stage52 13,292, Stage53 6,646, Stage54 19,944. All monitored masks complete (Stage49/50 07FF, other listed stages 1FFF), 29 health records per reviewed stage, zero missingObjects/dropped/overflow/mismatch/badCaller. String scan for ERROR/FAILED/FATAL found no root log matches, not an exhaustive semantics proof.
- Within-call Stage55: AE6E0 2,722 X and 2,816 Y differences from nested motion; **ACDF0 480 X (P1 248, P2 232), no Y, only f1 O+7C changed**, anchor X unchanged; AC8A0 all 13 fields unchanged. Stage54 19,944 windows unchanged. Stage53/52 AEF10 show four initial/reinitialization O+18 0→4 transitions; no X/Y changes; Stage49–51 and Stage52 AED90 watched fields unchanged.
- Strict same side/player/object/thread and QPC interval nesting matches 6,576 Stage47 windows inside exactly one AE6E0 parent; Stage47 POST X/Y equals enclosing parent POST in 6,576/6,576. Adjacent parent POST→ACDF0 PRE, ACDF0 POST→AC8A0 PRE, AC8A0 POST→Stage54 PRE each yield 6,588 unique matches and raw X/Y/anchor equality. 6,576 complete Stage47→Stage54 chains contain **480 X and zero Y differences**, **all isolated to ACDF0**. Excluded 12 unmatched parent chains and 60 Stage54 no-predecessor tail rows. Prior Stage54-only 504 X changes were from a *different capture*.
- **Only ACDF0 whole-function attribution is proven**. ACF2A/ACFA3 and called helper paths are candidates; no individual writer, collision semantic, visible movement or better AI inferred. The report records the prior analyst's correlation. The complete Python analyzer and output CSV are not yet independently re-executed/published with this report; retain evidence original bytes.

## 2026-11-10 — Stage56 two ACDF0 native writer observers SOURCE READY (runtime pending)

- [Stage56 branch](https://github.com/Hawky11/1/tree/stage56-acdf0-two-fstp-writer-observer) based on source commit 00614967. The private original AALib.exe 175,556,608 bytes and SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` were locally reverified. GNU objdump disassembly: ACF27 `D8 40 7C` FADD [EAX+7C] + ACF2A `D9 58 7C` FSTP [EAX+7C]; ACFA0 / ACFA3 same; exact next instructions ACF2D and ACFA6. Each pair is a six-byte, instruction-aligned detour site. Both surrounding 16-byte original signatures verified, no PE HIGHLOW relocation overlaps.
- Two passive naked JMP+NOP observers execute original FADD/FSTP pairs exactly once with original native ESP and preserve GPR/EFLAGS/FXSAVE around C++ read-only observers; record actual object pointer, P1/P2 match, raw X before/after, thread, QPC and health. The new [Stage56 source](https://github.com/Hawky11/1/blob/stage56-acdf0-two-fstp-writer-observer/Stage56_Overlay/src/Stage56ACDF0Writer.inl) and [3-edit Stage38 adapter](https://github.com/Hawky11/1/blob/stage56-acdf0-two-fstp-writer-observer/Stage56_Overlay/src/Stage38CallbackWindow.inl) leave all prior stage source untouched. [Static rationale](https://github.com/Hawky11/1/blob/stage56-acdf0-two-fstp-writer-observer/Stage56_Analysis/STATIC_ANALYSIS.md), [verifier](https://github.com/Hawky11/1/blob/stage56-acdf0-two-fstp-writer-observer/Stage56_Analysis/tools/verify_stage56.py), [Russian installation guide](https://github.com/Hawky11/1/blob/stage56-acdf0-two-fstp-writer-observer/Stage56_Overlay/README_STAGE56_RU.md).
- **Windows MSVC x86 compile/link and game runtime NOT YET TESTED.** Static signature/relocation checks and exact adapter textual comparison were inspected, but ABI semantic equivalence under the live binary, exceptional x87 metadata and timing remain unverified. Keep Stage55 DLL for rollback.
- **Next:** user installs only new Stage38 adapter plus Stage56ACDF0Writer.inl, keeps Stage55/54 r2/earlier source and config, runs Build.bat, natural COM-vs-COM, uploads Build.log, Stage56 writer log and Stage55/47/54 same-run logs. Correlate writer-level raw X changes with Stage55 ACDF0 intervals; if stores cannot explain all changes, inspect nested helpers. No AI quality claim.


## 2026-11-10 — Stage56 x87 direct writer runtime VERIFIED; Stage57 source READY

- New same-run evidence uploads [23d451f](https://github.com/Hawky11/1/commit/23d451f6234b89f184d8ec17612b07ff49c4f558), [9307fc2](https://github.com/Hawky11/1/commit/9307fc20a2481a7e40b0374829ac52df9ed8cd78), final snapshot [0a60de5](https://github.com/Hawky11/1/commit/0a60de5d576efedb763daef9d1b1be4932cb8384). Source [Stage56 observer](https://github.com/Hawky11/1/tree/stage56-acdf0-two-fstp-writer-observer); [published detailed runtime report](https://github.com/Hawky11/1/blob/main/Stage56_Runtime/RUNTIME_ANALYSIS.md), [immutable evidence branch](https://github.com/Hawky11/1/tree/stage56-runtime-acdf0-writer-evidence), with [full 6,216 ACDF0 window CSV](https://github.com/Hawky11/1/blob/stage56-runtime-acdf0-writer-evidence/Stage56_Runtime/stage56_stage55_writer_join.csv) and summary.json. Windows x86 BUILD SUCCEEDED started 19:55:55 local, DLL **1,127,936 bytes**, existing C4754 warning only. Stage56 installed 2/2; Stage55 3/3, Stage54 3/3 and Stage47 present.
- Integrity: Stage47 6,194 unique contiguous AE70C windows; Stage54 18,990 windows (6,330 per sibling); Stage55 18,648 windows (6,216 each native); all 1FFF masks, 28 zero-error HEALTH each. Stage56 593 unique contiguous P1/P2 events (P1 231, P2 362), 21 health records, total `calls=593 rows=593 otherObjects=0 dropped=0 overflow=0 mismatch=0 readFail=0`. No error/failure/fatal strings in reviewed runtime logs.
- Strict same side/player/object/thread QPC joins: **6,194/6,194** nested Stage47→AE6E0 windows with equal nested and parent POST X/Y. AE6E0 POST→ACDF0 PRE and ACDF0 POST→AC8A0 PRE both 6,216 matches with raw X/Y/anchors equal, AC8A0 POST→first Stage54 PRE 6,210 matches with 6 end-tail unpaired. Stage47 POST→first Stage54 PRE **6,194 unique** matches with **1,337 raw X differences (P1 741/P2 596), zero Y**, and Stage47 POST X = Stage54 anchor X in all 6,194. **Exactly 1,337 ACDF0 whole-function native windows change X**; Stage54 and AC8A0 unchanged. All these numbers are the *new capture*, not Stage55's prior 480 or Stage54's prior 504.
- **Stage56 writer-level finding:** ACF27/ACF2A `FADD/FSTP [EAX+7C]` observed **593** times for tracked P1/P2, **272 raw X changes** (P1 69/P2 203). ACFA0/ACFA3 unobserved. All 593 events nested uniquely in one ACDF0 window. There were **225 changed ACDF0 windows explained entirely by before/after writer X**, **64 changed native windows with equal PRE to writer PRE but different POST from writer POST** (an additional *later* X update), and **1,048 changed ACDF0 windows without observed Stage56 writer event**. Thus ACF2A is proven an exact P1/P2 X writer, but not the complete explanation of ACDF0 X changes; no evidence of improved combat AI or collision.
- Static original SHA-pinned AALib.exe inspection: **ACE11→A2C40** early call and **ACFD6→15CEC0**, **ACFDB→15B8A0** final calls inside ACDF0. On-disk CALL byte signatures and callee prologues verified, original no-stack-arg plain RET targets, no overlaps with PE relocations (212,051 entries scanned).
- New [Stage57 source branch](https://github.com/Hawky11/1/tree/stage57-acdf0-tail-native-boundaries) adds passive original-once 3-site Stage57 snapshot logger and narrow Stage38 adapter, [static report](https://github.com/Hawky11/1/blob/stage57-acdf0-tail-native-boundaries/Stage57_Analysis/STATIC_ANALYSIS.md), [RU two-file install](https://github.com/Hawky11/1/blob/stage57-acdf0-tail-native-boundaries/Stage57_Overlay/README_STAGE57_RU.md) and [SHA-pinned verifier](https://github.com/Hawky11/1/blob/stage57-acdf0-tail-native-boundaries/Stage57_Analysis/tools/verify_stage57.py). **Windows MSVC x86 build and game runtime PENDING; Stage56 DLL is rollback baseline.** Next: user build, Stage57 log + Stage56/55/47/54 same-run, nested QPC isolation of early/final helper X changes.

## 2026-11-10 — Stage57 runtime VERIFIED; Stage58 candidate conditional native-call source READY

- User uploaded four fresh **same-session** logs on `main` at [commit `31f83c31677e9a3f1ff891d24ed8dd223c7f2a54`](https://github.com/Hawky11/1/commit/31f83c31677e9a3f1ff891d24ed8dd223c7f2a54); preserved exact raw upload on [`stage57-runtime-acdf0-verified-evidence`](https://github.com/Hawky11/1/tree/stage57-runtime-acdf0-verified-evidence). Published [complete Stage57 runtime analysis](https://github.com/Hawky11/1/blob/main/Stage57_Runtime/RUNTIME_ANALYSIS.md).
- Stage57 Windows x86 **BUILD SUCCEEDED**, DLL **1,137,664 bytes** (20:23:06 local); Stage55 installed 3/3, Stage56 2/2, Stage57 3/3; zero dropped, overflow, mismatch, missingObjects, badCaller, readFail in reviewed health.
- Stage55: 5,498 ACDF0 outer windows with **744 raw X** differences, zero Y. Stage57: 5,504 per helper site, **5,498 exact same-run triple helper windows** contained by Stage55. **18 final Stage57 rows** after last Stage55 logged parent excluded; not counted as matched, not asserted as dropped. Stage56: **359/359** writer observations nested unambiguously, all in pre-tail gap.
- Exact partition: **327** gap differences ACE11 POST→ACFD6 PRE (P1 121/P2 206) plus **420** changes inside **ACFD6→15CEC0** (P1 271/P2 149), with **three** P1 windows in both = **744 unique changed windows**. Zero X changes in ACE11 native, ACFDB native, pre-ACE11 and post-ACFDB boundary deltas. Stage56 **112** changed ACF2A direct writes (all P1) explain 112 of 327 gap differences. **215** pre-tail changes remain without a Stage56 writer event (**P1 9/P2 206**). All 359 direct-site events nested in early-to-tail gap, full raw-state continuity, no invented instruction attribution.
- Independently inspected SHA-pinned original private AALib.exe (175,556,608 bytes; SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`): three conditional original CALLs `ACF0F→B86A0`, `ACF7A→B86A0`, `ACF8A→E9A80`; verified exact 5-byte rel32 instructions and target prologues; x86 CDECL two stack args vs THISCALL one stack arg/RET4; **212,051 PE HIGHLOW relocations scanned, zero overlap** with five checked regions.
- Created [Stage58 source branch `stage58-acdf0-pre-tail-conditional-calls`](https://github.com/Hawky11/1/tree/stage58-acdf0-pre-tail-conditional-calls): [passive observer](https://github.com/Hawky11/1/blob/stage58-acdf0-pre-tail-conditional-calls/Stage58_Overlay/src/Stage58ACDF0ConditionalCalls.inl), [narrow Stage38 adapter](https://github.com/Hawky11/1/blob/stage58-acdf0-pre-tail-conditional-calls/Stage58_Overlay/src/Stage38CallbackWindow.inl) preserving Stage55–57, [static report](https://github.com/Hawky11/1/blob/stage58-acdf0-pre-tail-conditional-calls/Stage58_Analysis/STATIC_ANALYSIS.md), [read-only Python verifier](https://github.com/Hawky11/1/blob/stage58-acdf0-pre-tail-conditional-calls/Stage58_Analysis/tools/verify_stage58.py), [Russian two-file guide](https://github.com/Hawky11/1/blob/stage58-acdf0-pre-tail-conditional-calls/Stage58_Overlay/README_STAGE58_RU.md). The added source passes exact textual 3-edit adapter comparison, but **Windows MSVC compilation and actual game runtime NOT TESTED**. Last-working DLL is Stage57, 1,137,664 bytes. Next step: user build and fresh Stage55/56/57/58 logs; match same-run callsite X changes to 215 unresolved pre-tail differences.
- **No demonstrated AI/gameplay improvement**, animation, hit/damage or collision causality. These are passive diagnostic coordinate observations. Never publish private AALib.exe itself.


## 2026-11-10 — Stage58 conditional native probes runtime VERIFIED; Stage59 write attribution queued

- Original five-log upload [5c90de3](https://github.com/Hawky11/1/commit/5c90de3d792f366a6ffd910187859b8b0ce020b1), frozen snapshot [stage58-runtime-conditional-no-x-evidence](https://github.com/Hawky11/1/tree/stage58-runtime-conditional-no-x-evidence); [full report](https://github.com/Hawky11/1/blob/main/Stage58_Runtime/RUNTIME_ANALYSIS.md). Stage58 source branch unchanged. Windows x86 Build.log 20:54:37: **BUILD SUCCEEDED**, 1,145,344-byte DLL, only C4754. Stage55 3/3, Stage56 2/2, Stage57 3/3, Stage58 3/3 installed; zero loss/mismatch/health errors.
- Complete strict same-run Stage55 ACDF0 (5,538) to Stage57 triple helper nesting (5,538 each), full masks, QPC and object continuity: **1,179** whole-ACDF0 raw X changes, no Y; **776** pre-tail, **434** inside 15CEC0, **31** both. Stage58 822 rows / 411 ACF0F native calls; zero X/other-field changes; EAX=1 on 31 physical calls. Other two installed Stage58 conditional sites unobserved. 776 pre-tail changed windows split 168 before Stage58, 155 after Stage58, 453 without Stage58. Stage56 380 tracked ACF2A events all unchanged, ACFA3 unobserved. Do not combine prior Stage57/56 captures.
- Original SHA-pinned AALib.exe disassembly confirms two visible inline FSTP X writers in ACDF0, but live changes are **not attributed** to them in this capture. Full callee tree, other threads and existing hooks are not excluded. **No new gameplay AI action or quality improvement proved**.
- Extra Stage58→Stage56 conditional join: 411 paired calls of B86A0; **380 EAX=0** each uniquely lead to one ACF2A direct write 2–83 QPC ticks later; **31 EAX=1** each lead to zero ACF2A events before the next call. The original ACF17 TEST / ACF19 JNE supports the branch mechanism, but all 380 watched writes leave X unchanged in this capture.
- Next Stage59 **design only**: inspect Stage16 ownership of DR0/DR1 and current runtime mode, then plan a non-conflicting P1/P2 object+7C write watchpoint with exact EIP/QPC/side/health; use narrower per-loop windows if DR unavailable. No Stage59 source, Windows build or runtime claimed.


## 2026-11-10 — Stage59 conditional DR0/DR1 dynamic X writer source prepared (runtime PENDING)

- Previous [Stage58 verified runtime](https://github.com/Hawky11/1/blob/main/Stage58_Runtime/RUNTIME_ANALYSIS.md) still attributes 1,179 whole-ACDF0 raw X changes to 776 pre-tail gap and 434 inside 15CEC0, overlap 31. Stage58 confirms 380 B86A0 EAX=0 calls uniquely followed by ACF2A store, 31 EAX=1 calls skipping ACF2A; no direct-store X value change in that same-run Stage58 capture.
- Created [Stage59 branch](https://github.com/Hawky11/1/tree/stage59-dynamic-x-write-watchpoint) based on unchanged Stage58 source, with [two-file overlay](https://github.com/Hawky11/1/tree/stage59-dynamic-x-write-watchpoint/Stage59_Overlay), [safety rationale](https://github.com/Hawky11/1/blob/stage59-dynamic-x-write-watchpoint/Stage59_Analysis/STATIC_ANALYSIS.md), [static verifier](https://github.com/Hawky11/1/blob/stage59-dynamic-x-write-watchpoint/Stage59_Analysis/tools/verify_stage59.py) and [RU installation](https://github.com/Hawky11/1/blob/stage59-dynamic-x-write-watchpoint/Stage59_Overlay/README_STAGE59_RU.md). Stage38 adapter diff inspected: exactly one added include and Stage58-success guarded worker start; Stage55–58 source unchanged.
- New Stage59 uses P1/P2 dynamically dereferenced object+0x7C DWORD write-only hardware watchpoints on DR0/DR1; refuses to run with Stage16 control/bit17 modes enabled, skips foreign register owners, preserves DR2/DR3, stores post-write EIP/QPC/actual raw X and best-effort old shadow in ring, flushes outside VEH. Stops/disarms on object pointer drift. Diagnostic only; no intended AI policy or coordinate writes.
- Original private AALib.exe local fingerprint rechecked 175,556,608 bytes SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba; verified ACF0F B86A0 call, ACF17/ACF19 branch and ACF2A/ACFA3 FSTP bytes. SHA-pinned verifier published but no verified Windows x86 compilation, runtime coverage, VEH correctness or AI quality effect yet. Keep Stage58 DLL 1,145,344 bytes for rollback.


## 2026-11-10 — Stage59 r1 Windows BUILD VERIFIED; watchpoints did not arm; Stage59 r2 startup fix

- [Original six-file capture](https://github.com/Hawky11/1/commit/7eb07e6bff9392708720cd3ff76ac713703af1c9); [frozen evidence branch](https://github.com/Hawky11/1/tree/stage59-r1-runtime-timeout-evidence); [full analysis](https://github.com/Hawky11/1/blob/main/Stage59_Runtime/R1_STARTUP_ANALYSIS.md). Stage59 r1 Windows x86 Build.log started 21:53:04: **BUILD SUCCEEDED**, DLL **1,152,512 bytes**; Stage55/56/57/58 installation markers all correct. Stage59 r1 worker 21:53:34 → **ERROR 21:53:39** waiting for readable P1/P2 object pointers; **NO DR0/DR1 registers touched**, no armed watchpoints or X_WRITE records.
- Stage55 first valid objects **21:53:53** (P1 0E4A1F78→2A710020, P2 0E4CCCA0→2A710C50), Stage57 first at 21:53:54, Stage58/Stage56 first at 21:53:55. Stage59 r1's 5s wait thus expired 14s before first observed object. Stage55 12,606 rows, Stage57 12,624, Stage56 176, Stage58 414; health failures zero. 368 Stage55/Stage57 whole-ACDF0/tail X changes in this run; **no data about their direct writer from Stage59**.
- Separate [Stage59 r2 source branch](https://github.com/Hawky11/1/tree/stage59-r2-wait-for-combat-objects) extends ONLY Stage59 watchpoint worker's object startup wait to **600s** and adds periodic waiting + readiness diagnostics, r2 marker. Existing Stage59 adapter and older stages unchanged, no new gameplay patches. [One-file update instructions](https://github.com/Hawky11/1/blob/stage59-r2-wait-for-combat-objects/Stage59_Overlay/README_STAGE59_RU.md). **R2 Windows build and actual watchpoint installation are pending.** Keep baseline DLLs for rollback; do not infer AI improvement.

---

## Stage59 r2 — direct P1/P2 X write attribution VERIFIED (2026-11-10)

**Provenance:** [six same-session files uploaded at `d2ce2444`](https://github.com/Hawky11/1/commit/d2ce2444b569e439db66e076f8428ac4dfc611cc), [frozen raw evidence branch](https://github.com/Hawky11/1/tree/stage59-r2-runtime-writer-attribution-evidence), [exact analysis](https://github.com/Hawky11/1/blob/main/Stage59_Runtime/R2_RUNTIME_ANALYSIS.md), [source branch](https://github.com/Hawky11/1/tree/stage59-r2-wait-for-combat-objects). Stage59 r1 had built successfully but timed out before COM-vs-COM objects existed; r2 extends startup wait, preserving earlier hooks. This stage does not change AI behavior.

Windows x86 Build.log on 2026-11-10 22:07:10 **BUILD SUCCEEDED**, 1,153,024-byte `dinput8.dll`, old C4754 only. Runtime 22:07:43–22:08:23. `STAGE59_OBJECTS_READY waitedMs=12750`, `WATCH_ARMED`, 62 `THREAD_INSTALLED` markers; final 61 owned threads. `STAGE59_X_WRITE` seq **1–16121** continuous (one observed writer TID), no dropped/readFail/ownershipConflicts/installFailures/teardownFailures/objectDrift. Stage55/56/57/58 installed 3/3, 2/2, 3/3, 3/3. All inspected HEALTH counters zero and masks complete.

Single-run row counts: Stage55 **15,966** (5,322 per site), Stage57 **15,984** (5,328 per site), Stage56 **270** (`ACF2A` only, 50 changed raw X), Stage58 **664** (`ACF0F` only, all helper own-window X unchanged), Stage59 **16,121** (2,718 shadow differences). Stage58 had 332 B86A0 physical calls: all 270 EAX=0 had exactly one following Stage56 ACF2A event 4–77 QPC ticks later, all 62 EAX=1 had none.

Strict side/player/object/thread/QPC matching gives **5,322** complete Stage55 ACDF0 windows containing one each of Stage57 ACE11/ACFD6/ACFDB. Stage55 early AE6E0: **1,930 net X differences**, all 1,930 covered by Stage59 post-write `1E70DE`; original store `1E70DB: FSTP DWORD PTR [ESI+0x7C]`. Stage55 ACDF0: **719 net X differences** (P1 **485**, P2 **234**), every changed parent covered by matching Stage59 write with matching first PRE/last POST raw X. Of these, **429 pre-tail-gap** changes comprise **379** at after-RVA `ACC5A` (native `ACC57: FSTP DWORD PTR [EBX+0x7C]`) and **50** detoured `ACF2A` changes, while **290** belong to native helper `15CEC0` (145 each after-RVA `126A9D` and `126B4C`, original FSTP stores `126A9A`/`126B49`). No overlap of changing phases in this specific run. Stage57 logs four additional ACFD6 changed samples beyond complete Stage55 parents; excluded, not counted as 294/719. Stage55 AC8A0 itself does not change tracked raw X.

All 288 Stage59 outside-AALib EIP `78E2FEF2` hits include 50 shadow changes; **270** uniquely nest in Stage56 writer PRE/POST windows, including all **50** changed Stage56 observations. Remaining 18 are beyond logged Stage56 windows and do not justify module attribution. The post-write EIP is not the writer RVA; independent local disassembly of private SHA-pinned original AALib.exe verified instruction addresses, including `1E70DB`, `ACC57`, `126A9A`, `126B49`, and auxiliary original stores.

**Conclusion:** Stage59 r2 resolves original writer attribution for all observed Stage55 early and ACDF0 net-X changes in this short COM-vs-COM session, with no lost watchpoint data indicated. It does **not** establish smarter AI, action-selection change, collision/damage change or causal hover-dash effect. Preserve raw evidence and r2 DLL. **Stage60 next:** static call graph and operand provenance upstream of those original X stores; do not patch gameplay state before linking observed writes to AI decisions and testing behavioral effects separately.

---

## Stage60 — SHA-pinned original X-store caller and operand provenance (2026-11-10)

**Static-only branch:** [stage60-original-x-write-dataflow-static-analysis](https://github.com/Hawky11/1/tree/stage60-original-x-write-dataflow-static-analysis). [Full report](https://github.com/Hawky11/1/blob/stage60-original-x-write-dataflow-static-analysis/Stage60_Analysis/STATIC_ANALYSIS.md), [read-only Python3 verifier](https://github.com/Hawky11/1/blob/stage60-original-x-write-dataflow-static-analysis/Stage60_Analysis/tools/verify_stage60.py), [Russian guide](https://github.com/Hawky11/1/blob/stage60-original-x-write-dataflow-static-analysis/Stage60_Analysis/README_STAGE60_RU.md). Private original `AALib.exe` PE32 x86 SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. The locally executed offline verifier passed **10 direct rel32 CALL target checks, five FSTP-to-EIP-after signature checks, and nine raw load/add source-term checks**. No change to game, DLL, INI or earlier source. Previous Stage59 r2 is the latest Windows runtime-verified module.

**Early movement integration:** `A119B→AE6E0` invokes `1E66E0` via `AE70C` among object-list update iterations. The observed `1E70DB FSTP [ESI+7C]` is a position integrator combining original X with terms sourced from `object+88`, `object+150`, `object+D4` and orientation flags `object+24`; Y has its own store at `1E711A`. This static study did not identify who *produces* those input values.

**Pair adjustment:** coordinator `A11A0→ACDF0`, helper `ACB80` called twice at `ACE3D` and `ACE56`; `ACB80` calls scalar helper `1E43A0` twice (`ACBF2`, `ACC02`), bounds local terms and writes current X plus a derived delta at `ACC57 FSTP [EBX+7C]`. Separate guarded inline store at `ACF2A FSTP [EAX+7C]`, whose predicate includes `ACF0F→B86A0` AL flag, confirmed dynamically by Stage58/59.

**Tail paired-object record resolution:** `ACFD6→15CEC0→12D520→126580` via exact direct callsites `15CF5B` and `12D58A`. The latter call is guarded by a record's first DWORD equal to 7 (not proof of action ID). `126580` reads paired pointers `record+40/60`, compares `object+DC`, reads `object+88`, object orientation `+24` and bounded scalar fields `+1B8/+1BC`, then writes `X+local_delta` at original `126A9A` or `126B49`. These observations fit spatial/contact corrections; labels like physics/collision remain interpretations without engine symbols.

**Conclusion:** Stage59's measured X writers are already fully attributed for its test run and correspond to object coordinate update/correction. No evidence that any of these is the **AI action-selection** decision itself, no demonstrated smarter AI, no causal claim for hover-dash, attacks or damage. **Stage61 static-first:** locate writers of the input motion fields and separately correlate COM command/action producer flow, seeking a justified same-actor causal handoff before designing any new passive hook or behavior change. Keep working Stage59 r2 DLL as rollback.

---

## Stage61 — upstream native motion setter provenance + strict same-run action-to-X correlator (2026-11-10)

**Status:** STATIC VERIFIED, OFFLINE TOOL SYNTHETIC-TESTED; Stage61 runtime triple pending. No new DLL, game hook, watchpoint owner or gameplay write. Stage59 r2 DLL remains the working 1,153,024-byte rollback baseline.

Source and report: [stage61-action-definition-motion-join](https://github.com/Hawky11/1/tree/stage61-action-definition-motion-join), [STATIC_ANALYSIS.md](https://github.com/Hawky11/1/blob/stage61-action-definition-motion-join/Stage61_Analysis/STATIC_ANALYSIS.md), [correlator](https://github.com/Hawky11/1/blob/stage61-action-definition-motion-join/Stage61_Analysis/tools/correlate_stage61.py), [RU guide](https://github.com/Hawky11/1/blob/stage61-action-definition-motion-join/Stage61_Analysis/README_STAGE61_RU.md). Private local original AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. **15** original direct x86 rel32 call targets and **14** store/load signatures verified. Add new native provenance without repeating established Stage46/47 results: `152767/1527F8/153675→1E4A00` (`1E4A99 FSTP O+150`, `1E4AA2 FSTP O+154`), `15541F/155479/1554CB/155515→1E4AB0`, 1E66E0 conditional O+150 updates at `1E6B70/1E6B81/1E6C10`, O+D4 at `1E6BB2/1E6BD4/1E6C4D`, O+88 at `1E70EE`, plus known definition-driven O+90 stores at `1E8B97/1E8C34`. They feed the original `1E70DB FSTP [ESI+7C]` raw X update in the sampled engine motion route. No assertion these setters are COM-only or improve AI.

Stage46 previously identified definition `+30` state dispatch with O+88/90/150 initializations; Stage47 previously instrumented two of six 1E66E0 calls; Stage59 r2 watched original after-EIP 1E70DE. Stage61's new Python3 offline program matches (1) latest preceding Stage46 action definition by same side/object/TID, (2) Stage47 call with same definition and native QPC interval, (3) Stage59 original X store by same side/object/TID and watched==object+7C whose QPC lies **inside** the Stage47 interval. Reports active O+60 ID, definition mode, net raw X and shadow-based before/after consistency separately. Four synthetic tests pass: positive triple; disjoint QPC sessions rejected; actor mismatch rejected; definition mismatch not counted. This is conditional temporal/structural provenance, not a COM decision or animation/hit proof.

**Real-run blocker:** GitHub main's Stage46 and Stage47 files were recorded **19:56:41–19:57:25**, P1 object `2A35A020`/TID 10280; Stage59 r2 original upload was **22:07:43–22:08:23**, P1 object `2A89A020`/TID 15132. Not one session; never join or normalize their QPCs. No Stage61 real-run triple verified, no evidence Stage61 changed behavior.

**Next action:** user should upload missing Stage46/47 from the original 22:07 capture if preserved (only two files), or run unchanged Stage59 r2 once and upload three fresh same-run Stage46/47/59 logs. Stage62 can then use the strictly matched action IDs and metadata modes to investigate the native command/COM choice chain, correlating Stage40/42/44 if stronger decision attribution is required.

---

## Stage61 runtime — 4,616 same-session action-definition / native-motion / X-writer joins VERIFIED (2026-11-10)

**New user evidence** [c09bc526](https://github.com/Hawky11/1/commit/c09bc52690f149b0c7159014d01b86deb8ae5f79): two uploaded Stage46/47 logs from the **same 22:07** game run as previously frozen Stage59 r2 `d2ce2444`; raw snapshot branch [stage61-runtime-action-motion-x-evidence](https://github.com/Hawky11/1/tree/stage61-runtime-action-motion-x-evidence). [Detailed report](https://github.com/Hawky11/1/blob/main/Stage61_Runtime/RUNTIME_ANALYSIS.md). Source-only correlator [stage61-action-definition-motion-join](https://github.com/Hawky11/1/tree/stage61-action-definition-motion-join) was prepared before this upload; **no new DLL, native hook or gameplay modification**. Existing verified Stage59 r2 DLL size 1,153,024 bytes.

Stage46 **1,476** unique contiguous stage44Seq, two exact P1/P2 objects, 1FFF/1FFF read masks and resolverCount=1; Stage47 **5,292** unique contiguous seq with `AE70C` native source, 1FFF/1FFF masks and 23 healthy no-error Stage47_HEALTH snapshots. Stage59 **16,121** unique contiguous write rows, as previously verified. All three use same native actor objects P1 `2A89A020`, P2 `2A89AC50`, TID `15132` and QPC ranges in the 22:07–22:08 run. Stage46 logging does not have independent HEALTH records. No mixing with the older 19:56 Stage40/42/44 root captures.

Of Stage59's **4,711** after-write events at `RVA_after=001E70DE` (original `1E70DB FSTP [ESI+7C]`), **4,617** nest within a unique same-side/object/TID Stage47 call QPC interval; **4,617/4,617** last newRaw agrees with Stage47 POST X, and first best-effort oldShadow with PRE X. These represent **4,617 distinct Stage47 windows**, **1,906** of which change raw X. The other **94** target post-write events (47 per side) are **after the latest logged Stage47 QPC** and remain excluded as tail coverage, not a different inferred writer. 675 Stage47 windows lack a nested X write at this post-EIP and all have unchanged net X.

Matching each nested window to the **latest earlier** Stage46 same-actor binding by exact `definitionAfter == Stage47.def PRE` yields **4,616/4,617 strict triples**. All **4,616** have identical active action ID O+60 at Stage46 POST and Stage47 PRE; **1,905** triple windows change raw X. Last Stage47 P1 seq 5292 has definition `261E62D8` vs latest available Stage46 seq1476 definition `261E627C`, though O+60=00000700 matches and X changes; data stream ends immediately afterwards. A newer binding might be missing; **not proven an AI bug, corruption or missed game action**.

Mode (definition+30 low byte) vs net changed X among 4,616 strict triples: mode0 **3,989 / 1,567**, mode1 **134 / 2**, mode2 **57 / 13**, mode3 **226 / 213**, mode4 **109 / 107**, mode5 **98 / 0**, mode7 **3 / 3**. Mode3/4 have high observed changed-X proportions (94.2%/98.2%), consistent with earlier Stage46 definition-driven motion-float initialization, but not proof of intentional COM behavior or improved AI. One active ID may persist across hundreds of repeated motion windows; do not equate windows with unique decisions.

**Stage62 next:** use only Stage40/42/44 logs from the **22:07** session (if user retained them) to connect native request consumption and activation to the verified action definition and motion chain. Current root Stage40/42/44 are **19:56** stale captures. Request these three specific missing logs, or one new homogeneous session on the unchanged Stage59 r2 DLL if they were overwritten. Do not yet modify game-state variables, animation, hit logic or AI decisions.

## Stage74 runtime — native selector PRE/POST capture verifies only pre-callback transitions (2026-11-11)

- **User upload frozen:** [211f088](https://github.com/Hawky11/1/commit/211f0882e6b8e220877e4c15b072e5a7b8a7b067). **Analysis branch:** [stage74-runtime-selector-transition-evidence](https://github.com/Hawky11/1/tree/stage74-runtime-selector-transition-evidence), branched from exact upload commit, retaining original five logs. [Full report](Stage74_Runtime/RUNTIME_ANALYSIS.md), [RU summary](Stage74_Runtime/README_STAGE74_RU.md), [results](Stage74_Runtime/results.json). Stage74 source already archived at [stage74-callback-selector-boundary-snapshot](https://github.com/Hawky11/1/tree/stage74-callback-selector-boundary-snapshot).
- User Build.log 11:36:55, **MSVC 14.51.36231 x86 BUILD SUCCEEDED**, DLL **1,893,376 bytes**, old C4754 warning only. Same-run 11:38:06–11:38:42 P1=`0DF21F78/2A10F020`, P2=`0DF4CCA0/2A10FC50`, TID 19292. Stage38 installed 2/2; 3,708 complete 32-field rows (P1 1802/P2 1906) all PRE/POST masks FFFFFFFF. Stage65 313, Stage70 3, Stage59 11,957 events; Stage65/70/59 all known HEALTH errors zero. Stage59 contains all unique seq 1..11957 but ring flush physically emits seq8179..8192 after seq8352; sort by QPC.
- **Five selector P+3F50 changes occur ONLY between Stage38 previous POST and next PRE**: P2 `278→280:0→1`; P2 `651→653:1→0`; P1 `2528→2530:0→1`; P1 `2746→2748:1→2`; P1 `3164→3166:2→1`. No within-window selector changes. All five next windows contain callback `158570` with argument `11h` and descriptor update to `155160`; PRE selector was already changed. Original switch `00558DC4[11h]=00558ACB` conditionally consumes selector via `158ADC`, which is a **read**, not exact writer EIP.
- Stage38 `f31=P+3F54` shows 7 within-window `0→1` state changes all callback `1580F0`, plus 16 between-window state changes. Three same-frame Stage65/70 request/deferred pairs inside Stage38 callback `1566B0`: P1 `2F→25` selector1, P1 `33→26` selector2, P2 `2E→24` selector0. Source of numeric selector choices not proven, no gameplay/AI modifications.
- **Stage75 static first**: original pre-callback producer and registration of callback parameter `11h`; no blind 11 hook instrumentation or DR conflicts. Known-good Stage74 DLL **1,893,376 bytes**, Stage70 r2 DLL **1,892,864 bytes** retained for rollback. No new Stage74 DLL built by analyst, only user Build.log evidence; no new logs needed to close Stage74.

## Stage75 — native event-to-callback 0x11 registration STATIC VERIFIED (2026-11-11)

- **Branch:** [stage75-event-record-callback11-static-provenance](https://github.com/Hawky11/1/tree/stage75-event-record-callback11-static-provenance), based on Stage74 runtime-verified branch; [full report](Stage75_Analysis/STATIC_ANALYSIS.md), [RU guide](Stage75_Analysis/README_STAGE75_RU.md), [original PE verifier](Stage75_Analysis/tools/verify_stage75.py), [results](Stage75_Analysis/results.json). Private original SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` independently checked; **35 native opcode ranges and 8 relative CALLs** passed. User's 2026-11-11 Stage74 raw upload [211f088](https://github.com/Hawky11/1/commit/211f0882e6b8e220877e4c15b072e5a7b8a7b067) reused unchanged. **STATIC ONLY; no new build/runtime/DLL/hook/DR/gameplay modification.**
- Original **`158E90`** reads **P+3A36** as signed 16-bit WORD (native previous event code) and calls **`0F7E70`** to set **`P+3ED0=158570`** and **`P+3ED4=sign_extended(P+3A36)`**; one original direct E8 caller of 158E90: **`EE643`** in `EE5E0`, gated by active event and **P+3A34=0** previous mode. This explains a concrete native source path for Stage74's f4 callback arg **0x11** distinct from **selector P+3F50=1**.
- Original event record base **P+3A2C**: current mode/code at **P+3A2E/P+3A30** written via `F2DE0`; conditional `F2E50` (called at `EE57D` from `EE480`) copies current mode/code to **previous P+3A34/P+3A36** and invalidates the current code. No Stage74 record-field snapshots or exact EIP hits exist; other direct writes/aliases remain possible.
- After Stage38 POST-before-FC60, original loop calls **`EE7C7→FC60`**, **`EE7D1→EE480`**, **`EE7D6→E8DA0`**, and potentially **`EE7EA→EE5E0`** (P+3B7C bit23). Thus callback registration can occur outside Stage38's observed callback window. Original switch parameter **11h** routes `158570→158ACB`, whose gated instruction `158ADC` is a **selector read**, not the missing writer proof.
- **Stage76 next:** scoped passive post-callback phase attribution at original call boundaries, full ABI/original-opcode and owner audit before any DLL; do not force selector/AI actions or allocate DR0/DR1, currently Stage59-owned. Retain Stage74 working DLL **1,893,376 bytes**, Stage70 r2 rollback **1,892,864 bytes**. No AI quality improvement claim.

## Stage76 — source-ready native after-callback EE480/E8DA0/EE5E0 phase snapshots (2026-11-11)

- **Branch:** [stage76-after-callback-phase-snapshots-source](https://github.com/Hawky11/1/tree/stage76-after-callback-phase-snapshots-source) based on Stage75. [Two-file source overlay](Stage76_Overlay/src/Stage38CallbackWindow.inl) plus [new 3-phase observer](Stage76_Overlay/src/Stage76AfterCallbackPhases.inl), [technical report](Stage76_Analysis/STATIC_ANALYSIS.md), [RU setup](Stage76_Analysis/README_STAGE76_RU.md), [static verifier](Stage76_Analysis/tools/verify_stage76.py), [results](Stage76_Analysis/results.json). Original private PE SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` rechecked. Exact original E8 CALLs and surrounding bytes checked against binary: **`EE7D1→EE480`**, **`EE7D6→E8DA0`**, **`EE7EA→EE5E0`** (returns EE7D6/EE7DB/EE7EF). **STATIC SOURCE READY, NOT WINDOWS BUILD OR RUNTIME VERIFIED**.
- The new probe wraps original same-length CALL sites, executes each real native callee once and samples selector P+3F50 and state P+3F54 PRE/POST, packed event record P+3A2C/3A30/3A34, P+3ED0/3ED4 callback descriptor, P+3B7C flags, P+3F4C immediate request and P+4 object; logs player/O/TID/QPC/masks, phased health, with TLS slots and SRWLOCK ring. The conditional EE5E0 argument is re-pushed from original stack; native EE7EF still cleans original arg. **No DR register ownership or gameplay/AI writes.** Return address visible inside original callee differs from uninstrumented code: safety/equivalence requires Windows game test.
- Integration to tested Stage74 adapter is exact three edits: extra include, install Stage76 after Stage70 success before Stage59 worker, flush Stage76 from existing Stage38. Stage74 original source blob `106dca56d662fe1c8e0b7083013a535b67cde3c8`; Stage76 adapter `f439a0f35e08891590acdb6099afd04ed8d821a4`; new probe `4d1fad129a6269c764d6950fd8a8269bb6b18189`. Installs 3 patches under existing mutex and peer suspension, exact opcode guards, transaction/rollback, no extra conflict with Stage38 EE7C7.
- **Next Stage76 Windows user:** back up tested Stage74 DLL **1,893,376 bytes** and adapter; place exactly two Stage76 overlay .inl files in local project src; keep other source/ini unchanged; run optional SHA/source verifier, Build.bat, then one COM-vs-COM run. Expect `STAGE76_INSTALLED 3/3` + Stage38 2/2, Stage70 1/1, Stage59 WATCH_ARMED and HEALTH zero. Upload Build.log + Stage76/38/65/70/59 same-run logs; abort/rollback on any crash/ABI regression. **No new DLL was built by analyst; do not invent runtime results, AI improvements, exact writer IP or a size for new DLL.** Original user's frozen logs and default main remain unchanged.

## Stage77 — selector before native 156C00, source ready (2026-11-11)

- **Closed prior stage:** user [Stage76 upload d0fddb14](https://github.com/Hawky11/1/commit/d0fddb14bdccadaf0e42be5208cba44fcf544f0d) was analyzed and frozen at [stage76-r1-runtime-after-callback-selector-gap](https://github.com/Hawky11/1/tree/stage76-r1-runtime-after-callback-selector-gap): Windows x86 BUILD SUCCEEDED, Stage76 3/3 and 11,478 records, zero selector changes inside 5,739 EE480 and 5,739 E8DA0 calls; 0 EE5E0 events; five selector transitions between callback windows; six Stage65→70 original-site deferred writes in their Stage38 callback. DLL size 1,901,568 bytes. No writer EIP.
- **New Stage77 source:** [stage77-pre156c00-selector-boundary-source](https://github.com/Hawky11/1/tree/stage77-pre156c00-selector-boundary-source), one-file [Stage38 replacement](Stage77_Overlay/src/Stage38CallbackWindow.inl), [static audit](Stage77_Analysis/STATIC_ANALYSIS.md) and [RU install instructions](Stage77_Analysis/README_STAGE77_RU.md). It captures five P fields and QPC immediately before the native 156C00 using PUSHFD/PUSHAD/FXSAVE, then matches the existing after-156C00 Stage38 PRE, logs n0..n4 in Stage38 log sharing seq. Uses fixed TLS depth16 and existing Stage38 ring; preserves native 156C00 original-once call; no extra native patch/DR or AI write.
- **Status:** source-level checks only, no Stage77 MSVC build, runtime install, or game test. Next step is user one-file replacement, Build.bat, one same-run COM-vs-COM capture; correlate n0 from Stage77 with Stage76 phase1 and Stage38 PRE by player/object/TID/QPC. Retain tested Stage76 DLL 1,901,568 and Stage74 DLL 1,893,376 for rollback. No AI enhancement claimed.

## Stage78 — exact literal-one selector writer observer SOURCE READY (2026-11-11)

- Based on Stage77 source and its separately frozen [runtime evidence](https://github.com/Hawky11/1/blob/stage77-runtime-selector-positive-callback-evidence/Stage77_Runtime/RUNTIME_ANALYSIS.md) from user commit 2ee8534. Positive Stage77 P2 callback 1560B0 changed selector 0→1, state1→3, with event21 matching static 1560B9→155B70→155D74/155D29. Direct original 155D29 EIP still pending; separate P1 inter-frame change remains unknown.
- Local user-owned AALib.zip original EXE SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba verified without uploading binary. Seven opcode sites verified including exact ten-byte 155D29 MOV DWORD [ESI+3F50],1, 155D27 PUSH 21, 155D33 PUSH0 and two CALL 155B70 entrances.
- New [stage78-literal-one-writer-exact-site-source](https://github.com/Hawky11/1/tree/stage78-literal-one-writer-exact-site-source), [native observer](Stage78_Overlay/src/Stage78LiteralOneWriter.inl), [Stage77-derived adapter](Stage78_Overlay/src/Stage38CallbackWindow.inl), [audit](Stage78_Analysis/STATIC_ANALYSIS.md), [user guide](Stage78_Analysis/README_STAGE78_RU.md), verifier and machine summary. Single native detour at 155D29 (10 bytes), once-only replay original MOV, read-only snapshots and QPC before/after, EBP+4 caller return, TLS/ring/HEALTH. No new debug-register owner or extra gameplay write; original Stage77/76 included.
- **Only source/static checks complete. Windows x86 build, hook installation, same-run positive native execution, ABI equivalence and gameplay runtime NOT VERIFIED.** Next: user two-file copy, Build.bat, short COM-vs-COM, same-run Build+Stage78/38/76 logs. Preserve tested Stage77 1,905,664-byte DLL for rollback. A negative single-site test does not exclude ten register-derived writer sites or indirect/alias writes. No improved AI behavior demonstrated.

## Stage79 — common native E89E0 origin and passive single-call observer (2026-11-11)

- Investigated user-owned SHA-verified original AALib.exe from Library AALib.zip without publishing binary. Found all ten Stage71/73 computed numeric selector1/2 MOV sites in **four** native functions: F3930(4), F4360(2), F46F0(2), F7150(2). Disassembled and independently checked **11** exact original CALL E8 edges connecting these functions under original E89E0, with **single direct original CALL EFFE8→E89E0**. Exhaustive byte-wide direct E8 target scan confirms **8** expected direct caller inventories; indirect/aliased paths remain unknown.
- Created source branch [stage79-effe8-common-selector-family-observer-source](https://github.com/Hawky11/1/tree/stage79-effe8-common-selector-family-observer-source). New [Stage79SelectorFamilyBoundary.inl](Stage79_Overlay/src/Stage79SelectorFamilyBoundary.inl) intercepts only EFFE8 5-byte native CALL, captures P+3F50 selector and 12 supporting fields around original E89E0 call (once) with P/O/TID, QPC, return, full masks/HEALTH. No new DR/VEH ownership or extra gameplay writes. [Stage79 Stage38 adapter](Stage79_Overlay/src/Stage38CallbackWindow.inl) is Stage78 + exactly three include/install/flush edits. User guide, static report, pinned verifier and results recorded under Stage79_Analysis.
- **Stage79 source/static verified only: Windows x86 build/COM-vs-COM runtime NOT tested.** A positive Stage79 family-level value change will NOT by itself identify a specific native writer EIP. Next user test is a two-file copy, Build.bat, same-run Stage79/38/76/78 logs. Rollback user-tested Stage78 DLL 1,911,808 bytes; Stage77 1,905,664; Stage76 1,901,568. Preserve frozen Stage78 zero-hit user logs untouched; no COM behavioral enhancement proven.

## Stage80 — narrow native late F7150 observation SOURCE READY (2026-11-11)

- From frozen Stage79 user [1b2aada](https://github.com/Hawky11/1/commit/1b2aadaeabeebf959fa0575cf59023060e481323), the successful original common call `EFFE8→E89E0` selector transitions were **P1 0→1 (Stage79 seq2901), P2 0→1 (seq3404), P2 1→0 (seq3740)**. Full-mask QPC windows, event code `P+3A30:FFFF→11`, state P+3F54 unchanged=0, Stage78 direct literal-one site 0 hits; true Stage38 same-player transitions exactly 3. Tested Stage79 DLL **1,918,976 bytes**.
- Original SHA-pinned private AALib.exe static audit: E89E0 gates P+3B7C bit15, P+3F7C and P+3B90 bit4, all three positive recordings clear these early paths. Later original call `E8A8D PUSH ESI; E8A8E CALL F7150; E8A93 ADD ESP,4` may run after earlier helpers; **not yet directly observed**. F7150 has candidate native selector writes `F7D22` (1/2), `F7D38` (1/2), `F7D43` (0); no individual writer IP proven.
- New [stage80-e8a8e-late-f7150-boundary-source](https://github.com/Hawky11/1/tree/stage80-e8a8e-late-f7150-boundary-source): [Stage80F7150LateBoundary.inl](Stage80_Overlay/src/Stage80F7150LateBoundary.inl) intercepts exactly one 5-byte CALL E8A8E, copies the original P argument once to call native F7150 exactly once, preserves original caller arg for E8A93 ADD ESP,4, and logs P/O/TID, QPC and 12 selector/flag/event fields with zero-error HEALTH. No new DR/VEH owner or game-field write. [Stage38 adapter](Stage80_Overlay/src/Stage38CallbackWindow.inl) is Stage79 + exactly three include/install/flush edits. RU guide, static report, local pinned verifier and results documented.
- **Stage80 source/static prepared only; Windows build, installation, ABI safety, runtime signal and COM gameplay UNTESTED.** Next user two-file copy, Build.bat, short COM-vs-COM and same-session Stage80/79/38/76 logs. Rollback verified Stage79 DLL **1,918,976 bytes**, Stage78 **1,911,808** and Stage77 **1,905,664**. A Stage80 positive proves function-level interval only; direct writer EIP still requires later proof. No AI enhancement claimed.

## 2026-11-11 — Stage81 r2 selector-origin runtime / main history reconciliation

- Source of same-session evidence: [commit c003261e](https://github.com/Hawky11/1/commit/c003261e9e63e817c92bd5e58fd4afa35ee34e52), originally pushed to `stage80-e8a8e-late-f7150-boundary-source` rather than `main`. Older main branch had stale README top entry at Stage61 and overlapping Stage81 r1 logs.
- Stage81 r1 installed writer detours before Stage80 verified original writer bytes; Stage80 refused patch with `nativeOK=0`. Stage81 r2 source-only installer delayed native writer hook installation until after Stage80: `STAGE81_PREREQ_STAGE80_OK`, `STAGE81_INSTALLED 3/3`.
- Windows MSVC x86 BUILD SUCCEEDED, DLL 1,934,336 bytes. 5,628 Stage79 common windows, 5,185 Stage80 native F7150 windows, all 5,185 fully nested unambiguously in same-side/player/object/thread Stage79 windows. Four positive selector changes: P2 0→1, P1 0→2, P1 2→1, P2 0→2. See [Stage81 runtime analysis](Stage81_Runtime/RUNTIME_ANALYSIS.md).
- Stage81 exact native writer hooks F7D22, F7D38, F7D43 installed but each recorded **zero** hits despite these four positive transitions. This invalidates treating them as proven executed writer EIPs in this session. All observers reported zero dropped/overflow/mismatch/readFail.
- Reconciled two diverged branches with a **non-force two-parent merge**: original `main` and Stage80 work-branch histories preserved. Latest Stage81 r2 raw logs and Build.log are placed at root; Stage81 r1 conflicting root logs retained under `Stage81_Runtime/r1_failed/`. No AI/gameplay behavior change claimed.
- Next Stage82: static callgraph / writer provenance analysis of F7150 and nested/indirect paths, then minimal additional read-only instrumentation only if justified.


## 2026-11-11 — Stage82 r1 signature check refused; nested selector boundaries still unresolved

[User-uploaded main snapshot](https://github.com/Hawky11/1/commit/de188f061689f5ed00e594413adec786a6735335) and [runtime analysis](Stage82_Runtime/RUNTIME_ANALYSIS.md): MSVC x86 build passed, 1,942,528-byte DLL. Stage79 6,084, Stage80 5,652, perfect 5,652/5,652 QPC containment; five same actor/object selector transitions; all HEALTH errors zero. Stage81 exact MOV hooks hit twice without selector changes. Stage82 failed preflight (`call signature; no patch`), so no nested-call writer attribution. Static verified original AALib.exe retains both expected E8 callsite signatures F79E4→F4360 and F7AD1→F3930. Next is fail-closed Stage82 r2 in-memory byte audit, source-only pending Windows validation. No claimed COM behavior change.
