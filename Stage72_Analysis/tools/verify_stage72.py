#!/usr/bin/env python3
"""Stage72: verify original callback-dispatch wiring and frozen Stage70 P1 timeline.

Stdlib only; read-only SHA-pinned AALib.exe and optional four archived logs.
No debugger, no gameplay writes, no DLL build. Runtime mode checks a *specific*
2026-11-11 capture, not all possible selector origins.

Usage:
  py -3 Stage72_Analysis/tools/verify_stage72.py "D:\PATH\AALib.exe"
  py -3 Stage72_Analysis/tools/verify_stage72.py "D:\PATH\AALib.exe" ^
        --logs "D:\PATH\FROZEN_LOGS"
"""
import argparse
import hashlib
import json
import re
import struct
from pathlib import Path

SHA256="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
OPCODES={
  0x0F7E70:"558bec8b45088b4d0c8b551089088950045dc3",
  0x156C00:"558bec8b450c8b4d0889018b90d43e000089510c8b108951088b40048941045dc3",
  0x0EE78D:"8bbed03e0000",               # load [P+3ED0] callback pointer into EDI
  0x0EE7BE:"ffd7",                       # execute callback via CALL EDI
  0x158578:"8b5e0c",                    # load callback-context dword +0C
  0x1585C5:"83fb300f87e8070000",       # jump-table bound for EBX <= 30h
  0x1585CE:"ff249dc48d5500",           # JMP [EBX*4 + 00558DC4]
  0x158CE0:"56e87addffff83c4045f5e5b5dc3", # case 22h -> 156A60
  0x156A60:"558bec8b45088b0853568bb1543f0000",
  0x156B82:"6aff68b0665500eb3c",      # register callback 1566B0
  0x156BA1:"8b91503f0000",             # other branch reads selector
  0x156BC0:"6aff68b0605500",           # register callback 1560B0
  0x156BC7:"81c1d03e000051",           # address P+3ED0, pass as arg
  0x156BCE:"e89d12faff",              # call descriptor setter 0F7E70
  0x156794:"68b060550051e8d116faff", # 1566B0 stores follow-on callback 1560B0
  0x155D27:"6a21",                     # event code 21
  0x155D29:"c786503f000001000000",   # selector literal one
  0x155D33:"6a0081c62c3a000056",     # event payload pointer P+3A2C
  0x155D3C:"e89fd0f9ff",              # call event field writer F2DE0
  0x0F2DEC:"8b4d0c",                   # event field writer: arg2 is mode
  0x0F2DFC:"8b5510",                   # event field writer: arg3 is event code
  0x0F2E0B:"66895004",                # stores event at [record+4]
  0x0F2E0F:"66894802",                # stores mode at [record+2]
}

def pe_reader(path):
    data=path.read_bytes()
    digest=hashlib.sha256(data).hexdigest()
    if digest!=SHA256:raise ValueError("Wrong original EXE sha256: "+digest)
    pe=struct.unpack_from("<I",data,0x3c)[0]
    if data[:2]!=b"MZ" or data[pe:pe+4]!=b"PE\0\0":
        raise ValueError("Not a PE")
    n=struct.unpack_from("<H",data,pe+6)[0]
    optsize=struct.unpack_from("<H",data,pe+20)[0]
    op=pe+24
    machine=struct.unpack_from("<H",data,pe+4)[0]
    imagebase=struct.unpack_from("<I",data,op+28)[0]
    if (machine,struct.unpack_from("<H",data,op)[0],imagebase)!=(0x14c,0x10b,0x400000):
        raise ValueError("Unexpected machine/optional-header/base")
    sections=[]
    for i in range(n):
        p=op+optsize+40*i
        name=data[p:p+8].split(b"\0")[0].decode("ascii")
        _virtual_size,rva,raw_size,raw=struct.unpack_from("<IIII",data,p+8)
        sections.append((name,rva,raw_size,raw))
    def read(rva,size):
        hits=[(name,raw+rva-start) for name,start,length,raw in sections
              if start<=rva and rva+size<=start+length]
        if len(hits)!=1:raise ValueError("Unmapped/ambiguous RVA "+hex(rva))
        name,off=hits[0]
        if name!=".text":raise ValueError("Code data not in .text: "+hex(rva))
        return data[off:off+size]
    return digest,read

def original_audit(path):
    digest,read=pe_reader(path)
    for rva,signature in OPCODES.items():
        needle=bytes.fromhex(signature)
        if read(rva,len(needle))!=needle:
            raise ValueError(f"Original code mismatch RVA={rva:06X}: "+
                             read(rva,len(needle)).hex())
    # 158570 uses EBX as dispatch key; in frozen run the callback context
    # input P+3ED4 is 0x22, and original JMP table index 0x22 points 158CE0.
    table_rva=0x158DC4
    slot=0x22
    target_va=struct.unpack("<I",read(table_rva+4*slot,4))[0]
    if target_va!=0x00558CE0:
        raise ValueError("Native 158570 switch case 22 differs: "+hex(target_va))
    # Direct calls are E8 relative instructions. These specific sites do not
    # depend on the PE image base; callback function literals do.
    for source,target in [(0x158CE1,0x156A60),(0x156BCE,0x0F7E70),
                          (0x15679A,0x0F7E70),(0x155D3C,0x0F2DE0)]:
        op=read(source,5)
        if op[0]!=0xE8 or source+5+struct.unpack("<i",op[1:])[0]!=target:
            raise ValueError(f"Native CALL mismatch {source:06X}->{target:06X}")
    return {"originalSHA256":digest,"exactOpcodeRanges":len(OPCODES),
            "switchTableVA":"00558DC4","dispatchKey":34,
            "switchCaseTargetRVA":"158CE0","switchCallTargetRVA":"156A60",
            "descriptorSetterRVA":"0F7E70",
            "descriptorFields":{"callback":"P+3ED0","parameter":"P+3ED4"},
            "followOnCallbackRVA":"1560B0","event21RecordCodeField":"P+3A30",
            "event21DifferentFromDispatchKey22":True,
            "selectorOriginalWriterIPRuntimeVerified":False}

