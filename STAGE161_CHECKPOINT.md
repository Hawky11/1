# ArcanaAIOverhaul — Stage161 bounded local assignment-alias source hints (2026-11-14)

**Status: Stage161 release built and independently re-tested after clean ZIP extraction, 99/99 synthetic/negative tests PASS; user Windows Stage161 current-tree audit NOT YET RUN.** The **actual** Stage160 Windows source-only audit is confirmed PASS (73/73, 66/66, 443 source API call candidates, 0 direct RVA address operand tokens).

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
