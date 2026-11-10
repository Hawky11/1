#!/usr/bin/env python3
"""Original PE, reachable control flow, relocation and overlay checks.
Requires capstone and pefile; does not build or run the Windows DLL.
"""
import argparse, hashlib, re, zipfile
from pathlib import Path
import capstone, pefile

ROOT = Path(__file__).resolve().parents[2]
SHA = '28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
CALLS = {0xA119B: (0xAE6E0, 'E8 40 D5 00 00'),
         0xA11A0: (0xACDF0, 'E8 4B BC 00 00'),
         0xA11A5: (0xAC8A0, 'E8 F6 B6 00 00')}
PROLOGUES = {0xAE6E0: '55 8B EC 51 53 56 6A 00',
             0xACDF0: '55 8B EC 81 EC AC 00 00 00 53 56 57',
             0xAC8A0: '55 8B EC 83 EC 0C 53 56 57'}
RETURNS = {0xAE6E0: 0xAE872, 0xACDF0: 0xACFE6, 0xAC8A0: 0xACA8C}

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--aalib', required=True, type=Path)
    args = ap.parse_args()
    if args.aalib.suffix.lower() == '.zip':
        with zipfile.ZipFile(args.aalib) as z: data = z.read('AALib.exe')
    else: data = args.aalib.read_bytes()
    assert hashlib.sha256(data).hexdigest() == SHA, 'original EXE fingerprint'
    pe = pefile.PE(data=data)
    assert pe.FILE_HEADER.Machine == 0x14c and pe.OPTIONAL_HEADER.Magic == 0x10b
    assert pe.OPTIONAL_HEADER.ImageBase == 0x400000
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    reloc = {e.rva for block in pe.DIRECTORY_ENTRY_BASERELOC for e in block.entries if e.type == 3}
    signatures = dict(PROLOGUES)
    signatures.update({site: sig for site, (_, sig) in CALLS.items()})
    for addr, signature in signatures.items():
        expected = bytes.fromhex(signature)
        assert pe.get_data(addr, len(expected)) == expected, hex(addr)
        assert sum(i.size for i in md.disasm(expected, addr)) == len(expected)
        assert not any(addr < r+4 and r < addr+len(expected) for r in reloc), 'relocation overlap'
        print(f'PASS full instruction signature {addr:06X}; no HIGHLOW overlap')
    for site, (target, _) in CALLS.items():
        i = next(md.disasm(pe.get_data(site, 5), site))
        assert i.mnemonic == 'call' and i.operands[0].imm == target
    for start, ret in RETURNS.items():
        todo, seen, returns = [start], {}, []
        while todo:
            address = todo.pop()
            if address in seen: continue
            assert start <= address <= ret, 'unexpected tail/outside branch'
            i = next(md.disasm(pe.get_data(address, 16), address))
            seen[address] = i
            assert not any(op.type == capstone.x86.X86_OP_MEM and
                           op.mem.base == capstone.x86.X86_REG_EBP and op.mem.disp >= 8
                           for op in i.operands), 'stack argument reference'
            if i.mnemonic.startswith('ret'):
                assert bytes(i.bytes) == b'\xc3'
                returns.append(address)
                continue
            if i.mnemonic.startswith('j'):
                assert i.operands[0].type == capstone.x86.X86_OP_IMM, 'unresolved indirect jump'
                todo.append(i.operands[0].imm)
                if i.mnemonic == 'jmp': continue
            todo.append(address+i.size)
        assert returns == [ret]
        print(f'PASS {start:06X} reachable CFG: {len(seen)} instructions; plain RET {ret:06X}; no EBP stack arguments')
    for site, target in [(0xAE70C,0x1E66E0),(0xAE93E,0x1E66E0),(0xA2B81,0xA1B80)]:
        i = next(md.disasm(pe.get_data(site,5),site))
        assert i.mnemonic == 'call' and i.operands[0].imm == target
    for addr in (0xACF2A,0xACFA3):
        assert pe.get_data(addr,3) == bytes.fromhex('D9 58 7C')
    src = (ROOT/'Stage55_Overlay/src/Stage55EarlierMotionCoordinate.inl').read_text()
    adapter = (ROOT/'Stage55_Overlay/src/Stage38CallbackWindow.inl').read_text()
    previous = (ROOT/'Stage54_Overlay/src/Stage38CallbackWindow.inl').read_text()
    # Exact narrow adapter delta: no prior hook/flush logic may be rewritten.
    expected = previous.replace('#include "Stage54PreParentCoordinate.inl"', '#include "Stage54PreParentCoordinate.inl"\n#include "Stage55EarlierMotionCoordinate.inl"').replace(
        'if(InstallStage53Hooks())InstallStage54Hooks();',
        'if(InstallStage53Hooks()){\n                                                    if(InstallStage54Hooks())InstallStage55Hooks();\n                                                    else S55Log("ERROR: Stage55 requires installed Stage54; no patches.");\n                                                }').replace('    FlushStage54();','    FlushStage54();\n    FlushStage55();')
    assert adapter == expected
    assert src.count('fxsave [edi]') == src.count('fxrstor [edi]') == 6
    for letter in 'ABC': assert src.count(f'call dword ptr [g_s55Native{letter}]') == 1
    for needle in ['g_s54Installed,0,0)!=3','STAGE55_INSTALLED 3/3','STAGE55_EARLIER_WINDOW',
                   'STAGE55_HEALTH','STAGE55_EARLIER_MOTION_COORD_BUILD=2026-11-10-r1',
                   'protectionsOk&&valid()', 'SafeReadDword(g_gameBase+rva+',
                   'rvas[i]+5u+rel==targets[i]']:
        assert needle in src, needle
    # Source signatures must match the verified binary constants.
    for name, address in zip(['aPrologue','bPrologue','cPrologue'], PROLOGUES):
        body = re.search(r'const BYTE '+name+r'\[\]=\{([^}]+)\}', src).group(1)
        assert bytes(int(x.strip(),16) for x in body.split(',')) == bytes.fromhex(PROLOGUES[address])
    print('PASS overlay structure and source signatures; Windows MSVC build/game runtime PENDING')

if __name__ == '__main__': main()
