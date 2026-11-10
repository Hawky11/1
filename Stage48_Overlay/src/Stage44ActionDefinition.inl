// Stage44: native active-ID -> action-definition metadata binding, read-only.
// Original CALL: 1E9E2D -> 1E8490, within 1E9C70 activation.
// Included inside Stage38CallbackWindow.inl after Stage42ActivationHook.inl.
// New hook redirects only one original CALL; original native executes once.
constexpr DWORD kS44Capacity=2048u,kS44DepthMax=8u;
struct S44Rec {
    DWORD Seq=0,Side=0,Player=0,Object=0,Tid=0;
    ULONGLONG Q0=0,Q1=0;
    DWORD PreMask=0,PostMask=0,Pre[8]{},Post[8]{};
    DWORD MetaPreMask=0,MetaPostMask=0,MetaPre[3]{},MetaPost[3]{};
    DWORD S46Seen=0,S46Def=0,S46PreMask=0,S46PostMask=0,S46MetaMask=0;
    DWORD S46Pre[13]{},S46Post[13]{},S46Meta[4]{};
    ULONGLONG S46Q0=0;
    DWORD S48CallerRva=0,S48CallerRead=0;

};
// Implemented after Stage47 source inclusion by Stage48VariantBinder.inl.
void S48FlushRecord(const S44Rec& r);
volatile LONG g_s44Installed=0,g_s44AllCalls=0,g_s44OtherObjects=0;
volatile LONG g_s44Dropped=0,g_s44Overflow=0,g_s44Mismatch=0;
DWORD g_s44NativeBinding=0;
__declspec(thread) DWORD g_s44Depth=0;
__declspec(thread) S44Rec g_s44Pending[kS44DepthMax]{};
SRWLOCK g_s44Lock=SRWLOCK_INIT;
S44Rec g_s44Ring[kS44Capacity]{};
DWORD g_s44Read=0,g_s44Write=0,g_s44Count=0,g_s44Seq=0;

