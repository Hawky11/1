#!/usr/bin/env python3
"""Read-only, same-run Stage46 -> Stage47 -> Stage59 motion evidence join."""
import argparse
import bisect
import collections
import json
import re
import sys
from pathlib import Path

EVENTS = {
    "46": "STAGE46_MOTION_WINDOW ",
    "47": "STAGE47_CONSUMER_WINDOW ",
    "59": "STAGE59_X_WRITE ",
}
KEY_VALUE = re.compile(r"([A-Za-z][A-Za-z0-9_]*)=([^\s]+)")


def read_events(path, stage):
    events = []
    with path.open("r", encoding="utf-8-sig", errors="replace") as stream:
        for line_number, line in enumerate(stream, 1):
            if EVENTS[stage] not in line:
                continue
            v = dict(KEY_VALUE.findall(line))
            try:
                r = {
                    "side": v["side"],
                    "tid": int(v["tid"]),
                    "object": int(v["object"], 16) if stage != "59"
                              else int(v["watched"], 16) - 0x7C,
                    "qpc_after": int(v["qpcAfter"]),
                    "line": line_number,
                }
                if stage == "46":
                    r.update(qpc_before=int(v["bindingQpcBefore"]),
                             definition=int(v["definitionAfter"], 16),
                             mode=(int(v["meta30"], 16) & 255),
                             active=int(v["f0"].split(">")[-1], 16))
                elif stage == "47":
                    r.update(qpc_before=int(v["qpcBefore"]),
                             definition=int(v["def"].split(">")[0], 16),
                             active=int(v["f0"].split(">")[0], 16),
                             x_before=int(v["f5"].split(">")[0], 16),
                             x_after=int(v["f5"].split(">")[1], 16),
                             call_rva=v["callRVA"])
                else:
                    r.update(rva=v["RVA_after"].upper().removeprefix("0X"),
                             new_raw=int(v["newRaw"], 16),
                             old_shadow=int(v["oldShadow"], 16),
                             watched=int(v["watched"], 16))
                events.append(r)
            except (KeyError, ValueError, IndexError) as error:
                raise ValueError(
                    f"Stage{stage} malformed event at {path}:{line_number}: {error}"
                ) from error
    if not events:
        raise ValueError(
            f"Stage{stage}: no {EVENTS[stage].strip()} records in {path}"
        )
    return events


def actor(e):
    return e["side"], e["object"], e["tid"]


def qpc_bounds(rows):
    return min(x["qpc_after"] for x in rows), max(x["qpc_after"] for x in rows)


