// Stage58: passive original-once call-boundary observer at three conditional calls
// inside ACDF0's ACE11_POST -> ACFD6_PRE interval.
// Original SHA256: 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba
// ACF0F -> B86A0 (cdecl 2 args), ACF7A -> B86A0 (cdecl 2 args)
// ACF8A -> E9A80 (thiscall 1 stack arg, RET 4).
// Depends on Stage57's passive S57Snap; never intentionally modifies game state.
constexpr DWORD kS58Capacity=8192u,kS58DepthMax=8u,kS58Fields=13u;
struct S58Row {
    DWORD Seq=0,Side=0,Player=0,Object=0,Tid=0,CallerRva=0,CallerRead=0,ResultEax=0;
    ULONGLONG Q0=0,Q1=0;
    DWORD PreMask=0,PostMask=0,Pre[kS58Fields]{},Post[kS58Fields]{};
};
struct S58Pending { S58Row Rows[2]{}; };
volatile LONG g_s58Installed=0,g_s58Calls=0,g_s58Rows=0,g_s58Missing=0;
volatile LONG g_s58Dropped=0,g_s58Overflow=0,g_s58Mismatch=0,g_s58BadCaller=0;
DWORD g_s58NativeCheck=0,g_s58NativeLookup=0;
__declspec(thread) DWORD g_s58Depth=0;
__declspec(thread) S58Pending g_s58Pending[kS58DepthMax]{};
SRWLOCK g_s58Lock=SRWLOCK_INIT;
S58Row g_s58Ring[kS58Capacity]{};
DWORD g_s58Read=0,g_s58Write=0,g_s58Count=0,g_s58Seq=0;

