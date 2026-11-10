#!/usr/bin/env python3
"""Stage66 offline evidence verifier: object pending-ID promotion and alternate zero writer.

Never modifies/executes AALib.exe. Optional --logs accepts the original same-run
Stage40/41/42/43/65 logs and validates exact two ID25 anomalies and Stage40 seq320.
Reports original static possibilities separately from proven runtime branches.
"""
import argparse
import hashlib
import json
import re
import struct
from pathlib import Path

EXPECTED_SHA = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
IMAGE_BASE = 0x400000
SITES = {
    0xAD75C: "E8 DF C7 13 00",       # caller to original 1E9F40
    0x1E9F22: "89 41 64",            # independent direct pending-ID write
    0x1E9F40: "80 79 30 00",       # [object+30] gate
    0x1E9F44: "74 16",
    0x1E9F46: "8B 41 68",          # load deferred object+68
    0x1E9F49: "83 F8 FF",          # compare sentinel
    0x1E9F4C: "74 0E",
    0x1E9F4E: "89 41 64",          # promote object+68 -> object+64
    0x1E9F51: "C7 41 68 FF FF FF FF",
    0x1E9F58: "83 49 08 01",      # set pending bit0
    0x1E9F62: "E8 09 FD FF FF",    # Stage42 native activation call
    0xE9E30: "55 8B EC 56 8B F1",
    0xE9E3C: "C1 E8 0F",
    0xE9E3F: "A8 01",
    0xE9E41: "75 23",             # skip zero write if P+3B7C bit15 set
    0xE9E4E: "E8 7D 90 00 00",    # conditional F2ED0 helper
    0xE9E56: "84 C0",
    0xE9E58: "74 0C",             # skip write if helper AL=0
    0xE9E5A: "8B 15 E8 54 82 00", # read original global VA 008254E8
    0xE9E60: "89 96 4C 3F 00 00",
    0xE9E70: "81 A6 7C 3B 00 00 FF F7 FF FF",
}
INPUTS = {
    "s65": ("ArcanaAI_Stage65RequestOriginTrace.log", "STAGE65_WRITER_WINDOW"),
    "s40": ("ArcanaAI_Stage40ActionConsumerTrace.log", "STAGE40_CONSUMER_WINDOW"),
    "s41": ("ArcanaAI_Stage41ObjectCommitTrace.log", "STAGE41_OBJECT_WINDOW"),
    "s42": ("ArcanaAI_Stage42ActionActivationTrace.log", "STAGE42_ACTIVATION_WINDOW"),
    "s43": ("ArcanaAI_Stage43ActivationOriginTrace.log", "STAGE43_ORIGIN_WINDOW"),
}
KV = re.compile(r"([A-Za-z]\w*)=([^\s]+)")
STAMP = re.compile(r"^\[(\d{4}-\d\d-\d\d \d\d:\d\d:\d\d)\]")

def section_reader(data):
    pe = struct.unpack_from("<I", data, 0x3c)[0]
    assert data[pe:pe+4] == b"PE\x00\x00", "not PE"
    machine, count = struct.unpack_from("<HH", data, pe+4)
    optsz = struct.unpack_from("<H", data, pe+20)[0]
    base = struct.unpack_from("<I", data, pe+24+28)[0]
    magic = struct.unpack_from("<H", data, pe+24)[0]
    assert (machine, magic, base) == (0x14c, 0x10b, IMAGE_BASE)
    sections = []
    for i in range(count):
        pos = pe+24+optsz+i*40
        name = data[pos:pos+8].split(b"\0")[0].decode("ascii")
        vs, va, rs, rp = struct.unpack_from("<IIII", data, pos+8)
        sections.append((name, vs, va, rs, rp))
    def read(rva, n):
        for name, vs, va, rs, rp in sections:
            if va <= rva and rva+n <= va+min(vs, rs):
                return data[rp+rva-va:rp+rva-va+n]
        raise ValueError("unmapped original RVA %08X" % rva)
    return read, sections

