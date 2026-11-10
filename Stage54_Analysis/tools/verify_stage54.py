#!/usr/bin/env python3
"""Static Stage54 original PE and source verifier (stdlib only)."""
import argparse, hashlib, struct, zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
SHA="28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba"
CHECKS={
 0xA2B81:"E8 FA EF FF FF",0xA2B86:"E8 F5 CF 02 00",0xA2B8B:"E8 D0 7F 01 00",
 0xA2B95:"E8 F6 CE 02 00",
 0xA1B80:"80 3D 5C 1B 0A 0E 00",
 0xCFB80:"E8 5B 06 11 00 6A 01 E8 54 06 11 00 59 C3",
 0xBAB60:"55 8B EC 81 EC C0 00 00 00 53 56 33 DB 57"}
TARGETS={0xA2B81:0xA1B80,0xA2B86:0xCFB80,0xA2B8B:0xBAB60}
def check(condition,what):
 if not condition: raise AssertionError(what)
 print("PASS",what)
def rva(data,address,n):
 pe=struct.unpack_from("<I",data,0x3C)[0]
 count=struct.unpack_from("<H",data,pe+6)[0]
 opt=struct.unpack_from("<H",data,pe+20)[0]
 first=pe+24+opt
 for i in range(count):
  p=first+i*40
  vsize,virtual,rawsize,raw=struct.unpack_from("<IIII",data,p+8)
  if virtual<=address and address+n<=virtual+max(vsize,rawsize):
   return data[raw+address-virtual:raw+address-virtual+n]
 raise AssertionError("RVA not mapped: "+hex(address))
def main():
 ap=argparse.ArgumentParser()
 ap.add_argument("--aalib",required=True,type=Path)
 args=ap.parse_args()
 if args.aalib.suffix.lower()==".zip":
  with zipfile.ZipFile(args.aalib) as z: b=z.read("AALib.exe")
 else: b=args.aalib.read_bytes()
 check(len(b)==175556608,"size")
 # Original physical PE relocation table must explicitly contain HIGHLOW A1B82.
 pe=struct.unpack_from("<I",b,0x3C)[0]
 dd=pe+24+96
 reloc_rva,reloc_size=struct.unpack_from("<II",b,dd+5*8)
 check(reloc_rva!=0 and reloc_size>0,"PE relocation directory exists")
 section_count=struct.unpack_from("<H",b,pe+6)[0]
 optional_size=struct.unpack_from("<H",b,pe+20)[0]
 section_base=pe+24+optional_size
 def offset(rv):
  for j in range(section_count):
   p=section_base+j*40
   vs,va,raw_size,raw_offset=struct.unpack_from("<IIII",b,p+8)
   if va<=rv<va+max(vs,raw_size):return raw_offset+rv-va
  raise AssertionError("unmapped RVA "+hex(rv))
 reloc_start=offset(reloc_rva)
 cursor=reloc_start
 end=reloc_start+reloc_size
 found=False
 while cursor+8<=end:
  page,size=struct.unpack_from("<II",b,cursor)
  if size<8:break
  for j in range((size-8)//2):
   entry=struct.unpack_from("<H",b,cursor+8+j*2)[0]
   if page+(entry&0x0FFF)==0xA1B82 and entry>>12==3:found=True
  cursor+=size
 check(found,"A1B82 HIGHLOW base relocation present")
 original_abs=struct.unpack_from("<I",b,offset(0xA1B82))[0]
 check(original_abs==0x0E0A1B5C,"original A1B82 operand")
 check((original_abs+(0x00800000-0x00400000))&0xFFFFFFFF==0x0E4A1B5C,"expected rebased absolute operand")

 check(hashlib.sha256(b).hexdigest()==SHA,"exact original SHA256")
 pe=struct.unpack_from("<I",b,0x3c)[0]
 check(b[pe:pe+4]==b"PE\x00\x00","PE header")
 check(struct.unpack_from("<H",b,pe+24)[0]==0x10b,"PE32")
 check(struct.unpack_from("<I",b,pe+24+28)[0]==0x400000,"ImageBase")
 for rv,sig in CHECKS.items():check(rva(b,rv,len(bytes.fromhex(sig)))==bytes.fromhex(sig),f"native RVA {rv:06X}")
 for site,target in TARGETS.items():
  ins=rva(b,site,5);rel=struct.unpack("<i",ins[1:])[0]
  check(ins[0]==0xE8 and site+5+rel==target,f"CALL {site:06X}->{target:06X}")
 src=(ROOT/"Stage54_Overlay/src/Stage54PreParentCoordinate.inl").read_text(encoding="utf-8")
 adapter=(ROOT/"Stage54_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
 for name in ("S54A1B80Hook","S54CFB80Hook","S54BAB60Hook","InstallStage54Hooks",
              "STAGE54_INSTALLED 3/3","STAGE54_SIBLING_WINDOW","STAGE54_HEALTH",
              "g_s53Installed","SuspendOtherProcessThreads","ResumeThreads"):
  check(name in src,"source "+name)
 for letter in "ABC":
  check(src.count(f"call dword ptr [g_s54Native{letter}]")==1,f"native {letter} once")
 check(src.count("fxsave [edi]")==6 and src.count("fxrstor [edi]")==6,"six paired FXSAVE restorations")
 check('#include "Stage53ParentCoordinate.inl"\n#include "Stage54PreParentCoordinate.inl"' in adapter,"adapter includes Stage53+54")
 check("if(InstallStage53Hooks())InstallStage54Hooks();" in adapter,"installation after Stage53")
 check("FlushStage53();\n    FlushStage54();" in adapter,"flush after Stage53")
 for needle in ("STAGE54_PRE_PARENT_COORD_BUILD=2026-11-10-r2","0xA1B82u",
                "actualAOperand==expectedAOperand","0x0E0A1B5Cu+(g_gameBase-0x00400000u)",
                "callsValid[i]=same(rvas[i],originals[i],5u)","STAGE54_ASLR_CHECK",
                "STAGE54_SIGNATURE_FAILURE"):
  check(needle in src,"r2 source "+needle)
 check("aPrologue,sizeof(aPrologue)" not in src,"no unrelocated compare")
 print("Stage54 r2 original PE/static source PASS; Windows x86 compile/runtime PENDING.")
if __name__=="__main__":
 try:main()
 except (OSError,ValueError,AssertionError,struct.error,KeyError) as e:
  raise SystemExit(f"FAIL: {e}")
