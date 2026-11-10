#!/usr/bin/env python3
"""Stage62 passive, offline native-log join. Python 3 stdlib only; no game access."""
import argparse
import json
import re
from datetime import datetime
from pathlib import Path

FILES = {
    "37": ("ArcanaAI_Stage37Type1CommitTrace.log", "STAGE37_TYPE1_EFFECT"),
    "39": ("ArcanaAI_Stage39ActionOutputTrace.log", "STAGE39_OUTPUT_WINDOW"),
    "40": ("ArcanaAI_Stage40ActionConsumerTrace.log", "STAGE40_CONSUMER_WINDOW"),
    "41": ("ArcanaAI_Stage41ObjectCommitTrace.log", "STAGE41_OBJECT_WINDOW"),
    "42": ("ArcanaAI_Stage42ActionActivationTrace.log", "STAGE42_ACTIVATION_WINDOW"),
    "43": ("ArcanaAI_Stage43ActivationOriginTrace.log", "STAGE43_ORIGIN_WINDOW"),
    "44": ("ArcanaAI_Stage44ActionDefinitionTrace.log", "STAGE44_BINDING_WINDOW"),
    "46": ("ArcanaAI_Stage46DefinitionMotionTrace.log", "STAGE46_MOTION_WINDOW"),
}
KV = re.compile(r"([A-Za-z]\w*)=([^\s]+)")
STAMP = re.compile(r"^\[(\d{4}-\d\d-\d\d \d\d:\d\d:\d\d)\]")


def hx(value):
    return int(value, 16)


def q(value):
    return int(value)


def before(value):
    return value.split(">", 1)[0]


def after(value):
    return value.split(">", 1)[1]


def actor(a, b):
    return a["side"] == b["side"] and hx(a["player"]) == hx(b["player"]) and a["tid"] == b["tid"]


def load(base, name, kind):
    p = base / name
    if not p.is_file():
        raise ValueError("Missing " + str(p))
    result = []
    start = None
    for line in p.read_text(encoding="utf-8-sig", errors="replace").splitlines():
        if start is None and (m := STAMP.match(line)):
            start = datetime.strptime(m.group(1), "%Y-%m-%d %H:%M:%S")
        if "] " + kind + " " in line:
            item = dict(KV.findall(line))
            if "side" not in item or "tid" not in item or "player" not in item:
                raise ValueError(name + ": malformed " + kind)
            result.append(item)
    if start is None or not result:
        raise ValueError(name + ": empty log or no observed " + kind)
    return result, start


def one(items, reason):
    if len(items) != 1:
        raise ValueError(reason + ": expected exactly one, found " + str(len(items)))
    return items[0]


