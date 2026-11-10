#!/usr/bin/env python3
"""Stage47 read-only static/native-signature + overlay integration verification.

Requires only Python stdlib. This is NOT a Windows x86 build or runtime test;
does NOT execute native wrappers or validate unseen indirect callsites.
"""
from __future__ import annotations
import argparse
import hashlib
from pathlib import Path
import struct

EXPECTED_SIZE = 175556608
EXPECTED_SHA256 = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
SITES = {
    0xAE70C: "e8cf7f1300",  # root E2E02C8 ordinary loop
    0xAE79F: "e83c7f1300",  # other object list
    0xAE7BD: "e81e7f1300",  # other object list
    0xAE849: "e8927e1300",  # other object list
    0xAE93E: "e89d7d1300",  # root E2E02C8 alternate loop
    0xAEA0E: "e8cd7c1300",  # other object list
}
HOOKED = (0xAE70C, 0xAE93E)
CONSUMERS = {
    0x1E66E0: "558bec83ec24568bf1",  # native thiscall O, stack argument
    0x1E67F1: "c20400",              # early RET 4
    0x1E78D4: "c20400",              # normal RET 4
    0x1E7089: "d98688000000",        # FLD O+88
    0x1E70DB: "d95e7c",              # FSTP O+7C
    0x1E70DE: "d98690000000",        # FLD O+90
    0x1E70E8: "d88688000000",        # FADD O+88
    0x1E70EE: "d99e88000000",        # FSTP O+88
    0x1E70F4: "d9868c000000",        # FLD O+8C
    0x1E7114: "d88680000000",        # FADD O+80
    0x1E711A: "d99e80000000",        # FSTP O+80
    0x1E7349: "d98694000000",        # FLD O+94
    0x1E7353: "d8868c000000",        # FADD O+8C
    0x1E7359: "d99e8c000000",        # FSTP O+8C
}

def pe_reader(data: bytes):
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    assert data[:2] == b"MZ" and data[pe:pe+4] == b"PE\\0\\0"
    nsec = struct.unpack_from("<H", data, pe+6)[0]
    opt = pe+24
    assert struct.unpack_from("<H", data, opt)[0] == 0x10B
    assert struct.unpack_from("<I", data, opt+28)[0] == 0x400000
    start = opt + struct.unpack_from("<H", data, pe+20)[0]
    sections = []
    for i in range(nsec):
        at = start+i*40
        name, vsize, va, rawsize, rawptr = struct.unpack_from("<8sIIII", data, at)
        sections.append((name.rstrip(b"\\0"), va, vsize, rawsize, rawptr))
    def read(rva: int, size: int) -> bytes:
        for _, va, _, rawsize, rawptr in sections:
            if va <= rva and rva+size <= va+rawsize:
                return data[rawptr+rva-va:rawptr+rva-va+size]
        raise AssertionError(f"missing RVA 0x{rva:06X}, size={size}")
    return read

def check_binary(exe: Path) -> None:
    data = exe.read_bytes()
    assert len(data) == EXPECTED_SIZE, "original EXE length differs"
    assert hashlib.sha256(data).hexdigest() == EXPECTED_SHA256, "original SHA mismatch"
    read = pe_reader(data)
    for rva, expected in SITES.items():
        original = bytes.fromhex(expected)
        assert read(rva,5) == original, f"CALL RVA {rva:06X} bytes changed"
        assert read(rva-4,4) == bytes.fromhex("6a008bce"), "PUSH 0 / MOV ECX,ESI"
        signed_disp = struct.unpack("<i", original[1:5])[0]
        assert rva+5+signed_disp == 0x1E66E0
    # Both selected paths iterate the same object-list root as the P1/P2
    # Stage43 AD75C path (E2E02C8). Other four calls are intentionally unhooked.
    for rva in (0xAE6ED,0xAE886):
        assert read(rva,6) == bytes.fromhex("8b35c8022e0e")
    for rva, sig in CONSUMERS.items():
        value = bytes.fromhex(sig)
        assert read(rva,len(value)) == value, f"consumer RVA {rva:06X} mismatch"
    assert HOOKED == (0xAE70C,0xAE93E)

