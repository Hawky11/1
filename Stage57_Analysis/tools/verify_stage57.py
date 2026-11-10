#!/usr/bin/env python3
"""Stage57 source and original PE x86 verification (stdlib only; read-only).

Does NOT compile or prove runtime ABI equivalence of the new wrappers.
"""
import argparse
import hashlib
import struct
from pathlib import Path

EXPECTED_SHA = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
EXPECTED_BYTES = 175_556_608
# RVA, call bytes, native RVA, entire native prologue/signature.
SITES = (
    (0xACE11, bytes.fromhex("E8 2A 5E FF FF"), 0xA2C40,
     bytes.fromhex("55 8B EC B8 60 14 00 00 E8 83 88 16 00")),
    (0xACFD6, bytes.fromhex("E8 E5 FE 0A 00"), 0x15CEC0,
     bytes.fromhex("55 8B EC 51 56 57 E8 A5 7D 08 00")),
    (0xACFDB, bytes.fromhex("E8 C0 E8 0A 00"), 0x15B8A0,
     bytes.fromhex("55 8B EC 81 EC C8 03 00 00 53 56 57")),
)


def require(pred, label):
    if not pred:
        raise AssertionError(label)
    print("PASS", label)


def u16(b, o):
    return struct.unpack_from("<H", b, o)[0]


def u32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def sections_and_reloc(raw):
    require(raw[:2] == b"MZ", "DOS signature")
    pe = u32(raw, 0x3C)
    require(raw[pe:pe + 4] == b"PE\0\0", "PE signature")
    require(u16(raw, pe + 4) == 0x14C, "x86 Machine")
    opt = pe + 24
    require(u16(raw, opt) == 0x10B, "PE32")
    require(u32(raw, opt + 28) == 0x400000, "original ImageBase")
    nsec = u16(raw, pe + 6)
    sect_off = opt + u16(raw, pe + 20)
    sec = []
    for i in range(nsec):
        p = sect_off + i * 40
        virt_size, rva, raw_size, raw_off = struct.unpack_from("<IIII", raw, p + 8)
        sec.append((rva, max(virt_size, raw_size), raw_off, raw_size))
    rel_rva, rel_size = struct.unpack_from("<II", raw, opt + 96 + 5 * 8)
    require(rel_rva > 0 and rel_size > 0, "base relocation directory")
    return sec, rel_rva, rel_size


def rva_offset(r, sec):
    for base, length, offset, size in sec:
        if base <= r < base + length and r - base < size:
            return offset + r - base
    raise AssertionError("unmapped file RVA %08X" % r)


def verify_binary(raw):
    require(len(raw) == EXPECTED_BYTES, "exact binary byte length")
    require(hashlib.sha256(raw).hexdigest() == EXPECTED_SHA, "SHA-pinned original EXE")
    sec, rel_rva, rel_size = sections_and_reloc(raw)
    watched = []
    for site, insn, native, signature in SITES:
        actual = raw[rva_offset(site, sec):rva_offset(site, sec) + 5]
        require(actual == insn, "original CALL at %08X" % site)
        rel = struct.unpack_from("<i", insn, 1)[0]
        require(site + 5 + rel == native,
                "decoded original CALL target at %08X" % site)
        require(raw[rva_offset(native, sec):rva_offset(native, sec) + len(signature)] ==
                signature, "original native prologue at %08X" % native)
        watched.extend(((site, 5), (native, len(signature))))
    pos = rva_offset(rel_rva, sec)
    end = pos + rel_size
    count = 0
    overlap = []
    while pos + 8 <= end:
        page, block = struct.unpack_from("<II", raw, pos)
        if page == 0 or block == 0:
            break
        if block < 8 or block % 2 or pos + block > end:
            raise AssertionError("corrupt relocation block")
        for p in range(pos + 8, pos + block, 2):
            entry = u16(raw, p)
            typ, off = entry >> 12, entry & 0xfff
            if not typ:
                continue
            count += 1
            rva = page + off
            width = 4 if typ == 3 else 2 if typ in (1, 2) else 1
            for start, length in watched:
                if rva < start + length and start < rva + width:
                    overlap.append((rva, start, typ))
        pos += block
    require(not overlap, "no PE relocation overlapping any Stage57 CALL or native signature")
    print("PE relocation entries examined:", count)


def verify_source(root):
    orig = (root / "Stage56_Overlay/src/Stage38CallbackWindow.inl").read_text(
        encoding="utf-8")
    result = (root / "Stage57_Overlay/src/Stage38CallbackWindow.inl").read_text(
        encoding="utf-8")
    edits = (
        ('#include "Stage56ACDF0Writer.inl"',
         '#include "Stage56ACDF0Writer.inl"\n'
         '#include "Stage57ACDF0HelperBoundary.inl"'),
        ('if(InstallStage55Hooks())InstallStage56Hooks();',
         'if(InstallStage55Hooks()){\n'
         '                                                            if(InstallStage56Hooks())InstallStage57Hooks();\n'
         '                                                            else S57Log("ERROR: Stage57 requires installed Stage56; no patches.");\n'
         '                                                        }'),
        ('    FlushStage56();', '    FlushStage56();\n    FlushStage57();'),
    )
    for a, b in edits:
        require(orig.count(a) == 1, "unique adapter anchor " + a[:45])
        orig = orig.replace(a, b)
    require(orig == result, "only three narrow Stage56→57 adapter edits")
    text = (root / "Stage57_Overlay/src/Stage57ACDF0HelperBoundary.inl").read_text(
        encoding="utf-8")
    for symbol in ("S57A2C40Hook", "S57CEC0Hook", "S57B8A0Hook",
                   "InstallStage57Hooks", "FlushStage57", "S57Enter", "S57Leave",
                   "STAGE57_HELPER_WINDOW", "STAGE57_INSTALLED 3/3",
                   "g_s56Installed", "g_s55Installed"):
        require(symbol in text, "source has " + symbol)
    for target in ("0xACE11u", "0xACFD6u", "0xACFDBu", "0xA2C40u",
                   "0x15CEC0u", "0x15B8A0u"):
        require(target in text, "source checks " + target)
    require(text.count("call dword ptr [g_s57Native") == 3,
            "exactly three original native calls in wrapper bodies")
    require(text.count("__declspec(naked)") == 3, "three naked wrapper bodies")


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--aalib", type=Path, required=True)
    p.add_argument("--repo-root", type=Path,
                   default=Path(__file__).resolve().parents[2])
    args = p.parse_args()
    verify_binary(args.aalib.read_bytes())
    verify_source(args.repo_root)
    print("STAGE57 STATIC CHECKS PASS; WINDOWS BUILD AND GAME RUNTIME PENDING.")


if __name__ == "__main__":
    main()
