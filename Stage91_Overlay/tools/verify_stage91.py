#!/usr/bin/env python3
"""Stage91 original PE32 and C++ hook source-only verifier. No Windows execution claims."""
import argparse
import hashlib
import json
import struct
import zipfile
from pathlib import Path

EXE_SHA='28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
SITES=(
 (0xF5A4D,9,bytes.fromhex('8dbe083a00006a0957'),bytes.fromhex('83c40884c0750f')),
 (0xF5A5C,10,bytes.fromhex('6a0a57'),bytes.fromhex('83c40884c0740b')),
 (0xF5A76,11,bytes.fromhex('6a0b57'),bytes.fromhex('83c4086a038bce84c074')),
)
CALLS=(bytes.fromhex('e88efeffff'),bytes.fromhex('e87ffeffff'),bytes.fromhex('e865feffff'))

def load(p):
 p=Path(p)
 if p.suffix.lower()=='.zip':
  with zipfile.ZipFile(p) as z:return z.read('AALib.exe')
 return p.read_bytes()

def sections(raw):
 pe=struct.unpack_from('<I',raw,0x3c)[0]
 assert raw[:2]==b'MZ' and raw[pe:pe+4]==b'PE\0\0'
 assert struct.unpack_from('<H',raw,pe+4)[0]==0x14c
 assert struct.unpack_from('<H',raw,pe+24)[0]==0x10b
 n,opt=struct.unpack_from('<H',raw,pe+6)[0],struct.unpack_from('<H',raw,pe+20)[0]
 for i in range(n):
  p=pe+24+opt+40*i
  virtual,rva,size,off=struct.unpack_from('<IIII',raw,p+8)
  yield (rva,max(virtual,size),off)

def at(raw,secs,rva,n):
 for base,size,off in secs:
  if base<=rva and rva+n<=base+size:
   return raw[off+rva-base:off+rva-base+n]
 raise AssertionError(f'RVA 0x{rva:X} not mapped')

def verify(raw,root):
 sha=hashlib.sha256(raw).hexdigest()
 assert sha==EXE_SHA,f'wrong executable SHA {sha}'
 sec=list(sections(raw));checks=0
 # A real update path invokes F5990 per actor. Native F5990 computes bitmap pointer.
 assert at(raw,sec,0xACAB8,5)==bytes.fromhex('e8d38e0400')
 assert 0xACAB8+5+struct.unpack_from('<i',at(raw,sec,0xACAB8,5),1)[0]==0xF5990
 checks+=2
 assert at(raw,sec,0xF5A3F,3)==bytes.fromhex('f6c201') # guard in P+3B70
 assert at(raw,sec,0xF5A44,6)==bytes.fromhex('8dbe083a0000') # P+3A08
 checks+=2
 # Prove F58E0 generic (non-0x26) bit extraction: index &31, 1<<index, [bitmap+index/32*4+4].
 core={0xF58E0:'558bec8b450c568b7508',0xF58EA:'83f826',0xF590F:'8bc8',0xF5923:'bb01000000',0xF5928:'d3e3',0xF592A:'03c2',0xF592C:'c1f805',0xF592F:'235c8604'}
 for addr,hx in core.items():
  expect=bytes.fromhex(hx)
  assert at(raw,sec,addr,len(expect))==expect,f'generic predicate signature {addr:X}'
  checks+=1
 for i,(site,index,pre,post) in enumerate(SITES):
  call=at(raw,sec,site,5)
  assert call==CALLS[i],f'CALL signature failed 0x{site:X}'
  checks+=1
  assert site+5+struct.unpack_from('<i',call,1)[0]==0xF58E0
  checks+=1
  assert at(raw,sec,site-len(pre),len(pre))==pre,f'PRE signature failed 0x{site:X}'
  checks+=1
  assert at(raw,sec,site+5,len(post))==post,f'POST signature failed 0x{site:X}'
  checks+=1
  assert at(raw,sec,site-3,3)==bytes((0x6A,index,0x57))
  checks+=1
  assert index-9==i and (1<<index)==(0x200<<i)
  checks+=1
 adapter=(root/'src/Stage81SelectorWriter.inl').read_text()
 source=(root/'src/Stage91DirectSpecialReader.inl').read_text()
 assert adapter.count('#include "Stage91DirectSpecialReader.inl"')==1
 assert adapter.count('arcana_stage91::Install();')==1
 assert adapter.count('arcana_stage91::Flush();')==1
 assert adapter.count('arcana_stage91::Health();')==1
 checks+=4
 for item in ('arcana_stage90::Start();','arcana_stage89::Install();','arcana_stage88::Start();','#include "Stage90AllF57D0.inl"','#include "Stage89F58E0Reader.inl"'):
  assert item in adapter,item
  checks+=1
 for item in ('kSites[3]={0xF5A4Du,0xF5A5Cu,0xF5A76u}', 'kCall[3][5]', 'kBefore[3][9]', 'kAfter[3][10]', 'Stage91Before', 'Stage91After', 'Stage91Hook0()', 'Stage91Hook1()', 'Stage91Hook2()', 'GetCurrentThreadId()', 'QueryPerformanceCounter(&t)', 'g_Stage91F58E0NativeTarget=g_gameBase+0xF58E0u', 'return (index>=9u&&index<=11u&&(values[1]&(1u<<index)))?1u:0u;', 'STAGE91_INSTALLED', 'STAGE91_F58E0', 'STAGE91_HEALTH','Stage91ReaderSiteMask', 'EnableStage91DirectSpecialReaderTrace'):
  assert item in source,item
  checks+=1
 for hookname in ('Stage91Hook0()','Stage91Hook1()','Stage91Hook2()'):
  start=source.index(hookname);end=source.index('\n}\n',start)
  part=source[start:end]
  assert part.count('call dword ptr [g_Stage91F58E0NativeTarget]')==1
  assert part.count('fxsave [esp]')==2 and part.count('fxrstor [esp]')==2
  assert 'pushfd' in part and 'popfd' in part and 'pushad' in part and 'popad' in part
  checks+=3
 assert 'Stage91Hook3' not in source
 assert 'STAGE89_' not in source
 checks+=2
 # PE32 only; strict target bytes + source guards. These are static tests, not compiler tests.
 return {'status':'PASS_STATIC_SOURCE_ONLY','checks':checks,'exe_sha256':sha,
 'native_function':'F58E0','upstream_actor_tick':'ACAB8->F5990',
 'sites':[{'rva':f'0x{s:X}','index':i,'bit':f'0x{1<<i:X}'} for s,i,_,_ in SITES],
 'global_f58e0_detour':False,'new_EE306_hook':False,'game_data_writes':False,
 'windows_compiled':False,'game_runtime_tested':False}

def main():
 ap=argparse.ArgumentParser()
 ap.add_argument('exe_or_zip');ap.add_argument('--source',type=Path,default=Path(__file__).resolve().parents[1]);ap.add_argument('--output',type=Path)
 a=ap.parse_args();r=verify(load(a.exe_or_zip),a.source)
 s=json.dumps(r,ensure_ascii=False,indent=2)+'\n'
 print(s,end='')
 if a.output:a.output.write_text(s)
if __name__=='__main__':main()
