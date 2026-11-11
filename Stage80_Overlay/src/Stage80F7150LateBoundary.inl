// Stage80 SOURCE ONLY: one passive native CALL boundary inside original E89E0.
// Hook original E8A8E CALL F7150; E8A8D PUSH ESI supplies P, E8A93 ADD ESP,4.
// Original callee invoked ONCE with a copied P argument. Caller native stack
// restored before continuing original ADD ESP,4. No selector/gameplay edits.
// This boundary narrows the four-function Stage79 family to native F7150,
// which itself contains computed selector writes F7D22/F7D38 and zero F7D43.
// No DR/VEH ownership. Include AFTER Stage79SelectorFamilyBoundary.inl.
constexpr DWORD kS80CallRva=0x0E8A8Eu,kS80TargetRva=0x0F7150u,kS80RetRva=0x0E8A93u;
constexpr DWORD kS80Capacity=4096u,kS80MaxDepth=16u,kS80Fields=12u;
constexpr DWORD kS80Offsets[kS80Fields]={
    0x3F50u,0x3F54u,0x04u,0x016Eu,0x0170u,0x3B78u,
    0x3B7Cu,0x3A30u,0x3A34u,0x3ED0u,0x3ED4u,0x3B90u
};
struct S80Record {
    DWORD Seq=0,P=0,O=0,Side=0,Tid=0,ReturnVA=0,Arg=0,NativeEax=0;
    DWORD Pre[kS80Fields]{},Post[kS80Fields]{},PreMask=0,PostMask=0;
    ULONGLONG Q0=0,Q1=0;
};
S80Record g_s80Ring[kS80Capacity]{};
SRWLOCK g_s80Lock=SRWLOCK_INIT;
DWORD g_s80Read=0,g_s80Write=0,g_s80Count=0,g_s80Seq=0;
__declspec(thread) DWORD g_s80Depth=0;
__declspec(thread) S80Record g_s80Pending[kS80MaxDepth]{};
volatile LONG g_s80Installed=0,g_s80Calls=0,g_s80Filtered=0;
volatile LONG g_s80Dropped=0,g_s80Overflow=0,g_s80Mismatch=0,g_s80ReadFail=0;
DWORD g_s80NativeTarget=0;
void S80Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage80F7150LateTrace.log"))return;
    SYSTEMTIME t{};GetLocalTime(&t);
    char line[2100]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,message);
    if(n<=0)return;
    HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return;
    DWORD w=0;WriteFile(f,line,static_cast<DWORD>(n),&w,nullptr);CloseHandle(f);
}
void S80Snapshot(S80Record& r,bool after)
{
    DWORD* out=after?r.Post:r.Pre;
    DWORD& mask=after?r.PostMask:r.PreMask;
    for(DWORD j=0;j<kS80Fields;++j){
        if(SafeReadDword(r.P+kS80Offsets[j],&out[j]))mask|=1u<<j;
        else InterlockedIncrement(&g_s80ReadFail);
    }
    if(!after&&(mask&4u))r.O=out[2];
}
void __cdecl S80Enter(const DWORD* saved)
{
    InterlockedIncrement(&g_s80Calls);
    const DWORD d=g_s80Depth++;
    if(d>=kS80MaxDepth){InterlockedIncrement(&g_s80Overflow);return;}
    S80Record& r=g_s80Pending[d];r=S80Record{};
    // Original CALL E8A8E: [ESP] is E8A93; [ESP+4] is PUSH ESI
    // from E8A8D. PUSHFD/PUSHAD captured at saved[9]/[10].
    r.P=saved[1];r.Arg=saved[10];r.ReturnVA=saved[9];r.Tid=GetCurrentThreadId();
    r.Side=r.P==g_playerP1?1u:r.P==g_playerP2?2u:0u;
    if(!r.Side){InterlockedIncrement(&g_s80Filtered);return;}
    if(r.Arg!=r.P||r.ReturnVA!=g_gameBase+kS80RetRva){
        InterlockedIncrement(&g_s80Mismatch);r.Side=0;return;
    }
    S80Snapshot(r,false);r.Q0=Stage31Qpc();
}
void __cdecl S80Leave(const DWORD* saved)
{
    if(!g_s80Depth){InterlockedIncrement(&g_s80Mismatch);return;}
    const DWORD d=--g_s80Depth;
    if(d>=kS80MaxDepth)return;
    S80Record& r=g_s80Pending[d];
    if(!r.Side)return;
    r.Q1=Stage31Qpc();r.NativeEax=saved[7];
    if(r.P!=saved[1]||r.Tid!=GetCurrentThreadId()||
       r.Arg!=saved[10]||r.ReturnVA!=saved[9]){
        InterlockedIncrement(&g_s80Mismatch);r.Side=0;return;
    }
    S80Snapshot(r,true);
    if((r.PreMask&r.PostMask&4u)&&
       (r.O!=r.Pre[2]||r.O!=r.Post[2]))
        InterlockedIncrement(&g_s80Mismatch);
    AcquireSRWLockExclusive(&g_s80Lock);
    r.Seq=++g_s80Seq;
    if(g_s80Count>=kS80Capacity)InterlockedIncrement(&g_s80Dropped);
    else{
        g_s80Ring[g_s80Write]=r;
        g_s80Write=(g_s80Write+1u)%kS80Capacity;
        ++g_s80Count;
    }
    ReleaseSRWLockExclusive(&g_s80Lock);
    r.Side=0;
}
// Original caller E8A8D PUSH ESI before E8A8E CALL F7150.
// Hook entered by original CALL (not JMP), with return [ESP] and
// original P argument [ESP+4]. Replay that argument for original F7150,
// then remove ONLY the copied argument. Original arg remains for the
// original E8A93 ADD ESP,4. Original native function executes ONCE.
// Native return address observed inside F7150 changes to wrapper-local;
// that ABI difference requires Windows/game validation.
__declspec(naked) void S80HookLateF7150()
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
        call S80Enter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd

        push dword ptr [esp+4] // copy original P argument once
        call dword ptr [g_s80NativeTarget] // original F7150 ONCE
        add esp,4 // remove only duplicated P

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
        call S80Leave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret // original E8A93: ADD ESP,4 handles original PUSH ESI
    }
}
bool S80OriginalOK()
{
    const BYTE previous[1]={0x56};
    const BYTE call[5]={0xE8,0xBD,0xE6,0x00,0x00};
    const BYTE next[5]={0x83,0xC4,0x04,0x84,0xC0};
    const BYTE native[12]={0x55,0x8B,0xEC,0x81,0xEC,0xD4,0,0,0,0x53,0x56,0x57};
    const BYTE writer1[6]={0x89,0x97,0x50,0x3F,0,0};
    const BYTE writer2[6]={0x89,0x8F,0x50,0x3F,0,0};
    const BYTE writer0[10]={0xC7,0x87,0x50,0x3F,0,0,0,0,0,0};
    const BYTE* base=reinterpret_cast<const BYTE*>(static_cast<std::uintptr_t>(g_gameBase));
    return std::memcmp(base+kS80CallRva-1u,previous,1u)==0 &&
           std::memcmp(base+kS80CallRva,call,5u)==0 &&
           std::memcmp(base+kS80RetRva,next,5u)==0 &&
           std::memcmp(base+kS80TargetRva,native,12u)==0 &&
           std::memcmp(base+0xF7D22u,writer1,6u)==0 &&
           std::memcmp(base+0xF7D38u,writer2,6u)==0 &&
           std::memcmp(base+0xF7D43u,writer0,10u)==0;
}
bool InstallStage80Hooks()
{
    S80Log("STAGE80_BUILD=2026-11-11-r1 SOURCE_ONLY late E8A8E->F7150 once; DRwrites=0 extraGameWrites=0");
    const bool x86=sizeof(void*)==4u;
    const bool xmm=IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)!=0;
    const LONG s38=InterlockedCompareExchange(&g_stage38Installed,0,0);
    const LONG s76=InterlockedCompareExchange(&g_s76Installed,0,0);
    const LONG s78=InterlockedCompareExchange(&g_s78Installed,0,0);
    const LONG s79=InterlockedCompareExchange(&g_s79Installed,0,0);
    const LONG s59=InterlockedCompareExchange(&g_s59Started,0,0);
    const LONG s80=InterlockedCompareExchange(&g_s80Installed,0,0);
    const bool nativeOK=S80OriginalOK();
    char guard[410]{};
    sprintf_s(guard,"STAGE80_GUARD x86=%u xmm=%u stage38=%ld stage76=%ld stage78=%ld stage79=%ld stage59Started=%ld stage80=%ld nativeOK=%u gameBase=%08lX",
        x86?1u:0u,xmm?1u:0u,s38,s76,s78,s79,s59,s80,nativeOK?1u:0u,g_gameBase);
    S80Log(guard);
    if(!x86||!xmm||s38!=1||s76!=3||s78!=1||s79!=1||s59!=0||s80!=0||!nativeOK){
        S80Log("ERROR: Stage80 prerequisite/original signature mismatch; NO patch.");
        return false;
    }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S80Log("ERROR: Stage80 patch mutex unavailable.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
        CloseHandle(mutex);S80Log("ERROR: Stage80 mutex wait failed.");return false;
    }
    HANDLE peers[512]{};DWORD count=0;
    const bool suspended=S76SuspendPeers(peers,count);
    bool eipOK=suspended;
    if(eipOK){
        for(DWORD i=0;i<count;++i){
            CONTEXT c{};c.ContextFlags=CONTEXT_CONTROL;
            if(!GetThreadContext(peers[i],&c)||
               (c.Eip>=g_gameBase+kS80CallRva&&c.Eip<g_gameBase+kS80RetRva)){
                eipOK=false;break;
            }
        }
    }
    BYTE* site=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+kS80CallRva));
    BYTE patch[5]{};
    const bool callOK=MakeRelativeCall(site,reinterpret_cast<const void*>(&S80HookLateF7150),patch);
    DWORD oldProtect=0,unused=0;
    const bool writable=eipOK&&callOK&&S80OriginalOK()&&
        VirtualProtect(site,5u,PAGE_EXECUTE_READWRITE,&oldProtect)!=0;
    bool installed=false,restoreOK=true;
    if(writable&&S80OriginalOK()){
        g_s80NativeTarget=g_gameBase+kS80TargetRva;
        const BYTE original[5]={0xE8,0xBD,0xE6,0,0};
        std::memcpy(site,patch,5u);
        installed=FlushInstructionCache(GetCurrentProcess(),site,5u)!=0;
        if(!installed){
            std::memcpy(site,original,5u);
            FlushInstructionCache(GetCurrentProcess(),site,5u);
        }
        if(!VirtualProtect(site,5u,oldProtect,&unused)){
            restoreOK=false;
            if(installed){
                std::memcpy(site,original,5u);
                FlushInstructionCache(GetCurrentProcess(),site,5u);
            }
            installed=false;
        }
    }
    ResumeThreads(peers,count);ReleaseMutex(mutex);CloseHandle(mutex);
    if(!installed||!restoreOK){
        S80Log("ERROR: Stage80 installation refused/rolled back; use tested Stage79 DLL.");
        return false;
    }
    InterlockedExchange(&g_s80Installed,1);
    S80Log("STAGE80_INSTALLED 1/1 callRVA=0E8A8E targetRVA=0F7150 originalOnce=1 argumentCopied=1 DRwrites=0");
    return true;
}
void FlushStage80()
{
    for(DWORD i=0;i<kS80Capacity;++i){
        S80Record r{};
        AcquireSRWLockExclusive(&g_s80Lock);
        const bool has=g_s80Count!=0;
        if(has){
            r=g_s80Ring[g_s80Read];
            g_s80Read=(g_s80Read+1u)%kS80Capacity;--g_s80Count;
        }
        ReleaseSRWLockExclusive(&g_s80Lock);
        if(!has)break;
        DWORD changed=0;
        for(DWORD j=0;j<kS80Fields;++j)
            if((r.PreMask&r.PostMask&(1u<<j))&&r.Pre[j]!=r.Post[j])
                changed|=1u<<j;
        char msg[1700]{};
        int n=sprintf_s(msg,
            "STAGE80_LATE_F7150 seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu "
            "callRVA=0E8A8E targetRVA=0F7150 returnRVA=0E8A93 arg=%08lX "
            "qpcBefore=%I64u qpcAfter=%I64u nativeEax=%08lX "
            "preMask=%03lX postMask=%03lX changedMask=%03lX",
            r.Seq,r.Side,r.P,r.O,r.Tid,r.Arg,r.Q0,r.Q1,
            r.NativeEax,r.PreMask,r.PostMask,changed);
        if(n<=0)continue;
        for(DWORD j=0;j<kS80Fields;++j){
            const int added=sprintf_s(msg+n,sizeof(msg)-static_cast<size_t>(n),
                " f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
            if(added<=0)break;n+=added;
        }
        S80Log(msg);
    }
    const LONG calls=InterlockedExchange(&g_s80Calls,0);
    const LONG filtered=InterlockedExchange(&g_s80Filtered,0);
    const LONG dropped=InterlockedExchange(&g_s80Dropped,0);
    const LONG overflow=InterlockedExchange(&g_s80Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s80Mismatch,0);
    const LONG readFail=InterlockedExchange(&g_s80ReadFail,0);
    if(InterlockedCompareExchange(&g_s80Installed,0,0)==1){
        char health[330]{};
        sprintf_s(health,
            "STAGE80_HEALTH calls=%ld filtered=%ld dropped=%ld overflow=%ld mismatch=%ld readFail=%ld ring=4096 depth=16 DRwrites=0",
            calls,filtered,dropped,overflow,mismatch,readFail);
        S80Log(health);
    }
}
