// Stage53 passive parent boundary: original CALL RVA A2B90 -> native AF0A0.
// No input/state/coordinate writes; original no-arg native executes exactly once.
// Stage52 AEF10/AED90 and Stage51/50/49 calls occur WITHIN the original AF0A0.
constexpr DWORD kS53Capacity=4096u,kS53Depth=8u,kS53Fields=13u;
struct S53Row {
  DWORD Seq=0,Side=0,Player=0,Object=0,Tid=0,CallerRva=0,CallerRead=0;
  ULONGLONG Q0=0,Q1=0;
  DWORD PreMask=0,PostMask=0,Pre[kS53Fields]{},Post[kS53Fields]{};
};
struct S53Pending { S53Row rows[2]{}; };
volatile LONG g_s53Installed=0,g_s53Calls=0,g_s53Rows=0,g_s53Missing=0;
volatile LONG g_s53Dropped=0,g_s53Overflow=0,g_s53Mismatch=0,g_s53BadCaller=0;
DWORD g_s53Native=0;
__declspec(thread) DWORD g_s53Depth=0;
__declspec(thread) S53Pending g_s53Pending[kS53Depth]{};
SRWLOCK g_s53Lock=SRWLOCK_INIT;
S53Row g_s53Ring[kS53Capacity]{};
DWORD g_s53Read=0,g_s53Write=0,g_s53Count=0,g_s53Seq=0;

