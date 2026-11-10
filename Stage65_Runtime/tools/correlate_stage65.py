#!/usr/bin/env python3
"""Stage65 same-launch offline provenance join (read-only, Python 3 stdlib).

Checks source-writer -> native consumer -> pending commit -> activation.
Does NOT assume every activation's latest preceding request is its source,
does NOT label native ID values as AI choices, and does NOT alter game state.
"""
import argparse
import collections
import json
import re
from datetime import datetime
from pathlib import Path

INPUTS={
    "s65":("ArcanaAI_Stage65RequestOriginTrace.log","STAGE65_WRITER_WINDOW"),
    "s40":("ArcanaAI_Stage40ActionConsumerTrace.log","STAGE40_CONSUMER_WINDOW"),
    "s41":("ArcanaAI_Stage41ObjectCommitTrace.log","STAGE41_OBJECT_WINDOW"),
    "s42":("ArcanaAI_Stage42ActionActivationTrace.log","STAGE42_ACTIVATION_WINDOW"),
    "s43":("ArcanaAI_Stage43ActivationOriginTrace.log","STAGE43_ORIGIN_WINDOW"),
    "s44":("ArcanaAI_Stage44ActionDefinitionTrace.log","STAGE44_BINDING_WINDOW"),
    "s39":("ArcanaAI_Stage39ActionOutputTrace.log","STAGE39_OUTPUT_WINDOW"),
}
KV=re.compile(r"([A-Za-z]\w*)=([^\s]+)")
STAMP=re.compile(r"^\[(\d{4}-\d\d-\d\d \d\d:\d\d:\d\d)\]")


def hx(x): return int(x.removeprefix("0x"),16)
def q(x): return int(x)
def aft(x): return x.split(">",1)[1]
def bef(x): return x.split(">",1)[0]
def actor(x,y):
    return x["side"]==y["side"] and hx(x["player"])==hx(y["player"]) and x["tid"]==y["tid"]


def read(path, tag):
    out=[]
    stamp=None
    for line in path.read_text(encoding="utf-8-sig",errors="replace").splitlines():
        if stamp is None and (m:=STAMP.match(line)):
            stamp=datetime.strptime(m.group(1),"%Y-%m-%d %H:%M:%S")
        if "] "+tag+" " in line:
            row=dict(KV.findall(line))
            if not all(k in row for k in ("side","player","tid")):
                raise ValueError(f"{path}: incomplete {tag}")
            out.append(row)
    if stamp is None or not out:
        raise ValueError(f"Missing header or {tag}: {path}")
    return stamp,out


