#!/usr/bin/env python3
"""Stage79: read-only SHA-pinned original AALib.exe call-tree and source verifier.

Usage:
  py -3 Stage79_Analysis/tools/verify_stage79.py "D:\PATH\AALib.zip"
  py -3 Stage79_Analysis/tools/verify_stage79.py "D:\PATH\AALib.exe"

No EXE, DLL, process, debug registers, or game state is modified.
"""
import argparse
import hashlib
import json
import struct
import zipfile
from pathlib import Path

SHA="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
WRITERS={
  0x0F3DE9:("F3930","898e503f0000",0x95),
  0x0F3E02:("F3930","8996503f0000",0x94),
  0x0F3F50:("F3930","8996503f0000",0x95),
  0x0F3F69:("F3930","898e503f0000",0x94),
  0x0F466F:("F4360","898e503f0000",0x95),
  0x0F4688:("F4360","8996503f0000",0x94),
  0x0F48A9:("F46F0","898e503f0000",0x95),
  0x0F48C2:("F46F0","8996503f0000",0x94),
  0x0F7D22:("F7150","8997503f0000",0x95),
  0x0F7D38:("F7150","898f503f0000",0x94),
}
# Every edge confirmed to be a valid original decoded CALL E8 rel32.
EDGES={
  0x0EFFE8:0x0E89E0,
  0x0E89EE:0x0F6960,
  0x0E8A03:0x0F7150,
  0x0E8A28:0x0F70A0,
  0x0E8A8E:0x0F7150,
  0x0F6987:0x0F46F0,
  0x0F70E4:0x0F7010,
  0x0F705C:0x0F3930,
  0x0F7106:0x0F3930,
  0x0F7AD1:0x0F3930,
  0x0F79E4:0x0F4360,
}
# Function entrance target: every direct E8 hit found by bounded byte scan.
EXPECTED_DIRECT_CALLERS={
  0x0E89E0:[0x0EFFE8],
  0x0F6960:[0x0E89EE],
  0x0F7150:[0x0E8A03,0x0E8A8E],
  0x0F70A0:[0x0E8A28],
  0x0F7010:[0x0F70E4],
  0x0F3930:[0x0F705C,0x0F7106,0x0F7AD1],
  0x0F4360:[0x0F79E4],
  0x0F46F0:[0x0F6987],
}

def load_binary(path):
    if path.suffix.lower()==".zip":
        with zipfile.ZipFile(path) as z:
            names=[name for name in z.namelist() if Path(name).name.lower()=="aalib.exe"]
            if len(names)!=1:raise ValueError("AALib.zip must contain exactly one AALib.exe")
            return z.read(names[0])
    return path.read_bytes()

def read_original(path):
    data=load_binary(path)
    digest=hashlib.sha256(data).hexdigest()
    if digest!=SHA:raise ValueError("Unexpected source PE SHA256 "+digest)
    if data[:2]!=b"MZ":raise ValueError("Missing MZ")
    pe=struct.unpack_from("<I",data,0x3C)[0]
    if data[pe:pe+4]!=b"PE"+bytes((0,0)):raise ValueError("Missing PE")
    n=struct.unpack_from("<H",data,pe+6)[0]
    opt=pe+24
    if struct.unpack_from("<H",data,pe+4)[0]!=0x14C or struct.unpack_from("<H",data,opt)[0]!=0x10B:
        raise ValueError("Expected x86 PE32")
    if struct.unpack_from("<I",data,opt+28)[0]!=0x400000:raise ValueError("Unexpected ImageBase")
    tbl=opt+struct.unpack_from("<H",data,pe+20)[0]
    for i in range(n):
        off=tbl+40*i
        if data[off:off+8].split(bytes((0,)))[0]!=b".text":continue
        vs,rva,size,raw=struct.unpack_from("<IIII",data,off+8)
        text=data[raw:raw+size]
        if len(text)!=size:raise ValueError("Truncated .text")
        return rva,text
    raise ValueError("No .text")

