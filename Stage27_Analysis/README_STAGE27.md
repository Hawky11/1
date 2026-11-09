# Stage27 — static bit0 lifecycle and caller analysis

**Status:** offline evidence analysis from the verified Stage26 run. **No new DLL, no MSVC compilation, and no gameplay run** were made for Stage27. Reuse the archived Stage26 source/dinput8.dll if gameplay is needed; Stage27 changes nothing in the game.

## Results

- All 140 Stage26 records form 70 alternating sets and clears on the proper P1/P2 object fields; detailed pairs in `cycles_stage26.csv`.
- Primary clear instruction `AALib+0x97873` accounts for 54/70 clears, reached through two different observed return addresses: RVA `0x979D1` (34) and `0x9760E` (20).
- The other writer sites, complete signature bytes and observed registers are in `STAGE27_LIFECYCLE_ANALYSIS.md`.
- Only two setter records have EBX=`0x1E`, and they match the two P2→P1 type1/payload0x1E events.

## Reproduce the analysis

Using Python 3 and this folder, run:

```powershell
py -3 .\Stage27_Analysis\tools\analyze_bit0_lifecycle.py --log .\ArcanaAI_Stage26ObjectBit0WriteTrace.log --json .\stage27_cycles.json --markdown .\stage27_recomputed.md
py -3 -m unittest discover -s .\Stage27_Analysis\tests -p "test_stage27.py" -v
```

The log file is already at the root of the evidence branch. The Python analyzer uses only the standard library.

### Optional exact-binary static check

**If available locally**, run this against the installed *original* `AALib.exe`:

```powershell
py -3 .\Stage27_Analysis\tools\analyze_bit0_lifecycle.py --log .\ArcanaAI_Stage26ObjectBit0WriteTrace.log --binary "PATH\TO\AALib.exe" --static-report .\Stage27_AALib_StaticReport.txt
```

The tool validates SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, verifies exact game writer bytes, confirms the entry signature of the 0x97870 helper, and checks candidate E8 CALL instruction bytes/targets at RVA `0x97609` and `0x979CC`. Without Capstone installed, it still emits raw hex windows. With optional Capstone installed, it additionally prints disassembly. If the hash differs, the script stops rather than making unsafe assumptions.

**Do not upload the full game executable to GitHub.** The text file `Stage27_AALib_StaticReport.txt` is enough for the next investigation and can be shared in the conversation or placed into a new evidence branch.

## Project bookkeeping

- Stage26 source: `stage26-object-bit0-write-trace`, commit `a249f452353c340fe8f1dab23375a8a932b079b4`.
- Stage26 preserved runtime: `stage26-object-bit0-write-runtime-evidence`, commit `4b7564ff71664fedd5745e3bcfc4210d74540075`.
- Stage27 has analysis scripts, derived reports and tests only. Its new branch is a descendant of the Stage26 runtime evidence; no historical branch was rewritten.
- Python tests are shipped but **have not been executed in a user Windows environment**; actual optional binary validation awaits the pinned PE.
