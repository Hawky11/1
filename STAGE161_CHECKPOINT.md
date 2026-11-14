# ArcanaAIOverhaul — Stage161 bounded local assignment-alias source hints (2026-11-14)

**Latest status: Stage161 user-confirmed Windows SOURCE_ONLY PASS: 99/99 tests, 66/66 SHA, 12 local alias candidates (4 each for three Stage67/70 RVAs), 15 blocked; real writer→target proof remains 0.** The **actual** Stage160 Windows source-only audit is confirmed PASS (73/73, 66/66, 443 source API call candidates, 0 direct RVA address operand tokens).

## User-confirmed Stage161 Windows current-project SOURCE_ONLY PASS — 2026-11-14

The user ran `Stage161_SourceOnly/Invoke-Stage161Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'` on their actual Windows source tree and pasted the console transcript. This is user-provided console evidence, **not** an independently uploaded complete Stage161 JSON or private C++ code.

```text
STAGE161_MODE SOURCE_ONLY READ_ONLY NO_GAME_DLL_ACCESS NO_NATIVE_BUILD
STAGE161_MANIFEST_PASS files=10
Ran 99 tests in 8.552s
OK
STAGE161_HASHES matched=66 expected=66
STAGE161_BASELINE stage160_api_candidates=443 direct_rva_operands=0
STAGE161_ALIAS_CANDIDATES bounded=12 blocked=15 proven_writer_target_edges=0
STAGE161_TARGET stage=STAGE10 rva=F34D0 alias_candidates=0 owner=UNKNOWN
STAGE161_TARGET stage=STAGE10 rva=F3930 alias_candidates=0 owner=UNKNOWN
STAGE161_TARGET stage=STAGE10_PARENT rva=F3280 alias_candidates=0 owner=UNKNOWN
STAGE161_TARGET stage=STAGE10_PARENT rva=F4030 alias_candidates=0 owner=UNKNOWN
STAGE161_TARGET stage=STAGE10_PARENT rva=F4360 alias_candidates=0 owner=UNKNOWN
STAGE161_TARGET stage=STAGE129 rva=F30B0 alias_candidates=0 owner=UNKNOWN
STAGE161_TARGET stage=STAGE129 rva=F325A alias_candidates=0 owner=UNKNOWN
STAGE161_TARGET stage=STAGE129 rva=F326A alias_candidates=0 owner=UNKNOWN
STAGE161_TARGET stage=STAGE81 rva=F7D22 alias_candidates=0 owner=UNKNOWN
STAGE161_TARGET stage=STAGE81 rva=F7D38 alias_candidates=0 owner=UNKNOWN
STAGE161_TARGET stage=STAGE81 rva=F7D43 alias_candidates=0 owner=UNKNOWN
STAGE161_TARGET stage=STAGE67 rva=AD75C alias_candidates=4 owner=UNKNOWN
STAGE161_TARGET stage=STAGE67 rva=E9E60 alias_candidates=4 owner=UNKNOWN
STAGE161_TARGET stage=STAGE70 rva=156784 alias_candidates=4 owner=UNKNOWN
STAGE161_LIFETIME RAW_CALLER_PROOF=NO WORKER_JOIN_PROOF=NO VEH_RETIREMENT=NO ROLLBACK_PROOF=NO
STAGE161_METADATA C:\Users\Noma\AppData\Local\Temp\ArcanaAI_Stage161_SourceOnly\stage161_alias_lineage_metadata.json
STAGE161_SOURCE_ONLY_PREFLIGHT_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
STAGE161_LOCAL_SOURCE_ONLY_TESTS_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
```

**Evidence interpretation:** 99/99 unittest PASS on actual Windows 3.14 (including previous non-admin symlink test), manifest 10/10, 66/66 current-source SHA and Stage160 baseline 443/0 consistency. **Twelve bounded assignment-chain source candidates found**, all at the three Stage67/70 risk target RVAs (**four each**), while **15 candidates are blocked/ambiguous**. The other 11 declared native target RVAs yield zero candidates **under Stage161's bounded 1–3 hop/200-line same-file lexical rule**, not a proof of no writer. The 12 are syntactic hints, not 12 installed hooks, not proven calls/ownership, not rollback transactions. **`proven_writer_target_edges=0`** and all live safety gates remain unchanged.

**Next Stage162 — targeted semantic source-only review, not yet executed:** consume the exact local Stage161 metadata (or targeted redacted source snippets), classify the 12 bounded Stage67/70 candidates by path/line, API role and ambiguity reasons, trace true source variable definitions, guards and earlier writes and restore-on-failure paths. Pay special attention to Stage67 `0xAD75C`/5, `0xE9E60`/6 and Stage70 `0x156784`/6; prior Stage150/151 model indicated partial write/rollback gaps. Treat no matches on Stage10/129/81 as analyzer coverage gaps, never as proof of absent patches. Do not change game code or introduce hooks. Stage162 has not been built or run yet.

