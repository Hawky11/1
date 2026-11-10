#!/usr/bin/env python3
"""Stage56 read-only static verifier: SHA-pinned AALib.exe + exact overlay delta.

No third-party packages. This does not compile MSVC code or exercise Windows runtime.
"""
import argparse
import hashlib
from pathlib import Path
import struct

EXPECTED_SHA256 = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
EXPECTED_SIZE = 175_556_608
SIGNATURES = (
    (0xACF23, bytes.fromhex("D9 44 8D A4 D8 40 7C D9 58 7C 8B 97 70 3B 00 00")),
    (0xACF99, bytes.fromhex("D9 84 8D 54 FF FF FF D8 40 7C D9 58 7C 8B 5D F4")),
)
SITES = ((0xACF27, 0xACF2D), (0xACFA0, 0xACFA6))
PAIR = bytes.fromhex("D8 40 7C D9 58 7C")


def u16(buf, off):
    return struct.unpack_from("<H", buf, off)[0]


def u32(buf, off):
    return struct.unpack_from("<I", buf, off)[0]


def check(value, description):
    if not value:
        raise AssertionError(description)
    print("[PASS]", description)


def pe_sections(raw):
    check(raw[:2] == b"MZ", "DOS header")
    pe = u32(raw, 0x3C)
    check(raw[pe:pe + 4] == b"PE\0\0", "PE signature")
    check(u16(raw, pe + 4) == 0x014C, "i386 Machine")
    check(u16(raw, pe + 24) == 0x010B, "PE32 OptionalHeader")
    check(u32(raw, pe + 24 + 28) == 0x00400000, "preferred ImageBase 00400000")
    n = u16(raw, pe + 6)
    size_opt = u16(raw, pe + 20)
    section_offset = pe + 24 + size_opt
    sections = []
    for i in range(n):
        at = section_offset + 40 * i
        name = raw[at:at + 8].rstrip(b"\0").decode("ascii")
        virtual_size, virtual_address, raw_size, raw_ptr = struct.unpack_from("<IIII", raw, at + 8)
        sections.append((name, virtual_address, max(virtual_size, raw_size), raw_ptr, raw_size))
    reloc_rva, reloc_size = struct.unpack_from("<II", raw, pe + 24 + 96 + 5 * 8)
    return sections, reloc_rva, reloc_size


def offset_of(rva, sections):
    for _name, base, vsize, fileoff, filesize in sections:
        if base <= rva < base + vsize and rva - base < filesize:
            return fileoff + rva - base
    raise AssertionError("RVA cannot be mapped: %08X" % rva)


def overlaps(a, an, b, bn):
    return a < b + bn and b < a + an


def verify_relocations(raw, sections, reloc_rva, reloc_size):
    check(reloc_rva != 0 and reloc_size != 0, "PE base relocation directory present")
    pos = offset_of(reloc_rva, sections)
    end = pos + reloc_size
    signature_regions = [(rva, len(sig)) for rva, sig in SIGNATURES]
    relocations = 0
    conflicts = []
    while pos + 8 <= end:
        page, size = struct.unpack_from("<II", raw, pos)
        if not page or not size:
            break
        if size < 8 or size % 2 or pos + size > end:
            raise AssertionError("Invalid PE relocation block")
        for at in range(pos + 8, pos + size, 2):
            entry = u16(raw, at)
            kind, sub = entry >> 12, entry & 0x0FFF
            if kind == 0:
                continue
            relocations += 1
            where = page + sub
            width = 4 if kind == 3 else (2 if kind in (1, 2) else 1)
            if any(overlaps(where, width, start, length) for start, length in signature_regions):
                conflicts.append((where, kind))
        pos += size
    check(not conflicts, "both 16-byte signatures avoid PE relocation overlaps")
    print("Relocation entries examined:", relocations)


def verify_adapter(root):
    original = (root / "Stage55_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
    actual = (root / "Stage56_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
    edits = (
        ('#include "Stage55EarlierMotionCoordinate.inl"',
         '#include "Stage55EarlierMotionCoordinate.inl"\n#include "Stage56ACDF0Writer.inl"'),
        ('if(InstallStage54Hooks())InstallStage55Hooks();',
         'if(InstallStage54Hooks()){\n'
         '                                                        if(InstallStage55Hooks())InstallStage56Hooks();\n'
         '                                                        else S56Log("ERROR: Stage56 requires installed Stage55; no patches.");\n'
         '                                                    }'),
        ('    FlushStage55();', '    FlushStage55();\n    FlushStage56();'),
    )
    for before, after in edits:
        check(original.count(before) == 1, "Stage55 adapter anchor unique")
        original = original.replace(before, after)
    check(actual == original, "Stage56 adapter differs from Stage55 by only three intended edits")
    source = (root / "Stage56_Overlay/src/Stage56ACDF0Writer.inl").read_text(encoding="utf-8")
    for name in ("S56WriterAHook", "S56WriterBHook", "InstallStage56Hooks",
                 "S56Enter", "S56Leave", "FlushStage56"):
        check(name in source, "source contains " + name)
    for marker in ("fadd dword ptr [eax+7Ch]", "fstp dword ptr [eax+7Ch]",
                   "patches[i][0]=0xE9u", "patches[i][5]=0x90u",
                   "g_s56ResumeA=g_gameBase+0xACF2Du",
                   "g_s56ResumeB=g_gameBase+0xACFA6u"):
        check(marker in source, "source contains " + marker)
    check(source.count("fadd dword ptr [eax+7Ch]") == 2 and
          source.count("fstp dword ptr [eax+7Ch]") == 2,
          "two wrapper bodies repeat original FADD+FSTP")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--aalib", type=Path, required=True,
                        help="Private SHA-pinned AALib.exe (do NOT commit)")
    parser.add_argument("--repo-root", type=Path,
                        default=Path(__file__).resolve().parents[2])
    args = parser.parse_args()
    raw = args.aalib.read_bytes()
    check(len(raw) == EXPECTED_SIZE, "original EXE byte length")
    check(hashlib.sha256(raw).hexdigest() == EXPECTED_SHA256, "original EXE SHA256")
    sections, reloc_rva, reloc_size = pe_sections(raw)
    for rva, signature in SIGNATURES:
        check(raw[offset_of(rva, sections):offset_of(rva, sections) + len(signature)]
              == signature, "16-byte complete-instruction signature at RVA %08X" % rva)
    for site, resume in SITES:
        check(resume - site == 6, "six-byte patch / resume at RVA %08X" % site)
        check(raw[offset_of(site, sections):offset_of(site, sections) + 6]
              == PAIR, "unmodified FADD+FSTP pair at RVA %08X" % site)
    verify_relocations(raw, sections, reloc_rva, reloc_size)
    verify_adapter(args.repo_root)
    print("STAGE56 STATIC CHECKS PASS. MSVC build and runtime remain PENDING.")


if __name__ == "__main__":
    main()
