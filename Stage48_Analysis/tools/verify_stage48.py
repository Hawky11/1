#!/usr/bin/env python3
"""Stage48 SHA-pinned original CALL/guards and minimal overlay integration.

Static verification only: native wrapper reuse does NOT replace a real Windows
x86 build or COM-vs-COM runtime validation.
"""
from __future__ import annotations
import argparse
import hashlib
import struct
from pathlib import Path

SHA256 = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
LEN = 175556608
SIG = {
    0x1E8DA1: "397e6c74078bce",
    0x1E8DA8: "e8e3f6ffff",
    0x1E9088: "39566c74078bce",
    0x1E908F: "e8fcf3ffff",
    0x1E8490: "558bec83ec0c5356578bf1",
    0x1E9E2D: "e85ee6ffff",
}
def pe_read(data: bytes):
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    assert data[:2] == b"MZ"
    assert data[pe:pe+4] == bytes([0x50,0x45,0,0])
    assert struct.unpack_from("<H", data, pe+24)[0] == 0x10B
    assert struct.unpack_from("<I", data, pe+24+28)[0] == 0x400000
    nsec = struct.unpack_from("<H", data, pe+6)[0]
    start = pe+24+struct.unpack_from("<H", data, pe+20)[0]
    sec=[struct.unpack_from("<IIII",data,start+i*40+8) for i in range(nsec)]
    def read(rva,n):
        for vsize,va,rawsize,rawptr in sec:
            if va<=rva and rva+n<=va+rawsize:
                return data[rawptr+rva-va:rawptr+rva-va+n]
        raise AssertionError(f"unmapped RVA: 0x{rva:08X}")
    return read

def verify_exe(path:Path):
    b=path.read_bytes()
    assert len(b)==LEN, "Wrong original EXE size"
    assert hashlib.sha256(b).hexdigest()==SHA256, "Wrong original SHA"
    read=pe_read(b)
    for r,h in SIG.items():
        assert read(r,len(bytes.fromhex(h)))==bytes.fromhex(h),f"mismatch RVA {r:08X}"
    for r in (0x1E8DA8,0x1E908F,0x1E9E2D):
        disp=struct.unpack("<i",read(r+1,4))[0]
        assert r+5+disp==0x1E8490

def verify_overlay(root:Path):
    def get(path):return (root/path).read_text(encoding="utf-8")
    base44=get("Stage46_Overlay/src/Stage44ActionDefinition.inl")
    base38=get("Stage47_Overlay/src/Stage38CallbackWindow.inl")
    new44=get("Stage48_Overlay/src/Stage44ActionDefinition.inl")
    new38=get("Stage48_Overlay/src/Stage38CallbackWindow.inl")
    extra=get("Stage48_Overlay/src/Stage48VariantBinder.inl")
    assert new44.split("    __asm {")[1].split("\n    }",1)[0] == base44.split("    __asm {")[1].split("\n    }",1)[0]
    assert new44.count("MakeRelativeCall(") == base44.count("MakeRelativeCall(") == 1
    assert 'r.S48CallerRva=nativeReturn-g_gameBase-5u' in new44
    assert "void S48FlushRecord(const S44Rec& r);" in new44
    assert "S48FlushRecord(r);" in new44
    assert new38.count('#include "Stage48VariantBinder.inl"')==1
    assert new38.count("InstallStage48Hooks()")==1
    assert extra.count("MakeRelativeCall(")==1
    assert "g_s44NativeBinding!=g_gameBase+0x1E8490u" in extra
    assert "reinterpret_cast<const void*>(&S44BindingHook)" in extra
    assert "g_s47Installed" in extra and "g_s45Installed" in extra
    assert "STAGE48_INSTALLED 2/2" in extra
    assert "p0 && p1 && valid()" in extra
    assert extra.count("VirtualProtect(")==4  # two writable + two restored pages
    assert "CreateThread(" not in extra and "CreateThread(" not in new44
    expect44 = base44.replace("    ULONGLONG S46Q0=0;\n",
        "    ULONGLONG S46Q0=0;\n    DWORD S48CallerRva=0,S48CallerRead=0;\n")
    expect44 = expect44.replace("};\nvolatile LONG g_s44Installed",
        "};\n// Implemented after Stage47 source inclusion by Stage48VariantBinder.inl.\n"
        "void S48FlushRecord(const S44Rec& r);\nvolatile LONG g_s44Installed")
    expect44 = expect44.replace(
        "    r.Object=object;r.Tid=GetCurrentThreadId();\n    r.Q0=Stage31Qpc();",
        "    r.Object=object;r.Tid=GetCurrentThreadId();\n"
        "    // PUSHAD saved ESP is the pushed FLAGS location; original CALL return\n"
        "    // address is at [saved[3] + 4], without changing native registers.\n"
        "    DWORD nativeReturn=0;\n"
        "    if(SafeReadDword(saved[3]+4u,&nativeReturn) && nativeReturn>=g_gameBase+5u){\n"
        "        r.S48CallerRead=1u;\n"
        "        r.S48CallerRva=nativeReturn-g_gameBase-5u;\n"
        "    }\n"
        "    r.Q0=Stage31Qpc();")
    expect44=expect44.replace("        S46FlushRecord(r);\n",
        "        S46FlushRecord(r);\n        S48FlushRecord(r);\n")
    assert new44==expect44, "Unexpected Stage44 baseline edits"
    expect38=base38.replace('#include "Stage47MotionConsumer.inl"',
        '#include "Stage47MotionConsumer.inl"\n#include "Stage48VariantBinder.inl"')
    expect38=expect38.replace("if(InstallStage45Hooks())InstallStage47Hooks();",
        "if(InstallStage45Hooks()){\n"
        "                        if(InstallStage47Hooks())InstallStage48Hooks();\n"
        '                        else S48Log("ERROR: Stage48 requires installed Stage47; no patches.");\n'
        "                    }")
    assert new38==expect38, "Unexpected Stage38 adapter edits"

def main():
    p=argparse.ArgumentParser()
    p.add_argument("exe",type=Path)
    p.add_argument("--root",type=Path,default=Path(__file__).resolve().parents[2])
    a=p.parse_args()
    verify_exe(a.exe)
    verify_overlay(a.root)
    print("PASS Stage48: original EXE SHA/6 signatures + 3 CALL targets;")
    print("Stage44 wrapper asm unchanged, Stage47 adapter unchanged except")
    print("strict Stage48 integration, no new worker or native state write.")
    print("Windows x86 compilation and in-game runtime NOT TESTED.")

if __name__=="__main__":main()