def run(folder):
    data={}
    stamps={}
    for key,(name,tag) in INPUTS.items():
        stamps[key],data[key]=read(folder/name,tag)
    if (max(stamps.values())-min(stamps.values())).total_seconds()>5:
        raise ValueError("Mixed log starts; use one exact COM-vs-COM session")
    W,C,B,A,G,D,F=(data[k] for k in ("s65","s40","s41","s42","s43","s44","s39"))
    for key,rows,seq in (("s65",W,"seq"),("s40",C,"seq"),("s42",A,"seq"),("s44",D,"seq")):
        if sorted(int(r[seq]) for r in rows)!=list(range(1,len(rows)+1)):
            raise ValueError(key+" has duplicate or missing sequence IDs")
    bmap={x["stage40Seq"]:x for x in B}
    gmap={x["stage42Seq"]:x for x in G}
    if len(bmap)!=len(B) or len(gmap)!=len(G):raise ValueError("duplicate cross-stage seq")
    writers={}
    unexplained_consumers=[]
    for c in C:
        b=bmap.get(c["seq"])
        if (not b or not actor(c,b) or b["qpcBefore"]!=c["qpcBefore"] or
            b["qpcAfter"]!=c["qpcAfter"] or hx(b["request"])!=hx(c["requestAtSelector"]) or
            hx(b["objectAfter"])!=hx(aft(c["p1"]))):
            raise ValueError("Stage40/41 provenance mismatch at seq "+c["seq"])
        older=[w for w in W if actor(w,c) and q(w["qpcAfter"])<=q(c["qpcBefore"])]
        w=max(older,key=lambda x:q(x["qpcAfter"])) if older else None
        if w is None or hx(w["request"])!=hx(c["requestAtSelector"]):
            unexplained_consumers.append({
                "seq":int(c["seq"]),"side":c["side"],"request":c["requestAtSelector"],
                "preceding_writer_seq":int(w["seq"]) if w else None,
                "preceding_writer_id":w["request"] if w else None
            })
            continue
        if w["seq"] in writers.values():
            raise ValueError("One Stage65 writer reused for multiple consumers: "+w["seq"])
        if (w["readMask"].upper()!="3F" or w["directE8"]!="1" or
            hx(w["postRequest"])!=hx(w["request"]) or
            (w["callerCallRVA"].upper()=="FFFFFFFF")):
            raise ValueError("Stage65 writer incomplete at "+w["seq"])
        writers[c["seq"]]=w["seq"]
    if len(writers)!=len(set(writers.values())):
        raise ValueError("duplicate Stage65 consumed writer")
    if len(writers)!=len(W):
        raise ValueError("Not every observed Stage65 writer was consumed exactly once")

    a_to_c={}
    anomalous_activations=[]
    for a in A:
        g=gmap.get(a["seq"])
        bmatches=[d for d in D if actor(a,d) and d["object"]==a["object"]
                  and q(a["qpcBefore"])<=q(d["qpcBefore"]) and
                  q(d["qpcAfter"])<=q(a["qpcAfter"])]
        if (not g or not actor(g,a) or g["qpcBefore"]!=a["qpcBefore"]
            or g["qpcAfter"]!=a["qpcAfter"] or g["callerRVA"]!="000AD75C"
            or len(bmatches)!=1 or hx(aft(bmatches[0]["f0"]))!=hx(aft(a["o2"]))):
            raise ValueError("Stage42→43→44 mismatch at "+a["seq"])
        prior=[c for c in C if actor(c,a) and
               q(c["qpcAfter"])<=q(a["qpcBefore"])]
        c=max(prior,key=lambda x:q(x["qpcAfter"])) if prior else None
        if c is None or hx(c["requestAtSelector"])!=hx(aft(a["o2"])):
            anomalous_activations.append({
                "seq":int(a["seq"]), "side":a["side"],"activated_id":aft(a["o2"]),
                "latest_stage40_seq":int(c["seq"]) if c else None,
                "latest_stage40_id":c["requestAtSelector"] if c else None
            })
        else:
            if c["seq"] in a_to_c.values():
                raise ValueError("One Stage40 consumption reused for multiple activations")
            a_to_c[a["seq"]]=c["seq"]

    consumed_by_activation=set(a_to_c.values())
    nonactivation=[c for c in C if c["seq"] not in consumed_by_activation]
    dup_sites=collections.Counter()
    for c in nonactivation:
        b=bmap[c["seq"]]
        if not (bef(b["o0"])==aft(b["o0"]) and bef(b["o3"])==aft(b["o3"])
                and hx(aft(b["o3"]))==hx(bef(b["o2"]))
                and hx(aft(b["o3"]))==hx(c["requestAtSelector"])):
            raise ValueError("Unmatched Stage40 has nonduplicate object update: "+c["seq"])
        wseq=writers.get(c["seq"])
        w=next((r for r in W if r["seq"]==wseq),None)
        dup_sites[w["callerCallRVA"] if w else "UNKNOWN"]+=1

    matched39=[]
    for f in F:
        following=sorted((c for c in C if actor(f,c) and
                         q(c["qpcBefore"])>=q(f["qpcAfter"])),
                         key=lambda x:q(x["qpcBefore"]))
        c=following[0] if following else None
        matches=[seq for seq,cseq in a_to_c.items() if c is not None and cseq==c["seq"]]
        if (c is not None and hx(c["requestAtSelector"])==hx(aft(f["a0"])) and len(matches)==1):
            matched39.append(matches[0])
    stage39_unique=set(matched39)
    callsites=collections.Counter(w["callerCallRVA"] for w in W)
    activation_writers={writers[cseq] for cseq in a_to_c.values() if cseq in writers}
    summary={
      "session_log_starts":{k:stamps[k].isoformat(sep=" ") for k in INPUTS},
      "counts":{k:len(data[k]) for k in INPUTS},
      "stage65_matched_to_stage40":len(writers),
      "stage40_exact_stage41_pairs":len(C),
      "stage42_exact_stage43_stage44_pairs":len(A),
      "stage42_latest_matching_stage40":len(a_to_c),
      "stage42_latest_matching_stage40_with_stage65":len(activation_writers),
      "stage39_matched_to_activation":len(stage39_unique),
      "stage40_without_distinct_activation":len(nonactivation),
      "nonactivation_stage65_caller_counts":dict(dup_sites),
      "writer_unique_original_call_sites":len(callsites),
      "top_original_call_sites":callsites.most_common(15),
      "stage65_modes":dict(collections.Counter(w["mode"] for w in W)),
      "unexplained_stage40":unexplained_consumers,
      "anomalous_stage42":anomalous_activations,
      "conclusion_limit":"Event-site attribution, not native tactical COM choice scoring or AI-quality proof."
    }
    return summary


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dir",type=Path,required=True)
    parser.add_argument("--json",type=Path)
    args=parser.parse_args()
    obj=run(args.dir)
    result=json.dumps(obj,indent=2,ensure_ascii=False)
    print(result)
    if args.json:
        args.json.write_text(result+"\n",encoding="utf-8")

if __name__=="__main__":main()
