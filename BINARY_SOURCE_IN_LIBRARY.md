# AALib.exe original game binary — location for future sessions

**IMPORTANT: READ THIS BEFORE ASKING THE USER FOR GAME EXECUTABLE OR WRITING AN EXE EXTRACTION SCRIPT.**

The exact original unmodified Arcana Heart 3: Love Max Six Stars!!!!!! XTEND / Arcana Heart 3 - LS XTEND game executable **is already stored in the user's persistent ChatGPT File Library**, not in GitHub. The user explicitly confirmed this on 2026-11-09 and asked future assistants to use it automatically.

## File Library retrieval — critical procedure

1. Use the ChatGPT `files__list` tool, `{"surface":"library","library_path":"/","recursive":true,"limit":100}` (paginate if necessary), to find **`/AALib.zip`**. Alternatively list the Library root directly; the library file had size **12,865,549 bytes** when verified.
2. Take the **actual file_id returned by this session's files tool**; never guess a mount path or assume a prior session's file_id will remain available. Use `files__materialize` with `{"items":[{"file_ref":{"file_id":"<actual-file-id>"},"outputs":[{"representation":"raw_file"}]}]}`.
3. The materialized ZIP contains exactly **`AALib.exe`** (uncompressed 175,556,608 bytes). Extract in the private analysis runtime with Python's `zipfile` or stream individual bytes (e.g. by opening the member). No need to ask the user to re-upload, use their machine, run another PowerShell export, or copy the entire binary to GitHub.
4. **Verify SHA-256 of decompressed AALib.exe before binary analysis**: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. This exact fingerprint was confirmed in Stages16,25,27,28. If another version appears, stop and explain the discrepancy before applying old RVAs.
5. The executable is x86 PE32, preferred image base `0x00400000`. The `.text` section starts at **RVA `0x00001000`**, raw file offset **`0x00000400`**, and virtual size `0x002BDEBE`. **An RVA is not a file offset**: translate using the PE section table before reading.
6. For disassembly use `objdump -D -b binary -m i386 -M intel --adjust-vma=0x1000` on the extracted `.text` bytes, or `capstone` / `pefile` if installed. Keep the original EXE private; commit to GitHub **only relevant code snippets, xref lists, source tools, and textual analysis**, not AALib.exe or the ZIP.

## Provenance and caveats

- Source: **user's ChatGPT Library `/AALib.zip`**, not GitHub, not the local `D:\INCUBATOR\ArcanaAIOverhaul` source tree. ChatGPT Library files are session-accessible through `files__list` + `files__materialize`; the prior local materialization path `/mnt/data/files/AALib.zip` is **session-specific**, do not assume it persists.
- First successfully accessed for Stage28 on 2026-11-09; exact ZIP file name, inner member, size, PE metadata, and SHA verified.
- Read `NEXT_SESSION.md` and `PROJECT_HISTORY.md` for the active diagnostic stage and immutable source/evidence branches before modifying any code.
- NEVER instruct the user to supply the full original EXE again without first checking the persistent Library. If the Library is temporarily unavailable, state the access problem instead of claiming the file is absent.
