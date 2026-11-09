#!/usr/bin/env python3
"""Stage27 offline analysis of Stage26 bit0 transitions; optional pinned AALib PE inspection.

Pure Python standard library. Optional capstone improves disassembly, but is not needed
for the analysis or byte-signature checks. Do not load or patch the game process.
"""
import argparse
import collections
import hashlib
import json
import pathlib
import re
import statistics
import struct

EXPECTED_SHA256 = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
SETTER_EIP = 0x50B0C267  # Stage25 wrapper instance in the captured run, NOT portable across launches.
CLEAR_SITES = {
    0x9787A: ("0x97873: AND [EDI+0xC8], 0xFFFFFFF2", 0x97873, bytes.fromhex("83 A7 C8 00 00 00 F2")),
    0x975AC: ("0x975A6: MOV [ESI+0xC8], EAX (EAX bit0 masked)", 0x975A6, bytes.fromhex("89 86 C8 00 00 00")),
    0x95F5B: ("0x95F55: MOV [EDI+0xC8], EAX (observed EAX=0)", 0x95F55, bytes.fromhex("89 87 C8 00 00 00")),
    0x97037: ("0x97031: MOV [ESI+0xC8], EBX (observed EBX=0)", 0x97031, bytes.fromhex("89 9E C8 00 00 00")),
}
DOMINANT_CALL_RETURNS = (0x979D1, 0x9760E)


def parse_log(text):
    if "STAGE26_OBJECT_BIT0_WRITE_BUILD=2026-11-09-r1" not in text:
        raise ValueError("Missing exact Stage26 marker; refuse ambiguous or stale input")
    rows = []
    for line in text.splitlines():
        if "STAGE26_OBJECT_BIT0_WRITE seq=" not in line:
            continue
        data = dict(re.findall(r"([A-Za-z][A-Za-z0-9_]*)=([^\s]+)", line))
        for name in ("seq", "side", "qpc", "oldBit0", "newBit0", "rvaAfter", "eipAfter", "player", "field",
                     "inAALib", "shadowInitialized", "EAX", "ESI", "EDI", "EBX", "stackMask"):
            if name not in data:
                raise ValueError("Missing " + name + " in: " + line[:200])
        stack = re.search(r"stack=\[([0-9A-Fa-f,]+)\]", line)
        if not stack:
            raise ValueError("Missing stack snapshot for seq=" + data["seq"])
        data["stack"] = [int(v, 16) for v in stack.group(1).split(",")]
        data["seq"] = int(data["seq"])
        data["qpc"] = int(data["qpc"], 16)
        for k in ("oldBit0", "newBit0", "shadowInitialized", "inAALib"):
            data[k] = int(data[k])
        for k in ("rvaAfter", "eipAfter", "player", "field", "EAX", "ESI", "EDI", "EBX", "stackMask"):
            data[k] = int(data[k], 16)
        rows.append(data)
    actual = [r["seq"] for r in rows]
    if actual != list(range(1, len(rows) + 1)):
        raise ValueError("Stage26 sequence has gaps, duplicates, or out-of-order records")
    if not rows:
        raise ValueError("Stage26 marker present but no write events")
    for r in rows:
        if r["field"] != r["player"] + 0xF8:
            raise ValueError("Field/player alias mismatch at seq=" + str(r["seq"]))
    return rows


