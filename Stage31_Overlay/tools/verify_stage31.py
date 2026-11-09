#!/usr/bin/env python3
"""Verify Stage31 source signatures against the pinned original PE, read-only."""
import argparse,hashlib,pathlib,re,struct
p=argparse.ArgumentParser();p.add_argument('--binary',required=True);args=p.parse_args()
b=pathlib.Path(args.binary).read_bytes()
assert hashlib.sha256(b).hexdigest()=='28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba','Wrong executable'
pe=struct.unpack_from('<I',b,0x3c)[0]
n=struct.unpack_from('<H',b,pe+6)[0];opt=struct.unpack_from('<H',b,pe+20)[0]
sections=[struct.unpack_from('<4I',b,pe+24+opt+i*40+8) for i in range(n)]
def read(rva,size):
    for vs,va,rs,raw in sections:
        if va<=rva and rva+size<=va+rs:return b[raw+rva-va:raw+rva-va+size]
    raise ValueError('Unmapped RVA')
s=(pathlib.Path(__file__).resolve().parents[1]/'src/AIQueueTrace.cpp').read_text()
for name,rva in [('Prefix',0xf6a88),('Call',0xf6a8c),('Suffix',0xf6a91),('Prologue',0x130a10)]:
    literal=re.search(r'kStage31'+name+r'\[\d+\] = \{([^}]+)\}',s)[1]
    expected=bytes(int(x.strip(),0) for x in literal.split(','))
    assert read(rva,len(expected))==expected,(name,hex(rva))
assert 0xf6a91+struct.unpack('<i',read(0xf6a8d,4))[0]==0x130a10
assert 0x156e4c+struct.unpack('<i',read(0x156e48,4))[0]==0xf2de0
assert 'EnableStage31AlternatePathTrace=1' in (pathlib.Path(__file__).resolve().parents[1]/'ArcanaAI.ini').read_text()
print('PASS: pinned SHA256, four source byte signatures, alternate/queue E8 targets, enabled INI')
