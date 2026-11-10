#!/usr/bin/env python3
"""Read-only Stage58 original PE + adapter verifier (stdlib; no Windows build)."""
import argparse
import hashlib
import struct
from pathlib import Path

EXPECTED_SHA="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
EXPECTED_SIZE=175_556_608
SITES=[
    (0xACF0F, bytes.fromhex("E8 8C B7 00 00"), 0xB86A0),
    (0xACF7A, bytes.fromhex("E8 21 B7 00 00"), 0xB86A0),
    (0xACF8A, bytes.fromhex("E8 F1 CA 03 00"), 0xE9A80),
]
NATIVES=[
    (0xB86A0, bytes.fromhex("55 8B EC 56 8B 75 08 83 FE FF")),
    (0xE9A80, bytes.fromhex("55 8B EC 8B 45 08 56 8B 74 81 08"))
]
def u16(b,o):return struct.unpack_from("<H",b,o)[0]
def u32(b,o):return struct.unpack_from("<I",b,o)[0]
def check(condition,message):
    if not condition:raise AssertionError(message)
    print("PASS:",message)
def sections(raw):
    pe=u32(raw,0x3C)
    check(raw[:2]==b"MZ" and raw[pe:pe+4]==b"PE\0\0","PE signatures")
    check(u16(raw,pe+4)==0x14c and u16(raw,pe+24)==0x10b,"PE32 x86")
    count=u16(raw,pe+6)
    optsize=u16(raw,pe+20)
    out=[]
    for i in range(count):
        at=pe+24+optsize+40*i
        virtual_size,va,size,file_offset=struct.unpack_from("<IIII",raw,at+8)
        out.append((va,virtual_size,size,file_offset))
    return pe,out
def raw_at(raw,sec,rva,length):
    for va,vsize,rsize,foff in sec:
        if va<=rva and rva+length<=va+rsize and rva+length<=va+max(vsize,rsize):
            start=foff+rva-va
            return raw[start:start+length]
    raise AssertionError(f"unmapped/non-raw RVA 0x{rva:X}")
def relocations(raw,pe,sec):
    opt=pe+24
    rel_rva,rel_size=struct.unpack_from("<II",raw,opt+96+5*8)
    rel=raw_at(raw,sec,rel_rva,rel_size)
    pos=0
    entries=[]
    while pos<len(rel):
        if pos+8>len(rel):break
        page,size=struct.unpack_from("<II",rel,pos)
        if size<8 or pos+size>len(rel):raise AssertionError("bad relocation block")
        for j in range(pos+8,pos+size,2):
            word=u16(rel,j)
            if word>>12==3:entries.append(page+(word&0xfff))
        pos+=size
    return entries
def verify(aalib,root):
    raw=Path(aalib).read_bytes()
    check(len(raw)==EXPECTED_SIZE,"Original AALib.exe size")
    check(hashlib.sha256(raw).hexdigest()==EXPECTED_SHA,"Original AALib.exe SHA256")
    pe,sec=sections(raw)
    regions=[]
    for rva,sig,target in SITES:
        check(raw_at(raw,sec,rva,5)==sig,f"Original CALL signature 0x{rva:X}")
        rel=struct.unpack("<i",sig[1:])[0]
        check(rva+5+rel==target,f"Original rel32 target 0x{rva:X}->0x{target:X}")
        regions.append((rva,rva+5))
    for rva,sig in NATIVES:
        check(raw_at(raw,sec,rva,len(sig))==sig,f"Native function prologue 0x{rva:X}")
        regions.append((rva,rva+len(sig)))
    for rva in (0xB8703,0xB8732,0xB876B):
        check(raw_at(raw,sec,rva,1)==b"\xC3",f"B86A0 plain RET branch RVA 0x{rva:X}")
    for rva in (0xE9A93,0xE9AB3,0xE9ACF,0xE9ADF,0xE9AFE,0xE9B5E):
        check(raw_at(raw,sec,rva,3)==b"\xC2\x04\x00",f"E9A80 RET 4 branch RVA 0x{rva:X}")
    entries=relocations(raw,pe,sec)
    overlaps=[(r,lo,hi) for r in entries for lo,hi in regions if r<hi and r+4>lo]
    check(not overlaps,f"PE HIGHLOW relocation non-overlap ({len(entries)} entries)")
    old=(root/"Stage57_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
    new=(root/"Stage58_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
    edits=[
        ('#include "Stage57ACDF0HelperBoundary.inl"',
         '#include "Stage57ACDF0HelperBoundary.inl"\n#include "Stage58ACDF0ConditionalCalls.inl"'),
        ('if(InstallStage56Hooks())InstallStage57Hooks();',
         'if(InstallStage56Hooks()){\n'
         '                                                                if(InstallStage57Hooks())InstallStage58Hooks();\n'
         '                                                                else S58Log("ERROR: Stage58 requires installed Stage57; no patches.");\n'
         '                                                            }'),
        ('    FlushStage57();','    FlushStage57();\n    FlushStage58();')
    ]
    expected=old
    for before,after in edits:
        check(expected.count(before)==1,f"Stage57 adapter unique anchor {before[:45]}")
        expected=expected.replace(before,after,1)
    check(new==expected,"Stage58 adapter exactly three additions; Stage47-57 preserved")
    src=(root/"Stage58_Overlay/src/Stage58ACDF0ConditionalCalls.inl").read_text(encoding="utf-8")
    required=[
        "S58CheckHook","S58LookupHook","S58Enter","S58Leave",
        "S57Snap","push dword ptr [esp+8]","push dword ptr [esp+4]",
        "lea esp,[esp+8]","ret 4","STAGE58_INSTALLED 3/3",
        "Stage58_ACDF0Conditional", # matched against marker in the source below
    ]
    required[-1]="ArcanaAI_Stage58ACDF0ConditionalCallTrace.log"
    for token in required:check(token in src,f"Stage58 source contains {token}")
    print("STATIC VERIFIED. Windows MSVC build and runtime ABI still PENDING.")
if __name__=="__main__":
    p=argparse.ArgumentParser()
    p.add_argument("--aalib",required=True,type=Path)
    p.add_argument("--repo-root",default=Path("."),type=Path)
    a=p.parse_args()
    verify(a.aalib,a.repo_root)
