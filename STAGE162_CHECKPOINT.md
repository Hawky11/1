# Stage162 — Stage67/70 focused write order, alias provenance, and partial-failure review (2026-11-14)

**Status: Stage162 USER WINDOWS SOURCE_ONLY PASS (15/15 tests and 12/15 ledger reconciliation).** No game DLL/process access, no native build, no source edits, no installation/unload. `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`; Stage36 OFF, origin(0x27)=UNKNOWN.

## Evidence and scope

- Actual user Stage161 Windows console: manifest 10/10, Python 99/99, 66/66 source hashes, 12 bounded same-file alias candidates (4 per Stage67 `AD75C`, Stage67 `E9E60`, Stage70 `156784`), 15 blocked/ambiguous, **0 proven writer→target edges**. Full local Stage161 JSON was NOT supplied to the chat; it remains `%TEMP%\ArcanaAI_Stage161_SourceOnly\stage161_alias_lineage_metadata.json`.
- An already uploaded **Stage146 full-current-source evidence snapshot** was inspected locally (not published in repository). Running the exact Stage161 lexical matcher over this archived snapshot independently reproduced **12 bounded + 15 blocked**. The 15 blocked are 6 Stage129 and 9 Stage81 records, each `MULTIPLE_ASSIGNMENTS_OR_SHADOWING`; they are not Stage67/70 hooks and do **not** prove absence of other writers.
- The Stage146 snapshot may differ from the user's current Windows tree. To avoid false proof, Stage162 requires exact current SHA-256 matches for only `src/Stage67NativePaths.inl` (`f919af18379d85594c442e021b559a298b5a068b06ce7ebefc33f0cde94d3620`), `src/Stage70DeferredWriter.inl` (`bf3becbf1893426002fb3dfee0eb849caf1836c2d42059379c40958b7995b8df`), and `src/Stage38CallbackWindow.inl` (`d0ad34650a94102e6c9dfc8b74228cf841510d04a690739dd11933af2e51557a`). Stage162 also checks the exact twelve Stage161 record lines/API roles and fifteen blocked flags. Mismatch => `REVIEW_REQUIRED`.

## Three RVA assignments and native writer sequence (archived-source evidence only)

| Target | Assignment line | API lines: enable-write → byte-write → flush → restore-protect | Byte write |
|---|---:|---|---|
| Stage67 `0xAD75C` | Stage67:256 | 270 → **275** → 276 → 282 | `memcpy` 5 B, CALL rel32 |
| Stage67 `0xE9E60` | Stage67:257 | 271 → **275** → 277 → 281 | `memcpy` 6 B, JMP rel32 + NOP |
| Stage70 `0x156784` | Stage70:212 | 227 → **231** → 232 → 235 | `memcpy` 6 B, JMP rel32 + NOP |

Each group of four Stage161 candidates represents **one destination alias used in four APIs**, *not four writes or four installed hooks*. The three **actual code byte-copy operations** are two `memcpy` calls at Stage67:275 (in order AD75C 5 B, E9E60 6 B) and one at Stage70:231 (156784 6 B). Confirmed by inspecting the Stage146 source and reproduced syntactic witnesses; source-order is not runtime owner/atomicity proof.

### Source-level transaction and failure logic

- **Stage67**: prerequisites/signatures at 249–254; pointer definitions 256–257; construct patch operands 258–261; mutex 262–265; suspend + EIP reject 266–267; `VirtualProtect` both pages 270–271; recheck signatures at 273; `memcpy(a,pa,5)` then `memcpy(b,pb,6)` at 275; two cache flushes 276–277; set installed state `2` at 278; restore protections b then a at 281–282; resume/release 283. If an ordinary API cache flush or protect restoration fails **after writes**, lines 285–287 return `false` without restoring original code bytes, with installed flag already set. A fault between two `memcpy` calls could leave the first 5 B patched; this exception path has no visible recovery handler in this installer (fault occurrence was **not** observed). If second protect acquisition fails, normal path does not enter either `memcpy`; failure to restore first page's protection is still a separate risk.
- **Stage70**: prerequisites/signature 190–210 (requires Stage67 installed state 2); destination 212; mutex 220–223; suspend + EIP check 224–225; protect 227; recheck 229; `memcpy(a,patch,6)` 231; cache flush 232; set installed state 1 at 233; restore protection 235; resume/release 236. On flush/protection restoration failure **after byte write**, 241–243 return `false` without restoring bytes, with flag set. Fault mid-`memcpy` is not handled transactionally in this installer.
- **Stage38 integration**: Stage38 lines 333–346 gate Stage70 invocation on the **boolean return** of Stage67, while `StartStage59Watch()` is outside that Stage67-success conditional. Therefore a Stage67 post-write false can skip Stage70 but still reach Stage59. This is source-path possibility, not observed game execution.
- No verified full rollback transaction, original-byte snapshot, handler-based recovery, foreign hook ownership, trampoline ABI/SEH/unwind, nine-worker join, three-VEH retirement, or module lease. Do not mistake page-protection restoration for restoration of original code bytes.

## Deliverable and tests

