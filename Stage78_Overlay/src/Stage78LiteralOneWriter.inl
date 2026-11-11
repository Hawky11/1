// Stage78 SOURCE ONLY: exact one-site read-only observer of original selector=1 store.
// Native AALib+155D29: C7 86 50 3F 00 00 01 00 00 00
// The original instruction is replayed ONCE, unchanged in effect, inside hook.
// No DR0..DR3 ownership or intentional extra gameplay writes. Untested until x86 build/run.
// Include from Stage38CallbackWindow.inl after Stage76AfterCallbackPhases.inl.
constexpr DWORD kS78Capacity=4096u,kS78DepthMax=16u,kS78Fields=6u;
constexpr DWORD kS78Site=0x155D29u,kS78Resume=0x155D33u;
constexpr DWORD kS78Offsets[kS78Fields]={0x3F50u,0x3F54u,0x04u,0x3ED0u,0x3ED4u,0x3A30u};
struct S78Record {
    DWORD Seq=0,P=0,O=0,Side=0,Tid=0,Frame=0,ReturnVA=0,StackTop=0;
    DWORD Pre[kS78Fields]{},Post[kS78Fields]{},PreMask=0,PostMask=0;
    ULONGLONG Q0=0,Q1=0;
};
__declspec(thread) DWORD g_s78Depth=0;
__declspec(thread) S78Record g_s78Pending[kS78DepthMax]{};
S78Record g_s78Ring[kS78Capacity]{};
SRWLOCK g_s78Lock=SRWLOCK_INIT;
DWORD g_s78Read=0,g_s78Write=0,g_s78Count=0,g_s78Seq=0;
volatile LONG g_s78Installed=0,g_s78Hits=0,g_s78Filtered=0;
volatile LONG g_s78Dropped=0,g_s78Overflow=0,g_s78Mismatch=0,g_s78ReadFail=0;
DWORD g_s78ResumeVA=0;

