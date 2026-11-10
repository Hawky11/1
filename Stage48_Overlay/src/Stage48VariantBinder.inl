// Stage48 passive attribution of two additional native action-definition
// binders invoked on O+6C variant/index change. Original CALLs only.
// Reuses proven Stage44 S44BindingHook and its ring/health/Stage45->46 nesting.
// No new worker, no game-state writes, no original-argument modifications.
volatile LONG g_s48Installed=0;
void S48Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage48VariantBindingTrace.log"))return;
    SYSTEMTIME t{};GetLocalTime(&t);
    char line[900]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,message);
    if(n<=0)return;
    HANDLE file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return;
    DWORD written=0;WriteFile(file,line,static_cast<DWORD>(n),&written,nullptr);
    CloseHandle(file);
}
void S48FlushRecord(const S44Rec& r)
{
    if(InterlockedCompareExchange(&g_s48Installed,0,0)!=1)return;
    const DWORD caller=r.S48CallerRva;
    if(caller!=0x1E9E2Du && caller!=0x1E8DA8u && caller!=0x1E908Fu)return;
    char line[800]{};
    const int n=sprintf_s(line,
        "STAGE48_VARIANT_BIND_WINDOW stage44Seq=%lu callRVA=%08lX callerRead=%lu "
        "side=P%lu player=%08lX object=%08lX tid=%lu qpcBefore=%I64u qpcAfter=%I64u "
        "activeID=%08lX>%08lX variant=%08lX>%08lX definition=%08lX>%08lX "
        "resolverCount=%lu resolvedQpc=%I64u resolverEAX=%08lX "
        "preMask=%02lX postMask=%02lX s46PreMask=%04lX s46PostMask=%04lX",
        r.Seq,r.S48CallerRva,r.S48CallerRead,
        r.Side,r.Player,r.Object,r.Tid,r.Q0,r.Q1,
        r.Pre[0],r.Post[0],r.Pre[4],r.Post[4],r.Pre[1],r.Post[1],
        r.S46Seen,r.S46Q0,r.S46Def,
        r.PreMask,r.PostMask,r.S46PreMask,r.S46PostMask);
    if(n>0)S48Log(line);
}
bool InstallStage48Hooks()
{
    S48Log("STAGE48_VARIANT_BIND_BUILD=2026-11-10-r1");
    if(InterlockedCompareExchange(&g_s44Installed,0,0)!=1 ||
       InterlockedCompareExchange(&g_s45Installed,0,0)!=1 ||
       InterlockedCompareExchange(&g_s47Installed,0,0)!=1){
        S48Log("ERROR: Stage48 requires working Stage44, Stage45/46 and Stage47; no patches.");
        return false;
    }
    if(!g_s44NativeBinding ||
       !IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
        S48Log("ERROR: Stage48 native binder/FXSAVE unavailable; no patches.");
        return false;
    }
    BYTE* native=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E8490u));
    BYTE* sites[2]={
        reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E8DA8u)),
        reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E908Fu))
    };
    // These are different 4KiB pages, so protect/restore each site separately.
    const BYTE calls[2][5]={
        {0xE8,0xE3,0xF6,0xFF,0xFF},
        {0xE8,0xFC,0xF3,0xFF,0xFF}
    };
    const BYTE nativePrologue[]={
        0x55,0x8B,0xEC,0x83,0xEC,0x0C,0x53,0x56,0x57,0x8B,0xF1
    };
    const BYTE condition0[]={0x39,0x7E,0x6C,0x74,0x07,0x8B,0xCE};
    const BYTE condition1[]={0x39,0x56,0x6C,0x74,0x07,0x8B,0xCE};
    auto valid=[&]()->bool{
        if(std::memcmp(native,nativePrologue,sizeof(nativePrologue)))return false;
        if(g_s44NativeBinding!=g_gameBase+0x1E8490u)return false;
        if(std::memcmp(sites[0]-7u,condition0,sizeof(condition0)) ||
           std::memcmp(sites[1]-7u,condition1,sizeof(condition1)))return false;
        for(DWORD i=0;i<2u;++i){
            if(std::memcmp(sites[i],calls[i],sizeof(calls[i])))return false;
            std::int32_t rel=0;std::memcpy(&rel,sites[i]+1u,sizeof(rel));
            if(sites[i]+5u+rel!=native)return false;
        }
        return true;
    };
    if(!valid()){
        S48Log("ERROR: Stage48 exact variant-change guards/CALL/native target signature mismatch; no patches.");
        return false;
    }
    BYTE patches[2][5]{};
    for(DWORD i=0;i<2u;++i){
        if(!MakeRelativeCall(sites[i],reinterpret_cast<const void*>(&S44BindingHook),patches[i])){
            S48Log("ERROR: Stage48 redirect rel32 out of range; no patches.");
            return false;
        }
    }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S48Log("ERROR: Stage48 patch mutex unavailable; no patches.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0 && wait!=WAIT_ABANDONED){
        CloseHandle(mutex);
        S48Log("ERROR: Stage48 patch mutex wait failed; no patches.");
        return false;
    }
    HANDLE suspended[512]{};
    const DWORD stopped=SuspendOtherProcessThreads(suspended,512);
    DWORD old[2]{};
    const bool p0=VirtualProtect(sites[0],5,PAGE_EXECUTE_READWRITE,&old[0])!=0;
    const bool p1=p0 && VirtualProtect(sites[1],5,PAGE_EXECUTE_READWRITE,&old[1])!=0;
    bool installed=false;
    if(p0 && p1 && valid()){
        // All-or-none: both pages writable, all originals verified once more.
        for(DWORD i=0;i<2u;++i){
            std::memcpy(sites[i],patches[i],5);
            FlushInstructionCache(GetCurrentProcess(),sites[i],5);
        }
        InterlockedExchange(&g_s48Installed,1);
        installed=true;
    }
    if(p1){DWORD ignored=0;VirtualProtect(sites[1],5,old[1],&ignored);}
    if(p0){DWORD ignored=0;VirtualProtect(sites[0],5,old[0],&ignored);}
    ResumeThreads(suspended,stopped);
    ReleaseMutex(mutex);CloseHandle(mutex);
    S48Log(installed?
        "STAGE48_INSTALLED 2/2 callsites=1E8DA8,1E908F native=1E8490 wrapper=Stage44; native called once; shared Stage44 ring/health.":
        "ERROR: Stage48 hook install failed; earlier Stage44/45/46/47 unaffected.");
    return installed;
}