void S58Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage58ACDF0ConditionalCallTrace.log"))return;
    SYSTEMTIME t{};GetLocalTime(&t);
    char line[2100]{};
    int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,message);
    if(n<=0)return;
    HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,
        OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return;
    DWORD written=0;WriteFile(f,line,static_cast<DWORD>(n),&written,nullptr);CloseHandle(f);
}
void __cdecl S58Enter(const DWORD* saved)
{
    InterlockedIncrement(&g_s58Calls);
    DWORD d=g_s58Depth++;
    if(d>=kS58DepthMax){InterlockedIncrement(&g_s58Overflow);return;}
    S58Pending& p=g_s58Pending[d];p=S58Pending{};
    DWORD ret=0;
    const DWORD entryEsp=saved[3]+4u; // PUSHFD + PUSHAD: original wrapper entry ESP
    const bool gotCaller=SafeReadDword(entryEsp,&ret);
    const DWORD caller=(gotCaller&&ret>=g_gameBase+5u)?ret-g_gameBase-5u:0u;
    if(!gotCaller||(caller!=0xACF0Fu&&caller!=0xACF7Au&&caller!=0xACF8Au)){
        InterlockedIncrement(&g_s58BadCaller);return;
    }
    const DWORD players[2]={g_playerP1,g_playerP2};
    DWORD objects[2]{};
    const bool readable[2]={SafeReadDword(players[0]+4u,&objects[0]),
                            SafeReadDword(players[1]+4u,&objects[1])};
    for(DWORD i=0;i<2u;++i){
        S58Row& r=p.Rows[i];
        if(!readable[i]||!objects[i]){InterlockedIncrement(&g_s58Missing);continue;}
        r.Side=i+1u;r.Player=players[i];r.Object=objects[i];
        r.Tid=GetCurrentThreadId();r.CallerRva=caller;r.CallerRead=1u;
        r.Q0=Stage31Qpc();S57Snap(r.Object,r.Pre,r.PreMask);
    }
}
void __cdecl S58Leave(const DWORD* saved)
{
    if(!g_s58Depth){InterlockedIncrement(&g_s58Mismatch);return;}
    DWORD d=--g_s58Depth;if(d>=kS58DepthMax)return;
    S58Pending& p=g_s58Pending[d];
    for(DWORD i=0;i<2u;++i){
        S58Row& r=p.Rows[i];if(!r.Side)continue;
        r.Q1=Stage31Qpc();S57Snap(r.Object,r.Post,r.PostMask);
        r.ResultEax=saved[7]; // PUSHAD captures original native's EAX
        AcquireSRWLockExclusive(&g_s58Lock);
        r.Seq=++g_s58Seq;
        if(g_s58Count==kS58Capacity)InterlockedIncrement(&g_s58Dropped);
        else {
            g_s58Ring[g_s58Write]=r;g_s58Write=(g_s58Write+1u)%kS58Capacity;
            ++g_s58Count;
        }
        ReleaseSRWLockExclusive(&g_s58Lock);
        InterlockedIncrement(&g_s58Rows);r.Side=0;
    }
}
// B86A0 is cdecl with TWO original stack arguments and caller-side ADD ESP,8.
// Copy both args onto the native call stack. LEA (not ADD) preserves EFLAGS
// while dropping the two COPIES; RET leaves original args for the original caller.
__declspec(naked) void S58CheckHook()
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
        call S58Enter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        push dword ptr [esp+8] // original second argument
        push dword ptr [esp+8] // original first argument, after first PUSH
        call dword ptr [g_s58NativeCheck]
        lea esp,[esp+8]       // drop copied args without altering original native EFLAGS
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
        call S58Leave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret                  // caller owns original ADD ESP,8
    }
}
// E9A80 thiscall expects original ECX plus ONE stack DWORD, returns with RET 4.
// Copy original argument; the native RET 4 removes the copy; wrapper RET 4
// removes the original argument while preserving original native EAX/EFLAGS/FPU.
__declspec(naked) void S58LookupHook()
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
        call S58Enter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        push dword ptr [esp+4]
        call dword ptr [g_s58NativeLookup]
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
        call S58Leave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret 4
    }
}
bool InstallStage58Hooks()
{
    S58Log("STAGE58_ACDF0_CONDITIONAL_BUILD=2026-11-10-r1");
    if(InterlockedCompareExchange(&g_s57Installed,0,0)!=3 ||
       InterlockedCompareExchange(&g_s56Installed,0,0)!=2 ||
       InterlockedCompareExchange(&g_s55Installed,0,0)!=3){
        S58Log("ERROR: Stage58 requires Stage57 3/3, Stage56 2/2, Stage55 3/3; no patches.");
        return false;
    }
    if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
        S58Log("ERROR: Stage58 requires FXSAVE; no patches.");return false;
    }
    const DWORD rvas[3]={0xACF0Fu,0xACF7Au,0xACF8Au};
    const DWORD targets[3]={0xB86A0u,0xB86A0u,0xE9A80u};
    const BYTE originals[3][5]={
        {0xE8,0x8C,0xB7,0x00,0x00},
        {0xE8,0x21,0xB7,0x00,0x00},
        {0xE8,0xF1,0xCA,0x03,0x00}
    };
    // Compare exact, instruction-aligned original PE bytes at BOTH callsites
    // and complete callee prologues before touching protection/page permissions.
    const BYTE checkPrologue[]={0x55,0x8B,0xEC,0x56,0x8B,0x75,0x08,0x83,0xFE,0xFF};
    const BYTE lookupPrologue[]={0x55,0x8B,0xEC,0x8B,0x45,0x08,0x56,0x8B,0x74,0x81,0x08};
    auto same=[&](DWORD rva,const BYTE* bytes,size_t count)->bool{
        for(size_t i=0;i<count;++i){
            DWORD actual=0;
            if(!SafeReadDword(g_gameBase+rva+static_cast<DWORD>(i),&actual) ||
               static_cast<BYTE>(actual)!=bytes[i])return false;
        }
        return true;
    };
    auto valid=[&]()->bool{
        bool ok=same(0xB86A0u,checkPrologue,sizeof(checkPrologue)) &&
                same(0xE9A80u,lookupPrologue,sizeof(lookupPrologue));
        for(DWORD i=0;i<3u;++i){
            DWORD rel=0;
            ok=ok&&same(rvas[i],originals[i],5u) &&
                SafeReadDword(g_gameBase+rvas[i]+1u,&rel) &&
                (rvas[i]+5u+rel==targets[i]);
        }
        if(!ok)S58Log("STAGE58_SIGNATURE_FAILURE; no new patches.");
        return ok;
    };
    if(!valid()){S58Log("ERROR: Stage58 signature validation failed; no patches.");return false;}
    BYTE* sites[3]{};
    BYTE patches[3][5]{};
    const void* wrappers[3]={
        reinterpret_cast<const void*>(&S58CheckHook),
        reinterpret_cast<const void*>(&S58CheckHook),
        reinterpret_cast<const void*>(&S58LookupHook)
    };
    for(DWORD i=0;i<3u;++i){
        sites[i]=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+rvas[i]));
        if(!MakeRelativeCall(sites[i],wrappers[i],patches[i])){
            S58Log("ERROR: Stage58 wrapper outside rel32 range; no patches.");return false;
        }
    }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S58Log("ERROR: Stage58 mutex failed; no patches.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
        CloseHandle(mutex);S58Log("ERROR: Stage58 mutex wait failed; no patches.");return false;
    }
    HANDLE suspended[512]{};
    const DWORD stopped=SuspendOtherProcessThreads(suspended,512);
    DWORD oldProtection[3]{};
    bool writable[3]{};
    bool protectionsOk=true;
    for(DWORD i=0;i<3u;++i){
        writable[i]=VirtualProtect(sites[i],5u,PAGE_EXECUTE_READWRITE,&oldProtection[i])!=0;
        if(!writable[i]){protectionsOk=false;break;}
    }
    bool installed=false;
    if(protectionsOk&&valid()){
        g_s58NativeCheck=g_gameBase+targets[0];
        g_s58NativeLookup=g_gameBase+targets[2];
        for(DWORD i=0;i<3u;++i){
            std::memcpy(sites[i],patches[i],5u);
            FlushInstructionCache(GetCurrentProcess(),sites[i],5u);
        }
        InterlockedExchange(&g_s58Installed,3);installed=true;
    }
    for(DWORD i=0;i<3u;++i)if(writable[i]){
        DWORD ignored=0;VirtualProtect(sites[i],5u,oldProtection[i],&ignored);
    }
    ResumeThreads(suspended,stopped);ReleaseMutex(mutex);CloseHandle(mutex);
    S58Log(installed?
        "STAGE58_INSTALLED 3/3 ACF0F->B86A0 ACF7A->B86A0 ACF8A->E9A80; native once per site; no gameplay writes.":
        "ERROR: Stage58 install failed; previous stages untouched.");
    return installed;
}
void FlushStage58()
{
    for(DWORD i=0;i<kS58Capacity;++i){
        S58Row r{};
        AcquireSRWLockExclusive(&g_s58Lock);
        const bool has=g_s58Count!=0;
        if(has){r=g_s58Ring[g_s58Read];g_s58Read=(g_s58Read+1u)%kS58Capacity;--g_s58Count;}
        ReleaseSRWLockExclusive(&g_s58Lock);
        if(!has)break;
        DWORD changed=0;
        for(DWORD j=0;j<kS58Fields;++j)
            if((r.PreMask&r.PostMask&(1u<<j))&&r.Pre[j]!=r.Post[j])changed|=1u<<j;
        char line[1900]{};
        int n=sprintf_s(line,
            "STAGE58_CONDITIONAL_WINDOW seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu "
            "callRVA=%08lX callerRead=%lu resultEax=%08lX "
            "qpcBefore=%I64u qpcAfter=%I64u preMask=%04lX postMask=%04lX changedMask=%04lX",
            r.Seq,r.Side,r.Player,r.Object,r.Tid,r.CallerRva,r.CallerRead,r.ResultEax,
            r.Q0,r.Q1,r.PreMask,r.PostMask,changed);
        if(n<=0)continue;
        for(DWORD j=0;j<kS58Fields;++j){
            int m=sprintf_s(line+n,sizeof(line)-static_cast<size_t>(n),
                " f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
            if(m<=0){n=0;break;}n+=m;
        }
        if(n>0)S58Log(line);
    }
    LONG calls=InterlockedExchange(&g_s58Calls,0),rows=InterlockedExchange(&g_s58Rows,0);
    LONG missing=InterlockedExchange(&g_s58Missing,0);
    LONG dropped=InterlockedExchange(&g_s58Dropped,0);
    LONG overflow=InterlockedExchange(&g_s58Overflow,0);
    LONG mismatch=InterlockedExchange(&g_s58Mismatch,0);
    LONG badCaller=InterlockedExchange(&g_s58BadCaller,0);
    if(calls||rows||missing||dropped||overflow||mismatch||badCaller){
        char line[280]{};
        sprintf_s(line,
            "STAGE58_HEALTH calls=%ld rows=%ld missingObjects=%ld dropped=%ld "
            "overflow=%ld mismatch=%ld badCaller=%ld ring=8192 depth=8.",
            calls,rows,missing,dropped,overflow,mismatch,badCaller);
        S58Log(line);
    }
}
