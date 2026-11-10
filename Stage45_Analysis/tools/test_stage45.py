#!/usr/bin/env python3
"""Original PE and extracted wrapper validation; requires unicorn and GNU binutils.
Does not replace Windows x86 compilation or runtime validation.
"""
from pathlib import Path
import argparse, hashlib, struct, re, subprocess, tempfile, random
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import *

def main():
    ap=argparse.ArgumentParser();ap.add_argument('exe',type=Path);args=ap.parse_args()
    root=Path(__file__).resolve().parents[2]
    src=(root/'Stage45_Overlay/src/Stage45DefinitionResolver.inl').read_text()
    b=args.exe.read_bytes()
    assert len(b)==175556608 and hashlib.sha256(b).hexdigest()=='28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
    p=struct.unpack_from('<I',b,60)[0];opt=p+24;ss=opt+struct.unpack_from('<H',b,p+20)[0]
    sections=[struct.unpack_from('<III',b,ss+i*40+12) for i in range(struct.unpack_from('<H',b,p+6)[0])]
    def read(r,n):
        for va,sz,raw in sections:
            if va<=r and r+n<=va+sz:return b[raw+r-va:raw+r-va+n]
        raise AssertionError(hex(r))
    code=read(0x1df2d0,0x33)
    assert code.hex()=='558bec8b450880bc0800000b0000761ac1e00703450c0fbf044148780d6bc05c03810c0b0b005dc20800b8e0492e0e5dc20800'
    assert read(0x1e84b3,5)==bytes.fromhex('e8 18 6e ff ff')
    assert 0x1e84b8+struct.unpack('<i',read(0x1e84b4,4))[0]==0x1df2d0
    rr,rs=struct.unpack_from('<II',b,opt+96+40);relocs=read(rr,rs);k=0;hits=[]
    while k+8<=len(relocs):
        page,size=struct.unpack_from('<II',relocs,k)
        if size<8:break
        for j in range(k+8,k+size,2):
            v=struct.unpack_from('<H',relocs,j)[0]
            if page+(v&4095)==0x1df2fb:hits.append(v>>12)
        k+=size
    assert hits==[3]
    for base in (0x400000,0x800000,0x1000000):
        assert (struct.unpack('<I',code[0x2b:0x2f])[0]+base-0x400000)==base+0xdee49e0
    integration=(root/'Stage45_Overlay/src/Stage38CallbackWindow.inl').read_text()
    assert 'if(InstallStage44Hooks())InstallStage45Hooks();' in integration
    assert integration.index('#include "Stage44ActionDefinition.inl"')<integration.index('#include "Stage45DefinitionResolver.inl"')
    assert 'FlushStage45();' in integration
    asm=src.split('    __asm {',1)[1].split('\n    }',1)[0]
    asm=re.sub(r'//[^\n]*','',asm)
    asm=re.sub(r'\b([0-9][0-9A-Fa-f]*)h\b',r'0x\1',asm)
    asm=asm.replace('S45Enter','enter_stub').replace('S45Leave','leave_stub').replace('g_s45NativeBinding','native_ptr')
    asm='.intel_syntax noprefix\n.global _start\n_start:\n'+asm+'\nenter_stub: ret\nleave_stub: ret\nnative_ptr: .long 0x5df2d0\n'
    with tempfile.TemporaryDirectory() as td:
        td=Path(td);(td/'t.s').write_text(asm)
        subprocess.run(['as','--32',str(td/'t.s'),'-o',str(td/'t.o')],check=True)
        subprocess.run(['ld','-m','elf_i386','-Ttext','0x300000','-o',str(td/'t'),str(td/'t.o')],check=True)
        subprocess.run(['objcopy','-O','binary','--only-section=.text',str(td/'t'),str(td/'t.bin')],check=True)
        wrapper=(td/'t.bin').read_bytes()
        symbols={l.split()[-1]:int(l.split()[0],16) for l in subprocess.check_output(['nm',str(td/'t')],text=True).splitlines() if len(l.split())==3}
    regs=[UC_X86_REG_EAX,UC_X86_REG_EBX,UC_X86_REG_ECX,UC_X86_REG_EDX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP,UC_X86_REG_ESP,UC_X86_REG_EFLAGS,UC_X86_REG_XMM0,UC_X86_REG_XMM7,UC_X86_REG_FPCW,UC_X86_REG_MXCSR]
    rng=random.Random(45)
    for case in range(128):
        ident=rng.randrange(0,0x600);variant=rng.randrange(128)
        gate=(0,1,255)[case%3];word=(0,1,2,32767,32768,65535)[case%6]
        ctx=0x1000000;table=0x2000000;esp=0x70f000;stop=0x301000
        initial=[rng.getrandbits(32) for _ in range(7)]+[esp,0x202|(case%2)*0x400, rng.getrandbits(128),rng.getrandbits(128),0x37f,0x1f80]
        initial[2]=ctx
        def run(wrapped):
            u=Uc(UC_ARCH_X86,UC_MODE_32)
            for addr,size in [(0x300000,0x2000),(0x5df000,0x1000),(0x700000,0x10000),(ctx,0x100000)]:u.mem_map(addr,size)
            u.mem_write(0x300000,wrapper);u.mem_write(0x5df2d0,code)
            u.mem_write(esp,struct.pack('<III',stop,ident,variant))
            u.mem_write(ctx+ident+0xb0000,bytes([gate]));u.mem_write(ctx+((ident<<7)+variant)*2,struct.pack('<H',word));u.mem_write(ctx+0xb0b0c,struct.pack('<I',table))
            for reg,v in zip(regs,initial):u.reg_write(reg,v)
            seen=[];calls=[]
            def hook(uc,address,size,unused):
                if address==0x5df2d0:
                    st=uc.reg_read(UC_X86_REG_ESP);assert struct.unpack('<II',uc.mem_read(st+4,8))==(ident,variant);calls.append(1)
                if address in (symbols['enter_stub'],symbols['leave_stub']):
                    st=uc.reg_read(UC_X86_REG_ESP);saved=struct.unpack('<I',uc.mem_read(st+4,4))[0];data=struct.unpack('<8I',uc.mem_read(saved,32));seen.append((address,data))
                    if address==symbols['enter_stub']:
                        assert data[6]==ctx
                        assert struct.unpack('<II',uc.mem_read(data[3]+8,8))==(ident,variant)
                    # Model volatile bookkeeping clobbers and flags/SSE modification.
                    for reg in (UC_X86_REG_EAX,UC_X86_REG_ECX,UC_X86_REG_EDX):uc.reg_write(reg,0xdeadbeef)
                    uc.reg_write(UC_X86_REG_EFLAGS,0x246);uc.reg_write(UC_X86_REG_XMM0,0x1234);uc.reg_write(UC_X86_REG_XMM7,0x5678)
                    uc.reg_write(UC_X86_REG_FPCW,0x27f);uc.reg_write(UC_X86_REG_MXCSR,0x3f80)
            u.hook_add(UC_HOOK_CODE,hook)
            u.emu_start(0x300000 if wrapped else 0x5df2d0,stop,count=1000)
            result=[u.reg_read(r) for r in regs]
            assert len(calls)==1 and result[7]==esp+12
            if wrapped:assert len(seen)==2 and seen[1][1][7]==result[0]
            return result
        actual=run(False);wrapped=run(True);assert actual==wrapped,(case,actual,wrapped)
        signed=word if word<32768 else word-65536
        expected=0xe2e49e0 if gate==0 or signed<=0 else table+(signed-1)*0x5c
        assert actual[0]==expected
    print('PASS: original SHA, full native resolver, CALL, HIGHLOW fallback relocation, integration; 128 extracted-wrapper/native equivalence cases (GPR/ESP/EFLAGS, XMM0/7, FPCW/MXCSR), once-only call and arguments; Windows x86 build/runtime PENDING.')

if __name__=='__main__':main()
