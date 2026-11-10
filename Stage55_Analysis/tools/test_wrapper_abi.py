#!/usr/bin/env python3
"""Extract the actual MS inline-asm bodies; assemble with GNU as; emulate with Unicorn.
Differential synthetic native/logger test, NOT a game or MSVC build test.
Requires binutils and unicorn. Checks 128 cases for each of three wrappers.
"""
import random, re, struct, subprocess, tempfile
from pathlib import Path
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import *

ROOT = Path(__file__).resolve().parents[2]
BASE, WRAP, ENTER, LEAVE, NATIVE, PTR, STOP, INIT, SAVE = [0x100000+x for x in (0,0x1000,0x2000,0x2100,0x3000,0x4000,0x5000,0x6000,0x7000)]
STACK, ESP = 0x200000, 0x208000
REGS = [UC_X86_REG_EAX,UC_X86_REG_EBX,UC_X86_REG_ECX,UC_X86_REG_EDX,
        UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP,UC_X86_REG_ESP,UC_X86_REG_EFLAGS]

def assemble(code):
    with tempfile.TemporaryDirectory() as d:
        p = Path(d)
        (p/'a.s').write_text('.intel_syntax noprefix\n.code32\n.text\n'+code+'\n')
        subprocess.run(['as','--32','-o',str(p/'a.o'),str(p/'a.s')],check=True)
        subprocess.run(['objcopy','-O','binary','-j','.text',str(p/'a.o'),str(p/'a.bin')],check=True)
        return (p/'a.bin').read_bytes()

def wrapper(name, letter):
    src = (ROOT/'Stage55_Overlay/src/Stage55EarlierMotionCoordinate.inl').read_text()
    asm = re.search(r'void '+name+r'\(\)\s*\{\s*__asm\s*\{(.*?)\n  \}',src,re.S).group(1)
    asm = re.sub(r'\b([0-9][0-9A-Fa-f]*)h\b',lambda m:'0x'+m.group(1),asm)
    asm = asm.replace('pushfd','pushfd').replace('g_s55Native'+letter,str(PTR))
    # Make GAS resolve direct relative calls from the actual load address.
    asm = asm.replace('call S55Enter',f'call . + ({ENTER-WRAP}) - (. - start)')
    asm = asm.replace('call S55Leave',f'call . + ({LEAVE-WRAP}) - (. - start)')
    return assemble('start:\n'+asm)

LOGGER = assemble('mov eax,0xdeadbeef\nmov ecx,0xaabbccdd\nmov edx,0x12345678\npxor xmm0,xmm0\nfld1\nstd\nret')
NATIVE_CODE = assemble('push ebp\nmov ebp,esp\nsub esp,16\nmov [ebp-4],eax\nadd eax,edx\nxor ecx,0x12345678\nsub edx,19\npxor xmm1,xmm1\nfld1\nmov esp,ebp\npop ebp\nret')

def run(code, seed, hooked):
    rnd = random.Random(seed)
    u = Uc(UC_ARCH_X86,UC_MODE_32)
    u.mem_map(BASE,0x10000);u.mem_map(STACK,0x10000)
    u.mem_write(WRAP,code);u.mem_write(ENTER,LOGGER);u.mem_write(LEAVE,LOGGER)
    u.mem_write(NATIVE,NATIVE_CODE);u.mem_write(PTR,struct.pack('<I',NATIVE))
    for r in REGS[:-2]: u.reg_write(r,rnd.getrandbits(32))
    u.reg_write(UC_X86_REG_ESP,ESP)
    u.mem_write(ESP,struct.pack('<I',STOP))
    # Seed x87 stack and rounding control, MXCSR, and all XMM registers.
    init = assemble('fninit\n'+('fld1\n'*(seed%4)))
    u.mem_write(INIT,init);u.emu_start(INIT,INIT+len(init))
    u.reg_write(UC_X86_REG_FPCW,0x37f | ((seed%4)<<10))
    u.reg_write(UC_X86_REG_MXCSR,0x1f80 | ((seed%4)<<13))
    for n in range(8):u.reg_write(UC_X86_REG_XMM0+n,rnd.getrandbits(128))
    u.reg_write(UC_X86_REG_EFLAGS,0x202 | (rnd.getrandbits(12)&0xcd5))
    calls, observations = [], []
    def capture(uc,address,size,context):
        if address == NATIVE:
            calls.append(address)
            observations.append(tuple(uc.reg_read(r) for r in REGS if r != UC_X86_REG_ESP))
        if address in (ENTER,LEAVE):
            # cdecl saved-register argument must point to PUSHAD layout.
            sp=uc.reg_read(UC_X86_REG_ESP)
            saved=struct.unpack('<I',uc.mem_read(sp+4,4))[0]
            saved_esp=struct.unpack('<I',uc.mem_read(saved+12,4))[0]
            assert saved_esp+4 == ESP
            assert struct.unpack('<I',uc.mem_read(saved_esp+4,4))[0] == STOP
    u.hook_add(UC_HOOK_CODE,capture)
    u.emu_start(WRAP if hooked else NATIVE,STOP,count=1000)
    assert u.reg_read(UC_X86_REG_EIP)==STOP and len(calls)==1
    final=tuple(u.reg_read(r) for r in REGS)
    # Save actual x87/MMX/XMM state after return; scratch register is excluded above.
    save_code=assemble(f'mov edi,{SAVE}\nfxsave [edi]')
    u.mem_write(INIT,save_code)
    u.ctl_remove_cache(INIT,INIT+32)
    u.emu_start(INIT,INIT+len(save_code))
    fx=bytearray(u.mem_read(SAVE,512))
    # The emulator comparison excludes x87 instruction/data-pointer metadata.
    # Compare CW/SW/TW/opcode,
    # MXCSR, all 80-bit ST payloads and all XMM payloads.
    fx[8:24]=bytes(16)
    fx=bytes(fx)
    return final,fx,observations

def main():
    for name,letter in [('S55AE6E0Hook','A'),('S55ACDF0Hook','B'),('S55AC8A0Hook','C')]:
        code=wrapper(name,letter)
        for seed in range(128):
            direct=run(code,seed,False);wrapped=run(code,seed,True)
            assert direct==wrapped,(name,seed,'native entry/output state differs')
        print('PASS',name,'128 cases: native once, GPR/EFLAGS/ESP/x87/MXCSR/XMM and logger saved-stack layout')
    print('PASS 384 synthetic cases; x87 instruction/data pointers excluded; not MSVC or in-game validation')

if __name__=='__main__':main()
