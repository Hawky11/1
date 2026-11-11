#!/usr/bin/env python3
"""Verify Stage78 original PE bytes and Stage77 adapter wiring. No writes."""
import argparse, hashlib, struct, zipfile
from pathlib import Path
SHA="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
def check(path):
    if path.suffix.lower()==".zip":
        with zipfile.ZipFile(path) as z:
            matches=[n for n in z.namelist() if n.lower().endswith("aalib.exe")]
            if len(matches)!=1: raise ValueError("Expected one AALib.exe")
            data=z.read(matches[0])
    else:data=path.read_bytes()
    digest=hashlib.sha256(data).hexdigest()
    assert digest==SHA,"Wrong original SHA256: "+digest
    pe=struct.unpack_from("<I",data,0x3C)[0]
    assert data[:2]==b"MZ" and data[pe:pe+4]==b"PE"+bytes((0,0))
    assert struct.unpack_from("<H",data,pe+4)[0]==0x14C
    opt=pe+24
    assert struct.unpack_from("<H",data,opt)[0]==0x10B
    assert struct.unpack_from("<I",data,opt+28)[0]==0x400000
    table=opt+struct.unpack_from("<H",data,pe+20)[0]
    text=None
    for i in range(struct.unpack_from("<H",data,pe+6)[0]):
        off=table+i*40
        if data[off:off+8].split(bytes((0,)))[0]==b".text":
            _,rva,size,raw=struct.unpack_from("<IIII",data,off+8)
            text=(rva,size,raw)
            break
    assert text is not None
    base,size,raw=text
    sites={
      0x155D27:"6a21",0x155D29:"c786503f000001000000",
      0x155D33:"6a00",0x155D74:"c786543f000003000000",
      0x155D7E:"eba7",0x1560B9:"e8b2faffff",
      0x1566F0:"e87bf4ffff",
    }
    for r,pattern in sites.items():
        expect=bytes.fromhex(pattern)
        assert base<=r and r+len(expect)<=base+size
        assert data[raw+r-base:raw+r-base+len(expect)]==expect,hex(r)
    root=Path(__file__).resolve().parents[2]
    adapter=(root/"Stage78_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
    probe=(root/"Stage78_Overlay/src/Stage78LiteralOneWriter.inl").read_text(encoding="utf-8")
    for token in ['#include "Stage78LiteralOneWriter.inl"',
      "if(InstallStage76Hooks()) InstallStage78Hooks();",
      "FlushStage78();", "StartStage59Watch();",
      "STAGE77_NATIVE_PREPARE"]:
        assert token in adapter,token
    assert adapter.count("call dword ptr [g_stage38PrepareTarget]")==1
    assert adapter.count("fxsave [edi]")==adapter.count("fxrstor [edi]")==3
    assert probe.count("mov dword ptr [esi+3F50h],1")==1
    assert probe.count("jmp dword ptr [g_s78ResumeVA]")==1
    assert probe.count("fxsave [edi]")==probe.count("fxrstor [edi]")==2
    for token in ["kS78Site=0x155D29u","kS78Resume=0x155D33u",
       "STAGE78_GUARD","STAGE78_INSTALLED 1/1","STAGE78_LITERAL_ONE",
       "STAGE78_HEALTH","S76SuspendPeers", "GetThreadContext",
       "VirtualProtect", "FlushInstructionCache"]:
        assert token in probe,token
    assert probe.index("call S78BeforeStore")<probe.index(
        "mov dword ptr [esi+3F50h],1")<probe.index("call S78AfterStore")
    print("PASS: original SHA, 7 opcode sites, Stage78 source structure")
    print("Original SHA256",digest)
    print("Warning: Windows build and COM-vs-COM runtime not tested")
if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_exe_or_zip",type=Path)
    check(p.parse_args().original_exe_or_zip)
