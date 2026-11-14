# ArcanaAIOverhaul — Stage160 bounded writer address-operand source evidence and dataflow gaps (2026-11-14)

**STATUS: Stage160 package completed and independently re-tested on freshly extracted ZIP; 73/73 Python tests PASS; current user's Windows Stage160 project audit NOT YET RUN.** The most recent *actual* Windows result is Stage159 r2 SOURCE_ONLY PASS (43/43 tests, 66/66 source hashes, 11/11 target source RVA declarations, 418 lexical write-API candidates, **0 proven writer→target edges**).

## Delivered Stage160 artifact

- Chat ZIP `ArcanaAI_Stage160_NativeWriter_OperandDataflow_SOURCE_ONLY.zip`, **27,945 bytes**, SHA256 **`db9eaed266575ba4f24b375045451e577b64042e4e29bd2044ab45c04dd44cc3`**.
- Archive has **9 entries**, ZIP CRC **PASS**. `verify_manifest.py` was run on a **fresh clean extraction**: `STAGE160_MANIFEST_PASS files=8`, all 8 SHA256/byte counts verified. Freshly extracted synthetic tests `Ran 73 tests ... OK` (**73/73 PASS**). Tests consist of **43 inherited Stage159 r2 tests** including Windows non-admin `WinError 1314` simulated fallback, and **30 new Stage160 negatives and source-operand tests**. Stage159 legacy scanner code is copied unchanged into artifact.
- Payload: `Stage160_SourceOnly/stage160_writer_dataflow_audit.py` (new), `stage159_provenance_audit.py` (read-only prior source scanner), `Invoke-Stage160Audit.ps1`, `verify_manifest.py`, `tests/test_stage159.py`, `tests/test_stage160.py`, `README.md`, `docs/STAGE160_AUDIT_RU.md`, `manifest.json`. No user's original C++, binaries, INI, private logs or keys in archive.

## Input and proof model

Reads *only* current user-local source files and historical source-only metadata:

1. `%TEMP%\ArcanaAI_Stage156_SourceOnly\stage156_current_source_inventory.json`;
2. `%TEMP%\ArcanaAI_Stage158_r2_SourceOnly\stage158_r2_native_lifecycle_metadata.json`;
3. `%TEMP%\ArcanaAI_Stage159_SourceOnly\stage159_native_provenance_metadata.json` (last user-confirmed Stage159 r2 run; r2 uses same metadata path as r1).

Read-only Stage159 preflight is recomputed against current **66** reachable source file SHA/size, eight compile roots and all eleven pinned targets. New Stage160 independently compares user-local Stage159 metadata to freshly recomputed Stage159 ledger: source hashes, target lines/declared lengths, writer candidate counts, Stage67/70 risk anchors, nine worker identities and three VEH anchors. Any stale/spoofed ledgers or unsafe prior live gate claims `true` trigger `REVIEW_REQUIRED`. No command under the game project is ever written by the package. Output only at `%TEMP%\ArcanaAI_Stage160_SourceOnly\stage160_writer_dataflow_metadata.json`.

## Stage160 new semantics — address argument vs same-file proximity

