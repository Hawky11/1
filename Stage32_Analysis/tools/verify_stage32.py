#!/usr/bin/env python3
"""SHA-pinned read-only Stage32 verification of the original AALib.exe (PE32 x86)."""
import argparse
import hashlib
import struct
from pathlib import Path

EXPECTED_SHA256 = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"


def pe_mapper(data):
    if data[:2] != b"MZ":
        raise ValueError("not MZ")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe:pe+4] != b"PE\0\0":
        raise ValueError("not PE")
    machine, nsec, _, _, _, optsize, _ = struct.unpack_from("<HHIIIHH", data, pe + 4)
    opt = pe + 24
    if machine != 0x14C or struct.unpack_from("<H", data, opt)[0] != 0x10B:
        raise ValueError("expected 32-bit x86 PE")
    if struct.unpack_from("<I", data, opt + 28)[0] != 0x400000:
        raise ValueError("unexpected preferred image base")
    sections = []
    for i in range(nsec):
        pos = opt + optsize + 40 * i
        virtual_size, rva, raw_size, raw_start = struct.unpack_from("<IIII", data, pos + 8)
        sections.append((rva, virtual_size, raw_size, raw_start))

    def read(rva, count):
        for base, vs, size, start in sections:
            if base <= rva and rva + count <= base + min(vs, size):
                return data[start + rva - base:start + rva - base + count]
        raise ValueError("unmapped RVA %X (%d bytes)" % (rva, count))
    return read


def verify(binary, source=None, ini=None):
    data = Path(binary).read_bytes()
    sha = hashlib.sha256(data).hexdigest()
    if sha != EXPECTED_SHA256:
        raise ValueError("wrong executable SHA-256: " + sha)
    read = pe_mapper(data)
    signatures = {
        0xF2DE0: "55 8B EC 8B 45 08",
        0xF2DE6: "80 78 0F 00",             # queue guard
        0xF2DF9: "C6 00 01",                # type1/2 active=1
        0xF2E03: "83 FA 1E 75 03 C6 00 01", # type0/payload1E special case
        0xF2E0B: "66 89 50 04 66 89 48 02",
        0xF2E20: "55 8B EC 8B 45 08",
        0xF2E2A: "66 83 FA FF",             # invalid current type
        0xF2E34: "66 83 F9 FF",             # invalid current payload
        0xF2E3A: "66 3B 50 08",             # compare history type
        0xF2E40: "66 3B 48 0A",             # compare history payload
        0xF2E50: "55 8B EC 8B 45 08",
        0xF2E6A: "66 89 48 0A",             # store history payload
        0xF2E72: "66 89 50 08",             # store history type
        0xF2E7D: "66 89 50 04",             # invalidate current payload
        0xF2E83: "66 89 48 02",             # invalidate current type
        0xEE480: "55 8B EC 83 EC 0C",
        0xEE589: "C6 06 01",                # activate pending changed tuple
        0xEE5B6: "80 BB 2C 3A 00 00 00",   # active before EE200 dispatch
        0xEE609: "80 BE 2C 3A 00 00 00",   # caller's active check
        0xE6C66: "81 8E 84 3B 00 00 00 00 01 00", # set bit16
        0xE9E66: "81 A6 84 3B 00 00 FF FF FE FF", # clear bit16
    }
    for rva, signature in signatures.items():
        want = bytes.fromhex(signature)
        have = read(rva, len(want))
        if have != want:
            raise ValueError("signature mismatch at RVA %06X: %s != %s" % (rva, have.hex(), want.hex()))
    calls = {
        0x156E47: 0xF2DE0,
        0xEE4AB: 0xF2E20,
        0xEE57D: 0xF2E50,
        0xEE5C6: 0xEE200,
        0xEE604: 0xEE480,
    }
    for rva, target in calls.items():
        op = read(rva, 5)
        if op[0] != 0xE8 or rva + 5 + struct.unpack_from("<i", op, 1)[0] != target:
            raise ValueError("wrong CALL at RVA %06X (expected %06X)" % (rva, target))
    if source is not None:
        s = Path(source).read_text(encoding="utf-8")
        for token in ("STAGE32_TYPE0_DISPATCH_BUILD=2026-11-09-r1",
                      "g_stage32Type0DispatchDetailEnabled", "eventType == 0 && payload == 0",
                      "original(player, eventType, payload);",
                      "STAGE31_ALTERNATE_PATH_BUILD=2026-11-09-r1"):
            if token not in s:
                raise ValueError("missing overlay token: " + token)
    if ini is not None:
        s = Path(ini).read_text(encoding="utf-8")
        for setting in ("EnableStage32Type0DispatchDetail=1",
                        "EnableCpuEventDispatchTrace=1",
                        "EnableStage31AlternatePathTrace=1"):
            if setting not in s:
                raise ValueError("missing INI setting: " + setting)
    print("PASS: original AALib.exe SHA, PE32 base, %d signatures, %d CALL targets" % (len(signatures), len(calls)))
    print("Stage32 hypothesis: changed type0/payload0 can be activated later at EE589, then dispatched at EE5C6")
    print("This is static control flow; individual Stage31 -> dispatch causality still requires runtime QPC evidence.")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--binary", required=True)
    ap.add_argument("--source")
    ap.add_argument("--ini")
    args = ap.parse_args()
    verify(args.binary, args.source, args.ini)
