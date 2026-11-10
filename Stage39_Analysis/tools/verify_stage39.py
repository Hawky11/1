#!/usr/bin/env python3
"""Stage39 original PE static verifier. Read-only, Python stdlib only."""
import argparse
import hashlib
import struct
import sys
import zipfile
from pathlib import Path

EXPECTED_SHA = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
EXPECTED_SIZE = 175556608
SIGS = {
    0x12EB10: "55 8B EC 53 56 8B 75 08",
    0x123AF0: "55 8B EC 56 8B 75 08",
    0x1DA490: "55 8B EC 51 56 57 8B 7D 08",
    0x12EB74: "FF 24 85 5C EE 52 00",
    0x1BD6EF: "8D B7 E4 3E 00 00",
    0x1BD702: "8D B7 AC 40 00 00",
    0x1B8068: "89 41 28",
    0x1DA453: "89 90 80 02 00 00",
    0xE5BF9:  "8B 45 08 81 89 7C 3B 00 00 00 00 00 01",
    0xEE7A9:  "8D 4D C8 56 51 E8 4D 84 06 00 83 C4 08 85 FF 74",
}
CALLS = {
    0x123B24: 0x1BD6E0,    # handler 123AF0 -> shared action routine
    0x1DA592: 0x1BD6E0,    # handler 1DA490 -> same routine
    0x131CC5: 0xE5BF0,     # 12EB10 -> 131CA0 -> state ID writer
    0x131CDA: 0xF7E70,     # 131CA0 -> callback pointer setup
    0x131D1C: 0xE5BF0,     # alternate table branch
    0x131D31: 0xF7E70,
    0x1BD7B0: 0x1B8050,    # primary path through action state
    0x1BD789: 0x1B80F0,    # alternate path
}

def load(path):
    if path.suffix.lower() == ".zip":
        with zipfile.ZipFile(path) as z:
            matches = [n for n in z.namelist() if n.replace("\\", "/").endswith("AALib.exe")]
            if len(matches) != 1:
                raise ValueError("Expected exactly one AALib.exe entry in ZIP")
            return z.read(matches[0])
    return path.read_bytes()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("binary", type=Path, help="Original AALib.exe or Library AALib.zip")
    ap.add_argument("--stage38-inl", type=Path)
    ap.add_argument("--stage39-inl", type=Path)
    args = ap.parse_args()
    b = load(args.binary)
    assert len(b) == EXPECTED_SIZE, f"EXE size {len(b)}"
    digest = hashlib.sha256(b).hexdigest()
    assert digest == EXPECTED_SHA, f"SHA-256 {digest}"
    peoff = struct.unpack_from("<I", b, 0x3C)[0]
    assert b[peoff:peoff+4] == b"PE\\0\\0"
    assert struct.unpack_from("<H", b, peoff+4)[0] == 0x14C, "PE machine is not i386"
    assert struct.unpack_from("<H", b, peoff+24)[0] == 0x10B, "Not PE32"
    assert struct.unpack_from("<I", b, peoff+24+28)[0] == 0x400000, "Unexpected preferred base"
    nsec, opt_size = struct.unpack_from("<H", b, peoff+6)[0], struct.unpack_from("<H", b, peoff+20)[0]
    sections = []
    for i in range(nsec):
        off = peoff + 24 + opt_size + 40*i
        name = b[off:off+8].rstrip(b"\\0").decode("ascii")
        vsize, rva, rawsize, raw = struct.unpack_from("<IIII", b, off+8)
        sections.append((name, rva, max(vsize, rawsize), raw))
    def at(rva, size):
        for name, start, span, raw in sections:
            if start <= rva < start+span:
                off = raw+rva-start
                return b[off:off+size]
        raise ValueError(f"Unmapped RVA {rva:08X}")
    for addr, sig in SIGS.items():
        expected = bytes.fromhex(sig)
        actual = at(addr, len(expected))
        assert actual == expected, f"opcode mismatch RVA {addr:08X}: {actual.hex()}"
    for site, target in CALLS.items():
        data = at(site, 5)
        assert data[0] == 0xE8, f"not E8 CALL at {site:08X}"
        actual = site + 5 + struct.unpack_from("<i", data, 1)[0]
        assert actual == target, f"CALL at {site:08X} points to {actual:08X}, wanted {target:08X}"
    if args.stage38_inl:
        source = args.stage38_inl.read_text(encoding="utf-8")
        for token in ('#include "Stage39ActionOutput.inl"', "Stage39TargetCallback",
                      "Stage39Snapshot", "Stage39LogWindow",
                      "STAGE39_ACTION_OUTPUT_BUILD=2026-11-10-r1",
                      "__declspec(naked) void Stage38PrepareHook()",
                      "__declspec(naked) void Stage38FinishHook()"):
            assert token in source, f"Stage38 inl missing {token}"
    if args.stage39_inl:
        source = args.stage39_inl.read_text(encoding="utf-8")
        for token in ("constexpr DWORD kStage39Fields = 30u;", "0x0012EB10u",
                      "0x00123AF0u", "0x001DA490u",
                      "0x3F4Cu", "0x3EE4u", "0x40ACu", "0x43B4u",
                      "ArcanaAI_Stage39ActionOutputTrace.log"):
            assert token in source, f"Stage39 inl missing {token}"
    print(f"PASS: SHA256 {digest}; PE32 i386; {len(SIGS)} exact byte ranges; "
          f"{len(CALLS)} exact E8 targets; optional sources verified")
    return 0

if __name__ == "__main__":
    try:
        sys.exit(main())
    except (AssertionError, ValueError, OSError, zipfile.BadZipFile) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        sys.exit(1)