def correlate(p46, p47, p59):
    a, b, c = (read_events(p46, "46"), read_events(p47, "47"),
               read_events(p59, "59"))
    actor46 = {actor(x) for x in a}
    actor47 = {actor(x) for x in b}
    actor59 = {actor(x) for x in c}
    low47 = min(x["qpc_before"] for x in b)
    high47 = max(x["qpc_after"] for x in b)
    low59, high59 = qpc_bounds(c)
    if not actor46.intersection(actor47) or not actor47.intersection(actor59):
        raise ValueError(
            "NO_COMPARABLE_ACTOR: Stage46/47/59 share no complete "
            "side+object+TID actor key; never mix sessions"
        )
    if low47 > high59 or low59 > high47:
        raise ValueError(
            "DISJOINT_QPC: Stage47 windows and Stage59 writes do not overlap; "
            "do not join separate sessions"
        )

    by46 = collections.defaultdict(list)
    by47 = collections.defaultdict(list)
    for x in a:
        by46[actor(x)].append(x)
    for x in b:
        by47[actor(x)].append(x)
    for k in by46:
        by46[k].sort(key=lambda x: x["qpc_after"])
    for k in by47:
        by47[k].sort(key=lambda x: x["qpc_before"])
    q46 = {k: [x["qpc_after"] for x in xs] for k, xs in by46.items()}
    q47 = {k: [x["qpc_before"] for x in xs] for k, xs in by47.items()}
    by_window = collections.defaultdict(list)
    count = collections.Counter()

    for e in c:
        if e["rva"] != "001E70DE":
            continue  # Original 1E70DB store inside native 1E66E0 only.
        count["59_1e70de_events"] += 1
        k = actor(e)
        windows = by47.get(k, [])
        j = bisect.bisect_right(q47.get(k, []), e["qpc_after"]) - 1
        if j < 0:
            count["59_outside_47"] += 1
            continue
        w = windows[j]
        if not (w["qpc_before"] <= e["qpc_after"] <= w["qpc_after"]):
            count["59_outside_47"] += 1
            continue
        by_window[(k, w["line"])].append(e)
        count["59_nested_47"] += 1

    modes = collections.Counter()
    action_ids = collections.Counter()
    examples = []
    for w in b:
        k = actor(w)
        samples = by_window.get((k, w["line"]), [])
        if not samples:
            continue
        count["47_with_59"] += 1
        if w["x_before"] != w["x_after"]:
            count["47_with_59_net_x_changed"] += 1
        samples.sort(key=lambda x: x["qpc_after"])
        if samples[-1]["new_raw"] == w["x_after"]:
            count["last_59_matches_47_post_x"] += 1
        if samples[0]["old_shadow"] == w["x_before"]:
            count["first_59_shadow_matches_47_pre_x"] += 1
        candidates = by46.get(k, [])
        pos = bisect.bisect_right(
            q46.get(k, []), w["qpc_before"]
        ) - 1
        if pos < 0:
            count["no_prior_46"] += 1
            continue
        src = candidates[pos]
        if src["definition"] != w["definition"]:
            count["prior_46_definition_mismatch"] += 1
            continue
        count["triple_46_47_59"] += 1
        if src["active"] == w["active"]:
            count["triple_active_id_agrees"] += 1
        else:
            count["triple_active_id_differs"] += 1
        if w["x_before"] != w["x_after"]:
            count["triple_net_x_changed"] += 1
        modes[src["mode"]] += 1
        action_ids[f"0x{src['active']:08X}"] += 1
        if len(examples) < 5:
            examples.append({
                "side": k[0], "object": f"0x{k[1]:08X}", "tid": k[2],
                "definition": f"0x{src['definition']:08X}",
                "active_id": f"0x{src['active']:08X}",
                "mode": src["mode"],
                "qpc_gap": w["qpc_before"] - src["qpc_after"],
                "stage46_line": src["line"], "stage47_line": w["line"],
                "writes": len(samples),
                "raw_x_changed": w["x_before"] != w["x_after"],
            })

    return {
        "status": "SAME_RUN_CANDIDATE_NOT_AI_CAUSALITY",
        "raw_counts": {
            "stage46": len(a), "stage47": len(b), "stage59": len(c)
        },
        "qpc_ranges": {
            "stage46": qpc_bounds(a),
            "stage47": (low47, high47),
            "stage59": qpc_bounds(c),
        },
        "counts": dict(sorted(count.items())),
        "triple_definition_modes": {
            str(k): v for k, v in sorted(modes.items())
        },
        "triple_active_ids_top10": action_ids.most_common(10),
        "examples": examples,
        "limits": (
            "Latest earlier same-actor Stage46 binding (matching definition) "
            "-> Stage47 native window -> nested Stage59 post-write event. "
            "Temporal/structural evidence, NOT proof of COM decision causality "
            "or improved AI. oldShadow is approximate. Stage47 instruments "
            "only two of six native 1E66E0 callsites."
        ),
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("stage46", type=Path)
    parser.add_argument("stage47", type=Path)
    parser.add_argument("stage59", type=Path)
    parser.add_argument("--output", type=Path, help="Optional JSON report path")
    args = parser.parse_args()
    try:
        result = correlate(args.stage46, args.stage47, args.stage59)
    except (OSError, ValueError) as error:
        print("REJECTED:", error, file=sys.stderr)
        sys.exit(2)
    data = json.dumps(result, ensure_ascii=False, indent=2)
    if args.output:
        args.output.write_text(data + "\n", encoding="utf-8")
    print(data)


if __name__ == "__main__":
    main()
