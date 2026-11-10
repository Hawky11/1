#!/usr/bin/env python3
"""Original PE32 signatures; no game process or gameplay writes."""
import hashlib,json,struct,sys
from pathlib import Path
b=Path(sys.argv[1]).read_bytes()
assert hashlib.sha256(b).hexdigest()=='28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
pe=struct.unpack_from('<I',b,0x3c)[0]
assert b[pe:pe+4]==b'PE\0\0' and struct.unpack_from('<H',b,pe+4)[0]==0x14c
ns=struct.unpack_from('<H',b,pe+6)[0]; opt=struct.unpack_from('<H',b,pe+20)[0]
assert struct.unpack_from('<H',b,pe+24)[0]==0x10b
sections=[struct.unpack_from('<8sIIII',b,pe+24+opt+i*40) for i in range(ns)]
def read(r,n):
 for name,vs,va,size,ptr in sections:
  if va<=r and r+n<=va+size:return b[ptr+r-va:ptr+r-va+n]
 raise ValueError(hex(r))
sigs=json.loads((Path(__file__).resolve().parents[1]/'signatures.json').read_text())
for addr,expected in sigs.items():assert read(int(addr,16),len(expected)//2).hex()==expected,addr
calls={0xee7ae:0x156c00,0xee7c7:0xfc60,0xee63b:0x12eee0,0x12ef82:0xf7e70,0x12efc3:0xf7e70}
for addr,target in calls.items():
 raw=read(addr,5); assert raw[0]==0xe8 and addr+5+struct.unpack('<i',raw[1:])[0]==target
assert read(0xee7be,2)==b'\xff\xd7'
print(f'PASS SHA256, PE32/i386, {len(sigs)} exact signatures, {len(calls)} direct calls, original CALL EDI')