def check_source(root: Path) -> None:
    s47 = (root/"Stage47_Overlay/src/Stage47MotionConsumer.inl").read_text(encoding="utf-8")
    adapter = (root/"Stage47_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
    baseline = (root/"Stage45_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
    stage44 = (root/"Stage46_Overlay/src/Stage44ActionDefinition.inl").read_text(encoding="utf-8")
    stage45 = (root/"Stage46_Overlay/src/Stage45DefinitionResolver.inl").read_text(encoding="utf-8")
    assert "void S46Snap(" in stage44 and "S46Resolved(" in stage45
    assert "S46Snap(object,fields,mask);" in s47
    assert "kS47Capacity=4096u" in s47
    assert s47.count("MakeRelativeCall(") == 1
    assert "void __cdecl S47Enter(" in s47 and "void __cdecl S47Leave(" in s47
    assert "g_playerP1+4u" in s47 and "g_playerP2+4u" in s47
    assert "S47Log(" in s47 and "FlushStage47()" in s47
    assert "CreateThread(" not in s47 and "CreateRemoteThread(" not in s47
    assert "if(writable && valid())" in s47 and "PAGE_EXECUTE_READWRITE" in s47
    assert "for(DWORD i=0;i<2u;++i)" in s47
    assert "0xAE70Cu" in s47 and "0xAE93Eu" in s47
    assert "0xAE79Fu" not in s47 and "0xAE7BDu" not in s47
    body = s47.split("void S47MotionConsumerHook()",1)[1].split("__asm {",1)[1].split("\n    }",1)[0]
    assert body.count("fxsave [edi]") == 2
    assert body.count("fxrstor [edi]") == 2
    assert body.count("pushfd") == 2 and body.count("popfd") == 2
    assert body.count("pushad") == 2 and body.count("popad") == 2
    assert body.count("call S47Enter") == 1 and body.count("call S47Leave") == 1
    assert body.count("push dword ptr [esp+4]") == 1
    assert body.count("call dword ptr [g_s47NativeUpdate]") == 1
    assert body.rstrip().endswith("ret 4")
    assert adapter.count('#include "Stage47MotionConsumer.inl"') == 1
    assert adapter.count("InstallStage47Hooks()") == 1
    assert adapter.count("FlushStage47();") == 1
    # Validate the integration changed only these insertion/replacement lines.
    reconstructed = adapter.replace('#include "Stage47MotionConsumer.inl"\n',"")
    reconstructed = reconstructed.replace(
        'if(InstallStage44Hooks()){\n'
        '                    if(InstallStage45Hooks())InstallStage47Hooks();\n'
        '                    else S47Log("ERROR: Stage47 requires installed Stage45/46; no patches.");\n'
        '                }else S47Log("ERROR: Stage47 requires installed Stage44/45/46; no patches.");',
        'if(InstallStage44Hooks())InstallStage45Hooks();')
    reconstructed = reconstructed.replace("    FlushStage47();\n","")
    assert reconstructed == baseline, "existing Stage38/39/40/42/44/45 adapter changed"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--root",type=Path,default=Path(__file__).resolve().parents[2])
    args = ap.parse_args()
    check_binary(args.exe)
    check_source(args.root)
    print("PASS Stage47: SHA-pinned original PE, 6 native CALLs (2 target object-list),")
    print("verified RET4 ABI, native field reads/writes, 2-site transactional patch,")
    print("wrapper preservation static checks, existing worker integration unchanged.")
    print("NOT TESTED: Windows x86 build, actual wrapper equivalence, game runtime,")
    print("animation, position, hit/damage, or P1/P2 callsite coverage.")

if __name__ == "__main__":
    main()
