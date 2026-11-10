#!/usr/bin/env python3
"""Stage51 exact SHA-pinned PE and Stage50-preserving adapter/source checks.

Static-only verifier; no claim about Windows x86 build/game runtime.
"""
from pathlib import Path
import argparse,hashlib,struct
SIZE=175556608
SHA="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
SIG={
0xAF117:"6a01",          # caller PUSH 1
0xAF119:"e802f1ffff",    # CALL native AE220
0xAF11E:"8b35",          # following MOV ESI,[abs], skip relocated operand
0xAF124:"83c404",        # caller cleanup one DWORD
0xAE220:"558bec51535657e84413ffff", # AE220 prologue
0xAE2B7:"e8c4190400",    # native internal call
0xAE6DD:"c3",            # native cdecl RET
}
def check_pe(path):
    b=path.read_bytes()
    assert len(b)==SIZE and hashlib.sha256(b).hexdigest()==SHA,"wrong physical AALib.exe"
    pe=struct.unpack_from("<I",b,0x3c)[0]
    assert b[:2]==b"MZ" and b[pe:pe+4]==b"PE\0\0"
    assert struct.unpack_from("<H",b,pe+24)[0]==0x10b
    assert struct.unpack_from("<I",b,pe+52)[0]==0x400000
    start=pe+24+struct.unpack_from("<H",b,pe+20)[0]
    sections=[]
    for i in range(struct.unpack_from("<H",b,pe+6)[0]):
        off=start+i*40
        vsize,va,rawsize,raw=struct.unpack_from("<IIII",b,off+8)
        sections.append((va,rawsize,raw))
    def rd(rva,n):
        for va,rawsize,raw in sections:
            if va<=rva and rva+n<=va+rawsize:return b[raw+rva-va:raw+rva-va+n]
        raise ValueError(f"RVA not backed: {rva:08X}")
    for rva,hexdump in SIG.items():
        expected=bytes.fromhex(hexdump)
        assert rd(rva,len(expected))==expected,f"native mismatch at {rva:08X}"
    rel=struct.unpack("<i",rd(0xAF11Au,4))[0]
    assert 0xAF11E+rel==0xAE220,"native target mismatch"
    return len(SIG)
def check_source(root):
    def get(p):return (root/p).read_text(encoding="utf-8")
    previous=get("Stage50_Overlay/src/Stage38CallbackWindow.inl")
    adapted=get("Stage51_Overlay/src/Stage38CallbackWindow.inl")
    src=get("Stage51_Overlay/src/Stage51GlobalCoordinate.inl")
    s49=get("Stage49_Overlay/src/Stage49LinkedMotion.inl")
    s50=get("Stage50_Overlay/src/Stage50PreLinkedCoordinate.inl")
    assert "STAGE49_LINKED_MOTION_BUILD=2026-11-10-r2" in s49
    assert "STAGE50_PRELINKED_COORD_BUILD=2026-11-10-r1" in s50
    assert "STAGE51_GLOBAL_COORD_BUILD=2026-11-10-r1" in src
    for required in (
      "0xAF119u","0xAE220u","0xAF117u","0xAF11Eu","0xAF124u",
      "0xAE2B7u","0xAE6DDu","const DWORD entryEsp=saved[3]+4u;",
      "SafeReadDword(entryEsp+4u,&arg)","arg!=1u",
      "g_playerP1+4u","g_playerP2+4u","g_s50Installed",
      "S51Snap(r.Object,r.Pre,r.PreMask)",
      "S51Snap(r.Object,r.Post,r.PostMask)",
      "STAGE51_HEALTH","STAGE51_GLOBAL_WINDOW",
      "if(writable&&valid())","VirtualProtect(site,5u",
      "ResumeThreads(suspended,stopped)"
    ):assert required in src,required
    assert "CreateThread(" not in src
    assert src.count("MakeRelativeCall(")==1
    body=src.split("void S51GlobalUpdateHook()",1)[1].split("__asm {",1)[1].split("\n  }",1)[0]
    for ins in ("pushfd","popfd","pushad","popad","fxsave [edi]","fxrstor [edi]"):
      assert body.count(ins)==2,ins
    assert body.count("call S51Enter")==1 and body.count("call S51Leave")==1
    assert body.count("push dword ptr [esp+4]")==1
    assert body.count("call dword ptr [g_s51Native]")==1
    assert body.count("add esp,4")==3,"bookkeeping twice + copied native arg cleanup"
    assert body.rstrip().endswith("ret")
    expected=previous.replace(
      '#include "Stage50PreLinkedCoordinate.inl"',
      '#include "Stage50PreLinkedCoordinate.inl"\n#include "Stage51GlobalCoordinate.inl"')
    expected=expected.replace(
      "if(InstallStage49Hooks())InstallStage50Hooks();",
      'if(InstallStage49Hooks()){\n'
      '                                    if(InstallStage50Hooks())InstallStage51Hooks();\n'
      '                                    else S51Log("ERROR: Stage51 requires installed Stage50; no patches.");\n'
      '                                }')
    expected=expected.replace("    FlushStage50();\n","    FlushStage50();\n    FlushStage51();\n")
    assert adapted==expected,"prior source modified unexpectedly"
if __name__=="__main__":
    ap=argparse.ArgumentParser()
    ap.add_argument("--binary",type=Path,required=True)
    ap.add_argument("--root",type=Path,default=Path(__file__).resolve().parents[2])
    a=ap.parse_args()
    n=check_pe(a.binary)
    check_source(a.root)
    print(f"PASS Stage51: original SHA/PE32 and {n} native signatures, AF119→AE220 cdecl arg1")
    print("PASS Stage51: preserved Stage49 r2/Stage50 and original-once wrapper/source adapter")
    print("PENDING: new Windows x86 Build.log, Stage51 install, actual COM-vs-COM runtime")