def analyze(rows):
    open_by_side = {}
    cycles = []
    anomalies = []
    all_sides = collections.Counter()
    setter_payloads = collections.Counter()
    caller_groups = collections.Counter()
    by_site = collections.Counter()
    game_bases = set()
    for r in rows:
        side = r["side"]
        all_sides[side] += 1
        if r["inAALib"]:
            game_bases.add(r["eipAfter"] - r["rvaAfter"])
    if len(game_bases) != 1:
        raise ValueError("Inconsistent module bases from runtime EIP/RVA pairs: " + str(game_bases))
    base = game_bases.pop()
    setter_count = 0
    for r in rows:
        side = r["side"]
        old, new = r["oldBit0"], r["newBit0"]
        if not r["shadowInitialized"]:
            anomalies.append("Uninitialized field shadow at seq=" + str(r["seq"]))
        if old == 0 and new == 1:
            setter_count += 1
            setter_payloads["0x%X" % r["EBX"]] += 1
            if side in open_by_side:
                anomalies.append("Overlapping setter at seq=" + str(r["seq"]))
            open_by_side[side] = r
            if r["EAX"] != r["player"]:
                anomalies.append("Setter EAX not target player at seq=" + str(r["seq"]))
            if r["eipAfter"] != SETTER_EIP and not r["inAALib"]:
                anomalies.append("Unknown DLL setter EIP at seq=" + str(r["seq"]))
        elif old == 1 and new == 0:
            site = r["rvaAfter"] if r["inAALib"] else None
            by_site["0x%05X" % site if site is not None else "DLL"] += 1
            begin = open_by_side.pop(side, None)
            if begin is None:
                anomalies.append("Unpaired clear at seq=" + str(r["seq"]))
                continue
            if r["qpc"] < begin["qpc"]:
                anomalies.append("Negative QPC interval at seq=" + str(r["seq"]))
            caller_rva = None
            if site == 0x9787A:
                if len(r["stack"]) >= 2 and (r["stackMask"] & 2):
                    caller_rva = r["stack"][1] - base
                    caller_groups["0x%05X" % caller_rva] += 1
                if r["EDI"] != r["player"] + 0x30:
                    anomalies.append("Unexpected primary-clear EDI alias seq=" + str(r["seq"]))
            elif site == 0x975AC and r["ESI"] != r["player"] + 0x30:
                anomalies.append("Unexpected 975AC ESI alias seq=" + str(r["seq"]))
            elif site == 0x95F5B and (r["EDI"] != r["player"] + 0x30 or r["EAX"] != 0):
                anomalies.append("Unexpected 95F5B register values seq=" + str(r["seq"]))
            elif site == 0x97037 and (r["ESI"] != r["player"] + 0x30 or r["EBX"] != 0):
                anomalies.append("Unexpected 97037 register values seq=" + str(r["seq"]))
            cycles.append({
                "side": side, "set_seq": begin["seq"], "clear_seq": r["seq"],
                "set_qpc": begin["qpc"], "clear_qpc": r["qpc"],
                "duration_qpc_ticks": r["qpc"] - begin["qpc"],
                "clear_rva_after": site, "clear_caller_rva_inferred": caller_rva,
                "set_ebx": begin["EBX"], "set_edi": begin["EDI"],
            })
        else:
            # For Stage26TransitionsOnly=0, no-change writes are expected.
            pass
    for side, r in open_by_side.items():
        anomalies.append("Unclosed setter side=%s seq=%s" % (side, r["seq"]))
    per_site = {}
    for site, count in sorted(by_site.items()):
        intervals = [x["duration_qpc_ticks"] for x in cycles
                     if ("0x%05X" % x["clear_rva_after"] if x["clear_rva_after"] is not None else "DLL") == site]
        per_site[site] = {
            "clears": count, "median_duration_qpc": int(statistics.median(intervals)) if intervals else None,
            "min_duration_qpc": min(intervals) if intervals else None,
            "max_duration_qpc": max(intervals) if intervals else None,
            "sides": dict(collections.Counter(x["side"] for x in cycles
                          if ("0x%05X" % x["clear_rva_after"] if x["clear_rva_after"] is not None else "DLL") == site)),
        }
    return {
        "events": len(rows), "sets": setter_count, "clears": sum(by_site.values()),
        "pairs": len(cycles), "field_updates_by_side": dict(all_sides),
        "runtime_game_base": "0x%08X" % base,
        "clear_sites": per_site, "dominant_clear_caller_returns": dict(caller_groups),
        "set_ebx_distribution": dict(sorted(setter_payloads.items())),
        "anomalies": anomalies, "cycles": cycles,
    }


def pe_rva_mapper(blob):
    if blob[:2] != b"MZ" or len(blob) < 0x40:
        raise ValueError("Not a PE file (missing MZ)")
    pe = struct.unpack_from("<I", blob, 0x3C)[0]
    if blob[pe:pe + 4] != b"PE\x00\x00":
        raise ValueError("Not a PE file (missing PE header)")
    count = struct.unpack_from("<H", blob, pe + 6)[0]
    optional_size = struct.unpack_from("<H", blob, pe + 20)[0]
    table = pe + 24 + optional_size
    sections = []
    for i in range(count):
        off = table + i * 40
        if off + 40 > len(blob):
            raise ValueError("Truncated section header")
        virtual_size, rva, raw_size, raw_start = struct.unpack_from("<IIII", blob, off + 8)
        sections.append((rva, rva + max(virtual_size, raw_size), raw_start, raw_size))
    def view(rva, length):
        for start, end, raw, raw_size in sections:
            rel = rva - start
            if start <= rva < end and rel >= 0 and rel + length <= raw_size and raw + rel + length <= len(blob):
                return blob[raw + rel: raw + rel + length]
        raise ValueError("RVA 0x%X length %s not backed by file bytes" % (rva, length))
    return view


