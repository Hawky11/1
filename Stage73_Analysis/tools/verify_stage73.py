#!/usr/bin/env python3
"""Stage73 read-only source-route / selector Boolean audit, pinned to the original PE32.
Usage: py -3 Stage73_Analysis/tools/verify_stage73.py "D:\\PATH\\AALib.exe"
Does not modify executable, process, memory, game logic, or debugging registers.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

SHA = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
ZERO = [0x0F3E0A,0x0F3F71,0x0F4690,0x0F48CA,0x0F7D43,
        0x124CFE,0x155669,0x1BDDDE,0x1BF38E]
# site: (pointer register, value register, SETcc opcode)
REG = {
  0x0F3DE9:("esi","ecx",0x95),0x0F3E02:("esi","edx",0x94),
  0x0F3F50:("esi","edx",0x95),0x0F3F69:("esi","ecx",0x94),
  0x0F466F:("esi","ecx",0x95),0x0F4688:("esi","edx",0x94),
  0x0F48A9:("esi","ecx",0x95),0x0F48C2:("esi","edx",0x94),
  0x0F7D22:("edi","edx",0x95),0x0F7D38:("edi","ecx",0x94),
}

def original_text(path):
    blob=path.read_bytes()
    digest=hashlib.sha256(blob).hexdigest()
    if digest!=SHA: raise ValueError("Wrong original SHA-256: "+digest)
    pe=struct.unpack_from("<I",blob,0x3C)[0]
    if blob[:2]!=b"MZ" or blob[pe:pe+4]!=b"PE"+bytes((0,0)):
        raise ValueError("Not a PE executable")
    opt=pe+24
    if struct.unpack_from("<H",blob,pe+4)[0]!=0x14C or struct.unpack_from("<H",blob,opt)[0]!=0x10B:
        raise ValueError("Not original x86 PE32")
    if struct.unpack_from("<I",blob,opt+28)[0]!=0x400000:
        raise ValueError("Unexpected image base")
    sections=opt+struct.unpack_from("<H",blob,pe+20)[0]
    for i in range(struct.unpack_from("<H",blob,pe+6)[0]):
        off=sections+40*i
        if blob[off:off+8].split(bytes((0,)))[0]==b".text":
            vs,rva,size,raw=struct.unpack_from("<IIII",blob,off+8)
            return rva,blob[raw:raw+size]
    raise ValueError("Missing .text")

def audit(path):
    base,text=original_text(path)
    def read(rva,n):
        i=rva-base
        if i<0 or i+n>len(text): raise ValueError("Unmapped original RVA")
        return text[i:i+n]
    def exact(rva,hexbytes):
        val=bytes.fromhex(hexbytes)
        if read(rva,len(val))!=val:
            raise ValueError("Original opcode changed at RVA %06X" % rva)
    for site in ZERO:
        mod="87" if site in (0x0F7D43,0x1BF38E) else "81" if site==0x155669 else "86"
        exact(site,"c7"+mod+"503f000000000000")
    exact(0x155D29,"c786503f000001000000")
    exact(0x155D1D,"c786543f000004000000")
    exact(0x155D74,"c786543f000003000000")
    exact(0x155D7E,"eba7")
    if 0x155D80+struct.unpack("<b",read(0x155D7F,1))[0]!=0x155D27:
        raise ValueError("State3 branch no longer converges")
    exact(0x155D27,"6a21")
    exact(0x155D3C,"e89fd0f9ff")
    direct=[]
    for off in range(len(text)-4):
        if text[off]==0xE8 and base+off+5+struct.unpack_from("<i",text,off+1)[0]==0x155B70:
            direct.append(base+off)
    if direct!=[0x1560B9,0x1566F0]:
        raise ValueError("Direct callsites changed: "+str(direct))
    for rva,opcode in {
      0x1560B9:"e8b2faffff", 0x1560C1:"84c0754b",
      0x1566F0:"e87bf4ffff", 0x1566F8:"84c0752c",
      0x1566BB:"83be543f000003",0x1566C5:"8b9e503f0000",
      0x156769:"e882f4f8ff",0x156770:"8b82503f0000",
      0x156787:"895068",
    }.items(): exact(rva,opcode)
    for site,(pointer,reg,cc) in REG.items():
        store="89"+({"esi":{"ecx":"8e","edx":"96"},"edi":{"ecx":"8f","edx":"97"}}[pointer][reg])+"503f0000"
        exact(site,store)
        regclear="33c9" if reg=="ecx" else "33d2"
        lowreg="c1" if reg=="ecx" else "c2"
        reginc="41" if reg=="ecx" else "42"
        pre=regclear+"84c00f"+("%02x" % cc)+lowreg+reginc
        exact(site-len(bytes.fromhex(pre)),pre)
    for cc in (0x94,0x95):
        for al in (0,1):
            result=1+(int(al==0) if cc==0x94 else int(al!=0))
            assert result in (1,2)
    return {
        "status":"STAGE73 STATIC VERIFIED; runtime selector writer EIP unknown",
        "originalSha256":SHA,"literalOne":"155D29",
        "literalZero":len(ZERO),"registerDerived":len(REG),
        "setneSites":[f"{k:06X}" for k,v in REG.items() if v[2]==0x95],
        "seteSites":[f"{k:06X}" for k,v in REG.items() if v[2]==0x94],
        "directCallersOf155B70":[f"{x:06X}" for x in direct],
        "sameInvocationSuccessfulSetterAndDeferredWriter":False,
        "runtimeWriterEipSeen":False,
    }

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_exe",type=Path)
    print(json.dumps(audit(p.parse_args().original_exe),indent=2))
    print("PASS: static source paths only; no new runtime attribution")

if __name__=="__main__": main()
