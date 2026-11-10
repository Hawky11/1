#!/usr/bin/env python3
"""Stage70 same-session correlator. Read-only, no game binary or DLL needed.

Usage: py -3 Stage70_Analysis/tools/correlate_stage70.py "D:\CAPTURE"
Requires same-run Stage70, Stage65, Stage41, Stage67 logs.
Only counts direct site writes when original site guard/installation and masks hold.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import re

FILES = {
    70: "ArcanaAI_Stage70DeferredWriterTrace.log",
    65: "ArcanaAI_Stage65RequestOriginTrace.log",
    41: "ArcanaAI_Stage41ObjectCommitTrace.log",
    67: "ArcanaAI_Stage67NativePathsTrace.log",
}
TAGS = {
    70: "STAGE70_DEFERRED_WRITE ",
    65: "STAGE65_WRITER_WINDOW ",
    41: "STAGE41_OBJECT_WINDOW ",
    67: "STAGE67_NATIVE_WINDOW ",
}
KEYVAL = re.compile(r'(\w+)=([^\s]+)')
ERR_KEYS = ("dropped", "overflow", "mismatch", "readFail", "ownershipConflicts",
            "installFailures", "teardownFailures", "objectDrift")


def hx(x): return int(x, 16)
def q0(row): return int(row["qpcBefore"])
def q1(row): return int(row["qpcAfter"])
def pair(row,key): return tuple(int(x,16) for x in row[key].split(">"))
def actor(row): return (row["side"], row["player"], row["tid"])
def writer_object(row, stage):
    return row["object"] if stage != 41 else row["objectBefore"]


def parse_capture(folder):
    records, markers, health = {}, {}, {}
    headers = []
    for stage,path in FILES.items():
        filename = folder / path
        lines = filename.read_text(encoding="utf-8-sig").splitlines()
        if not lines or "]" not in lines[0]:
            raise ValueError("Missing initialization header: "+path)
        headers.append(lines[0].split("]",1)[0])
        records[stage] = [dict(KEYVAL.findall(l)) for l in lines if TAGS[stage] in l]
        markers[stage]=lines
        health[stage]=[dict(KEYVAL.findall(l)) for l in lines
                       if f"STAGE{stage}_HEALTH " in l]
        if any("ERROR:" in l for l in lines):
            raise ValueError("Error marker in "+path)
        for row in health[stage]:
            for key in ERR_KEYS:
                if key in row and int(row[key]) != 0:
                    raise ValueError(f"Nonzero Stage{stage} health: {key}={row[key]}")
    if len(set(headers)) != 1:
        raise ValueError("Different initialization headers (mixed sessions)")
    if not any("STAGE70_INSTALLED 1/1" in l for l in markers[70]):
        raise ValueError("No confirmed Stage70 installed marker")
    if not any("STAGE67_INSTALLED 2/2" in l for l in markers[67]):
        raise ValueError("No confirmed Stage67 installed marker")
    if not records[70]:
        raise ValueError("Stage70 installed but no matching native deferred writes: no provenance claim")
    for stage, rows in records.items():
        seqkey = "stage40Seq" if stage == 41 else "seq"
        seq = sorted(int(r[seqkey]) for r in rows)
        if seq != list(range(1, len(rows)+1)):
            raise ValueError(f"Stage{stage} missing/duplicate sequence")
        for row in rows:
            if q0(row) > q1(row):
                raise ValueError(f"Stage{stage} QPC order")
    return records


def audit(folder):
    d=parse_capture(folder)
    s70,s65,s41,s67=(d[s] for s in (70,65,41,67))
    known_actors={actor(r) for r in s67}
    summary=Counter()
    examples=[]
    for w in s70:
        if actor(w) not in known_actors:
            raise ValueError("Stage70 alien actor/TID")
        if (w["siteRVA"],w["originalStoreRVA"]) != ("156784","156787"):
            raise ValueError("Stage70 wrong native site")
        if w["preMask"]!="07" or w["postMask"]!="07":
            raise ValueError("Stage70 incomplete read masks")
        p=actor(w); obj=w["object"]
        if hx(w["objectPtr"].split(">")[0])!=hx(obj) or \
           hx(w["objectPtr"].split(">")[1])!=hx(obj):
            raise ValueError("Stage70 object identity drift")
        old,new=pair(w,"deferred")
        actual=hx(w["valueEDX"])
        if new!=actual or hx(w["eaxAfter"])!=hx(obj) or \
           hx(w["edxAfter"])!=actual:
            raise ValueError("Stage70 store mismatch")
        summary["observedNativeSiteWrites"]+=1
        if old!=new:
            summary["changedNativeDeferredWrites"]+=1
        if new==0x25: summary["ID25writes"]+=1
        if new==0x26: summary["ID26writes"]+=1
        previous=[a for a in s65 if actor(a)==p and a["object"]==obj
                  and q1(a)<q0(w)]
        source=max(previous,key=q1) if previous else None
        joined65 = source is not None and source["callerCallRVA"]=="00156769"
        if joined65: summary["latestStage65from156769"]+=1
        other70=[x for x in s70 if x is not w and actor(x)==p
                 and x["object"]==obj and q0(x)>q1(w)]
        next70=min((q0(x) for x in other70),default=10**40)
        later=[a for a in s41 if actor(a)==p and writer_object(a,41)==obj
               and q0(a)>q1(w) and q0(a)<next70]
        next41=min(later,key=q0) if later else None
        stage41_match=next41 is not None and hx(next41["o4"].split(">")[0])==new
        if stage41_match: summary["nextStage41seesDeferred"]+=1
        request_match=joined65 and stage41_match and \
                      hx(next41["request"])==hx(source["postRequest"])
        if request_match: summary["sameActorRequestDeferredCommit"]+=1
        promotions=[a for a in s67 if a["kind"]=="0" and actor(a)==p
                    and a["object"]==obj and q0(a)>q1(w)
                    and q0(a)<next70 and pair(a,"v4")== (new,0xFFFFFFFF)
                    and hx(a["v1"].split(">")[0]) != 0]
        promoted=min(promotions,key=q0) if promotions else None
        if promoted: summary["laterNativePromotions"]+=1
        if len(examples)<40:
            examples.append({
                "writerSeq":int(w["seq"]), "side":w["side"], "O":obj,
                "old":f"{old:08X}", "new":f"{new:08X}",
                "selectorBeforeAfter":w["selector"],
                "stage65Seq":int(source["seq"]) if joined65 else None,
                "stage65Request":source["postRequest"] if joined65 else None,
                "stage41Seq":int(next41["stage40Seq"]) if stage41_match else None,
                "stage67PromotionSeq":int(promoted["seq"]) if promoted else None,
                "qpcWriterBefore":q0(w), "qpcWriterAfter":q1(w),
                "qpcGapFromStage65":q0(w)-q1(source) if joined65 else None
            })
    return {"status":"RUNTIME OBSERVATION OF INSTRUMENTED ORIGINAL STORE SITE",
            "caveat":"Instrumented displaced store executes in DLL; siteRVA/originalStoreRVA identify verified original instruction position, not CPU EIP during replay.",
            "counts":dict(summary), "examples":examples}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log_dir",type=Path)
    parser.add_argument("--output",type=Path)
    a=parser.parse_args()
    result=audit(a.log_dir)
    output=json.dumps(result,indent=2,ensure_ascii=False)
    if a.output: a.output.write_text(output+"\n",encoding="utf-8")
    print(output)


if __name__=="__main__":
    main()
