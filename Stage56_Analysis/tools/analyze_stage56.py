#!/usr/bin/env python3
"""Stage56/Stage55 same-run raw-X writer join (Python stdlib only).

Matches writer events nested *inside* ACDF0 function windows by exact
side+object+thread and BOTH QPC interval bounds. No cross-run joins.
"""
import argparse
from collections import defaultdict
import csv
from bisect import bisect_left
import json
from pathlib import Path
import re

FIELDS = re.compile(r"([A-Za-z][A-Za-z0-9]*)=([^\s]+)")
RAW = re.compile(r"^([0-9A-Fa-f]{8})>([0-9A-Fa-f]{8})$")


def parse_fields(text):
    return dict(FIELDS.findall(text))


def hexi(fields, name):
    return int(fields[name], 16)


def deci(fields, name):
    return int(fields[name], 10)


def side(fields):
    v = fields["side"]
    if v not in ("P1", "P2"):
        raise ValueError("Unknown side: " + v)
    return v


def parse_stage55(path):
    rows, health, seqseen = [], [], set()
    installed = False
    with path.open(encoding="utf-8", errors="replace") as f:
        for line_no, line in enumerate(f, 1):
            if "STAGE55_INSTALLED 3/3" in line:
                installed = True
            if "STAGE55_HEALTH " in line:
                health.append(parse_fields(line))
            marker = "STAGE55_EARLIER_WINDOW "
            if marker not in line:
                continue
            fields = parse_fields(line[line.index(marker) + len(marker):])
            seq = deci(fields, "seq")
            if seq in seqseen:
                raise ValueError(f"Stage55 duplicate seq {seq} line {line_no}")
            seqseen.add(seq)
            if hexi(fields, "callRVA") != 0xA11A0:
                continue
            before, after = RAW.fullmatch(fields["f1"]).groups()
            obj = hexi(fields, "object")
            rows.append({
                "seq": seq, "side": side(fields), "object": obj,
                "tid": deci(fields, "tid"),
                "q0": deci(fields, "qpcBefore"), "q1": deci(fields, "qpcAfter"),
                "x0": int(before, 16), "x1": int(after, 16),
                "pre_mask": hexi(fields, "preMask"), "post_mask": hexi(fields, "postMask"),
            })
    if not installed:
        raise ValueError("Stage55 installed 3/3 marker missing")
    if not rows:
        raise ValueError("No ACDF0 Stage55 rows")
    if any(r["pre_mask"] & 0x2 == 0 or r["post_mask"] & 0x2 == 0
           or r["q0"] > r["q1"] for r in rows):
        raise ValueError("Stage55 invalid X mask or inverted QPC window")
    return rows, health


def parse_stage56(path):
    rows, health, seqseen = [], [], set()
    installed = False
    with path.open(encoding="utf-8", errors="replace") as f:
        for line_no, line in enumerate(f, 1):
            if "STAGE56_INSTALLED 2/2" in line:
                installed = True
            if "STAGE56_HEALTH " in line:
                health.append(parse_fields(line))
            marker = "STAGE56_WRITER "
            if marker not in line:
                continue
            fields = parse_fields(line[line.index(marker) + len(marker):])
            seq = deci(fields, "seq")
            if seq in seqseen:
                raise ValueError(f"Stage56 duplicate seq {seq} line {line_no}")
            seqseen.add(seq)
            before, after = RAW.fullmatch(fields["rawX"]).groups()
            patch = hexi(fields, "patchRVA")
            store = hexi(fields, "storeRVA")
            if (patch, store) not in ((0xACF27, 0xACF2A), (0xACFA0, 0xACFA3)):
                raise ValueError(f"Unexpected Stage56 site {patch:08X}/{store:08X}")
            rows.append({
                "seq": seq, "side": side(fields),
                "object": hexi(fields, "object"), "tid": deci(fields, "tid"),
                "q0": deci(fields, "qpcBefore"), "q1": deci(fields, "qpcAfter"),
                "x0": int(before, 16), "x1": int(after, 16),
                "site": patch, "pre_read": deci(fields, "preRead"),
                "post_read": deci(fields, "postRead"),
            })
    if not installed:
        raise ValueError("Stage56 installed 2/2 marker missing")
    if any(r["q0"] > r["q1"] for r in rows):
        raise ValueError("Stage56 inverted QPC writer interval")
    return rows, health


def health_stats(health, tag):
    if not health:
        raise ValueError(tag + " has no HEALTH rows")
    failures = defaultdict(int)
    for entry in health:
        for key in ("dropped", "overflow", "mismatch", "readFail",
                    "missingObjects", "badCaller"):
            if key in entry:
                failures[key] += int(entry[key])
    return dict(failures)


