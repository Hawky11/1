// Stage79 SOURCE ONLY: passive boundary around the ORIGINAL EFFE8 CALL E89E0.
// Exact original: E8 F3 89 FF FF at RVA EFFE8, return RVA EFFED.
// One original call only. Direct native call tree reaches the four functions
// containing all 10 original register-derived P+3F50 writer sites.
// This is a FAMILY BOUNDARY, NOT direct attribution of any individual writer IP.
// No additional DR0..DR3, VEH or intentional gameplay writes.
// Include after Stage78LiteralOneWriter.inl and install before Stage59 worker.
constexpr DWORD kS79Site=0x0EFFE8u,kS79Target=0x0E89E0u,kS79Return=0x0EFFEDu;
constexpr DWORD kS79Capacity=8192u,kS79DepthMax=16u,kS79Fields=13u;
constexpr DWORD kS79Offsets[kS79Fields]={
    0x3F50u,0x3F54u,0x04u,0x016Eu,0x3B78u,0x3B7Cu,0x3A30u,
    0x3A34u,0x3ED0u,0x3ED4u,0x3F4Cu,0x3F7Cu,0x3B90u
};
// f3 DWORD at P+016E includes two gate bytes: low byte P+16E and
// byte 2 P+170. f11 = P+3F7C route gate, f12=P+3B90 event gate.
struct S79Record {
    DWORD Seq=0,Side=0,P=0,O=0,Tid=0,ReturnVA=0;
    DWORD Pre[kS79Fields]{},Post[kS79Fields]{};
    DWORD PreMask=0,PostMask=0,NativeEax=0;
    ULONGLONG Q0=0,Q1=0;
};
S79Record g_s79Ring[kS79Capacity]{};
SRWLOCK g_s79Lock=SRWLOCK_INIT;
DWORD g_s79Read=0,g_s79Write=0,g_s79Count=0,g_s79Seq=0;
__declspec(thread) DWORD g_s79Depth=0;
__declspec(thread) S79Record g_s79Pending[kS79DepthMax]{};
volatile LONG g_s79Installed=0,g_s79Calls=0,g_s79Filtered=0;
volatile LONG g_s79Dropped=0,g_s79Overflow=0,g_s79Mismatch=0,g_s79ReadFail=0;
DWORD g_s79OriginalTarget=0;
void S79Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage79SelectorFamilyTrace.log"))return;
    SYSTEMTIME t{};GetLocalTime(&t);
    char line[2050]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,message);
    if(n<=0)return;
    HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return;
    DWORD w=0;WriteFile(f,line,static_cast<DWORD>(n),&w,nullptr);
    CloseHandle(f);
}
void S79Snapshot(S79Record& r,bool after)
{
    DWORD* dst=after?r.Post:r.Pre;
    DWORD& mask=after?r.PostMask:r.PreMask;
    for(DWORD i=0;i<kS79Fields;++i){
        if(SafeReadDword(r.P+kS79Offsets[i],&dst[i]))mask|=1u<<i;
        else InterlockedIncrement(&g_s79ReadFail);
    }
    if(!after&&(mask&4u))r.O=dst[2];
}
void __cdecl S79Enter(const DWORD* saved)
{
    // Original EFFE8 is a CALL, so saved[9] is native return EFFED.
    // Original E89E0 uses native ESI as P; no explicit argument PUSH.
    InterlockedIncrement(&g_s79Calls);
    const DWORD depth=g_s79Depth++;
    if(depth>=kS79DepthMax){InterlockedIncrement(&g_s79Overflow);return;}
    S79Record& r=g_s79Pending[depth];r=S79Record{};
    r.P=saved[1];r.ReturnVA=saved[9];r.Tid=GetCurrentThreadId();
    if(r.P==g_playerP1)r.Side=1u;
    else if(r.P==g_playerP2)r.Side=2u;
    if(!r.Side){InterlockedIncrement(&g_s79Filtered);return;}
    if(r.ReturnVA!=g_gameBase+kS79Return){
        InterlockedIncrement(&g_s79Mismatch);r.Side=0;return;
    }
    S79Snapshot(r,false);
    r.Q0=Stage31Qpc();
}
void __cdecl S79Leave(const DWORD* saved)
{
    if(!g_s79Depth){InterlockedIncrement(&g_s79Mismatch);return;}
    const DWORD depth=--g_s79Depth;
    if(depth>=kS79DepthMax)return;
    S79Record& r=g_s79Pending[depth];
    if(!r.Side)return;
    r.Q1=Stage31Qpc();r.NativeEax=saved[7];
    if(r.P!=saved[1]||r.Tid!=GetCurrentThreadId()||
       r.ReturnVA!=saved[9]){
        InterlockedIncrement(&g_s79Mismatch);r.Side=0;return;
    }
    S79Snapshot(r,true);
    if((r.PreMask&r.PostMask&4u)&&
       (r.O!=r.Pre[2]||r.O!=r.Post[2]))
        InterlockedIncrement(&g_s79Mismatch);
    AcquireSRWLockExclusive(&g_s79Lock);
    r.Seq=++g_s79Seq;
    if(g_s79Count>=kS79Capacity)InterlockedIncrement(&g_s79Dropped);
    else{
        g_s79Ring[g_s79Write]=r;
        g_s79Write=(g_s79Write+1u)%kS79Capacity;
        ++g_s79Count;
    }
    ReleaseSRWLockExclusive(&g_s79Lock);r.Side=0;
}
// Inherited Stage76 native-CALL observer ABI model:
// PUSHFD/PUSHAD and FXSAVE before each passive C++ snapshot, then restore
// FLAGS/GPR/x87/SSE before original E89E0 and after its return.
// The original E89E0 CALL executes exactly ONCE with ESI=P.
// The callee sees a wrapper-local return address (known limitation), but
// the original EFFE8 caller sees its own original return via final RET.
__declspec(naked) void S79HookE89E0()
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
        call S79Enter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd

        call dword ptr [g_s79OriginalTarget] // ORIGINAL 0EFFE8 -> 0E89E0 ONCE

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
        call S79Leave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret // original 0EFFED: TEST AL,AL ; JE F0007
    }
}
bool S79OriginalOK()
{
    const BYTE before[2]={0x75,0x1F}; // EFFE6: JNE F0007
    const BYTE site[5]={0xE8,0xF3,0x89,0xFF,0xFF};
    const BYTE after[4]={0x84,0xC0,0x74,0x16};
    const BYTE target[6]={0x8B,0x86,0x7C,0x3B,0x00,0x00};
    const BYTE nestedF7150[5]={0xE8,0x48,0xE7,0x00,0x00}; // E8A03
    const BYTE nestedF6960[5]={0xE8,0x6D,0xDF,0x00,0x00}; // E89EE
    const BYTE nestedF70A0[5]={0xE8,0x73,0xE6,0x00,0x00}; // E8A28
    const BYTE* base=reinterpret_cast<const BYTE*>(static_cast<std::uintptr_t>(g_gameBase));
    return std::memcmp(base+kS79Site-2u,before,2u)==0 &&
           std::memcmp(base+kS79Site,site,5u)==0 &&
           std::memcmp(base+kS79Return,after,4u)==0 &&
           std::memcmp(base+kS79Target,target,6u)==0 &&
           std::memcmp(base+0xE8A03u,nestedF7150,5u)==0 &&
           std::memcmp(base+0xE89EEu,nestedF6960,5u)==0 &&
           std::memcmp(base+0xE8A28u,nestedF70A0,5u)==0;
}
bool InstallStage79Hooks()
{
    S79Log("STAGE79_BUILD=2026-11-11-r1 SOURCE_ONLY original EFFE8->E89E0 boundary; DRwrites=0 extraGameWrites=0");
    const bool x86=sizeof(void*)==4u;
    const bool xmm=IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)!=0;
    const LONG s38=InterlockedCompareExchange(&g_stage38Installed,0,0);
    const LONG s76=InterlockedCompareExchange(&g_s76Installed,0,0);
    const LONG s78=InterlockedCompareExchange(&g_s78Installed,0,0);
    const LONG s59=InterlockedCompareExchange(&g_s59Started,0,0);
    const LONG s79=InterlockedCompareExchange(&g_s79Installed,0,0);
    const bool nativeOK=S79OriginalOK();
    char guard[400]{};
    sprintf_s(guard,
        "STAGE79_GUARD x86=%u xmm=%u stage38=%ld stage76=%ld stage78=%ld stage59Started=%ld stage79=%ld nativeOK=%u gameBase=%08lX",
        x86?1u:0u,xmm?1u:0u,s38,s76,s78,s59,s79,nativeOK?1u:0u,g_gameBase);
    S79Log(guard);
    if(!x86||!xmm||s38!=1||s76!=3||s78!=1||s59!=0||s79!=0||!nativeOK){
        S79Log("ERROR: Stage79 original signature/prerequisites refused; no patch.");
        return false;
    }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S79Log("ERROR: Stage79 mutex unavailable.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
        CloseHandle(mutex);S79Log("ERROR: Stage79 mutex wait failed.");return false;
    }
    HANDLE peers[512]{};DWORD count=0;
    const bool suspended=S76SuspendPeers(peers,count);
    bool eipOK=suspended;
    if(eipOK){
        for(DWORD i=0;i<count;++i){
            CONTEXT context{};context.ContextFlags=CONTEXT_CONTROL;
            if(!GetThreadContext(peers[i],&context)||
               (context.Eip>=g_gameBase+kS79Site &&
                context.Eip<g_gameBase+kS79Site+5u)){
                eipOK=false;break;
            }
        }
    }
    BYTE* addr=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+kS79Site));
    BYTE patch[5]{};
    const bool callOK=MakeRelativeCall(addr,reinterpret_cast<const void*>(&S79HookE89E0),patch);
    DWORD oldProtect=0,unused=0;
    const bool writable=eipOK&&callOK&&S79OriginalOK()&&
        VirtualProtect(addr,5u,PAGE_EXECUTE_READWRITE,&oldProtect)!=0;
    bool installed=false,restoreOK=true;
    if(writable&&S79OriginalOK()){
        g_s79OriginalTarget=g_gameBase+kS79Target;
        const BYTE orig[5]={0xE8,0xF3,0x89,0xFF,0xFF};
        std::memcpy(addr,patch,5u);
        installed=FlushInstructionCache(GetCurrentProcess(),addr,5u)!=0;
        if(!installed){
            std::memcpy(addr,orig,5u);
            FlushInstructionCache(GetCurrentProcess(),addr,5u);
        }
        if(!VirtualProtect(addr,5u,oldProtect,&unused)){
            restoreOK=false;
            if(installed){
                std::memcpy(addr,orig,5u);
                FlushInstructionCache(GetCurrentProcess(),addr,5u);
            }
            installed=false;
        }
    }
    ResumeThreads(peers,count);
    ReleaseMutex(mutex);CloseHandle(mutex);
    if(!installed||!restoreOK){
        S79Log("ERROR: Stage79 install refused/rollback. Keep validated Stage78 DLL.");
        return false;
    }
    InterlockedExchange(&g_s79Installed,1);
    S79Log("STAGE79_INSTALLED 1/1 callRVA=0EFFE8 targetRVA=0E89E0 originalsOnce=1 DRwrites=0");
    return true;
}
void FlushStage79()
{
    for(DWORD i=0;i<kS79Capacity;++i){
        S79Record r{};
        AcquireSRWLockExclusive(&g_s79Lock);
        const bool found=g_s79Count!=0;
        if(found){
            r=g_s79Ring[g_s79Read];
            g_s79Read=(g_s79Read+1u)%kS79Capacity;--g_s79Count;
        }
        ReleaseSRWLockExclusive(&g_s79Lock);
        if(!found)break;
        DWORD changed=0;
        for(DWORD j=0;j<kS79Fields;++j)
            if((r.PreMask&r.PostMask&(1u<<j))&&r.Pre[j]!=r.Post[j])
                changed|=1u<<j;
        char msg[1900]{};
        int n=sprintf_s(msg,
            "STAGE79_COMMON_WINDOW seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu callRVA=0EFFE8 targetRVA=0E89E0 "
            "returnRVA=0EFFED qpcBefore=%I64u qpcAfter=%I64u nativeEax=%08lX preMask=%04lX postMask=%04lX changedMask=%04lX",
            r.Seq,r.Side,r.P,r.O,r.Tid,r.Q0,r.Q1,r.NativeEax,
            r.PreMask,r.PostMask,changed);
        if(n<=0)continue;
        for(DWORD j=0;j<kS79Fields;++j){
            const int added=sprintf_s(msg+n,sizeof(msg)-static_cast<size_t>(n),
                " f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
            if(added<=0)break;n+=added;
        }
        S79Log(msg);
    }
    const LONG calls=InterlockedExchange(&g_s79Calls,0);
    const LONG filtered=InterlockedExchange(&g_s79Filtered,0);
    const LONG dropped=InterlockedExchange(&g_s79Dropped,0);
    const LONG overflow=InterlockedExchange(&g_s79Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s79Mismatch,0);
    const LONG readFail=InterlockedExchange(&g_s79ReadFail,0);
    if(InterlockedCompareExchange(&g_s79Installed,0,0)==1){
        char health[330]{};
        sprintf_s(health,
            "STAGE79_HEALTH calls=%ld filtered=%ld dropped=%ld overflow=%ld mismatch=%ld readFail=%ld ring=8192 depth=16 DRwrites=0",
            calls,filtered,dropped,overflow,mismatch,readFail);
        S79Log(health);
    }
}
