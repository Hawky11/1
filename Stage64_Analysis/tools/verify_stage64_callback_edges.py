#!/usr/bin/env python3
"""Read-only SHA-pinned callback-helper call-graph checks for Stage64."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct

SHA = '28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
RVA_BASE = 0x400000
LINE = re.compile(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2}\s+)+)\s*(.*)$')
EXPECTED = {0x131CA0: 19, 0x131D00: 11}
DIRECT_EDGES = {0x123B24: 0x1BD6E0, 0x1DA592: 0x1BD6E0,
                0x131CC5: 0xE5BF0, 0x131D1C: 0xE5BF0,
                0x12EB83: 0x131CA0, 0x12EBD5: 0x131D00}


def inspect(exe, disasm):
    if hashlib.sha256(exe.read_bytes()).hexdigest() != SHA:
        raise ValueError('Not SHA-pinned original AALib.exe')
    all_edges = []
    for line in disasm.open(errors='replace'):
        m = LINE.match(line)
        if not m:
            continue
        site = int(m.group(1), 16) - RVA_BASE
        code = bytes.fromhex(m.group(2))
        if code[:1] != b'\xe8' or len(code) != 5:
            continue
        if not m.group(3).strip().startswith('call'):
            raise ValueError('E8 disassembly unexpectedly not CALL at %06X' % site)
        dest = (site + 5 + struct.unpack_from('<i', code, 1)[0]) & 0xffffffff
        all_edges.append((site, dest))
    table = dict(all_edges)
    for src, dst in DIRECT_EDGES.items():
        if table.get(src) != dst:
            raise ValueError('Direct edge mismatch %06X to %06X' % (src, dst))
    internal = {}
    for target, expected in EXPECTED.items():
        hits = [site for site,dest in all_edges
                if 0x12EB10 <= site < 0x12EE60 and dest == target]
        if len(hits) != expected:
            raise ValueError('Callback 12EB10 target %06X count %d not %d' % (target,len(hits),expected))
        internal[f'{target:06X}'] = [f'{h:06X}' for h in hits]
    shared = [site for site,dest in all_edges if dest == 0x1BD6E0]
    if len(shared) != 34:
        raise ValueError('Native shared callback helper 1BD6E0 xrefs %d != 34' % len(shared))
    return {
        'original_sha256': SHA,
        'callback_12EB10_direct_edges': internal,
        'callback_123AF0_to_shared_helper': '123B24',
        'callback_1DA490_to_shared_helper': '1DA592',
        'shared_helper_1BD6E0_direct_xrefs': len(shared),
        'request_setter_E5BF0_calls_from_handlers': ['131CC5', '131D1C'],
        'limitations': 'Static direct CALL edges only: conditional execution, request success, and COM utility scoring remain unproven.'
    }


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('exe',type=Path)
    p.add_argument('--objdump',type=Path,required=True)
    p.add_argument('--json',type=Path)
    a=p.parse_args()
    result=inspect(a.exe,a.objdump)
    print('PASS 12EB10 -> 131CA0 = 19, 131D00 = 11; shared 1BD6E0 xrefs = 34')
    if a.json:
        a.json.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')

if __name__=='__main__':
    main()
