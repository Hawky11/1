#!/usr/bin/env python3
"""Inspect verified AALib.exe x86 call flow, especially 0x97609 -> 0x97970 -> 0x97870.

Read-only, offline, stdlib only. Does not patch or load the executable.
"""
import argparse
import hashlib
from pathlib import Path
import struct

SHA256 = '28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
CALLS = {0x97609: 0x97970, 0x979CC: 0x97870}
ENTRY_BYTES = bytes.fromhex('57 8B F8 83 A7 C8 00 00 00 F2')
WINDOWS = (
    ('upstream around caller +0x97609', 0x975D0, 0x80),
    ('entry/cross-branches +0x97970', 0x97940, 0xC0),
    ('main clearer +0x97870', 0x97850, 0x90),
    ('secondary clear +0x975A6', 0x97570, 0x80),
    ('secondary clear +0x95F55', 0x95F30, 0x70),
    ('reset clear +0x97031', 0x97010, 0x70),
)


def map_pe(blob):
    if len(blob) < 0x40 or blob[:2] != b'MZ':
        raise ValueError('Missing DOS header')
    pe = struct.unpack_from('<I', blob, 0x3C)[0]
    if pe + 24 > len(blob) or blob[pe:pe + 4] != b'PE\0\0':
        raise ValueError('Missing or truncated PE header')
    machine, sections, _, _, _, optional, _ = struct.unpack_from('<HHIIIHH', blob, pe + 4)
    if machine != 0x14C:
        raise ValueError('Expected x86 PE (machine 0x14C), got 0x%X' % machine)
    base = pe + 24 + optional
    entries = []
    for index in range(sections):
        off = base + index * 40
        if off + 40 > len(blob):
            raise ValueError('Truncated PE section table')
        size, rva, raw_size, raw_offset = struct.unpack_from('<IIII', blob, off + 8)
        entries.append((rva, rva + max(size, raw_size), raw_offset, raw_size))

    def view(rva, count):
        for start, stop, raw, raw_count in entries:
            delta = rva - start
            if start <= rva and rva + count <= stop and delta + count <= raw_count:
                pos = raw + delta
                if pos + count <= len(blob):
                    return blob[pos:pos + count]
        raise ValueError('RVA 0x%X cannot be read from PE raw sections (length %d)' % (rva, count))
    return view


def direct_rel32_target(view, call_rva):
    instruction = view(call_rva, 5)
    if instruction[0] != 0xE8:
        raise ValueError('Expected E8 rel32 at RVA 0x%X, found %s' % (call_rva, instruction.hex(' ')))
    signed = struct.unpack_from('<i', instruction, 1)[0]
    return (call_rva + 5 + signed) & 0xFFFFFFFF, instruction


def inspect(binary):
    data = Path(binary).read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != SHA256:
        raise ValueError('SHA256 mismatch; do not analyze a different game build: ' + digest)
    view = map_pe(data)
    out = [
        'Stage27 x86 call-flow inspection: ORIGINAL EXE VERIFIED',
        'SHA256=' + digest,
        'All addresses below are RVAs in the fingerprinted AALib.exe.',
        'Previous Stage26: 54 bit0 clears by the +0x97870 helper; 34 stack returns +0x979D1, 20 +0x9760E.',
        'Important: a caller-return does not prove its immediate call destination.',
    ]
    if view(0x97870, len(ENTRY_BYTES)) != ENTRY_BYTES:
        raise ValueError('Clearer helper signature mismatch at +0x97870')
    out.append('MATCH 0x97870 entry: push edi; mov edi,eax; and dword ptr [edi+0xc8],0xfffffff2')
    for call, expected in CALLS.items():
        target, instruction = direct_rel32_target(view, call)
        if target != expected:
            raise ValueError('Unexpected CALL target at +0x%X: +0x%X, expected +0x%X' % (call, target, expected))
        out.append('MATCH E8 CALL RVA 0x%05X -> 0x%05X, return 0x%05X, bytes %s' %
                   (call, target, call + 5, instruction.hex(' ')))
    for label, start, size in WINDOWS:
        out.append('WINDOW %s; start RVA 0x%05X, length %d' % (label, start, size))
        region = view(start, size)
        for offset in range(0, len(region), 16):
            out.append('  %05X: %s' % (start + offset, region[offset:offset + 16].hex(' ')))
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    except ImportError:
        out.append('Capstone not installed. Raw windows above preserve the bytes needed for offline disassembly.')
    else:
        cs = Cs(CS_ARCH_X86, CS_MODE_32)
        for label, start, size in WINDOWS[:3]:
            # Context windows need not start on instruction boundaries. Decode from known aligned entries only.
            aligned = 0x97970 if '97970' in label else (0x97870 if '97870' in label else 0x97609)
            out.append('DECODE (aligned at RVA 0x%05X) %s:' % (aligned, label))
            for ins in cs.disasm(view(aligned, 0x80), aligned):
                out.append('  %05X: %-7s %s' % (ins.address, ins.mnemonic, ins.op_str))
    out.append('INTERPRETATION: CALL 0x97609 targets 0x97970, not 0x97870. The path from 0x97970 to 0x97870 is still to be established from bytes/branches. No claim of a direct CALL is justified by the +0x9760E return address alone.')
    return '\n'.join(out) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    report = inspect(args.binary)
    args.output.write_text(report, encoding='utf-8')
    print('Saved:', args.output, '\n' + '\n'.join(report.splitlines()[:8]))


if __name__ == '__main__':
    main()