def run(directory):
    data = {}
    timestamps = {}
    for k, (filename, kind) in FILES.items():
        data[k], timestamps[k] = load(directory, filename, kind)
    # Fail closed on obvious mixed sessions. Same process/actor/QPC checks below
    # additionally protect against coincident IDs in unrelated traces.
    span = (max(timestamps.values()) - min(timestamps.values())).total_seconds()
    if span > 15:
        raise ValueError("Different log start times: " + str(span) + "s")
    e, f, c, b, a, g, d, m = [data[k] for k in ("37", "39", "40", "41", "42", "43", "44", "46")]
    by41 = {x["stage40Seq"]: x for x in b}
    by43 = {x["stage42Seq"]: x for x in g}
    by46 = {x["stage44Seq"]: x for x in m}
    if len(by41) != len(b) or len(by43) != len(g) or len(by46) != len(m):
        raise ValueError("Duplicate cross-observer sequence ID")
    for kind, items, seqname in (("40", c, "seq"), ("42", a, "seq"),
                                 ("44", d, "seq"), ("46", m, "stage44Seq")):
        seqs = sorted(int(x[seqname]) for x in items)
        if seqs != list(range(1, len(seqs) + 1)):
            raise ValueError(kind + ": noncontiguous observation sequence")

    activation_links = {}
    used_c = set()
    for active in a:
        prior = [x for x in c if actor(x, active) and q(x["qpcAfter"]) <= q(active["qpcBefore"])]
        if not prior:
            raise ValueError("No preceding Stage40: Stage42 " + active["seq"])
        request = max(prior, key=lambda x: q(x["qpcAfter"]))
        pending = one([by41.get(request["seq"])] if request["seq"] in by41 else [],
                      "Missing Stage41 for " + request["seq"])
        origin = one([by43.get(active["seq"])] if active["seq"] in by43 else [],
                     "Missing Stage43 for " + active["seq"])
        definitions = [x for x in d if actor(x, active)
                       and hx(x["object"]) == hx(active["object"])
                       and q(active["qpcBefore"]) <= q(x["qpcBefore"])
                       and q(x["qpcAfter"]) <= q(active["qpcAfter"])]
        bind = one(definitions, "Ambiguous Stage44 in Stage42 " + active["seq"])
        motion = one([by46.get(bind["seq"])] if bind["seq"] in by46 else [],
                     "Missing Stage46 " + bind["seq"])
        ID = hx(after(active["o2"]))
        if not (hx(request["requestAtSelector"]) == hx(pending["request"]) ==
                hx(after(pending["o3"])) == hx(origin["activeID"]) ==
                hx(origin["pendingID"]) == hx(after(bind["f0"])) ==
                hx(after(motion["f0"])) == ID):
            raise ValueError("Action ID mismatch at activation " + active["seq"])
        if not (actor(request, pending) and actor(request, origin) and
                actor(request, bind) and actor(request, motion)):
            raise ValueError("Actor mismatch at activation " + active["seq"])
        if not (pending["qpcBefore"] == request["qpcBefore"] and
                pending["qpcAfter"] == request["qpcAfter"] and
                origin["qpcBefore"] == active["qpcBefore"] and
                origin["qpcAfter"] == active["qpcAfter"] and
                origin["callerRVA"].upper() == "000AD75C" and
                motion["bindingQpcBefore"] == bind["qpcBefore"] and
                motion["qpcAfter"] == bind["qpcAfter"] and
                hx(motion["definitionAfter"]) == hx(after(bind["f1"]))):
            raise ValueError("Provenance/QPC/definition mismatch at " + active["seq"])
        if not (int(request["preMask"], 16) == int(request["postMask"], 16) == 255
                and int(active["preMask"], 16) == int(active["postMask"], 16) == 31
                and int(bind["preMask"], 16) == int(bind["postMask"], 16) == 255
                and int(motion["preMask"], 16) == int(motion["postMask"], 16) == 8191):
            raise ValueError("Incomplete read mask in activation " + active["seq"])
        if request["seq"] in used_c:
            raise ValueError("One request linked to multiple activations: " + request["seq"])
        used_c.add(request["seq"])
        activation_links[request["seq"]] = active["seq"]

    # Every Stage40 event has the direct same-interval Stage41 snapshot,
    # even when it was only a duplicate request and did not activate.
    for request in c:
        pending = by41.get(request["seq"])
        if pending is None or not actor(request, pending):
            raise ValueError("Stage41 absent or actor mismatch: " + request["seq"])
        if (pending["qpcBefore"] != request["qpcBefore"] or
            pending["qpcAfter"] != request["qpcAfter"] or
            hx(pending["request"]) != hx(request["requestAtSelector"])):
            raise ValueError("Stage40/41 disagreement: " + request["seq"])

    chains = []
    used_e = set()
    for output in f:
        prior_events = [x for x in e if actor(x, output) and q(x["qpcAfter"]) <= q(output["qpcBefore"])]
        if not prior_events:
            raise ValueError("No Stage37 before Stage39 " + output["stage38Seq"])
        event = max(prior_events, key=lambda x: q(x["qpcAfter"]))
        subsequent = sorted((x for x in c if actor(x, output) and
                             q(x["qpcBefore"]) >= q(output["qpcAfter"])),
                            key=lambda x: q(x["qpcBefore"]))
        if not subsequent:
            raise ValueError("No Stage40 after Stage39 " + output["stage38Seq"])
        request = subsequent[0]
        id_value = hx(after(output["a0"]))
        if (hx(output["payload"]) != hx(event["payload"]) or
            id_value != hx(request["requestAtSelector"]) or
            request["seq"] not in activation_links):
            raise ValueError("37->39->40->42 mismatch: " + output["stage38Seq"])
        if event["dispatchSeq"] in used_e:
            raise ValueError("Stage37 reused: " + event["dispatchSeq"])
        used_e.add(event["dispatchSeq"])
        chains.append({
            "stage37": int(event["dispatchSeq"]), "stage38_callback": int(output["stage38Seq"]),
            "stage40": int(request["seq"]), "stage42": int(activation_links[request["seq"]]),
            "side": output["side"], "payload": output["payload"],
            "action_id": f"0x{id_value:08X}",
            "qpc37_to39": q(output["qpcBefore"]) - q(event["qpcAfter"]),
            "qpc39_to40": q(request["qpcBefore"]) - q(output["qpcAfter"]),
        })

    duplicate_requests = [x for x in c if x["seq"] not in used_c]
    duplicate_ids = sorted({x["requestAtSelector"] for x in duplicate_requests})
    if len(chains) != len(f):
        raise ValueError("Missing Stage39 chain")
    return {
        "session_start": min(timestamps.values()).isoformat(sep=" "),
        "counts": {k: len(data[k]) for k in FILES},
        "stage40_to41": len(c),
        "stage40_to42_to43_to44_to46": len(activation_links),
        "stage37_to39_to40_to42_to44_to46": len(chains),
        "stage40_without_separate_activation": len(duplicate_requests),
        "unmatched_stage40_request_ids": duplicate_ids,
        "stage37_without_stage39": [int(x["dispatchSeq"]) for x in e
                                      if x["dispatchSeq"] not in used_e],
        "chains": chains,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dir", type=Path, required=True, help="Folder with same-run raw logs")
    parser.add_argument("--json", type=Path, help="Optional output report JSON")
    args = parser.parse_args()
    result = run(args.dir)
    print(json.dumps({k: v for k, v in result.items() if k != "chains"},
                     indent=2, ensure_ascii=False))
    if args.json:
        args.json.write_text(json.dumps(result, indent=2, ensure_ascii=False) + "\n",
                             encoding="utf-8")
        print("Wrote", args.json)


if __name__ == "__main__":
    main()