def verify_static(path):
    data = path.read_bytes()
    sha = hashlib.sha256(data).hexdigest()
    if sha != EXPECTED_SHA:
        raise ValueError("different AALib.exe SHA: "+sha)
    read, sections = section_reader(data)
    for rva, spelling in SITES.items():
        expected = bytes.fromhex(spelling)
        if read(rva, len(expected)) != expected:
            raise ValueError("original instruction mismatch at RVA %06X" % rva)
    def call(src, dst):
        bs=read(src, 5)
        target=(src+5+struct.unpack_from("<i", bs, 1)[0]) & 0xffffffff
        if bs[0]!=0xE8 or target!=dst:
            raise ValueError("native direct call mismatch %06X -> %06X" % (src,dst))
    call(0xAD75C, 0x1E9F40)
    call(0x1E9F62, 0x1E9C70)
    call(0xE9E4E, 0xF2ED0)
    # Global 008254E8 is in original read-only .rdata, not .text or writable .data.
    global_va = 0x008254E8
    global_rva = global_va-IMAGE_BASE
    rdata = next(s for s in sections if s[0]==".rdata")
    assert rdata[2]<=global_rva<rdata[2]+rdata[1], "global not in .rdata"
    global_value=struct.unpack("<I",read(global_rva,4))[0]
    if global_value != 0:
        raise ValueError("zero-source original global no longer zero")
    text=next(s for s in sections if s[0]==".text")
    buf=read(text[2], min(text[1],text[3]))
    count = {}
    for target in (0xE9E30, 0x1E9F00, 0x1E9F40):
        sources=[]
        for i in range(len(buf)-4):
            if buf[i]!=0xE8:
                continue
            site=text[2]+i
            dest=(site+5+struct.unpack_from("<i",buf,i+1)[0]) & 0xffffffff
            if dest==target:
                sources.append("%06X" % site)
        count["%06X" % target]={"direct_encoded_E8_count":len(sources),"rvas":sources}
    assert [count[x]["direct_encoded_E8_count"] for x in ("0E9E30","1E9F00","1E9F40")] == [11,476,14]
    return {
        "sha256":sha,
        "original_instruction_ranges_verified":len(SITES),
        "native_rel32_edges_checked":3,
        "read_only_global_008254E8":global_value,
        "direct_E8_targets":count,
        "original_pending_promotion": {
            "caller_RVA":"AD75C",
            "function_RVA":"1E9F40",
            "preconditions":"O+30 byte !=0 and O+68 != FFFFFFFF",
            "writes":["O+64=O+68 at 1E9F4E","O+68=FFFFFFFF at 1E9F51","O+08 bit0 OR1 at 1E9F58"],
            "before_stage42_call":"1E9F62 -> 1E9C70",
        },
        "original_alternate_zero": {
            "function_RVA":"E9E30",
            "requires":"P+3B7C bit15 CLEAR and F2ED0 returns nonzero AL",
            "store_RVA":"E9E60",
            "writes":"P+3F4C = [008254E8] = 00000000",
        },
    }

def rows(folder, filename, tag):
    result=[]
    stamp=None
    for line in (folder/filename).read_text(encoding="utf-8-sig", errors="replace").splitlines():
        if stamp is None and (m:=STAMP.match(line)):
            stamp=m.group(1)
        if "] "+tag+" " in line:
            result.append(dict(KV.findall(line)))
    if not result or stamp is None:
        raise ValueError("missing log "+filename)
    return stamp,result

def hx(s):return int(s.removeprefix("0x"),16)
def q(s):return int(s)
def before(s):return s.split(">",1)[0]
def after(s):return s.split(">",1)[1]
def same(a,b):
    return a["side"]==b["side"] and hx(a["player"])==hx(b["player"]) and a["tid"]==b["tid"]

