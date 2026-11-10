// Stage42: passive native 1E9F62 CALL -> 1E9C70 activation boundary.
// Included in Stage38CallbackWindow.inl AFTER Stage40RequestConsumer.inl.
// Only read-only snapshots of the actual P1/P2 object. Original callee once.
// No input injection, no game memory writes except optional rel32 CALL patch.
constexpr DWORD kS42Capacity=2048u,kS42MaxDepth=8u;
struct S42Rec {
    DWORD Seq=0,Tid=0,Side=0,Player=0,Object=0;
    ULONGLONG Q0=0,Q1=0;
    DWORD PreMask=0,PostMask=0,Pre[5]{},Post[5]{};
    // Stage43 caller provenance: actual stack returns before original 1E9C70.
    DWORD NativeRetRVA=0xFFFFFFFFu,CallerRetRVA=0xFFFFFFFFu;
    DWORD CallerRVA=0xFFFFFFFFu,KnownCaller=0,CallerRead=0;
};
volatile LONG g_s42Installed=0,g_s42Dropped=0,g_s42Overflow=0,g_s42Mismatch=0;
volatile LONG g_s42AllCalls=0,g_s42OtherObjects=0;
DWORD g_s42NativeActivation=0;
__declspec(thread) DWORD g_s42Depth=0;
__declspec(thread) S42Rec g_s42Pending[kS42MaxDepth]{};
SRWLOCK g_s42Lock=SRWLOCK_INIT;
S42Rec g_s42Ring[kS42Capacity]{};
DWORD g_s42Read=0,g_s42Write=0,g_s42Count=0,g_s42Seq=0;

