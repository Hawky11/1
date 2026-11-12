// Stage94 r1: passive F3930-return result and type1 gate observer.
// Native F7AD6 replays ADD ESP,14h; MOV ESI,EAX once after the real F3930.
// Native F7B0A replays JNE as branch-to-exact-native-destination, with
// GPR, FLAGS, FPU/XMM restored before the conditional decision.
// No game-state/DR writes, no new F7C5D/EE306 hook, Stage36 stays OFF.
#ifndef _M_IX86
#error Stage94 requires MSVC Windows x86 inline assembly.
#endif
extern "C" {
DWORD g_Stage94ResultResume=0, g_Stage94GateAllow=0, g_Stage94GateReject=0;
void __cdecl Stage94ProducerReturned(const DWORD* saved);
void __cdecl Stage94GateDecision(const DWORD* saved);
}
namespace arcana_stage94 {
constexpr DWORD kResult=0x000F7AD6u, kGate=0x000F7B0Au;
constexpr DWORD kResultResume=0x000F7ADBu, kGateAllow=0x000F7B10u, kGateReject=0x000F7C68u;
constexpr DWORD kCapacity=8192u;
constexpr BYTE kResultOriginal[5]={0x83,0xC4,0x14,0x8B,0xF0};
constexpr BYTE kGateOriginal[6]={0x0F,0x85,0x58,0x01,0x00,0x00};
constexpr BYTE kProducerCall[5]={0xE8,0x5A,0xBE,0xFF,0xFF};
constexpr BYTE kProducerNext[6]={0x8B,0x4D,0xF8,0x83,0xF9,0xFF};
constexpr BYTE kGateCmp[3]={0x83,0xF9,0x01};
constexpr BYTE kGateNext[3]={0x83,0xFE,0x20};
constexpr BYTE kFinalArgs[4]={0x56,0x6A,0x01,0x53};
struct Event {
 DWORD seq,kind,side,player,tid,frame,esi,eax,ecx,eflags;
 DWORD sourceSeq,sourceEax,sourceValueMatches,sourceFrameMatches;
 DWORD flag3B70,readMask,gateForward,gatePredicted,valid;
 ULONGLONG qpc,sourceQpcDelta;
};
struct LastProducer { DWORD seq,player,frame,result;ULONGLONG qpc; };
__declspec(thread) LastProducer g_last{};
SRWLOCK g_lock=SRWLOCK_INIT;
Event g_events[kCapacity]{};
DWORD g_read=0,g_write=0,g_count=0;
volatile LONG g_installed=0,g_seq=0,g_producerCalls=0,g_gateCalls=0;
volatile LONG g_forward=0,g_reject=0,g_filtered=0,g_dropped=0,g_mismatch=0,g_readFail=0;
void Log(const char* str){
 wchar_t path[MAX_PATH+1]{};
 if(!arcana_stage81::GetPath(path,_countof(path),L"ArcanaAI_Stage94ProducerGateTrace.log"))return;
 HANDLE h=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(h==INVALID_HANDLE_VALUE)return;
 DWORD n=0;WriteFile(h,str,static_cast<DWORD>(std::strlen(str)),&n,nullptr);
 WriteFile(h,"\r\n",2u,&n,nullptr);CloseHandle(h);
}
ULONGLONG Qpc(){LARGE_INTEGER q{};QueryPerformanceCounter(&q);return static_cast<ULONGLONG>(q.QuadPart);}
bool ReadFlag(DWORD player,DWORD& flag){
 __try{flag=*reinterpret_cast<volatile DWORD*>(static_cast<uintptr_t>(player+0x3B70u));return true;}
 __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&g_readFail);return false;}
}
void Push(const Event& e){
 AcquireSRWLockExclusive(&g_lock);
 if(g_count==kCapacity)InterlockedIncrement(&g_dropped);
 else{g_events[g_write]=e;g_write=(g_write+1u)%kCapacity;++g_count;}
 ReleaseSRWLockExclusive(&g_lock);
}
void Producer(const DWORD* saved){
 InterlockedIncrement(&g_producerCalls);
 if(!saved)return;
 Event e{};e.kind=1u;e.player=saved[0];e.side=arcana_stage81::Side(e.player);
 if(!e.side){InterlockedIncrement(&g_filtered);return;}
 e.tid=GetCurrentThreadId();e.frame=saved[2];e.esi=saved[1];e.eax=saved[7];
 e.ecx=saved[6];e.eflags=saved[8];e.qpc=Qpc();
 e.seq=static_cast<DWORD>(InterlockedIncrement(&g_seq));
 if(ReadFlag(e.player,e.flag3B70))e.readMask=1u;
 e.valid=e.readMask;
 g_last.seq=e.seq;g_last.player=e.player;g_last.frame=e.frame;
 g_last.result=e.eax;g_last.qpc=e.qpc;
 Push(e);
}
void Gate(const DWORD* saved){
 InterlockedIncrement(&g_gateCalls);
 if(!saved)return;
 Event e{};e.kind=2u;e.player=saved[0];e.side=arcana_stage81::Side(e.player);
 if(!e.side){InterlockedIncrement(&g_filtered);return;}
 e.tid=GetCurrentThreadId();e.frame=saved[2];e.esi=saved[1];e.eax=saved[7];
 e.ecx=saved[6];e.eflags=saved[8];e.qpc=Qpc();
 e.seq=static_cast<DWORD>(InterlockedIncrement(&g_seq));
 if(ReadFlag(e.player,e.flag3B70))e.readMask=1u;
 // Original F7B07 CMP ECX,1 sets ZF; original F7B0A JNE rejects if ZF=0.
 e.gateForward=((e.eflags&0x40u)!=0)?1u:0u;
 e.gatePredicted=e.ecx==1u?1u:0u;
 if(e.gateForward)InterlockedIncrement(&g_forward);else InterlockedIncrement(&g_reject);
 e.valid=e.gateForward==e.gatePredicted&&e.readMask==1u?1u:0u;
 if(!e.valid)InterlockedIncrement(&g_mismatch);
 // Same frame is only a provenance CANDIDATE; not proof no other ESI writer ran.
 if(g_last.seq&&g_last.player==e.player&&g_last.frame==e.frame&&
    e.qpc>=g_last.qpc&&(e.qpc-g_last.qpc)<=1000000ull){
  e.sourceFrameMatches=1u;e.sourceSeq=g_last.seq;e.sourceEax=g_last.result;
  e.sourceQpcDelta=e.qpc-g_last.qpc;
  e.sourceValueMatches=e.esi==g_last.result?1u:0u;
 }
 Push(e);
}
struct Suspended{HANDLE h[512]{};DWORD count=0;};
void Resume(Suspended& peers){
 while(peers.count){HANDLE h=peers.h[--peers.count];ResumeThread(h);CloseHandle(h);}
}
bool SuspendAll(Suspended& peers){
 const DWORD pid=GetCurrentProcessId(),self=GetCurrentThreadId();
 HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
 if(snap==INVALID_HANDLE_VALUE)return false;
 THREADENTRY32 t{};t.dwSize=sizeof(t);BOOL more=Thread32First(snap,&t);bool ok=more!=FALSE;
 while(more&&ok){
  if(t.th32OwnerProcessID==pid&&t.th32ThreadID!=self){
   if(peers.count==_countof(peers.h)){ok=false;break;}
   HANDLE h=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT,FALSE,t.th32ThreadID);
   if(!h){ok=false;break;}
   if(SuspendThread(h)==static_cast<DWORD>(-1)){CloseHandle(h);ok=false;break;}
   peers.h[peers.count++]=h;
   CONTEXT c{};c.ContextFlags=CONTEXT_CONTROL;
   if(!GetThreadContext(h,&c)){ok=false;break;}
   const DWORD base=arcana_stage81::g_gameBase;
   if((c.Eip>=base+kResult&&c.Eip<base+kResultResume)||
      (c.Eip>=base+kGate&&c.Eip<base+kGateAllow)){ok=false;break;}
  }
  t.dwSize=sizeof(t);more=Thread32Next(snap,&t);
 }
 CloseHandle(snap);if(!ok)Resume(peers);return ok;
}
bool SignatureOK(){
 const BYTE* b=reinterpret_cast<const BYTE*>(static_cast<uintptr_t>(arcana_stage81::g_gameBase));
 if(std::memcmp(b+kResult,kResultOriginal,sizeof(kResultOriginal))!=0||
    std::memcmp(b+kGate,kGateOriginal,sizeof(kGateOriginal))!=0||
    std::memcmp(b+kResult-5u,kProducerCall,sizeof(kProducerCall))!=0||
    std::memcmp(b+kResultResume,kProducerNext,sizeof(kProducerNext))!=0||
    std::memcmp(b+kGate-3u,kGateCmp,sizeof(kGateCmp))!=0||
    std::memcmp(b+kGateAllow,kGateNext,sizeof(kGateNext))!=0||
    std::memcmp(b+0xF7C59u,kFinalArgs,sizeof(kFinalArgs))!=0)return false;
 std::int32_t callRel=0,jneRel=0;
 std::memcpy(&callRel,b+kResult-4u,4u);
 std::memcpy(&jneRel,b+kGate+2u,4u);
 if(static_cast<DWORD>(kResult+callRel)!=0xF3930u||
    static_cast<DWORD>(kGateAllow+jneRel)!=kGateReject)return false;
 return true;
}
// Diagnostics ONLY: read the live code bytes after earlier stages have installed.
// Never trust an original-file comparison as proof of untouched runtime code.
// Called outside peer-thread suspension; no disassembler or memory modifications.
void DescribeSignatureFailure(){
 const BYTE* b=reinterpret_cast<const BYTE*>(static_cast<uintptr_t>(arcana_stage81::g_gameBase));
 struct Guard { const char* label; DWORD rva; const BYTE* expected; DWORD length; };
 const Guard checks[]={
  {"RESULT_ADD_MOV",kResult,kResultOriginal,sizeof(kResultOriginal)},
  {"GATE_JNE",kGate,kGateOriginal,sizeof(kGateOriginal)},
  {"F3930_CALL",kResult-5u,kProducerCall,sizeof(kProducerCall)},
  {"RESULT_NEXT",kResultResume,kProducerNext,sizeof(kProducerNext)},
  {"GATE_CMP",kGate-3u,kGateCmp,sizeof(kGateCmp)},
  {"GATE_NEXT",kGateAllow,kGateNext,sizeof(kGateNext)},
  {"FINAL_PUSH",0xF7C59u,kFinalArgs,sizeof(kFinalArgs)}
 };
 DWORD bad=0u;
 for(const Guard& check:checks){
  if(std::memcmp(b+check.rva,check.expected,check.length)==0)continue;
  ++bad;
  char actual[48]{},reference[48]{};
  for(DWORD j=0;j<check.length;++j){
   sprintf_s(actual+j*2u,sizeof(actual)-j*2u,"%02X",b[check.rva+j]);
   sprintf_s(reference+j*2u,sizeof(reference)-j*2u,"%02X",check.expected[j]);
  }
  char line[320]{};
  sprintf_s(line,"STAGE94_SIGFAIL region=%s rva=%06lX expected=%s actual=%s gameBase=%08lX",
   check.label,check.rva,reference,actual,arcana_stage81::g_gameBase);
  Log(line);
  // A preexisting near JMP is probably another installed hook, but do not infer its owner.
  if(b[check.rva]==0xE9&&check.length>=5u){
   std::int32_t rel=0;std::memcpy(&rel,b+check.rva+1u,4u);
   const DWORD target=static_cast<DWORD>(arcana_stage81::g_gameBase+check.rva+5u+rel);
   sprintf_s(line,"STAGE94_SIGFAIL_JMP region=%s siteRVA=%06lX liveTargetVA=%08lX owner=UNKNOWN",
    check.label,check.rva,target);
   Log(line);
  }
 }
 std::int32_t relCall=0,relJne=0;
 std::memcpy(&relCall,b+kResult-4u,4u);
 std::memcpy(&relJne,b+kGate+2u,4u);
 const DWORD callDst=static_cast<DWORD>(kResult+relCall);
 const DWORD jneDst=static_cast<DWORD>(kGateAllow+relJne);
 if(callDst!=0xF3930u||jneDst!=kGateReject){
  ++bad;
  char line[240]{};
  sprintf_s(line,"STAGE94_SIGFAIL_TARGET callF3930Expected=0F3930 actualRVA=%06lX "
   "jneExpected=0F7C68 actualRVA=%06lX",callDst,jneDst);
  Log(line);
 }
 if(!bad)Log("STAGE94_SIGFAIL_UNEXPLAINED signature guard refused but no mismatched bytes found");
}
bool Patch(BYTE* p,const BYTE* bytes,DWORD len){
 DWORD old=0,unused=0;
 if(!VirtualProtect(p,len,PAGE_EXECUTE_READWRITE,&old))return false;
 std::memcpy(p,bytes,len);
 const bool flush=FlushInstructionCache(GetCurrentProcess(),p,len)!=FALSE;
 const bool restore=VirtualProtect(p,len,old,&unused)!=FALSE;
 return flush&&restore;
}
} // namespace arcana_stage94
extern "C" {
void __cdecl Stage94ProducerReturned(const DWORD* saved){arcana_stage94::Producer(saved);}
void __cdecl Stage94GateDecision(const DWORD* saved){arcana_stage94::Gate(saved);}
}
// The original F3930 has *already returned*. Replay only its original caller
// epilogue: ADD ESP,14h then MOV ESI,EAX (same five bytes, same FLAGS result).
extern "C" __declspec(naked) void Stage94ResultHook(){
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
  call Stage94ProducerReturned
  lea esp,[esp+4]
  fxrstor [esp]
  mov esp,ebx
  popad
  popfd
  // Emit the exact original five bytes: ADD ESP,14h; MOV ESI,EAX.
  // x86 assemblers can otherwise choose the equivalent 89 C6 MOV encoding.
  _emit 0x83
  _emit 0xC4
  _emit 0x14
  _emit 0x8B
  _emit 0xF0
  jmp dword ptr [g_Stage94ResultResume]
 }
}
// The original F7B0A is a JNE after CMP ECX,1. No game data changes.
// Restore the actual CMP flags, then branch to the original native targets.
extern "C" __declspec(naked) void Stage94GateHook(){
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
  call Stage94GateDecision
  lea esp,[esp+4]
  fxrstor [esp]
  mov esp,ebx
  popad
  popfd
  jnz Stage94Reject
  jmp dword ptr [g_Stage94GateAllow]
 Stage94Reject:
  jmp dword ptr [g_Stage94GateReject]
 }
}
namespace arcana_stage94 {
bool Install(){
 Log("STAGE94_BUILD=2026-11-12-r2 DIAGNOSTIC_SOURCE_ONLY F7AD6 after-F3930 producer+F7B0A native JNE; originalOnce=1 gameWrites=0 DRwrites=0");
 wchar_t ini[MAX_PATH+1]{};
 if(arcana_stage81::GetPath(ini,_countof(ini),L"ArcanaAI.ini")&&
    GetPrivateProfileIntW(L"AIResearch",L"EnableStage94ProducerGateTrace",1,ini)==0){
  Log("STAGE94_DISABLED EnableStage94ProducerGateTrace=0");return false;
 }
 HMODULE game=GetModuleHandleW(L"AALib.exe");
 if(sizeof(void*)!=4u||!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)||
    !arcana_stage81::Fingerprint(game)||
    InterlockedCompareExchange(&arcana_stage93::g_installed,0,0)!=1){
  Log("STAGE94_ERROR missing Stage93/fingerprint/x86 prerequisites; no patch");return false;
 }
 if(InterlockedCompareExchange(&g_installed,0,0)!=0){
  Log("STAGE94_ERROR observer already installed; no patch");return false;
 }
 if(!SignatureOK()){
  DescribeSignatureFailure();
  Log("STAGE94_ERROR native signature already modified/not exact; no patch");return false;
 }
 BYTE* base=reinterpret_cast<BYTE*>(static_cast<uintptr_t>(arcana_stage81::g_gameBase));
 const void* hooks[2]={reinterpret_cast<const void*>(&Stage94ResultHook),
                       reinterpret_cast<const void*>(&Stage94GateHook)};
 const DWORD sites[2]={kResult,kGate};
 BYTE p0[5]={0xE9,0,0,0,0},p1[6]={0xE9,0,0,0,0,0x90};
 BYTE* patches[2]={p0,p1};
 for(DWORD i=0;i<2;++i){
  const std::intptr_t delta=reinterpret_cast<std::intptr_t>(hooks[i])-
                             reinterpret_cast<std::intptr_t>(base+sites[i]+5u);
  if(delta<static_cast<std::intptr_t>(INT32_MIN)||delta>static_cast<std::intptr_t>(INT32_MAX)){
   Log("STAGE94_ERROR rel32 out of range; no patch");return false;
  }
  const std::int32_t rel=static_cast<std::int32_t>(delta);
  std::memcpy(patches[i]+1u,&rel,4u);
 }
 HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
 if(!mutex){Log("STAGE94_ERROR patch mutex creation failed");return false;}
 const DWORD wait=WaitForSingleObject(mutex,10000u);
 if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){CloseHandle(mutex);Log("STAGE94_ERROR patch mutex wait failed");return false;}
 Suspended peers{};
 bool ok=SuspendAll(peers),rollback=true;DWORD touched=0;
 if(ok){
  ok=SignatureOK();
  if(ok){
   g_Stage94ResultResume=arcana_stage81::g_gameBase+kResultResume;
   g_Stage94GateAllow=arcana_stage81::g_gameBase+kGateAllow;
   g_Stage94GateReject=arcana_stage81::g_gameBase+kGateReject;
   touched|=1u;if(!Patch(base+kResult,p0,sizeof(p0)))ok=false;
   if(ok){touched|=2u;if(!Patch(base+kGate,p1,sizeof(p1)))ok=false;}
  }
  if(!ok){
   if(touched&1u)if(!Patch(base+kResult,kResultOriginal,sizeof(kResultOriginal)))rollback=false;
   if(touched&2u)if(!Patch(base+kGate,kGateOriginal,sizeof(kGateOriginal)))rollback=false;
  }
  Resume(peers);
 }
 ReleaseMutex(mutex);CloseHandle(mutex);
 if(!ok||!rollback){
  Log("STAGE94_ERROR install refused; rollback attempted; restore tested Stage93 DLL on instability");return false;
 }
 InterlockedExchange(&g_installed,1);
 Log("STAGE94_INSTALLED 2/2 result=0F7AD6 resume=0F7ADB branch=0F7B0A allow=0F7B10 reject=0F7C68 nativeJNE=1 noF7C5D=1 noEE306=1 gameWrites=0 DRwrites=0");
 return true;
}
void Flush(){
 if(InterlockedCompareExchange(&g_installed,0,0)!=1)return;
 for(DWORD i=0;i<kCapacity;++i){
  Event e{};AcquireSRWLockExclusive(&g_lock);
  const bool has=g_count!=0;
  if(has){e=g_events[g_read];g_read=(g_read+1u)%kCapacity;--g_count;}
  ReleaseSRWLockExclusive(&g_lock);if(!has)break;
  char m[740]{};
  if(e.kind==1u){
   sprintf_s(m,"STAGE94_PRODUCER seq=%lu side=P%lu player=%08lX tid=%lu frame=%08lX "
    "qpc=%I64u callRVA=0F7AD1 callee=0F3930 resultEax=%08lX prevEsi=%08lX "
    "ecx=%08lX originalFlags=%08lX flag3B70=%08lX readMask=%lu valid=%lu",
    e.seq,e.side,e.player,e.tid,e.frame,e.qpc,e.eax,e.esi,e.ecx,e.eflags,
    e.flag3B70,e.readMask,e.valid);
  }else{
   sprintf_s(m,"STAGE94_GATE seq=%lu side=P%lu player=%08lX tid=%lu frame=%08lX "
    "qpc=%I64u branchRVA=0F7B0A chosenEsi=%08lX ecx=%08lX originalFlags=%08lX "
    "forwardType1=%lu predicted=%lu sourceCandidateSeq=%lu sourceCandidateEax=%08lX "
    "sourceFrameMatches=%lu sourceValueMatches=%lu sourceDeltaQpc=%I64u "
    "flag3B70=%08lX readMask=%lu valid=%lu",
    e.seq,e.side,e.player,e.tid,e.frame,e.qpc,e.esi,e.ecx,e.eflags,
    e.gateForward,e.gatePredicted,e.sourceSeq,e.sourceEax,e.sourceFrameMatches,
    e.sourceValueMatches,e.sourceQpcDelta,e.flag3B70,e.readMask,e.valid);
  }
  Log(m);
 }
}
void Health(){
 if(InterlockedCompareExchange(&g_installed,0,0)!=1)return;
 char m[400]{};
 sprintf_s(m,"STAGE94_HEALTH producer=%ld gates=%ld forward=%ld reject=%ld "
   "filtered=%ld dropped=%ld mismatch=%ld readFail=%ld ring=8192 nativeJNE=1 gameWrites=0 DRwrites=0",
   InterlockedExchange(&g_producerCalls,0),InterlockedExchange(&g_gateCalls,0),
   InterlockedExchange(&g_forward,0),InterlockedExchange(&g_reject,0),
   InterlockedExchange(&g_filtered,0),InterlockedExchange(&g_dropped,0),
   InterlockedExchange(&g_mismatch,0),InterlockedExchange(&g_readFail,0));
 Log(m);
}
} // namespace arcana_stage94
