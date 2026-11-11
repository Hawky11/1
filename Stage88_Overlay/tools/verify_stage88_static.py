#!/usr/bin/env python3
"""Stage88 static guard. No runtime claims and no binary modifications."""
import hashlib
import struct
import sys
import zipfile
from pathlib import Path

EXPECTED = '28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'

def main(path):
    data = Path(path).read_bytes()
    if zipfile.is_zipfile(path):
        with zipfile.ZipFile(path) as z:
            assert z.namelist() == ['AALib.exe'], 'unexpected ZIP members'
            data = z.read('AALib.exe')
    assert hashlib.sha256(data).hexdigest() == EXPECTED, 'wrong game binary SHA256'
    nt = struct.unpack_from('<I', data, 0x3c)[0]
    assert data[nt:nt+4] == b'PE\0\0' and struct.unpack_from('<H', data,nt+4)[0] == 0x14c
    opts = struct.unpack_from('<H',data,nt+20)[0]
    sects = []
    for i in range(struct.unpack_from('<H',data,nt+6)[0]):
        o = nt + 24 + opts + i*40
        va,virtual,raw_size,raw = (struct.unpack_from('<I',data,o+12)[0], struct.unpack_from('<I',data,o+8)[0], struct.unpack_from('<I',data,o+16)[0], struct.unpack_from('<I',data,o+20)[0])
        sects.append((va, max(virtual,raw_size),raw))
    def at(rva,n):
        for va,size,raw in sects:
            if va <= rva and rva+n <= va+size:
                return data[raw+rva-va:raw+rva-va+n]
        raise ValueError(f'unmapped RVA {rva:x}')
    signatures = {
       0xEE2D0: '83 ff 2c',
       0xEE2D5: '8b 8e 50 3f 00 00',
       0xEE2DB: '8d 47 e2',
       0xEE2E3: '8d 47 e3',
       0xEE2ED: '8d 41 0e',
       0xEE2F0: '8b 8e 34 3f 00 00',
       0xEE2F6: '8b 96 30 3f 00 00',
       0xEE2FC: '51 52 50 8d 86 08 3a 00 00 50',
       0xEE306: 'e8 c5 74 00 00',
       0xF57D0: '55 8b ec 8b 45 14 8b 4d 10',
       0xF57EC: 'f6 84 76 88 f1 dd 04 01',
       0xF5800: '89 37',
       0xF5817: '8b 83 50 3f 00 00',
       0xF586D: '8b 83 50 3f 00 00',
    }
    for rva,hexs in signatures.items():
        value=bytes.fromhex(hexs)
        assert at(rva,len(value))==value, f'bad original bytes at {rva:06x}'
    call=at(0xEE306,5); delta=struct.unpack_from('<i',call,1)[0]
    assert 0xEE306+5+delta==0xF57D0, 'Stage24 call target moved'
    desc=[]
    for code in (0x0e,0x0f,0x10):
        x=at(0x049DF188+code*3,3)
        assert x==b'\x01\x26\xff', f'unexpected F57D0 descriptor for {code:02x}: {x.hex()}'
        desc.append(x.hex())
    print(f'PASS_STATIC_STAGE88: SHA256={EXPECTED} 14 opcodes + 1 E8 target + 3 table rows')
    print('selector=0/1/2 mapped=0E/0F/10 descriptor=01 26 FF; native F5817 bit index=9/10/11')
    print('CAUTION: static mapping/bitmap != rendered animation, movement or successful attack')

if __name__=='__main__':
    main(sys.argv[1] if len(sys.argv)>1 else 'AALib.zip')