**Hard gate:** `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, origin(0x27)=UNKNOWN. External raw/foreign native caller census, trampoline/ABI/SEH, nine worker joins, three VEH retirement, installed byte ownership and rollback stay UNKNOWN/NO. Existing Stage129 working DLL 2,099,200 B and Stage122 fallback 2,089,984 B remain unchanged.

---

## Why Stage161

Stage160 looked at direct target RVA literals **inside address arguments** to `memcpy/memmove/memset/WriteProcessMemory` (bytes-write candidates), `VirtualProtect[Ex]` (protection candidates), and `FlushInstructionCache` (cache flush). In the user's current 66-source project it found 443 API syntax candidates but **0 direct-literal address witnesses across all 11 pinned target sites and 3 Stage67/70 risks**. This does not mean there are no installed or executable hooks. They may derive target pointers through local variables, helpers, macros, or separate functions.

Stage161 adds **bounded same-file source-only assignment-chain hints**:
- For a relevant RVA literal in the source, recognizes some **single-line RHS assignments** with a simple syntactic variable name and 0/1/2 subsequent alias assignments.
- A candidate requires an assignment before the API call, a source distance of **at most 200 lines per hop**, **at most 3 assignment hops**, and syntactically balanced API call/address operands inherited from Stage160. It does not cross source files, functions, macros, dynamic return paths, CFG branches, exceptional control flow, or prove actual reaching definitions.
- Reassigned or shadowed names produce `MULTIPLE_ASSIGNMENTS_OR_SHADOWING` rather than a unique hint; a preprocessor directive in the source span blocks the apparent candidate; unsupported constructs remain unknown. Values from `memcpy` **source** operands are never treated as write destinations; a direct literal from Stage160 is counted there, not inflated into a new Stage161 assignment hint.
- Outputs **relative file path, stage, RVA, assignment/API line numbers, API name/role, hop count and reason tags ONLY**; never exports source code, variable identifiers, private expressions or memory bytes. Every candidate has `runtime_dataflow_proven=false`, `live_hook_owner_proven=false`. Even a candidate does not establish an installed hook or safe native return/rollback.

## Inputs and source integrity

Reads only user-local current project sources and existing prior metadata:
- `%TEMP%\ArcanaAI_Stage156_SourceOnly\stage156_current_source_inventory.json`;
- `%TEMP%\ArcanaAI_Stage158_r2_SourceOnly\stage158_r2_native_lifecycle_metadata.json`;
- `%TEMP%\ArcanaAI_Stage159_SourceOnly\stage159_native_provenance_metadata.json`;
- `%TEMP%\ArcanaAI_Stage160_SourceOnly\stage160_writer_dataflow_metadata.json`.

Before the extra scan, the Stage160 report is **recomputed from current source** and reconciled against the saved user's Stage160 metadata: **66** hashes, **11** target declarations + **3** risk sites, 443 API candidates and zero direct operand witnesses *if unchanged*, all API role ledgers and target gap records. Stale SHA, forged source/worker/VEH/writer metadata or unsafe live-proof values lead to fail-closed `REVIEW_REQUIRED`. Output goes **only** to `%TEMP%\ArcanaAI_Stage161_SourceOnly\stage161_alias_lineage_metadata.json`. No source changes, no native build, no game process or DLL I/O.

## Release integrity — actually verified

- ZIP `ArcanaAI_Stage161_LocalAliasLineage_SOURCE_ONLY.zip`.
- Size **34,713 bytes**, SHA256 **`454ef0f329eba259ebf1f1ae0f399767acc948ba8e801be21b3405d78e550122`**.
- **11 ZIP entries** (10 payloads + manifest), archive CRC PASS.
- Fresh clean ZIP extraction, run `verify_manifest.py`: `STAGE161_MANIFEST_PASS files=10` (all SHA and sizes match).
- On that fresh extraction, Python stdlib regression suite **99/99 PASS** (73 inherited Stage159 r2 + Stage160, plus 26 new Stage161) in 7.867s. Negative cases include single/2/3-hop assignment, 4-hop denied, repeated assignment ambiguity, source operand vs destination, direct literal not double-counted, post-API assignment denied, #if span, comment/string fakes, forged Stage160 JSON/safety gates, SHA drift, code leak prevention, output-inside-project rejection, and unchanged Stage67/70 rollback DENY.
- Includes Python audit scripts Stage159/160/161, PowerShell wrapper, manifest verifier, three test modules, Russian docs and README. **No original user C++ or native binaries included.**

## Next Windows SOURCE_ONLY check — NOT YET RUN

Extract ZIP, open a normal PowerShell inside `Stage161_SourceOnly`:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage161Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Expected if upstream ledgers match (numbers N/M unknown, **do not invent**):

```text
STAGE161_MANIFEST_PASS files=10
Ran 99 tests ... OK
STAGE161_HASHES matched=66 expected=66
STAGE161_BASELINE stage160_api_candidates=443 direct_rva_operands=0
STAGE161_ALIAS_CANDIDATES bounded=N blocked=M proven_writer_target_edges=0
STAGE161_SOURCE_ONLY_PREFLIGHT_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
STAGE161_LOCAL_SOURCE_ONLY_TESTS_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
```

Zero alias candidates is a **valid successful static preflight**, not evidence hooks are absent; in that case targeted manual source-slice review with immutable SHA should be considered before implementing heavier symbolic dataflow.

## Hard safety boundary

**`SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`**, Stage36 OFF, origin(0x27)=UNKNOWN. Raw/foreign native callers, actual patch-byte writers and rollback, trampoline stolen-byte return, native ABI/SEH/unwind/pending frames, all nine worker stop/join, all three VEH unregister/future dispatch drain and external owner module lease remain UNKNOWN. Preserve working Stage129 DLL **2,099,200 B** and Stage122 fallback **2,089,984 B** without changes. The Stage161 audit does not patch source, write game memory, insert JMP, call Arm, install hooks, or FreeLibrary.
