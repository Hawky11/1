# Stage28 — verified offline state rearm analysis

**No new game patch or runtime build was needed.** The original user's game EXE was found in ChatGPT Library (`AALib.zip`) and analyzed locally. It is **not** included in this branch.

Full report: `STATIC_STAGE28_ANALYSIS.md`.

The fingerprinted PE SHA256 matches exactly: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Static verifier checked 26 code signatures, 16 direct CALL targets, and 1 tail JMP. Six tests passed locally, including one test against the exact original PE. The two Stage26 bit0 boundary values were always 0/1; bit1 remains unproven dynamically because transition-only filtering may omit writes that leave bit0 unchanged.

**Core finding:** `0x978B0` consumes bit1 in object+0xC8, compares selected fighter's record against `(type=0,code=0x13/0x14/0x15)` using `0xF2ED0` (predicate, not writer), or waits on a counter through `0x991E0`. It can call `0x97830` to write state1. `0x97870` writes state2 and clears bit0. The two Stage27 CALL/tail JMP branches remain proven.

### Re-run verification locally if desired

From the downloaded branch folder:

```powershell
py -3 .\Stage28_Analysis\tools\verify_stage28.py --binary "G:\Arcana Heart 3 - LS XTEND\AALib.exe" --output .\Stage28_Verification.json
py -3 -m unittest discover -s .\Stage28_Analysis\tests -p test_stage28.py -v
```

The second command runs five synthetic tests and skips the real-PE test unless environment variable `STAGE28_AALIB_EXE` points to the original binary. Set that variable to enable all six. These steps are **optional**; the assistant already verified the exact PE from the Library. No need to run `Build.bat` or start the game.

This branch derives from Stage27 `stage27-bit0-tailjump-verified`. All Stage25–27 source/evidence branches remain unchanged.