New chat artifact: `ArcanaAI_Stage162_Stage67_70_WriteOrder_PartialFailure_SOURCE_ONLY.zip`, **9,205 B**, SHA256 `f9adddc5d77eb70ab05e64661f4808095fcc52282e7c190e330eb1fc716e1dff`; six ZIP entries, CRC PASS, **5/5 SHA manifest**, **15/15 local synthetic and adversarial tests PASS** after clean extraction. Independent historical Stage146 source-only preview also PASS (12/15 candidate metadata and 3/3 pinned file hashes). ZIP contains only Python/PowerShell/README/tests/manifest; no private game C++ or DLLs.

**Historical Windows run instructions (already executed successfully)** (extract ZIP, CD to `Stage162_SourceOnly`):

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force
.\Invoke-Stage162Audit.ps1 -ProjectRoot 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Output: `%TEMP%\ArcanaAI_Stage162_SourceOnly\stage162_targeted_write_order_metadata.json`. `TARGETED_SOURCE_ONLY_PASS` means only that the **current** targeted files match the archived source hash pins and Stage161 metadata witness lines; it does not permit install/unload. If drift, inspect only relevant lines; no broad 443-API rerun.

**Follow-up after Windows PASS:** Stage163 should address source-level rollback transaction obligations and error-path recovery in a separate **design/source-only** analysis (not modify production game code without explicit direction), while keeping native owner/worker/VEH lifetime gates blocked. Stage129 working DLL (2,099,200 B) and Stage122 rollback DLL (2,089,984 B) unchanged.


---

## User Windows execution — PASS (2026-11-14)

**CURRENT STATUS: Stage162 has now been run by the user on the actual Windows source tree and PASSED.** Original user PowerShell console (provided in conversation, not an uploaded full metadata file) records:

```text
STAGE162_MODE SOURCE_ONLY READ_ONLY NO_GAME_DLL_ACCESS NO_NATIVE_BUILD
STAGE162_MANIFEST_PASS files=5
Ran 15 tests in 0.056s
OK
STAGE162_RECONCILE bounded=12 blocked=15
STAGE162_TARGET stage=STAGE67 rva=AD75C length=5 source_writer_target=HASH_PINNED_CPP_SOURCE_ONLY rollback=NO
STAGE162_TARGET stage=STAGE67 rva=E9E60 length=6 source_writer_target=HASH_PINNED_CPP_SOURCE_ONLY rollback=NO
STAGE162_TARGET stage=STAGE70 rva=156784 length=6 source_writer_target=HASH_PINNED_CPP_SOURCE_ONLY rollback=NO
STAGE162_METADATA C:\Users\Noma\AppData\Local\Temp\ArcanaAI_Stage162_SourceOnly\stage162_targeted_write_order_metadata.json
STAGE162_TARGETED_SOURCE_ONLY_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
STAGE162_LOCAL_SOURCE_ONLY_TESTS_PASS SAFE_TO_INSTALL=false SAFE_TO_UNLOAD_GAME_DLL=false
```

- The Windows run confirms the package manifest (5/5), all **15/15 synthetic/negative tests**, the exact Stage161 `bounded=12 blocked=15` reconciliation, and **three source SHA-pinned RVA writer candidates** with intended lengths 5, 6, 6. The audit implementation pins `Stage67NativePaths.inl`, `Stage70DeferredWriter.inl`, `Stage38CallbackWindow.inl`; this supports applying the archived-source findings below to the current **matching source text**. This is *not* an independently uploaded full Stage162 JSON or a runtime/game test.
- `source_writer_target=HASH_PINNED_CPP_SOURCE_ONLY` establishes source provenance under the exact pinned file/content and Stage161 witness record contract, not installed patch state, executable call path/owner, runtime atomicity, or safety.
- All three report **`rollback=NO`**. The Stage67 two-write order and Stage70 single write, post-write error paths, and Stage38 caller implications require further source-level failure modelling. In particular, a failed cache flush or protection restore after bytes have changed is not undone by restoration of page protections.
- New NEXT: **Stage163 SOURCE_ONLY fault-state/transaction obligations matrix**. Do not repeat the broad source scan; build a *per-site state machine* from Stage67/70 and Stage38 exact lines: protection obtained, first write committed, second write committed, ICache flush outcomes, installed flags, reverse-order protect restoration, suspend/resume/mutex cleanup, Stage38 admission. Distinguish ordinary API-return failures, C++ exceptions/SEH, partial memory-copy faults and asynchronous thread execution; model rollback only when original bytes are snapshotted, verified and safely restorable. Fail-closed on current-source SHA drift; no live patching, no DLL/source modification, no ownership claims.
- Persistent hard gate: `SAFE_TO_INSTALL=false`, `SAFE_TO_UNLOAD_GAME_DLL=false`, Stage36 OFF, origin(0x27)=UNKNOWN. Raw/foreign callers, trampolines/ABI/SEH/unwind, nine worker stop/join, three VEH future dispatch retirement, true installed-byte ownership, external module lease and complete rollback remain UNKNOWN/NO. Prior working Stage129 and fallback Stage122 DLLs should remain untouched.

---

