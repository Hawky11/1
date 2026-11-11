#!/usr/bin/env python3
"""Verify PE + original Stage89 callsite and F58E0 bytes; no runtime patching.

usage: python verify_stage89.py /path/to/AALib.zip
"""
import argparse,hashlib,json,struct,zipfile
from pathlib import Path
EXPECTED='28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
SITES={
 0x9A870:('05083a00006a2650','e86bb00500','83c40884c0744e'),
 0x9AA20:('05083a00006a2650','e8bbae0500','83c40884c07436'),
 0x9AB00:('05083a00006a2650','e8dbad0500','83c4085684c0743f'),
 0xAAB55:('8d86083a00006a2650','e886ad0400','83c40884c07511'),
}
NATIVE={
 0xF58E0:'558bec8b450c568b7508',
 0xF58EA:'83f826',
 0xF58EF:'8b4e04f7c1000200007405b0015e5dc3f7c1',
 0xF58FF:'f7c10004000075f3',
 0xF5907:'f7c10008000075eb',
 0xF592F:'235c8604',
}

def read_exe(path):
 p=Path(path)
 if p.suffix.lower()=='.zip':
  with zipfile.ZipFile(p) as z:return z.read('AALib.exe')
 return p.read_bytes()

def sections(data):
 pe=struct.unpack_from('<I',data,0x3c)[0]
 assert data[:2]==b'MZ' and data[pe:pe+4]==b'PE\0\0','Invalid PE32 header'
 assert struct.unpack_from('<H',data,pe+4)[0]==0x14c,'Not Windows x86'
 assert struct.unpack_from('<H',data,pe+24)[0]==0x10b,'Not PE32 optional header'
 n,sz=struct.unpack_from('<HH',data,pe+6)[0],struct.unpack_from('<H',data,pe+20)[0]
 for i in range(n):
  o=pe+24+sz+40*i
  vsize,rva,rawsize,rawpos=struct.unpack_from('<IIII',data,o+8)
  yield (rva,max(vsize,rawsize),rawpos)

def rva_bytes(data,sec,rva,n):
 for va,size,raw in sec:
  if va<=rva and rva+n<=va+size:return data[raw+rva-va:raw+rva-va+n]
 raise AssertionError(f'Unmapped RVA 0x{rva:06x}')

def verify(path):
 data=read_exe(path);sha=hashlib.sha256(data).hexdigest()
 assert sha==EXPECTED,f'Wrong AALib.exe SHA256: {sha}'
 sec=list(sections(data));checked=0
 for r,txt in NATIVE.items():
  sig=bytes.fromhex(txt)
  assert rva_bytes(data,sec,r,len(sig))==sig,f'Native bytes mismatch at 0x{r:X}'
  checked+=1
 for site,(p,op,s) in SITES.items():
  for addr,txt in [(site-len(bytes.fromhex(p)),p),(site,op),(site+5,s)]:
   sig=bytes.fromhex(txt)
   assert rva_bytes(data,sec,addr,len(sig))==sig,f'Site signature mismatch at 0x{addr:X}'
   checked+=1
  e8=bytes.fromhex(op)
  assert site+5+struct.unpack_from('<i',e8,1)[0]==0xF58E0,f'Wrong call destination at 0x{site:X}'
  checked+=1
 return {'status':'PASS_STATIC_SOURCE_ONLY','exe_sha256':sha,'signature_checks':checked,
   'f58e0_rva':'0xF58E0','sites':[f'0x{s:X}' for s in SITES],
   'native_calls_once_source_required':True,'windows_build_tested':False,'game_runtime_tested':False}

def main():
 p=argparse.ArgumentParser();p.add_argument('exe_or_zip');p.add_argument('--output')
 a=p.parse_args();result=verify(a.exe_or_zip)
 body=json.dumps(result,indent=2,ensure_ascii=False)+'\n'
 print(body,end='')
 if a.output:Path(a.output).write_text(body,encoding='utf-8')
if __name__=='__main__':main()
