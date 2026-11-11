// ArcanaAI Stage93 r1: observe native F5990 direct OR at F5A68.
// EXACT original instruction 83 8E 70 3B 00 00 08 executes ONCE, in-place semantics.
// Passive snapshots only, no synthetic commands, no DR or other game writes.
// Must be included AFTER Stage92F5990BranchTrace.inl at global scope.
#ifndef _M_IX86
#error Stage93 requires Windows MSVC x86 inline assembly
#endif

extern "C" {
DWORD g_Stage93Resume=0;
void __cdecl Stage93BeforeOr(const DWORD* saved);
void __cdecl Stage93AfterOr(const DWORD* saved);
}
namespace arcana_stage93 {
constexpr DWORD kSite=0x0F5A68u, kResume=kSite+7u, kCapacity=8192u, kDepth=16u;
const BYTE kOriginal[7]={0x83,0x8E,0x70,0x3B,0x00,0x00,0x08};
const BYTE kPrefix[4]={0x84,0xC0,0x74,0x0B}; // test al,al; je F5A73
const BYTE kSuffix[4]={0x5F,0x5E,0x5D,0xC3}; // return epilogue
struct Event {
 DWORD seq,player,side,tid,stage92Tick,stage92HelperSet,stage92HelperClear;
 DWORD preFlag,postFlag,preMask,postMask,preEflags,nativeOrEflags,valid;
 ULONGLONG qpcBefore,qpcAfter;
};
struct Pending {Event ev;bool active;};
__declspec(thread) Pending g_pending[kDepth]{};
__declspec(thread) DWORD g_depth=0;
SRWLOCK g_lock=SRWLOCK_INIT;
Event g_ring[kCapacity]{};
DWORD g_read=0,g_write=0,g_count=0;
volatile LONG g_installed=0,g_seq=0,g_calls=0,g_filtered=0,g_overflow=0;
volatile LONG g_dropped=0,g_mismatch=0,g_readFail=0;
void Log(const char* text) {
 wchar_t path[MAX_PATH+1]{};
 if(!arcana_stage81::GetPath(path,_countof(path),L"ArcanaAI_Stage93DirectBit3Trace.log"))return;
 HANDLE h=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
                     nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(h==INVALID_HANDLE_VALUE)return;
 DWORD n=0;WriteFile(h,text,static_cast<DWORD>(std::strlen(text)),&n,nullptr);
 WriteFile(h,"\r\n",2u,&n,nullptr);CloseHandle(h);
}
ULONGLONG Qpc(){LARGE_INTEGER q{};QueryPerformanceCounter(&q);return static_cast<ULONGLONG>(q.QuadPart);}
bool ReadFlag(DWORD actor,DWORD& out){
 __try {out=*reinterpret_cast<volatile DWORD*>(static_cast<uintptr_t>(actor+0x3B70u));return true;}
 __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&g_readFail);return false;}
}
void Enqueue(const Event& e){
 AcquireSRWLockExclusive(&g_lock);
 if(g_count==kCapacity)InterlockedIncrement(&g_dropped);
 else {g_ring[g_write]=e;g_write=(g_write+1u)%kCapacity;++g_count;}
 ReleaseSRWLockExclusive(&g_lock);
}
void Before(const DWORD* saved){
 InterlockedIncrement(&g_calls);
 if(!saved)return;
 const DWORD depth=g_depth++;
 if(depth>=kDepth){InterlockedIncrement(&g_overflow);return;}
 Pending& p=g_pending[depth];p=Pending{};
 Event& e=p.ev;
 // PUSHFD/PUSHAD: +4 = original ESI; +32 = original EFLAGS.
 e.player=saved[1];e.preEflags=saved[8];e.side=arcana_stage81::Side(e.player);
 e.tid=GetCurrentThreadId();
 if(!e.side){InterlockedIncrement(&g_filtered);return;}
 if(arcana_stage92::g_depth&&arcana_stage92::g_depth<=arcana_stage92::kDepth){
  const arcana_stage92::Pending& parent=arcana_stage92::g_pending[arcana_stage92::g_depth-1u];
  if(parent.valid&&parent.e.player==e.player&&parent.e.tid==e.tid){
   e.stage92Tick=parent.e.tickId;
   e.stage92HelperSet=parent.e.helperSets;e.stage92HelperClear=parent.e.helperClears;
  }
 }
 e.seq=static_cast<DWORD>(InterlockedIncrement(&g_seq));
 if(ReadFlag(e.player,e.preFlag))e.preMask=1u;
 e.qpcBefore=Qpc();p.active=true;
}
void After(const DWORD* saved){
 if(!saved)return;
 if(!g_depth){InterlockedIncrement(&g_mismatch);return;}
 const DWORD depth=--g_depth;
 if(depth>=kDepth)return;
 Pending& p=g_pending[depth];if(!p.active)return;
 Event e=p.ev;p.active=false;
 if(e.player!=saved[1]||e.tid!=GetCurrentThreadId()){
  InterlockedIncrement(&g_mismatch);return;
 }
 e.nativeOrEflags=saved[8];e.qpcAfter=Qpc();
 if(ReadFlag(e.player,e.postFlag))e.postMask=1u;
 e.valid=(e.preMask==1u&&e.postMask==1u&&((e.preFlag|8u)==e.postFlag))?1u:0u;
 // 'valid' compares complete dword, not only bit3. Interleaving writes
 // could yield a mismatch; report it, never change original execution.
 if(!e.valid)InterlockedIncrement(&g_mismatch);
 Enqueue(e);
}
struct Suspended { HANDLE h[512]{};DWORD count=0; };
void Resume(Suspended& s){
 while(s.count){HANDLE h=s.h[--s.count];ResumeThread(h);CloseHandle(h);}
}
bool SuspendAll(Suspended& s){
 const DWORD pid=GetCurrentProcessId(),self=GetCurrentThreadId();
 HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
 if(snap==INVALID_HANDLE_VALUE)return false;
 THREADENTRY32 t{};t.dwSize=sizeof(t);
 BOOL more=Thread32First(snap,&t);bool ok=more!=FALSE;
 while(more&&ok){
  if(t.th32OwnerProcessID==pid&&t.th32ThreadID!=self){
   if(s.count==_countof(s.h)){ok=false;break;}
   HANDLE h=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT,FALSE,t.th32ThreadID);
   if(!h){ok=false;break;}
   if(SuspendThread(h)==static_cast<DWORD>(-1)){CloseHandle(h);ok=false;break;}
   s.h[s.count++]=h;
   CONTEXT c{};c.ContextFlags=CONTEXT_CONTROL;
   if(!GetThreadContext(h,&c)){ok=false;break;}
   if(c.Eip>=arcana_stage81::g_gameBase+kSite&&
      c.Eip<arcana_stage81::g_gameBase+kResume){ok=false;break;}
  }
  t.dwSize=sizeof(t);more=Thread32Next(snap,&t);
 }
 CloseHandle(snap);if(!ok)Resume(s);return ok;
}
bool SignatureOK(){
 const BYTE* base=reinterpret_cast<const BYTE*>(static_cast<uintptr_t>(arcana_stage81::g_gameBase));
 return std::memcmp(base+kSite,kOriginal,sizeof(kOriginal))==0&&
        std::memcmp(base+kSite-sizeof(kPrefix),kPrefix,sizeof(kPrefix))==0&&
        std::memcmp(base+kResume,kSuffix,sizeof(kSuffix))==0;
}
bool Patch(BYTE* ptr,const BYTE* data,DWORD size){
 DWORD old=0,unused=0;
 if(!VirtualProtect(ptr,size,PAGE_EXECUTE_READWRITE,&old))return false;
 std::memcpy(ptr,data,size);
 const bool flush=FlushInstructionCache(GetCurrentProcess(),ptr,size)!=FALSE;
 const bool restore=VirtualProtect(ptr,size,old,&unused)!=FALSE;
 return flush&&restore;
}
} // arcana_stage93
extern "C" {
void __cdecl Stage93BeforeOr(const DWORD* saved){arcana_stage93::Before(saved);}
void __cdecl Stage93AfterOr(const DWORD* saved){arcana_stage93::After(saved);}
}
// The original OR is an instruction, not a CALL: jump back to F5A6F.
// Keep OR's EFLAGS *after* the AFTER observer. All state is restored.
extern "C" __declspec(naked) void Stage93DirectOrHook(){
 __asm {
  pushfd
  pushad
  mov ebx,esp
  sub esp,220h
  and esp,0FFFFFFF0h
  fxsave [esp]
  fninit
  cld
  push ebx
  call Stage93BeforeOr
  lea esp,[esp+4]
  fxrstor [esp]
  mov esp,ebx
  popad
  popfd
  or dword ptr [esi+3B70h],8
  pushfd
  pushad
  mov ebx,esp
  sub esp,220h
  and esp,0FFFFFFF0h
  fxsave [esp]
  fninit
  cld
  push ebx
  call Stage93AfterOr
  lea esp,[esp+4]
  fxrstor [esp]
  mov esp,ebx
  popad
  popfd
  jmp dword ptr [g_Stage93Resume]
 }
}
namespace arcana_stage93 {
bool Install(){
 Log("STAGE93_BUILD=2026-11-12-r1 SOURCE_ONLY directF5A68 OR once; originalAfterEflags=1 gameWritesExtra=0 DRwrites=0");
 wchar_t ini[MAX_PATH+1]{};
 if(arcana_stage81::GetPath(ini,_countof(ini),L"ArcanaAI.ini")&&
    GetPrivateProfileIntW(L"AIResearch",L"EnableStage93DirectBit3Trace",1,ini)==0){
  Log("STAGE93_DISABLED ini EnableStage93DirectBit3Trace=0");return false;
 }
 HMODULE game=GetModuleHandleW(L"AALib.exe");
 if(sizeof(void*)!=4u||!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)||
    !arcana_stage81::Fingerprint(game)||
    InterlockedCompareExchange(&arcana_stage92::g_installed,0,0)!=1){
  Log("STAGE93_ERROR fingerprint/Stage92 prerequisite missing; no patch");return false;
 }
 if(InterlockedCompareExchange(&g_installed,0,0)||!SignatureOK()){
  Log("STAGE93_ERROR signature already patched/invalid; no patch");return false;
 }
 BYTE* site=reinterpret_cast<BYTE*>(static_cast<uintptr_t>(arcana_stage81::g_gameBase+kSite));
 const std::intptr_t delta=reinterpret_cast<std::intptr_t>(&Stage93DirectOrHook)-
                             reinterpret_cast<std::intptr_t>(site+5u);
 if(delta<static_cast<std::intptr_t>(INT32_MIN)||delta>static_cast<std::intptr_t>(INT32_MAX)){
  Log("STAGE93_ERROR rel32 out of range; no patch");return false;
 }
 BYTE patch[7]={0xE9,0,0,0,0,0x90,0x90};
 const std::int32_t rel=static_cast<std::int32_t>(delta);
 std::memcpy(patch+1u,&rel,4u);
 HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
 if(!mutex){Log("STAGE93_ERROR patch mutex unavailable");return false;}
 const DWORD wait=WaitForSingleObject(mutex,10000u);
 if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){CloseHandle(mutex);Log("STAGE93_ERROR mutex acquisition failed");return false;}
 Suspended peers{};
 bool ok=SuspendAll(peers),touched=false,rollback=true;
 if(ok){
  ok=SignatureOK();
  if(ok){
   g_Stage93Resume=arcana_stage81::g_gameBase+kResume;
   touched=true;
   ok=Patch(site,patch,sizeof(patch));
  }
  if(!ok&&touched)rollback=Patch(site,kOriginal,sizeof(kOriginal));
  Resume(peers);
 }
 ReleaseMutex(mutex);CloseHandle(mutex);
 if(!ok||!rollback){
  Log("STAGE93_ERROR installation refused/rollback attempted; restore Stage92 r3 DLL if needed");
  return false;
 }
 InterlockedExchange(&g_installed,1);
 Log("STAGE93_INSTALLED 1/1 site=0F5A68 resume=0F5A6F directOR=1 originalOnce=1 Stage92required=1 noEE306=1 noFinalHook=1 gameWritesExtra=0 DRwrites=0");
 return true;
}
void Flush(){
 if(InterlockedCompareExchange(&g_installed,0,0)!=1)return;
 for(DWORD i=0;i<kCapacity;++i){
  Event e{};AcquireSRWLockExclusive(&g_lock);
  const bool has=g_count!=0;
  if(has){e=g_ring[g_read];g_read=(g_read+1u)%kCapacity;--g_count;}
  ReleaseSRWLockExclusive(&g_lock);if(!has)break;
  char msg[620]{};
  sprintf_s(msg,"STAGE93_OR seq=%lu side=P%lu player=%08lX tid=%lu "
   "stage92Tick=%lu s92HelperSetBefore=%lu s92HelperClearBefore=%lu "
   "qpcBefore=%I64u qpcAfter=%I64u preFlag=%08lX postFlag=%08lX "
   "preMask=%lu postMask=%lu preEflags=%08lX nativeOrEflags=%08lX "
   "bit3Rise=%lu valid=%lu",
   e.seq,e.side,e.player,e.tid,e.stage92Tick,e.stage92HelperSet,e.stage92HelperClear,
   e.qpcBefore,e.qpcAfter,e.preFlag,e.postFlag,e.preMask,e.postMask,
   e.preEflags,e.nativeOrEflags,(!(e.preFlag&8u)&&(e.postFlag&8u))?1u:0u,e.valid);
  Log(msg);
 }
}
void Health(){
 if(InterlockedCompareExchange(&g_installed,0,0)!=1)return;
 char msg[400]{};
 sprintf_s(msg,"STAGE93_HEALTH calls=%ld filtered=%ld overflow=%ld dropped=%ld "
  "mismatch=%ld readFail=%ld capacity=8192 depth=16 gameWritesExtra=0 DRwrites=0",
  InterlockedExchange(&g_calls,0),InterlockedExchange(&g_filtered,0),
  InterlockedExchange(&g_overflow,0),InterlockedExchange(&g_dropped,0),
  InterlockedExchange(&g_mismatch,0),InterlockedExchange(&g_readFail,0));
 Log(msg);
}
} // arcana_stage93
