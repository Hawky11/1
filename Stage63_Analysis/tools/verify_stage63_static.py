#!/usr/bin/env python3
"""Offline SHA-pinned AALib.exe P+3F4C native direct-write/callsite verifier."""
import argparse
import collections
import hashlib
import json
import struct
from pathlib import Path

SHA256 = '28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
SITES = {
    0xE5BF0: '55 8B EC 80 7D 0C 00 74 17',
    0xE5BFC: '81 89 7C 3B 00 00 00 00 00 01',
    0xE5C06: '89 81 4C 3F 00 00',
    0xE5C13: '89 91 4C 3F 00 00',
    0xE9E30: '55 8B EC 56 8B F1',
    0xE9E5A: '8B 15 E8 54 82 00',
    0xE9E60: '89 96 4C 3F 00 00',
    0xE853F: 'C7 86 4C 3F 00 00 FF FF FF FF',
    0xE8489: '8B 86 4C 3F 00 00',
    0x131CC5: 'E8 26 3F FB FF',
}
EDGES = {
    0xAD72F: 0xE8480,
    0x123B24: 0x1BD6E0,
    0x12EB83: 0x131CA0,
    0x131CC5: 0xE5BF0,
    0x131D1C: 0xE5BF0,
    0x1DA592: 0x1BD6E0,
}
CALL_TARGETS = {0xE5BF0: 296, 0xE9E30: 11, 0xE8480: 1}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('exe', type=Path, help='Original private AALib.exe, not a mod DLL')
    ap.add_argument('--json', type=Path, help='Optional JSON report')
    args = ap.parse_args()
    payload = args.exe.read_bytes()
    digest = hashlib.sha256(payload).hexdigest()
    if digest != SHA256:
        raise SystemExit(f'FAIL: SHA256 differs: {digest}')
    pe = struct.unpack_from('<I', payload, 0x3C)[0]
    if payload[pe:pe + 4] != b'PE\0\0':
        raise SystemExit('FAIL: PE signature')
    machine, count = struct.unpack_from('<HH', payload, pe + 4)
    optional_size = struct.unpack_from('<H', payload, pe + 20)[0]
    magic = struct.unpack_from('<H', payload, pe + 24)[0]
    base = struct.unpack_from('<I', payload, pe + 24 + 28)[0]
    if (machine, magic, base) != (0x14C, 0x10B, 0x400000):
        raise SystemExit('FAIL: not original PE32 i386 ImageBase 00400000')
    sections = []
    for i in range(count):
        off = pe + 24 + optional_size + i * 40
        name = payload[off:off + 8].split(b'\0')[0].decode()
        virtual_size, va, raw_size, raw_ptr = struct.unpack_from('<IIII', payload, off + 8)
        sections.append((name, va, virtual_size, raw_ptr, raw_size))

    def locate(rva, length):
        for name, va, vsize, ptr, raw_size in sections:
            if va <= rva and rva + length <= va + min(vsize, raw_size):
                return ptr + rva - va
        raise ValueError(f'RVA {rva:06X} not mapped')

    def code(rva, length):
        off = locate(rva, length)
        return payload[off:off + length]

    for rva, spelling in SITES.items():
        expected = bytes.fromhex(spelling)
        actual = code(rva, len(expected))
        if actual != expected:
            raise SystemExit(f'FAIL signature RVA {rva:06X}: actual {actual.hex()}')

    def rel_call_target(rva):
        op = code(rva, 5)
        if op[0] != 0xE8:
            raise SystemExit(f'FAIL no rel32 CALL at {rva:06X}')
        return (rva + 5 + struct.unpack_from('<i', op, 1)[0]) & 0xFFFFFFFF

    for src, dst in EDGES.items():
        actual = rel_call_target(src)
        if actual != dst:
            raise SystemExit(f'FAIL edge {src:06X} => {actual:06X}, expected {dst:06X}')

    text = next((s for s in sections if s[0] == '.text'), None)
    if text is None:
        raise SystemExit('FAIL no .text section')
    _, va, vsize, off, raw_size = text
    segment = payload[off:off + min(vsize, raw_size)]
    refs = collections.defaultdict(list)
    for i in range(len(segment) - 4):
        if segment[i] != 0xE8:
            continue
        addr = va + i
        target = (addr + 5 + struct.unpack_from('<i', segment, i + 1)[0]) & 0xFFFFFFFF
        if target in CALL_TARGETS:
            refs[target].append(addr)
    for target, expected_count in CALL_TARGETS.items():
        if len(refs[target]) != expected_count:
            raise SystemExit(f'FAIL calls to {target:06X}: {len(refs[target])} != {expected_count}')
    report = {
        'sha256': digest,
        'image_base': '0x00400000',
        'checked_write_and_read_signatures': len(SITES),
        'checked_example_rel32_edges': len(EDGES),
        'exact_direct_e8_xrefs': {
            f'0x{target:06X}': {
                'count': len(refs[target]),
                'site_rvas': [f'0x{v:06X}' for v in refs[target]],
            } for target in sorted(refs)
        },
        'limitations': 'Direct fixed-displacement and E8 callsite scan only; aliases, indirect calls, or engine behavior are not ruled out.',
    }
    print('PASS SHA-256/PE32/i386/ImageBase,', len(SITES), 'signatures,', len(EDGES), 'direct CALL edges')
    for rva in sorted(refs):
        print(f'  target RVA {rva:06X}: {len(refs[rva])} direct E8 references')
    if args.json:
        args.json.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        print('Report:', args.json)


if __name__ == '__main__':
    main()
