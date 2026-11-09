#!/usr/bin/env python3
"""Read-only Stage28 PE verifier. Never modifies AALib.exe or the game process."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct

SHA256 = "28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
SIGNATURES = {
    0x978B0: "55 8b ec 56 8b 75 08 57",
    0x97830: "57 8b f8 8d 47 10 6a 00",
    0x97870: "57 8b f8 83 a7 c8 00 00 00 f2",
    0x97970: "55 8b ec 56 8b 75 08",
    0x991E0: "55 8b ec 8b 55 08 56",
    0xF2ED0: "55 8b ec 8b 45 08 0f bf 48 08",
    0x978DC: "f6 c1 02",
    0x978E9: "83 e1 fd",
    0x978EF: "89 8e c8 00 00 00",
    0x978E1: "6a 13",
    0x97901: "6a 14",
    0x97912: "6a 15",
    0x97923: "f6 86 c4 00 00 00 01",
    0x9792C: "f6 86 cc 00 00 00 02",
    0x9793B: "83 be d4 00 00 00 00",
    0x97954: "7f df",
    0x9795E: "b0 01",
    0x97839: "c7 87 d4 00 00 00 01 00 00 00",
    0x97880: "c7 87 d4 00 00 00 02 00 00 00",
    0x97855: "81 8f c4 00 00 00 00 04 00 00",
    0x9789C: "81 8f c4 00 00 00 00 04 00 00",
    0x991F0: "2b 0a",
    0x991F2: "89 4c b2 04",
    0x991F8: "c7 44 b2 04 00 00 00 00",
    0xF2ED6: "0f bf 48 08",
    0xF2EDF: "0f bf 50 0a",
}
DIRECT_CALLS = {
    0x97609: 0x97970, 0x979CC: 0x97870, 0x979D2: 0x978B0,
    0x978CA: 0x9EEA0, 0x978F5: 0xF2ED0, 0x97906: 0xF2ED0,
    0x97917: 0xF2ED0, 0x9794A: 0x991E0, 0x97958: 0x97830,
    0x97843: 0xF7EB0, 0x9784A: 0x976D0, 0x97850: 0x99150,
    0x9788A: 0xF7EB0, 0x97891: 0x976D0, 0x97897: 0x99150,
    0x99160: 0x99040,
}
DIRECT_JUMPS = {0x979AC: 0x97870}


def pe_rva_reader(blob):
    if len(blob) < 0x40 or blob[:2] != b"MZ":
        raise ValueError("No DOS MZ header")
    pe, = struct.unpack_from("<I", blob, 0x3C)
    if pe + 24 > len(blob) or blob[pe:pe+4] != b"PE\x00\x00":
        raise ValueError("Invalid PE header")
    machine, nsec = struct.unpack_from("<HH", blob, pe+4)
    if machine != 0x14C:
        raise ValueError("Expected x86 PE machine 0x14C")
    opt_size, = struct.unpack_from("<H", blob, pe+20)
    sections = []
    for i in range(nsec):
        o = pe+24+opt_size+i*40
        if o+40 > len(blob):
            raise ValueError("Truncated section header")
        virtual, rva, rawsize, offset = struct.unpack_from("<IIII", blob, o+8)
        sections.append((rva, rva+max(virtual, rawsize), offset, rawsize))

    def read(rva, count):
        for start, end, offset, rawsize in sections:
            d = rva-start
            if start <= rva and rva+count <= end and d+count <= rawsize and offset+d+count <= len(blob):
                return blob[offset+d:offset+d+count]
        raise ValueError("Unmapped RVA 0x%X" % rva)
    return read


def rel32_target(read, address, opcode):
    b = read(address, 5)
    if b[0] != opcode:
        raise ValueError("RVA 0x%X: expected opcode 0x%02X, found 0x%02X" % (address, opcode, b[0]))
    return (address+5+struct.unpack_from("<i", b, 1)[0]) & 0xFFFFFFFF


def inspect(binary, stage26_log=None):
    blob = Path(binary).read_bytes()
    digest = hashlib.sha256(blob).hexdigest()
    if digest != SHA256:
        raise ValueError("Unexpected AALib.exe SHA256: " + digest)
    read = pe_rva_reader(blob)
    for address, pattern in SIGNATURES.items():
        expected = bytes.fromhex(pattern)
        found = read(address, len(expected))
        if expected != found:
            raise ValueError("Signature mismatch at RVA 0x%X" % address)
    for address, target in DIRECT_CALLS.items():
        if rel32_target(read, address, 0xE8) != target:
            raise ValueError("Unexpected CALL target at 0x%X" % address)
    for address, target in DIRECT_JUMPS.items():
        if rel32_target(read, address, 0xE9) != target:
            raise ValueError("Unexpected JMP target at 0x%X" % address)
    result = {
        "verified_sha256": digest, "binary_bytes": len(blob),
        "signatures_checked": len(SIGNATURES),
        "direct_calls_checked": len(DIRECT_CALLS),
        "direct_jumps_checked": len(DIRECT_JUMPS),
        "verified_calls": {"0x%X" % k: "0x%X" % v for k,v in DIRECT_CALLS.items()},
        "function_windows": {"0x%X" % a: read(a, size).hex(" ")
            for a,size in [(0x978B0,0xB3),(0x97830,0x34),(0x97870,0x3B),(0x991E0,0x23),(0xF2ED0,0x23)]},
        "interpretation": "F2ED0 compares existing type/code words (0,0x13..0x15); it does not insert an event. 97830 sets object state1. 991E0 decrements a counter and returns its previous value."
    }
    if stage26_log is not None:
        text = Path(stage26_log).read_text(encoding="utf-8-sig")
        if "STAGE26_OBJECT_BIT0_WRITE_BUILD=2026-11-09-r1" not in text:
            raise ValueError("Stage26 marker not found")
        old_new = []
        for line in text.splitlines():
            if "STAGE26_OBJECT_BIT0_WRITE seq=" not in line:
                continue
            match = re.search(r"\bold=(0x[0-9A-Fa-f]+)\s+new=(0x[0-9A-Fa-f]+)", line)
            if not match:
                raise ValueError("Stage26 old/new missing")
            old_new.append((int(match.group(1),16),int(match.group(2),16)))
        result["stage26"] = {
            "records":len(old_new),
            "full_dword_pairs":{"%08X->%08X" % (a,b):old_new.count((a,b)) for a,b in sorted(set(old_new))},
            "bit1_nonzero_samples":sum(bool((a|b)&2) for a,b in old_new)
        }
    return result


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--binary",type=Path,required=True)
    p.add_argument("--stage26-log",type=Path)
    p.add_argument("--output",type=Path)
    args=p.parse_args()
    result=inspect(args.binary,args.stage26_log)
    report=json.dumps(result,ensure_ascii=False,indent=2)+"\n"
    if args.output:
        args.output.write_text(report,encoding="utf-8")
        print("Saved:",args.output)
    print("PASS pinned PE: %d signatures, %d CALLs, %d JMPs" %
          (result["signatures_checked"],result["direct_calls_checked"],result["direct_jumps_checked"]))
    if "stage26" in result:
        print("Stage26:",result["stage26"])

if __name__ == "__main__":
    main()
