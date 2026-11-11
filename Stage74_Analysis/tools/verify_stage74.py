#!/usr/bin/env python3
"""Stage74 source-only guard: prove the Stage38 replacement is exactly four edits
against the frozen Stage70 r2 source. No DLL, game, debugger or memory writes.

Usage:
  py -3 Stage74_Analysis/tools/verify_stage74.py
  py -3 Stage74_Analysis/tools/verify_stage74.py --exe "D:\\PATH\\AALib.exe"
  py -3 Stage74_Analysis/tools/verify_stage74.py --overlay "D:\\PATH\\Stage38CallbackWindow.inl"
"""
import argparse
import hashlib
import json
from pathlib import Path

BASE_GIT_BLOB="ed6065fd5877cb5e1ab634493c07b32f054510bf"
PATCHED_GIT_BLOB="106dca56d662fe1c8e0b7083013a535b67cde3c8"
EXE_SHA="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"

PATCHES=[
    ("constexpr DWORD kStage38Fields = 30u;",
     "constexpr DWORD kStage38Fields = 32u;"),
    ("        0x80u,0x88u,0x8Cu,0x184u,0x278u,0x27Cu",
     "        0x80u,0x88u,0x8Cu,0x184u,0x278u,0x27Cu,\n"
     "        0x3F50u,0x3F54u // Stage74: P selector and companion state (f30/f31)"),
    ("        if (i>=15u && object==0) continue;",
     "        if (i>=15u && i<30u && object==0) continue;"),
    ("        if (SafeReadDword((i<15u ? player : object)+offsets[i], &values[i])) {",
     "        if (SafeReadDword(((i<15u || i>=30u) ? player : object)+offsets[i], &values[i])) {"),
]

def git_blob(data):
    header=b"blob "+str(len(data)).encode("ascii")+bytes((0,))
    return hashlib.sha1(header+data).hexdigest()

def check_source(path):
    raw=path.read_bytes()
    current=git_blob(raw)
    if current!=PATCHED_GIT_BLOB:
        raise ValueError("Stage74 source drift, git blob "+current)
    source=raw.decode("utf-8")
    for old,new in PATCHES:
        if source.count(new)!=1:
            raise ValueError("Expected one Stage74 patch: "+new)
        source=source.replace(new,old)
    recovered=source.encode("utf-8")
    baseline=git_blob(recovered)
    if baseline!=BASE_GIT_BLOB:
        raise ValueError("Recovered Stage70 r2 source mismatch: "+baseline)
    return {
        "status":"SOURCE STATIC VERIFIED; Windows x86 build and runtime NOT performed",
        "stage70SourceBlob":baseline, "stage74SourceBlob":current,
        "exactEdits":4,"originalFieldCount":30,"newFieldCount":32,
        "newFields":{"f30":"P+3F50 selector","f31":"P+3F54 state"},
        "existingF0ToF29Preserved":True, "newHooks":False,
        "newDebugRegisterOwners":False, "nativeExecutionValidated":False,
    }

def main():
    p=argparse.ArgumentParser(description=__doc__)
    default=Path(__file__).resolve().parents[2]/"Stage74_Overlay/src/Stage38CallbackWindow.inl"
    p.add_argument("--overlay",type=Path,default=default)
    p.add_argument("--exe",type=Path,help="Optional original private AALib.exe; hash only")
    args=p.parse_args()
    result=check_source(args.overlay)
    if args.exe:
        digest=hashlib.sha256(args.exe.read_bytes()).hexdigest()
        if digest!=EXE_SHA:raise ValueError("Unexpected original AALib.exe SHA-256: "+digest)
        result["originalExeSha256"]=digest
    print(json.dumps(result,indent=2))
    print("PASS: Stage74 source-only four-edit comparison. DLL build/runtime pending.")

if __name__=="__main__":
    main()
