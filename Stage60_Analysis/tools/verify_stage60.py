#!/usr/bin/env python3
"""Stage60 read-only original AALib.exe x86 PE call/store/dataflow verifier."""
import argparse
import hashlib
import struct
from pathlib import Path

EXPECTED_SHA256 = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"

# All values are RVAs. The original PE ImageBase is 0x00400000.
CALLS = (
    (0x0A119B, 0x0AE6E0, "coordinator -> early object update"),
    (0x0A11A0, 0x0ACDF0, "coordinator -> pair adjustment"),
    (0x0AE70C, 0x1E66E0, "object update -> movement integrator"),
    (0x0ACE3D, 0x0ACB80, "pair adjustment -> scalar helper, first pass"),
    (0x0ACE56, 0x0ACB80, "pair adjustment -> scalar helper, second pass"),
    (0x0ACBF2, 0x1E43A0, "scalar helper -> bound evaluation, first term"),
    (0x0ACC02, 0x1E43A0, "scalar helper -> bound evaluation, second term"),
    (0x0ACFD6, 0x15CEC0, "pair adjustment -> secondary solver"),
    (0x15CF5B, 0x12D520, "secondary solver -> record processor"),
    (0x12D58A, 0x126580, "record processor -> object-pair resolver"),
)
STORES = (
    (0x1E70DB, "D9 5E 7C", 0x1E70DE, "FSTP [ESI+7C]"),
    (0x0ACC57, "D9 5B 7C", 0x0ACC5A, "FSTP [EBX+7C]"),
    (0x126A9A, "D9 5E 7C", 0x126A9D, "FSTP [ESI+7C]"),
    (0x126B49, "D9 5E 7C", 0x126B4C, "FSTP [ESI+7C]"),
    (0x0ACF2A, "D9 58 7C", 0x0ACF2D, "FSTP [EAX+7C]"),
)
DATAFLOW = (
    (0x1E70B1, "D9 46 7C", "load early current X"),
    (0x1E70BD, "D9 86 50 01 00 00", "load object+150"),
    (0x1E70CF, "D9 86 D4 00 00 00", "load object+D4"),
    (0x0ACC51, "D9 45 08", "load local scalar"),
    (0x0ACC54, "D8 43 7C", "add object X"),
    (0x126A94, "D9 46 7C", "load pair object X, path A"),
    (0x126A97, "D8 45 FC", "add local [EBP-4]"),
    (0x126B43, "D9 46 7C", "load pair object X, path B"),
    (0x126B46, "D8 45 F4", "add local [EBP-C]"),
)


def pe_reader(raw):
    if raw[:2] != b"MZ":
        raise ValueError("Missing MZ")
    pe = struct.unpack_from("<I", raw, 0x3C)[0]
    if raw[pe:pe + 4] != b"PE\0\0":
        raise ValueError("Missing PE")
    coff = pe + 4
    machine, sections, _, _, _, opt_size, _ = struct.unpack_from("<HHIIIHH", raw, coff)
    opt = coff + 20
    magic = struct.unpack_from("<H", raw, opt)[0]
    image_base = struct.unpack_from("<I", raw, opt + 28)[0]
    if (machine, magic, image_base) != (0x14C, 0x10B, 0x400000):
        raise ValueError("Expected original PE32 x86, preferred ImageBase 0x400000")
    table = []
    for i in range(sections):
        offset = opt + opt_size + i * 40
        name, virt_size, rva, raw_size, raw_ptr = struct.unpack_from("<8sIIII", raw, offset)
        table.append((name.split(b"\0")[0], rva, raw_size, raw_ptr))

    def at(rva, count):
        for name, begin, size, pos in table:
            displacement = rva - begin
            if displacement >= 0 and displacement + count <= size:
                return raw[pos + displacement:pos + displacement + count]
        raise ValueError("Unmapped raw RVA %08X" % rva)
    return at


def verify(path):
    raw = path.read_bytes()
    sha = hashlib.sha256(raw).hexdigest()
    if sha != EXPECTED_SHA256:
        raise ValueError("SHA256 mismatch: " + sha)
    at = pe_reader(raw)
    print("PASS original PE32 x86 SHA256 " + sha)
    for site, expected, purpose in CALLS:
        code = at(site, 5)
        if code[0] != 0xE8:
            raise ValueError("Not E8 CALL at %08X" % site)
        target = site + 5 + struct.unpack("<i", code[1:])[0]
        if target != expected:
            raise ValueError("Incorrect call %08X -> %08X expected %08X" % (site, target, expected))
        print("PASS CALL %08X -> %08X %s" % (site, target, purpose))
    for site, hex_bytes, after, meaning in STORES:
        wanted = bytes.fromhex(hex_bytes)
        if at(site, len(wanted)) != wanted or site + len(wanted) != after:
            raise ValueError("Store mismatch at %08X" % site)
        print("PASS STORE %08X EIP_after=%08X %s" % (site, after, meaning))
    for site, hex_bytes, purpose in DATAFLOW:
        wanted = bytes.fromhex(hex_bytes)
        if at(site, len(wanted)) != wanted:
            raise ValueError("Dataflow mismatch at %08X" % site)
        print("PASS READ %08X %s" % (site, purpose))
    print("PASS Stage60: 10 call edges, 5 stores, 9 source terms")
    print("NOTE: static matching is not gameplay/AI causality evidence")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("aalib_exe", type=Path, help="private original AALib.exe; keep local")
    args = parser.parse_args()
    try:
        verify(args.aalib_exe)
    except (ValueError, OSError, struct.error) as error:
        parser.exit(1, "FAIL %s\n" % error)
