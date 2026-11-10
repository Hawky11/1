#!/usr/bin/env python3
"""Stage59 read-only source/instruction verifier. NOT a Windows/runtime test."""
import argparse
import hashlib
import struct
import zipfile
from pathlib import Path

EXPECTED_SHA = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
EXPECTED_SIZE = 175_556_608

def require(ok, label):
    if not ok:
        raise AssertionError(label)
    print("PASS:", label)

def original_bytes(path):
    if path.suffix.lower() == ".zip":
        with zipfile.ZipFile(path) as archive:
            return archive.read("AALib.exe")
    return path.read_bytes()

def pe_sections(b):
    require(b[:2] == b"MZ", "MZ image")
    pe, = struct.unpack_from("<I", b, 0x3c)
    require(b[pe:pe + 4] == b"PE\x00\x00", "PE signature")
    machine, nsec = struct.unpack_from("<HH", b, pe + 4)
    size_opt, = struct.unpack_from("<H", b, pe + 20)
    magic, = struct.unpack_from("<H", b, pe + 24)
    require(machine == 0x14c and magic == 0x10b, "PE32 x86")
    result = []
    for i in range(nsec):
        start = pe + 24 + size_opt + i * 40
        virtual_size, rva, raw_size, fileoff = struct.unpack_from("<IIII", b, start + 8)
        result.append((rva, virtual_size, raw_size, fileoff))
    return result

def raw_at(b, sections, rva, size):
    for va, vsize, nraw, start in sections:
        if va <= rva and rva + size <= va + nraw:
            return b[start + rva - va : start + rva - va + size]
    raise AssertionError(f"No raw mapping for RVA {rva:X}")

def verify(binary_path, root):
    b = original_bytes(binary_path)
    require(len(b) == EXPECTED_SIZE, "Original AALib.exe file size")
    require(hashlib.sha256(b).hexdigest() == EXPECTED_SHA, "Original AALib.exe SHA-256")
    sections = pe_sections(b)
    for rva, signature in [
        (0xACF0F, "E8 8C B7 00 00 83 C4 08 84 C0 75 12"),
        (0xACF2A, "D9 58 7C"),
        (0xACFA3, "D9 58 7C"),
    ]:
        expected = bytes.fromhex(signature)
        require(raw_at(b, sections, rva, len(expected)) == expected,
                f"Original instruction signature RVA 0x{rva:X}")
    # Branch at ACF19 skips the ACF2A FSTP when AL != 0.
    require(raw_at(b, sections, 0xACF17, 4) == bytes.fromhex("84 C0 75 12"),
            "Native TEST AL,AL / JNE skips to ACF2D")
    for i in range(2):
        rw = (0x00DD0005 >> (16 + 4*i)) & 3
        length = (0x00DD0005 >> (18 + 4*i)) & 3
        enabled = (0x00DD0005 >> (2*i)) & 1
        require((rw, length, enabled) == (1, 3, 1),
                f"DR{i} write-only, 4-byte, local-enabled")
    prior = (root / "Stage58_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
    new = (root / "Stage59_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
    edits = [
        ('#include "Stage58ACDF0ConditionalCalls.inl"',
         '#include "Stage58ACDF0ConditionalCalls.inl"\n#include "Stage59XWriterWatch.inl"'),
        ('if(InstallStage57Hooks())InstallStage58Hooks();',
         'if(InstallStage57Hooks()){\n'
         '                                                                    if(InstallStage58Hooks())StartStage59Watch();\n'
         '                                                                    else S59Log("ERROR: Stage59 requires successful Stage58 installation; no worker.");\n'
         '                                                                }'),
    ]
    expected = prior
    for before, after in edits:
        require(expected.count(before) == 1, f"Unique Stage58 adapter anchor: {before[:44]}")
        expected = expected.replace(before, after)
    require(new == expected, "Stage59 adapter has exactly two intended edits")
    src = (root / "Stage59_Overlay/src/Stage59XWriterWatch.inl").read_text(encoding="utf-8")
    tokens = [
        "STAGE59_X_WRITER_WATCH_BUILD", "STAGE59_WATCH_ARMED", "STAGE59_THREAD_INSTALLED",
        "STAGE59_X_WRITE", "STAGE59_HEALTH", "S59ExceptionHandler",
        "GetPrivateProfileIntW", "EnableControlFieldTrace", "EnablePlayerBit17WriteTrace",
        "EnableStage59XWriterWatch", "CONTEXT_DEBUG_REGISTERS", "CreateToolhelp32Snapshot",
        "EXCEPTION_SINGLE_STEP", "EXCEPTION_CONTINUE_SEARCH", "g_s59Addresses[0]",
        "g_s59Addresses[1]", "obj[0]+0x7Cu", "obj[1]+0x7Cu",
        "c.Dr0=0;c.Dr1=0;", "c.Dr7&=~kS59DrMask",
        "oldApprox=1", "RVA_after", "dropped=", "ownershipConflicts=",
    ]
    for token in tokens:
        require(token in src, "Stage59 source token: " + token)
    require("WriteProcessMemory" not in src, "No explicit game-memory write API")
    print("STATIC CHECKS PASSED. Windows MSVC compile, VEH safety, coverage and gameplay runtime PENDING.")

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--aalib", type=Path, required=True, help="Private AALib.exe or AALib.zip")
    p.add_argument("--repo-root", type=Path, default=Path("."))
    a = p.parse_args()
    verify(a.aalib, a.repo_root)
