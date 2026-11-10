#!/usr/bin/env python3
"""Read-only original EXE + actual wrapper assembly differential checks.
Requires pip install unicorn keystone-engine. Does not run AALib.exe.
Not a replacement for MSVC x86 compilation or an in-game test.
"""
import argparse
import hashlib
import random
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Stage66_Analysis/tools'))
from verify_stage66 import EXPECTED_SHA, section_reader

def check_static(exe, source, adapter):
    data = exe.read_bytes()
    assert hashlib.sha256(data).hexdigest() == EXPECTED_SHA, 'EXE SHA mismatch'
    read, _ = section_reader(data)
    # Check the exact arrays used by the runtime installer, not a second copy.
    guards = {'call':0xAD75C, 'promotion':0x1E9F40, 'exits':0x1E9F67,
              'write':0xE9E60, 'gate':0xE9E56, 'next':0xE9E66}
    for name, rva in guards.items():
        match = re.search(r'const BYTE '+name+r'\[\]=\{([^}]+)\}', source)
        assert match, name
        expected = bytes(int(x.strip(), 0) for x in match[1].split(','))
        assert read(rva, len(expected)) == expected, hex(rva)
    assert struct.unpack('<I', read(0xE9E5C, 4))[0] == 0x8254E8
    assert read(0x4254E8, 4) == bytes(4)
    assert adapter.index('InstallStage65Hooks();') < adapter.index('InstallStage67Hooks();') < adapter.index('StartStage59Watch();')
    assert adapter.count('InstallStage67Hooks();') == adapter.count('FlushStage67();') == 1
    assert adapter.index('#include "Stage65RequestOrigin.inl"') < adapter.index('#include "Stage67NativePaths.inl"')
    assert 'CONTEXT_DEBUG_REGISTERS' not in source and 'SetThreadContext' not in source
    return read

