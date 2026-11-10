#!/usr/bin/env python3
"""SHA-pinned, read-only classifier of original AALib.exe request-setter direct callers.

Requires a companion GNU/LLVM objdump disassembly generated from this same EXE
using Intel syntax. A site is 'decoded' only if its absolute VA is a disassembled
CALL instruction and it reaches the expected original target. 'CC islands' are
purely structural clusters after runs of INT3 padding, NOT certified functions.
"""
import argparse
import bisect
from collections import Counter, defaultdict
from hashlib import sha256
import json
from pathlib import Path
import re
import struct

EXPECTED_SHA = '28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
IMAGE_BASE = 0x400000
TARGETS = {0xE5BF0: 296, 0xE9E30: 11, 0xE8480: 1}
ADDR = re.compile(r'^\s*([0-9a-fA-F]+):\s+((?:[0-9a-fA-F]{2}\s+)+)\s*(.*)$')


def pe_text(data):
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    if data[pe:pe+4] != b'PE\x00\x00':
        raise ValueError('Bad PE')
    machine, sections = struct.unpack_from('<HH', data, pe + 4)
    optional_size = struct.unpack_from('<H', data, pe + 20)[0]
    magic = struct.unpack_from('<H', data, pe + 24)[0]
    image_base = struct.unpack_from('<I', data, pe + 24 + 28)[0]
    if (machine, magic, image_base) != (0x14c, 0x10b, IMAGE_BASE):
        raise ValueError('Unexpected PE32/i386/image base')
    for i in range(sections):
        off = pe + 24 + optional_size + i * 40
        name = data[off:off+8].split(b'\x00')[0]
        vs, rva, size, file_off = struct.unpack_from('<IIII', data, off + 8)
        if name == b'.text':
            return data[file_off:file_off+min(vs, size)], rva
    raise ValueError('Missing .text')


def scan_e8(code, base_rva):
    refs = defaultdict(list)
    for pos in range(len(code)-4):
        if code[pos] != 0xe8:
            continue
        rva = base_rva + pos
        target = (rva + 5 + struct.unpack_from('<i', code, pos+1)[0]) & 0xffffffff
        if target in TARGETS:
            refs[target].append(rva)
    for target, expected in TARGETS.items():
        if len(refs[target]) != expected:
            raise ValueError('E8 count mismatch at %06X: %d != %d' % (target, len(refs[target]), expected))
    return refs


def parse_disasm(path):
    instructions = []
    for line in path.open(encoding='utf-8', errors='replace'):
        match = ADDR.match(line)
        if not match:
            continue
        va = int(match.group(1), 16)
        bs = bytes.fromhex(match.group(2))
        asm = match.group(3).strip()
        if not bs:
            continue
        instructions.append((va, asm, bs))
    return instructions


def operand_class(x):
    if x is None:
        return 'unresolved'
    if x.startswith('0x'):
        return 'immediate'
    if x in {'eax','ebx','ecx','edx','esi','edi','ebp'}:
        return 'register'
    return 'other'


def conservative_args(instructions, ix):
    # Only examine nearest decoded straight-line block; no jumps or CALLs can
    # be crossed. If prior pushes are in another branch or >48 bytes back,
    # report as unresolved rather than force an ID/flag interpretation.
    target_va = instructions[ix][0]
    preceding = []
    for j in range(ix-1, max(-1, ix-32), -1):
        addr, asm, _ = instructions[j]
        if target_va - addr > 48:
            break
        mnemonic = asm.split()[0] if asm else ''
        if mnemonic in {'call', 'ret', 'retn', 'jmp', 'int3', 'ud2'} or mnemonic.startswith('j'):
            break
        preceding.append((addr, asm))
    preceding.reverse()
    pushes = [(v, asm.split(None, 1)[1].strip()) for v,asm in preceding if asm.startswith('push ')]
    flag_op = pushes[-2][1] if len(pushes)>=2 else None
    id_op = pushes[-1][1] if len(pushes)>=2 else None
    mode = ('0' if flag_op=='0x0' else '1' if flag_op=='0x1'
            else 'dynamic' if flag_op is not None else 'unresolved')
    return dict(mode=mode, mode_operand=flag_op, id_kind=operand_class(id_op),
                id_operand=id_op, immediate_id=(int(id_op,16) if id_op is not None and id_op.startswith('0x') else None))


