#!/usr/bin/env python3
"""Stage71 read-only native selector provenance audit on the SHA-pinned original AALib.exe.

Reproduces direct P+3F50 DWORD MOV site inventory, the explicit literal-1 setter,
both native callback caller links, and control-flow facts. Stdlib only. No game/DLL.
Direct memory stores do NOT exhaust alias-based/indirect writes, nor prove which
writer executed before the Stage70 observed selector=1.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

SHA256 = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
# Original-file RVAs and exact bytes of direct MOV DWORD [register+3F50],reg/imm.
# Produced by section-limited i386 disassembly and rechecked in the original PE.
DIRECT_WRITERS = [
    (0x0F3DE9,"898e503f0000"),
    (0x0F3E02,"8996503f0000"),
    (0x0F3E0A,"c786503f000000000000"),
    (0x0F3F50,"8996503f0000"),
    (0x0F3F69,"898e503f0000"),
    (0x0F3F71,"c786503f000000000000"),
    (0x0F466F,"898e503f0000"),
    (0x0F4688,"8996503f0000"),
    (0x0F4690,"c786503f000000000000"),
    (0x0F48A9,"898e503f0000"),
    (0x0F48C2,"8996503f0000"),
    (0x0F48CA,"c786503f000000000000"),
    (0x0F7D22,"8997503f0000"),
    (0x0F7D38,"898f503f0000"),
    (0x0F7D43,"c787503f000000000000"),
    (0x124CFE,"c786503f000000000000"),
    (0x155669,"c781503f000000000000"),
    (0x155D29,"c786503f000001000000"),
    (0x1BDDDE,"c786503f000000000000"),
    (0x1BF38E,"c787503f000000000000"),
]
# Original function call sites; their targets are verified from relative E8.
CALLS = {
    0x1566F0: 0x155B70,  # main callback -> selector logic
    0x1560B9: 0x155B70,  # alternate callback -> same selector logic
    0x155D3C: 0x0F2DE0,  # schedule action/event 0x21
    0x156769: 0x0E5BF0, # Stage65-confirmed immediate requester
}
SITE_BYTES = {
    0x155B79:"8b7d08",              # arg -> EDI
    0x155B7C:"8b37",                # [EDI] -> P=ESI
    0x155B7E:"8b4604",              # [P+4] -> O
    0x155BD1:"8b8e783b0000",       # P+3B78 flag word
    0x155BD7:"c1e902",             # bit2 extraction
    0x155BE0:"f6c101",             # bit2 branch gate
    0x155C16:"d98780000000",       # float O+80 comparison input
    0x155C6D:"d89788000000",       # float O+88 comparison input
    0x155D1D:"c786543f000004000000", # branch A state P+3F54=4
    0x155D27:"6a21",               # event code 0x21
    0x155D74:"c786543f000003000000", # branch B state P+3F54=3
    0x155D7E:"eba7",               # jump branch B -> event+writer
    0x155D44:"5f5eb001",           # function success epilogue begins
    0x1566BB:"83be543f000003",     # callback uses selector state
    0x1566C5:"8b9e503f0000",       # pre-helper selector snapshot
    0x1566F8:"84c0752c",           # successful setter -> early return
    0x1560C1:"84c0754b",           # alternate successful setter -> early return
    0x156770:"8b82503f0000",       # post-helper selector reread
    0x156787:"895068",             # native deferred-ID store
    0x156B84:"68b0665500",         # callback pointer literal in original file
}

def open_pe(path):
    data = path.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != SHA256:
        raise ValueError("Wrong original AALib.exe SHA256: "+digest)
    pe = struct.unpack_from("<I",data,0x3c)[0]
    if data[:2]!=b"MZ" or data[pe:pe+4]!=b"PE\0\0":
        raise ValueError("Not PE")
    machine,nsec=struct.unpack_from("<HH",data,pe+4)
    opt_size=struct.unpack_from("<H",data,pe+20)[0]
    opt=pe+24
    magic=struct.unpack_from("<H",data,opt)[0]
    imagebase=struct.unpack_from("<I",data,opt+28)[0]
    if (machine,magic,imagebase)!=(0x14c,0x10b,0x400000):
        raise ValueError("Unexpected x86 architecture or image base")
    sections=[]
    for i in range(nsec):
        p=opt+opt_size+40*i
        name=data[p:p+8].split(b"\0")[0].decode("ascii")
        virtual_size,rva,raw_size,offset=struct.unpack_from("<IIII",data,p+8)
        sections.append((name,rva,raw_size,offset))
    def read(rva,n):
        matches=[(name,offset+rva-start) for name,start,size,offset in sections
                 if start<=rva and rva+n<=start+size]
        if len(matches)!=1:
            raise ValueError(f"Ambiguous original RVA {rva:06X}")
        name,at=matches[0]
        if name!=".text":
            raise ValueError(f"Unexpected non-.text RVA {rva:06X}: {name}")
        return data[at:at+n]
    return digest,read

def verify(path):
    digest,read=open_pe(path)
    counts={"literal_one":0,"literal_zero":0,"register_derived":0}
    for rva,exact in DIRECT_WRITERS:
        expected=bytes.fromhex(exact)
        if read(rva,len(expected))!=expected:
            raise ValueError(f"Writer differs at {rva:06X}")
        if expected[:1]==b"\xC7":
            n=struct.unpack("<I",expected[-4:])[0]
            key="literal_one" if n==1 else "literal_zero" if n==0 else None
            if key is None:raise ValueError(f"Unexpected writer literal at {rva:06X}")
        else:key="register_derived"
        counts[key]+=1
    for rva,exact in SITE_BYTES.items():
        b=bytes.fromhex(exact)
        if read(rva,len(b))!=b:raise ValueError(f"Original chain opcode mismatch at {rva:06X}")
    for source,target in CALLS.items():
        b=read(source,5)
        if b[0]!=0xE8 or source+5+struct.unpack_from("<i",b,1)[0]!=target:
            raise ValueError(f"Wrong native CALL target at {source:06X}")
    # Both setter's accepted branches converge on the same event+selector write.
    j=read(0x155D7E,2)
    if j[0]!=0xEB or 0x155D80+struct.unpack("<b",j[1:])[0]!=0x155D27:
        raise ValueError("Branch B does not converge on selector write")
    # Caller returns immediately on successful event setter; later callback may
    # consume selector, but its precise scheduling timing is not yet captured.
    if 0x1566FC+struct.unpack("<b",read(0x1566FA,2)[1:])[0]!=0x156728:
        raise ValueError("Main callback return branch drift")
    if 0x1560C5+struct.unpack("<b",read(0x1560C3,2)[1:])[0]!=0x156110:
        raise ValueError("Alternate callback return branch drift")
    if counts!={"literal_one":1,"literal_zero":9,"register_derived":10}:
        raise ValueError(f"Unexpected direct writer counts: {counts}")
    return {
        "source_exe_sha256":digest,
        "direct_MOV_writer_sites_verified":len(DIRECT_WRITERS),
        "site_types":counts,
        "literal_1_writer_RVA":"155D29",
        "setter_function_RVA":"155B70",
        "native_callback_callers":["1566F0","1560B9"],
        "selector_related_state_written":["P+3F54=3","P+3F54=4"],
        "event_id":"0x21",
        "confirmed_source_path_in_original_binary":True,
        "writer_IP_seen_in_stage70_game_run":False,
        "AI_selector_decision_semantics_verified":False,
        "limitation":"Direct MOV-site inventory only, not alias writes; no in-game P+3F50 writer IP observed."
    }

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_exe",type=Path)
    args=parser.parse_args()
    print(json.dumps(verify(args.original_exe),indent=2,ensure_ascii=False))
    print("PASS: Stage71 static selector setter/callback linkage only.")

if __name__=="__main__":main()
