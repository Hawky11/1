#!/usr/bin/env python3
"""Compile extracted Stage31 C++ record logic with host mocks (not Windows ABI)."""
import pathlib,subprocess,tempfile
src=(pathlib.Path(__file__).resolve().parents[1]/'src/AIQueueTrace.cpp').read_text()
logic=src[src.index('    // Stage31: bounded'):src.index('    // Both diagnostic calls preserve')]
queue=src[src.index('        Stage31Record stage31{};'):src.index('        if (traceThisCall)\n',src.index('        Stage31Record stage31{};'))]
header=r'''
#include <cstdint>
#include <cassert>
#include <map>
#include <thread>
#include <iostream>
#define __declspec(x) thread_local
#define __cdecl
using DWORD=uint32_t; using BYTE=uint8_t; using WORD=uint16_t;
using LONG=int32_t; using ULONGLONG=uint64_t;
using SRWLOCK=int;
#define SRWLOCK_INIT 0
struct LARGE_INTEGER {long long QuadPart;};
struct QueueSnapshot {BYTE active,field1;WORD storedType,storedPayload;BYTE field6,field7;WORD field8,field10,field12;BYTE field14,guard;};
std::map<DWORD,DWORD> memory;
bool SafeReadDword(DWORD a,DWORD* v){auto it=memory.find(a);if(it==memory.end())return false;*v=it->second;return true;}
constexpr DWORD kPlayerQueueOffset=0x3A2C;
bool IsKnownPlayerQueue(DWORD q,DWORD* s,DWORD* p){if(q!=0x100000+kPlayerQueueOffset)return false;*s=1;*p=0x100000;return true;}
DWORD GetCurrentThreadId(){return 42;} DWORD GetTickCount(){return 123;}
void QueryPerformanceCounter(LARGE_INTEGER* q){static long long n=0;q->QuadPart=++n;}
LONG InterlockedIncrement(volatile LONG* p){return ++*p;}
LONG InterlockedCompareExchange(volatile LONG* p,LONG v,LONG c){LONG old=*p;if(old==c)*p=v;return old;}
void AcquireSRWLockExclusive(SRWLOCK*){} void ReleaseSRWLockExclusive(SRWLOCK*){}
bool SafeReadQueueSnapshot(DWORD,QueueSnapshot* s){s->storedType=0;s->storedPayload=4;return true;}
using QueueInsertFn=void(*)(void*,DWORD,DWORD);
int calls=0;void original(void*,DWORD t,DWORD p){assert(t==0 && p==4);++calls;}
QueueInsertFn g_originalQueueInsert=original;
'''
test=r'''
void QueueProbe(DWORD callerRva){
 DWORD eventType=0,payload=4,queueAddress=0x103a2c,side=0,playerObject=0;
 void* queue=reinterpret_cast<void*>(static_cast<uintptr_t>(queueAddress));
 QUEUE_CODE
}
int main(){
 DWORD context[11]={};
 memory[static_cast<DWORD>(reinterpret_cast<uintptr_t>(context+10))]=0x8000;
 memory[0x8000]=0x100000; memory[0x100004]=0x200000;
 memory[0x103b84]=0x20000;
 Stage31Begin(context); assert(g_stage31Depth==1 && g_stage31Count==1);
 assert(g_stage31Records[0].Values[4]==0x20000 && (g_stage31Records[0].Mask&(1u<<4)));
 assert(!(g_stage31Records[0].Mask&(1u<<5))); // unreadable != valid zero
 auto id=g_stage31Scopes[0].Episode;
 g_stage31Installed=1; QueueProbe(0x156e4c);
 assert(calls==1 && g_stage31Records[1].Episode==id && g_stage31Records[1].Kind==3);
 assert(g_stage31Records[1].QpcAfter>g_stage31Records[1].Qpc);
 Stage31Begin(context);assert(g_stage31Records[2].Parent==id);
 Stage31End(); Stage31End();assert(g_stage31Depth==0);
 QueueProbe(0x156e4c);assert(g_stage31Records[5].Episode==0 && calls==2);
 DWORD n=g_stage31Count;QueueProbe(0x12345);assert(g_stage31Count==n && calls==3);
 for(int i=0;i<9;++i)Stage31Begin(context);
 assert(g_stage31Depth==9 && g_stage31Overflow==1);
 QueueProbe(0x156e4c);assert(g_stage31Records[g_stage31Write-1].Episode==0);
 for(int i=0;i<9;++i){Stage31End();}
 assert(g_stage31Depth==0);
 Stage31Begin(context);
 std::thread t([]{assert(g_stage31Depth==0);});t.join();
 assert(g_stage31Depth==1);Stage31End();
 Stage31Record r{};while(g_stage31Count<kStage31Capacity)AddStage31Record(r);
 auto seq=g_stage31Sequence;AddStage31Record(r);
 assert(g_stage31Dropped==1 && g_stage31Count==kStage31Capacity && g_stage31Sequence==seq+1);
 std::cout << "PASS: read masks, parent matching, unscoped queue, filter, exactly-once call, nested overflow, TLS isolation, ring drops\n";
}
'''.replace('QUEUE_CODE',queue)
with tempfile.TemporaryDirectory() as d:
    p=pathlib.Path(d);(p/'test.cpp').write_text(header+logic+test)
    subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-pthread',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