def build(exe, disasm):
    data = exe.read_bytes()
    digest = sha256(data).hexdigest()
    if digest != EXPECTED_SHA:
        raise ValueError('Unexpected original SHA: ' + digest)
    code, base = pe_text(data)
    refs = scan_e8(code, base)
    instr = parse_disasm(disasm)
    indexed = {a:i for i,(a,_,_) in enumerate(instr)}
    if len(indexed) != len(instr):
        raise ValueError('Repeated objdump instruction address')
    for target, sites in refs.items():
        for rva in sites:
            va = IMAGE_BASE+rva
            if va not in indexed:
                raise ValueError('Potential embedded-data E8 at %06X' % rva)
            _, asm, bs = instr[indexed[va]]
            if len(bs) != 5 or bs[0]!=0xe8 or not asm.startswith('call'):
                raise ValueError('Not decoded as a direct CALL at %06X' % rva)
            if (rva+5+struct.unpack_from('<i',bs,1)[0]) & 0xffffffff != target:
                raise ValueError('Disassembly disagrees with rel32 at %06X' % rva)
    cc = [a for a,asm,bs in instr if bs==b'\xcc' and asm=='int3']
    sites = []
    for target in sorted(TARGETS):
        for rva in refs[target]:
            va = IMAGE_BASE+rva
            ccindex = bisect.bisect_left(cc, va)-1
            island_va = cc[ccindex]+1 if ccindex>=0 else (IMAGE_BASE+base)
            if island_va not in indexed:
                raise ValueError('INT3 island is not a decoded instruction at %06X' % rva)
            detail = conservative_args(instr, indexed[va]) if target==0xE5BF0 else {}
            sites.append(dict(target=f'{target:06X}', rva=f'{rva:06X}',
                island=f'{island_va-IMAGE_BASE:06X}', bucket=f'{rva >> 16:02X}', **detail))
    setters=[s for s in sites if s['target']=='0E5BF0']
    buck=Counter(s['bucket'] for s in setters)
    islands=Counter(s['island'] for s in setters)
    modes=Counter(s['mode'] for s in setters)
    id_kinds=Counter(s['id_kind'] for s in setters)
    helper_sites=['131CC5','131D1C']
    for site in helper_sites:
        if site not in {s['rva'] for s in setters}:
            raise ValueError('Expected callback helper E5BF0 call missing: '+site)
    return {
        'sha256':digest,
        'method':'SHA-pinned original PE .text E8 rel32, verified decoded objdump instruction boundaries; conservative straight-line PUSH analysis; INT3 padding structural clusters',
        'direct_counts':{f'{k:06X}':len(refs[k]) for k in sorted(TARGETS)},
        'setter_64kb_buckets':dict(sorted(buck.items())),
        'setter_int3_island_count':len(islands),
        'largest_structural_islands':[dict(start=k,count=v) for k,v in islands.most_common(15)],
        'conservative_setter_flag_counts':dict(sorted(modes.items())),
        'conservative_setter_id_operand_counts':dict(sorted(id_kinds.items())),
        'stage39_known_direct_setter_helpers':helper_sites,
        'conditional_writer_sites':[s['rva'] for s in sites if s['target']=='0E9E30'],
        'sites':sites,
        'limitations':[
            'A padding island is not a proven true function boundary.',
            'The disassembler can be confused by control-flow/data; only E8 sites confirmed as decoded CALL instructions are accepted.',
            'Constant mode/ID classifications require two preceding PUSH instructions with no intervening branch/call in <=48 bytes; unresolved is not a negative observation.',
            'Direct callers are possible static paths; no site-specific execution counts or COM attribution from this analysis.',
            'Indirect callsites, aliasing writes, utility/scoring logic, and runtime branch conditions remain unproven.',
        ]
    }


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('exe',type=Path)
    p.add_argument('--objdump',type=Path,required=True,help='Intel-syntax GNU/LLVM objdump -d output for the same original EXE')
    p.add_argument('--json',type=Path)
    a=p.parse_args()
    out=build(a.exe,a.objdump)
    print('PASS original SHA and %d/%d/%d decoded original direct CALLs' % (
        out['direct_counts']['0E5BF0'],out['direct_counts']['0E9E30'],out['direct_counts']['0E8480']))
    print('64KB buckets',out['setter_64kb_buckets'])
    print('INT3 padding islands',out['setter_int3_island_count'])
    print('Flag classifications',out['conservative_setter_flag_counts'])
    print('Request-ID operands',out['conservative_setter_id_operand_counts'])
    if a.json:
        a.json.write_text(json.dumps(out,indent=2)+'\n',encoding='utf-8')
        print('Report',a.json)


if __name__=='__main__':
    main()
