#!/usr/bin/env python3
"""Stage76 source-only verifier: SHA-pinned original machine code + exact adapter diff.

Windows (PowerShell):
  py -3 Stage76_Analysis/tools/verify_stage76.py "D:\PATH\TO\AALib.exe"
Read-only verifier. Does not compile, patch, run the game or install DR/VEH.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ORIGINAL_EXE_SHA256="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
STAGE74_ADAPTER_BLOB="106dca56d662fe1c8e0b7083013a535b67cde3c8"
STAGE76_ADAPTER_BLOB="f439a0f35e08891590acdb6099afd04ed8d821a4"
STAGE76_PROBE_BLOB="4d1fad129a6269c764d6950fd8a8269bb6b18189"
# Sites are 5-byte relative CALL instructions in original PE32 .text.
NATIVE={
    0xEE7CF:"8bce",
    0xEE7D1:"e8aafcffff",
    0xEE7D6:"e8c5a5ffff",
    0xEE7DB:"8b8e7c3b0000",
    0xEE7E4:"f6c101740956",
    0xEE7EA:"e8f1fdffff",
    0xEE7EF:"83c404",
}
TARGETS={0xEE7D1:0xEE480,0xEE7D6:0xE8DA0,0xEE7EA:0xEE5E0}
ADAPTER_EDITS=[
    ('#include "Stage70DeferredWriter.inl"\n',
     '#include "Stage70DeferredWriter.inl"\n#include "Stage76AfterCallbackPhases.inl"\n'),
    ('                                                                        if(stage67Ready) InstallStage70Hooks();\n',
     '                                                                        if(stage67Ready){\n'
     '                                                                            const bool stage70Ready=InstallStage70Hooks();\n'
     '                                                                            if(stage70Ready) InstallStage76Hooks();\n'
     '                                                                        }\n'),
    ('    FlushStage70();\n','    FlushStage70();\n    FlushStage76();\n'),
]
def git_blob(raw):
    return hashlib.sha1(b"blob "+str(len(raw)).encode()+bytes((0,))+raw).hexdigest()

def verify_sources(root):
    adapter=(root/"Stage76_Overlay/src/Stage38CallbackWindow.inl").read_bytes()
    probe=(root/"Stage76_Overlay/src/Stage76AfterCallbackPhases.inl").read_bytes()
    if git_blob(adapter)!=STAGE76_ADAPTER_BLOB:
        raise ValueError("Stage76 adapter source drift "+git_blob(adapter))
    if git_blob(probe)!=STAGE76_PROBE_BLOB:
        raise ValueError("Stage76 three-CALL probe source drift "+git_blob(probe))
    restored=adapter.decode("utf-8")
    for old,new in ADAPTER_EDITS:
        if restored.count(new)!=1:raise ValueError("Missing or duplicated adapter edit")
        restored=restored.replace(new,old)
    if git_blob(restored.encode("utf-8"))!=STAGE74_ADAPTER_BLOB:
        raise ValueError("Stage76 adapter differs beyond the 3 expected edits")
    text=probe.decode("utf-8")
    tokens=[
      "__declspec(naked) void S76HookEE480()",
      "__declspec(naked) void S76HookE8DA0()",
      "__declspec(naked) void S76HookEE5E0()",
      "g_s76NativeEE480=g_gameBase+kS76Targets[0]",
      "g_s76NativeE8DA0=g_gameBase+kS76Targets[1]",
      "g_s76NativeEE5E0=g_gameBase+kS76Targets[2]",
      "S76OriginalOK()",
      "S76SuspendPeers(peers,count)",
      "VirtualProtect(addr[0],0x1Eu,PAGE_EXECUTE_READWRITE,&oldProtect)",
      "FlushInstructionCache(GetCurrentProcess(),addr[0],0x1Eu)",
      "ResumeThreads(peers,count)",
      "STAGE76_INSTALLED 3/3",
      "STAGE76_PHASE",
      "STAGE76_HEALTH",
      "r.P=saved[1];r.ReturnVA=saved[9]",
      "push dword ptr [esp+4]",
      "call dword ptr [g_s76NativeEE5E0]",
      "add esp,4",
      "ret                                // native EE7EF, then ADD ESP,4",
    ]
    missing=[token for token in tokens if token not in text]
    if missing:raise ValueError("Missing source invariants: "+str(missing))
    forbidden=["SetThreadContext(","Dr0=","Dr1=","Dr2=","Dr3=","AddVectoredExceptionHandler("]
    found=[token for token in forbidden if token in text]
    if found:raise ValueError("Unexpected DR/VEH ownership: "+str(found))
    if text.count("pushfd")!=6 or text.count("popfd")!=6 or text.count("fxsave [edi]")!=6 or text.count("fxrstor [edi]")!=6:
        raise ValueError("Not exactly 6 balanced passive save/restore regions")
    return {"stage74SourceRestored":True, "adapterEdits":3,"newNativeHookSites":3,
            "nativeCallReplays":3,"debugRegistersAllocated":0,
            "sourceGuards":len(tokens),"sourceOnly":True}

def native_text(path):
    exe=path.read_bytes()
    digest=hashlib.sha256(exe).hexdigest()
    if digest!=ORIGINAL_EXE_SHA256:raise ValueError("Original private EXE SHA256 mismatch: "+digest)
    pe=struct.unpack_from("<I",exe,0x3c)[0]
    if exe[:2]!=b"MZ" or exe[pe:pe+4]!=b"PE"+bytes((0,0)):
        raise ValueError("Not a PE image")
    opt=pe+24
    machine=struct.unpack_from("<H",exe,pe+4)[0]
    optmagic=struct.unpack_from("<H",exe,opt)[0]
    imagebase=struct.unpack_from("<I",exe,opt+28)[0]
    if (machine,optmagic,imagebase)!=(0x14c,0x10b,0x400000):
        raise ValueError("Expected PE32 x86 image with base 00400000")
    count=struct.unpack_from("<H",exe,pe+6)[0]
    secs=opt+struct.unpack_from("<H",exe,pe+20)[0]
    for i in range(count):
        p=secs+i*40
        if exe[p:p+8].split(bytes((0,)))[0]==b".text":
            vsz,rva,rsz,raw=struct.unpack_from("<IIII",exe,p+8)
            return digest,rva,exe[raw:raw+rsz]
    raise ValueError("No .text section")

def verify_exe(exe):
    digest,base,t=native_text(exe)
    def read(rva,n):
        offset=rva-base
        if offset<0 or offset+n>len(t):raise ValueError("RVA outside .text")
        return t[offset:offset+n]
    for at,h in NATIVE.items():
        expected=bytes.fromhex(h)
        if read(at,len(expected))!=expected:
            raise ValueError("Original instruction mismatch at %06X: %s"%
                             (at,read(at,len(expected)).hex()))
    for at,dest in TARGETS.items():
        raw=read(at,5)
        actual=at+5+struct.unpack_from("<i",raw,1)[0]
        if raw[0]!=0xE8 or actual!=dest:
            raise ValueError("Original E8 target mismatch %06X→%06X"%(at,actual))
    return {"originalSha256":digest,"originalOpcodeRanges":len(NATIVE),
            "exactRelativeCallTargets":len(TARGETS),
            "callSites":{f"{k:06X}":f"{v:06X}" for k,v in TARGETS.items()},
            "callsiteReturnRVAs":[f"{x+5:06X}" for x in TARGETS]}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_exe",type=Path)
    p.add_argument("--repo-root",type=Path,default=Path(__file__).resolve().parents[2])
    a=p.parse_args()
    result={"stage":76,"status":"STATIC SOURCE VERIFIED ONLY; Windows build/runtime pending"}
    result.update(verify_exe(a.original_exe))
    result.update(verify_sources(a.repo_root))
    result["nativeExecutionVerified"]=False
    result["writerEipVerified"]=False
    print(json.dumps(result,indent=2))
    print("PASS Stage76 static; no Windows build or gameplay test performed.")

if __name__=="__main__":
    main()
