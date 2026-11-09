#!/usr/bin/env python3
"""Read-only Stage35 native downstream-consumer verifier for the original AALib.exe."""
import argparse
import hashlib
import struct
from pathlib import Path

EXPECTED_SHA256 = '28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
IMAGE_BASE = 0x400000
# All labels are RVA, NOT preferred VA or raw file offsets.
SIGNATURES = {
    0xA1106: 'e8 85 b9 00 00',
    0xACA90: '53 56 57 33 db 53 33 ff',
    0xACABD: '56 e8 fd 95 04 00',
    0xACAC3: '56 e8 07 91 04 00',
    0xACAC9: '8b 76 04 57 53',
    0xF58E0: '55 8b ec 8b 45 0c 56 8b 75 08',
    0xF58EA: '83 f8 26 75 20 8b 4e 04',
    0xF592F: '23 5c 86 04 f7 db 1b db f7 db',
    0xF5BD0: '55 8b ec 83 ec 1c 53 56 8b 75 08',
    0xF5C0C: '81 8e 78 3b 00 00 80 03 00 00',
    0xF5C71: '81 a6 78 3b 00 00 7f ff ff ff',
    0xF5F4C: '83 be 6c 3a 00 00 00 7e 0a',
    0xF5F55: '81 8e 78 3b 00 00 80 00 00 00',
    0xF60C0: '55 8b ec 83 ec 10 53 56 8b 75 08',
    0xF610E: '33 db 53 6a 01 8d be 3c 3a 00 00 6a 0b 57',
    0xF7ED0: '55 8b ec 80 7d 14 00 8b 4d 08',
    0xF7EB0: '55 8b ec 8b 45 0c 8b 4d 08',
    0xEB7D0: '8d 87 08 3a 00 00 6a 1e 50',
    0xEB7E1: '84 c0 74 0b',
}
CALLS = {
    0xA1106: 0xACA90,
    0xACAB2: 0xE6EC0,
    0xACAB8: 0xF5990,
    0xACABE: 0xF60C0,
    0xACAC4: 0xF5BD0,
    0xF611C: 0xF7ED0,
    0xEB7D9: 0xF58E0,
}

def verify(path):
    binary = Path(path).read_bytes()
    digest = hashlib.sha256(binary).hexdigest()
    if digest != EXPECTED_SHA256:
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
    if 0x3a3c + 4 + 11 * 4 != 0x3a6c:
        raise ValueError('Controller slot11 offset mismatch')
    print('PASS: SHA-256, PE32, %d signatures, %d direct CALL targets and slot11 offset' %
          (len(SIGNATURES), len(CALLS)))
    print('Verified: per-side ACA90 -> F60C0 -> F7ED0; ACA90 -> F5BD0; EB7D9 -> F58E0')
    print('Static analysis only: no successful action, MSVC build or runtime verified.')

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', required=True)
    verify(parser.parse_args().binary)