void S44Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage44ActionDefinitionTrace.log"))return;
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
void S46Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage46DefinitionMotionTrace.log"))return;
    SYSTEMTIME now{};GetLocalTime(&now);
    char line[2400]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,message);
    if(n<=0)return;
    HANDLE file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return;
    DWORD written=0;WriteFile(file,line,static_cast<DWORD>(n),&written,nullptr);
    CloseHandle(file);
}
// Stage46 uses existing Stage44 ring and Stage45 callback; no new hook.
void S46Snap(DWORD object,DWORD* fields,DWORD& mask)
{
    const DWORD offsets[13]={0x60u,0x6Cu,0x70u,0x08u,0x24u,0x7Cu,0x80u,
        0x88u,0x8Cu,0x90u,0x94u,0x150u,0x154u};
    mask=0;
    for(DWORD i=0;i<13u;++i)
        if(SafeReadDword(object+offsets[i],&fields[i]))mask|=1u<<i;
}
void S46Resolved(DWORD object,DWORD definition)
{
    if(!g_s44Depth || g_s44Depth>kS44DepthMax)return;
    S44Rec& r=g_s44Pending[g_s44Depth-1u];
    if(!r.Side || r.Object!=object)return;
    ++r.S46Seen;
    // Keep first resolution; count repeats rather than silently overwrite it.
    if(r.S46Seen!=1u)return;
    r.S46Q0=Stage31Qpc();r.S46Def=definition;
    S46Snap(object,r.S46Pre,r.S46PreMask);
    const DWORD offsets[4]={0x20u,0x30u,0x34u,0x38u};
    for(DWORD i=0;i<4u;++i)
        if(definition && SafeReadDword(definition+offsets[i],&r.S46Meta[i]))
            r.S46MetaMask|=1u<<i;
}
void S46FlushRecord(const S44Rec& r)
{
    char line[1800]{};
    int n=sprintf_s(line,
        "STAGE46_MOTION_WINDOW stage44Seq=%lu side=P%lu player=%08lX tid=%lu object=%08lX "
        "bindingQpcBefore=%I64u resolvedQpc=%I64u qpcAfter=%I64u resolverCount=%lu "
        "definition=%08lX definitionAfter=%08lX preMask=%04lX postMask=%04lX metaMask=%01lX "
        "meta20=%08lX meta30=%08lX meta34=%08lX meta38=%08lX",
        r.Seq,r.Side,r.Player,r.Tid,r.Object,r.Q0,r.S46Q0,r.Q1,r.S46Seen,
        r.S46Def,r.Post[1],r.S46PreMask,r.S46PostMask,r.S46MetaMask,
        r.S46Meta[0],r.S46Meta[1],r.S46Meta[2],r.S46Meta[3]);
    if(n<=0)return;
    for(DWORD i=0;i<13u;++i){
        const int added=sprintf_s(line+n,sizeof(line)-static_cast<size_t>(n),
            " f%lu=%08lX>%08lX",i,r.S46Pre[i],r.S46Post[i]);
        if(added<=0)return;
        n+=added;
    }
    S46Log(line);
}
void S44SnapObject(DWORD object,DWORD* fields,DWORD& mask)
{
    // f0 active ID O+60; f1 definition pointer O+34;
    // f2 O+26C includes ready-byte at +26D; f3 O+178 definition type;
    // f4 variant O+6C; f5 flags O+08; f6 flags O+0C; f7 class O+48.
    const DWORD offs[8]={0x60u,0x34u,0x26Cu,0x178u,0x6Cu,0x08u,0x0Cu,0x48u};
    mask=0;
    if(!object)return;
    for(DWORD i=0;i<8u;++i)
        if(SafeReadDword(object+offs[i],&fields[i]))mask|=1u<<i;
}
void S44SnapDefinition(DWORD definition,DWORD* fields,DWORD& mask)
{
    // Native definition structure: flags +0x20, type at +0x2C,
    // and a timing/parameter field at +0x48.
    const DWORD offs[3]={0x20u,0x2Cu,0x48u};
    mask=0;
    if(!definition)return;
    for(DWORD i=0;i<3u;++i)
        if(SafeReadDword(definition+offs[i],&fields[i]))mask|=1u<<i;
}
void __cdecl S44Enter(const DWORD* saved)
{
    InterlockedIncrement(&g_s44AllCalls);
    const DWORD d=g_s44Depth++;
    if(d>=kS44DepthMax){InterlockedIncrement(&g_s44Overflow);return;}
    S44Rec& r=g_s44Pending[d];r=S44Rec{};
    const DWORD object=saved[6]; // PUSHAD ECX, 1E8490 thiscall O
    DWORD o1=0,o2=0;
    const bool v1=SafeReadDword(g_playerP1+4u,&o1);
    const bool v2=SafeReadDword(g_playerP2+4u,&o2);
    if(v1 && object==o1){r.Side=1;r.Player=g_playerP1;}
    else if(v2 && object==o2){r.Side=2;r.Player=g_playerP2;}
    else {InterlockedIncrement(&g_s44OtherObjects);return;}
    r.Object=object;r.Tid=GetCurrentThreadId();
    // PUSHAD saved ESP is the pushed FLAGS location; original CALL return
    // address is at [saved[3] + 4], without changing native registers.
    DWORD nativeReturn=0;
    if(SafeReadDword(saved[3]+4u,&nativeReturn) && nativeReturn>=g_gameBase+5u){
        r.S48CallerRead=1u;
        r.S48CallerRva=nativeReturn-g_gameBase-5u;
    }
    r.Q0=Stage31Qpc();
    S44SnapObject(object,r.Pre,r.PreMask);
    if(r.PreMask&(1u<<1u))
        S44SnapDefinition(r.Pre[1],r.MetaPre,r.MetaPreMask);
}
void __cdecl S44Leave(const DWORD* /* saved */)
{
    if(!g_s44Depth){InterlockedIncrement(&g_s44Mismatch);return;}
    const DWORD d=--g_s44Depth;
    if(d>=kS44DepthMax)return;
    S44Rec& r=g_s44Pending[d];
    if(!r.Side)return;
    r.Q1=Stage31Qpc();
    S46Snap(r.Object,r.S46Post,r.S46PostMask);
    S44SnapObject(r.Object,r.Post,r.PostMask);
    if(r.PostMask&(1u<<1u))
        S44SnapDefinition(r.Post[1],r.MetaPost,r.MetaPostMask);
    AcquireSRWLockExclusive(&g_s44Lock);
    r.Seq=++g_s44Seq;
    if(g_s44Count==kS44Capacity)InterlockedIncrement(&g_s44Dropped);
    else {
        g_s44Ring[g_s44Write]=r;
        g_s44Write=(g_s44Write+1u)%kS44Capacity;
        ++g_s44Count;
    }
    ReleaseSRWLockExclusive(&g_s44Lock);
    r.Side=0;
}
// Same validated Stage42 wrapper ABI: PUSHFD/PUSHAD, FXSAVE/FXRSTOR,
// original thiscall no stack args, invoke exactly once, preserve native EAX.
__declspec(naked) void S44BindingHook()
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
        call S44Enter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        call dword ptr [g_s44NativeBinding]
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
        call S44Leave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret
    }
}
bool InstallStage44Hooks()
{
    S44Log("STAGE44_ACTION_DEFINITION_BUILD=2026-11-10-r1");
    if(InterlockedCompareExchange(&g_s42Installed,0,0)!=1){
        S44Log("ERROR: Stage44 requires installed Stage42/43; no patches.");return false;
    }
    if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
        S44Log("ERROR: Stage44 FXSAVE unavailable; no patches.");return false;
    }
    BYTE* site=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E9E2Du));
    BYTE* native=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E8490u));
    const BYTE origCall[]={0xE8,0x5E,0xE6,0xFF,0xFF};
    const BYTE origNative[]={0x55,0x8B,0xEC,0x83,0xEC,0x0C,0x53,0x56,0x57,0x8B,0xF1};
    auto valid=[&]()->bool{
        if(std::memcmp(site,origCall,sizeof(origCall)) ||
           std::memcmp(native,origNative,sizeof(origNative)))return false;
        std::int32_t rel=0;std::memcpy(&rel,site+1u,sizeof(rel));
        return site+5u+rel==native;
    };
    if(!valid()){
        S44Log("ERROR: Stage44 native callsite/prologue mismatch; no patches.");return false;
    }
    BYTE patch[5]{};
    if(!MakeRelativeCall(site,reinterpret_cast<const void*>(&S44BindingHook),patch)){
        S44Log("ERROR: Stage44 rel32 out of range; no patches.");return false;
    }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S44Log("ERROR: Stage44 mutex unavailable; no patches.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0 && wait!=WAIT_ABANDONED){
        CloseHandle(mutex);
        S44Log("ERROR: Stage44 mutex wait failed; no patches.");return false;
    }
    HANDLE suspended[512]{};
    const DWORD stopped=SuspendOtherProcessThreads(suspended,512);
    DWORD oldProtection=0;
    const bool writable=VirtualProtect(site,5,PAGE_EXECUTE_READWRITE,&oldProtection)!=0;
    bool installed=false;
    if(writable && valid()){
        g_s44NativeBinding=g_gameBase+0x1E8490u;
        std::memcpy(site,patch,5);
        FlushInstructionCache(GetCurrentProcess(),site,5);
        InterlockedExchange(&g_s44Installed,1);
        installed=true;
    }
    if(writable){DWORD ignored=0;VirtualProtect(site,5,oldProtection,&ignored);}
    ResumeThreads(suspended,stopped);
    ReleaseMutex(mutex);CloseHandle(mutex);
    S44Log(installed?
        "STAGE44_INSTALLED 1/1 callsite=1E9E2D native=1E8490; original called once; passive P1/P2 action-definition observations.":
        "ERROR: Stage44 install failed; existing Stage43/42/41/40 unaffected.");
    return installed;
}
void FlushStage44()
{
    for(DWORD i=0;i<kS44Capacity;++i){
        S44Rec r{};
        AcquireSRWLockExclusive(&g_s44Lock);
        const bool have=g_s44Count!=0;
        if(have){
            r=g_s44Ring[g_s44Read];
            g_s44Read=(g_s44Read+1u)%kS44Capacity;
            --g_s44Count;
        }
        ReleaseSRWLockExclusive(&g_s44Lock);
        if(!have)break;
        DWORD changed=0;
        for(DWORD j=0;j<8u;++j)
            if((r.PreMask&r.PostMask&(1u<<j)) && r.Pre[j]!=r.Post[j])changed|=1u<<j;
        char line[1250]{};
        const int n=sprintf_s(line,
            "STAGE44_BINDING_WINDOW seq=%lu side=P%lu player=%08lX tid=%lu object=%08lX "
            "qpcBefore=%I64u qpcAfter=%I64u preMask=%02lX postMask=%02lX changedMask=%02lX "
            "metaPreMask=%01lX metaPostMask=%01lX "
            "f0=%08lX>%08lX f1=%08lX>%08lX f2=%08lX>%08lX f3=%08lX>%08lX "
            "f4=%08lX>%08lX f5=%08lX>%08lX f6=%08lX>%08lX f7=%08lX>%08lX "
            "m0=%08lX>%08lX m1=%08lX>%08lX m2=%08lX>%08lX",
            static_cast<unsigned long>(r.Seq),static_cast<unsigned long>(r.Side),
            static_cast<unsigned long>(r.Player),static_cast<unsigned long>(r.Tid),
            static_cast<unsigned long>(r.Object),
            static_cast<unsigned long long>(r.Q0),static_cast<unsigned long long>(r.Q1),
            static_cast<unsigned long>(r.PreMask),static_cast<unsigned long>(r.PostMask),
            static_cast<unsigned long>(changed),
            static_cast<unsigned long>(r.MetaPreMask),static_cast<unsigned long>(r.MetaPostMask),
            static_cast<unsigned long>(r.Pre[0]),static_cast<unsigned long>(r.Post[0]),
            static_cast<unsigned long>(r.Pre[1]),static_cast<unsigned long>(r.Post[1]),
            static_cast<unsigned long>(r.Pre[2]),static_cast<unsigned long>(r.Post[2]),
            static_cast<unsigned long>(r.Pre[3]),static_cast<unsigned long>(r.Post[3]),
            static_cast<unsigned long>(r.Pre[4]),static_cast<unsigned long>(r.Post[4]),
            static_cast<unsigned long>(r.Pre[5]),static_cast<unsigned long>(r.Post[5]),
            static_cast<unsigned long>(r.Pre[6]),static_cast<unsigned long>(r.Post[6]),
            static_cast<unsigned long>(r.Pre[7]),static_cast<unsigned long>(r.Post[7]),
            static_cast<unsigned long>(r.MetaPre[0]),static_cast<unsigned long>(r.MetaPost[0]),
            static_cast<unsigned long>(r.MetaPre[1]),static_cast<unsigned long>(r.MetaPost[1]),
            static_cast<unsigned long>(r.MetaPre[2]),static_cast<unsigned long>(r.MetaPost[2]));
        if(n>0)S44Log(line);
        S46FlushRecord(r);
        S48FlushRecord(r);
    }
    const LONG calls=InterlockedExchange(&g_s44AllCalls,0);
    const LONG unrelated=InterlockedExchange(&g_s44OtherObjects,0);
    const LONG dropped=InterlockedExchange(&g_s44Dropped,0);
    const LONG overflow=InterlockedExchange(&g_s44Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s44Mismatch,0);
    if(calls||unrelated||dropped||overflow||mismatch){
        char line[240]{};
        sprintf_s(line,
            "STAGE44_HEALTH calls=%ld otherObjects=%ld dropped=%ld overflow=%ld mismatch=%ld ring=2048 depth=8.",
            calls,unrelated,dropped,overflow,mismatch);
        S44Log(line);
    }
}

