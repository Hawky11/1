#!/usr/bin/env python3
"""Stage53 static original AALib.exe + source/adapter verifier (stdlib only).
Usage: py -3 Stage53_Analysis\tools\verify_stage53.py --aalib "D:\path\AALib.zip"
"""
import argparse
import hashlib
import struct
import sys
import zipfile
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
SHA="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
CHECKS={
 0xA2B8B:"E8 D0 7F 01 00",
 0xA2B90:"E8 0B C5 00 00",
 0xA2B95:"E8 F6 CE 02 00",
 0xAF0A0:"55 8B EC B8 6C 14 00 00 E8 23 C4 15 00",
 0xAF34C:"5F 5E 5B 8B E5 5D C3"
}

def confirm(ok,msg):
    if not ok: raise AssertionError(msg)
    print("PASS",msg)

def read_binary(path):
    if path.suffix.lower()==".zip":
        with zipfile.ZipFile(path) as z: return z.read("AALib.exe")
    return path.read_bytes()

def at_rva(data,rva,n):
    pe=struct.unpack_from("<I",data,0x3c)[0]
    confirm(data[pe:pe+4]==b"PE\x00\x00","Valid PE")
    n_sections=struct.unpack_from("<H",data,pe+6)[0]
    size_optional=struct.unpack_from("<H",data,pe+20)[0]
    base=pe+24+size_optional
    for i in range(n_sections):
        p=base+40*i
        vsize,va,rsize,off=struct.unpack_from("<IIII",data,p+8)
        if va<=rva and rva+n<=va+max(vsize,rsize):
            return data[off+rva-va:off+rva-va+n]
    raise AssertionError("Unmapped RVA "+hex(rva))

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--aalib",type=Path,required=True)
    arg=parser.parse_args()
    b=read_binary(arg.aalib)
    confirm(len(b)==175556608,"Original file size")
    confirm(hashlib.sha256(b).hexdigest()==SHA,"Exact original SHA256")
    pe=struct.unpack_from("<I",b,0x3c)[0]
    confirm(struct.unpack_from("<H",b,pe+24)[0]==0x10b,"Original PE32")
    confirm(struct.unpack_from("<I",b,pe+24+28)[0]==0x400000,"ImageBase 0x400000")
    for rva,signature in CHECKS.items():
        sig=bytes.fromhex(signature)
        confirm(at_rva(b,rva,len(sig))==sig,f"Physical RVA {rva:06X}: {signature}")
    call=at_rva(b,0xA2B90,5)
    confirm(0xA2B90+5+struct.unpack("<i",call[1:])[0]==0xAF0A0,
            "A2B90 CALL destination AF0A0")
    s=(ROOT/"Stage53_Overlay/src/Stage53ParentCoordinate.inl").read_text(encoding="utf-8")
    a=(ROOT/"Stage53_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
    for needle in (
        "S53ParentAF0A0Hook()", "g_s52Installed", "g_s51Installed",
        "0xA2B90u", "0xAF0A0u", "site+5u+rel==native",
        "const BYTE original[]={0xE8,0x0B,0xC5,0x00,0x00};",
        "const BYTE nativeReturn[]={0x5F,0x5E,0x5B,0x8B,0xE5,0x5D,0xC3};",
        "std::memcpy(site,patch,5u)", "STAGE53_INSTALLED 1/1",
        "STAGE53_HEALTH", "S53Snap(r.Object,r.Pre,r.PreMask)",
        "S53Snap(r.Object,r.Post,r.PostMask)"):
        confirm(needle in s,"Source "+needle)
    confirm(s.count("call dword ptr [g_s53Native]")==1,"Original native once")
    confirm(s.count("call S53Enter")==1 and s.count("call S53Leave")==1,
            "Single snapshot pair")
    confirm(s.count("fxsave [edi]")==2 and s.count("fxrstor [edi]")==2,
            "Two x87/SSE restore pairs")
    confirm("#include \"Stage52PreGlobalCoordinate.inl\"\n#include \"Stage53ParentCoordinate.inl\"" in a,
            "Adapter retains Stage52 and adds Stage53")
    confirm("if(InstallStage52Hooks())InstallStage53Hooks();" in a,
            "Stage53 installed after Stage52")
    confirm("FlushStage52();\n    FlushStage53();" in a,"Stage53 flushed through Stage38")
    for n in ("Stage49LinkedMotion.inl","Stage50PreLinkedCoordinate.inl","Stage51GlobalCoordinate.inl"):
        confirm(f"#include \"{n}\"" in a, "Previous hook retained "+n)
    print("Stage53 static verification PASS; Windows x86 build and runtime PENDING.")

if __name__=="__main__":
    try: main()
    except (OSError,AssertionError,ValueError,struct.error,KeyError) as e:
        print("FAIL",e,file=sys.stderr)
        raise SystemExit(1)