def audit(path):
    base,text=read_original(path)
    def at(rva,n):
        off=rva-base
        if off<0 or off+n>len(text):raise ValueError("Out of .text RVA %06X"%rva)
        return text[off:off+n]
    def exact(rva,pattern):
        data=bytes.fromhex(pattern)
        if at(rva,len(data))!=data:raise ValueError("Mismatch at RVA %06X"%rva)

    exact(0x0EFFE6,"751f")
    exact(0x0EFFE8,"e8f389ffff")
    exact(0x0EFFED,"84c07416")
    exact(0x0E89E0,"8b867c3b0000")
    exact(0x0E89F9,"80be7c3f000000")
    exact(0x0E8A03,"e848e70000")
    exact(0x0E89EE,"e86ddf0000")
    exact(0x0E8A28,"e873e60000")

    for site,(_function,store,cc) in WRITERS.items():
        exact(site,store)
        # Two distinct original families: XOR; TEST AL; SETNE or SETE; INC.
        reg="ecx" if store[2:4] in ("8e","8f") else "edx"
        clear="33c9" if reg=="ecx" else "33d2"
        lowreg="c1" if reg=="ecx" else "c2"
        inc="41" if reg=="ecx" else "42"
        prefix=clear+"84c00f"+format(cc,"02x")+lowreg+inc
        exact(site-len(bytes.fromhex(prefix)),prefix)

    for source,target in EDGES.items():
        call=at(source,5)
        if call[0]!=0xE8 or source+5+struct.unpack_from("<i",call,1)[0]!=target:
            raise ValueError("Wrong original call %06X -> %06X"%(source,target))
    # This is a byte-wide scan of direct E8 candidates, not an exhaustive
    # inventory of indirect calls, runtime aliases or nonstandard entrypoints.
    for target,calls in EXPECTED_DIRECT_CALLERS.items():
        found=[base+i for i in range(len(text)-4)
               if text[i]==0xE8 and
               base+i+5+struct.unpack_from("<i",text,i+1)[0]==target]
        if found!=calls:
            raise ValueError("Unexpected direct E8 candidate set for %06X: %r"%(target,found))

    source_root=Path(__file__).resolve().parents[2]
    adapter=(source_root/"Stage79_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
    probe=(source_root/"Stage79_Overlay/src/Stage79SelectorFamilyBoundary.inl").read_text(encoding="utf-8")
    for token in ('#include "Stage79SelectorFamilyBoundary.inl"',
                  'if(InstallStage78Hooks()) InstallStage79Hooks();',
                  'FlushStage79();','StartStage59Watch();',
                  'STAGE77_NATIVE_PREPARE','FlushStage78();'):
        assert token in adapter,token
    assert adapter.count("call dword ptr [g_stage38PrepareTarget]")==1
    assert adapter.count("fxsave [edi]")==adapter.count("fxrstor [edi]")==3
    for token in ('kS79Site=0x0EFFE8u','kS79Target=0x0E89E0u',
                  'kS79Return=0x0EFFEDu',
                  'STAGE79_COMMON_WINDOW','STAGE79_INSTALLED 1/1',
                  'STAGE79_HEALTH','S76SuspendPeers',
                  'GetThreadContext','VirtualProtect','FlushInstructionCache',
                  'MakeRelativeCall','call dword ptr [g_s79OriginalTarget]'):
        assert token in probe,token
    assert probe.count('call dword ptr [g_s79OriginalTarget]')==1
    assert probe.count('fxsave [edi]')==probe.count('fxrstor [edi]')==2
    assert probe.index('call S79Enter')<probe.index('call dword ptr [g_s79OriginalTarget]')<probe.index('call S79Leave')
    print(json.dumps({
      "status":"STAGE79 STATIC VERIFIED; WINDOWS BUILD AND GAME RUNTIME UNTESTED",
      "originalSha256":SHA,
      "familyBoundary":{"callRVA":"0EFFE8","targetRVA":"0E89E0","returnRVA":"0EFFED"},
      "directCallEdgesValidated":len(EDGES),"directCallInventoriesValidated":len(EXPECTED_DIRECT_CALLERS),
      "numericOneOrTwoWriters":{name:sum(1 for f,_,_ in WRITERS.values() if f==name)
            for name in ("F3930","F4360","F46F0","F7150")},
      "movSitesValidated":len(WRITERS),
      "probeCallsOriginalOnce":True,
      "directWriterEipObserved":False,
      "scope":"bounded direct E8 call paths only; indirect calls and alias stores unproven",
    },indent=2))

if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_AALib_exe_or_zip",type=Path)
    audit(p.parse_args().original_AALib_exe_or_zip)
