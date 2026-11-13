# ArcanaAIOverhaul — Stage147 verified Stage146 evidence ZIP + bounded native owner collision source audit (2026-11-13)

**STATIC / SOURCE ONLY. SAFE_TO_INSTALL=false; SAFE_TO_UNLOAD_GAME_DLL=false; Stage36 OFF. No game DLL/source/config mutation.**

## User upload confirmed — no additional file needed

- User DID upload the exact requested `ArcanaAI_Stage146_Evidence_493fc20dff504bdabc3abc4924a9426f.zip` into this conversation. Previous prompt requesting the same archive again was an oversight, now corrected.
- Stage146 evidence ZIP SHA-256 **`6d9c39d59b4883dab6cc4ea95f08deac56fe8c477fd6b4746bd1c764f96a207b`**, **489,054 bytes**, **94 ZIP entries**, CRC PASS.
- Stage146 JSON `summary.json` records user Windows run at `2026-11-13T16:27:20.2635057Z`, source=85, config metadata=22, archive expected=70/matched=70/drift=0/missing=0/new=37, project items=24, literal include edges=409, INI key names=46, no read errors, 485 heuristic RVA references, no live owner proof.
- **Independently verified 85/85 source/project copies** packaged in the uploaded ZIP against `current_sources.json` exact byte-size and SHA-256. **Only 69/70 Stage134 matched paths are included as copied source**, since Build.bat etc are not copied; remaining archived-path equality uses the Stage146 collector's Windows generated metadata report. 0 read errors.
- The actual ZIP was received, opened and independently examined. No further upload is required for this bounded static analysis.

## 37 additional paths classified from their exact names

- **31 files in hidden `.stage*backup` histories** (backup manifests and older source copies), outside literal active project compile/include graph.
- **6 configuration/installation-state metadata files:** `ArcanaAI.ini` and `Stage88InstallState.json` through `Stage92InstallState.json`.
- `NEW` means absent from Stage134 70-path manifest, **NOT** 37 new compiled source modules.

## Actual source compile/include path

- Current `ArcanaAIOverhaul.vcxproj` declares **8 `ClCompile` C++ root units**. Recursively resolving literal quoted local `#include` statements from those roots yields **66 reachable src files**. This is a static graph only; not evaluation of all MSBuild conditions/defines.
- `src/Stage83F3930Writer.inl` is **NOT** in this include graph, while `src/Stage84F3930Writer.inl` is reached by `src/Proxy.cpp` -> `src/Stage81SelectorWriter.inl`. `src/dinput8.def` is a linker definition file, not an include.
- None of the 31 backup files is reached via local include graph.

## 485 heuristic RVA records recategorized (exact 485/485)

| Source/category | Records | Reality |
|---|---:|---|
| Archived .stage backup copies | 132 | Not active compile-graph source |
| `Stage83F3930Writer.inl` literal sites | 4 | Currently not in compile/include chain |
| Previously marked `PATCH_OR_OWNER_REVIEW_CANDIDATE` | 12 | **All false positives** in error logging strings, not patch write statements |
| All other reachable source RVA literal mentions | 337 | **Owner UNPROVEN** until following actual install/write call chains |
| TOTAL | **485** | Literal text, **not 485 installed hooks** |

The 12 false-positive lines are in `src/AIQueueTrace.cpp` and `src/AITrace.cpp` and say `ERROR: VirtualProtect failed...` or `hook was not installed`. Real patch writers usually calculate target from constants/variables, then use `VirtualProtect`/`memcpy`/relative JMP. Thus **no automatic owner determination from these 485 literals**.

## Additional evidence beyond prior five-site inventory

- **11 bounded concrete patch-site declarations** anchored in current packaged source:
  - Stage10 producer `src/AIProducerContextTrace.cpp:35-36`: `F34D0/5`, `F3930/6`.
  - Stage10 parent `src/AIParentFlowTrace.cpp:39-41,170-175`: `F3280/6`, `F4030/6`, `F4360/6` (three additional known source hook sites).
  - Stage129 `src/Stage129ProducerProbe.inl:9-10`: `F30B0/6`, `F325A/5`, `F326A/5`.
  - Stage81 `src/Stage81SelectorWriter.inl:53-54`: `F7D22/6`, `F7D38/6`, `F7D43/10` (three additional known source hook sites).
- All **55/55 pairwise comparisons among only these 11 addresses have zero byte-range overlap**. This is a narrowly bounded test, **not** an all-project patch-owner collision proof.
- **Latent Stage83 ↔ Stage84 conflict:** dormant `src/Stage83F3930Writer.inl:12-13` declares four 6-byte sites `F3DE9`, `F3E02`, `F3F50`, `F3F69`, identical to four of six sites in reachable `src/Stage84F3930Writer.inl:12-13`. Since Stage83 is **not included in this compile/include graph**, no current double installation is established. If Stage83 is reintroduced or invoked separately, site ownership must be coordinated (Stage83 disabled or unified with Stage84).

## Stage147 artifact & checks

- Chat-only **corrected metadata-only archive r2** `ArcanaAI_Stage147_CurrentHookOwner_CollisionAudit_MetadataOnly_r2.zip`, **15,749 bytes**, SHA-256 **`4f90a72b02f54b08f022553850fefc77185bf2852b4e93f81d278fb227858fe3`**, **11 files**, CRC PASS, **10/10** per-file SHA-256 manifest. Contains report `STAGE147_AUDIT_RU.md`, `rva_classification.csv` (485 rows), `extras.csv` (37 paths), interval/compile graph JSON and test script, **no user source, INI values, native/pyc binaries or credentials**. A preliminary Stage147 archive accidentally contained generated Python cache; use **r2 only**.
- Portable Python **21/21 tests PASS** for static counters/categories, restricted overlaps, conditional conflict, and fail-closed deployment flags. 85/85 source snapshots byte-hashed against Stage146 evidence metadata, original uploaded evidence ZIP CRC PASS.
- Same report separately available in chat as `ArcanaAI_Stage147_Audit_RU.md`, SHA-256 `dbc9ffd54b2a6e1aace12eec8ca5620a00f0d0cc9468dc62faccfb99574668d3`.
- No MSVC compile, game execution, hook install, memory readback or DLL unload was performed at Stage147; no runtime claim from static snapshots. **There is not yet a complete scan of the numerous native patch routines across the other reachable AITrace/AIQueueTrace/StageNN sources.**

## Next Stage148

Source-only deeper **patch-writer-to-target provenance** audit: trace each reachable `VirtualProtect`/`memcpy(target, patch)` install path to its constants, guard/config and exact stolen-byte length, including indirect callsite patches, conditionally installed StageNN workers, and gaps in stop/join. Resolve all possible overlapping active paths before any live read-only owner check. No new JMP or `Arm()` and no automatic game installation.

**Preserve Stage129 Windows-tested dinput8.dll 2,099,200 bytes and Stage122 rollback 2,089,984 bytes. Stage36 OFF. origin(0x27)=UNKNOWN. SAFE_TO_INSTALL=false.**
