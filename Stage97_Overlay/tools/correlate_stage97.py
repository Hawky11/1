#!/usr/bin/env python3
"""Stage97 candidate [EBP-2C] writer -> Stage95 F7A94 bypass, superseded by Stage96 if present.
Only same-run QPC/player/TID/frame evidence can establish local-to-bypass provenance.
"""
import argparse, collections, json, pathlib, re

def events(path, prefix):
    for line in pathlib.Path(path).read_text(encoding="utf-8-sig", errors="replace").splitlines():
        if not line.startswith(prefix + " "):
            continue
        d = dict(re.findall(r"([A-Za-z0-9_]+)=([A-Za-z0-9]+)", line))
        try:
            d["qpcN"] = int(d["qpc"])
            d["context"] = (d["player"], d["tid"], d["frame"])
            yield d
        except (KeyError, ValueError):
            continue

def join(inherited, writers, bypasses, max_lag=10000):
    by_context=collections.defaultdict(list)
    for row in inherited:
        row=dict(row, source="F77D4_LOCAL", value=row.get("nativeNextEsi", ""))
        by_context[row["context"]].append(row)
    for row in writers:
        row=dict(row, source="STAGE96_"+row.get("siteRVA", ""), value=row.get("nativeNextEsi", ""))
        by_context[row["context"]].append(row)
    for entries in by_context.values():
        entries.sort(key=lambda x:x["qpcN"])
    summary=collections.Counter()
    samples=[]
    for b in bypasses:
        if b.get("chosenEsi", "FFFFFFFF").upper()=="FFFFFFFF":
            continue
        summary["finite_bypass"]+=1
        candidates=by_context[b["context"]]
        recent=[e for e in candidates if 0 <= b["qpcN"]-e["qpcN"] <= max_lag]
        if not recent:
            summary["no_recent_writer"]+=1;result="UNATTRIBUTED";lag=None;prev=None
        else:
            prev=recent[-1]
            lag=b["qpcN"]-prev["qpcN"]
            if prev["value"].upper()==b["chosenEsi"].upper() and prev.get("valid")=="1":
                result=prev["source"];summary["latest_writer_value_matched"]+=1
                summary[result]+=1
            else:
                result="LATEST_WRITER_VALUE_DIFFERENT_OR_INVALID";summary["mismatch"]+=1
        if b.get("chosenEsi", "").upper()=="0000002C" or result.startswith("UNATTRIBUTED") or result.startswith("LATEST"):
            if len(samples)<30:
                samples.append({"bypass_seq":b.get("seq"),"value":b.get("chosenEsi"),"source":result,
                                "lag_ticks":lag,"previous_seq":prev.get("seq") if prev else None,
                                "localD4":prev.get("localD4") if prev else None})
    return dict(summary),samples

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--stage97",required=True)
    p.add_argument("--stage96",required=True)
    p.add_argument("--stage95",required=True)
    p.add_argument("--max-qpc-lag",type=int,default=10000)
    args=p.parse_args()
    a,b=join(list(events(args.stage97,"STAGE97_INHERITED")),
             list(events(args.stage96,"STAGE96_WRITER")),
             list(events(args.stage95,"STAGE95_BYPASS")),args.max_qpc_lag)
    print(json.dumps({"summary":a,"exceptions":b,"caution":"Same-run only; latest writer wins, including FFFFFFFF. No causal AI claim."},indent=2))
if __name__=="__main__":main()
