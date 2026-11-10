// Stage45: native active-ID -> action-definition metadata binding, read-only.
// Original CALL: 1E84B3 -> 1DF2D0, inside native definition binding.
// Included after Stage44ActionDefinition.inl; its TLS window supplies correlation.
// New hook redirects only one original CALL; original native executes once.
constexpr DWORD kS45Capacity=2048u,kS45DepthMax=8u;
struct S45Rec {
    DWORD Seq=0,Side=0,Player=0,Object=0,Tid=0;
    ULONGLONG Q0=0,Q1=0;
    ULONGLONG BindingQ0=0;
    DWORD Mask=0,Context=0,Id=0,Variant=0,Gate=0,IndexWord=0,TableBase=0;
    DWORD Expected=0,Result=0,Fallback=0,Path=0;
    // Mask bits: args 1/2, gate 4, index 8, table base 16.
    // Path: 0 unknown, 1 zero gate fallback, 2 nonpositive index fallback, 3 table.

};
volatile LONG g_s45Installed=0,g_s45AllCalls=0,g_s45OtherObjects=0;
volatile LONG g_s45Dropped=0,g_s45Overflow=0,g_s45Mismatch=0;
DWORD g_s45NativeBinding=0;
__declspec(thread) DWORD g_s45Depth=0;
__declspec(thread) S45Rec g_s45Pending[kS45DepthMax]{};
SRWLOCK g_s45Lock=SRWLOCK_INIT;
S45Rec g_s45Ring[kS45Capacity]{};
DWORD g_s45Read=0,g_s45Write=0,g_s45Count=0,g_s45Seq=0;

