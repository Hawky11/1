"""Static signatures plus x86 wrapper ABI test; requires pefile/unicorn and GNU binutils.
Usage: python verify_stage25.py /path/to/AALib.exe
This does not compile the Windows C++ translation unit.
"""
import hashlib,random,re,struct,subprocess,sys,tempfile
from pathlib import Path
import pefile
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE
from unicorn.x86_const import *
exe=Path(sys.argv[1]); raw=exe.read_bytes()
assert hashlib.sha256(raw).hexdigest()=='28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
pe=pefile.PE(data=raw)
assert pe.get_data(0xF58C1,17)==bytes.fromhex('E8 3A 96 FA FF 83 C4 08 83 88 F8 00 00 00 01 5D C3')
assert pe.get_data(0xEE2F0,12)==bytes.fromhex('8B 8E 34 3F 00 00 8B 96 30 3F 00 00')
source=(Path(__file__).resolve().parents[1]/'src/AIQueueTrace.cpp').read_text()
a=source.index('__declspec(naked) void Stage25SecondaryObjectFlagHook()')
asm=source[source.index('__asm',a):source.index('\n    }',a)]
asm=asm[asm.index('{')+1:asm.rindex('}')]
asm=asm.replace('0F8h','0xF8').replace('ArcanaLogStage25SecondaryObjectFlag','logger')
with tempfile.TemporaryDirectory() as td:
 p=Path(td);(p/'t.s').write_text('.intel_syntax noprefix\n.code32\n.global _start\n_start:\n'+asm+'\nlogger:\nxor eax,eax\nxor ecx,ecx\nxor edx,edx\nret\n')
 subprocess.run(['as','--32','-o',str(p/'t.o'),str(p/'t.s')],check=True)
 subprocess.run(['ld','-m','elf_i386','-Ttext=0x100000','-o',str(p/'t.elf'),str(p/'t.o')],check=True)
 subprocess.run(['objcopy','-O','binary','--only-section=.text',str(p/'t.elf'),str(p/'t.bin')],check=True)
 code=(p/'t.bin').read_bytes()
 regs=[UC_X86_REG_EAX,UC_X86_REG_EBX,UC_X86_REG_ECX,UC_X86_REG_EDX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP]
 rng=random.Random(25)
 for n in range(128):
  before=rng.getrandbits(32);vals=[0x200000]+[rng.getrandbits(32) for _ in regs[1:]];flags=0x202 | (rng.getrandbits(12)&0xCD5)
  results=[]; captured=[]
  for wrapped in [False,True]:
   u=Uc(UC_ARCH_X86,UC_MODE_32)
   for base in [0x100000,0x200000,0x300000]:u.mem_map(base,0x10000)
   u.mem_write(0x100000,code if wrapped else bytes.fromhex('83 88 F8 00 00 00 01'))
   u.mem_write(0x2000F8,struct.pack('<I',before))
   for reg,value in zip(regs,vals):u.reg_write(reg,value)
   u.reg_write(UC_X86_REG_EFLAGS,flags);u.reg_write(UC_X86_REG_ESP,0x308000-(4 if wrapped else 0))
   if wrapped:
    u.mem_write(0x307FFC,struct.pack('<I',0x101000))
    # Logger begins after the first RET. Verify exact PUSHAD context/local offsets.
    logger=0x100000+code.index(b'\xC3')+1
    def on_code(uc,address,size,data):
     if address==logger:
      esp=uc.reg_read(UC_X86_REG_ESP);ctx=struct.unpack('<I',uc.mem_read(esp+4,4))[0]
      c=struct.unpack('<9I',uc.mem_read(ctx,36));assert c[7]==vals[0] and c[2]==vals[-1]
      assert struct.unpack('<I',uc.mem_read(c[3]+4,4))[0]==before
      assert struct.unpack('<I',uc.mem_read(c[3]+8,4))[0]==0x101000
      captured.append(True)
    u.hook_add(UC_HOOK_CODE,on_code);u.emu_start(0x100000,0x101000,count=100)
   else:u.emu_start(0x100000,0x100007,count=1)
   results.append(([u.reg_read(r) for r in regs],u.reg_read(UC_X86_REG_ESP),u.reg_read(UC_X86_REG_EFLAGS),bytes(u.mem_read(0x2000F8,4))))
  assert results[0]==results[1],(n,results)
  assert captured==[True]
print('PASS: fingerprint, signatures, argument loads, 128 x86 cases: registers/ESP/EFLAGS/output/context match original OR.')
