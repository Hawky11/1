#!/usr/bin/env python3
"""Stage68 original-binary and existing-log verifier. Stdlib only.

Usage: py -3 verify_stage68.py AALib.exe [--logs-root CHECKOUT]
The O+68 positive-ID writer is NOT identified by this verifier.
"""
import argparse
import hashlib
import json
import re
import struct
from pathlib import Path

EXPECTED_SHA = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
# Exact original AALib.exe opcode signatures (RVA, bytes, purpose).
SITES = [
    (0x1E8DC8, "8b f1", "update binds ESI=ECX"),
    (0x1E8FDF, "8d 50 01", "threshold +1"),
    (0x1E8FE2, "3b 55 f8", "threshold compare"),
    (0x1E8FE5, "7c 0a", "conditional jump"),
    (0x1E8FE7, "c6 45 fe 01", "local flag"),
    (0x1E8FEB, "c6 46 30 01", "STATIC CANDIDATE O+30=1"),
    (0x0AE59A, "8b ce", "update ECX=ESI"),
    (0x0AE59C, "e8 1f a8 13 00", "update CALL 1E8DC0"),
    (0x0AD75A, "8b ce", "promotion ECX=ESI"),
    (0x0AD75C, "e8 df c7 13 00", "promotion CALL 1E9F40"),
    (0x1E9F40, "80 79 30 00", "check O+30"),
    (0x1E9F46, "8b 41 68", "read deferred O+68"),
    (0x1E9F4E, "89 41 64", "copy to pending O+64"),
    (0x1E9F51, "c7 41 68 ff ff ff ff", "clear deferred O+68"),
    (0x1E9F11, "c7 41 68 ff ff ff ff", "separate clear"),
    (0x0EDC41, "c7 40 68 ff ff ff ff", "player-linked clear"),
    (0x1904EE, "c7 46 68 ff ff ff ff", "separate clear"),
    (0x1E55F9, "89 7e 68", "init O+68"),
]
KEYS = re.compile(r"([A-Za-z][A-Za-z0-9]*)=([^\s]+)")


def check_exe(path):
    b = path.read_bytes()
    sha = hashlib.sha256(b).hexdigest()
    if sha != EXPECTED_SHA:
        raise ValueError("AALib.exe SHA mismatch: " + sha)
    if b[:2] != b"MZ":
        raise ValueError("bad DOS header")
    pe = struct.unpack_from("<I", b, 0x3c)[0]
    if b[pe:pe+4] != b"PE\x00\x00":
        raise ValueError("bad PE header")
    machine, num_sections = struct.unpack_from("<HH", b, pe+4)
    size_opt = struct.unpack_from("<H", b, pe+20)[0]
    opt = pe+24
    magic = struct.unpack_from("<H", b, opt)[0]
    imagebase = struct.unpack_from("<I", b, opt+28)[0]
    if (machine, magic, imagebase) != (0x14c, 0x10b, 0x400000):
        raise ValueError("not the expected PE32 i386 executable")
    sections = []
    for i in range(num_sections):
        ptr = opt+size_opt+40*i
        virtual_size, rva, rawsize, rawptr = struct.unpack_from("<IIII", b, ptr+8)
        sections.append((rva, rawsize, rawptr))
    checked = []
    for rva, opcodes, note in SITES:
        desired = bytes.fromhex(opcodes)
        offsets = [rawptr+rva-start for start, sz, rawptr in sections
                   if start <= rva and rva+len(desired) <= start+sz]
        if len(offsets) != 1:
            raise ValueError("RVA not uniquely file-backed: %X" % rva)
        actual = b[offsets[0]:offsets[0]+len(desired)]
        if actual != desired:
            raise ValueError("RVA %X: expected %s; found %s" %
                             (rva, desired.hex(), actual.hex()))
        checked.append({"rva": "%06X" % rva, "purpose": note})
    return {"sha256": sha, "siteCount": len(checked), "sites": checked}


