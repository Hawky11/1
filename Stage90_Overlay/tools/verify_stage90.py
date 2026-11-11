#!/usr/bin/env python3
"""Stage90 SHA-pinned native F57D0 bitmap-clearing static verification.
Source-only verification; no Windows build/runtime claims.
Usage: python verify_stage90.py /path/to/AALib.zip --source /path/to/stage90
"""
import argparse
import hashlib
import json
import struct
import zipfile
from pathlib import Path

EXE_SHA='28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba'
BASELINES={
 'Stage81SelectorWriter.inl':'ace3b4d3a38fccf40813ea1d923ae5b933ef7d9a49c9c4f4e2c4598cf4404088',
 'Stage88Stage24Join.inl':'f5d0e2ca436398575309b91fb87977e820609810f88d42a2a677ef6c528e3fbc',
}
SITES={
 0xEE2FF:'8d86083a000050e8c5740000',  # Existing Stage24 EE306->F57D0 one CALL.
 0xF57D0:'558bec8b45148b4d10535657',
 0xF57EC:'f6847688f1dd0401', # table[code*3] flag & 1
 0xF57F6:'7408', # skip clearing if flag=0
 0xF57F8:'33c0', # EAX=0
 0xF57FA:'894704', # output+4=0 = P+3A0C
 0xF57FD:'894708', # output+8=0 = P+3A10
 0xF5800:'8937', # output+0=code
 0xF5812:'83f926', # descriptor special bit index 0x26
 0xF5856:'0910', # OR special bit into bitset
 0xF5868:'83f926', # secondary descriptor special bit index
 0xF58AC:'0910', # OR secondary bit into bitset
 0xF58EF:'8b4e04f7c100020000', # downstream F58E0 reads bitmap+4 and tests bit0x200
 0xF58FF:'f7c100040000', # tests bit0x400
 0xF5907:'f7c100080000', # tests bit0x800
}

def load(path):
 path=Path(path)
 if path.suffix.lower()=='.zip':
  with zipfile.ZipFile(path) as z:return z.read('AALib.exe')
 return path.read_bytes()

def section_map(data):
 pe=struct.unpack_from('<I',data,0x3c)[0]
 assert data[:2]==b'MZ' and data[pe:pe+4]==b'PE\0\0'
 assert struct.unpack_from('<H',data,pe+4)[0]==0x14c,'Expected x86 binary'
 assert struct.unpack_from('<H',data,pe+24)[0]==0x10b,'Expected PE32'
 secn,optsize=struct.unpack_from('<H',data,pe+6)[0],struct.unpack_from('<H',data,pe+20)[0]
 for j in range(secn):
  off=pe+24+optsize+40*j
  virtual_size,rva,raw_size,raw_off=struct.unpack_from('<IIII',data,off+8)
  yield rva,max(virtual_size,raw_size),raw_off

def at(data,sections,rva,length):
 for va,size,raw in sections:
  if va<=rva and rva+length<=va+size:
   return data[raw+rva-va:raw+rva-va+length]
 raise AssertionError(f'RVA {rva:X} not mapped')

def verify(binary,source):
 data=load(binary)
 sha=hashlib.sha256(data).hexdigest()
 assert sha==EXE_SHA,f'Wrong SHA: {sha}'
 sec=list(section_map(data));checks=0
 for rva,b in SITES.items():
  expected=bytes.fromhex(b)
  assert at(data,sec,rva,len(expected))==expected,f'Signature error RVA {rva:X}'
  checks+=1
 raw=at(data,sec,0xEE306,5)
 assert raw==b'\xe8\xc5\x74\x00\x00'
 assert 0xEE306+5+struct.unpack_from('<i',raw,1)[0]==0xF57D0
 checks+=2
 descriptor={}
 for code in (0x0e,0x0f,0x10):
  d=at(data,sec,0x049DF188+3*code,3)
  assert d==bytes.fromhex('0126ff'),f'Descriptor for {code:X}: {d.hex()}'
  descriptor[f'0x{code:02X}']=d.hex()
  checks+=1
 paths={name:source/'src'/name for name in BASELINES}
 assert all(p.exists() for p in paths.values()),'Stage90 package missing overlay source'
 stage81=paths['Stage81SelectorWriter.inl'].read_text()
 bridge=paths['Stage88Stage24Join.inl'].read_text()
 stage90=(source/'src'/'Stage90AllF57D0.inl').read_text()
 assert stage81.count('#include "Stage90AllF57D0.inl"')==1
 assert stage81.count('arcana_stage90::Start();')==1
 assert stage81.count('arcana_stage90::Flush();')==1
 assert stage81.count('arcana_stage90::Health();')==1
 assert bridge.count('arcana_stage90::Pre(frame);arcana_stage88::Pre(frame);')==1
 assert bridge.count('arcana_stage88::Post(saved);arcana_stage90::Post(saved);')==1
 assert 'Stage88Stage24Pre' in bridge and 'Stage88Stage24Post' in bridge
 assert 'noNewHook=1' in stage90 and 'kSpecialBits=0x00000E00u' in stage90
 assert '#include "Stage89F58E0Reader.inl"' in stage81
 checks+=10
 return {'status':'PASS_STATIC_SOURCE_ONLY','exe_sha256':sha,'checks':checks,
   'descriptor_bytes':descriptor,'existing_call':'EE306 -> F57D0',
   'actual_clear_instructions':['0xF57FA [EDI+4]=0','0xF57FD [EDI+8]=0'],
   'writeback':['0xF5856 OR bitmap bit','0xF58AC OR bitmap bit'],
   'stage90_uses_existing_stage24_pre_post':True,'new_native_hooks':0,
   'windows_compiled':False,'runtime_tested':False}

def main():
 p=argparse.ArgumentParser();p.add_argument('exe_or_zip');p.add_argument('--source',type=Path,default=Path(__file__).resolve().parents[1]);p.add_argument('--output',type=Path)
 args=p.parse_args();r=verify(args.exe_or_zip,args.source)
 body=json.dumps(r,ensure_ascii=False,indent=2)+'\n'
 print(body,end='')
 if args.output:args.output.write_text(body,encoding='utf-8')
if __name__=='__main__':main()
