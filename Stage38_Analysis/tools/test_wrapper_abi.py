#!/usr/bin/env python3
"""GNU as + Unicorn x86 harness for exact inline-assembly wrapper bodies.
Requires unicorn; does NOT compile MSVC C++ or validate live Windows execution.
"""
from pathlib import Path
import re,subprocess,tempfile,struct
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import *
source=(Path(__file__).resolve().parents[2]/'Stage38_Overlay/src/Stage38CallbackWindow.inl').read_text()
regs=[UC_X86_REG_EAX,UC_X86_REG_EBX,UC_X86_REG_ECX,UC_X86_REG_EDX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP,UC_X86_REG_ESP,UC_X86_REG_EFLAGS,UC_X86_REG_XMM0,UC_X86_REG_MXCSR]
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp)
 blocks=[]
 for name in ['Stage38PrepareHook','Stage38FinishHook']:
  body=re.search(r'void '+name+r'\(\)\s*\{\s*__asm\s*\{(.*?)\n    \}',source,re.S)[1]
  body=re.sub(r'//[^\n]*','',body)
  body=re.sub(r'\b([0-9][0-9A-Fa-f]*)h\b',lambda m:'0x'+m[1],body)
  blocks.append(name+':\n'+body)
 asm='.intel_syntax noprefix\n.code32\n.text\n.global Stage38PrepareHook\n'+ '\n'.join(blocks)+'''
NativePrepare:
 mov eax,[esp+4]
 mov ecx,[esp+8]
 mov [eax],ecx
 xor edx,edx
 mov eax,0x76543210
 fld1
 pcmpeqd xmm0,xmm0
 ret
NativeFinish:
 mov eax,[esp+4]
 mov ecx,[eax]
 xor edx,edx
 fld1
 pcmpeqd xmm0,xmm0
 ret
Stage38Prepared:
 fninit
 fldz
 pxor xmm0,xmm0
 xor eax,eax
 xor ecx,ecx
 xor edx,edx
 ret
Stage38Finished:
 fninit
 fldz
 pxor xmm0,xmm0
 xor eax,eax
 xor ecx,ecx
 xor edx,edx
 ret
SaveFP:
 fxsave [0x400000]
 nop
.data
g_stage38PrepareTarget: .long NativePrepare
g_stage38FinishTarget: .long NativeFinish
'''
 (p/'a.s').write_text(asm)
 subprocess.run(['as','--32','-o',str(p/'a.o'),str(p/'a.s')],check=True)
 subprocess.run(['ld','-m','elf_i386','-Ttext=0x100000','-Tdata=0x200000','-e','Stage38PrepareHook','-o',str(p/'a.elf'),str(p/'a.o')],check=True)
 symbols={name:int(addr,16) for addr,typ,name in (l.split() for l in subprocess.check_output(['nm',str(p/'a.elf')],text=True).splitlines())}
 for sec in ['text','data']:subprocess.run(['objcopy','-O','binary','--only-section=.'+sec,str(p/'a.elf'),str(p/sec)],check=True)
 def run(entry,args,flag):
  u=Uc(UC_ARCH_X86,UC_MODE_32)
  for base in [0x100000,0x200000,0x300000,0x400000]:u.mem_map(base,0x10000)
  u.mem_write(0x100000,(p/'text').read_bytes());u.mem_write(0x200000,(p/'data').read_bytes())
  for i,r in enumerate(regs[:7]):u.reg_write(r,0x12340000+i*0x111)
  u.reg_write(UC_X86_REG_ESI,0x400100);u.reg_write(UC_X86_REG_EDI,0x12345678);u.reg_write(UC_X86_REG_EBP,0x308038)
  u.reg_write(UC_X86_REG_ESP,0x308000);u.reg_write(UC_X86_REG_EFLAGS,flag)
  u.reg_write(UC_X86_REG_XMM0,0x123456789abcdef);u.reg_write(UC_X86_REG_MXCSR,0x1f80)
  u.mem_write(0x308000,struct.pack('<'+'I'*(len(args)+1),0x10f000,*args));u.mem_write(0x400200,struct.pack('<I',0x400100))
  seen=[];native=[]
  def hook(u,addr,size,extra):
   if addr in [symbols['NativePrepare'],symbols['NativeFinish']]:native.append(addr)
   if addr in [symbols['Stage38Prepared'],symbols['Stage38Finished']]:
    sp=u.reg_read(UC_X86_REG_ESP);ptr=struct.unpack('<I',u.mem_read(sp+4,4))[0]
    saved=struct.unpack('<12I',u.mem_read(ptr,48));seen.append(saved)
  u.hook_add(UC_HOOK_CODE,hook)
  u.emu_start(symbols[entry],0x10f000,count=300)
  state=[u.reg_read(r) for r in regs]
  u.emu_start(symbols['SaveFP'],symbols['SaveFP']+7,count=1)
  fp=bytes(u.mem_read(0x400000,512))
  # Unicorn does not restore FIP through FXRSTOR; exclude only its 4 bytes.
  # x87 data/control/tag and all SIMD payload are still compared.
  return state,fp,seen,native,bytes(u.mem_read(0x400200,4))
 for flag in [0x202,0x246,0x603]:
  for native,wrap,args in [('NativePrepare','Stage38PrepareHook',[0x400200,0x400100]),('NativeFinish','Stage38FinishHook',[0x400200])]:
   a=run(native,args,flag);b=run(wrap,args,flag)
   assert a[0]==b[0],(wrap,'registers',a[0],b[0])
   assert a[1][:8]+a[1][12:]==b[1][:8]+b[1][12:],(wrap,'FXSAVE except emulator FIP')
   assert a[4]==b[4] and len(b[3])==1
   assert len(b[2])==1 and list(b[2][0][10:10+len(args)])==args
   assert b[2][0][0]==0x12345678 and b[2][0][1]==0x400100 and b[2][0][2]==0x308038
 print('PASS 6 wrapper executions: native once, args, stack, GPR, EFLAGS (including DF), x87/MMX/XMM/MXCSR (FIP excluded: Unicorn), saved-context layout')