def events(path, marker):
    lines = path.read_text(encoding="utf-8-sig").splitlines()
    if not lines or not lines[0].startswith("[2026-11-11 01:30:28]"):
        raise ValueError("unexpected initialization: " + path.name)
    records = [dict(KEYS.findall(line)) for line in lines if marker in line]
    if not records:
        raise ValueError("missing evidence rows: " + marker)
    return records


def change(record, field):
    return tuple(int(x, 16) for x in record[field].split(">"))


def check_existing_logs(root):
    stage67path = root/"ArcanaAI_Stage67NativePathsTrace.log"
    stage41path = root/"ArcanaAI_Stage41ObjectCommitTrace.log"
    rows = events(stage67path, "STAGE67_NATIVE_WINDOW")
    rows41 = events(stage41path, "STAGE41_OBJECT_WINDOW")
    if len(rows) != 4586 or len(rows41) != 280:
        raise ValueError("unexpected capture size")
    if sorted(int(x["seq"]) for x in rows) != list(range(1, 4587)):
        raise ValueError("missing/duplicate Stage67 sequence")
    if sorted(int(x["stage40Seq"]) for x in rows41) != list(range(1, 281)):
        raise ValueError("missing/duplicate Stage41 sequence")
    if any(x.get("preMask") != "1F" or x.get("postMask") != "1F"
           for x in rows if x.get("kind") == "0"):
        raise ValueError("incomplete Stage67 promotion masks")
    s = {int(x["seq"]): x for x in rows}
    t = {int(x["stage40Seq"]): x for x in rows41}
    prior, first, before_arm, arm = [s[i] for i in (4412, 4414, 4434, 4436)]
    commit = t[278]
    for x in (prior, first, before_arm, arm):
        if (x["kind"], x["side"], x["object"], x["tid"]) != ("0", "P1", "2A393020", "10788"):
            raise ValueError("different P1 actor/object/thread")
    if (commit["side"], commit["objectBefore"], commit["objectAfter"]) != ("P1", "2A393020", "2A393020"):
        raise ValueError("different Stage41 object")
    if not (change(prior, "v4") == (0xFFFFFFFF, 0xFFFFFFFF)
            and change(commit, "o4") == (0x25, 0x25)
            and change(first, "v4") == (0x25, 0x25)
            and change(before_arm, "v4") == (0x25, 0x25)
            and change(arm, "v4") == (0x25, 0xFFFFFFFF)):
        raise ValueError("deferred value mismatch")
    if not (change(first, "v1") == (0, 0)
            and change(before_arm, "v1") == (0, 0)
            and change(arm, "v1") == (1, 0)):
        raise ValueError("arming gate mismatch")
    p2 = s[3465]
    if (p2["side"], p2["object"], change(p2, "v4")) != ("P2", "2A393C50", (0x26, 0x26)):
        raise ValueError("P2 deferred ID26 mismatch")
    low, high = int(prior["qpcAfter"]), int(commit["qpcBefore"])
    arm_low, arm_high = int(before_arm["qpcAfter"]), int(arm["qpcBefore"])
    if not (low < high < int(first["qpcBefore"]) and arm_low < arm_high):
        raise ValueError("QPC ordering mismatch")
    return {
        "stage67Rows": len(rows), "stage41Rows": len(rows41),
        "deferredID25": {"afterQpc": low, "atOrBeforeQpc": high, "writerIP": "UNKNOWN"},
        "arming": {"afterQpc": arm_low, "atOrBeforeQpc": arm_high,
                   "writerIP": "NOT OBSERVED; static candidate 1E8FEB"},
        "promotionSeq": 4436, "separateP2DeferredID26Seq": 3465
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_exe", type=Path)
    parser.add_argument("--logs-root", type=Path, default=None)
    args = parser.parse_args()
    result = {"static": check_exe(args.original_exe)}
    if args.logs_root is not None:
        result["existingRuntime"] = check_existing_logs(args.logs_root)
    print(json.dumps(result, ensure_ascii=False, indent=2))
    print("PASS. O+30 native writer is a STATIC CANDIDATE. O+68=25 writer IP is UNKNOWN.")


if __name__ == "__main__":
    main()
