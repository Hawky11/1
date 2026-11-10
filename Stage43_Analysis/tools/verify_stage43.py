#!/usr/bin/env python3
"""Stage43 original PE caller-site and one-file source verifier, stdlib only."""
import argparse, hashlib, struct
from pathlib import Path
SHA="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
SIZE=175556608
SITES=[0x10164,0x1023A,0xAD75C,0xAD778,0xAD797,0xAD7BC,0xAEA69,
       0xAEAD9,0xAEB09,0x13A20B,0x13DFB9,0x1917AB,0x19C2E1,0x1A8E48]
def main():
    a=argparse.ArgumentParser()
    a.add_argument("exe",type=Path)
    a.add_argument("--source",type=Path)
    args=a.parse_args()
    b=args.exe.read_bytes()
    assert len(b)==SIZE and hashlib.sha256(b).hexdigest()==SHA,"Unexpected original EXE"
    pe=struct.unpack_from("<I",b,0x3c)[0]
    assert b[pe:pe+4]==b"PE\0\0"
    assert struct.unpack_from("<H",b,pe+4)[0]==0x14c
    opt=pe+24
    assert struct.unpack_from("<H",b,opt)[0]==0x10b
    secs=[]
    sh=opt+struct.unpack_from("<H",b,pe+20)[0]
    for i in range(struct.unpack_from("<H",b,pe+6)[0]):
        off=sh+40*i
        name=b[off:off+8].split(b"\0")[0]
        va,rawsize,raw=struct.unpack_from("<III",b,off+12)
        secs.append((va,rawsize,raw))
    def get(rva,n):
        for va,size,raw in secs:
            if va<=rva and rva+n<=va+size:
                return b[raw+rva-va:raw+rva-va+n]
        raise ValueError("Bad RVA %07X"%rva)
    assert get(0x1e9f40,6)==bytes.fromhex("80 79 30 00 74 16")
    assert get(0x1e9f62,5)==bytes.fromhex("E8 09 FD FF FF")
    for site in SITES:
        z=get(site,5)
        assert z[0]==0xe8 and site+5+struct.unpack("<i",z[1:])[0]==0x1e9f40
        print("CALL PASS %07X -> 1E9F40"%site)
    print("ORIGINAL EXE CALLS: 14/14 PASS")
    if args.source:
        c=args.source.read_text(encoding="utf-8")
        for anchor in (
            "STAGE43_ACTIVATION_ORIGIN_BUILD=2026-11-10-r1",
            "STAGE43_ENABLED piggyback=Stage42 1/1",
            "STAGE43_ORIGIN_WINDOW",
            "SafeReadDword(stackAfterPushfd+4u,&nativeRet)",
            "SafeReadDword(stackAfterPushfd+8u,&callerRet)",
            "r.CallerRetRVA-5u",
            "knownCalls[14]",
            "STAGE42_INSTALLED 1/1",
            "call dword ptr [g_s42NativeActivation]",
        ): assert anchor in c,"Missing Stage43 source anchor "+anchor
        for site in SITES:
            assert "0x%Xu"%site in c,"Missing caller RVA %X"%site
        assert c.count("MakeRelativeCall(")==1,"Must keep exactly one Stage42 original CALL patch"
        assert c.count("fxsave [edi]")==2 and c.count("fxrstor [edi]")==2
        print("STAGE43 SOURCE ANCHORS PASS, no new hooks")
    print("STAGE43_STATIC_PASS: NOT a Windows x86 build or native runtime test")
if __name__=="__main__":
    main()
