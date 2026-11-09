#!/usr/bin/env python3
"""Emulate the actual extracted Stage31 wrapper; GNU binutils + unicorn required.
This is not an MSVC build or Windows/game runtime validation.
"""
import pathlib,re,subprocess,tempfile,struct,unittest
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE
from unicorn.x86_const import *

ROOT=pathlib.Path(__file__).resolve().parents[1]
SOURCE=ROOT/'src/AIQueueTrace.cpp'
REGS=[UC_X86_REG_EAX,UC_X86_REG_EBX,UC_X86_REG_ECX,UC_X86_REG_EDX,
      UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP,UC_X86_REG_ESP,UC_X86_REG_EFLAGS]

def assemble():
    src=SOURCE.read_text()
    body=src.split('__declspec(naked) void Stage31AlternateHook()',1)[1].split('    void FlushStage31()',1)[0]
    body=body.split('__asm',1)[1].split('{',1)[1].rsplit('}',2)[0]
    lines=[]
    for line in body.splitlines():
        line=line.split('//')[0].strip()
        if not line:continue
        line=re.sub(r'\b([0-9][0-9A-Fa-f]*)h\b',r'0x\1',line)
        lines.append(line)
    asm='.intel_syntax noprefix\n.code32\n.global _start\n_start:\n'+ '\n'.join(lines)+'''
.org 0x200
Stage31Begin:
    mov eax, 0xdeadbeef
    mov ecx, 0xbaadf00d
    mov edx, 0xabcdef01
    pxor xmm0, xmm0
    fninit
    fld1
    stc
    ret
.org 0x240
Stage31End:
    mov eax, 0x12345678
    mov ecx, 0x87654321
    mov edx, 0x11111111
    pxor xmm0, xmm0
    fninit
    fldz
    clc
    ret
.org 0x300
g_stage31Target: .long 0x100400
.org 0x400
original:
    mov eax, [esp+4]
    mov ecx, 0x24681357
    mov edx, 0x10203040
    pcmpeqd xmm0,xmm0
    fld1
    stc
    ret
.org 0x500
finish:
    fxsave [0x200000]
    nop
'''
    with tempfile.TemporaryDirectory() as d:
        p=pathlib.Path(d);(p/'test.s').write_text(asm)
        subprocess.run(['as','--32','-o',str(p/'test.o'),str(p/'test.s')],check=True)
        subprocess.run(['ld','-m','elf_i386','-Ttext=0x100000','-o',str(p/'test.elf'),str(p/'test.o')],check=True)
        subprocess.run(['objcopy','-O','binary','-j','.text',str(p/'test.elf'),str(p/'test.bin')],check=True)
        return (p/'test.bin').read_bytes()

def run(code,wrapped,alignment,flags):
    u=Uc(UC_ARCH_X86,UC_MODE_32)
    u.mem_map(0x100000,0x1000);u.mem_write(0x100000,code)
    u.mem_map(0x200000,0x1000);u.mem_map(0x300000,0x20000)
    sp=0x310000+alignment
    for i,reg in enumerate(REGS[:-2]):u.reg_write(reg,0x76540000+i)
    u.reg_write(UC_X86_REG_ESP,sp);u.reg_write(UC_X86_REG_EFLAGS,flags)
    u.reg_write(UC_X86_REG_FPCW,0x37f);u.reg_write(UC_X86_REG_FPTAG,0xffff)
    u.reg_write(UC_X86_REG_MXCSR,0x1f80);u.reg_write(UC_X86_REG_XMM0,0x123456789abcdef)
    stack=struct.pack('<5I',0x100500,0x1234abcd,0xaabbccdd,0x01020304,0xf0f0f0f0)
    u.mem_write(sp,stack)
    observations=[]
    def hook(uc,address,size,user):
        if address==0x100200:
            ctx=struct.unpack('<I',uc.mem_read(uc.reg_read(UC_X86_REG_ESP)+4,4))[0]
            assert struct.unpack('<I',uc.mem_read(ctx+40,4))[0]==0x1234abcd
            assert ctx==sp-36
        if address==0x100400:
            assert struct.unpack('<I',uc.mem_read(uc.reg_read(UC_X86_REG_ESP)+4,4))[0]==0x1234abcd
            observations.append([uc.reg_read(r) for r in REGS if r!=UC_X86_REG_ESP])
    u.hook_add(UC_HOOK_CODE,hook)
    u.emu_start(0x100000 if wrapped else 0x100400,0x100507,count=500)
    return ([u.reg_read(r) for r in REGS],bytes(u.mem_read(0x200000,512)),
            bytes(u.mem_read(sp+4,16)),observations)

