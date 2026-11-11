// Stage87 r1 SOURCE ONLY. Passive observer at original EE2F0 MOV ECX,[ESI+3F34].
// EDI == 0x2C: original EE2D5 reads player+3F50, translates selector
// 0/1/2 to EAX 0E/0F/10 before EE306 -> F57D0.
// Original EE2F0 instruction replayed ONCE, flags/GPR/FPU/SSE preserved.
// NO game-data writes, input synthesis, DR writes, or gameplay changes.
#ifndef _M_IX86
#error Stage87 requires Windows MSVC x86 inline assembly
#endif
namespace arcana_stage87 {
using arcana_stage81::g_gameBase;
constexpr DWORD kRva=0x0EE2F0u,kLength=6u,kResumeRva=0x0EE2F5u;
constexpr DWORD kCapacity=8192u;
const BYTE kOriginal[kLength]={0x8B,0x8E,0x34,0x3F,0x00,0x00};
struct Record {
 DWORD seq=0,side=0,player=0,object=0,tid=0,edi=0,eax=0,selector=0,companion=0,mode=0,mask=0,expected=0,match=0;
 ULONGLONG qpc=0;
};
SRWLOCK g_lock=SRWLOCK_INIT;
Record g_ring[kCapacity]{};
DWORD g_head=0,g_tail=0,g_count=0;
volatile LONG g_seq=0,g_calls=0,g_events=0,g_filtered=0,g_dropped=0,g_mismatch=0,g_readFail=0,g_installed=0;
void Log(const char* msg) {
 wchar_t path[MAX_PATH+1]{};
 if(!arcana_stage81::GetPath(path,_countof(path),L"ArcanaAI_Stage87SelectorConsumerTrace.log"))return;
 HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(f==INVALID_HANDLE_VALUE)return;
 DWORD w=0;WriteFile(f,msg,static_cast<DWORD>(strlen(msg)),&w,nullptr);
 WriteFile(f,"\r\n",2,&w,nullptr);CloseHandle(f);
}
bool Read(DWORD address,DWORD& value) {
 __try{value=*reinterpret_cast<volatile DWORD*>(static_cast<uintptr_t>(address));return true;}
 __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&g_readFail);return false;}
}
void Observe(const DWORD* saved){
 InterlockedIncrement(&g_calls);
 if(!saved){InterlockedIncrement(&g_mismatch);return;}
 // PUSHFD/PUSHAD: 0=EDI 1=ESI 2=EBP 6=ECX 7=EAX 8=EFLAGS 9=call-return VA.
 // Our CALL at EE2F0 returns to EE2F5 (NOP), then native EE2F6 continues.
 if(saved[0]!=0x2Cu){InterlockedIncrement(&g_filtered);return;}
 Record r{};r.edi=saved[0];r.player=saved[1];r.eax=saved[7];
 r.side=arcana_stage81::Side(r.player);r.tid=GetCurrentThreadId();
 if(!r.side||saved[9]!=g_gameBase+kResumeRva){InterlockedIncrement(&g_mismatch);return;}
 if(Read(r.player+0x04u,r.object))r.mask|=1u;
 if(Read(r.player+0x3F50u,r.selector))r.mask|=2u;
 if(Read(r.player+0x3F54u,r.companion))r.mask|=4u;
 if(Read(r.player+0x3F30u,r.mode))r.mask|=8u;
 LARGE_INTEGER q{};QueryPerformanceCounter(&q);r.qpc=static_cast<ULONGLONG>(q.QuadPart);
 if((r.mask&2u)!=0u&&r.selector<=2u){
  r.expected=0x0Eu+r.selector;r.match=r.eax==r.expected?1u:0u;
  if(!r.match)InterlockedIncrement(&g_mismatch);
 }else{
  r.expected=0xFFFFFFFFu;InterlockedIncrement(&g_mismatch);
 }
 r.seq=static_cast<DWORD>(InterlockedIncrement(&g_seq));
 AcquireSRWLockExclusive(&g_lock);
 if(g_count==kCapacity)InterlockedIncrement(&g_dropped);
 else {g_ring[g_tail]=r;g_tail=(g_tail+1u)%kCapacity;++g_count;}
 ReleaseSRWLockExclusive(&g_lock);
 InterlockedIncrement(&g_events);
}
void Flush(){
 for(DWORD i=0;i<kCapacity;++i){
  Record r{};AcquireSRWLockExclusive(&g_lock);
  const bool has=g_count!=0;
  if(has){r=g_ring[g_head];g_head=(g_head+1u)%kCapacity;--g_count;}
  ReleaseSRWLockExclusive(&g_lock);if(!has)break;
  char msg[520]{};
  sprintf_s(msg,"STAGE87_CONSUMER seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu "
   "siteRVA=0EE2F0 nextCallRVA=0EE306 qpc=%I64u edi=%08lX selector=%08lX "
   "companion=%08lX mode=%08lX mappedEax=%08lX expected=%08lX match=%lu readMask=%lX",
   r.seq,r.side,r.player,r.object,r.tid,r.qpc,r.edi,r.selector,r.companion,r.mode,
   r.eax,r.expected,r.match,r.mask);
  Log(msg);
 }
}
void Health(){
 if(InterlockedCompareExchange(&g_installed,0,0)!=1)return;
 char msg[320]{};
 sprintf_s(msg,"STAGE87_HEALTH calls=%ld events=%ld filtered=%ld dropped=%ld mismatch=%ld readFail=%ld "
  "ring=8192 replayMOV=1 DRwrites=0 gameDataWrites=0",
  InterlockedExchange(&g_calls,0),InterlockedExchange(&g_events,0),
  InterlockedExchange(&g_filtered,0),InterlockedExchange(&g_dropped,0),
  InterlockedExchange(&g_mismatch,0),InterlockedExchange(&g_readFail,0));Log(msg);
}
struct Suspended{HANDLE handles[512];DWORD count=0;};
void Resume(Suspended& s){while(s.count){HANDLE h=s.handles[--s.count];ResumeThread(h);CloseHandle(h);}}
bool SuspendAndCheck(Suspended& s){
 const DWORD pid=GetCurrentProcessId(),self=GetCurrentThreadId();
 HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
 if(snap==INVALID_HANDLE_VALUE)return false;
 THREADENTRY32 t{};t.dwSize=sizeof(t);BOOL has=Thread32First(snap,&t);bool ok=has!=FALSE;
 while(has&&ok){
  if(t.th32OwnerProcessID==pid&&t.th32ThreadID!=self){
   if(s.count>=_countof(s.handles)){ok=false;break;}
   HANDLE h=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT,FALSE,t.th32ThreadID);
   if(!h){ok=false;break;}
   if(SuspendThread(h)==static_cast<DWORD>(-1)){CloseHandle(h);ok=false;break;}
   s.handles[s.count++]=h;CONTEXT c{};c.ContextFlags=CONTEXT_CONTROL;
   if(!GetThreadContext(h,&c)||(c.Eip>=g_gameBase+kRva&&c.Eip<g_gameBase+kRva+kLength)){ok=false;break;}
  }
  t.dwSize=sizeof(t);has=Thread32Next(snap,&t);
 }
 CloseHandle(snap);if(!ok)Resume(s);return ok;
}
} // namespace arcana_stage87
extern "C" {
void __cdecl Stage87ObserveEE2F0(const DWORD* saved){arcana_stage87::Observe(saved);}
__declspec(naked) void Stage87EE2F0Hook(){
 __asm {
  pushfd
  pushad
  mov ebx,esp
  sub esp,528
  and esp,0FFFFFFF0h
  fxsave [esp]
  fninit
  cld
  push ebx
  call Stage87ObserveEE2F0
  add esp,4
  fxrstor [esp]
  mov esp,ebx
  popad
  popfd
  // Replay EXACT original EE2F0 instruction, then return to EE2F5 NOP / EE2F6.
  mov ecx,dword ptr [esi+3F34h]
  ret
 }
}
}
namespace arcana_stage87 {
bool Install(){
 Log("STAGE87_BUILD=2026-11-11-r1 SOURCE_ONLY EE2F0 original MOV replay; passive mapped-selector observer");
 wchar_t ini[MAX_PATH+1]{};
 if(arcana_stage81::GetPath(ini,_countof(ini),L"ArcanaAI.ini")&&
    GetPrivateProfileIntW(L"AIResearch",L"EnableStage87",1,ini)==0){
  Log("STAGE87_DISABLED ini EnableStage87=0");return false;
 }
 HMODULE m=GetModuleHandleW(L"AALib.exe");
 if(sizeof(void*)!=4u||!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)||
    !arcana_stage81::Fingerprint(m)||
    InterlockedCompareExchange(&arcana_stage86::g_installed,0,0)!=1){
  Log("STAGE87_ERROR prerequisites x86/PE or Stage86 missing; no patch");return false;
 }
 const BYTE* base=reinterpret_cast<const BYTE*>(static_cast<uintptr_t>(g_gameBase));
 const BYTE reader[6]={0x8B,0x8E,0x50,0x3F,0,0}; // original EE2D5 reader
 const BYTE successor[6]={0x8B,0x96,0x30,0x3F,0,0}; // EE2F6 onward
 const BYTE cmpCode[3]={0x83,0xFF,0x2C};          // EE2D0 CMP EDI,2C
 const bool exact=memcmp(base+kRva,kOriginal,kLength)==0 &&
      memcmp(base+0x0EE2D5u,reader,sizeof(reader))==0 &&
      memcmp(base+kRva+kLength,successor,sizeof(successor))==0 &&
      memcmp(base+0x0EE2D0u,cmpCode,sizeof(cmpCode))==0;
 Log(exact?"STAGE87_SITE_AUDIT rva=0EE2F0 length=6 live=8B8E343F0000 reader=EE2D5 mappedBranch=2C exact=1":
           "STAGE87_SITE_AUDIT exact=0 EE2F0/EE2D5/EE2F6 mismatch, no patch");
 if(!exact)return false;
 BYTE* site=reinterpret_cast<BYTE*>(static_cast<uintptr_t>(g_gameBase+kRva));
 BYTE patch[kLength]={0xE8,0,0,0,0,0x90};
 DWORD rel=static_cast<DWORD>(reinterpret_cast<uintptr_t>(&Stage87EE2F0Hook))-(g_gameBase+kRva+5u);
 memcpy(patch+1,&rel,4u);
 HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
 if(!mutex){Log("STAGE87_ERROR mutex; no patch");return false;}
 const DWORD wait=WaitForSingleObject(mutex,10000);
 if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){CloseHandle(mutex);Log("STAGE87_ERROR mutex wait; no patch");return false;}
 Suspended peers{};bool good=SuspendAndCheck(peers),writable=false,restoreOK=true,rollbackOK=true;
 DWORD old=0,unused=0;
 if(good){
  if(memcmp(site,kOriginal,kLength)!=0)good=false;
  if(good){writable=VirtualProtect(site,kLength,PAGE_EXECUTE_READWRITE,&old)!=FALSE;good=writable;}
  if(good){memcpy(site,patch,kLength);good=FlushInstructionCache(GetCurrentProcess(),site,kLength)!=FALSE;}
  if(!good&&writable){memcpy(site,kOriginal,kLength);rollbackOK=FlushInstructionCache(GetCurrentProcess(),site,kLength)!=FALSE;}
  if(writable){
   restoreOK=VirtualProtect(site,kLength,old,&unused)!=FALSE;
   if(!restoreOK){memcpy(site,kOriginal,kLength);rollbackOK=FlushInstructionCache(GetCurrentProcess(),site,kLength)!=FALSE;
    restoreOK=VirtualProtect(site,kLength,old,&unused)!=FALSE;good=false;}
  }
  Resume(peers);
 }
 ReleaseMutex(mutex);CloseHandle(mutex);
 if(!good||!restoreOK||!rollbackOK){Log("STAGE87_ERROR refused/rolled back; restart before retry");return false;}
 InterlockedExchange(&g_installed,1);
 Log("STAGE87_INSTALLED 1/1 rva=0EE2F0 length=6 resume=0EE2F5+NOP replayMOV=1 "
     "FXSAVE=1 DRwrites=0 gameDataWrites=0");return true;
}
} // namespace arcana_stage87
