// Stage51: passive before/after the original per-frame CALL AF119 -> AE220.
// AE220 receives one caller-clean DWORD (PUSH 1 at AF117), native CDECL RET.
// Capture BOTH P1/P2 native objects; no changes to input, coordinates, or AI.
constexpr DWORD kS51Capacity=4096u,kS51Depth=8u,kS51Fields=13u;
struct S51Row {
  DWORD Seq=0,Side=0,Player=0,Object=0,Tid=0,CallerRva=0,CallerRead=0,Arg=0;
  ULONGLONG Q0=0,Q1=0;
  DWORD PreMask=0,PostMask=0,Pre[kS51Fields]{},Post[kS51Fields]{};
};
struct S51Pending {S51Row rows[2]{};};
volatile LONG g_s51Installed=0,g_s51Calls=0,g_s51Rows=0,g_s51Missing=0;
volatile LONG g_s51Dropped=0,g_s51Overflow=0,g_s51Mismatch=0,g_s51BadCaller=0,g_s51BadArg=0;
DWORD g_s51Native=0;
__declspec(thread) DWORD g_s51Depth=0;
__declspec(thread) S51Pending g_s51Pending[kS51Depth]{};
SRWLOCK g_s51Lock=SRWLOCK_INIT;
S51Row g_s51Ring[kS51Capacity]{};
DWORD g_s51Read=0,g_s51Write=0,g_s51Count=0,g_s51Seq=0;

