#!/usr/bin/env python3
"""Stage41 static PE32 and source verifier. Read-only, stdlib only.
No Windows x86 compilation or runtime behavior is implied by a PASS.
"""
import argparse
import collections
import hashlib
from pathlib import Path
import struct

EXPECTED_SHA = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
EXPECTED_SIZE = 175556608
CODE = {
    0xE84AB: "E8 50 1A 10 00",
    0xE84E3: "E8 38 D4 03 00",
    0x1E9F00: "55 8B EC 80 7D 10 00 8B 45 08",
    0x1E9F0C: "39 41 60",
    0x1E9F11: "C7 41 68 FF FF FF FF",
    0x1E9F1E: "C6 41 30 00",
    0x1E9F22: "89 41 64",
    0x1E9F25: "83 49 08 01",
    0x1E9F34: "B0 01",
    0x1E9F37: "C2 0C 00",
    0xE853F: "C7 86 4C 3F 00 00 FF FF FF FF",
}

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--inl",type=Path,help="Stage41_Overlay/src/Stage40RequestConsumer.inl")
    args=ap.parse_args()
    data=args.exe.read_bytes()
    assert len(data)==EXPECTED_SIZE, "Unexpected original EXE size"
    assert hashlib.sha256(data).hexdigest()==EXPECTED_SHA, "Original EXE hash mismatch"
    assert data[:2]==b"MZ", "Not an MZ executable"
    pe=struct.unpack_from("<I",data,0x3C)[0]
    assert data[pe:pe+4]==b"PE\0\0", "Missing PE header"
    assert struct.unpack_from("<H",data,pe+4)[0]==0x014C, "Not x86"
    count=struct.unpack_from("<H",data,pe+6)[0]
    op=pe+24
    assert struct.unpack_from("<H",data,op)[0]==0x10B, "Not PE32"
    assert struct.unpack_from("<I",data,op+28)[0]==0x400000, "Unexpected image base"
    pos=op+struct.unpack_from("<H",data,pe+20)[0]
    sections=[]
    for i in range(count):
        p=pos+i*40
        va,size,raw=struct.unpack_from("<III",data,p+12)
        sections.append((va,size,raw))
    def read(rva,n):
        for va,size,raw in sections:
            if va<=rva and rva+n<=va+size:
                return data[raw+rva-va:raw+rva-va+n]
        raise ValueError("Unmapped raw RVA %08X"%rva)
    for rva,code in CODE.items():
        expected=bytes.fromhex(code)
        assert read(rva,len(expected))==expected, "Native signature mismatch at %07X"%rva
        print("SIGNATURE PASS %07X"%rva)
    for site,dest in ((0xE84AB,0x1E9F00),(0xE84E3,0x125920)):
        call=read(site,5)
        assert call[0]==0xE8
        assert site+5+struct.unpack("<i",call[1:])[0]==dest
        print("CALL PASS %07X->%07X"%(site,dest))
    table=read(0x58F7260,500)
    modes=collections.Counter()
    n=0
    while True:
        row=table[n*6:n*6+6]
        assert len(row)==6, "Lookup terminator missing"
        mode,pad,raw_id,mapped=struct.unpack("<BBHH",row)
        if mode==0xFE:break
        modes[mode]+=1
        n+=1
    assert n==40 and modes=={8:3,9:28,29:9}, "Unexpected native lookup modes"
    print("LOOKUP_TABLE PASS 40 entries, mode8=3, mode9=28, mode29=9")
    if args.inl:
        s=args.inl.read_text(encoding="utf-8")
        anchors=("STAGE41_DIRECT_OBJECT_BUILD=2026-11-10-r1",
                 "S41SnapObject(r.objectBefore,r.objectPre,r.objectPreMask)",
                 "S41SnapObject(r.objectAfter,r.objectPost,r.objectPostMask)",
                 "STAGE41_OBJECT_WINDOW stage40Seq=",
                 "STAGE40_ACTION_CONSUMER_BUILD=2026-11-10-r2",
                 "expectedSelectGlobal=0x05CF7260u+(g_gameBase-0x00400000u)")
        for anchor in anchors:assert anchor in s, "Missing source anchor: "+anchor
        assert s.count("MakeRelativeCall(")==3, "Stage41 must not add a CALL patch"
        assert s.count("push dword ptr [esp+8]")==2
        assert s.count("push dword ptr [esp+10h]")==4
        print("STAGE41_SOURCE_ANCHORS PASS, three existing CALL patches only")
    print("STAGE41_STATIC_PASS — no Windows x86 compilation/live runtime implied")

if __name__=="__main__":
    main()
