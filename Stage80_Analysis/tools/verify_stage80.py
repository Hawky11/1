#!/usr/bin/env python3
"""Verify pinned original AALib PE and Stage80 source structure (no writes).

Usage:
  py -3 Stage80_Analysis/tools/verify_stage80.py "D:\PATH\AALib.zip"
  py -3 Stage80_Analysis/tools/verify_stage80.py "D:\PATH\AALib.exe"
"""
import argparse
import hashlib
import json
import struct
import zipfile
from pathlib import Path

SHA="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
EXACT={
  0x0E8A8D:"56",                   # original PUSH ESI argument
  0x0E8A8E:"e8bde60000",           # CALL F7150, five bytes
  0x0E8A93:"83c40484c0",           # original ADD ESP,4; TEST AL,AL
  0x0F7150:"558bec81ecd4000000535657",  # F7150 prologue
  0x0F7D22:"8997503f0000",         # computed selector 1/2
  0x0F7D38:"898f503f0000",         # computed selector 1/2
  0x0F7D43:"c787503f000000000000", # selector literal 0
  0x0EFFE8:"e8f389ffff",           # inherited Stage79, original before detours
}
def read_original(path):
    if path.suffix.lower()==".zip":
        with zipfile.ZipFile(path) as z:
            names=[n for n in z.namelist() if Path(n).name.lower()=="aalib.exe"]
            if len(names)!=1:raise ValueError("Expected exactly one AALib.exe")
            data=z.read(names[0])
    else:data=path.read_bytes()
    digest=hashlib.sha256(data).hexdigest()
    if digest!=SHA:raise ValueError("Incorrect original SHA256 "+digest)
    if data[:2]!=b"MZ":raise ValueError("Not an MZ image")
    pe=struct.unpack_from("<I",data,0x3c)[0]
    if data[pe:pe+4]!=b"PE"+bytes((0,0)):raise ValueError("Missing PE signature")
    n=struct.unpack_from("<H",data,pe+6)[0]
    opt=pe+24
    if struct.unpack_from("<H",data,pe+4)[0]!=0x14c or struct.unpack_from("<H",data,opt)[0]!=0x10b:
        raise ValueError("Expected i386 PE32")
    if struct.unpack_from("<I",data,opt+28)[0]!=0x400000:
        raise ValueError("Unexpected PE ImageBase")
    sec=opt+struct.unpack_from("<H",data,pe+20)[0]
    for i in range(n):
        off=sec+i*40
        if data[off:off+8].split(bytes((0,)))[0]==b".text":
            _vsize,rva,size,raw=struct.unpack_from("<IIII",data,off+8)
            text=data[raw:raw+size]
            if len(text)!=size:raise ValueError("Truncated .text")
            return rva,text
    raise ValueError("No .text")

def audit(path):
    base,text=read_original(path)
    def read(rva,n):
        offset=rva-base
        if offset<0 or offset+n>len(text):raise ValueError("Unmapped .text RVA %06X"%rva)
        return text[offset:offset+n]
    for rva,hexbytes in EXACT.items():
        expected=bytes.fromhex(hexbytes)
        if read(rva,len(expected))!=expected:
            raise ValueError("Original opcode mismatch RVA %06X"%rva)
    for site,target in [(0x0E8A8E,0x0F7150),(0x0EFFE8,0x0E89E0)]:
        op=read(site,5)
        if op[0]!=0xE8 or site+5+struct.unpack_from("<i",op,1)[0]!=target:
            raise ValueError("Original CALL target mismatch RVA %06X"%site)
    root=Path(__file__).resolve().parents[2]
    observer=(root/"Stage80_Overlay/src/Stage80F7150LateBoundary.inl").read_text(encoding="utf-8")
    adapter=(root/"Stage80_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
    for token in (
        "kS80CallRva=0x0E8A8Eu","kS80TargetRva=0x0F7150u","kS80RetRva=0x0E8A93u",
        "push dword ptr [esp+4]","call dword ptr [g_s80NativeTarget]",
        "STAGE80_LATE_F7150","STAGE80_HEALTH","STAGE80_INSTALLED 1/1",
        "S76SuspendPeers","GetThreadContext","VirtualProtect","FlushInstructionCache",
        "MakeRelativeCall"):
        if token not in observer:raise ValueError("Missing source token: "+token)
    if observer.count("call dword ptr [g_s80NativeTarget]")!=1:
        raise ValueError("Expected exactly one replay of native F7150")
    if observer.count("push dword ptr [esp+4]")!=1:
        raise ValueError("Expected exactly one duplicated native player argument")
    if observer.count("fxsave [edi]")!=observer.count("fxrstor [edi]") or observer.count("fxsave [edi]")!=2:
        raise ValueError("Stage80 FXSAVE/FXRSTOR imbalance")
    if not(observer.index("call S80Enter")<observer.index("call dword ptr [g_s80NativeTarget]")<observer.index("call S80Leave")):
        raise ValueError("Incorrect before/native/after ordering")
    for token in (
        '#include "Stage76AfterCallbackPhases.inl"',
        '#include "Stage78LiteralOneWriter.inl"',
        '#include "Stage79SelectorFamilyBoundary.inl"',
        '#include "Stage80F7150LateBoundary.inl"',
        'if(InstallStage79Hooks()) InstallStage80Hooks();',
        "FlushStage76();","FlushStage78();","FlushStage79();","FlushStage80();",
        "StartStage59Watch();","STAGE77_NATIVE_PREPARE"):
        if token not in adapter:raise ValueError("Missing Stage80 adapter token: "+token)
    if adapter.count("call dword ptr [g_stage38PrepareTarget]")!=1:
        raise ValueError("Stage38 native 156C00 call drift")
    if adapter.count("fxsave [edi]")!=adapter.count("fxrstor [edi]") or adapter.count("fxsave [edi]")!=3:
        raise ValueError("Stage38 FXSAVE/FXRSTOR drift")
    if not(adapter.index("InstallStage80Hooks()")<adapter.index("StartStage59Watch()")):
        raise ValueError("Stage80 must install before Stage59 worker")
    print(json.dumps({
        "status":"STAGE80 ORIGINAL SHA AND SOURCE STRUCTURE VERIFIED; WINDOWS/GAME NOT TESTED",
        "originalSha256":SHA,
        "originalOpcodeSitesVerified":len(EXACT),
        "directOriginalCALLsVerified":2,
        "nativeSite":"0E8A8E","nativeTarget":"0F7150","nativeReturn":"0E8A93",
        "targetWriters":["0F7D22","0F7D38","0F7D43"],
        "hookCallsOriginalOnce":True,
        "originalPlayerArgCopiedOnce":True,
        "additionalDebugRegisters":0,
        "directIndividualWriterEipProven":False
    },indent=2))
if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_AALib_exe_or_zip",type=Path)
    audit(p.parse_args().original_AALib_exe_or_zip)
