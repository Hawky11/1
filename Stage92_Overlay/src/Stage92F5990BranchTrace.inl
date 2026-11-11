// Stage92 r1 — passive native F5990 boundary and exact selected helper CALL sites.
// ACAB8 calls original F5990 ONCE; F5A86/F5A8F tail-chain to original
// 9ED20 (set controller bit3) / 9ED60 (clear controller bit3) ONCE.
// No global F58E0 detour, second EE306 hook, game-data or DR writes.
#ifndef _M_IX86
#error Stage92 requires MSVC Windows x86 inline assembly
#endif
extern "C" {
DWORD g_Stage92NativeF5990=0, g_Stage92NativeSet=0, g_Stage92NativeClear=0;
void __cdecl Stage92TickBefore(const DWORD* saved);
void __cdecl Stage92TickAfter(const DWORD* saved);
void __cdecl Stage92HelperSeen(const DWORD* saved, DWORD kind);
}
namespace arcana_stage92 {
using arcana_stage81::g_gameBase;
constexpr DWORD kCount=3u,kCapacity=8192u,kDepth=16u;
constexpr DWORD kSites[3]={0xACAB8u,0xF5A86u,0xF5A8Fu};
constexpr DWORD kTargets[3]={0xF5990u,0x9ED20u,0x9ED60u};
constexpr BYTE kCalls[3][5]={
 {0xE8,0xD3,0x8E,0x04,0x00},
 {0xE8,0x95,0x92,0xFA,0xFF},
 {0xE8,0xCC,0x92,0xFA,0xFF}
};
constexpr BYTE kBefore0[]={0x8B,0xCE,0xE8,0x09,0xA4,0x03,0x00,0x56};
constexpr BYTE kAfter0[]={0x56,0xE8,0xFD,0x95,0x04,0x00,0x56};
constexpr BYTE kBefore1[]={0x83,0xC4,0x08,0x6A,0x03,0x8B,0xCE,0x84,0xC0,0x74,0x09};
constexpr BYTE kAfterHelper[]={0x5F,0x5E,0x5D,0xC3};
constexpr BYTE kBefore2[]={0x5F,0x5E,0x5D,0xC3};
constexpr BYTE kTargetStart[]={0x55,0x8B,0xEC,0x56,0x8B,0x75,0x08};
struct Event {
 DWORD seq,kind,tickId,player,side,tid,returnRva,originalArg,ecx,esi;
 DWORD flagsBefore,flagsAfter,maskBefore,maskAfter,bitmapBefore,bitmapAfter;
 DWORD helperSets,helperClears,stage91SeqBefore,stage91SeqAfter;
 DWORD nativeEax,nativeFlags,observedIndex,valid;
 ULONGLONG qpcBefore,qpcAfter;
};
struct Pending {Event e;bool valid;};
// POD TLS: avoid dynamic C++ thread-local initialization in an injected DLL.
__declspec(thread) Pending g_pending[kDepth]{};
__declspec(thread) DWORD g_depth=0;
SRWLOCK g_lock=SRWLOCK_INIT;
Event g_ring[kCapacity]{};
DWORD g_read=0,g_write=0,g_count=0;
volatile LONG g_installed=0,g_seq=0,g_ticks=0,g_helpersSet=0,g_helpersClear=0;
volatile LONG g_filtered=0,g_overflow=0,g_dropped=0,g_mismatch=0,g_readFail=0;
void Log(const char* value){
 wchar_t path[MAX_PATH+1]{};
 if(!arcana_stage81::GetPath(path,_countof(path),L"ArcanaAI_Stage92F5990BranchTrace.log"))return;
 HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(f==INVALID_HANDLE_VALUE)return;
 DWORD written=0;
 WriteFile(f,value,static_cast<DWORD>(std::strlen(value)),&written,nullptr);
 WriteFile(f,"\r\n",2u,&written,nullptr);CloseHandle(f);
}
ULONGLONG Qpc(){LARGE_INTEGER t{};QueryPerformanceCounter(&t);return static_cast<ULONGLONG>(t.QuadPart);}
bool ReadDword(DWORD address,DWORD& value){
 __try{value=*reinterpret_cast<volatile DWORD*>(static_cast<uintptr_t>(address));return true;}
 __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&g_readFail);return false;}
}
void PushEvent(const Event& e){
 AcquireSRWLockExclusive(&g_lock);
 if(g_count>=kCapacity)InterlockedIncrement(&g_dropped);
 else{g_ring[g_write]=e;g_write=(g_write+1u)%kCapacity;++g_count;}
 ReleaseSRWLockExclusive(&g_lock);
}
void Before(const DWORD* saved){
 InterlockedIncrement(&g_ticks);
 if(!saved)return;
 const DWORD depth=g_depth++;
 if(depth>=kDepth){InterlockedIncrement(&g_overflow);return;}
 Pending& p=g_pending[depth];p=Pending{};
 Event& e=p.e;
 e.kind=0;e.tid=GetCurrentThreadId();e.returnRva=kSites[0]+5u;
 // PUSHFD/PUSHAD: 9=original return, 10=actor argument.
 if(saved[9]!=g_gameBase+e.returnRva){InterlockedIncrement(&g_mismatch);return;}
 e.originalArg=saved[10];e.player=e.originalArg;e.side=arcana_stage81::Side(e.player);
 if(!e.side){InterlockedIncrement(&g_filtered);return;}
 e.esi=saved[1];e.ecx=saved[6];e.seq=static_cast<DWORD>(InterlockedIncrement(&g_seq));
 e.tickId=e.seq;e.stage91SeqBefore=static_cast<DWORD>(InterlockedCompareExchange(&arcana_stage91::g_seq,0,0));
 if(ReadDword(e.player+0x3B70u,e.flagsBefore))e.maskBefore|=1u;
 if(ReadDword(e.player+0x3A0Cu,e.bitmapBefore))e.maskBefore|=2u;
 e.qpcBefore=Qpc();p.valid=true;
}
void After(const DWORD* saved){
 if(!saved)return;
 if(!g_depth){InterlockedIncrement(&g_mismatch);return;}
 const DWORD depth=--g_depth;if(depth>=kDepth)return;
 Pending& p=g_pending[depth];if(!p.valid)return;
 Event e=p.e;p.valid=false;
 if(saved[9]!=g_gameBase+kSites[0]+5u||saved[10]!=e.player||e.tid!=GetCurrentThreadId()){
    InterlockedIncrement(&g_mismatch);return;
 }
 e.qpcAfter=Qpc();e.nativeEax=saved[7];e.nativeFlags=saved[8];
 e.stage91SeqAfter=static_cast<DWORD>(InterlockedCompareExchange(&arcana_stage91::g_seq,0,0));
 if(ReadDword(e.player+0x3B70u,e.flagsAfter))e.maskAfter|=1u;
 if(ReadDword(e.player+0x3A0Cu,e.bitmapAfter))e.maskAfter|=2u;
 e.valid=((e.maskBefore&3u)==3u&&(e.maskAfter&3u)==3u)?1u:0u;
 PushEvent(e);
}
void Helper(const DWORD* saved,DWORD kind){
 if(!saved||kind<1u||kind>2u)return;
 if(kind==1u)InterlockedIncrement(&g_helpersSet);else InterlockedIncrement(&g_helpersClear);
 const DWORD idx=kind,expectedRet=g_gameBase+kSites[idx]+5u;
 if(saved[9]!=expectedRet||saved[10]!=3u){InterlockedIncrement(&g_mismatch);return;}
 Event e{};e.kind=kind;e.tid=GetCurrentThreadId();e.returnRva=kSites[idx]+5u;
 e.ecx=saved[6];e.esi=saved[1];e.player=e.ecx;e.originalArg=saved[10];
 e.side=arcana_stage81::Side(e.player);
 if(!e.side){InterlockedIncrement(&g_filtered);return;}
 e.seq=static_cast<DWORD>(InterlockedIncrement(&g_seq));e.observedIndex=3u;
 e.qpcBefore=Qpc();
 if(ReadDword(e.player+0x3B70u,e.flagsBefore))e.maskBefore=1u;
 if(ReadDword(e.player+0x3A0Cu,e.bitmapBefore))e.maskBefore|=2u;
 if(g_depth&&g_depth<=kDepth){
   Pending& p=g_pending[g_depth-1u];
   if(p.valid&&p.e.player==e.player&&p.e.tid==e.tid){
      e.tickId=p.e.tickId;
      if(kind==1u)++p.e.helperSets;else ++p.e.helperClears;
   }
 }
 PushEvent(e);
}
struct Suspended {HANDLE h[512]{};DWORD count=0;};
void Resume(Suspended& s){while(s.count){HANDLE h=s.h[--s.count];ResumeThread(h);CloseHandle(h);}}
bool SuspendAll(Suspended& s){
 const DWORD pid=GetCurrentProcessId(),self=GetCurrentThreadId();
 HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
 if(snap==INVALID_HANDLE_VALUE)return false;
 THREADENTRY32 t{};t.dwSize=sizeof(t);BOOL more=Thread32First(snap,&t);bool ok=more!=FALSE;
 while(more&&ok){
  if(t.th32OwnerProcessID==pid&&t.th32ThreadID!=self){
    if(s.count>=_countof(s.h)){ok=false;break;}
    HANDLE h=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT,FALSE,t.th32ThreadID);
    if(!h){ok=false;break;}
    if(SuspendThread(h)==static_cast<DWORD>(-1)){CloseHandle(h);ok=false;break;}
    s.h[s.count++]=h;
    CONTEXT c{};c.ContextFlags=CONTEXT_CONTROL;
    if(!GetThreadContext(h,&c)){ok=false;break;}
    for(DWORD i=0;i<kCount;++i){
       const DWORD rva=kSites[i];
       if(c.Eip>=g_gameBase+rva&&c.Eip<g_gameBase+rva+5u){ok=false;break;}
    }
  }
  t.dwSize=sizeof(t);more=Thread32Next(snap,&t);
 }
 CloseHandle(snap);if(!ok)Resume(s);return ok;
}
bool SignatureOK(){
 const BYTE* base=reinterpret_cast<const BYTE*>(static_cast<uintptr_t>(g_gameBase));
 for(DWORD i=0;i<kCount;++i){
    const BYTE* s=base+kSites[i];
    if(std::memcmp(s,kCalls[i],5u)!=0)return false;
    std::int32_t disp=0;std::memcpy(&disp,s+1,4u);
    if(static_cast<DWORD>(kSites[i]+5u+disp)!=kTargets[i])return false;
 }
 if(std::memcmp(base+kSites[0]-sizeof(kBefore0),kBefore0,sizeof(kBefore0))!=0||
    std::memcmp(base+kSites[0]+5u,kAfter0,sizeof(kAfter0))!=0||
    std::memcmp(base+kSites[1]-sizeof(kBefore1),kBefore1,sizeof(kBefore1))!=0||
    std::memcmp(base+kSites[1]+5u,kAfterHelper,sizeof(kAfterHelper))!=0||
    std::memcmp(base+kSites[2]-sizeof(kBefore2),kBefore2,sizeof(kBefore2))!=0||
    std::memcmp(base+kSites[2]+5u,kAfterHelper,sizeof(kAfterHelper))!=0)return false;
 for(DWORD i=1;i<3u;++i)
    if(std::memcmp(base+kTargets[i],kTargetStart,sizeof(kTargetStart))!=0)return false;
 // Exact once-only native setter/clearer opcodes and stdcall callee cleanup.
 const BYTE setter[]={0x09,0x10},clearer[]={0x21,0x10},ret4[]={0xC2,0x04,0x00};
 if(std::memcmp(base+0x9ED50u,setter,2u)!=0||std::memcmp(base+0x9ED92u,clearer,2u)!=0||
    std::memcmp(base+0x9ED53u,ret4,3u)!=0||std::memcmp(base+0x9ED95u,ret4,3u)!=0)return false;
 return true;
}
bool Patch(BYTE* at,const BYTE* bytes){
 DWORD old=0,unused=0;
 if(!VirtualProtect(at,5u,PAGE_EXECUTE_READWRITE,&old))return false;
 std::memcpy(at,bytes,5u);
 const bool flushed=FlushInstructionCache(GetCurrentProcess(),at,5u)!=FALSE;
 const bool restored=VirtualProtect(at,5u,old,&unused)!=FALSE;
 return flushed&&restored;
}
} // arcana_stage92
extern "C" {
void __cdecl Stage92TickBefore(const DWORD* saved){arcana_stage92::Before(saved);}
void __cdecl Stage92TickAfter(const DWORD* saved){arcana_stage92::After(saved);}
void __cdecl Stage92HelperSeen(const DWORD* saved,DWORD kind){arcana_stage92::Helper(saved,kind);}
}
// All three wrappers restore original registers, flags and FPU/SSE state.
// This wrapper replays the original cdecl argument while leaving its caller's
// existing argument on stack for ACAD3's original ADD ESP,14h.
extern "C" __declspec(naked) void Stage92TickHook(){
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
   call Stage92TickBefore
   lea esp,[esp+4]
   fxrstor [esp]
   mov esp,ebx
   popad
   popfd
   push dword ptr [esp+4]
   call dword ptr [g_Stage92NativeF5990]
   lea esp,[esp+4]
   pushfd
   pushad
   mov ebx,esp
   sub esp,220h
   and esp,0FFFFFFF0h
   fxsave [esp]
   fninit
   cld
   push ebx
   call Stage92TickAfter
   lea esp,[esp+4]
   fxrstor [esp]
   mov esp,ebx
   popad
   popfd
   ret
 }
}
// Exact helper CALL-site wrappers: read-only observer, then TAIL JMP.
// Because native 9ED20/9ED60 perform RET 4, a tail JMP is mandatory:
// the original helper itself consumes the already-pushed index=3, exactly once.
extern "C" __declspec(naked) void Stage92SetHook(){
 __asm {
   pushfd
   pushad
   mov ebx,esp
   sub esp,220h
   and esp,0FFFFFFF0h
   fxsave [esp]
   fninit
   cld
   push 1
   push ebx
   call Stage92HelperSeen
   lea esp,[esp+8]
   fxrstor [esp]
   mov esp,ebx
   popad
   popfd
   jmp dword ptr [g_Stage92NativeSet]
 }
}
extern "C" __declspec(naked) void Stage92ClearHook(){
 __asm {
   pushfd
   pushad
   mov ebx,esp
   sub esp,220h
   and esp,0FFFFFFF0h
   fxsave [esp]
   fninit
   cld
   push 2
   push ebx
   call Stage92HelperSeen
   lea esp,[esp+8]
   fxrstor [esp]
   mov esp,ebx
   popad
   popfd
   jmp dword ptr [g_Stage92NativeClear]
 }
}
namespace arcana_stage92 {
bool Install(){
 Log("STAGE92_BUILD=2026-11-11-r1 SOURCE_ONLY F5990 boundary+selected 9ED20/9ED60 CALL sites; originalOnce=1 gameWrites=0 DRwrites=0");
 wchar_t ini[MAX_PATH+1]{};
 if(arcana_stage81::GetPath(ini,_countof(ini),L"ArcanaAI.ini")&&
  GetPrivateProfileIntW(L"AIResearch",L"EnableStage92BranchTrace",1,ini)==0){
    Log("STAGE92_DISABLED ini EnableStage92BranchTrace=0");return false;
 }
 HMODULE game=GetModuleHandleW(L"AALib.exe");
 if(sizeof(void*)!=4u||!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)||
    !arcana_stage81::Fingerprint(game)||
    InterlockedCompareExchange(&arcana_stage91::g_installed,0,0)!=1){
    Log("STAGE92_ERROR PE/Stage91 prerequisites missing; no patch");return false;
 }
 if(InterlockedCompareExchange(&g_installed,0,0)!=0||!SignatureOK()){
    Log("STAGE92_ERROR native signature mismatch/already installed; no patch");return false;
 }
 BYTE* base=reinterpret_cast<BYTE*>(static_cast<uintptr_t>(g_gameBase));
 const void* hooks[kCount]={reinterpret_cast<const void*>(&Stage92TickHook),
                             reinterpret_cast<const void*>(&Stage92SetHook),
                             reinterpret_cast<const void*>(&Stage92ClearHook)};
 BYTE patch[kCount][5]{};
 for(DWORD i=0;i<kCount;++i){
   const std::intptr_t delta=reinterpret_cast<std::intptr_t>(hooks[i])-
                              reinterpret_cast<std::intptr_t>(base+kSites[i]+5u);
   if(delta<static_cast<std::intptr_t>(INT32_MIN)||delta>static_cast<std::intptr_t>(INT32_MAX)){
      Log("STAGE92_ERROR out-of-range rel32; no patch");return false;
   }
   patch[i][0]=0xE8u;const std::int32_t rel=static_cast<std::int32_t>(delta);
   std::memcpy(patch[i]+1,&rel,4u);
 }
 HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
 if(!mutex){Log("STAGE92_ERROR patch mutex unavailable");return false;}
 DWORD wait=WaitForSingleObject(mutex,10000u);
 if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){CloseHandle(mutex);Log("STAGE92_ERROR mutex wait failed");return false;}
 Suspended peers{};bool ok=SuspendAll(peers),rollbackOK=true;DWORD touched=0;
 if(ok){
   ok=SignatureOK();
   if(ok){
      g_Stage92NativeF5990=g_gameBase+kTargets[0];
      g_Stage92NativeSet=g_gameBase+kTargets[1];
      g_Stage92NativeClear=g_gameBase+kTargets[2];
      for(DWORD i=0;i<kCount;++i){
         touched|=1u<<i;
         if(!Patch(base+kSites[i],patch[i])){ok=false;break;}
      }
   }
   if(!ok)for(DWORD i=0;i<kCount;++i)if(touched&(1u<<i))
       if(!Patch(base+kSites[i],kCalls[i]))rollbackOK=false;
   Resume(peers);
 }
 ReleaseMutex(mutex);CloseHandle(mutex);
 if(!ok||!rollbackOK){
    Log("STAGE92_ERROR install refused/rollback attempted; restore Stage91 tested DLL if necessary");return false;
 }
 InterlockedExchange(&g_installed,1);
 Log("STAGE92_INSTALLED 3/3 ACAB8->F5990 F5A86->9ED20 F5A8F->9ED60 originalOnce=1 noEE306=1 gameWrites=0 DRwrites=0");
 return true;
}
void Flush(){
 if(InterlockedCompareExchange(&g_installed,0,0)!=1)return;
 for(DWORD i=0;i<kCapacity;++i){
  Event e{};AcquireSRWLockExclusive(&g_lock);
  const bool has=g_count!=0;
  if(has){e=g_ring[g_read];g_read=(g_read+1u)%kCapacity;--g_count;}
  ReleaseSRWLockExclusive(&g_lock);if(!has)break;
  char msg[800]{};
  if(e.kind==0u){
    sprintf_s(msg,"STAGE92_TICK seq=%lu tick=%lu side=P%lu player=%08lX tid=%lu "
     "qpcBefore=%I64u qpcAfter=%I64u returnRVA=%06lX actorArg=%08lX "
     "flagBefore=%08lX flagAfter=%08lX bitmapBefore=%08lX bitmapAfter=%08lX "
     "maskBefore=%lX maskAfter=%lX helperSet=%lu helperClear=%lu "
     "stage91SeqBefore=%lu stage91SeqAfter=%lu nativeEax=%08lX nativeFlags=%08lX valid=%lu",
     e.seq,e.tickId,e.side,e.player,e.tid,e.qpcBefore,e.qpcAfter,e.returnRva,e.originalArg,
     e.flagsBefore,e.flagsAfter,e.bitmapBefore,e.bitmapAfter,e.maskBefore,e.maskAfter,
     e.helperSets,e.helperClears,e.stage91SeqBefore,e.stage91SeqAfter,e.nativeEax,e.nativeFlags,e.valid);
  }else{
    sprintf_s(msg,"STAGE92_HELPER seq=%lu kind=%s tick=%lu side=P%lu player=%08lX tid=%lu "
     "qpc=%I64u callRVA=%06lX ecx=%08lX esi=%08lX index=%lu flagPre=%08lX bitmapPre=%08lX preMask=%lX",
     e.seq,e.kind==1u?"SET_9ED20":"CLEAR_9ED60",e.tickId,e.side,e.player,e.tid,
     e.qpcBefore,e.returnRva-5u,e.ecx,e.esi,e.observedIndex,e.flagsBefore,e.bitmapBefore,e.maskBefore);
  }
  Log(msg);
 }
}
void Health(){
 if(InterlockedCompareExchange(&g_installed,0,0)!=1)return;
 char msg[380]{};
 sprintf_s(msg,"STAGE92_HEALTH ticks=%ld helperSet=%ld helperClear=%ld "
   "filtered=%ld overflow=%ld dropped=%ld mismatch=%ld readFail=%ld ring=8192 depth=16 gameWrites=0 DRwrites=0",
   InterlockedExchange(&g_ticks,0),InterlockedExchange(&g_helpersSet,0),
   InterlockedExchange(&g_helpersClear,0),InterlockedExchange(&g_filtered,0),
   InterlockedExchange(&g_overflow,0),InterlockedExchange(&g_dropped,0),
   InterlockedExchange(&g_mismatch,0),InterlockedExchange(&g_readFail,0));
 Log(msg);
}
} // arcana_stage92
