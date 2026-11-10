// Stage52: passive, original-once boundary observers for AF10D->AEF10 and AF112->AED90.
// These two native functions take no stack arguments and return with RET (cdecl-like).
// No game coordinates, controls, action IDs, or other gameplay state is modified here.
constexpr DWORD kS52Capacity=8192u,kS52Depth=8u,kS52Fields=13u;
struct S52Row {
  DWORD Seq=0,Site=0,Side=0,Player=0,Object=0,Tid=0,CallerRva=0,CallerRead=0;
  ULONGLONG Q0=0,Q1=0;
  DWORD PreMask=0,PostMask=0,Pre[kS52Fields]{},Post[kS52Fields]{};
};
struct S52Pending { S52Row rows[2]{}; };
volatile LONG g_s52Installed=0,g_s52CallsA=0,g_s52CallsB=0,g_s52Rows=0,g_s52Missing=0;
volatile LONG g_s52Dropped=0,g_s52Overflow=0,g_s52Mismatch=0,g_s52BadCaller=0;
DWORD g_s52NativeA=0,g_s52NativeB=0;
__declspec(thread) DWORD g_s52Depth=0;
__declspec(thread) S52Pending g_s52Pending[kS52Depth]{};
SRWLOCK g_s52Lock=SRWLOCK_INIT;
S52Row g_s52Ring[kS52Capacity]{};
DWORD g_s52Read=0,g_s52Write=0,g_s52Count=0,g_s52Seq=0;

