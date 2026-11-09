# Stage27 — verified static CALL targets and follow-up byte export

This branch preserves the previous Stage27 and Stage26 archives and adds the **exact original user-uploaded static report**, the corrected call graph, and a new read-only binary inspector. No new runtime game hook or DLL build is involved.

## Corrected call graph

- `CALL RVA 0x979CC → RVA 0x97870` is **verified**.
- `CALL RVA 0x97609 → RVA 0x97970` is **verified**; it does **not** directly call the `0x97870` clearer.
- The link from `0x97970` to the `0x97870` clearer is not fully decoded. Stage26 observed 34 stack returns `0x979D1` and 20 `0x9760E`. These are dynamic observations, not proof of the missing branch.
- Read `VERIFIED_CALL_GRAPH.md` for instructions and the additional reconstructed state changes at `0x97870`, `0x975A6`, `0x95F55` and `0x97031`.

## One-command follow-up (PowerShell from D:\INCUBATOR)

First download only the updated inspector:

```powershell
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/Hawky11/1/stage27-verified-static-callgraph/Stage27_VerifiedStatic/tools/inspect_call_flow.py" -OutFile ".\inspect_call_flow.py"
```

Then run it on the same SHA256-pinned local EXE:

```powershell
py -3 .\inspect_call_flow.py --binary "G:\Arcana Heart 3 - LS XTEND\AALib.exe" --output ".\Stage27_CallFlow_97970.txt"
```

It produces `D:\INCUBATOR\Stage27_CallFlow_97970.txt`, including raw bytes of the missing function `0x97970`. Upload only this TXT (not the full game EXE). No new COM-vs-COM bout or Build.bat is necessary.

For development regression tests after downloading the branch:

```powershell
py -3 -m unittest discover -s .\Stage27_VerifiedStatic\tests -p test_inspect_call_flow.py -v
```

The five tests passed in the assistant's local Python container against synthetic data; original EXE inspection is pending. See `LOCAL_TEST_RESULTS.txt`.
