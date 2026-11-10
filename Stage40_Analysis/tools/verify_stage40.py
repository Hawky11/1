#!/usr/bin/env python3
"""Stage40: stdlib-only read-only verifier for the exact original i386 AALib.exe.
No DLL build, install, debugger or native code execution is performed.
"""
import argparse
import hashlib
from pathlib import Path
import struct

EXPECTED_SHA256 = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
EXPECTED_SIZE = 175556608
SIGNATURES = {
    0xAD72F: bytes.fromhex("E8 4C AD 03 00"),
    0xE84E3: bytes.fromhex("E8 38 D4 03 00"),
    0xE8505: bytes.fromhex("E8 E6 D2 04 00"),
    0xE8480: bytes.fromhex("55 8B EC 83 EC 10 56 8B F1 8B 86 4C 3F 00 00 83 F8 FF"),
    0x125920: bytes.fromhex("55 8B EC 8A 15 60 72 CF 05"),
    0x1357F0: bytes.fromhex("55 8B EC 8B 45 08 83 EC 08"),
    0xE8489: bytes.fromhex("8B 86 4C 3F 00 00"),
    0xE84ED: bytes.fromhex("83 FF FF 74 43"),
    0xE8514: bytes.fromhex("89 88 D0 02 00 00"),
    0xE8523: bytes.fromhex("89 90 80 02 00 00"),
    0xE853F: bytes.fromhex("C7 86 4C 3F 00 00 FF FF FF FF"),
    0xE8549: bytes.fromhex("81 A6 7C 3B 00 00 FF FF FF FE"),
}
TARGETS = {0xAD72F: 0xE8480, 0xE84E3: 0x125920, 0xE8505: 0x1357F0}

def sections(b):
    if b[:2] != b"MZ":
        raise ValueError("Missing MZ")
    pe=struct.unpack_from("<I",b,0x3C)[0]
    if b[pe:pe+4] != b"PE\x00\x00":
        raise ValueError("Missing PE")
    n=struct.unpack_from("<H",b,pe+6)[0]
    optional=pe+24
    if struct.unpack_from("<H",b,optional)[0]!=0x10B:
        raise ValueError("Not PE32")
    if struct.unpack_from("<H",b,pe+4)[0]!=0x14C:
        raise ValueError("Not i386")
    section_base=optional+struct.unpack_from("<H",b,pe+20)[0]
    result=[]
    for i in range(n):
        p=section_base+i*40
        vsize, va, rawsize, rawptr=struct.unpack_from("<IIII",b,p+8)
        result.append((va,rawsize,rawptr))
    return result

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--stage38",type=Path)
    ap.add_argument("--stage40",type=Path)
    args=ap.parse_args()
    b=args.exe.read_bytes()
    assert len(b)==EXPECTED_SIZE, "Wrong EXE size"
    h=hashlib.sha256(b).hexdigest()
    assert h==EXPECTED_SHA256, "Wrong EXE SHA-256"
    mapping=sections(b)
    def read(rva,size):
        for va,rawsize,rawptr in mapping:
            if va<=rva and rva+size<=va+rawsize:
                return b[rawptr+rva-va:rawptr+rva-va+size]
        raise ValueError("RVA not mapped: %08X"%rva)
    # Original RVA 125925: PE HIGHLOW, VA 05CF7260. Relocate the
    # immediate in 125920's prologue when comparing loaded image memory.
    pe=struct.unpack_from("<I",b,0x3C)[0]
    opt=pe+24
    preferred_base=struct.unpack_from("<I",b,opt+28)[0]
    assert preferred_base==0x00400000, "Unexpected preferred PE base"
    reloc_rva,reloc_size=struct.unpack_from("<II",b,opt+96+5*8)
    def offset(rva):
        for va,rawsize,rawptr in mapping:
            if va<=rva<va+rawsize:
                return rawptr+rva-va
        raise ValueError("Unmapped RVA %08X"%rva)
    assert struct.unpack("<I",read(0x125925,4))[0]==0x05CF7260
    pos=offset(reloc_rva)
    end=pos+reloc_size
    reloc_type=None
    while pos<end:
        page,size=struct.unpack_from("<II",b,pos)
        assert size>=8 and size%2==0, "Broken relocation block"
        for k in range((size-8)//2):
            entry=struct.unpack_from("<H",b,pos+8+k*2)[0]
            if page+(entry&0x0FFF)==0x125925:
                reloc_type=entry>>12
        pos+=size
    assert reloc_type==3, "Missing PE IMAGE_REL_BASED_HIGHLOW at RVA 125925"
    for loaded_base in (0x00400000,0x00800000,0x00C00000):
        expected=(0x05CF7260+loaded_base-preferred_base)&0xFFFFFFFF
        actual=(struct.unpack("<I",read(0x125925,4))[0]+loaded_base-preferred_base)&0xFFFFFFFF
        assert actual==expected, "ASLR relocation arithmetic mismatch"
        print("RELOCATION PASS base %08X: RVA 125925 -> VA %08X"%(loaded_base,actual))
    for rva,expected in SIGNATURES.items():
        assert read(rva,len(expected))==expected, "Signature mismatch %08X"%rva
        print("SIGNATURE PASS %07X (%d bytes)"%(rva,len(expected)))
    for rva,target in TARGETS.items():
        data=read(rva,5)
        assert data[0]==0xE8, "Not a CALL at %07X"%rva
        actual=rva+5+struct.unpack("<i",data[1:])[0]
        assert actual==target, "CALL target mismatch %07X"%rva
        print("CALL PASS %07X -> %07X"%(rva,target))
    if args.stage38:
        c=args.stage38.read_text(encoding="utf-8")
        for s in ['#include "Stage39ActionOutput.inl"',
                  '#include "Stage40RequestConsumer.inl"',
                  'InstallStage40Hooks();', 'FlushStage40();']:
            assert s in c, "Missing Stage38 integration: "+s
        print("Stage38/39/40 integration PASS")
    if args.stage40:
        c=args.stage40.read_text(encoding="utf-8")
        for s in ["cmp dword ptr [ecx+03F4Ch],0FFFFFFFFh",
                  "jmp dword ptr [g_s40NativeEntry]",
                  "call dword ptr [g_s40NativeSelect]",
                  "call dword ptr [g_s40NativeActor]",
                  "push dword ptr [esp+8]",
                  "push dword ptr [esp+10h]",
                  "add esp,8", "add esp,10h",
                  "expectedSelectGlobal=0x05CF7260u+(g_gameBase-0x00400000u)",
                  "std::memcpy(&actualSelectGlobal,ns+5u,sizeof(actualSelectGlobal))",
                  "actualSelectGlobal!=expectedSelectGlobal",
                  "S40SnapActor(r.actor,r.actorPre,r.actorPreMask)",
                  "S40SnapActor(r.actor,r.actorPost,r.actorPostMask)"]:
            assert s in c, "Missing source anchor: "+s
        assert c.count("push dword ptr [esp+8]")==2, "Selector must copy two arguments"
        assert c.count("push dword ptr [esp+10h]")==4, "Actor must copy four arguments"
        print("Stage40 wrapper/replay source anchors PASS")
    print("STAGE40_STATIC_PASS — not a Windows x86 build or a runtime test")

if __name__=="__main__":
    main()
