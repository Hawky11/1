// Stage59: diagnostic-only x86 DR0/DR1 write watchpoints at dynamic [player+4]+0x7C.
// Source only until Windows MSVC and game runtime are tested. Never writes AI state.
// Must be included from AIQueueTrace.cpp via the Stage38 adapter (after Stage58).
// DR0/DR1 are reserved only when Stage16 control/bit17 modes are BOTH disabled.
// Foreign debug-register ownership is never overwritten. DR2/DR3 are preserved.
constexpr DWORD kS59Capacity=8192u, kS59ThreadCapacity=256u;
constexpr DWORD kS59DrMask=0x00FF000Fu, kS59DrConfig=0x00DD0005u; // 2 x write-only DWORD
struct S59Row {
    volatile LONG State=0; // 0 free, -1 being written/read, 1 ready
    DWORD Seq=0,Side=0,Tid=0,Address=0,After=0,ShadowBefore=0;
    DWORD ReadOK=0,EipAfter=0,Eax=0,Ebx=0,Ecx=0,Edx=0,Esi=0,Edi=0,Eflags=0;
    ULONGLONG Qpc=0;
};
struct S59Tracked {
    HANDLE Handle=nullptr;
    DWORD Tid=0;
};
S59Row g_s59Rows[kS59Capacity]{};
S59Tracked g_s59Tracked[kS59ThreadCapacity]{};
volatile LONG g_s59Seq=0,g_s59Writes=0,g_s59Dropped=0,g_s59ReadFail=0;
volatile LONG g_s59Conflicts=0,g_s59InstallFail=0,g_s59TeardownFail=0;
volatile LONG g_s59Started=0,g_s59Ready=0,g_s59Capturing=0,g_s59Drift=0;
volatile LONG g_s59Shadow[2]{};
DWORD g_s59Addresses[2]{},g_s59Objects[2]{},g_s59GameImageSize=0;
PVOID g_s59Handler=nullptr;