void S51Log(const char* message)
{
  wchar_t path[MAX_PATH+1]{};
  if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage51GlobalCoordinateTrace.log"))return;
  SYSTEMTIME t{};GetLocalTime(&t);
  char line[2100]{};
  int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
     t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,message);
  if(n<=0)return;
  HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(f==INVALID_HANDLE_VALUE)return;
  DWORD written=0;WriteFile(f,line,static_cast<DWORD>(n),&written,nullptr);CloseHandle(f);
}
void S51Snap(DWORD object,DWORD* data,DWORD& mask)
{
  // f0 activeID+60, f1 X+7C, f2 Y+80, f3 anchorX+AC, f4 anchorY+B0,
  // f5 type+3C, f6 subtype+1A4, f7 +18, f8 +24, f9 link+2F0,
  // f10 X-velocity+88, f11 Y-velocity+8C, f12 flags+10.
  const DWORD offsets[kS51Fields]={0x60u,0x7Cu,0x80u,0xACu,0xB0u,
    0x3Cu,0x1A4u,0x18u,0x24u,0x2F0u,0x88u,0x8Cu,0x10u};
  mask=0;if(!object)return;
  for(DWORD i=0;i<kS51Fields;++i)
    if(SafeReadDword(object+offsets[i],&data[i]))mask|=1u<<i;
}
void __cdecl S51Enter(const DWORD* saved)
{
  InterlockedIncrement(&g_s51Calls);
  DWORD d=g_s51Depth++;
  if(d>=kS51Depth){InterlockedIncrement(&g_s51Overflow);return;}
  S51Pending& p=g_s51Pending[d];p=S51Pending{};
  DWORD ret=0,arg=0;
  const DWORD entryEsp=saved[3]+4u; // PUSHAD original stack: return address; arg at +4.
  const bool callerRead=SafeReadDword(entryEsp,&ret);
  const DWORD caller=(callerRead&&ret>=g_gameBase+5u)?ret-g_gameBase-5u:0u;
  const bool argRead=SafeReadDword(entryEsp+4u,&arg);
  if(!callerRead||caller!=0xAF119u){InterlockedIncrement(&g_s51BadCaller);return;}
  if(!argRead||arg!=1u)InterlockedIncrement(&g_s51BadArg);
  DWORD ptrs[2]{};
  const bool available[2]={SafeReadDword(g_playerP1+4u,&ptrs[0]),SafeReadDword(g_playerP2+4u,&ptrs[1])};
  const DWORD players[2]={g_playerP1,g_playerP2};
  for(DWORD i=0;i<2u;++i){
    S51Row& r=p.rows[i];
    if(!available[i]||!ptrs[i]){InterlockedIncrement(&g_s51Missing);continue;}
    r.Side=i+1u;r.Player=players[i];r.Object=ptrs[i];r.Tid=GetCurrentThreadId();
    r.CallerRva=caller;r.CallerRead=1u;r.Arg=arg;
    r.Q0=Stage31Qpc();
    S51Snap(r.Object,r.Pre,r.PreMask);
  }
}
void __cdecl S51Leave(const DWORD* /*saved*/)
{
  if(!g_s51Depth){InterlockedIncrement(&g_s51Mismatch);return;}
  DWORD d=--g_s51Depth;if(d>=kS51Depth)return;
  S51Pending& p=g_s51Pending[d];
  for(DWORD i=0;i<2u;++i){
    S51Row& r=p.rows[i];if(!r.Side)continue;
    r.Q1=Stage31Qpc();S51Snap(r.Object,r.Post,r.PostMask);
    AcquireSRWLockExclusive(&g_s51Lock);
    r.Seq=++g_s51Seq;
    if(g_s51Count==kS51Capacity)InterlockedIncrement(&g_s51Dropped);
    else {
      g_s51Ring[g_s51Write]=r;g_s51Write=(g_s51Write+1u)%kS51Capacity;
      ++g_s51Count;
    }
    ReleaseSRWLockExclusive(&g_s51Lock);
    InterlockedIncrement(&g_s51Rows);
    r.Side=0;
  }
}
// Naked x86 cdecl observer: preserve GPR/EFLAGS and x87/SSE for bookkeeping.
// Important: push a COPY of the caller's arg1 at [ESP+4] onto AE220's stack.
// Return with plain RET; original caller AF124 still owns the original arg.
__declspec(naked) void S51GlobalUpdateHook()
{
  __asm {
    pushfd
    pushad
    mov ebx,esp
    sub esp,210h
    lea edi,[esp+0Fh]
    and edi,0FFFFFFF0h
    fxsave [edi]
    fninit
    cld
    push ebx
    call S51Enter
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    push dword ptr [esp+4]
    call dword ptr [g_s51Native]
    add esp,4
    pushfd
    pushad
    mov ebx,esp
    sub esp,210h
    lea edi,[esp+0Fh]
    and edi,0FFFFFFF0h
    fxsave [edi]
    fninit
    cld
    push ebx
    call S51Leave
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    ret
  }
}
bool InstallStage51Hooks()
{
  S51Log("STAGE51_GLOBAL_COORD_BUILD=2026-11-10-r1");
  if(InterlockedCompareExchange(&g_s50Installed,0,0)!=1||
     InterlockedCompareExchange(&g_s49Installed,0,0)!=1||
     InterlockedCompareExchange(&g_s47Installed,0,0)!=1){
    S51Log("ERROR: Stage51 requires installed Stage50/49/47; no patches.");return false;
  }
  if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
    S51Log("ERROR: Stage51 requires FXSAVE; no patches.");return false;
  }
  BYTE* site=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAF119u));
  BYTE* native=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAE220u));
  const BYTE argPush[]={0x6A,0x01};
  const BYTE original[]={0xE8,0x02,0xF1,0xFF,0xFF};
  const BYTE afterOpcode[]={0x8B,0x35}; // AF11E relocated absolute operand omitted
  const BYTE callerClean[]={0x83,0xC4,0x04};
  const BYTE nativePrologue[]={0x55,0x8B,0xEC,0x51,0x53,0x56,0x57,0xE8,0x44,0x13,0xFF,0xFF};
  const BYTE nestedCall[]={0xE8,0xC4,0x19,0x04,0x00};
  const BYTE nativeRet[]={0xC3};
  auto same=[&](DWORD rva,const BYTE* bytes,size_t size)->bool{
    return std::memcmp(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(g_gameBase+rva)),bytes,size)==0;
  };
  auto valid=[&]()->bool{
    if(!same(0xAF117u,argPush,sizeof(argPush))||
       !same(0xAF119u,original,sizeof(original))||
       !same(0xAF11Eu,afterOpcode,sizeof(afterOpcode))||
       !same(0xAF124u,callerClean,sizeof(callerClean))||
       !same(0xAE220u,nativePrologue,sizeof(nativePrologue))||
       !same(0xAE2B7u,nestedCall,sizeof(nestedCall))||
       !same(0xAE6DDu,nativeRet,sizeof(nativeRet)))return false;
    std::int32_t rel=0;std::memcpy(&rel,site+1u,sizeof(rel));
    return site+5u+rel==native;
  };
  if(!valid()){S51Log("ERROR: Stage51 CALL/arg/native/return signature mismatch; no patches.");return false;}
  BYTE patch[5]{};
  if(!MakeRelativeCall(site,reinterpret_cast<const void*>(&S51GlobalUpdateHook),patch)){
    S51Log("ERROR: Stage51 wrapper CALL out of rel32 range; no patches.");return false;
  }
  HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
  if(!mutex){S51Log("ERROR: Stage51 patch mutex failed; no patches.");return false;}
  DWORD wait=WaitForSingleObject(mutex,INFINITE);
  if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
    CloseHandle(mutex);S51Log("ERROR: Stage51 patch mutex wait failed; no patches.");return false;
  }
  HANDLE suspended[512]{};
  DWORD stopped=SuspendOtherProcessThreads(suspended,512);
  DWORD oldProtection=0;
  bool writable=VirtualProtect(site,5u,PAGE_EXECUTE_READWRITE,&oldProtection)!=0;
  bool installed=false;
  if(writable&&valid()){
    g_s51Native=g_gameBase+0xAE220u;
    std::memcpy(site,patch,5u);FlushInstructionCache(GetCurrentProcess(),site,5u);
    InterlockedExchange(&g_s51Installed,1);installed=true;
  }
  if(writable){DWORD ignored=0;VirtualProtect(site,5u,oldProtection,&ignored);}
  ResumeThreads(suspended,stopped);ReleaseMutex(mutex);CloseHandle(mutex);
  S51Log(installed?
    "STAGE51_INSTALLED 1/1 AF119->AE220; original arg1 copied and native invoked once; original caller cleanup untouched.":
    "ERROR: Stage51 installation failed; all prior installed hooks unaffected.");
  return installed;
}
void FlushStage51()
{
  for(DWORD i=0;i<kS51Capacity;++i){
    S51Row r{};
    AcquireSRWLockExclusive(&g_s51Lock);
    bool has=g_s51Count!=0;
    if(has){
      r=g_s51Ring[g_s51Read];g_s51Read=(g_s51Read+1u)%kS51Capacity;
      --g_s51Count;
    }
    ReleaseSRWLockExclusive(&g_s51Lock);
    if(!has)break;
    DWORD changed=0;
    for(DWORD j=0;j<kS51Fields;++j)
      if((r.PreMask&r.PostMask&(1u<<j))&&r.Pre[j]!=r.Post[j])changed|=1u<<j;
    char line[1800]{};
    int n=sprintf_s(line,
      "STAGE51_GLOBAL_WINDOW seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu callRVA=%08lX "
      "arg=%08lX callerRead=%lu qpcBefore=%I64u qpcAfter=%I64u preMask=%04lX postMask=%04lX changedMask=%04lX",
      r.Seq,r.Side,r.Player,r.Object,r.Tid,r.CallerRva,r.Arg,r.CallerRead,
      r.Q0,r.Q1,r.PreMask,r.PostMask,changed);
    if(n<=0)continue;
    for(DWORD j=0;j<kS51Fields;++j){
      int m=sprintf_s(line+n,sizeof(line)-static_cast<size_t>(n)," f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
      if(m<=0){n=0;break;}n+=m;
    }
    if(n>0)S51Log(line);
  }
  LONG calls=InterlockedExchange(&g_s51Calls,0),rows=InterlockedExchange(&g_s51Rows,0);
  LONG missing=InterlockedExchange(&g_s51Missing,0);
  LONG dropped=InterlockedExchange(&g_s51Dropped,0),overflow=InterlockedExchange(&g_s51Overflow,0);
  LONG mismatch=InterlockedExchange(&g_s51Mismatch,0),badCaller=InterlockedExchange(&g_s51BadCaller,0);
  LONG badArg=InterlockedExchange(&g_s51BadArg,0);
  if(calls||rows||missing||dropped||overflow||mismatch||badCaller||badArg){
    char line[250]{};
    sprintf_s(line,
      "STAGE51_HEALTH calls=%ld rows=%ld missingObjects=%ld dropped=%ld overflow=%ld mismatch=%ld badCaller=%ld badArg=%ld ring=4096 depth=8.",
      calls,rows,missing,dropped,overflow,mismatch,badCaller,badArg);
    S51Log(line);
  }
}
