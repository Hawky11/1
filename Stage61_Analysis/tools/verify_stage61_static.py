#!/usr/bin/env python3
"""Read-only Stage61 static verifier; do not redistribute original AALib.exe."""
import argparse
import hashlib
import struct
from pathlib import Path

SHA256 = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
# Relative virtual addresses, not image-base-adjusted VAs.
CALLS = {
    0x152767: 0x1E4A00, 0x1527F8: 0x1E4A00,
    0x153675: 0x1E4A00, 0x1E4ADE: 0x1E4A00,
    0x15541F: 0x1E4AB0, 0x155479: 0x1E4AB0,
    0x1554CB: 0x1E4AB0, 0x155515: 0x1E4AB0,
    0x1E4C2B: 0x1E4AB0,
    0x0AE70C: 0x1E66E0, 0x0AE79F: 0x1E66E0,
    0x0AE7BD: 0x1E66E0, 0x0AE849: 0x1E66E0,
    0x0AE93E: 0x1E66E0, 0x0AEA0E: 0x1E66E0,
}
WRITES_AND_READS = {
    0x1E4A99: "D9 99 50 01 00 00",  # FSTP object+150
    0x1E4AA2: "D9 99 54 01 00 00",  # FSTP object+154
    0x1E6B70: "D9 96 50 01 00 00",  # FST object+150
    0x1E6B81: "D9 96 50 01 00 00",
    0x1E6C10: "D9 96 50 01 00 00",
    0x1E6BB2: "D9 96 D4 00 00 00",  # FST object+D4
    0x1E6BD4: "D9 96 D4 00 00 00",
    0x1E6C4D: "D9 96 D4 00 00 00",
    0x1E70EE: "D9 9E 88 00 00 00",  # FSTP object+88
    0x1E8B97: "D9 9E 90 00 00 00",  # FSTP object+90
    0x1E8C34: "D9 9E 90 00 00 00",
    0x1E70DB: "D9 5E 7C",           # FSTP object+7C
    0x1E70BD: "D9 86 50 01 00 00",  # FLD object+150
    0x1E70CF: "D9 86 D4 00 00 00",  # FLD object+D4
}


def check(path):
    raw = path.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    if digest != SHA256:
        raise ValueError("Unexpected original SHA256: " + digest)
    if raw[:2] != b"MZ":
        raise ValueError("Not a PE file")
    pe = struct.unpack_from("<I", raw, 0x3C)[0]
    if raw[pe:pe+4] != b"PE\x00\x00":
        raise ValueError("PE signature missing")
    coff = pe + 4
    machine, nsections = struct.unpack_from("<HH", raw, coff)
    optional_size = struct.unpack_from("<H", raw, coff+16)[0]
    optional = coff+20
    image_base = struct.unpack_from("<I", raw, optional+28)[0]
    magic = struct.unpack_from("<H", raw, optional)[0]
    if (machine, magic, image_base) != (0x14C, 0x10B, 0x400000):
        raise ValueError("Expected original PE32 i386 image")
    sections = []
    for i in range(nsections):
        item = optional + optional_size + 40*i
        virtual_size, rva, file_size, offset = struct.unpack_from(
            "<IIII", raw, item+8
        )
        sections.append((rva, file_size, offset))

    def read_rva(rva, count):
        for begin, size, offset in sections:
            if begin <= rva and rva + count <= begin + size:
                return raw[offset + rva - begin:offset + rva - begin + count]
        raise ValueError("Unmapped RVA: %08X" % rva)

    for site, dest in CALLS.items():
        code = read_rva(site, 5)
        if code[0] != 0xE8:
            raise ValueError("CALL opcode mismatch at %08X" % site)
        actual = site + 5 + struct.unpack("<i", code[1:])[0]
        if actual != dest:
            raise ValueError("CALL target mismatch at %08X" % site)
        print("PASS CALL %08X -> %08X" % (site, actual))
    for site, hexbytes in WRITES_AND_READS.items():
        expected = bytes.fromhex(hexbytes)
        if read_rva(site, len(expected)) != expected:
            raise ValueError("Field instruction mismatch at %08X" % site)
        print("PASS FIELD %08X" % site)
    print("PASS Stage61: %d calls, %d instruction signatures" %
          (len(CALLS), len(WRITES_AND_READS)))
    print("LIMIT: byte signatures do not prove executed COM-only paths")


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("aalib_exe", type=Path, help="Local/private original executable")
    args = p.parse_args()
    try:
        check(args.aalib_exe)
    except (OSError, ValueError, struct.error) as exc:
        p.exit(1, "FAIL %s\n" % exc)
