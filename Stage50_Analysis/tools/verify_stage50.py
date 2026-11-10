#!/usr/bin/env python3
"""Stage50: SHA-pinned PE32 native coordinate-store and source adapter checks.

Original AALib.exe is already in the user's Library; do not ask to reupload.
This verifier does NOT claim a Windows x86 build or installed runtime hooks.
"""
from pathlib import Path
import argparse,hashlib,struct

SIZE=175556608
SHA256="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
SIG={
  0xAF130:"837e38ff741a",        # ESI+38 active loop guard
  0xAF136:"e8c5efffff",          # native AE100
  0xAF13B:"8bce",                # MOV ECX,ESI -> subsequent Stage49 AF13D
  0xAE100:"558bec83ec08837e3c04", # native prologue + type guard
  0xAE114:"39bea4010000",        # code==0x32
  0xAE1A6:"d9567c",              # object+7C
  0xAE1A9:"d99eac000000",        # object+AC
  0xAE1B8:"d99680000000",        # object+80
  0xAE1BE:"d99eb0000000",        # object+B0
  0xAE216:"c3",                  # no-args RET
}
def check_pe(path:Path):
    b=path.read_bytes()
    assert len(b)==SIZE and hashlib.sha256(b).hexdigest()==SHA256, "original EXE size/SHA mismatch"
    pe=struct.unpack_from("<I",b,0x3c)[0]
    assert b[:2]==b"MZ" and b[pe:pe+4]==b"PE\0\0"
    assert struct.unpack_from("<H",b,pe+24)[0]==0x10b
    assert struct.unpack_from("<I",b,pe+52)[0]==0x400000
    n=struct.unpack_from("<H",b,pe+6)[0]
    start=pe+24+struct.unpack_from("<H",b,pe+20)[0]
    sections=[]
    for i in range(n):
        off=start+40*i
        vsize,va,rs,raw=struct.unpack_from("<IIII",b,off+8)
        sections.append((va,rs,raw))
    def rd(rva,count):
        for va,rs,raw in sections:
            if va<=rva and rva+count<=va+rs:return b[raw+rva-va:raw+rva-va+count]
        raise AssertionError(f"no PE backing RVA={rva:08X}")
    for rva,pattern in SIG.items():
        expected=bytes.fromhex(pattern)
        assert rd(rva,len(expected))==expected,f"native mismatch RVA={rva:08X}"
    rel=struct.unpack("<i",rd(0xAF137,4))[0]
    assert 0xAF13B+rel==0xAE100,"CALL AF136 target mismatch"
    return len(SIG)

def check_source(root:Path):
    def get(p):return (root/p).read_text(encoding="utf-8")
    prior=get("Stage49_Overlay/src/Stage38CallbackWindow.inl")
    adapter=get("Stage50_Overlay/src/Stage38CallbackWindow.inl")
    source=get("Stage50_Overlay/src/Stage50PreLinkedCoordinate.inl")
    s49=get("Stage49_Overlay/src/Stage49LinkedMotion.inl")
    assert "2026-11-10-r2" in s49,"Stage49 r2 must be retained"
    assert "STAGE50_PRELINKED_COORD_BUILD=2026-11-10-r1" in source
    assert "const DWORD object=saved[1]" in source,"ESI is the original native object"
    assert "const BYTE original[]={0xE8,0xC5,0xEF,0xFF,0xFF};" in source
    assert "g_gameBase+0xAE100u" in source
    assert "r.CallerRva!=0xAF136u" in source
    assert source.count("MakeRelativeCall(")==1
    assert source.count("VirtualProtect(")==2
    assert "S50Snap(r.Object,r.Pre,r.PreMask)" in source
    assert "S50Snap(r.Object,r.Post,r.PostMask)" in source
    assert "gateCandidate=" in source
    assert "STAGE50_HEALTH" in source
    assert "CreateThread(" not in source
    body=source.split("void S50PreLinkedHook()",1)[1].split("__asm {",1)[1].split("\n    }",1)[0]
    for i in ("pushfd","popfd","pushad","popad","fxsave [edi]","fxrstor [edi]"):
        assert body.count(i)==2,i
    assert body.count("call S50Enter")==1
    assert body.count("call S50Leave")==1
    assert body.count("call dword ptr [g_s50Native]")==1
    assert body.rstrip().endswith("ret")
    expected=prior.replace(
        '#include "Stage49LinkedMotion.inl"',
        '#include "Stage49LinkedMotion.inl"\n#include "Stage50PreLinkedCoordinate.inl"')
    expected=expected.replace(
        "if(InstallStage48Hooks())InstallStage49Hooks();",
        'if(InstallStage48Hooks()){\n'
        '                                if(InstallStage49Hooks())InstallStage50Hooks();\n'
        '                                else S50Log("ERROR: Stage50 requires installed Stage49; no patches.");\n'
        '                            }')
    expected=expected.replace("    FlushStage49();\n","    FlushStage49();\n    FlushStage50();\n")
    assert adapter==expected,"Stage50 adapter modified prior working instrumentation unexpectedly"

if __name__=="__main__":
    p=argparse.ArgumentParser()
    p.add_argument("--binary",type=Path,required=True,help="original physical AALib.exe")
    p.add_argument("--root",type=Path,default=Path(__file__).resolve().parents[2])
    a=p.parse_args()
    n=check_pe(a.binary)
    check_source(a.root)
    print(f"PASS Stage50: PE32 SHA/size; {n} exact original signatures; CALL AF136->AE100")
    print("PASS Stage50: observer original-once ABI/source/adapter constraints")
    print("PENDING: Windows x86 Build.log, Stage50 install marker, COM-vs-COM runtime")
