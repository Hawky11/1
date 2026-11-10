#!/usr/bin/env python3
"""Stage44 source-only original PE32 SHA/target/signature verifier.
PASS does NOT prove Windows x86 compile, DLL installation, or runtime effects.
"""
import argparse
import hashlib
from pathlib import Path
import struct

SHA="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
SIZE=175556608
SIGNATURES={
    0x1E9E10:"89 4E 60",
    0x1E9E2D:"E8 5E E6 FF FF",
    0x1E8490:"55 8B EC 83 EC 0C 53 56 57 8B F1",
    0x1E84B3:"E8 18 6E FF FF",
    0x1DF2D0:"55 8B EC 8B 45 08",
    0x1E84B8:"89 46 34",
    0x1E84BB:"C6 86 6D 02 00 00 01",
    0x1E84D5:"89 8E 78 01 00 00",
}
def main():
    p=argparse.ArgumentParser()
    p.add_argument("exe",type=Path)
    p.add_argument("--stage38",type=Path)
    p.add_argument("--stage44",type=Path)
    args=p.parse_args()
    b=args.exe.read_bytes()
    assert len(b)==SIZE and hashlib.sha256(b).hexdigest()==SHA, "Wrong original executable"
    pe=struct.unpack_from("<I",b,0x3C)[0]
    assert b[pe:pe+4]==b"PE\x00\x00"
    assert struct.unpack_from("<H",b,pe+4)[0]==0x14C
    opt=pe+24
    assert struct.unpack_from("<H",b,opt)[0]==0x10B
    section_start=opt+struct.unpack_from("<H",b,pe+20)[0]
    sec=[]
    for i in range(struct.unpack_from("<H",b,pe+6)[0]):
        off=section_start+i*40
        va,size,raw=struct.unpack_from("<III",b,off+12)
        sec.append((va,size,raw))
    def read(rva,n):
        for va,size,raw in sec:
            if va<=rva and rva+n<=va+size:
                return b[raw+rva-va:raw+rva-va+n]
        raise ValueError("RVA unmapped %07X"%rva)
    for rva,h in SIGNATURES.items():
        expected=bytes.fromhex(h)
        assert read(rva,len(expected))==expected, "Signature mismatch %07X"%rva
        print("SIGNATURE PASS %07X"%rva)
    for site,target in ((0x1E9E2D,0x1E8490),(0x1E84B3,0x1DF2D0)):
        code=read(site,5)
        assert code[0]==0xE8
        actual=site+5+struct.unpack("<i",code[1:])[0]
        assert actual==target, "Native CALL mismatch %07X"%site
        print("CALL PASS %07X -> %07X"%(site,target))
    assert read(0x1DF2F7,3)==bytes.fromhex("C2 08 00"), "Native selector must clean two stack args"
    print("NATIVE 1DF2D0 RET 8 PASS")
    if args.stage38:
        s=args.stage38.read_text(encoding="utf-8")
        for x in (
            '#include "Stage40RequestConsumer.inl"',
            '#include "Stage42ActivationHook.inl"',
            '#include "Stage44ActionDefinition.inl"',
            "InstallStage40Hooks();",
            "InstallStage42Hooks();",
            "InstallStage44Hooks();",
            "FlushStage40();",
            "FlushStage42();",
            "FlushStage44();",
        ):
            assert x in s, "Missing integration: "+x
        assert s.index('#include "Stage42ActivationHook.inl"') < s.index('#include "Stage44ActionDefinition.inl"')
        print("STAGE38/40/41/42/43/44 integration PASS")
    if args.stage44:
        s=args.stage44.read_text(encoding="utf-8")
        for x in (
            "STAGE44_ACTION_DEFINITION_BUILD=2026-11-10-r1",
            "STAGE44_INSTALLED 1/1 callsite=1E9E2D native=1E8490",
            "STAGE44_BINDING_WINDOW",
            "STAGE44_HEALTH",
            "S44SnapObject(object,r.Pre,r.PreMask)",
            "S44SnapObject(r.Object,r.Post,r.PostMask)",
            "S44SnapDefinition(r.Post[1],r.MetaPost,r.MetaPostMask)",
            "call dword ptr [g_s44NativeBinding]",
            "fxsave [edi]",
            "fxrstor [edi]",
            "origCall[]={0xE8,0x5E,0xE6,0xFF,0xFF}",
        ):
            assert x in s, "Missing Stage44 source anchor: "+x
        assert s.count("MakeRelativeCall(")==1, "Stage44 must patch only one native CALL"
        assert s.count("call dword ptr [g_s44NativeBinding]")==1
        assert s.count("fxsave [edi]")==2 and s.count("fxrstor [edi]")==2
        print("Stage44 source/replay PASS")
    print("STAGE44_STATIC_PASS — Windows x86 build and native runtime still PENDING")

if __name__=="__main__":
    main()