def join(stage55, stage56):
    indexed = defaultdict(list)
    for r in stage55:
        indexed[(r["side"], r["object"], r["tid"])].append(r)
    time_index = {}
    for key, group in indexed.items():
        group.sort(key=lambda r: (r["q0"], r["q1"], r["seq"]))
        time_index[key] = ([r["q0"] for r in group], group)

    writers_by_parent = defaultdict(list)
    orphan = []
    ambiguous = []
    for w in stage56:
        key = (w["side"], w["object"], w["tid"])
        starts, group = time_index.get(key, ([], []))
        # A writer may only be inside parents whose PRE <= writer PRE.
        last = bisect_left(starts, w["q0"] + 1)
        candidates = [r for r in group[max(0, last-8):last]
                      if r["q0"] <= w["q0"] and w["q1"] <= r["q1"]]
        # If there is unusual deep nesting, check the full candidate prefix.
        if last > 8 and not candidates:
            candidates = [r for r in group[:last]
                          if r["q0"] <= w["q0"] and w["q1"] <= r["q1"]]
        if len(candidates) == 1:
            writers_by_parent[candidates[0]["seq"]].append(w)
        elif len(candidates) == 0:
            orphan.append(w["seq"])
        else:
            ambiguous.append(w["seq"])

    out = []
    for parent in sorted(stage55, key=lambda r: r["seq"]):
        hits = sorted(writers_by_parent[parent["seq"]],
                      key=lambda r: (r["q0"], r["seq"]))
        reads_ok = all(w["pre_read"] == 1 and w["post_read"] == 1 for w in hits)
        initial_ok = bool(hits and hits[0]["x0"] == parent["x0"])
        end_ok = bool(hits and hits[-1]["x1"] == parent["x1"])
        continuous = bool(hits and all(a["x1"] == b["x0"]
                                      for a, b in zip(hits, hits[1:])))
        changed = parent["x0"] != parent["x1"]
        all_explained = reads_ok and initial_ok and end_ok and continuous
        out.append({
            "stage55_seq": parent["seq"], "side": parent["side"],
            "object": f"{parent['object']:08X}", "tid": parent["tid"],
            "qpcBefore": parent["q0"], "qpcAfter": parent["q1"],
            "stage55_xBefore": f"{parent['x0']:08X}",
            "stage55_xAfter": f"{parent['x1']:08X}",
            "stage55_changed": int(changed),
            "writer_count": len(hits),
            "writer_changed_count": sum(w["x0"] != w["x1"] for w in hits),
            "writer_seq": ";".join(str(w["seq"]) for w in hits),
            "writer_sites": ";".join(f"{w['site']:08X}" for w in hits),
            "first_pre_matches": int(initial_ok),
            "last_post_matches": int(end_ok),
            "writer_chain_continuous": int(continuous),
            "writer_reads_complete": int(reads_ok),
            "writer_chain_explains_net": int(all_explained),
        })
    return out, orphan, ambiguous


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--stage55", type=Path, required=True)
    ap.add_argument("--stage56", type=Path, required=True)
    ap.add_argument("--out", type=Path, default=Path("Stage56_Runtime"))
    args = ap.parse_args()
    if args.stage55.resolve() == args.stage56.resolve():
        raise ValueError("Stage55 and Stage56 must be different files")
    a, ah = parse_stage55(args.stage55)
    b, bh = parse_stage56(args.stage56)
    ah_stats = health_stats(ah, "Stage55")
    bh_stats = health_stats(bh, "Stage56")
    out, orphan, ambiguous = join(a, b)
    changed = [r for r in out if r["stage55_changed"]]
    summary = {
        "provenance": "Same-run user-supplied Stage55 and Stage56 logs ONLY; verify capture timestamps separately",
        "stage55_acdf0_rows": len(a),
        "stage55_changed_x_rows": len(changed),
        "stage56_writer_rows": len(b),
        "stage56_writer_changed_x_rows": sum(w["x0"] != w["x1"] for w in b),
        "stage56_unmatched_writer_seq": orphan,
        "stage56_ambiguous_writer_seq": ambiguous,
        "stage55_changed_without_writer": sum(r["writer_count"] == 0 for r in changed),
        "stage55_changed_explained_by_writer_chain":
            sum(r["writer_chain_explains_net"] for r in changed),
        "stage55_changed_unexplained_by_writer_chain":
            sum(not r["writer_chain_explains_net"] for r in changed),
        "stage55_health_totals": ah_stats,
        "stage56_health_totals": bh_stats,
        "stage55_health_samples": len(ah),
        "stage56_health_samples": len(bh),
        "caution": "Matching net raw X is not proof no other store ran or that AI improved",
    }
    args.out.mkdir(parents=True, exist_ok=True)
    csv_path = args.out / "stage56_stage55_writer_join.csv"
    with csv_path.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=list(out[0].keys()))
        w.writeheader()
        w.writerows(out)
    json_path = args.out / "summary.json"
    json_path.write_text(json.dumps(summary, indent=2, ensure_ascii=False) + "\n",
                         encoding="utf-8")
    print(json.dumps({k: v for k, v in summary.items()
                      if not isinstance(v, list)}, indent=2))
    print("Wrote:", csv_path, json_path)
    if ambiguous:
        raise SystemExit("WARNING: ambiguous writer→parent windows; review CSV and timestamps")


if __name__ == "__main__":
    main()
