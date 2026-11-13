# ArcanaAIOverhaul — Stage146 full current source/config owner inventory READ-ONLY (2026-11-13)

**SOURCE-ONLY; no game files or native hooks modified; SAFE_TO_INSTALL=false; Stage36 OFF.**

## User Windows PowerShell 5.1 current-source capture — USER-REPORTED COMPLETE (2026-11-13)

- The user ran `Capture-Stage146CurrentSources.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul' -IncludeSourceSnapshot` in the project's Stage146 test directory. Console reports **`STAGE146_CAPTURE_COMPLETE`** and zero read errors.
- Current local inventory reported `source=85`, `config=22`, `skippedGenerated=11`, `skippedSensitive=0`, `readErrors=0`. Full Stage134 archival comparison: **`expected=70 match=70 drift=0 missing=0 new=37`**. This is the first *reported* 70/70 comparison against the actual current Windows source tree, superseding the former five-file-only status.
- `.vcxproj` items=24, `#include` edges=409, names-only INI keys=46. Known owner-site static review: **5 sites; 5 archived source hashes match; zero overlaps among those five known intervals**; `liveOwnerProof=NO`. Heuristic RVA references: **485**, expressly `UNVERIFIED_NOT_OWNER_PROOF`. These are text references, not confirmed patch writes or 485 installed hooks.
- Actual produced evidence ZIP path (on **the user's Windows disk, not yet uploaded here**): `C:\Users\Noma\AppData\Local\Temp\ArcanaAI_Stage146_Evidence_493fc20dff504bdabc3abc4924a9426f.zip`. The tool-generated ZIP's contents, manifest, current complete sources and individual 485 heuristic lines **have not been inspected in this conversation**. **Stage147 file-level ownership/conflict classification is pending that upload**.
- `-IncludeSourceSnapshot` was used; user should **review source for embedded credentials/private strings before sharing**, despite config values being omitted. Do not send game binaries or actual secrets.
- No game process opened; `STAGE146_NO_LIVE_OWNER_PROOF` explicitly printed. **No game owner readback, active-detour quiescence, ABI/SEH, FLS or worker lifetime verification; SAFE_TO_INSTALL=false**, Stage36 OFF.

Exact user-provided console output:

```text
STAGE146_SOURCE_INVENTORY source=85 config=22 skippedGenerated=11 skippedSensitive=0 readErrors=0
STAGE146_STAGE134_COMPARE expected=70 match=70 drift=0 missing=0 new=37
STAGE146_PROJECT_ITEMS count=24 includeEdges=409 iniKeysWithoutValues=46
STAGE146_KNOWN_SITE_REVIEW sites=5 archiveSourceHashMatch=5 knownIntervalOverlaps=0 liveOwnerProof=NO
STAGE146_HEURISTIC_RVA_REFERENCES count=485 classification=UNVERIFIED_NOT_OWNER_PROOF
STAGE146_EVIDENCE_ZIP C:\Users\Noma\AppData\Local\Temp\ArcanaAI_Stage146_Evidence_493fc20dff504bdabc3abc4924a9426f.zip
STAGE146_NO_LIVE_OWNER_PROOF: no game process access; source-only evidence; SAFE_TO_INSTALL=false
STAGE146_CAPTURE_COMPLETE: current source/config metadata captured; SAFE_TO_INSTALL=false
```

## Predecessor and current evidence boundary

- User Windows Stage145 `Test-Stage145LocalEvidence.ps1`: **11/11 archival references SHA MATCH**, **5/5 current project sources SHA MATCH**, **38/38 selected surface records** and **17/17 obligations listed**. Console explicitly reports `STAGE145_NO_LIVE_OWNER_PROOF`. The 17 obligations are **not proven satisfied**. See [Stage145 checkpoint](STAGE145_CHECKPOINT.md).
- The Stage134 snapshot manifest (dated **2026-11-12**) lists **70 archived source files**. A prior check of 70/70 archives **does not** establish matching current Windows source files. Stage146's goal is to collect *current* source metadata, archived SHA comparisons, C++/project include references, possible hook-site mentions, and source copies only if explicitly requested, with **no process attachment**.

## Stage146 delivered source-only artifact

- Chat-only ZIP: **`ArcanaAI_Stage146_FullCurrentSource_OwnerInventory_ReadOnly_SourceOnly.zip`**; **60,443 bytes**, SHA-256 **`b9bc8317aae43a29a024f65f3581b61d4bb4d277a46eeea63642e064bff9f9db`**, **22 files**, ZIP CRC PASS, `package_manifest.json` **21/21 SHA-256 + size PASS**. No EXE/DLL/PDB/LIB/OBJ/PYC and no changes to current project.
- Archive root `Stage146_SourceOnly/`: `tests/Capture-Stage146CurrentSources.ps1`, `tools/analyze_evidence.py`, `tests/test_analysis.py`, `tests/audit_stage146.py`, README + safety/owner docs, 70-entry Stage134 captured SHA manifest, historical 38-surface Stage144 inventory and 17-obligation Stage145 ledger, and **five byte-identical archival reference files**.
- Current-source script **Windows PowerShell 5.1**, no MSVC/Python/administrator or running game required. It reads files under the supplied ProjectRoot but **writes only to a GUID directory under %TEMP%** by default; explicit OutputRoot inside ProjectRoot is rejected. Does not call game process APIs, patch instructions, install native hooks, build DLLs, run Build.bat or change INI settings.
- Files with source/project extensions are hashed; selected config files are hashed with **name-only INI section/key inventory** (`value=OMITTED`) and never copied as config contents. Files named like credentials/private keys and build/object/generated directories are excluded. An opt-in `-IncludeSourceSnapshot` adds copies of source/project files only, rechecks copy SHA, but does **not** copy .ini or executable DLLs. Review any source snapshot for embedded secrets before sharing.
- Local evidence files: `summary.json`, `current_sources.json`, `stage134_drift.json`, `known_owner_sites.json`, `candidate_rva_references.json`, `vcxproj_items.json`, `include_edges.json`, `ini_key_names.json`, `read_errors.json`. The output is zipped as `STAGE146_EVIDENCE_ZIP ...`, separate from the deliverable.
- Comparison covers **all 70 historic expected paths**, with `MATCH/DRIFT/MISSING`; `NEW` means present in current scanned tree but **not in Stage134 archive**, not proof that a file was newly authored. Presence of additional config files is normal.
- Five historic nonoverlapping owner intervals: Stage10 **F34D0 5B, F3930 6B**, Stage129 **F30B0 6B, F325A 5B, F326A 5B**. Five-interval mutual overlap count is zero from archived source; **other current potential hook sites and live owners are NOT covered by that finding**.
- Heuristic C++ RVA text references are explicitly `UNVERIFIED_NOT_OWNER_PROOF`; many references may be reads, constants or diagnostics rather than competing patch ownership. `vcxproj_items` and `include_edges` are source-level text listings, **not evaluated MSBuild graph or live installed DLL evidence**.
- The standalone Python 3 `tools/analyze_evidence.py` can analyze the returned Windows ZIP after extraction and reject contradictory counts, false SHA matches, attempts to assert live ownership, or altered source copies.

## Offline checks completed

- **Static audit 104/104 PASS** (five source SHA anchored to Stage134, 70 unique archived records, exact 5 known owner intervals, 10 pairwise nonoverlaps, source-only/no-unsafe-operation text gates and doc/source inventory).
- **24/24** portable Python unit scenarios PASS, including archived SHA drift and missing detection, duplicate paths, fake live-owner claims, synthetic hook-reference collision and source snapshot corruption. No native/game executable was run.
- **ZIP CRC PASS; per-file manifest SHA256/size 21/21 PASS**. This is the produced archive's verified state, not current Windows project evidence.

## Windows handoff — **USER RUN COMPLETE; EVIDENCE ZIP NOT YET SHARED**

Unzip Stage146; from Windows PowerShell 5.1 inside `Stage146_SourceOnly\tests`:

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
.\Capture-Stage146CurrentSources.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul' -IncludeSourceSnapshot
```

Omit `-IncludeSourceSnapshot` to produce metadata only. With the switch, **inspect the generated ZIP for sensitive text before uploading**.

**Expected markers only** (counts depend on actual current tree; do not presume 70/70):

```text
STAGE146_SOURCE_INVENTORY source=... config=... skippedGenerated=... skippedSensitive=... readErrors=0
STAGE146_STAGE134_COMPARE expected=70 match=... drift=... missing=... new=...
STAGE146_PROJECT_ITEMS count=... includeEdges=... iniKeysWithoutValues=...
STAGE146_KNOWN_SITE_REVIEW sites=5 archiveSourceHashMatch=... knownIntervalOverlaps=0 liveOwnerProof=NO
STAGE146_HEURISTIC_RVA_REFERENCES count=... classification=UNVERIFIED_NOT_OWNER_PROOF
STAGE146_EVIDENCE_ZIP <external temporary zip path>
STAGE146_NO_LIVE_OWNER_PROOF: no game process access; source-only evidence; SAFE_TO_INSTALL=false
STAGE146_CAPTURE_COMPLETE: current source/config metadata captured; SAFE_TO_INSTALL=false
```

**Windows script executed successfully according to the user's console output above**. The exact generated current-source evidence ZIP has not been uploaded/inspected, so the reported 70/70 hash matches and RVA inventory remain unindependently audited at individual-record level. Record any failures and fix them as a source-only r2 if necessary.

## Current safety and Stage147

- **SAFE_TO_INSTALL=false**, **SAFE_TO_UNLOAD_GAME_DLL=false**, Stage36 OFF, `origin(0x27)=UNKNOWN`, `Arm()` NOT enabled, no new JMP, no gameplay/DR writes. Preserve Windows-tested Stage129 `dinput8.dll` **2,099,200 B** and Stage122 rollback **2,089,984 B**.
- Actual installed native hook ownership, game ABI/SEH/unwind, worker quiescence, independent external callers, FLS callback code pin/lifetime and sink durability remain **unproven**.
- **Next:** obtain and inspect Stage146 user's **actual Windows capture** including full current source copies (if shared) before a Stage147 owner-collision audit. Then classify each textual RVA reference by actual patch write/installer/provenance, not just repeated address strings. No automatic installation or in-game unloading.