void S45Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage45DefinitionResolverTrace.log"))return;
    SYSTEMTIME now{};GetLocalTime(&now);
    char line[1800]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,message);
    if(n<=0)return;
    HANDLE file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return;
    DWORD written=0;WriteFile(file,line,static_cast<DWORD>(n),&written,nullptr);
    CloseHandle(file);
}
void __cdecl S45Enter(const DWORD* saved)
{
    InterlockedIncrement(&g_s45AllCalls);
    const DWORD d=g_s45Depth++;
    if(d>=kS45DepthMax){InterlockedIncrement(&g_s45Overflow);return;}
    S45Rec& r=g_s45Pending[d];r=S45Rec{};
    // Only resolver calls inside a captured Stage44 binding window.
    if(!g_s44Depth || g_s44Depth>kS44DepthMax){
        InterlockedIncrement(&g_s45OtherObjects);return;
    }
    const S44Rec& parent=g_s44Pending[g_s44Depth-1u];
    // ESI is O in original 1E8490; do not label unrelated nested calls.
    if(!parent.Side || saved[1]!=parent.Object){
        InterlockedIncrement(&g_s45OtherObjects);return;
    }
    r.Side=parent.Side;r.Player=parent.Player;r.Object=parent.Object;
    r.BindingQ0=parent.Q0;r.Tid=GetCurrentThreadId();r.Q0=Stage31Qpc();
    r.Context=saved[6]; // PUSHAD ECX, native lookup context
    r.Fallback=g_gameBase+0x0DEE49E0u;
    // PUSHAD saved ESP points at PUSHFD word; original entry ESP is +4.
    const DWORD entryEsp=saved[3]+4u;
    if(SafeReadDword(entryEsp+4u,&r.Id))r.Mask|=1u;
    if(SafeReadDword(entryEsp+8u,&r.Variant))r.Mask|=2u;
    DWORD value=0;
    if((r.Mask&1u) && SafeReadDword(r.Context+r.Id+0xB0000u,&value)){
        r.Gate=value&0xFFu;r.Mask|=4u;
        if(!r.Gate){r.Path=1u;r.Expected=r.Fallback;}
        else if((r.Mask&2u) && SafeReadDword(r.Context+((r.Id<<7u)+r.Variant)*2u,&value)){
            r.IndexWord=value&0xFFFFu;r.Mask|=8u;
            const LONG index=static_cast<LONG>(static_cast<short>(r.IndexWord));
            if(index<=0){r.Path=2u;r.Expected=r.Fallback;}
            else if(SafeReadDword(r.Context+0xB0B0Cu,&r.TableBase)){
                r.Mask|=16u;r.Path=3u;
                r.Expected=r.TableBase+static_cast<DWORD>(index-1)*0x5Cu;
            }
        }
    }
}
void __cdecl S45Leave(const DWORD* saved)
{
    if(!g_s45Depth){InterlockedIncrement(&g_s45Mismatch);return;}
    const DWORD d=--g_s45Depth;
    if(d>=kS45DepthMax)return;
    S45Rec& r=g_s45Pending[d];
    if(!r.Side)return;
    r.Q1=Stage31Qpc();
    r.Result=saved[7]; // preserved native EAX
    AcquireSRWLockExclusive(&g_s45Lock);
    r.Seq=++g_s45Seq;
    if(g_s45Count==kS45Capacity)InterlockedIncrement(&g_s45Dropped);
    else {
        g_s45Ring[g_s45Write]=r;
        g_s45Write=(g_s45Write+1u)%kS45Capacity;
        ++g_s45Count;
    }
    ReleaseSRWLockExclusive(&g_s45Lock);
    r.Side=0;
}
// Two-argument thiscall wrapper: preserve GPR/EFLAGS/x87/SSE; replay once.
// RET 8 cleans caller arguments after native RET 8 cleans copied arguments.
__declspec(naked) void S45BindingHook()
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
        call S45Enter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        // Replay two original arguments; native RET 8 removes only the copies.
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        call dword ptr [g_s45NativeBinding]
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
        call S45Leave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret 8
    }
}
bool InstallStage45Hooks()
{
    S45Log("STAGE45_DEFINITION_RESOLVER_BUILD=2026-11-10-r1");
    if(InterlockedCompareExchange(&g_s44Installed,0,0)!=1){
        S45Log("ERROR: Stage45 requires installed Stage44; no patches.");return false;
    }
    if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
        S45Log("ERROR: Stage45 FXSAVE unavailable; no patches.");return false;
    }
    BYTE* site=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E84B3u));
    BYTE* native=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1DF2D0u));
    const BYTE origCall[]={0xE8,0x18,0x6E,0xFF,0xFF};
    const BYTE origNative[]={0x55,0x8B,0xEC,0x8B,0x45,0x08,0x80,0xBC,0x08,0x00,0x00,0x0B,0x00,0x00,0x76,0x1A,0xC1,0xE0,0x07,0x03,0x45,0x0C,0x0F,0xBF,0x04,0x41,0x48,0x78,0x0D,0x6B,0xC0,0x5C,0x03,0x81,0x0C,0x0B,0x0B,0x00,0x5D,0xC2,0x08,0x00,0xB8};
    auto valid=[&]()->bool{
        if(std::memcmp(site,origCall,sizeof(origCall)) ||
           std::memcmp(native,origNative,sizeof(origNative)))return false;
        std::int32_t rel=0;std::memcpy(&rel,site+1u,sizeof(rel));
        DWORD fallback=0;
        if(!SafeReadDword(g_gameBase+0x1DF2FBu,&fallback))return false;
        const BYTE tail[]={0x5D,0xC2,0x08,0x00};
        return site+5u+rel==native && native[0x2A]==0xB8 &&
            fallback==g_gameBase+0x0DEE49E0u &&
            !std::memcmp(native+0x2Fu,tail,sizeof(tail));
    };
    if(!valid()){
        S45Log("ERROR: Stage45 native callsite/prologue mismatch; no patches.");return false;
    }
    BYTE patch[5]{};
    if(!MakeRelativeCall(site,reinterpret_cast<const void*>(&S45BindingHook),patch)){
        S45Log("ERROR: Stage45 rel32 out of range; no patches.");return false;
    }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S45Log("ERROR: Stage45 mutex unavailable; no patches.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0 && wait!=WAIT_ABANDONED){
        CloseHandle(mutex);
        S45Log("ERROR: Stage45 mutex wait failed; no patches.");return false;
    }
    HANDLE suspended[512]{};
    const DWORD stopped=SuspendOtherProcessThreads(suspended,512);
    DWORD oldProtection=0;
    const bool writable=VirtualProtect(site,5,PAGE_EXECUTE_READWRITE,&oldProtection)!=0;
    bool installed=false;
    if(writable && valid()){
        g_s45NativeBinding=g_gameBase+0x1DF2D0u;
        std::memcpy(site,patch,5);
        FlushInstructionCache(GetCurrentProcess(),site,5);
        InterlockedExchange(&g_s45Installed,1);
        installed=true;
    }
    if(writable){DWORD ignored=0;VirtualProtect(site,5,oldProtection,&ignored);}
    ResumeThreads(suspended,stopped);
    ReleaseMutex(mutex);CloseHandle(mutex);
    S45Log(installed?
        "STAGE45_INSTALLED 1/1 callsite=1E84B3 native=1DF2D0; original called once; passive nested Stage44 resolver observations.":
        "ERROR: Stage45 install failed; existing Stage44/43/42/41/40 unaffected.");
    return installed;
}
void FlushStage45()
{
    for(DWORD i=0;i<kS45Capacity;++i){
        S45Rec r{};
        AcquireSRWLockExclusive(&g_s45Lock);
        const bool have=g_s45Count!=0;
        if(have){
            r=g_s45Ring[g_s45Read];
            g_s45Read=(g_s45Read+1u)%kS45Capacity;
            --g_s45Count;
        }
        ReleaseSRWLockExclusive(&g_s45Lock);
        if(!have)break;
        char line[1000]{};
        const int n=sprintf_s(line,
            "STAGE45_RESOLVER_WINDOW seq=%lu side=P%lu player=%08lX tid=%lu object=%08lX "
            "bindingQpcBefore=%I64u qpcBefore=%I64u qpcAfter=%I64u "
            "readMask=%02lX context=%08lX id=%08lX variant=%08lX gate=%02lX "
            "indexWord=%04lX tableBase=%08lX path=%lu expected=%08lX "
            "nativeEAX=%08lX fallback=%08lX resultIsFallback=%lu predictionMatches=%lu",
            r.Seq,r.Side,r.Player,r.Tid,r.Object,
            r.BindingQ0,r.Q0,r.Q1,r.Mask,r.Context,r.Id,r.Variant,r.Gate,
            r.IndexWord,r.TableBase,r.Path,r.Expected,r.Result,r.Fallback,
            static_cast<DWORD>(r.Result==r.Fallback),
            static_cast<DWORD>(r.Path!=0u && r.Result==r.Expected));
        if(n>0)S45Log(line);
    }
    const LONG calls=InterlockedExchange(&g_s45AllCalls,0);
    const LONG unrelated=InterlockedExchange(&g_s45OtherObjects,0);
    const LONG dropped=InterlockedExchange(&g_s45Dropped,0);
    const LONG overflow=InterlockedExchange(&g_s45Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s45Mismatch,0);
    if(calls||unrelated||dropped||overflow||mismatch){
        char line[240]{};
        sprintf_s(line,
            "STAGE45_HEALTH calls=%ld otherObjects=%ld dropped=%ld overflow=%ld mismatch=%ld ring=2048 depth=8.",
            calls,unrelated,dropped,overflow,mismatch);
        S45Log(line);
    }
}