def verify_logs(folder):
    inputs={key:rows(folder,*spec) for key,spec in INPUTS.items()}
    if len({stamp[:10] for stamp,values in inputs.values()})!=1:
        raise ValueError("mixed log dates")
    w,c,b,a,g=[inputs[key][1] for key in INPUTS]
    cmap={v["seq"]:v for v in c}
    bmap={v["stage40Seq"]:v for v in b}
    gmap={v["stage42Seq"]:v for v in g}
    anomalies=[]
    for activation in a:
        earlier=[v for v in c if same(v,activation) and q(v["qpcAfter"])<=q(activation["qpcBefore"])]
        last=max(earlier,key=lambda v:q(v["qpcAfter"])) if earlier else None
        if last is not None and hx(last["requestAtSelector"])!=hx(after(activation["o2"])):
            bm=bmap[last["seq"]]
            gm=gmap[activation["seq"]]
            anomalies.append({
                "stage42_seq":int(activation["seq"]),
                "side":activation["side"],
                "stage40_seq":int(last["seq"]),
                "stage41_pending_after":after(bm["o3"]),
                "stage42_pending_at_activation":before(activation["o3"]),
                "stage42_active_after":after(activation["o2"]),
                "stage43_caller_RVA":gm["callerRVA"],
                "stage42_o68_pre":before(activation["o4"]),
                "stage42_o68_post":after(activation["o4"]),
            })
    target = next((v for v in c if v["seq"]=="320"),None)
    if target is None:raise ValueError("Stage40 seq320 not in supplied run")
    older=[v for v in w if same(v,target) and q(v["qpcAfter"])<=q(target["qpcBefore"])]
    preceding=max(older,key=lambda v:q(v["qpcAfter"])) if older else None
    later_zero=[v for v in w if same(v,target) and hx(v["request"])==0
                and q(v["qpcBefore"])>q(target["qpcAfter"])]
    if sorted([x["stage42_seq"] for x in anomalies])!=[39,155]:
        raise ValueError("expected two Stage42 discrepancies, check session")
    for row in anomalies:
        if (row["side"]!="P1" or row["stage41_pending_after"]!="0000002F" or
            row["stage42_pending_at_activation"]!="00000025" or
            row["stage42_active_after"]!="00000025" or
            row["stage43_caller_RVA"]!="000AD75C"):
            raise ValueError("anomaly values/source do not match 00:44 run")
    if (hx(target["requestAtSelector"])!=0 or preceding is None or
        hx(preceding["request"])==0):
        raise ValueError("missing Stage65 writer anomaly not reproduced")
    return {
        "writer_windows":len(w),"stage40_consumers":len(c),
        "stage41_snapshots":len(b),"stage42_activations":len(a),
        "anomalies":anomalies,
        "stage40_320": {
            "request":target["requestAtSelector"],"side":target["side"],
            "qpcBefore":target["qpcBefore"],
            "prior_stage65_seq":preceding["seq"],
            "prior_stage65_request":preceding["request"],
            "next_stage65_zero_seq":min(later_zero,key=lambda r:q(r["qpcBefore"]))["seq"] if later_zero else None,
            "note":"No observed matching preceding E5BF0 writer; E9E30 remains only a static candidate.",
        },
        "limitation":"O+30/O+68 at entry 1E9F40 and actual E9E30 execution were NOT observed.",
    }

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe",type=Path)
    parser.add_argument("--logs",type=Path,help="Directory of same-run 2026-11-11 00:44 logs")
    parser.add_argument("--json",type=Path)
    args=parser.parse_args()
    result={"static":verify_static(args.exe)}
    print("PASS original SHA, %d byte ranges, 3 native CALL edges, global zero, exact callsite counts" %
          result["static"]["original_instruction_ranges_verified"])
    if args.logs:
        result["runtime"]=verify_logs(args.logs)
        print("PASS runtime anomaly fingerprints: 2 pending overrides and 1 unobserved ID0 producer")
    if args.json:
        args.json.write_text(json.dumps(result,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
        print("Report:",args.json)
    print("Neither original path is proved taken without a separate entry observation.")

if __name__=="__main__":
    main()
