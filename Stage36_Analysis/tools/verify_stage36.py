#!/usr/bin/env python3
"""Stage36 read-only verifier: original AALib.exe and native command/predicate sites."""
import argparse
import hashlib
import struct
from pathlib import Path

SHA256 = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
IMAGE_BASE = 0x400000
# All keys are RVAs; table preferred VA is not a raw file offset.
SIGNATURES = {
    0xEB70C: "8D 87 08 3A 00 00 6A 1F 50 88 5D FF",
    0xEB718: "E8 C3 A1 00 00",
    0xEB71D: "83 C4 08 84 C0 0F 84 A9 03 00 00",
    0xEB7D0: "8D 87 08 3A 00 00 6A 1E 50",
    0xEB7D9: "E8 02 A1 00 00",
    0xEB7DE: "83 C4 08 84 C0 74 0B",
    0xF58E0: "55 8B EC 8B 45 0C 56 8B 75 08",
    0xF58EA: "83 F8 26 75 20 8B 4E 04",
    0xF592F: "23 5C 86 04 F7 DB 1B DB F7 DB",
    0xF79D9: "8D 45 FC 50 8D 4D F8 51 56 53 57",
    0xF79E4: "E8 77 C9 FF FF",
    0xF79E9: "83 C4 14 83 7D F8 00 8B F0",
    0xF4360: "55 8B EC 83 EC 14 53 56 8B 75 08",
    0xF43B1: "E8 1A EC FF FF",
    0xF7C59: "56 6A 01 53",
    0xF7C5D: "E8 7E B1 FF FF",
    0xF7C62: "8B 45 FC 83 C4 0C",
    0xEE200: "55 8B EC 53 8B 5D 0C",
    0xEE23B: "83 F8 01 75 56",
    0xEE273: "E8 48 08 04 00",
    0xEE306: "E8 C5 74 00 00",
    0xF5F4C: "83 BE 6C 3A 00 00 00 7E 0A",
}
CALLS = {
    0xEB718: 0xF58E0,
    0xEB7D9: 0xF58E0,
    0xF79CF: 0xF3280,
    0xF79E4: 0xF4360,
    0xF43B1: 0xF2FD0,
    0xF7C5D: 0xF2DE0,
    0xEE273: 0x12EAC0,
    0xEE306: 0xF57D0,
}
def verify(path):
    b = Path(path).read_bytes()
    actual = hashlib.sha256(b).hexdigest()
    if actual != SHA256:
        raise ValueError("AALib.exe SHA-256 mismatch: " + actual)
    if b[:2] != b"MZ":
        raise ValueError("Not MZ")
    pe = struct.unpack_from("<I", b, 0x3C)[0]
    if b[pe:pe+4] != b"PE\x00\x00":
        raise ValueError("Not PE")
    machine, count = struct.unpack_from("<HH", b, pe+4)
    opt = pe+24
    hs = struct.unpack_from("<H", b, pe+20)[0]
    if machine != 0x14C or struct.unpack_from("<H", b, opt)[0] != 0x10B:
        raise ValueError("Expected i386 PE32")
    if struct.unpack_from("<I", b, opt+28)[0] != IMAGE_BASE:
        raise ValueError("Unexpected image base")
    sections = []
    for i in range(count):
        o = opt+hs+40*i
        vs, rva, raw_size, raw = struct.unpack_from("<IIII", b, o+8)
        sections.append((rva, min(vs, raw_size), raw))
    def read(rva, n):
        for start, size, raw in sections:
            if start <= rva and rva+n <= start+size:
                return b[raw+rva-start:raw+rva-start+n]
        raise ValueError("Unmapped RVA 0x%x" % rva)
    for rva, code in SIGNATURES.items():
        expected = bytes.fromhex(code)
        if read(rva, len(expected)) != expected:
            raise ValueError("Signature mismatch RVA 0x%X" % rva)
    for rva, target in CALLS.items():
        code = read(rva, 5)
        dest = rva+5+struct.unpack_from("<i", code, 1)[0] if code[0] == 0xE8 else -1
        if dest != target:
            raise ValueError("CALL mismatch RVA 0x%X: 0x%X != 0x%X" % (rva, dest, target))
    if 0x3A08+4 != 0x3A0C or 0x3A08+8 != 0x3A10:
        raise ValueError("Controller bitmap offset mismatch")
    print("PASS: original SHA256, i386 PE32, %d signatures, %d direct CALLs" %
          (len(SIGNATURES), len(CALLS)))
    print("F58E0 input1F/input1E, F4360 candidate and F7C5D queue emission verified.")
    print("This is STATIC ONLY: no Windows MSVC build, runtime data, action animation or hit verified.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", required=True)
    verify(parser.parse_args().binary)