def inspect_binary(path):
    blob = pathlib.Path(path).read_bytes()
    digest = hashlib.sha256(blob).hexdigest()
    if digest.lower() != EXPECTED_SHA256:
        raise ValueError("AALib.exe SHA256 mismatch; refusing to mix instructions from a different build: " + digest)
    read = pe_rva_mapper(blob)
    out = ["Exact AALib.exe SHA-256 confirmed: " + digest]
    for after, (name, start, expected) in sorted(CLEAR_SITES.items()):
        actual = read(start, len(expected))
        if actual != expected:
            raise ValueError("Clear signature mismatch at RVA 0x%05X: %s" % (start, actual.hex()))
        out.append("MATCH %s (post-EIP RVA 0x%05X)" % (name, after))
        low = start - 32
        snippet = read(low, 128)
        out.append("RVA 0x%05X window: %s" % (low, snippet.hex(" ")))
    helper = read(0x97870, 10)
    if helper != bytes.fromhex("57 8B F8 83 A7 C8 00 00 00 F2"):
        raise ValueError("0x97870 helper prologue mismatch")
    out.append("MATCH helper entry 0x97870: PUSH EDI; MOV EDI,EAX; AND [EDI+0xC8],0xFFFFFFF2")
    for caller_return in DOMINANT_CALL_RETURNS:
        call_site = caller_return - 5
        b = read(call_site, 5)
        if b[0] == 0xE8:
            destination = (caller_return + struct.unpack_from("<i", b, 1)[0]) & 0xFFFFFFFF
            out.append("Callsite 0x%05X E8 rel32 target RVA 0x%05X (return 0x%05X)" %
                       (call_site, destination, caller_return))
            if destination != 0x97870:
                out.append("WARNING: direct call destination differs from primary helper entry")
        else:
            out.append("Callsite 0x%05X is not E8 rel32: bytes=%s; indirect/other call requires disassembly" %
                       (call_site, b.hex(" ")))
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    except ImportError:
        out.append("OPTIONAL: install capstone to emit instruction disassembly. Byte/signature checks above still pass.")
    else:
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        for start in [0x97870, 0x975A2, 0x95F4E, 0x97028, 0x97609, 0x979CC]:
            out.append("Disassembly from RVA 0x%05X (alignment at caller addresses not verified):" % start)
            for ins in decoder.disasm(read(start, 80), start):
                out.append("  %08X  %-7s %s" % (ins.address, ins.mnemonic, ins.op_str))
    return "\n".join(out) + "\n"


def markdown_report(report):
    x = ["# Stage27 — offline bit0 lifecycle report", "",
         "Source is Stage26's captured runtime records; not a new gameplay run.",
         "",
         "Events: %s; sets: %s; clears: %s; paired cycles: %s." %
         (report["events"], report["sets"], report["clears"], report["pairs"]),
         "Runtime game base: " + report["runtime_game_base"], "",
         "## Clear sites", "", "| Post-write RVA | Count | Median QPC ticks | P1 | P2 |",
         "| --- | ---: | ---: | ---: | ---: |"]
    for site, item in sorted(report["clear_sites"].items()):
        x.append("| %s | %d | %s | %d | %d |" %
                 (site, item["clears"], item["median_duration_qpc"],
                  item["sides"].get("P1", 0), item["sides"].get("P2", 0)))
    x.extend(["", "## Dynamic caller-return candidates for helper +0x97870", ""])
    for k, count in sorted(report["dominant_clear_caller_returns"].items()):
        x.append("- Return RVA %s: %d observations (not yet a proven static CALL xref)." % (k, count))
    x.extend(["", "## Unresolved / integrity anomalies", ""])
    x.extend("- " + y for y in report["anomalies"])
    if not report["anomalies"]:
        x.append("None observed in the captured Stage26 log.")
    x.extend(["", "## Caution", "",
              "EIP/RVA is post-write. Caller address is inferred from stack[1] ONLY for the verified",
              "PUSH EDI helper prologue. A full static parent-call graph requires the exact pinned PE.",
              "Shadow old values and per-thread coverage are observational, not atomic completeness proofs.", ""])
    return "\n".join(x)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--log", required=True, type=pathlib.Path)
    p.add_argument("--binary", type=pathlib.Path, help="optional exact SHA256-pinned AALib.exe")
    p.add_argument("--json", type=pathlib.Path, help="write full JSON with paired cycles")
    p.add_argument("--markdown", type=pathlib.Path, help="write readable summary")
    p.add_argument("--static-report", type=pathlib.Path, help="write optional binary signature/caller diagnostics")
    args = p.parse_args()
    report = analyze(parse_log(args.log.read_text(encoding="utf-8-sig")))
    summary = markdown_report(report)
    print(summary)
    if args.json:
        args.json.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    if args.markdown:
        args.markdown.write_text(summary, encoding="utf-8")
    if args.binary:
        static_text = inspect_binary(args.binary)
        print(static_text)
        if args.static_report:
            args.static_report.write_text(static_text, encoding="utf-8")
    if report["anomalies"]:
        raise SystemExit("WARNING: Stage27 analysis completed with record-level anomalies; inspect output")
if __name__ == "__main__":
    main()
