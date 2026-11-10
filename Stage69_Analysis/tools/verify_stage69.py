#!/usr/bin/env python3
"""Stage69: static chain to the positive deferred-ID store (no runtime attribution)."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

EXPECTED_SHA = '28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
SITES = [
 (0x1566B6, '8b 7d 08', 'callback argument to EDI'),
 (0x1566B9, '8b 37', 'load player pointer P'),
 (0x1566BB, '83 be 54 3f 00 00 03', 'player P+3F54 check'),
 (0x1566C2, '8b 46 04', 'load object O from P+4'),
 (0x1566C5, '8b 9e 50 3f 00 00', 'selector P+3F50'),
 (0x1566CB, '89 45 08', 'save O to callback stack slot'),
 (0x156745, '83 fb 01', 'selector 1 conditional'),
 (0x156753, '83 fb 02', 'selector 2 conditional'),
 (0x156767, '8b 0f', 'reload P'),
 (0x156769, 'e8 82 f4 f8 ff', 'native helper call 0E5BF0'),
 (0x15676E, '8b 17', 'reload P after helper'),
 (0x156770, '8b 82 50 3f 00 00', 'selector from P+3F50'),
 (0x156776, '8b 0c 85 64 76 31 06', 'mutable index array at VA 06317664'),
 (0x15677D, '8b 14 8d e8 54 82 00', 'ID table at VA 008254E8'),
 (0x156784, '8b 45 08', 'restore O'),
 (0x156787, '89 50 68', 'write deferred O+68 from EDX'),
 (0x156B84, '68 b0 66 55 00', 'native callback pointer registration'),
 (0x156BA1, '8b 91 50 3f 00 00', 'related selector use'),
 (0x156BA7, '8b 04 95 64 76 31 06', 'related index array'),
 (0x156BAE, '8b 14 85 e8 54 82 00', 'related ID lookup'),
 (0x1E8FEB, 'c6 46 30 01', 'separate O+30 arming candidate'),
 (0x1E9F46, '8b 41 68', 'native deferred read'),
 (0x1E9F4E, '89 41 64', 'native deferred->pending copy'),
 (0x1E9F51, 'c7 41 68 ff ff ff ff', 'native deferred clear'),
]


def inspect(exe):
    data = exe.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != EXPECTED_SHA:
        raise ValueError('Unrecognized original EXE SHA: ' + digest)
    if data[:2] != b'MZ':
        raise ValueError('MZ header missing')
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    if data[pe:pe + 4] != b'PE\0\0':
        raise ValueError('PE signature missing')
    machine, nsec = struct.unpack_from('<HH', data, pe + 4)
    opt_size = struct.unpack_from('<H', data, pe + 20)[0]
    optional = pe + 24
    magic = struct.unpack_from('<H', data, optional)[0]
    imagebase = struct.unpack_from('<I', data, optional + 28)[0]
    if (machine, magic, imagebase) != (0x14c, 0x10b, 0x400000):
        raise ValueError('Unexpected architecture or image base')
    sections = []
    for idx in range(nsec):
        p = optional + opt_size + idx * 40
        name = data[p:p+8].split(b'\0')[0].decode('ascii', errors='replace')
        _, rva, size, raw = struct.unpack_from('<IIII', data, p + 8)
        sections.append((name, rva, size, raw))
    def raw_at_rva(rva, count):
        matches = [(name, raw + rva - base) for name, base, size, raw in sections
                   if base <= rva and rva + count <= base + size]
        if len(matches) != 1:
            raise ValueError(f'RVA {rva:08X} not uniquely file-backed')
        name, offset = matches[0]
        if offset + count > len(data):
            raise ValueError('Truncated file for ' + name)
        return name, data[offset:offset + count]
    for rva, hex_bytes, note in SITES:
        expected = bytes.fromhex(hex_bytes)
        name, actual = raw_at_rva(rva, len(expected))
        if name != '.text' or actual != expected:
            raise ValueError(f'{note}: RVA {rva:06X} expected {hex_bytes} got {actual.hex()} in {name}')
    def dword_va(va):
        _, raw = raw_at_rva(va - imagebase, 4)
        return struct.unpack('<I', raw)[0]
    outputs = []
    for selector, expected_index, expected_action in [(0, 0x1a, 0x24), (1, 0x1b, 0x25), (2, 0x1c, 0x26)]:
        index = dword_va(0x06317664 + 4 * selector)
        action = dword_va(0x008254E8 + 4 * index)
        if (index, action) != (expected_index, expected_action):
            raise ValueError(f'Unexpected original table mapping selector {selector}: {index:X} -> {action:X}')
        outputs.append({'selector': selector, 'fileInitializedIndex': f'{index:X}', 'fileInitializedDeferredID': f'{action:X}'})
    return {'original_sha256': digest, 'exact_site_count': len(SITES),
            'candidate_writer_rva': '156787', 'caller_function_rva': '1566B0',
            'object_from_player_plus_4': True, 'selection_from_player_plus_3F50': True,
            'initial_file_table_mapping': outputs, 'runtime_hit_confirmed': False,
            'note': 'The index table is in writable .data. These are original-file initial values, not measured runtime values.'}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('exe', type=Path)
    args = ap.parse_args()
    print(json.dumps(inspect(args.exe), indent=2, ensure_ascii=False))
    print('PASS: Static native deferred-ID writer candidate. Runtime origin unverified.')

if __name__ == '__main__':
    main()
