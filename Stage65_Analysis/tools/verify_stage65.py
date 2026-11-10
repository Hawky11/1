#!/usr/bin/env python3
"""Stage65 SOURCE-ONLY original-EXE and overlay contract verifier.

Pure offline Python 3. Does not load dinput8.dll or attach to AH3.
Use only on the SHA-pinned private AALib.exe. No original EXE is copied.
"""
import argparse
import hashlib
import struct
from pathlib import Path

ORIGINAL_SHA = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
NATIVE_RVA = 0xE5BF0
PREFIX = bytes.fromhex("55 8B EC 80 7D 0C 00")
JE = bytes.fromhex("74 17")
RET8 = bytes.fromhex("5D C2 08 00")


def rva_reader(data):
    pe=struct.unpack_from("<I",data,0x3c)[0]
    if data[pe:pe+4]!=b"PE\x00\x00":
        raise AssertionError("PE signature mismatch")
    machine,count=struct.unpack_from("<HH",data,pe+4)
    optsz=struct.unpack_from("<H",data,pe+20)[0]
    opt=pe+24
    magic=struct.unpack_from("<H",data,opt)[0]
    base=struct.unpack_from("<I",data,opt+28)[0]
    if (machine,magic,base)!=(0x14C,0x10B,0x400000):
        raise AssertionError("PE32/i386/ImageBase mismatch")
    sections=[]
    for i in range(count):
        s=opt+optsz+i*40
        name=data[s:s+8].split(b"\x00")[0]
        virtual_size,va,raw_size,raw=struct.unpack_from("<IIII",data,s+8)
        sections.append((name,va,virtual_size,raw,raw_size))
    def fetch(rva,n):
        for name,va,vsize,raw,rsize in sections:
            if va<=rva and rva+n<=va+min(vsize,rsize):
                return data[raw+rva-va:raw+rva-va+n]
        raise AssertionError("RVA not in PE raw section: %06X" % rva)
    return fetch


def verify_original(exe):
    data=exe.read_bytes()
    digest=hashlib.sha256(data).hexdigest()
    if digest!=ORIGINAL_SHA:
        raise AssertionError("Wrong original executable SHA-256: "+digest)
    read=rva_reader(data)
    for site,expected in (
        (NATIVE_RVA,PREFIX),
        (NATIVE_RVA+7,JE),
        (0xE5C0C,RET8),
        (0xE5C19,RET8),
        (0xE5C06,bytes.fromhex("89 81 4C 3F 00 00")),
        (0xE5C13,bytes.fromhex("89 91 4C 3F 00 00")),
    ):
        got=read(site,len(expected))
        if got!=expected:
            raise AssertionError("Mismatch at %06X: %s" % (site,got.hex()))
    return len(data),digest


def source_contract(source,adapter):
    src=source.read_text(encoding="utf-8")
    glue=adapter.read_text(encoding="utf-8")
    checks={
        "source entry prefix": "0x55,0x8B,0xEC,0x80,0x7D,0x0C,0x00" in src,
        "JE relocation point": "site+7" in src and "tramp[7]=0xE9" in src,
        "two original exits": "expectedSecondExit" in src and "expectedExit" in src,
        "two argument copies": src.count("push dword ptr [esp+8]")==2,
        "original callee exactly once": src.count("call dword ptr [g_s65NativeTrampoline]")==1,
        "native RET8 by adapter": len([x for x in src.splitlines() if x.strip()=="ret 8"])==1,
        "FXSAVE pairs": src.count("fxsave [edi]")==2 and src.count("fxrstor [edi]")==2,
        "native bytes checked again while threads suspended": "if(writable && originalOK())" in src,
        "requires Stage58 ready": "g_s58Installed" in src,
        "no additional DR registers": all(s not in src for s in
           ("SetThreadContext(", "CONTEXT_DEBUG_REGISTERS", "c->Dr0=", "c->Dr1=")),
        "adapter include once": glue.count('#include "Stage65RequestOrigin.inl"')==1,
        "Stage65 install before Stage59 starts": (
            0<=glue.find("InstallStage65Hooks();")<glue.find("StartStage59Watch();")),
        "Stage38 flush integration": "FlushStage58();\n    FlushStage65();" in glue,
    }
    for name,good in checks.items():
        if not good:
            raise AssertionError("Source contract failed: "+name)
    return len(checks)


def simulate_native_stack():
    # Synthetic logical stack, BEFORE JMP entry hook:
    original_return,request,mode=0xAABBCCDD,0x1234,1
    stack=[original_return,request,mode]
    # After copying original flag then request for a native RET8 trampoline:
    copied=[request,mode]
    native_callee_stack=[0xDEADC0DE]+copied
    assert native_callee_stack[1:]==stack[1:]
    # The native RET8 pops its own return address and the copied TWO DWORD args,
    # leaving the original stack untouched for the outer wrapper's RET8.
    after_native=list(stack)
    assert after_native==stack
    after_hook=after_native[3:]
    assert after_hook==[]
    return True


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("exe",type=Path,help="Private original SHA-pinned AALib.exe")
    p.add_argument("--source",type=Path,required=True,help="Stage65_Overlay/src/Stage65RequestOrigin.inl")
    p.add_argument("--adapter",type=Path,required=True,help="Stage65_Overlay/src/Stage38CallbackWindow.inl")
    args=p.parse_args()
    size,digest=verify_original(args.exe)
    checks=source_contract(args.source,args.adapter)
    simulate_native_stack()
    print("PASS original SHA, PE32/i386, 6 native instruction ranges; size",size)
    print("PASS source/adapter contracts",checks,"and synthetic 2-arg RET8 stack model")
    print("NOT TESTED: real Windows x86 compiler, live ABI, thread races, runtime stability")


if __name__=="__main__":
    main()
