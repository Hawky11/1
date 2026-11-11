#!/usr/bin/env python3
"""Stage75: read-only SHA-pinned native event -> callback-0x11 provenance verifier.

Usage: py -3 Stage75_Analysis/tools/verify_stage75.py "D:\PATH\AALib.exe"
Never modifies process memory, debug registers, DLL or the original game.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

SHA = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
OPCODES = {
  0x0EE7BE:"ffd7",                  # CALL EDI (native callback)
  0x0EE7C7:"e89414f2ff",           # call original FC60 cleanup
  0x0EE7D1:"e8aafcffff",           # EE480 after callback
  0x0EE7D6:"e8c5a5ffff",           # E8DA0 after callback
  0x0EE7E4:"f6c1017409",           # bit23 gate
  0x0EE7E9:"56e8f1fdffff",         # push player; call EE5E0
  0x0EE5E0:"558bec83ec3853568b75088b5e04",
  0x0EE602:"33ff",                 # EDI=0 (mode branch)
  0x0EE604:"e877feffff",           # EE480 again inside EE5E0
  0x0EE609:"80be2c3a000000",       # record valid flag
  0x0EE621:"0fbf86343a0000",       # P+3A34 previous mode
  0x0EE62A:"7416",                 # mode 0 -> EE642
  0x0EE642:"56e848a80600",         # push P, call 158E90
  0x0EE57D:"e8ce480000",           # call F2E50
  0x0EE576:"8db32c3a000056",       # pass P+3A2C event record
  0x0F2E6A:"6689480a",             # previous code [record+0A]
  0x0F2E72:"66895008",             # previous mode [record+08]
  0x0F2E7D:"66895004",             # clear current code [record+04]
  0x0F2DE0:"558bec8b4508",
  0x0F2E0B:"66895004",             # event current code
  0x0F2E0F:"66894802",             # event current mode
  0x158E90:"558bec8b4508",
  0x158E96:"0fbf88363a0000",       # signed word P+3A36
  0x158E9D:"51",                   # push parameter
  0x158E9E:"05d03e0000",           # P+3ED0 callback descriptor address
  0x158EA3:"6870855500",           # original callback VA 00558570
  0x158EA8:"50",                   # push descriptor
  0x158EA9:"e8c2eff9ff",           # call F7E70 setter
  0x0F7E70:"558bec8b45088b4d0c8b551089088950045dc3",
  0x158578:"8b5e0c",               # callback argument in context frame
  0x1585CE:"ff249dc48d5500",     # native switch dispatch
  0x158ACB:"8b877c3b0000",         # switch key 11 branch
  0x158AD1:"c1e80fa801",           # player flags bit15 test
  0x158AD6:"0f84a1000000",         # optional skip selector read
  0x158ADC:"8b87503f0000",         # selector P+3F50 is READ here
}
CALLS = {
  0x0EE7C7:0x00FC60,0x0EE7D1:0x0EE480,0x0EE7D6:0x0E8DA0,
  0x0EE7EA:0x0EE5E0,0x0EE604:0x0EE480,0x0EE643:0x158E90,
  0x0EE57D:0x0F2E50,0x158EA9:0x0F7E70,
}

def original_text(path):
    blob=path.read_bytes()
    digest=hashlib.sha256(blob).hexdigest()
    if digest!=SHA:raise ValueError("Original EXE SHA mismatch: "+digest)
    pe=struct.unpack_from("<I",blob,0x3C)[0]
    if blob[:2]!=b"MZ" or blob[pe:pe+4]!=b"PE"+bytes((0,0)):
        raise ValueError("Missing PE header")
    opt=pe+24
    if (struct.unpack_from("<H",blob,pe+4)[0],
        struct.unpack_from("<H",blob,opt)[0],
        struct.unpack_from("<I",blob,opt+28)[0])!=(0x14C,0x10B,0x400000):
        raise ValueError("Unexpected PE32 x86 image base")
    sec=opt+struct.unpack_from("<H",blob,pe+20)[0]
    for i in range(struct.unpack_from("<H",blob,pe+6)[0]):
        p=sec+i*40
        if blob[p:p+8].split(bytes((0,)))[0]==b".text":
            size,rva,rawsz,raw=struct.unpack_from("<IIII",blob,p+8)
            return digest,rva,blob[raw:raw+rawsz]
    raise ValueError("No native .text section")

def audit(path):
    digest,base,data=original_text(path)
    def read(rva,n):
        offset=rva-base
        if offset<0 or offset+n>len(data):
            raise ValueError("Unmapped RVA "+hex(rva))
        return data[offset:offset+n]
    for rva,h in OPCODES.items():
        expected=bytes.fromhex(h)
        if read(rva,len(expected))!=expected:
            raise ValueError("Opcode changed at RVA %06X"%rva)
    for at,dest in CALLS.items():
        raw=read(at,5)
        if raw[0]!=0xE8 or at+5+struct.unpack_from("<i",raw,1)[0]!=dest:
            raise ValueError("CALL mismatch at RVA %06X"%at)
    all_callers=[]
    for i in range(len(data)-4):
        if data[i]==0xE8 and base+i+5+struct.unpack_from("<i",data,i+1)[0]==0x158E90:
            all_callers.append(base+i)
    if all_callers!=[0x0EE643]:
        raise ValueError("Direct caller inventory changed: "+str(all_callers))
    refs=[]
    pattern=bytes.fromhex("70855500")
    start=0
    while True:
        idx=data.find(pattern,start)
        if idx<0:break
        refs.append(base+idx)
        start=idx+1
    if refs!=[0x158EA4,0x158EB2]:
        raise ValueError("Native callback literal sites differ: "+str(refs))
    tableva=struct.unpack("<I",read(0x158DC4+0x11*4,4))[0]
    if tableva!=0x00558ACB:
        raise ValueError("Native 158570 switch key 11 changed")
    return {
        "status":"STAGE75 STATIC VERIFIED; selector writer IP UNOBSERVED",
        "originalSha256":digest,"opcodeRanges":len(OPCODES),
        "exactRelativeCalls":len(CALLS),
        "directCallersOf158E90":[f"{x:06X}" for x in all_callers],
        "callback":{"registered":"158570","registrationRVA":"158E90",
                    "parameterSource":"signed WORD P+3A36",
                    "descriptorSetterRVA":"0F7E70",
                    "parameterField":"P+3ED4"},
        "record":{"base":"P+3A2C","currentMode":"P+3A2E",
                  "currentCode":"P+3A30","previousMode":"P+3A34",
                  "previousCode":"P+3A36","conditionalTransferRVA":"0F2E50"},
        "switchKey11":{"caseRVA":"158ACB","conditionalSelectorReadRVA":"158ADC",
                       "isWriter":False},
        "newDLL":False,"newRuntimeCapture":False
    }

if __name__=="__main__":
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("original_exe",type=Path)
    print(json.dumps(audit(ap.parse_args().original_exe),indent=2))
    print("PASS: event-to-callback preparation, no native selector writer EIP proof.")
