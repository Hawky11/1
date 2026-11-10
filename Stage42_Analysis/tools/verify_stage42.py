#!/usr/bin/env python3
"""Static, read-only native PE and source signature checks for Stage42.
PASS does not mean Windows x86 compilation or real runtime installation.
"""
import argparse
import hashlib
from pathlib import Path
import struct

SHA="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
SIZE=175556608
SIGS={
    0x1E9F40:"80 79 30 00 74 16",
    0x1E9F5C:"F6 41 08 01",
    0x1E9F62:"E8 09 FD FF FF",
    0x1E9C70:"56 8B F1 83 66 08 FE",
    0x1E9C73:"83 66 08 FE",
    0x1E9E0B:"8B 4E 64 33 C0 89 4E 60",
    0xAD72F:"E8 4C AD 03 00",
    0xAD75C:"E8 DF C7 13 00",
    0xAD778:"E8 C3 C7 13 00",
    0xAD797:"E8 A4 C7 13 00",
    0xAD7BC:"E8 7F C7 13 00",
}
def main():
    p=argparse.ArgumentParser()
    p.add_argument("exe",type=Path)
    p.add_argument("--stage38",type=Path)
    p.add_argument("--stage42",type=Path)
    args=p.parse_args()
    b=args.exe.read_bytes()
    assert len(b)==SIZE
    assert hashlib.sha256(b).hexdigest()==SHA, "Wrong original EXE"
    pe=struct.unpack_from("<I",b,0x3C)[0]
    assert b[pe:pe+4]==b"PE\x00\x00"
    assert struct.unpack_from("<H",b,pe+4)[0]==0x14C
    op=pe+24
    assert struct.unpack_from("<H",b,op)[0]==0x10B
    sh=op+struct.unpack_from("<H",b,pe+20)[0]
    sections=[]
    for i in range(struct.unpack_from("<H",b,pe+6)[0]):
        j=sh+i*40
        va,size,off=struct.unpack_from("<III",b,j+12)
        sections.append((va,size,off))
    def read(rva,n):
        for va,size,off in sections:
            if va<=rva and rva+n<=va+size:
                return b[off+rva-va:off+rva-va+n]
        raise ValueError("Unmapped RVA %07X"%rva)
    for rva,h in SIGS.items():
        v=bytes.fromhex(h)
        assert read(rva,len(v))==v, "Native signature failure %07X"%rva
        print("SIGNATURE PASS %07X"%rva)
    for site,target in [(0x1E9F62,0x1E9C70),(0xAD75C,0x1E9F40),
                        (0xAD778,0x1E9F40),(0xAD797,0x1E9F40),
                        (0xAD7BC,0x1E9F40)]:
        z=read(site,5)
        assert z[0]==0xE8
        actual=site+5+struct.unpack("<i",z[1:])[0]
        assert actual==target,"Native CALL failure %07X"%site
        print("CALL PASS %07X -> %07X"%(site,target))
    if args.stage38:
        c=args.stage38.read_text(encoding="utf-8")
        for anchor in ('#include "Stage40RequestConsumer.inl"',
                       '#include "Stage42ActivationHook.inl"',
                       'InstallStage40Hooks();','InstallStage42Hooks();',
                       'FlushStage40();','FlushStage42();'):
            assert anchor in c,"Missing integration: "+anchor
        assert c.index('#include "Stage40RequestConsumer.inl"')<c.index('#include "Stage42ActivationHook.inl"')
        print("STAGE38/40/41/42_INTEGRATION PASS")
    if args.stage42:
        s=args.stage42.read_text(encoding="utf-8")
        for anchor in ('STAGE42_ACTION_ACTIVATION_BUILD=2026-11-10-r1',
                       'STAGE42_INSTALLED 1/1',
                       'S41SnapObject(object,r.Pre,r.PreMask)',
                       'S41SnapObject(r.Object,r.Post,r.PostMask)',
                       'call dword ptr [g_s42NativeActivation]',
                       'fxsave [edi]','fxrstor [edi]',
                       'expectedCall[]={0xE8,0x09,0xFD,0xFF,0xFF}',
                       'std::memcmp(native,expectedNative,sizeof(expectedNative))',
                       'STAGE42_ACTIVATION_WINDOW'):
            assert anchor in s,"Missing source anchor: "+anchor
        assert s.count("MakeRelativeCall(")==1,"Stage42 must patch exactly one CALL"
        print("STAGE42_SOURCE_ANCHORS PASS")
    print("STAGE42_STATIC_PASS — NOT a Windows x86 build or runtime test")

if __name__=="__main__":
    main()