void S42Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage42ActionActivationTrace.log"))return;
    SYSTEMTIME now{};GetLocalTime(&now);
    char line[1450]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,message);
    if(n<=0)return;
    HANDLE file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return;
    DWORD written=0;WriteFile(file,line,static_cast<DWORD>(n),&written,nullptr);
    CloseHandle(file);
}
void S43Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage43ActivationOriginTrace.log"))return;
    SYSTEMTIME now{};GetLocalTime(&now);
    char line[1250]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,message);
    if(n<=0)return;
    HANDLE file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return;
    DWORD written=0;WriteFile(file,line,static_cast<DWORD>(n),&written,nullptr);
    CloseHandle(file);
}
void __cdecl S42Enter(const DWORD* saved)
{
    InterlockedIncrement(&g_s42AllCalls);
    const DWORD d=g_s42Depth++;
    if(d>=kS42MaxDepth){InterlockedIncrement(&g_s42Overflow);return;}
    S42Rec& r=g_s42Pending[d];r=S42Rec{};
    const DWORD object=saved[6]; // PUSHAD ECX, native 1E9C70 thiscall O
    DWORD p1Object=0,p2Object=0;
    const bool validP1=SafeReadDword(g_playerP1+4u,&p1Object);
    const bool validP2=SafeReadDword(g_playerP2+4u,&p2Object);
    if(validP1 && object==p1Object){r.Side=1;r.Player=g_playerP1;}
    else if(validP2 && object==p2Object){r.Side=2;r.Player=g_playerP2;}
    else {InterlockedIncrement(&g_s42OtherObjects);return;}
    r.Object=object;
    r.Tid=GetCurrentThreadId();
    // PUSHFD; PUSHAD captures ESP in saved[3] AFTER PUSHFD.
    // Thus [saved[3]+4] is return RVA 1E9F67 from the hooked CALL;
    // [saved[3]+8] is the caller's return address out of leaf 1E9F40.
    // 1E9F40 makes no stack frame and pushes no arguments at this point.
    DWORD nativeRet=0, callerRet=0;
    const DWORD stackAfterPushfd=saved[3];
    const bool nOk=SafeReadDword(stackAfterPushfd+4u,&nativeRet);
    const bool cOk=SafeReadDword(stackAfterPushfd+8u,&callerRet);
    if(nOk && nativeRet>=g_gameBase)
        r.NativeRetRVA=nativeRet-g_gameBase;
    if(cOk && callerRet>=g_gameBase){
        r.CallerRead=1;
        r.CallerRetRVA=callerRet-g_gameBase;
        if(r.CallerRetRVA>=5u)r.CallerRVA=r.CallerRetRVA-5u;
    }
    const DWORD knownCalls[14]={0x10164u,0x1023Au,0xAD75Cu,0xAD778u,
        0xAD797u,0xAD7BCu,0xAEA69u,0xAEAD9u,0xAEB09u,0x13A20Bu,
        0x13DFB9u,0x1917ABu,0x19C2E1u,0x1A8E48u};
    for(DWORD i=0;i<14u;++i)
        if(r.CallerRVA==knownCalls[i]){r.KnownCaller=1;break;}
    r.Q0=Stage31Qpc();
    S41SnapObject(object,r.Pre,r.PreMask);
}
void __cdecl S42Leave(const DWORD* /* saved */)
{
    if(!g_s42Depth){InterlockedIncrement(&g_s42Mismatch);return;}
    const DWORD d=--g_s42Depth;
    if(d>=kS42MaxDepth)return;
    S42Rec& r=g_s42Pending[d];
    if(!r.Side)return;
    r.Q1=Stage31Qpc();
    S41SnapObject(r.Object,r.Post,r.PostMask);
    AcquireSRWLockExclusive(&g_s42Lock);
    r.Seq=++g_s42Seq;
    if(g_s42Count==kS42Capacity)InterlockedIncrement(&g_s42Dropped);
    else {
        g_s42Ring[g_s42Write]=r;
        g_s42Write=(g_s42Write+1u)%kS42Capacity;
        ++g_s42Count;
    }
    ReleaseSRWLockExclusive(&g_s42Lock);
    r.Side=0;
}
// Preserves x87/SSE, all GPRs and FLAGS around each C++ bookkeeping call.
// The original thiscall 1E9C70 has no stack args and returns with RET.
// The patched 1E9F62 site keeps its original CALL semantics.
__declspec(naked) void S42ActivationHook()
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
        call S42Enter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        call dword ptr [g_s42NativeActivation]
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
        call S42Leave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret
    }
}
bool InstallStage42Hooks()
{
    S42Log("STAGE42_ACTION_ACTIVATION_BUILD=2026-11-10-r1");
    S43Log("STAGE43_ACTIVATION_ORIGIN_BUILD=2026-11-10-r1");
    if(InterlockedCompareExchange(&g_s40Installed,0,0)!=1){
        S42Log("ERROR: Stage42 requires working Stage40/41; no patches.");return false;
    }
    if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
        S42Log("ERROR: Stage42 FXSAVE unsupported; no patches.");return false;
    }
    BYTE* site=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E9F62u));
    BYTE* native=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E9C70u));
    const BYTE expectedCall[]={0xE8,0x09,0xFD,0xFF,0xFF};
    const BYTE expectedNative[]={0x56,0x8B,0xF1,0x83,0x66,0x08,0xFE};
    auto valid=[&]()->bool{
        if(std::memcmp(site,expectedCall,sizeof(expectedCall))!=0||
           std::memcmp(native,expectedNative,sizeof(expectedNative))!=0)return false;
        std::int32_t rel=0;std::memcpy(&rel,site+1u,sizeof(rel));
        return site+5u+rel==native;
    };
    if(!valid()){
        S42Log("ERROR: Stage42 1E9F62->1E9C70 signature/target mismatch; no patches.");return false;
    }
    BYTE patch[5]{};
    if(!MakeRelativeCall(site,reinterpret_cast<const void*>(&S42ActivationHook),patch)){
        S42Log("ERROR: Stage42 rel32 out of range; no patches.");return false;
    }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S42Log("ERROR: Stage42 mutex unavailable; no patches.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
        CloseHandle(mutex);S42Log("ERROR: Stage42 mutex wait failed; no patches.");return false;
    }
    HANDLE suspended[512]{};
    const DWORD count=SuspendOtherProcessThreads(suspended,512);
    DWORD oldProtect=0;
    const bool protectedPage=VirtualProtect(site,5,PAGE_EXECUTE_READWRITE,&oldProtect)!=0;
    bool installed=false;
    if(protectedPage && valid()){
        g_s42NativeActivation=g_gameBase+0x1E9C70u;
        std::memcpy(site,patch,5);
        FlushInstructionCache(GetCurrentProcess(),site,5);
        InterlockedExchange(&g_s42Installed,1);
        installed=true;
    }
    if(protectedPage){DWORD old=0;VirtualProtect(site,5,oldProtect,&old);}
    ResumeThreads(suspended,count);
    ReleaseMutex(mutex);
    CloseHandle(mutex);
    S42Log(installed?
        "STAGE42_INSTALLED 1/1 callsite=1E9F62 native=1E9C70 original called once; only P1/P2 object snapshots.":
        "ERROR: Stage42 patch install failed; Stage40/41 remain active.");
    S43Log(installed?
        "STAGE43_ENABLED piggyback=Stage42 1/1; original stack return addresses only; no new hooks or game-state writes.":
        "ERROR: Stage43 requires Stage42 1/1; no Stage43 records.");
    return installed;
}
void FlushStage42()
{
    for(DWORD i=0;i<kS42Capacity;++i){
        S42Rec r{};
        AcquireSRWLockExclusive(&g_s42Lock);
        const bool have=g_s42Count!=0;
        if(have){
            r=g_s42Ring[g_s42Read];
            g_s42Read=(g_s42Read+1u)%kS42Capacity;
            --g_s42Count;
        }
        ReleaseSRWLockExclusive(&g_s42Lock);
        if(!have)break;
        DWORD changed=0;
        for(DWORD j=0;j<5u;++j)
            if((r.PreMask&r.PostMask&(1u<<j))&&r.Pre[j]!=r.Post[j])changed|=1u<<j;
        char line[900]{};
        const int n=sprintf_s(line,
            "STAGE42_ACTIVATION_WINDOW seq=%lu side=P%lu player=%08lX tid=%lu object=%08lX "
            "qpcBefore=%I64u qpcAfter=%I64u preMask=%01lX postMask=%01lX changedMask=%01lX "
            "o0=%08lX>%08lX o1=%08lX>%08lX o2=%08lX>%08lX "
            "o3=%08lX>%08lX o4=%08lX>%08lX bit0=%lu>%lu",
            static_cast<unsigned long>(r.Seq),static_cast<unsigned long>(r.Side),
            static_cast<unsigned long>(r.Player),static_cast<unsigned long>(r.Tid),
            static_cast<unsigned long>(r.Object),
            static_cast<unsigned long long>(r.Q0),static_cast<unsigned long long>(r.Q1),
            static_cast<unsigned long>(r.PreMask),static_cast<unsigned long>(r.PostMask),
            static_cast<unsigned long>(changed),
            static_cast<unsigned long>(r.Pre[0]),static_cast<unsigned long>(r.Post[0]),
            static_cast<unsigned long>(r.Pre[1]),static_cast<unsigned long>(r.Post[1]),
            static_cast<unsigned long>(r.Pre[2]),static_cast<unsigned long>(r.Post[2]),
            static_cast<unsigned long>(r.Pre[3]),static_cast<unsigned long>(r.Post[3]),
            static_cast<unsigned long>(r.Pre[4]),static_cast<unsigned long>(r.Post[4]),
            static_cast<unsigned long>(r.Pre[0]&1u),static_cast<unsigned long>(r.Post[0]&1u));
        if(n>0)S42Log(line);
        char origin[700]{};
        const int written=sprintf_s(origin,
            "STAGE43_ORIGIN_WINDOW stage42Seq=%lu side=P%lu player=%08lX tid=%lu object=%08lX "
            "qpcBefore=%I64u qpcAfter=%I64u nativeReturnRVA=%08lX "
            "callerReturnRVA=%08lX callerRVA=%08lX callerRead=%lu knownCaller=%lu "
            "pendingID=%08lX activeID=%08lX o30Before=%08lX o68Before=%08lX",
            static_cast<unsigned long>(r.Seq),static_cast<unsigned long>(r.Side),
            static_cast<unsigned long>(r.Player),static_cast<unsigned long>(r.Tid),
            static_cast<unsigned long>(r.Object),
            static_cast<unsigned long long>(r.Q0),static_cast<unsigned long long>(r.Q1),
            static_cast<unsigned long>(r.NativeRetRVA),
            static_cast<unsigned long>(r.CallerRetRVA),static_cast<unsigned long>(r.CallerRVA),
            static_cast<unsigned long>(r.CallerRead),static_cast<unsigned long>(r.KnownCaller),
            static_cast<unsigned long>(r.Pre[3]),static_cast<unsigned long>(r.Post[2]),
            static_cast<unsigned long>(r.Pre[1]),static_cast<unsigned long>(r.Pre[4]));
        if(written>0)S43Log(origin);
    }
    const LONG calls=InterlockedExchange(&g_s42AllCalls,0);
    const LONG other=InterlockedExchange(&g_s42OtherObjects,0);
    const LONG dropped=InterlockedExchange(&g_s42Dropped,0);
    const LONG overflow=InterlockedExchange(&g_s42Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s42Mismatch,0);
    if(calls || other || dropped || overflow || mismatch){
        char status[220]{};
        sprintf_s(status,"STAGE42_HEALTH calls=%ld otherObjects=%ld dropped=%ld overflow=%ld mismatch=%ld ring=2048 depth=8.",
            calls,other,dropped,overflow,mismatch);
        S42Log(status);
    }
}
