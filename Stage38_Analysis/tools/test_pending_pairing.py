#!/usr/bin/env python3
"""Compile real C++ pairing/snapshot code with stubbed OS APIs (not Windows build)."""
from pathlib import Path
import re,subprocess,tempfile
s=(Path(__file__).resolve().parents[2]/'Stage38_Overlay/src/Stage38CallbackWindow.inl').read_text()
s=s[:s.index('__declspec(naked)')]
s=re.sub(r'void AppendStage38Log\(.*?\n}\n','',s,flags=re.S)
s=s.replace('__declspec(thread)','thread_local').replace('__cdecl','')
pre='''#include <cstdint>
#include <cassert>
#include <unordered_map>
using DWORD=uint32_t; using LONG=int32_t; using ULONGLONG=uint64_t;
using SRWLOCK=int;
#define SRWLOCK_INIT 0
DWORD g_playerP1=0x10000,g_playerP2=0x20000;
std::unordered_map<DWORD,DWORD> mem;
bool SafeReadDword(DWORD a,DWORD* v) { auto it=mem.find(a); if(it==mem.end()) return false; *v=it->second; return true; }
LONG InterlockedIncrement(volatile LONG* p){ return ++*p; }
void AcquireSRWLockExclusive(SRWLOCK*){} void ReleaseSRWLockExclusive(SRWLOCK*){}
DWORD GetCurrentThreadId(){return 7;} ULONGLONG Stage31Qpc(){static ULONGLONG q=0;return ++q;}
'''
post='''
int main(){
 DWORD a[12]={}; a[0]=0x34567; a[1]=g_playerP1; a[2]=0x30038; a[10]=0x30000; a[11]=g_playerP1;
 mem[a[10]]=g_playerP1; mem[g_playerP1+4]=0x40000; mem[0x40000+0x278]=1;
 Stage38Prepared(a); mem[0x40000+0x278]=2; Stage38Finished(a);
 assert(g_stage38Count==1 && g_stage38PendingDepth==0);
 assert(g_stage38Ring[0].Pre[28]==1 && g_stage38Ring[0].Post[28]==2);
 assert(g_stage38Ring[0].PreMask==((1u<<28)|1u));
 DWORD b[12]; for(int i=0;i<12;i++)b[i]=a[i]; b[2]+=0x100; b[10]+=0x100; mem[b[10]]=g_playerP1;
 Stage38Prepared(a); Stage38Prepared(b); Stage38Finished(b); Stage38Finished(a);
 assert(g_stage38Count==3 && g_stage38PendingDepth==0 && g_stage38Mismatch==0);
 a[0]=0; Stage38Prepared(a); Stage38Finished(a); assert(g_stage38Ring[3].Callback==0);
 Stage38Finished(a); assert(g_stage38Mismatch==1);
 for(unsigned i=0;i<kStage38MaxDepth+1;i++) Stage38Prepared(a);
 for(unsigned i=0;i<kStage38MaxDepth+1;i++) Stage38Finished(a);
 assert(g_stage38Overflow==1 && g_stage38PendingDepth==0);
 g_stage38Count=kStage38Capacity; auto seq=g_stage38Sequence;
 Stage38Prepared(a); Stage38Finished(a); assert(g_stage38Dropped==1 && g_stage38Sequence==seq+1);
 a[11]=0; Stage38Prepared(a); Stage38Finished(a); assert(g_stage38Mismatch==2 && g_stage38PendingDepth==0);
 a[1]=0; Stage38Prepared(a); Stage38Finished(a); assert(g_stage38PendingDepth==0);
}
'''
with tempfile.TemporaryDirectory() as t:
 p=Path(t); (p/'test.cpp').write_text(pre+s+post)
 subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('PASS real snapshot/pairing C++: nested, zero target, unreadable masks, overflow, mismatch, full-ring drop and sequence gap, unknown player')
