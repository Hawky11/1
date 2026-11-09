#!/usr/bin/env python3
"""Read-only verification of four native EE200 type0 payload variants."""
import argparse
import hashlib
import struct
from pathlib import Path

SHA = '28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
IMAGE_BASE = 0x400000
SIGNATURES = {
    0xEE200: '55 8b ec 53 8b 5d 0c',
    0xEE221: '53 e8 39 a3 06 00',
    0xEE227: '0f b6 48 04 0f b6 50 02 0f b6 78 03',
    0xEE322: '83 fb 04 74 39 83 fb 05 74 34',
    0xEE360: '6a 3d e8 29 71 03 00 e8 14 a5 11 00',
    0xEE36D: '8d 96 3c 3a 00 00 6a 0b 52',
    0x991C0: '55 8b ec 8b 45 10 8b 55 0c',
    0x991CB: 'c1 e1 04 2b c8',
    0x991D9: '89 4c 90 04',
    0x158560: '55 8b ec 8b 45 08 8d 04 c5 80 1f 52 06',
    0x158EC0: '55 8b ec 8b 45 0c 32 c9',
    0x158ECD: '8a 88 2c 22 52 06',
    0xF57EC: 'f6 84 76 88 f1 dd 04 01',
    0xF5800: '89 37',
    0xED19F: '81 a6 84 3b 00 00 ff ff f9 ff',
}
CALLS = {0xEE222: 0x158560, 0xEE2C6: 0x158EC0, 0xEE306: 0xF57D0,
         0xEE315: 0xEC4F0, 0xEE376: 0x991C0, 0xED193: 0xF2ED0}
ROWS = {
    0: '0f 00 00 00 00 00 00 00',
    1: '0f 80 01 01 01 00 00 00',
    4: '0f 00 00 00 00 28 06 00',
    5: '0f 00 01 01 01 28 06 00',
}

def verify(path):
    binary = Path(path).read_bytes()
    digest = hashlib.sha256(binary).hexdigest()
    if digest != SHA:
        raise ValueError('AALib.exe SHA-256 mismatch: ' + digest)
    if binary[:2] != b'MZ':
        raise ValueError('Not an MZ executable')
    pe = struct.unpack_from('<I', binary, 0x3c)[0]
    if binary[pe:pe + 4] != b'PE\x00\x00':
        raise ValueError('Missing PE header')
    machine, count = struct.unpack_from('<HH', binary, pe + 4)
    header_size = struct.unpack_from('<H', binary, pe + 20)[0]
    optional = pe + 24
    if machine != 0x14c or struct.unpack_from('<H', binary, optional)[0] != 0x10b:
        raise ValueError('Expected x86 PE32')
    if struct.unpack_from('<I', binary, optional + 28)[0] != IMAGE_BASE:
        raise ValueError('Unexpected image base')
    sections = []
    for i in range(count):
        h = optional + header_size + i * 40
        virtual_size, rva, raw_size, raw = struct.unpack_from('<IIII', binary, h + 8)
        sections.append((rva, min(virtual_size, raw_size), raw))
    def read(rva, length):
        for start, size, raw in sections:
            if start <= rva and rva + length <= start + size:
                return binary[raw + rva - start:raw + rva - start + length]
        raise ValueError('Unmapped RVA 0x%x' % rva)
    for rva, expected in SIGNATURES.items():
        if read(rva, len(bytes.fromhex(expected))) != bytes.fromhex(expected):
            raise ValueError('Signature mismatch RVA 0x%x' % rva)
    for rva, target in CALLS.items():
        code = read(rva, 5)
        found = rva + 5 + struct.unpack_from('<i', code, 1)[0] if code[0] == 0xe8 else -1
        if found != target:
            raise ValueError('CALL target mismatch RVA 0x%x' % rva)
    for payload, expected in ROWS.items():
        row = read(0x6521F80 - IMAGE_BASE + payload * 8, 8)
        if row != bytes.fromhex(expected):
            raise ValueError('Payload table mismatch %d' % payload)
        flag_id, control_id, output = row[2], row[3], row[4]
        should = 0 if payload in (0, 4) else 1
        if (flag_id, control_id, output) != (should, should, should):
            raise ValueError('Unexpected payload output %d' % payload)
        flag_byte = read(0x652222C - IMAGE_BASE + flag_id, 1)[0]
        controller_desc = read(0x4DDF188 - IMAGE_BASE + control_id * 3, 3)
        if flag_byte != (1 if should == 0 else 3):
            raise ValueError('Unexpected flag table output %d' % payload)
        if controller_desc != (bytes.fromhex('01 00 ff') if should == 0 else bytes.fromhex('01 01 ff')):
            raise ValueError('Unexpected controller descriptor %d' % payload)
        print('payload=%d table=%s 158EC0_flags=0x%02X F57D0_descriptor=%s' %
              (payload, row.hex(' '), flag_byte, controller_desc.hex(' ')))
    if 0x3a3c + 4 + 11 * 4 != 0x3a6c:
        raise ValueError('Controller slot 11 offset mismatch')
    print('PASS: SHA-256, PE32, %d instruction signatures, %d direct CALL targets, 4 payload rows and 8 derived table reads' %
          (len(SIGNATURES), len(CALLS)))
    print('Static analysis only; no MSVC build or runtime behavior verified.')

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', required=True)
    args = parser.parse_args()
    verify(args.binary)