void S53Log(const char* message)
{
  wchar_t path[MAX_PATH+1]{};
  if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage53ParentCoordinateTrace.log"))return;
  SYSTEMTIME t{};GetLocalTime(&t);
  char line[2100]{};
  const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
    t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,message);
  if(n<=0)return;
  HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(f==INVALID_HANDLE_VALUE)return;
  DWORD written=0;WriteFile(f,line,static_cast<DWORD>(n),&written,nullptr);CloseHandle(f);
}
void S53Snap(DWORD object,DWORD* data,DWORD& mask)
{
  // Same thirteen field indices and offsets as Stage52 and Stage51.
  // f0=O+60, f1=O+7C X, f2=O+80 Y, f3=O+AC anchor X,
  // f4=O+B0 anchor Y, f5=O+3C, f6=O+1A4, f7=O+18,
  // f8=O+24, f9=O+2F0, f10=O+88, f11=O+8C, f12=O+10.
  const DWORD offsets[kS53Fields]={0x60u,0x7Cu,0x80u,0xACu,0xB0u,
    0x3Cu,0x1A4u,0x18u,0x24u,0x2F0u,0x88u,0x8Cu,0x10u};
  mask=0;if(!object)return;
  for(DWORD i=0;i<kS53Fields;++i)
    if(SafeReadDword(object+offsets[i],&data[i]))mask|=1u<<i;
}
void __cdecl S53Enter(const DWORD* saved)
{
  InterlockedIncrement(&g_s53Calls);
  DWORD d=g_s53Depth++;
  if(d>=kS53Depth){InterlockedIncrement(&g_s53Overflow);return;}
  S53Pending& p=g_s53Pending[d];p=S53Pending{};
  DWORD ret=0;
  const DWORD entryEsp=saved[3]+4u; // PUSHAD saved stack + PUSHFD.
  const bool callerRead=SafeReadDword(entryEsp,&ret);
  const DWORD caller=(callerRead&&ret>=g_gameBase+5u)?ret-g_gameBase-5u:0u;
  if(!callerRead||caller!=0xA2B90u){InterlockedIncrement(&g_s53BadCaller);return;}
  const DWORD players[2]={g_playerP1,g_playerP2};
  DWORD objects[2]{};
  const bool readable[2]={SafeReadDword(g_playerP1+4u,&objects[0]),
                          SafeReadDword(g_playerP2+4u,&objects[1])};
  for(DWORD i=0;i<2u;++i){
    S53Row& r=p.rows[i];
    if(!readable[i]||!objects[i]){InterlockedIncrement(&g_s53Missing);continue;}
    r.Side=i+1u;r.Player=players[i];r.Object=objects[i];
    r.Tid=GetCurrentThreadId();r.CallerRva=caller;r.CallerRead=1u;
    r.Q0=Stage31Qpc();S53Snap(r.Object,r.Pre,r.PreMask);
  }
}
void __cdecl S53Leave(const DWORD* /*saved*/)
{
  if(!g_s53Depth){InterlockedIncrement(&g_s53Mismatch);return;}
  DWORD d=--g_s53Depth;if(d>=kS53Depth)return;
  S53Pending& p=g_s53Pending[d];
  for(DWORD i=0;i<2u;++i){
    S53Row& r=p.rows[i];if(!r.Side)continue;
    r.Q1=Stage31Qpc();S53Snap(r.Object,r.Post,r.PostMask);
    AcquireSRWLockExclusive(&g_s53Lock);
    r.Seq=++g_s53Seq;
    if(g_s53Count==kS53Capacity)InterlockedIncrement(&g_s53Dropped);
    else {
      g_s53Ring[g_s53Write]=r;g_s53Write=(g_s53Write+1u)%kS53Capacity;
      ++g_s53Count;
    }
    ReleaseSRWLockExclusive(&g_s53Lock);
    InterlockedIncrement(&g_s53Rows);
    r.Side=0;
  }
}
// Original no-arg function AF0A0, plain RET. Caller A2B90 has no arguments.
// This hook brackets the entire AF0A0, including existing Stage52->Stage49 hooks.
__declspec(naked) void S53ParentAF0A0Hook()
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
    call S53Enter
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    call dword ptr [g_s53Native]
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
    call S53Leave
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    ret
  }
}
bool InstallStage53Hooks()
{
  S53Log("STAGE53_PARENT_COORD_BUILD=2026-11-10-r1");
  if(InterlockedCompareExchange(&g_s52Installed,0,0)!=2||
     InterlockedCompareExchange(&g_s51Installed,0,0)!=1||
     InterlockedCompareExchange(&g_s50Installed,0,0)!=1||
     InterlockedCompareExchange(&g_s49Installed,0,0)!=1||
     InterlockedCompareExchange(&g_s47Installed,0,0)!=1){
    S53Log("ERROR: Stage53 requires installed Stage52/51/50/49/47; no patches.");return false;
  }
  if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
    S53Log("ERROR: Stage53 requires FXSAVE; no patches.");return false;
  }
  BYTE* site=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xA2B90u));
  BYTE* native=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAF0A0u));
  const BYTE priorCall[]={0xE8,0xD0,0x7F,0x01,0x00}; // A2B8B -> BAB60
  const BYTE original[]={0xE8,0x0B,0xC5,0x00,0x00}; // A2B90 -> AF0A0
  const BYTE afterCall[]={0xE8,0xF6,0xCE,0x02,0x00}; // A2B95 -> CFA90
  const BYTE nativePrologue[]={0x55,0x8B,0xEC,0xB8,0x6C,0x14,0x00,0x00,0xE8,0x23,0xC4,0x15,0x00};
  const BYTE nativeReturn[]={0x5F,0x5E,0x5B,0x8B,0xE5,0x5D,0xC3}; // AF34C..352
  auto same=[&](DWORD rva,const BYTE* bytes,size_t count)->bool{
    return std::memcmp(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(g_gameBase+rva)),bytes,count)==0;
  };
  auto valid=[&]()->bool{
    if(!same(0xA2B8Bu,priorCall,sizeof(priorCall))||
       !same(0xA2B90u,original,sizeof(original))||
       !same(0xA2B95u,afterCall,sizeof(afterCall))||
       !same(0xAF0A0u,nativePrologue,sizeof(nativePrologue))||
       !same(0xAF34Cu,nativeReturn,sizeof(nativeReturn)))return false;
    std::int32_t rel=0;std::memcpy(&rel,site+1u,sizeof(rel));
    return site+5u+rel==native;
  };
  if(!valid()){S53Log("ERROR: Stage53 parent CALL/native ABI signature mismatch; no patches.");return false;}
  BYTE patch[5]{};
  if(!MakeRelativeCall(site,reinterpret_cast<const void*>(&S53ParentAF0A0Hook),patch)){
    S53Log("ERROR: Stage53 wrapper call outside rel32 range; no patches.");return false;
  }
  HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
  if(!mutex){S53Log("ERROR: Stage53 mutex failed; no patches.");return false;}
  const DWORD wait=WaitForSingleObject(mutex,INFINITE);
  if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
    CloseHandle(mutex);S53Log("ERROR: Stage53 mutex wait failed; no patches.");return false;
  }
  HANDLE suspended[512]{};
  const DWORD stopped=SuspendOtherProcessThreads(suspended,512);
  DWORD oldProtection=0;
  const bool writable=VirtualProtect(site,5u,PAGE_EXECUTE_READWRITE,&oldProtection)!=0;
  bool installed=false;
  if(writable&&valid()){
    g_s53Native=g_gameBase+0xAF0A0u;
    std::memcpy(site,patch,5u);
    FlushInstructionCache(GetCurrentProcess(),site,5u);
    InterlockedExchange(&g_s53Installed,1);
    installed=true;
  }
  if(writable){DWORD ignored=0;VirtualProtect(site,5u,oldProtection,&ignored);}
  ResumeThreads(suspended,stopped);ReleaseMutex(mutex);CloseHandle(mutex);
  S53Log(installed?
    "STAGE53_INSTALLED 1/1 A2B90->AF0A0; native called once, no game-state writes; Stage52/51/50/49 preserved.":
    "ERROR: Stage53 install failed, previous hooks unaffected.");
  return installed;
}
void FlushStage53()
{
  for(DWORD i=0;i<kS53Capacity;++i){
    S53Row r{};
    AcquireSRWLockExclusive(&g_s53Lock);
    const bool has=g_s53Count!=0;
    if(has){
      r=g_s53Ring[g_s53Read];g_s53Read=(g_s53Read+1u)%kS53Capacity;
      --g_s53Count;
    }
    ReleaseSRWLockExclusive(&g_s53Lock);
    if(!has)break;
    DWORD changed=0;
    for(DWORD j=0;j<kS53Fields;++j)
      if((r.PreMask&r.PostMask&(1u<<j))&&r.Pre[j]!=r.Post[j])changed|=1u<<j;
    char line[1900]{};
    int n=sprintf_s(line,
      "STAGE53_PARENT_WINDOW seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu callRVA=%08lX "
      "callerRead=%lu qpcBefore=%I64u qpcAfter=%I64u preMask=%04lX postMask=%04lX changedMask=%04lX",
      r.Seq,r.Side,r.Player,r.Object,r.Tid,r.CallerRva,r.CallerRead,
      r.Q0,r.Q1,r.PreMask,r.PostMask,changed);
    if(n<=0)continue;
    for(DWORD j=0;j<kS53Fields;++j){
      const int m=sprintf_s(line+n,sizeof(line)-static_cast<size_t>(n),
        " f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
      if(m<=0){n=0;break;}n+=m;
    }
    if(n>0)S53Log(line);
  }
  LONG calls=InterlockedExchange(&g_s53Calls,0),rows=InterlockedExchange(&g_s53Rows,0);
  LONG missing=InterlockedExchange(&g_s53Missing,0);
  LONG dropped=InterlockedExchange(&g_s53Dropped,0),overflow=InterlockedExchange(&g_s53Overflow,0);
  LONG mismatch=InterlockedExchange(&g_s53Mismatch,0),badCaller=InterlockedExchange(&g_s53BadCaller,0);
  if(calls||rows||missing||dropped||overflow||mismatch||badCaller){
    char line[280]{};
    sprintf_s(line,
      "STAGE53_HEALTH calls=%ld rows=%ld missingObjects=%ld dropped=%ld overflow=%ld "
      "mismatch=%ld badCaller=%ld ring=4096 depth=8.",
      calls,rows,missing,dropped,overflow,mismatch,badCaller);
    S53Log(line);
  }
}