def parse_log(folder,name,tag):
    lines=(folder/name).read_text(encoding="utf-8-sig").splitlines()
    rows=[]
    for line in lines:
        if tag in line:rows.append(dict(re.findall(r"(\w+)=([^\s]+)",line)))
    return rows,lines

def one(rows,key,value):
    matches=[r for r in rows if r.get(key)==str(value)]
    if len(matches)!=1:raise ValueError(f"{key}={value} match count {len(matches)}")
    return matches[0]

def split_hex_pair(row,key):
    return [int(t,16) for t in row[key].split(">")]

def runtime_audit(folder):
    s38,l38=parse_log(folder,"ArcanaAI_Stage38CallbackWindowTrace.log",
                      "STAGE38_CALLBACK_WINDOW ")
    s65,l65=parse_log(folder,"ArcanaAI_Stage65RequestOriginTrace.log",
                      "STAGE65_WRITER_WINDOW ")
    s70,l70=parse_log(folder,"ArcanaAI_Stage70DeferredWriterTrace.log",
                      "STAGE70_DEFERRED_WRITE ")
    if not any("STAGE70_INSTALLED 1/1" in l for l in l70):
        raise ValueError("Stage70 was not installed")
    for r,n in [(s38,38),(s65,65),(s70,70)]:
        vals=[int(x["seq"]) for x in r]
        if vals!=list(range(1,len(vals)+1)):
            raise ValueError(f"Stage{n} sequence gaps or ordering mismatch")
    a=one(s38,"seq",4090);b=one(s38,"seq",4116)
    c=one(s38,"seq",4118); w=one(s65,"seq",319);d=one(s70,"seq",1)
    for r in [a,b,c,w,d]:
        if r.get("side")!="P1":
            raise ValueError("Not P1")
        if r.get("tid")!="20812":
            raise ValueError("Wrong target TID")
    for r in [a,b,c,w,d]:
        player=r.get("player","").removeprefix("0x").upper()
        if player!="0E4A1F78":raise ValueError("Wrong player identity")
    if a["callback"]!="0x00958570" or split_hex_pair(a,"f3")!=[0x00958570,0x009566B0]:
        raise ValueError("Original dispatcher callback assignment not observed")
    if split_hex_pair(a,"f4")!=[0x22,0xFFFFFFFF]:
        raise ValueError("Native callback argument 22 not observed")
    if b["callback"]!="0x009566B0" or split_hex_pair(b,"f3")!=[0x009566B0,0x009560B0]:
        raise ValueError("Native deferred callback continuation not observed")
    if c["callback"]!="0x009560B0":
        raise ValueError("Follow-on callback not observed")
    if split_hex_pair(b,"f20")!=[0xFFFFFFFF,0x25]:
        raise ValueError("Original deferred field not updated in Stage38 window")
    if w["callerCallRVA"]!="00156769" or int(w["request"],16)!=0x2F:
        raise ValueError("Unexpected Stage65 source")
    if d["siteRVA"]!="156784" or d["originalStoreRVA"]!="156787" or \
       split_hex_pair(d,"deferred")!=[0xFFFFFFFF,0x25] or \
       split_hex_pair(d,"selector")!=[1,1] or d["preMask"]!="07" or d["postMask"]!="07":
        raise ValueError("Stage70 deferred store not proven")
    q=lambda r,key:int(r[key])
    if not(q(a,"qpcBefore")<q(a,"qpcAfter")<q(b,"qpcBefore")<
           q(w,"qpcBefore")<q(w,"qpcAfter")<
           q(d,"qpcBefore")<q(d,"qpcAfter")<=q(b,"qpcAfter")<
           q(c,"qpcBefore")):
        raise ValueError("QPC callback nesting/order mismatch")
    if not all(x["f0"].split(">")[0]=="2A3A3020" for x in [a,b,c]):
        raise ValueError("Changed object")
    return {"status":"EXISTING RUNTIME CALLBACK DISPATCH AND CONTINUATION VERIFIED",
            "session":"2026-11-11 02:54:29","P1":"0E4A1F78","TID":20812,
            "Stage38_dispatch":{"seq":4090,"callbackRVA":"158570",
                "parameter_before":"22","next_callbackRVA":"1566B0",
                "qpc":a["qpcBefore"]},
            "Stage38_deferred":{"seq":4116,"callbackRVA":"1566B0",
                "next_callbackRVA":"1560B0","qpcBefore":b["qpcBefore"],
                "qpcAfter":b["qpcAfter"]},
            "Stage65_request":{"seq":319,"callerRVA":"156769","request":"2F"},
            "Stage70_deferred":{"seq":1,"writeValue":"25",
                "sourceSelector":1,"sourceStoreRVA":"156787"},
            "Stage38_next_callback":{"seq":4118,"callbackRVA":"1560B0"},
            "selectorWriterIPSeen":False,
            "note":"Dispatch key 22 at P+3ED4 is not event code 21 at P+3A30."}

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_exe",type=Path)
    parser.add_argument("--logs",type=Path)
    args=parser.parse_args()
    result={"static":original_audit(args.original_exe)}
    if args.logs:result["runtime"]=runtime_audit(args.logs)
    print(json.dumps(result,indent=2,ensure_ascii=False))
    print("PASS: native dispatcher wiring / frozen Stage70 callback sequence; selector writer still unresolved.")

if __name__=="__main__":
    main()
