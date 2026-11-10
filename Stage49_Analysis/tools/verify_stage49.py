#!/usr/bin/env python3
"""Stage49 r2 SHA-pinned native propagation and relocation-aware overlay verification.

This checks static native bytes and wrapper source shape, NOT Windows x86 build
or live gameplay. Original AALib.exe must remain in user's ChatGPT Library.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import hashlib
import struct

SIZE=175556608
SHA256="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
SIGNATURES={
    0xAF11E:"8b35c8022e0e",  # E2E02C8 original main linked list
    0xAF19E:"8b35c0022e0e",  # E2E02C0 conditional alternate list
    0xAF13B:"8bce",          # ECX=ESI before CALL
    0xAF13D:"e82ea01300",
    0xAF1A8:"8bce",
    0xAF1AA:"e8c19f1300",
    0x1E9170:"558bec83ec08568bf1",
    0x1E9381:"c3",
    0x1E92C8:"8bbed0020000",  # linked O+2D0
    0x1E92CE:"8b4760",        # linked O+60
    0x1E92D1:"3b8680020000",  # compare O+280
    0x1E92DD:"d9477c",        # fld linked+7C
    0x1E92E6:"d9567c",        # fst O+7C
    0x1E92E9:"d99eac000000",  # fstp O+AC
    0x1E92EF:"d98780000000",  # fld linked+80
    0x1E92FB:"d99680000000",  # fst O+80
    0x1E9301:"d99eb0000000",  # fstp O+B0
}
def read_pe(path:Path):
    b=path.read_bytes()
    assert len(b)==SIZE and hashlib.sha256(b).hexdigest()==SHA256
    pe=struct.unpack_from("<I",b,0x3c)[0]
    assert b[:2]==b"MZ" and b[pe:pe+4]==bytes((0x50,0x45,0,0))
    assert struct.unpack_from("<H",b,pe+24)[0]==0x10b
    assert struct.unpack_from("<I",b,pe+52)[0]==0x400000
    start=pe+24+struct.unpack_from("<H",b,pe+20)[0]
    count=struct.unpack_from("<H",b,pe+6)[0]
    secs=[]
    for i in range(count):
        a=start+40*i
        name=b[a:a+8].split(bytes((0,)),1)[0]
        virtualsize,rva,rawsize,offset=struct.unpack_from("<IIII",b,a+8)
        secs.append((name,rva,rawsize,offset))
    def rd(rva,n):
        for name,va,size,offset in secs:
            if va<=rva and rva+n<=va+size:return b[offset+rva-va:offset+rva-va+n]
        raise AssertionError(f"RVA not backed: {rva:08X}")
    for rva,h in SIGNATURES.items():
        expected=bytes.fromhex(h)
        assert rd(rva,len(expected))==expected,f"signature mismatch {rva:08X}"
    for site in (0xAF13D,0xAF1AA):
        rel=struct.unpack("<i",rd(site+1,4))[0]
        assert site+5+rel==0x1E9170,"native call destination mismatch"
    return len(SIGNATURES)

def check_source(root:Path):
    def get(p):return (root/p).read_text(encoding="utf-8")
    baseline=get("Stage48_Overlay/src/Stage38CallbackWindow.inl")
    adapter=get("Stage49_Overlay/src/Stage38CallbackWindow.inl")
    source=get("Stage49_Overlay/src/Stage49LinkedMotion.inl")
    s44=get("Stage48_Overlay/src/Stage44ActionDefinition.inl")
    s48=get("Stage48_Overlay/src/Stage48VariantBinder.inl")
    assert "S48FlushRecord(r);" in s44 and "STAGE48_INSTALLED 2/2" in s48
    assert source.count("MakeRelativeCall(")==1
    assert '0xAF13Du' in source and '0xAF1AAu' in source
    assert '0x1E9170u' in source
    assert '0xAF11Eu' in source and '0xAF19Eu' in source
    assert 'g_s48Installed' in source and 'g_s47Installed' in source
    assert "STAGE49_LINKED_MOTION_BUILD=2026-11-10-r2" in source
    assert "const DWORD preferredImageBase=0x00400000u;" in source
    assert "const DWORD root0Preferred=0x0E2E02C8u;" in source
    assert "const DWORD root1Preferred=0x0E2E02C0u;" in source
    assert "preferredOperand+(g_gameBase-preferredImageBase)" in source
    assert "sameRoot(0xAF11Eu,root0Preferred)" in source
    assert "sameRoot(0xAF19Eu,root1Preferred)" in source
    assert "Stage49 native signature RVA=" in source
    assert "Stage49 root RVA=" in source
    assert "original no-arg thiscall once" in source
    assert "CreateThread(" not in source
    assert "if(writable&&valid())" in source
    assert "staticGateCandidate" in source
    assert "S49Snap(r.Object" in source
    assert "g_playerP1+4u" in source and "g_playerP2+4u" in source
    body=source.split("void S49LinkedMotionHook()",1)[1].split("__asm {",1)[1].split("\n    }",1)[0]
    for instr in ("fxsave [edi]","fxrstor [edi]","pushfd","popfd","pushad","popad"):
        assert body.count(instr)==2,instr
    assert body.count("call S49Enter")==1
    assert body.count("call S49Leave")==1
    assert body.count("call dword ptr [g_s49Native]")==1
    assert body.rstrip().endswith("ret")
    assert adapter.count('#include "Stage49LinkedMotion.inl"')==1
    assert adapter.count("InstallStage49Hooks()")==1
    assert adapter.count("FlushStage49();")==1
    # Existing Stage38/39/40/41/42/43/44/45/46/47/48 source unchanged.
    expected=baseline.replace('#include "Stage48VariantBinder.inl"',
        '#include "Stage48VariantBinder.inl"\n#include "Stage49LinkedMotion.inl"')
    expected=expected.replace('if(InstallStage47Hooks())InstallStage48Hooks();',
        'if(InstallStage47Hooks()){\n'
        '                            if(InstallStage48Hooks())InstallStage49Hooks();\n'
        '                            else S49Log("ERROR: Stage49 requires installed Stage48; no patches.");\n'
        '                        }')
    expected=expected.replace("    FlushStage47();\n","    FlushStage47();\n    FlushStage49();\n")
    assert adapter==expected,"Stage49 adapter changed existing instrumentation"
    # Aside from extra inclusion and install/flush, all prior writer paths are untouched.

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("exe",type=Path)
    parser.add_argument("--root",type=Path,default=Path(__file__).resolve().parents[2])
    args=parser.parse_args()
    n=read_pe(args.exe)
    check_source(args.root)
    print(f"PASS Stage49: original PE SHA/size, {n} exact native signatures,")
    print("two verified direct calls to 1E9170, linked-float copy instructions,")
    print("scoped native once-only wrapper and existing worker integration.")
    print("Stage49 r2 source guard checks PASS; Windows x86 build/runtime PENDING.")

if __name__=="__main__":main()