- Stage159 counted **418** lexical `VirtualProtect[Ex]`, `WriteProcessMemory`, `memcpy/memmove` and `FlushInstructionCache` candidates across all 66 files. This is a **prior Windows result**, not remeasured Stage160.
- Stage160 recognizes source API-like calls to `memcpy`, `memmove`, `memset`, `WriteProcessMemory` (potential write destination), `VirtualProtect/VirtualProtectEx` (address with protection role), `FlushInstructionCache` (flush address) and attempts a **balanced-operand source parse** up to 24k chars per match, across multiline calls and nested parentheses/brackets/braces.
- Detects whether one of **14** source-pinned target literals (the 11 Stage10/Parent/129/81 + three Stage67/70 declared rollback risks) is present as a *whole C++ hex token in the API's address argument*, not merely 60 lines away. Each match is emitted as `SOURCE_API_ADDRESS_ARGUMENT_LITERAL`, with only source relative path, line, API, role, RVA, and counts in the metadata; no source code, argument strings or user constants are copied.
- A literal in `memcpy(dst, src+RVA, n)` is **NOT** a writer destination witness, but `memcpy(dst+RVA,src,n)` is a **source argument witness**. That witness is still NOT enough for dataflow proof, runtime installation, native owner, trampoline or rollback. Preprocessor macros, unknown computed aliases, function declarations, other patch helpers and inline asm are out of scope; `UNPARSED_UNKNOWN` never elevates safety.
- For each target the following remain `UNKNOWN` even if source witnesses exist: pointer alias dataflow, executable writer call reachability, actual patch bytes and restored originals, trampoline replay, native ABI/SEH/unwind/pending frames, foreign/raw callers and live owner.
- Report always includes `proven_writer_to_target_dataflow_edges=0`, `proven_trampoline_replay_edges=0`, `proven_native_raw_caller_ownership_edges=0`, `nine_worker_stop_and_join=NO`, `three_veh_future_dispatch_retirement=NO`, `module_lease_and_transaction_rollback=NO`, and `stage67_70_rollback_risk=STILL_UNPROVEN`.

**This is a stronger source-level observation than Stage159 proximity but STILL NOT complete writer→RVA dataflow analysis or proof of patch installation.** The Stage160 pass marker means **source-only preflight pass** — never `SAFE_TO_INSTALL=true` or unload approval.

## Tests and known limitations

- **73/73** offline tests passed on the unpacked release, including the 43 Stage159 r2 regression cases and 30 Stage160 cases: direct vs source operand, protection vs byte write vs flush, multiline nesting, macros/unparsed expressions, duplicate RVA literals in one call, nonexistent source paths, SHA drift, forged Stage159 targets/worker/VEH and live-safety flags, metadata-only outputs, output-inside-project denial, Stage67/70 syntactic write not treated as proven rollback.
- Source masked with Stage159's UTF8 C++ comment/literal rules; raw C++ strings are deliberately unsupported and fail-closed. No real Windows reparse-point privilege test; the Stage159 symlink negative includes WinError1314 safe simulation for non-admin Windows, which is not full junction coverage.
- **Current user's Windows Stage160 not executed yet.** No claims of known number of source operand witnesses on user's tree, no claim of Stage160 hash PASS until console shown, no Game DLL tested.
- A true call with target computed via variable, macro or helper may yield **zero direct target-literal witnesses** even though a writer exists. This is NOT a source mismatch or evidence that no hook exists. The user should share the console summary; targeted source-slice review can then establish real writer expression/alias linkage.

## Windows command — pending

From extracted `Stage160_SourceOnly`, in ordinary Windows PowerShell (no Admin, no MSVC, no game needed):

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage160Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Expected **if preflight matches** (not claimed observed): `STAGE160_MANIFEST_PASS files=8`, `Ran 73 tests ... OK`, `STAGE160_HASHES matched=66 expected=66`, `STAGE160_DECLARED_SITES current=11 risk_stage67_70=3 ...`, `STAGE160_WRITE_OPERANDS direct_source_literal_witnesses=N writer_target_dataflow_proven=0`, and `STAGE160_LOCAL_SOURCE_ONLY_TESTS_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false`.

## Safeguards / next Stage161

**`SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`**, Stage36 OFF, `origin(0x27)=UNKNOWN`. Preserve working Stage129 game DLL **2,099,200 B** and Stage122 backup **2,089,984 B** unchanged. No native build, game process, register writes, patch installation, game memory readback or FreeLibrary.

After actual Windows Stage160 output, prioritize a tightly bounded source-slice audit of target pointer variable assignment and effective `memcpy(dst)/WriteProcessMemory(address)` argument derivation for selected Stage10/129 hooks; never attribute an owner from mere lexical closeness or module/RVA name. Independently track Stage67/70 partial rollback, nine worker joins, three VEH retirement and all raw/foreign entrypoint provenance.
