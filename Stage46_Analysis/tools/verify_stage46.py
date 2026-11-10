#!/usr/bin/env python3
"""SHA-pinned native movement branch verification; requires Unicorn.
Windows build and in-game behavior remain pending.
"""
from pathlib import Path
import argparse,hashlib,struct
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32
from unicorn.x86_const import *

def main():
    p=argparse.ArgumentParser();p.add_argument('exe',type=Path);p.add_argument('--prior44',type=Path);p.add_argument('--prior45',type=Path);a=p.parse_args()
    b=a.exe.read_bytes();assert len(b)==175556608
    assert hashlib.sha256(b).hexdigest()=='28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
    pe=struct.unpack_from('<I',b,60)[0];o=pe+24;ss=o+struct.unpack_from('<H',b,pe+20)[0]
    sec=[struct.unpack_from('<III',b,ss+i*40+12) for i in range(struct.unpack_from('<H',b,pe+6)[0])]
    def read(r,n):
        for va,sz,raw in sec:
            if va<=r and r+n<=va+sz:return b[raw+r-va:raw+r-va+n]
        raise AssertionError(hex(r))
    targets=[0x1e8a54,0x1e8a71,0x1e8b6d,0x1e8c1c,0x1e8c84,0x1e8c9c,0x1e8caa,0x1e8cb7,0x1e8cc4]
    assert list(struct.unpack('<9I',read(0x1e8d30,36)))==[x+0x400000 for x in targets]
    assert struct.unpack('<d',read(0x2c6ac8,8))[0]==100.0
    assert struct.unpack('<d',read(0x2c7d28,8))[0]==-1.0
    signatures={0x1e8a1f:'8b4e70d9ee3b4e6c',0x1e8a2d:'8b5608c1ea09f6c201',0x1e8a3c:'8b7e340fb647304883f808',0x1e8d06:'8b466c3b46707e03894670'}
    for r,h in signatures.items():assert read(r,len(bytes.fromhex(h)))==bytes.fromhex(h)
    root=Path(__file__).resolve().parents[2];s44=(root/'Stage46_Overlay/src/Stage44ActionDefinition.inl').read_text();s45=(root/'Stage46_Overlay/src/Stage45DefinitionResolver.inl').read_text()
    for old,new in [(a.prior44,s44),(a.prior45,s45)]:
        if old:
            prev=old.read_text();assert prev.split('    __asm {')[1].split('\n    }')[0]==new.split('    __asm {')[1].split('\n    }')[0]
            assert prev.count('MakeRelativeCall(')==new.count('MakeRelativeCall(')==1
    assert s45.index('r.Result=saved[7]')<s45.index('S46Resolved(r.Object,r.Result)')
    assert 'S46FlushRecord(r);' in s44 and 'S46Seen!=1u' in s44
    assert '0x60u,0x6Cu,0x70u,0x08u,0x24u,0x7Cu,0x80u' in s44
    count=0
    # Execute original branch bytes. Helper flag query 1EB0 is stubbed false.
    # Modes 2/7/8 have additional helper semantics and are deliberately excluded.
    for mode in [0,1,3,4,5,6,9,10]:
      for blocked in [False,True]:
       for old_variant in [-1,0]:
        for mirror in [False,True]:
            u=Uc(UC_ARCH_X86,UC_MODE_32)
            for base,size in [(0x5e8000,0x2000),(0x6c6000,0x2000),(0x401000,0x1000),(0x1000000,0x2000),(0x700000,0x10000)]:u.mem_map(base,size)
            u.mem_write(0x5e8000,read(0x1e8000,0x2000));u.mem_write(0x6c6000,read(0x2c6000,0x2000))
            u.mem_write(0x401eb0,bytes.fromhex('31c0c20400'))
            obj=0x1000000;definition=obj+0x1000
            def dw(off,v):u.mem_write(obj+off,struct.pack('<I',v&0xffffffff))
            def fl(off,v):u.mem_write(obj+off,struct.pack('<f',v))
            dw(0x34,definition);dw(0x6c,0);dw(0x70,old_variant);dw(8,0x200 if blocked else 0);dw(0x24,0x100 if mirror else 0)
            vals={0x7c:40.,0x80:50.,0x88:1.,0x8c:2.,0x90:3.,0x94:4.,0x150:5.,0x154:6.}
            for off,v in vals.items():fl(off,v)
            u.mem_write(definition+0x30,bytes([mode,0])+struct.pack('<hhhh',100,-200,300,-400))
            u.reg_write(UC_X86_REG_ESI,obj);u.reg_write(UC_X86_REG_EBP,0x70f000);u.reg_write(UC_X86_REG_ESP,0x70ef00)
            u.reg_write(UC_X86_REG_FPCW,0x37f)
            u.emu_start(0x5e8a1f,0x5e8d11,count=1000)
            expected=dict(vals)
            if not blocked and old_variant<0:
                if mode in (1,5):
                    for off in (0x88,0x8c,0x90,0x94):expected[off]=0.
                if mode in (5,6):expected[0x150]=expected[0x154]=0.
                if mode==3:
                    expected.update({0x88:1.,0x90:-2.,0x8c:-3. if mirror else 3.,0x94:4. if mirror else -4.})
                if mode==4:expected.update({0x90:-2.,0x94:4. if mirror else -4.})
                if mode==9:expected[0x94]=-4.
            actual={off:struct.unpack('<f',u.mem_read(obj+off,4))[0] for off in vals}
            assert actual==expected,(mode,blocked,old_variant,mirror,actual,expected)
            assert struct.unpack('<I',u.mem_read(obj+0x70,4))[0]==0
            count+=1
    print('PASS: original SHA/signatures, nine-way dispatch, constants, unchanged wrapper assembly, '+str(count)+' original movement-fragment cases; helper query stubbed false; Windows x86 build/runtime PENDING.')
if __name__=='__main__':main()
