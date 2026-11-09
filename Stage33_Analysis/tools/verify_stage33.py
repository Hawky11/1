#!/usr/bin/env python3
"""SHA-pinned, read-only PE32 verifier for Stage33's native type0/payload0 path."""
import argparse
import hashlib
import struct
from pathlib import Path

SHA256 = '28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
SIGNATURES = {
    0xEE200: '55 8b ec 53 8b 5d 0c',
    0xEE216: '8b 45 08 33 d2 33 c9 3b c7 75 1a',
    0xEE221: '53',
    0xEE227: '0f b6 48 04 0f b6 50 02 0f b6 78 03',
    0xEE2C0: '89 8e 64 3f 00 00',
    0xEE2FF: '8d 86 08 3a 00 00',
    0xEE31A: '85 ff',
    0xEE322: '83 fb 04',
    0xEE34F: '8b 76 04',
    0xEE352: '81 66 08 ff fb ff ff',
    0xEC4F0: '55 8b ec 83 ec 20',
    0xEC50A: '81 a6 70 3b 00 00 ff 8f df 7f',
    0xEC550: 'e8 5b b9 00 00',
    0xEC55D: '6a 39 53',
    0xEC568: '83 7d 08 00',
    0xEC57F: '8b 46 04 ba df ff ff ff',
    0xED188: '6a 1e',
    0xED18A: '8d 86 2c 3a 00 00',
    0xED190: '6a 01 50',
    0xED19B: '84 c0 75 0a',
    0xED19F: '81 a6 84 3b 00 00 ff ff f9 ff',
    0xF2ED0: '55 8b ec 8b 45 08',
    0xF2ED6: '0f bf 48 08',
    0xF2EDF: '0f bf 50 0a',
}
CALLS = {0xEE222:0x158560, 0xEE2C6:0x158EC0, 0xEE306:0xF57D0,
         0xEE315:0xEC4F0, 0xED193:0xF2ED0}
TABLES = {
    0x6521F80: bytes.fromhex('0f 00 00 00 00 00 00 00'),
    0x652222C: bytes.fromhex('01'),
    0x4DDF188: bytes.fromhex('01 00 ff'),
}

def verify(exe):
    blob=Path(exe).read_bytes()
    h=hashlib.sha256(blob).hexdigest()
    if h != SHA256:
        raise ValueError('unexpected EXE SHA256 '+h)
    assert blob[:2] == b'MZ'
    pe=struct.unpack_from('<I',blob,0x3c)[0]
    assert blob[pe:pe+4] == b'PE\0\0'
    machine,num=struct.unpack_from('<HH',blob,pe+4)
    optsize=struct.unpack_from('<H',blob,pe+20)[0]
    optional=pe+24
    assert machine==0x14c and struct.unpack_from('<H',blob,optional)[0]==0x10b
    base=struct.unpack_from('<I',blob,optional+28)[0]
    assert base==0x400000
    sections=[]
    for i in range(num):
        at=optional+optsize+40*i
        virtual_size,rva,raw_size,raw_pointer=struct.unpack_from('<IIII',blob,at+8)
        sections.append((rva,min(virtual_size,raw_size),raw_pointer))
    def read(rva,n):
        for start,extent,ptr in sections:
            if rva >= start and rva+n <= start+extent:
                return blob[ptr+rva-start:ptr+rva-start+n]
        raise ValueError('unmapped RVA 0x%X' % rva)
    for at,hexbytes in SIGNATURES.items():
        b=bytes.fromhex(hexbytes)
        have=read(at,len(b))
        if have != b: raise ValueError('signature at %X: got %s expected %s' % (at,have.hex(),b.hex()))
    for at,target in CALLS.items():
        b=read(at,5)
        actual=at+5+struct.unpack_from('<i',b,1)[0] if b[0]==0xe8 else -1
        if actual!=target: raise ValueError('CALL at %X: %X != %X' %(at,actual,target))
    for va,expected in TABLES.items():
        actual=read(va-base,len(expected))
        if actual != expected: raise ValueError('table VA 0x%X differs'%va)
    print('PASS AALib.exe: SHA256, PE32, %d signatures, %d call targets, %d tables' %
          (len(SIGNATURES),len(CALLS),len(TABLES)))
    print('type0 payload0: index0 table returns edx=0, edi=0, ecx=0; native EC4F0 calls F2ED0(queue,1,0x1E) and conditionally clears bit17/18')
    print('NOTE static proof only: no claim of successful gameplay action or Windows DLL build')

if __name__=='__main__':
    ap=argparse.ArgumentParser()
    ap.add_argument('--binary',required=True)
    args=ap.parse_args()
    verify(args.binary)