def check_wrappers(source, read, cases):
    from keystone import Ks, KS_ARCH_X86, KS_MODE_32
    from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_WRITE
    import unicorn.x86_const as x
    ks = Ks(KS_ARCH_X86, KS_MODE_32)
    def asm(s, addr):
        return bytes(ks.asm(s, addr)[0])
    base=0x400000; wrapper=0x700000; enter=0x710000; leave=0x711000
    variables=0x720000; stop=0x730000; stack=0x810000
    obj=0x900000; player=0x910000
    regs=[x.UC_X86_REG_EAX,x.UC_X86_REG_ECX,x.UC_X86_REG_EDX,x.UC_X86_REG_EBX,
          x.UC_X86_REG_ESP,x.UC_X86_REG_EBP,x.UC_X86_REG_ESI,x.UC_X86_REG_EDI]
    extra=[x.UC_X86_REG_EFLAGS,x.UC_X86_REG_MXCSR,x.UC_X86_REG_FPCW,
           x.UC_X86_REG_FPSW,x.UC_X86_REG_FPTAG]+[getattr(x,'UC_X86_REG_XMM'+str(i)) for i in range(8)]+[getattr(x,'UC_X86_REG_FP'+str(i)) for i in range(8)]
    # Actual original function, with only its deep activation mocked identically
    # in reference and instrumented runs. Test includes both conditional gates.
    native=base+0x1E9F40
    callee=base+0x1E9C70
    mock=asm('mov eax,[ecx+0x64]; mov [ecx+0x60],eax; and dword ptr [ecx+8],0xfffffffe; mov edx,0x13579bdf; ret',callee)
    # Deliberately perturb all caller-saved GPRs, flags/DF, x87 and XMM0/MXCSR
    # inside C++ observer stubs; ABI-preserved EBX/ESI/EDI/EBP stay intact.
    callback=asm('mov eax,0xabcdef01; mov ecx,0x23456789; mov edx,0x98765432; '
                 'fninit; xorps xmm0,xmm0; ldmxcsr [0x720008]; stc; std; ret', enter)
    def body(name):
        m=re.search(r'__declspec\(naked\) void '+name+r'\(\)\s*\{\s*__asm\s*\{(.*?)\}\s*\}',source,re.S)
        assert m, name
        s=m[1]
        for symbol, addr in [('S67PromotionEnter',enter),('S67WriterEnter',enter),
                             ('S67PromotionLeave',leave),('S67WriterLeave',leave),
                             ('g_s67Native',variables),('g_s67Resume',variables+4)]:
            s=s.replace(symbol,hex(addr))
        s=re.sub(r'\b([0-9][0-9A-Fa-f]*)h\b',lambda m:hex(int(m[1],16)),s)
        return asm(s,wrapper)
    rng=random.Random(67)
    for kind in (0,1):
        code=body('S67PromotionHook' if kind==0 else 'S67WriterHook')
        for case in range(cases):
            initial=[rng.getrandbits(32) for _ in regs]
            initial[1]=obj;initial[4]=stack;initial[6]=player
            # Include DF, arithmetic FLAGS and valid x87/MXCSR modes.
            flags=2 | (rng.getrandbits(12)&0xCD5)
            xmm=[rng.getrandbits(128) for _ in range(8)]
            # Covers armed/disarmed, empty/nonempty deferred and bit0 on/off.
            pending=0x2F if case%3==0 else rng.randrange(0x800)
            deferred=0xFFFFFFFF if case%4==0 else (0x25 if case%3==0 else rng.randrange(0x800))
            gate=0 if case%4==1 else rng.randrange(1,256)
            state=bytearray(0x100)
            for off,val in [(8,case%2),(0x30,gate),(0x60,0x48),(0x64,pending),(0x68,deferred)]:
                struct.pack_into('<I',state,off,val)
            old=rng.getrandbits(32)
            value=0 if case%2==0 else rng.getrandbits(32)
            if kind: initial[2]=value
            outcomes=[]
            for wrapped in (False,True):
                u=Uc(UC_ARCH_X86,UC_MODE_32)
                for addr,size in [(0x400000,0x200000),(0x700000,0x40000),(0x800000,0x20000),(0x900000,0x20000)]: u.mem_map(addr,size)
                u.mem_write(obj,bytes(state));u.mem_write(player+0x3F4C,struct.pack('<I',old))
                u.mem_write(native,read(0x1E9F40,0x2D));u.mem_write(callee,mock)
                u.mem_write(wrapper,code);u.mem_write(enter,callback);u.mem_write(leave,callback)
                u.mem_write(variables,struct.pack('<III',native,stop,0x1F80))
                u.mem_write(stack,struct.pack('<I',stop))
                setup=asm('fninit; fld1; fldpi; fldl2e; jmp '+hex(stop),0x712000)
                u.mem_write(0x712000,setup);u.emu_start(0x712000,stop,count=10)
                for reg,val in zip(regs,initial):u.reg_write(reg,val)
                u.reg_write(x.UC_X86_REG_EFLAGS,flags)
                u.reg_write(x.UC_X86_REG_MXCSR,0x1F80|((case%4)<<13))
                u.reg_write(x.UC_X86_REG_FPCW,0x37F|((case%4)<<10))
                for i,val in enumerate(xmm):u.reg_write(getattr(x,'UC_X86_REG_XMM'+str(i)),val)
                visits=[];writes=[];native_calls=[]
                def observe(uc,addr,size,_):
                    if addr==native:native_calls.append(addr)
                    if addr in (enter,leave):
                        esp=uc.reg_read(x.UC_X86_REG_ESP)
                        saved=struct.unpack('<I',uc.mem_read(esp+4,4))[0]
                        snap=struct.unpack('<9I',uc.mem_read(saved,36))
                        visits.append((addr,snap,bytes(uc.mem_read(obj,0x70)),struct.unpack('<I',uc.mem_read(player+0x3F4C,4))[0]))
                def write(uc,access,addr,size,val,_):
                    if addr==player+0x3F4C:writes.append(val&0xFFFFFFFF)
                u.hook_add(UC_HOOK_CODE,observe);u.hook_add(UC_HOOK_MEM_WRITE,write)
                if wrapped:start=wrapper
                elif kind==0:start=native
                else:
                    start=wrapper;u.mem_write(wrapper,asm('mov dword ptr [esi+0x3f4c],edx; jmp '+hex(stop),wrapper))
                u.emu_start(start,stop,count=500)
                assert u.reg_read(x.UC_X86_REG_EIP)==stop, 'did not terminate'
                outcomes.append(([u.reg_read(r) for r in regs+extra],bytes(u.mem_read(obj,0x70)),bytes(u.mem_read(player+0x3F4C,4))))
                assert len(native_calls)==(1 if kind==0 else 0)
                assert writes==([value] if kind else [])
                if wrapped:
                    assert [v[0] for v in visits]==[enter,leave]
                    # saved PUSHAD is EDI,ESI,EBP,ESP-after-PUSHFD,EBX,EDX,ECX,EAX.
                    before=visits[0][1]
                    assert before[6]==obj and before[1]==player and before[5]==initial[2]
                    assert before[8]==flags
                    if kind: assert visits[0][3]==old and visits[1][3]==value
                    else: assert visits[0][2]==bytes(state[:0x70])
            assert outcomes[0]==outcomes[1], (kind,case,'register/FLAGS/FPU/SSE/stack/memory mismatch')
    print(f'PASS: {cases*2} differential cases, actual extracted wrappers; native call/store exactly once')

def main():
    ap=argparse.ArgumentParser();ap.add_argument('exe',type=Path)
    ap.add_argument('--cases',type=int,default=256)
    args=ap.parse_args()
    source=(ROOT/'Stage67_Overlay/src/Stage67NativePaths.inl').read_text()
    adapter=(ROOT/'Stage67_Overlay/src/Stage38CallbackWindow.inl').read_text()
    read=check_static(args.exe,source,adapter)
    print('PASS: SHA/PE, six runtime guard arrays, source global, adapter order, no DR writes')
    check_wrappers(source,read,args.cases)
    print('SOURCE ONLY: no MSVC build or runtime verification claimed')

if __name__=='__main__':main()