void S59Log(const char* msg)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage59XWriterWatchTrace.log"))return;
    SYSTEMTIME t{}; GetLocalTime(&t);
    char line[1900]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,msg);
    if(n<=0)return;
    HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return;
    DWORD written=0;WriteFile(f,line,static_cast<DWORD>(n),&written,nullptr);
    CloseHandle(f);
}
// The exception is post-instruction for x86 data breakpoints: EIP_after is not
// the start of the writing instruction. Shadow-before is BEST EFFORT ONLY.
LONG CALLBACK S59ExceptionHandler(EXCEPTION_POINTERS* ep)
{
    if(!ep||!ep->ExceptionRecord||!ep->ContextRecord||
       ep->ExceptionRecord->ExceptionCode!=EXCEPTION_SINGLE_STEP||
       InterlockedCompareExchange(&g_s59Ready,0,0)==0)
        return EXCEPTION_CONTINUE_SEARCH;
    CONTEXT* c=ep->ContextRecord;
    const DWORD hits=c->Dr6&0x3u;
    if(!hits || c->Dr0!=g_s59Addresses[0] || c->Dr1!=g_s59Addresses[1] ||
       (c->Dr7&kS59DrMask)!=kS59DrConfig)
        return EXCEPTION_CONTINUE_SEARCH;
    // A second debug-register owner (e.g. AICommandTrace DR2/DR3) must see its
    // unhandled bits. Clear ONLY the DR0/DR1 bits proven to belong to Stage59.
    c->Dr6 &= ~hits;
    if(InterlockedCompareExchange(&g_s59Capturing,0,0)!=0) {
        for(DWORD i=0;i<2u;++i) {
            if(!(hits&(1u<<i)))continue;
            DWORD value=0;
            const BOOL readOK=SafeReadDword(g_s59Addresses[i],&value)?TRUE:FALSE;
            if(!readOK)InterlockedIncrement(&g_s59ReadFail);
            const DWORD prior=static_cast<DWORD>(InterlockedCompareExchange(&g_s59Shadow[i],0,0));
            if(readOK)InterlockedExchange(&g_s59Shadow[i],static_cast<LONG>(value));
            const DWORD seq=static_cast<DWORD>(InterlockedIncrement(&g_s59Seq));
            S59Row& row=g_s59Rows[(seq-1u)%kS59Capacity];
            if(InterlockedCompareExchange(&row.State,-1,0)!=0) {
                InterlockedIncrement(&g_s59Dropped);
                continue;
            }
            LARGE_INTEGER clock{};QueryPerformanceCounter(&clock);
            row.Seq=seq;row.Side=i+1u;row.Tid=GetCurrentThreadId();
            row.Address=g_s59Addresses[i];row.After=value;
            row.ShadowBefore=prior;row.ReadOK=readOK?1u:0u;
            row.EipAfter=c->Eip;row.Eax=c->Eax;row.Ebx=c->Ebx;
            row.Ecx=c->Ecx;row.Edx=c->Edx;row.Esi=c->Esi;
            row.Edi=c->Edi;row.Eflags=c->EFlags;
            row.Qpc=static_cast<ULONGLONG>(clock.QuadPart);
            InterlockedExchange(&row.State,1);
            InterlockedIncrement(&g_s59Writes);
        }
    }
    // Do not swallow simultaneous independent DR2/DR3 or BS/BD/BT hits.
    if((c->Dr6 & (0x0000000Cu|0x0000E000u))!=0)
        return EXCEPTION_CONTINUE_SEARCH;
    return EXCEPTION_CONTINUE_EXECUTION;
}
void S59Flush()
{
    for(DWORD i=0;i<kS59Capacity;++i) {
        S59Row& r=g_s59Rows[i];
        if(InterlockedCompareExchange(&r.State,-1,1)!=1)continue;
        const DWORD seq=r.Seq, side=r.Side, tid=r.Tid,addr=r.Address;
        const DWORD after=r.After,before=r.ShadowBefore,ok=r.ReadOK,eip=r.EipAfter;
        const DWORD eax=r.Eax,ebx=r.Ebx,ecx=r.Ecx,edx=r.Edx,esi=r.Esi,edi=r.Edi,ef=r.Eflags;
        const ULONGLONG qpc=r.Qpc;
        InterlockedExchange(&r.State,0);
        // EIP/RVA is POST-write. Code bytes are decoded offline from the
        // SHA-pinned original binary, not read in the VEH path.
        const DWORD rva=(eip>=g_gameBase && eip-g_gameBase<g_s59GameImageSize)?
            eip-g_gameBase:0xFFFFFFFFu;
        char msg[600]{};
        sprintf_s(msg,
            "STAGE59_X_WRITE seq=%lu side=P%lu tid=%lu watched=%08lX "
            "qpcAfter=%I64u EIP_after=%08lX RVA_after=%08lX "
            "oldShadow=%08lX newRaw=%08lX oldApprox=1 readOK=%lu "
            "EAX=%08lX EBX=%08lX ECX=%08lX EDX=%08lX "
            "ESI=%08lX EDI=%08lX EFLAGS=%08lX",
            seq,side,tid,addr,qpc,eip,rva,before,after,ok,
            eax,ebx,ecx,edx,esi,edi,ef);
        S59Log(msg);
    }
}
DWORD S59CountOwned()
{
    DWORD n=0;
    for(DWORD i=0;i<kS59ThreadCapacity;++i)
        if(g_s59Tracked[i].Handle)n++;
    return n;
}
// Remove only DR0/DR1 states still owned by us. If another owner took over,
// leave its registers untouched and retain the VEH to absorb stale own hits.
void S59UninstallOwned()
{
    for(DWORD i=0;i<kS59ThreadCapacity;++i) {
        S59Tracked& t=g_s59Tracked[i];
        if(!t.Handle)continue;
        if(WaitForSingleObject(t.Handle,0)==WAIT_TIMEOUT) {
            if(SuspendThread(t.Handle)!=static_cast<DWORD>(-1)) {
                CONTEXT c{};c.ContextFlags=CONTEXT_DEBUG_REGISTERS;
                if(GetThreadContext(t.Handle,&c) &&
                   c.Dr0==g_s59Addresses[0] && c.Dr1==g_s59Addresses[1] &&
                   (c.Dr7&kS59DrMask)==kS59DrConfig) {
                    c.Dr0=0;c.Dr1=0;c.Dr6&=~3u;
                    c.Dr7&=~kS59DrMask;
                    if(!SetThreadContext(t.Handle,&c))
                        InterlockedIncrement(&g_s59TeardownFail);
                } else InterlockedIncrement(&g_s59TeardownFail);
                ResumeThread(t.Handle);
            } else InterlockedIncrement(&g_s59TeardownFail);
        }
        CloseHandle(t.Handle);t=S59Tracked{};
    }
}
void S59ScanThreads()
{
    const DWORD myTid=GetCurrentThreadId();
    const DWORD pid=GetCurrentProcessId();
    for(DWORD i=0;i<kS59ThreadCapacity;++i) {
        S59Tracked& t=g_s59Tracked[i];
        if(t.Handle && WaitForSingleObject(t.Handle,0)==WAIT_OBJECT_0) {
            CloseHandle(t.Handle);t=S59Tracked{};
        }
    }
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    if(snap==INVALID_HANDLE_VALUE){InterlockedIncrement(&g_s59InstallFail);return;}
    THREADENTRY32 e{};e.dwSize=sizeof(e);
    if(Thread32First(snap,&e))do {
        if(e.th32OwnerProcessID!=pid || e.th32ThreadID==myTid)continue;
        bool tracked=false;
        DWORD slot=kS59ThreadCapacity;
        for(DWORD i=0;i<kS59ThreadCapacity;++i) {
            if(g_s59Tracked[i].Handle && g_s59Tracked[i].Tid==e.th32ThreadID) {
                tracked=true;break;
            }
            if(slot==kS59ThreadCapacity && !g_s59Tracked[i].Handle)slot=i;
        }
        if(tracked)continue;
        if(slot==kS59ThreadCapacity){InterlockedIncrement(&g_s59InstallFail);continue;}
        HANDLE h=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT|
            THREAD_SET_CONTEXT|THREAD_QUERY_INFORMATION|SYNCHRONIZE,
            FALSE,e.th32ThreadID);
        if(!h){InterlockedIncrement(&g_s59InstallFail);continue;}
        bool installed=false;
        if(SuspendThread(h)!=static_cast<DWORD>(-1)) {
            CONTEXT c{};c.ContextFlags=CONTEXT_DEBUG_REGISTERS;
            if(GetThreadContext(h,&c)) {
                const bool empty=(c.Dr0==0 && c.Dr1==0 &&
                                  (c.Dr7&kS59DrMask)==0);
                if(empty) {
                    c.Dr0=g_s59Addresses[0];c.Dr1=g_s59Addresses[1];
                    c.Dr6 &= ~3u;
                    c.Dr7=(c.Dr7&~kS59DrMask)|kS59DrConfig;
                    installed=SetThreadContext(h,&c)!=FALSE;
                    if(!installed)InterlockedIncrement(&g_s59InstallFail);
                } else InterlockedIncrement(&g_s59Conflicts);
            } else InterlockedIncrement(&g_s59InstallFail);
            ResumeThread(h);
        } else InterlockedIncrement(&g_s59InstallFail);
        if(installed) {
            g_s59Tracked[slot].Tid=e.th32ThreadID;
            g_s59Tracked[slot].Handle=h;
            char msg[250]{};
            sprintf_s(msg,"STAGE59_THREAD_INSTALLED tid=%lu DR0=%08lX DR1=%08lX DR7=%08lX",
                e.th32ThreadID,g_s59Addresses[0],g_s59Addresses[1],kS59DrConfig);
            S59Log(msg);
        } else CloseHandle(h);
        e.dwSize=sizeof(e);
    } while(Thread32Next(snap,&e));
    CloseHandle(snap);
}
DWORD WINAPI S59Worker(LPVOID)
{
    wchar_t ini[MAX_PATH+1]{};
    if(!GetSiblingPath(ini,MAX_PATH,L"ArcanaAI.ini")){
        S59Log("ERROR: Stage59 INI path unavailable; no debug registers touched.");
        return 0;
    }
    if(GetPrivateProfileIntW(L"AIResearch",L"EnableStage59XWriterWatch",1,ini)==0) {
        S59Log("STAGE59_DISABLED EnableStage59XWriterWatch=0; no debug registers touched.");
        return 0;
    }
    const UINT control=GetPrivateProfileIntW(L"AIResearch",L"EnableControlFieldTrace",0,ini);
    const UINT bit17=GetPrivateProfileIntW(L"AIResearch",L"EnablePlayerBit17WriteTrace",0,ini);
    if(control||bit17){
        char msg[240]{};
        sprintf_s(msg,"ERROR: Stage59 requires Stage16 DR0/DR1 modes OFF; control=%u bit17=%u; no registers touched.",control,bit17);
        S59Log(msg);
        return 0;
    }
    DWORD obj[2]{}, scratchX[2]{};
    bool valid=false;
    for(DWORD attempt=0;attempt<100u;++attempt) {
        if(g_playerP1 && g_playerP2 &&
           SafeReadDword(g_playerP1+4u,&obj[0]) &&
           SafeReadDword(g_playerP2+4u,&obj[1]) &&
           obj[0] && obj[1] && obj[0]!=obj[1] &&
           (obj[0]&3u)==0 && (obj[1]&3u)==0 &&
           SafeReadDword(obj[0]+0x7Cu,&scratchX[0]) &&
           SafeReadDword(obj[1]+0x7Cu,&scratchX[1])) {
            valid=true;break;
        }
        Sleep(50);
    }
    if(!valid){
        S59Log("ERROR: Stage59 timed out waiting for two readable player+4 object pointers; no registers touched.");
        return 0;
    }
    // The scratch reads validate the two X DWORDs. Address assignment follows.
    g_s59Objects[0]=obj[0];g_s59Objects[1]=obj[1];
    g_s59Addresses[0]=obj[0]+0x7Cu;g_s59Addresses[1]=obj[1]+0x7Cu;
    if((g_s59Addresses[0]&3u)||(g_s59Addresses[1]&3u)) {
        S59Log("ERROR: Stage59 X DWORD alignment failed; no registers touched.");
        return 0;
    }
    DWORD initial[2]{};
    if(!SafeReadDword(g_s59Addresses[0],&initial[0]) ||
       !SafeReadDword(g_s59Addresses[1],&initial[1])) {
        S59Log("ERROR: Stage59 initial X DWORD unreadable; no registers touched.");
        return 0;
    }
    InterlockedExchange(&g_s59Shadow[0],static_cast<LONG>(initial[0]));
    InterlockedExchange(&g_s59Shadow[1],static_cast<LONG>(initial[1]));
    const IMAGE_DOS_HEADER* dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(g_gameBase);
    const IMAGE_NT_HEADERS32* pe=reinterpret_cast<const IMAGE_NT_HEADERS32*>(
        g_gameBase+static_cast<DWORD>(dos->e_lfanew));
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE || pe->Signature!=IMAGE_NT_SIGNATURE ||
       pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        S59Log("ERROR: Stage59 PE32 image metadata unavailable; no registers touched.");
        return 0;
    }
    g_s59GameImageSize=pe->OptionalHeader.SizeOfImage;
    g_s59Handler=AddVectoredExceptionHandler(1,S59ExceptionHandler);
    if(!g_s59Handler){S59Log("ERROR: Stage59 AddVectoredExceptionHandler failed; no registers touched.");return 0;}
    InterlockedExchange(&g_s59Ready,1);
    InterlockedExchange(&g_s59Capturing,1);
    char msg[450]{};
    sprintf_s(msg,
        "STAGE59_WATCH_ARMED DR0/DR1 write-only 4-byte P1_object=%08lX P1_X=%08lX initial=%08lX "
        "P2_object=%08lX P2_X=%08lX initial=%08lX; DR2/DR3 preserved; oldShadow approximate",
        obj[0],g_s59Addresses[0],initial[0],obj[1],g_s59Addresses[1],initial[1]);
    S59Log(msg);
    DWORD period=0;
    for(;;) {
        DWORD nowObj[2]{};
        if(!SafeReadDword(g_playerP1+4u,&nowObj[0]) ||
           !SafeReadDword(g_playerP2+4u,&nowObj[1]) ||
           nowObj[0]!=g_s59Objects[0] || nowObj[1]!=g_s59Objects[1]) {
            InterlockedIncrement(&g_s59Drift);
            S59Log("STAGE59_OBJECT_DRIFT object pointers changed; stopping and disarming owned watchpoints.");
            break;
        }
        S59ScanThreads();
        S59Flush();
        if((++period%4u)==0u) {
            sprintf_s(msg,"STAGE59_HEALTH ownedThreads=%lu totalWrites=%ld totalSeq=%ld "
                "dropped=%ld readFail=%ld ownershipConflicts=%ld installFailures=%ld "
                "teardownFailures=%ld objectDrift=%ld",
                S59CountOwned(),InterlockedCompareExchange(&g_s59Writes,0,0),
                InterlockedCompareExchange(&g_s59Seq,0,0),
                InterlockedCompareExchange(&g_s59Dropped,0,0),
                InterlockedCompareExchange(&g_s59ReadFail,0,0),
                InterlockedCompareExchange(&g_s59Conflicts,0,0),
                InterlockedCompareExchange(&g_s59InstallFail,0,0),
                InterlockedCompareExchange(&g_s59TeardownFail,0,0),
                InterlockedCompareExchange(&g_s59Drift,0,0));
            S59Log(msg);
        }
        Sleep(250);
    }
    // Keep VEH registered for process lifetime; this prevents unhandled
    // traps if a thread races with cleanup. Do NOT remove foreign DR owners.
    InterlockedExchange(&g_s59Capturing,0);
    S59UninstallOwned();
    S59Flush();
    sprintf_s(msg,"STAGE59_STOPPED teardownFailures=%ld dropped=%ld",
        InterlockedCompareExchange(&g_s59TeardownFail,0,0),
        InterlockedCompareExchange(&g_s59Dropped,0,0));
    S59Log(msg);
    return 0;
}
bool StartStage59Watch()
{
    S59Log("STAGE59_X_WRITER_WATCH_BUILD=2026-11-10-r1; diagnostic only; runtime pending.");
    if(InterlockedCompareExchange(&g_s58Installed,0,0)!=3) {
        S59Log("ERROR: Stage59 requires Stage58 3/3; worker not started.");
        return false;
    }
    if(InterlockedCompareExchange(&g_s59Started,1,0)!=0)return true;
    HANDLE h=CreateThread(nullptr,0,S59Worker,nullptr,0,nullptr);
    if(!h){S59Log("ERROR: Stage59 worker creation failed.");return false;}
    CloseHandle(h);
    S59Log("STAGE59_WORKER_STARTED; watchpoints not yet verified, await STAGE59_WATCH_ARMED and THREAD_INSTALLED.");
    return true;
}