void S52Log(const char* message)
{
  wchar_t path[MAX_PATH+1]{};
  if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage52PreGlobalCoordinateTrace.log"))return;
  SYSTEMTIME t{};GetLocalTime(&t);
  char line[2100]{};
  const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
    t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,message);
  if(n<=0)return;
  HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
                       nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(f==INVALID_HANDLE_VALUE)return;
  DWORD written=0;WriteFile(f,line,static_cast<DWORD>(n),&written,nullptr);CloseHandle(f);
}
void S52Snap(DWORD object,DWORD* data,DWORD& mask)
{
  // Same 13 field indices as Stage51 for direct cross-stage raw DWORD comparisons.
  // f0 O+60 active ID; f1 O+7C X; f2 O+80 Y; f3 O+AC anchor X;
  // f4 O+B0 anchor Y; f5 O+3C; f6 O+1A4; f7 O+18;
  // f8 O+24; f9 O+2F0; f10 O+88; f11 O+8C; f12 O+10.
  const DWORD offsets[kS52Fields]={0x60u,0x7Cu,0x80u,0xACu,0xB0u,
    0x3Cu,0x1A4u,0x18u,0x24u,0x2F0u,0x88u,0x8Cu,0x10u};
  mask=0;
  if(!object)return;
  for(DWORD i=0;i<kS52Fields;++i)
    if(SafeReadDword(object+offsets[i],&data[i]))mask|=1u<<i;
}
void __cdecl S52Enter(const DWORD* saved,DWORD site)
{
  if(site==0xAF10Du)InterlockedIncrement(&g_s52CallsA);
  else InterlockedIncrement(&g_s52CallsB);
  DWORD d=g_s52Depth++;
  if(d>=kS52Depth){InterlockedIncrement(&g_s52Overflow);return;}
  S52Pending& p=g_s52Pending[d];p=S52Pending{};
  DWORD ret=0;
  const DWORD entryEsp=saved[3]+4u; // PUSHAD saved ESP + skipped PUSHFD.
  const bool callerRead=SafeReadDword(entryEsp,&ret);
  const DWORD caller=(callerRead&&ret>=g_gameBase+5u)?ret-g_gameBase-5u:0u;
  if(!callerRead||caller!=site){InterlockedIncrement(&g_s52BadCaller);return;}
  const DWORD players[2]={g_playerP1,g_playerP2};
  DWORD objects[2]{};
  const bool readable[2]={SafeReadDword(g_playerP1+4u,&objects[0]),
                          SafeReadDword(g_playerP2+4u,&objects[1])};
  for(DWORD i=0;i<2u;++i){
    S52Row& r=p.rows[i];
    if(!readable[i]||!objects[i]){InterlockedIncrement(&g_s52Missing);continue;}
    r.Site=site;r.Side=i+1u;r.Player=players[i];r.Object=objects[i];
    r.Tid=GetCurrentThreadId();r.CallerRva=caller;r.CallerRead=1u;
    r.Q0=Stage31Qpc();S52Snap(r.Object,r.Pre,r.PreMask);
  }
}
void __cdecl S52Leave(const DWORD* /*saved*/)
{
  if(!g_s52Depth){InterlockedIncrement(&g_s52Mismatch);return;}
  DWORD d=--g_s52Depth;
  if(d>=kS52Depth)return;
  S52Pending& p=g_s52Pending[d];
  for(DWORD i=0;i<2u;++i){
    S52Row& r=p.rows[i];if(!r.Side)continue;
    r.Q1=Stage31Qpc();S52Snap(r.Object,r.Post,r.PostMask);
    AcquireSRWLockExclusive(&g_s52Lock);
    r.Seq=++g_s52Seq;
    if(g_s52Count==kS52Capacity)InterlockedIncrement(&g_s52Dropped);
    else {
      g_s52Ring[g_s52Write]=r;
      g_s52Write=(g_s52Write+1u)%kS52Capacity;
      ++g_s52Count;
    }
    ReleaseSRWLockExclusive(&g_s52Lock);
    InterlockedIncrement(&g_s52Rows);
    r.Side=0;
  }
}
// The two original no-argument functions are invoked EXACTLY ONCE.
// The wrappers preserve GPR/EFLAGS and x87/SSE around each observer callback.
// Each hook returns with plain RET, as do the original native functions.
__declspec(naked) void S52NativeAEF10Hook()
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
    push 0AF10Dh
    push ebx
    call S52Enter
    add esp,8
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    call dword ptr [g_s52NativeA]
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
    call S52Leave
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    ret
  }
}
__declspec(naked) void S52NativeAED90Hook()
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
    push 0AF12h
    push ebx
    call S52Enter
    add esp,8
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    call dword ptr [g_s52NativeB]
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
    call S52Leave
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    ret
  }
}
bool InstallStage52Hooks()
{
  S52Log("STAGE52_PRE_GLOBAL_COORD_BUILD=2026-11-10-r1");
  if(InterlockedCompareExchange(&g_s51Installed,0,0)!=1||
     InterlockedCompareExchange(&g_s50Installed,0,0)!=1||
     InterlockedCompareExchange(&g_s49Installed,0,0)!=1||
     InterlockedCompareExchange(&g_s47Installed,0,0)!=1){
    S52Log("ERROR: Stage52 requires installed Stage51/50/49/47; no patches.");return false;
  }
  if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
    S52Log("ERROR: Stage52 requires FXSAVE; no patches.");return false;
  }
  BYTE* siteA=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAF10Du));
  BYTE* siteB=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAF112u));
  BYTE* nativeA=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAEF10u));
  BYTE* nativeB=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAED90u));
  const BYTE callA[]={0xE8,0xFE,0xFD,0xFF,0xFF};
  const BYTE callB[]={0xE8,0x79,0xFC,0xFF,0xFF};
  const BYTE nextPush[]={0x6A,0x01}; // AF117 PUSH 1 for Stage51 AF119->AE220
  const BYTE aPrologue[]={0x55,0x8B,0xEC,0xB8,0x54,0x14,0x00,0x00,0xE8,0xB3,0xC5,0x15,0x00};
  const BYTE bPrologue[]={0x55,0x8B,0xEC,0x83,0xEC,0x08,0x53,0x56,0x33,0xF6,0x57,0x89,0x75,0xFC,0x8B,0xFF};
  const BYTE nativeRet[]={0xC3}; // both no-arg native functions RET, no stack pop
  const BYTE writeX[]={0xD9,0x56,0x7C};
  const BYTE writeAX[]={0xD9,0x9E,0xAC,0x00,0x00,0x00};
  const BYTE writeY[]={0xD9,0x96,0x80,0x00,0x00,0x00};
  const BYTE writeAY[]={0xD9,0x9E,0xB0,0x00,0x00,0x00};
  auto same=[&](DWORD rva,const BYTE* bytes,size_t count)->bool{
    return std::memcmp(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(g_gameBase+rva)),bytes,count)==0;
  };
  auto callTarget=[&](BYTE* site,BYTE* target)->bool{
    std::int32_t rel=0;std::memcpy(&rel,site+1u,sizeof(rel));
    return site+5u+rel==target;
  };
  auto valid=[&]()->bool {
    return same(0xAF10Du,callA,sizeof(callA))&&
           same(0xAF112u,callB,sizeof(callB))&&
           same(0xAF117u,nextPush,sizeof(nextPush))&&
           same(0xAEF10u,aPrologue,sizeof(aPrologue))&&
           same(0xAED90u,bPrologue,sizeof(bPrologue))&&
           same(0xAF09Au,nativeRet,sizeof(nativeRet))&&
           same(0xAEE5Eu,nativeRet,sizeof(nativeRet))&&
           same(0xAEE01u,writeX,sizeof(writeX))&&
           same(0xAEE04u,writeAX,sizeof(writeAX))&&
           same(0xAEE16u,writeY,sizeof(writeY))&&
           same(0xAEE1Cu,writeAY,sizeof(writeAY))&&
           callTarget(siteA,nativeA)&&callTarget(siteB,nativeB);
  };
  if(!valid()){S52Log("ERROR: Stage52 physical CALL/native/RET/coordinate-writer signature mismatch; no patches.");return false;}
  BYTE patchA[5]{},patchB[5]{};
  if(!MakeRelativeCall(siteA,reinterpret_cast<const void*>(&S52NativeAEF10Hook),patchA)||
     !MakeRelativeCall(siteB,reinterpret_cast<const void*>(&S52NativeAED90Hook),patchB)){
    S52Log("ERROR: Stage52 CALL wrapper outside rel32 range; no patches.");return false;
  }
  HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
  if(!mutex){S52Log("ERROR: Stage52 patch mutex failed; no patches.");return false;}
  const DWORD wait=WaitForSingleObject(mutex,INFINITE);
  if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
    CloseHandle(mutex);S52Log("ERROR: Stage52 patch mutex wait failed; no patches.");return false;
  }
  HANDLE suspended[512]{};
  const DWORD stopped=SuspendOtherProcessThreads(suspended,512);
  DWORD oldProtection=0;
  const bool writable=VirtualProtect(siteA,10u,PAGE_EXECUTE_READWRITE,&oldProtection)!=0;
  bool installed=false;
  if(writable&&valid()){
    g_s52NativeA=g_gameBase+0xAEF10u;
    g_s52NativeB=g_gameBase+0xAED90u;
    std::memcpy(siteA,patchA,5u);
    std::memcpy(siteB,patchB,5u);
    FlushInstructionCache(GetCurrentProcess(),siteA,10u);
    InterlockedExchange(&g_s52Installed,2);installed=true;
  }
  if(writable){DWORD ignored=0;VirtualProtect(siteA,10u,oldProtection,&ignored);}
  ResumeThreads(suspended,stopped);ReleaseMutex(mutex);CloseHandle(mutex);
  S52Log(installed?
    "STAGE52_INSTALLED 2/2 AF10D->AEF10 and AF112->AED90; two native calls once each; zero gameplay writes.":
    "ERROR: Stage52 hook installation aborted; prior Stage51/50/49 untouched.");
  return installed;
}
void FlushStage52()
{
  for(DWORD i=0;i<kS52Capacity;++i){
    S52Row r{};
    AcquireSRWLockExclusive(&g_s52Lock);
    const bool has=g_s52Count!=0;
    if(has){
      r=g_s52Ring[g_s52Read];g_s52Read=(g_s52Read+1u)%kS52Capacity;
      --g_s52Count;
    }
    ReleaseSRWLockExclusive(&g_s52Lock);
    if(!has)break;
    DWORD changed=0;
    for(DWORD j=0;j<kS52Fields;++j)
      if((r.PreMask&r.PostMask&(1u<<j))&&r.Pre[j]!=r.Post[j])changed|=1u<<j;
    char line[1900]{};
    int n=sprintf_s(line,
      "STAGE52_BOUNDARY_WINDOW seq=%lu site=%s side=P%lu player=%08lX object=%08lX tid=%lu callRVA=%08lX "
      "callerRead=%lu qpcBefore=%I64u qpcAfter=%I64u preMask=%04lX postMask=%04lX changedMask=%04lX",
      r.Seq,r.Site==0xAF10Du?"AEF10":"AED90",r.Side,r.Player,r.Object,r.Tid,
      r.CallerRva,r.CallerRead,r.Q0,r.Q1,r.PreMask,r.PostMask,changed);
    if(n<=0)continue;
    for(DWORD j=0;j<kS52Fields;++j){
      const int m=sprintf_s(line+n,sizeof(line)-static_cast<size_t>(n),
        " f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
      if(m<=0){n=0;break;}n+=m;
    }
    if(n>0)S52Log(line);
  }
  LONG a=InterlockedExchange(&g_s52CallsA,0),b=InterlockedExchange(&g_s52CallsB,0);
  LONG rows=InterlockedExchange(&g_s52Rows,0),missing=InterlockedExchange(&g_s52Missing,0);
  LONG dropped=InterlockedExchange(&g_s52Dropped,0),overflow=InterlockedExchange(&g_s52Overflow,0);
  LONG mismatch=InterlockedExchange(&g_s52Mismatch,0),badCaller=InterlockedExchange(&g_s52BadCaller,0);
  if(a||b||rows||missing||dropped||overflow||mismatch||badCaller){
    char line[280]{};
    sprintf_s(line,
      "STAGE52_HEALTH callsAEF10=%ld callsAED90=%ld rows=%ld missingObjects=%ld dropped=%ld "
      "overflow=%ld mismatch=%ld badCaller=%ld ring=8192 depth=8.",
      a,b,rows,missing,dropped,overflow,mismatch,badCaller);
    S52Log(line);
  }
}
