#!/usr/bin/env python3
"""Stage52 original AALib.exe + source/adapter static verifier, stdlib only.

Usage:
  py -3 Stage52_Analysis\tools\verify_stage52.py --aalib "D:\path\AALib.exe"
  py -3 Stage52_Analysis\tools\verify_stage52.py --aalib "D:\path\AALib.zip"

Validates the exact SHA-pinned original, not a running or relocated binary.
Passing is NOT proof of a successful MSVC build, hook install, or gameplay.
"""
import argparse
import hashlib
import struct
import sys
import zipfile
from pathlib import Path

EXPECTED_SHA = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
CHECKS = {
    0xAF10D: "E8 FE FD FF FF",
    0xAF112: "E8 79 FC FF FF",
    0xAF117: "6A 01",
    0xAEF10: "55 8B EC B8 54 14 00 00 E8 B3 C5 15 00",
    0xAED90: "55 8B EC 83 EC 08 53 56 33 F6 57 89 75 FC 8B FF",
    0xAF09A: "C3",
    0xAEE5E: "C3",
    0xAEE01: "D9 56 7C",
    0xAEE04: "D9 9E AC 00 00 00",
    0xAEE16: "D9 96 80 00 00 00",
    0xAEE1C: "D9 9E B0 00 00 00",
}
ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "Stage52_Overlay/src/Stage52PreGlobalCoordinate.inl"
ADAPTER = ROOT / "Stage52_Overlay/src/Stage38CallbackWindow.inl"


def checked(cond, message):
    if not cond:
        raise AssertionError(message)
    print("PASS", message)


def load_exe(path):
    if path.suffix.lower() == ".zip":
        with zipfile.ZipFile(path) as z:
            checked("AALib.exe" in z.namelist(), "ZIP contains exact AALib.exe name")
            return z.read("AALib.exe")
    return path.read_bytes()


def section_bytes(data, rva, count):
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    checked_pe = data[pe:pe + 4] == b"PE\x00\x00"
    if not checked_pe:
        raise AssertionError("Not a PE executable")
    number = struct.unpack_from("<H", data, pe + 6)[0]
    size_opt = struct.unpack_from("<H", data, pe + 20)[0]
    section_base = pe + 24 + size_opt
    for idx in range(number):
        o = section_base + idx * 40
        virtual_size, va, raw_size, raw_start = struct.unpack_from("<IIII", data, o + 8)
        if va <= rva and rva + count <= va + max(virtual_size, raw_size):
            file_offset = raw_start + rva - va
            checked(file_offset + count <= len(data), "RVA mapping inside the original file")
            return data[file_offset:file_offset + count]
    raise AssertionError(f"Unmapped RVA {rva:08X}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--aalib", type=Path, required=True, help="Original AALib.exe or AALib.zip")
    args = parser.parse_args()
    data = load_exe(args.aalib)
    checked(len(data) == 175556608, "Original AALib.exe size 175,556,608 bytes")
    checked(hashlib.sha256(data).hexdigest() == EXPECTED_SHA, "Exact original AALib.exe SHA256")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    checked(data[pe:pe + 4] == b"PE\x00\x00", "Valid PE header")
    checked(struct.unpack_from("<H", data, pe + 24)[0] == 0x10B, "PE32 x86 optional-header magic")
    checked(struct.unpack_from("<I", data, pe + 24 + 28)[0] == 0x400000, "Original PE ImageBase=0x400000")

    for rva, sig in CHECKS.items():
        expected = bytes.fromhex(sig)
        actual = section_bytes(data, rva, len(expected))
        checked(actual == expected, f"Original opcode signature {rva:06X}: {sig}")
    for caller, target in ((0xAF10D, 0xAEF10), (0xAF112, 0xAED90)):
        call = section_bytes(data, caller, 5)
        displacement = struct.unpack("<i", call[1:])[0]
        checked(caller + 5 + displacement == target, f"Original CALL {caller:06X} -> {target:06X}")

    src = SOURCE.read_text(encoding="utf-8")
    adapter = ADAPTER.read_text(encoding="utf-8")
    for needle in (
        "S52NativeAEF10Hook()", "S52NativeAED90Hook()",
        "push 0AF10Dh", "push 0AF112h",
        "const BYTE callA[]={0xE8,0xFE,0xFD,0xFF,0xFF};",
        "const BYTE callB[]={0xE8,0x79,0xFC,0xFF,0xFF};",
        "site+5u+rel==target", "g_s52Installed,2",
        "std::memcpy(siteA,patchA,5u)", "std::memcpy(siteB,patchB,5u)",
        "STAGE52_INSTALLED 2/2", "STAGE52_HEALTH",
        "g_s51Installed", "callTarget(siteA,nativeA)&&callTarget(siteB,nativeB)",
    ):
        checked(needle in src, f"Stage52 source guard: {needle}")
    for var in ("g_s52NativeA", "g_s52NativeB"):
        checked(src.count(f"call dword ptr [{var}]") == 1, f"Original native called once by {var} wrapper")
    checked(src.count("call S52Enter") == 2 and src.count("call S52Leave") == 2,
            "Each of two wrappers has one enter and one leave observer")
    checked(src.count("fxsave [edi]") == 4 and src.count("fxrstor [edi]") == 4,
            "Four FXSAVE/FXRSTOR pairs protect x87/SSE")
    checked(src.count("popfd") == 4 and src.count("popad") == 4,
            "Four GPR/EFLAGS restore sequences")
    checked(src.count("S52Snap(r.Object,r.Pre,r.PreMask)") == 1 and
            src.count("S52Snap(r.Object,r.Post,r.PostMask)") == 1,
            "Passive before/after snapshots of unchanged object")
    checked("#include \"Stage51GlobalCoordinate.inl\"\n#include \"Stage52PreGlobalCoordinate.inl\"" in adapter,
            "Adapter retains Stage51 and adds Stage52 include")
    checked("if(InstallStage51Hooks())InstallStage52Hooks();" in adapter,
            "Stage52 installation is chained after Stage51 success")
    checked("FlushStage51();\n    FlushStage52();" in adapter,
            "Stage52 logs flush via existing Stage38 worker")
    checked("#include \"Stage49LinkedMotion.inl\"" in adapter and
            "#include \"Stage50PreLinkedCoordinate.inl\"" in adapter,
            "Stage49 r2 and Stage50 intact in adapter")
    print("\nStage52 static checks PASS. Windows x86 Build.bat and runtime still PENDING.")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, AssertionError, struct.error) as err:
        print("FAIL", str(err), file=sys.stderr)
        raise SystemExit(1)