def native_run(code,wrapped,alignment,flags):
    # Linux IA32 execution checks FIP/FDP too: Unicorn 2.1.4 omits their restore.
    code=bytearray(code);code[0x500:0x505]=b'\xe9'+struct.pack('<i',0x800-0x505)
    with tempfile.TemporaryDirectory() as d:
        p=pathlib.Path(d);(p/'code.bin').write_bytes(code)
        asm=f'''.intel_syntax noprefix
.code32
.text
.incbin "{p/'code.bin'}"
.org 0x700
.global native_start
native_start:
    lea esp, [stack+65536+{alignment}]
    fninit
    pxor xmm0,xmm0
    push {flags}
    popfd
    mov eax,0x76540000
    mov ebx,0x76540001
    mov ecx,0x76540002
    mov edx,0x76540003
    mov esi,0x76540004
    mov edi,0x76540005
    mov ebp,0x76540006
    push 0xf0f0f0f0
    push 0x01020304
    push 0xaabbccdd
    push 0x1234abcd
    push 0x100500
    jmp {('0x100000' if wrapped else '0x100400')}
.org 0x800
native_finish:
    fxsave [0x200000]
    pushfd
    pushad
    mov esi,esp
    mov edi,0x200200
    mov ecx,9
    cld
    rep movsd
    mov eax,4
    mov ebx,1
    mov ecx,0x200000
    mov edx,548
    int 0x80
    mov eax,1
    xor ebx,ebx
    int 0x80
.data
.space 4096
.bss
.balign 16
stack: .space 131072
'''
        (p/'native.s').write_text(asm)
        subprocess.run(['as','--32','-o',str(p/'native.o'),str(p/'native.s')],check=True)
        subprocess.run(['ld','-m','elf_i386','-Ttext=0x100000','-Tdata=0x200000','-e','native_start','-o',str(p/'native'),str(p/'native.o')],check=True)
        return subprocess.check_output([str(p/'native')])

class WrapperTests(unittest.TestCase):
    def test_stack_register_flags_fx_preservation(self):
        code=assemble()
        for alignment in range(0,16,4):
            for flags in (0x202,0x247,0x602):
                with self.subTest(alignment=alignment,flags=flags):
                    direct=run(code,False,alignment,flags)
                    wrapped=run(code,True,alignment,flags)
                    # Only FIP/FDP metadata is excluded from this emulator check;
                    # the native test below compares the entire 512-byte FX image.
                    self.assertEqual(direct[0],wrapped[0])
                    self.assertEqual(direct[1][:8]+direct[1][24:],wrapped[1][:8]+wrapped[1][24:])
                    self.assertEqual(direct[2:],wrapped[2:])
                    self.assertEqual(len(wrapped[3]),1)

    def test_native_full_fx_and_register_image(self):
        code=assemble()
        try:
            native_run(code,False,0,0x202)
        except OSError as error:
            if error.errno==8:
                self.skipTest('Host cannot execute IA32 ELF; full FIP/FDP native check unavailable')
            raise
        for alignment in range(0,16,4):
            for flags in (0x202,0x247,0x602):
                with self.subTest(alignment=alignment,flags=flags):
                    self.assertEqual(native_run(code,False,alignment,flags),native_run(code,True,alignment,flags))

if __name__=='__main__':unittest.main(verbosity=2)