void S78Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage78LiteralOneWriterTrace.log"))return;
    SYSTEMTIME t{};GetLocalTime(&t);
    char line[1800]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,message);
    if(n<=0)return;
    HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return;
    DWORD w=0;WriteFile(f,line,static_cast<DWORD>(n),&w,nullptr);CloseHandle(f);
}
void S78Snapshot(S78Record& r,bool after)
{
    DWORD* out=after?r.Post:r.Pre;
    DWORD& mask=after?r.PostMask:r.PreMask;
    for(DWORD i=0;i<kS78Fields;++i){
        if(SafeReadDword(r.P+kS78Offsets[i],&out[i]))mask|=1u<<i;
        else InterlockedIncrement(&g_s78ReadFail);
    }
    if(!after&&(mask&4u))r.O=out[2];
}
void __cdecl S78BeforeStore(const DWORD* saved)
{
    InterlockedIncrement(&g_s78Hits);
    const DWORD player=saved[1]; // PUSHFD/PUSHAD: ESI[1]
    if(player!=g_playerP1 && player!=g_playerP2){
        InterlockedIncrement(&g_s78Filtered);return;
    }
    const DWORD depth=g_s78Depth++;
    if(depth>=kS78DepthMax){InterlockedIncrement(&g_s78Overflow);return;}
    S78Record& r=g_s78Pending[depth];r=S78Record{};
    r.P=player;r.Side=player==g_playerP1?1u:2u;r.Tid=GetCurrentThreadId();
    r.Frame=saved[2];r.StackTop=saved[9]; // original PUSH 21h at 155D27
    if(r.StackTop!=0x21u)InterlockedIncrement(&g_s78Mismatch);
    if(!SafeReadDword(r.Frame+4u,&r.ReturnVA))
        InterlockedIncrement(&g_s78ReadFail);
    S78Snapshot(r,false);
    r.Q0=Stage31Qpc();
}
void __cdecl S78AfterStore(const DWORD* saved)
{
    const DWORD player=saved[1];
    if(player!=g_playerP1 && player!=g_playerP2)return;
    if(!g_s78Depth){InterlockedIncrement(&g_s78Mismatch);return;}
    const DWORD depth=--g_s78Depth;
    if(depth>=kS78DepthMax)return;
    S78Record& r=g_s78Pending[depth];
    if(r.P!=player||r.Tid!=GetCurrentThreadId()||
       r.Frame!=saved[2]||r.StackTop!=saved[9]){
        InterlockedIncrement(&g_s78Mismatch);r.Side=0;return;
    }
    r.Q1=Stage31Qpc();S78Snapshot(r,true);
    if((r.PreMask&r.PostMask&4u)&&
       (r.O!=r.Pre[2]||r.O!=r.Post[2]))
        InterlockedIncrement(&g_s78Mismatch);
    if((r.PostMask&1u)&&r.Post[0]!=1u)
        InterlockedIncrement(&g_s78Mismatch);
    AcquireSRWLockExclusive(&g_s78Lock);
    r.Seq=++g_s78Seq;
    if(g_s78Count>=kS78Capacity)InterlockedIncrement(&g_s78Dropped);
    else {
        g_s78Ring[g_s78Write]=r;
        g_s78Write=(g_s78Write+1u)%kS78Capacity;++g_s78Count;
    }
    ReleaseSRWLockExclusive(&g_s78Lock);r.Side=0;
}
// Native enter via JMP, NOT CALL: no synthetic return address pushed.
// The ESI pointer, original event-code 21h stack argument, registers, flags,
// x87 and XMM are preserved. Original 10-byte MOV is executed exactly once.
__declspec(naked) void S78LiteralOneHook()
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
        call S78BeforeStore
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd

        mov dword ptr [esi+3F50h],1 // ORIGINAL 155D29 executes ONCE

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
        call S78AfterStore
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        jmp dword ptr [g_s78ResumeVA] // original 155D33: PUSH 0
    }
}
bool S78OriginalOK()
{
    const BYTE before[2]={0x6A,0x21};
    const BYTE siteBytes[10]={0xC7,0x86,0x50,0x3F,0,0,1,0,0,0};
    const BYTE after[2]={0x6A,0x00};
    const BYTE state3[10]={0xC7,0x86,0x54,0x3F,0,0,3,0,0,0};
    const BYTE from1560B9[5]={0xE8,0xB2,0xFA,0xFF,0xFF};
    const BYTE from1566F0[5]={0xE8,0x7B,0xF4,0xFF,0xFF};
    const BYTE* base=reinterpret_cast<const BYTE*>(static_cast<std::uintptr_t>(g_gameBase));
    return std::memcmp(base+kS78Site-2u,before,2u)==0 &&
           std::memcmp(base+kS78Site,siteBytes,10u)==0 &&
           std::memcmp(base+kS78Resume,after,2u)==0 &&
           std::memcmp(base+0x155D74u,state3,10u)==0 &&
           std::memcmp(base+0x1560B9u,from1560B9,5u)==0 &&
           std::memcmp(base+0x1566F0u,from1566F0,5u)==0;
}
bool InstallStage78Hooks()
{
    S78Log("STAGE78_BUILD=2026-11-11-r1 SOURCE_ONLY exact 155D29 literal-one MOV; extraGameWrites=0 DRwrites=0");
    const bool x86=sizeof(void*)==4u;
    const bool xmm=IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)!=0;
    const LONG s38=InterlockedCompareExchange(&g_stage38Installed,0,0);
    const LONG s76=InterlockedCompareExchange(&g_s76Installed,0,0);
    const LONG s70=InterlockedCompareExchange(&g_s70Installed,0,0);
    const LONG s59=InterlockedCompareExchange(&g_s59Started,0,0);
    const LONG s78=InterlockedCompareExchange(&g_s78Installed,0,0);
    const bool nativeOK=S78OriginalOK();
    char guard[350]{};
    sprintf_s(guard,
        "STAGE78_GUARD x86=%u xmm=%u stage38=%ld stage76=%ld stage70=%ld stage59Started=%ld stage78=%ld nativeOK=%u base=%08lX",
        x86?1u:0u,xmm?1u:0u,s38,s76,s70,s59,s78,nativeOK?1u:0u,g_gameBase);
    S78Log(guard);
    if(!x86||!xmm||s38!=1||s76!=3||s70!=1||s59!=0||s78!=0||!nativeOK){
        S78Log("ERROR: Stage78 prerequisites/original bytes failed; no patch.");
        return false;
    }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S78Log("ERROR: Stage78 install mutex unavailable.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
        CloseHandle(mutex);S78Log("ERROR: Stage78 mutex wait failed.");return false;
    }
    HANDLE peers[512]{};DWORD n=0;
    const bool suspended=S76SuspendPeers(peers,n); // suspends all peers; validates Stage76 sites
    bool eipOK=suspended;
    BYTE* site=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+kS78Site));
    if(eipOK){
        for(DWORD i=0;i<n;++i){
            CONTEXT context{};context.ContextFlags=CONTEXT_CONTROL;
            if(!GetThreadContext(peers[i],&context) ||
                (context.Eip>=g_gameBase+kS78Site&&
                 context.Eip<g_gameBase+kS78Resume)){
                eipOK=false;break;
            }
        }
    }
    const BYTE original[10]={0xC7,0x86,0x50,0x3F,0,0,1,0,0,0};
    BYTE detour[10]={0xE9,0,0,0,0,0x90,0x90,0x90,0x90,0x90};
    const DWORD displacement=static_cast<DWORD>(
        reinterpret_cast<std::uintptr_t>(&S78LiteralOneHook)-
        reinterpret_cast<std::uintptr_t>(site+5u));
    std::memcpy(detour+1u,&displacement,4u);
    DWORD oldProtect=0,ignored=0;
    const bool writable=eipOK&&S78OriginalOK() &&
        VirtualProtect(site,10u,PAGE_EXECUTE_READWRITE,&oldProtect)!=0;
    bool installed=false,restoredProtection=true;
    if(writable){
        g_s78ResumeVA=g_gameBase+kS78Resume;
        std::memcpy(site,detour,10u);
        installed=FlushInstructionCache(GetCurrentProcess(),site,10u)!=0;
        if(!installed){
            std::memcpy(site,original,10u);
            FlushInstructionCache(GetCurrentProcess(),site,10u);
        }
        if(!VirtualProtect(site,10u,oldProtect,&ignored)){
            restoredProtection=false;
            if(installed){
                std::memcpy(site,original,10u);
                FlushInstructionCache(GetCurrentProcess(),site,10u);
            }
            installed=false;
        }
    }
    ResumeThreads(peers,n);
    ReleaseMutex(mutex);CloseHandle(mutex);
    if(!installed||!restoredProtection){
        S78Log("ERROR: Stage78 installation refused/rolled back. Use tested Stage77 DLL if any anomaly.");
        return false;
    }
    InterlockedExchange(&g_s78Installed,1);
    S78Log("STAGE78_INSTALLED 1/1 RVA=155D29 bytes=10 replay=originalOnce resume=155D33 DRwrites=0");
    return true;
}
void FlushStage78()
{
    for(DWORD i=0;i<kS78Capacity;++i){
        S78Record r{};
        AcquireSRWLockExclusive(&g_s78Lock);
        const bool found=g_s78Count!=0;
        if(found){
            r=g_s78Ring[g_s78Read];
            g_s78Read=(g_s78Read+1u)%kS78Capacity;--g_s78Count;
        }
        ReleaseSRWLockExclusive(&g_s78Lock);
        if(!found)break;
        char msg[1250]{};
        const DWORD retRva=(r.ReturnVA>=g_gameBase)?r.ReturnVA-g_gameBase:0xFFFFFFFFu;
        int c=sprintf_s(msg,
            "STAGE78_LITERAL_ONE seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu frame=%08lX "
            "siteRVA=155D29 returnRVA=%06lX stackTop=%08lX qpcBefore=%I64u qpcAfter=%I64u "
            "preMask=%02lX postMask=%02lX",
            r.Seq,r.Side,r.P,r.O,r.Tid,r.Frame,retRva,r.StackTop,
            r.Q0,r.Q1,r.PreMask,r.PostMask);
        if(c<=0)continue;
        for(DWORD j=0;j<kS78Fields;++j){
            const int k=sprintf_s(msg+c,sizeof(msg)-static_cast<size_t>(c),
                " f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
            if(k<=0)break;c+=k;
        }
        S78Log(msg);
    }
    const LONG hits=InterlockedExchange(&g_s78Hits,0);
    const LONG filtered=InterlockedExchange(&g_s78Filtered,0);
    const LONG dropped=InterlockedExchange(&g_s78Dropped,0);
    const LONG overflow=InterlockedExchange(&g_s78Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s78Mismatch,0);
    const LONG readFail=InterlockedExchange(&g_s78ReadFail,0);
    if(hits||filtered||dropped||overflow||mismatch||readFail){
        char health[340]{};
        sprintf_s(health,
            "STAGE78_HEALTH hits=%ld filtered=%ld dropped=%ld overflow=%ld mismatch=%ld readFail=%ld ring=4096 depth=16 DRwrites=0",
            hits,filtered,dropped,overflow,mismatch,readFail);
        S78Log(health);
    }
}
