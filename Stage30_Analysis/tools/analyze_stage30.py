#!/usr/bin/env python3
"""Read-only, SHA-pinned Stage30 extraction. Requires GNU objdump for disassembly."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile

SHA = '28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
WINDOWS = {
    'producer': (0xf6a63, 0xf6ae8),
    'orientation': (0xe8560, 0xe86cd),
    'snapshot': (0x156c00, 0x156c21),
    'alternate': (0x130a10, 0x1313a0),
    'scalar_update': (0xe6c60, 0xe6d7d),
    'selected_object': (0xe6960, 0xe6996),
    'field_helpers': (0xf5350, 0xf53ae),
    'continuation_entry': (0x12f3b0, 0x12f412),
    'continuation_tail': (0x12fae9, 0x12fb96),
    'type0_producer': (0x156da0, 0x156e5e),
    'scaled_slot_writer': (0x991c0, 0x991df),
    'bit17_clear': (0xed188, 0xed1a9),
}
CALLS = {0xf6a79:0xe8560,0xf6a83:0x156c00,0xf6a8c:0x130a10,
         0xf6acc:0xe8560,0xf6ad7:0xf2de0,0x130bdc:0xe8560,
         0x130be2:0x156c30,0x130c3b:0xf5350,0x130c4d:0xf53a0,
         0x130cb2:0xe6c60,0x130cfb:0x12f3b0,0xe6d6f:0xf5390,
         0x12fb04:0x991c0,0x12fb53:0x156da0,0x156e47:0xf2de0,
         0xed193:0xf2ed0}

def analyze(binary, output):
    data = pathlib.Path(binary).read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != SHA:
        raise ValueError('Executable SHA256 mismatch; no RVA analysis performed')
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    assert data[pe:pe+4] == b'PE\0\0'
    assert struct.unpack_from('<H', data, pe+4)[0] == 0x14c
    count, optsize = struct.unpack_from('<H',data,pe+6)[0], struct.unpack_from('<H',data,pe+20)[0]
    base = struct.unpack_from('<I',data,pe+24+28)[0]
    assert base == 0x400000
    sections = []
    for i in range(count):
        off = pe+24+optsize+i*40
        vs, va, size, raw = struct.unpack_from('<4I',data,off+8)
        sections.append((va,size,raw))
    def read(rva, length):
        for va,size,raw in sections:
            if va <= rva and rva+length <= va+size:
                return data[raw+rva-va:raw+rva-va+length]
        raise ValueError('RVA outside file-backed section')
    verified = []
    for site,target in CALLS.items():
        code = read(site,5)
        assert code[0] == 0xe8 and site+5+struct.unpack('<i',code[1:])[0] == target
        verified.append({'site':hex(site),'target':hex(target),'bytes':code.hex()})
    signatures = {0xf6a73:'7428',0xf6a96:'b001',0xe6c66:'818e843b000000000100',
                  0xed19f:'81a6843b0000fffff9ff',0x130a7b:'899e38420000',
                  0x130a81:'899e3c420000',0x130c29:'818e803b000000000100',
                  0x12f40c:'0f85d7060000',0x156e4f:'818e703b000000000200'}
    for site,expected in signatures.items():
        assert read(site,len(bytes.fromhex(expected))).hex() == expected
    constants = {hex(va):struct.unpack('<f',read(va-base,4))[0]
                 for va in (0x7e4760,0x7fe2e0,0x800710,0x6c6388)}
    scale = struct.unpack('<d',read(0x6c6ac8-base,8))[0]
    assert list(constants.values()) == [-2.0,-0.5,-0.25,-1.0] and scale == 100.0
    out = pathlib.Path(output); out.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        for name,(start,end) in WINDOWS.items():
            chunk = pathlib.Path(tmp)/'window.bin'; chunk.write_bytes(read(start,end-start))
            text = subprocess.check_output(['objdump','-D','-b','binary','-m','i386','-M','intel',
                    '--insn-width=16',f'--adjust-vma={start}',str(chunk)],text=True)
            text = text.replace(str(chunk),'SHA256-pinned AALib.exe window')
            (out/(name+'.asm')).write_text(text)
    summary = {'sha256':digest,'size':len(data),'image_base':hex(base),
               'verified_calls':verified,'verified_signatures':len(signatures),
               'float_constants_by_preferred_VA':constants,'double_scale':scale,
               'windows':{k:[hex(x) for x in v] for k,v in WINDOWS.items()}}
    (out/'verification.json').write_text(json.dumps(summary,indent=2)+'\n')
    print(json.dumps({'verified_calls':len(verified),'verified_signatures':len(signatures),'windows':len(WINDOWS),'sha256':digest}))

if __name__ == '__main__':
    parser=argparse.ArgumentParser()
    parser.add_argument('--binary',required=True)
    parser.add_argument('--output',required=True)
    args=parser.parse_args()
    analyze(args.binary,args.output)
