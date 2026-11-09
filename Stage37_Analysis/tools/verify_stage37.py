#!/usr/bin/env python3
"""Stage37 SHA-pinned original PE32 EE200 type1 -> controller/object state chain.

Static verification only: does not imply actual animation, hit or player command
completion. No binary modifications and no dependency on third-party modules.
"""
import argparse
import hashlib
import struct
from pathlib import Path

EXPECTED_SHA256 = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
SITES = {
    0xEE200: "55 8B EC 53 8B 5D 0C 56 8B F1 8B 46 04 57 33 FF",
    0xEE23B: "83 F8 01 75 56",
    0xEE273: "E8 48 08 04 00",
    0xEE27F: "89 8A 84 01 00 00",
    0xEE2C0: "89 8E 64 3F 00 00",
    0xEE2C6: "E8 F5 AB 06 00",
    0xEE306: "E8 C5 74 00 00",
    0xEE311: "53 57 8B CE",
    0xEE315: "E8 D6 E1 FF FF",
    0xF57D0: "55 8B EC 8B 45 14 8B 4D 10 53 56 57",
    0xF57F8: "33 C0 89 47 04 89 47 08 89 37",
    0xF5800: "89 37 0F B6 8C 76 89 F1 DD 04",
    0xF5854: "D3 E2 09 10",
    0xF58A9: "D3 E2 09 10",
    0xEC4F0: "55 8B EC 83 EC 20 56 8B F1 8D 86 D0 3E 00 00",
    0xEC50A: "81 A6 70 3B 00 00 FF 8F DF 7F",
    0xEC528: "21 BE 74 3B 00 00",
    0xEC53C: "C7 86 84 3F 00 00 02 00 00 00",
    0xEC582: "BA DF FF FF FF 21 50 08",
    0xEC58A: "8B 46 04 B9 7F FF FF FF 21 48 08",
    0xEC5FA: "FF 80 78 02 00 00",
    0xEC603: "C6 82 7C 02 00 00 00",
}
CALL_TARGETS = {
    0xEE273: 0x12EAC0,
    0xEE2C6: 0x158EC0,
    0xEE306: 0xF57D0,
    0xEE315: 0xEC4F0,
    0xEC550: 0xF7EB0,
    0xEC558: 0xF7EB0,
    0xEC560: 0xF7EB0,
}
def main(file):
    data = Path(file).read_bytes()
    digest=hashlib.sha256(data).hexdigest()
    if digest != EXPECTED_SHA256:
        raise RuntimeError("Original EXE SHA256 mismatch: " + digest)
    pe=struct.unpack_from("<I",data,0x3C)[0]
    if data[:2]!=b"MZ" or data[pe:pe+4]!=b"PE\x00\x00":
        raise RuntimeError("Not original PE executable")
    machine,nsections=struct.unpack_from("<HH",data,pe+4)
    opt_size=struct.unpack_from("<H",data,pe+20)[0]
    opt=pe+24
    if machine!=0x14C or struct.unpack_from("<H",data,opt)[0]!=0x10B:
        raise RuntimeError("Expected x86 PE32")
    if struct.unpack_from("<I",data,opt+28)[0]!=0x400000:
        raise RuntimeError("Preferred image base mismatch")
    sections=[]
    for i in range(nsections):
        o=opt+opt_size+40*i
        virt_size,rva,raw_size,raw_ptr=struct.unpack_from("<IIII",data,o+8)
        sections.append((rva,min(virt_size,raw_size),raw_ptr))
    def get(rva,length):
        for start,size,raw in sections:
            if start <= rva and rva+length <= start+size:
                return data[raw+rva-start:raw+rva-start+length]
        raise RuntimeError("Unmapped RVA %08X" % rva)
    for rva,hex_code in SITES.items():
        expected=bytes.fromhex(hex_code)
        actual=get(rva,len(expected))
        if actual != expected:
            raise RuntimeError("Opcode mismatch at RVA %08X: %s" % (rva,actual.hex()))
    for rva,target in CALL_TARGETS.items():
        buf=get(rva,5)
        actual=rva+5+struct.unpack_from("<i",buf,1)[0] if buf[0]==0xE8 else -1
        if actual!=target:
            raise RuntimeError("Direct CALL at RVA %08X targets %08X, expected %08X" %
                               (rva,actual,target))
    print("PASS Stage37: original SHA256, i386 PE32, %d site signatures, %d E8 calls" %
          (len(SITES),len(CALL_TARGETS)))
    print("Verified static TYPE1 EE200->F57D0 controller bitmap and EC4F0 player/object reset.")
    print("NO Windows Stage37 build/run, animation, move, hit or combat result verified.")

if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("--binary",required=True,help="Original AALib.exe from the ChatGPT Library")
    args=parser.parse_args()
    main(args.binary)
