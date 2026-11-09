// Stage36 source-only: passive native predicate and candidate-return observers.
// Included by Stage36_Overlay/src/AIQueueTrace.cpp inside its anonymous namespace.
// No native state modifications: original cdecl helpers invoked once with exact args.
constexpr DWORD kStage36Input1FCall = 0xEB718u;
constexpr DWORD kStage36Input1ECall = 0xEB7D9u;
constexpr DWORD kStage36InputTarget = 0xF58E0u;
constexpr DWORD kStage36CandidateCall = 0xF79E4u;
constexpr DWORD kStage36CandidateTarget = 0xF4360u;
constexpr DWORD kStage36RingSize = 2048u;
constexpr DWORD kStage36MaxDepth = 8u;
constexpr DWORD kStage36Fields = 10u;
enum { Stage36Input1F = 1, Stage36Input1E = 2, Stage36Candidate = 3 };
volatile LONG g_stage36Installed = 0;
DWORD g_stage36InputTarget = 0, g_stage36CandidateTarget = 0;
volatile LONG g_stage36Dropped = 0, g_stage36DepthOverflow = 0;
struct Stage36Record
{
    DWORD Seq, Kind, Player, Side, Tid, TickBefore, TickAfter, CallReturn;
    ULONGLONG QpcBefore, QpcAfter;
    DWORD Args[5], ArgMask, EaxReturn;
    DWORD PreMask, PostMask, Pre[kStage36Fields], Post[kStage36Fields];
    DWORD OutPre[2], OutPost[2], OutMaskPre, OutMaskPost;
};
__declspec(thread) DWORD g_stage36Depth = 0;
__declspec(thread) Stage36Record g_stage36Scopes[kStage36MaxDepth] = {};
SRWLOCK g_stage36Lock = SRWLOCK_INIT;
Stage36Record g_stage36Ring[kStage36RingSize]{};
DWORD g_stage36Read = 0, g_stage36Write = 0, g_stage36Count = 0, g_stage36Sequence = 0;

void AppendStage36Log(const char* message)
{
    wchar_t path[MAX_PATH + 1]{};
    if (!GetSiblingPath(path, MAX_PATH, L"ArcanaAI_Stage36ControlToCommandTrace.log"))
        return;
    SYSTEMTIME now{};
    GetLocalTime(&now);
    char line[2800]{};
    int size = sprintf_s(line, "[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond,
        message ? message : "(empty)");
    if (size <= 0) return;
    HANDLE file = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                              nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return;
    DWORD written = 0;
    WriteFile(file, line, static_cast<DWORD>(size), &written, nullptr);
    CloseHandle(file);
}
void Stage36Snapshot(DWORD p, DWORD* fields, DWORD& mask)
{
    if (p != g_playerP1 && p != g_playerP2) return;
    const DWORD offsets[8] = {
        0x3A08u, 0x3A0Cu, 0x3A10u, 0x3A6Cu, 0x3B78u,
        0x3B70u, 0x3B84u, 0x3F64u
    };
    for (DWORD i = 0; i < 8u; ++i)
        if (SafeReadDword(p + offsets[i], &fields[i])) mask |= (1u << i);
    DWORD o = 0;
    if (SafeReadDword(p + 4u, &o) && o != 0u)
    {
        if (SafeReadDword(o + 8u, &fields[8])) mask |= (1u << 8);
        if (SafeReadDword(o + 0x24u, &fields[9])) mask |= (1u << 9);
    }
}
void Stage36Enqueue(Stage36Record& r)
{
    AcquireSRWLockExclusive(&g_stage36Lock);
    r.Seq = ++g_stage36Sequence; // dropped records create detectable seq holes
    if (g_stage36Count == kStage36RingSize)
        InterlockedIncrement(&g_stage36Dropped);
    else
    {
        g_stage36Ring[g_stage36Write] = r;
        g_stage36Write = (g_stage36Write + 1u) % kStage36RingSize;
        ++g_stage36Count;
    }
    ReleaseSRWLockExclusive(&g_stage36Lock);
}
void __cdecl Stage36Begin(const DWORD* context, DWORD family)
{
    const DWORD depth = g_stage36Depth++;
    if (depth >= kStage36MaxDepth)
    {
        InterlockedIncrement(&g_stage36DepthOverflow);
        return;
    }
    Stage36Record& r = g_stage36Scopes[depth];
    r = Stage36Record{};
    if (context == nullptr) return;
    // PUSHFD/PUSHAD layout: context[9] native CALL-return, context[10..] args.
    const DWORD callReturn = context[9];
    DWORD kind = 0;
    if (family == 1u)
    {
        if (callReturn == g_gameBase + kStage36Input1FCall + 5u) kind = Stage36Input1F;
        if (callReturn == g_gameBase + kStage36Input1ECall + 5u) kind = Stage36Input1E;
    }
    else if (family == 3u && callReturn == g_gameBase + kStage36CandidateCall + 5u)
        kind = Stage36Candidate;
    if (kind == 0u) return;
    const DWORD argCount = kind == Stage36Candidate ? 5u : 2u;
    for (DWORD i = 0; i < argCount; ++i)
        if (SafeReadDword(static_cast<DWORD>(
                reinterpret_cast<std::uintptr_t>(context + 10 + i)), &r.Args[i]))
            r.ArgMask |= 1u << i;
    const DWORD expectedMask = kind == Stage36Candidate ? 0x1Fu : 0x3u;
    if (r.ArgMask != expectedMask) return;
    DWORD player = 0;
    if (kind == Stage36Candidate)
        player = r.Args[0];
    else if (r.Args[0] >= 0x3A08u)
        player = r.Args[0] - 0x3A08u;
    if (player != g_playerP1 && player != g_playerP2) return;
    if ((kind == Stage36Input1E && r.Args[1] != 0x1Eu) ||
        (kind == Stage36Input1F && r.Args[1] != 0x1Fu))
        return;
    r.Kind = kind;
    r.Player = player;
    r.Side = player == g_playerP1 ? 1u : 2u;
    r.Tid = GetCurrentThreadId();
    r.CallReturn = callReturn;
    r.TickBefore = GetTickCount();
    r.QpcBefore = Stage31Qpc();
    Stage36Snapshot(player, r.Pre, r.PreMask);
    if (kind == Stage36Candidate)
    {
        if (SafeReadDword(r.Args[3], &r.OutPre[0])) r.OutMaskPre |= 1u;
        if (SafeReadDword(r.Args[4], &r.OutPre[1])) r.OutMaskPre |= 2u;
    }
}
void __cdecl Stage36End(const DWORD* context, DWORD family)
{
    if (g_stage36Depth == 0u)
    {
        InterlockedIncrement(&g_stage36DepthOverflow);
        return;
    }
    const DWORD depth = --g_stage36Depth;
    if (depth >= kStage36MaxDepth) return;
    Stage36Record& r = g_stage36Scopes[depth];
    if (r.Kind == 0u) return;
    if (context == nullptr || (family == 1u && r.Kind == Stage36Candidate) ||
        (family == 3u && r.Kind != Stage36Candidate))
    {
        InterlockedIncrement(&g_stage36DepthOverflow);
        return;
    }
    r.EaxReturn = context[7]; // PUSHAD captured native EAX before diagnostics
    r.TickAfter = GetTickCount();
    r.QpcAfter = Stage31Qpc();
    Stage36Snapshot(r.Player, r.Post, r.PostMask);
    if (r.Kind == Stage36Candidate)
    {
        if (SafeReadDword(r.Args[3], &r.OutPost[0])) r.OutMaskPost |= 1u;
        if (SafeReadDword(r.Args[4], &r.OutPost[1])) r.OutMaskPost |= 2u;
    }
    Stage36Enqueue(r);
    r.Kind = 0u;
}
// Two-argument cdecl wrapper. Five-byte CALL sites EB718 and EB7D9
// both invoke the same F58E0 function; caller-return discriminates input index.
__declspec(naked) void Stage36InputHook()
{
    __asm {
        pushfd
        pushad
        mov ebx, esp
        sub esp, 210h
        lea edi, [esp+0Fh]
        and edi, 0FFFFFFF0h
        fxsave [edi]
        fninit
        cld
        push 1
        push ebx
        call Stage36Begin
        add esp, 8
        fxrstor [edi]
        mov esp, ebx
        popad
        popfd

        push dword ptr [esp+8]  // original index, arg2
        push dword ptr [esp+8]  // original pointer, arg1 (shifted)
        call dword ptr [g_stage36InputTarget]
        lea esp, [esp+8]       // preserve native EFLAGS

        pushfd
        pushad
        mov ebx, esp
        sub esp, 210h
        lea edi, [esp+0Fh]
        and edi, 0FFFFFFF0h
        fxsave [edi]
        fninit
        cld
        push 1
        push ebx
        call Stage36End
        add esp, 8
        fxrstor [edi]
        mov esp, ebx
        popad
        popfd
        ret
    }
}
// Five-argument cdecl wrapper for F79E4 -> F4360 action candidate filter.
__declspec(naked) void Stage36CandidateHook()
{
    __asm {
        pushfd
        pushad
        mov ebx, esp
        sub esp, 210h
        lea edi, [esp+0Fh]
        and edi, 0FFFFFFF0h
        fxsave [edi]
        fninit
        cld
        push 3
        push ebx
        call Stage36Begin
        add esp, 8
        fxrstor [edi]
        mov esp, ebx
        popad
        popfd

        push dword ptr [esp+14h] // original arg5
        push dword ptr [esp+14h] // original arg4
        push dword ptr [esp+14h] // original arg3
        push dword ptr [esp+14h] // original arg2
        push dword ptr [esp+14h] // original arg1
        call dword ptr [g_stage36CandidateTarget]
        lea esp, [esp+14h] // preserve native EFLAGS

        pushfd
        pushad
        mov ebx, esp
        sub esp, 210h
        lea edi, [esp+0Fh]
        and edi, 0FFFFFFF0h
        fxsave [edi]
        fninit
        cld
        push 3
        push ebx
        call Stage36End
        add esp, 8
        fxrstor [edi]
        mov esp, ebx
        popad
        popfd
        ret
    }
}
// Stage10 ParentFlow normally installs its own six-byte inline detour
// at F4360 before Stage36. Only trust an executable jump back into our
// dinput8.dll; never accept an arbitrary foreign/unknown native patch.
bool Stage36RecognizeStage10Detour(const BYTE* entry, DWORD& destination, DWORD& shape)
{
    destination=0u; shape=0u;
    if (!entry) return false;
    DWORD target=0u;
    if (entry[0]==0xE9 && entry[5]==0x90)
    {
        std::int32_t disp=0;
        std::memcpy(&disp,entry+1,4);
        target=static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(entry+5)+disp);
        shape=1u;
    }
    else if (entry[0]==0xFF && entry[1]==0x25)
    {
        DWORD ptr=0u;
        std::memcpy(&ptr,entry+2,4);
        if (!SafeReadDword(ptr,&target)) return false;
        shape=2u;
    }
    else if (entry[0]==0x68 && entry[5]==0xC3)
    {
        std::memcpy(&target,entry+1,4);
        shape=3u;
    }
    if (!target) return false;
    HMODULE self=nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&Stage36CandidateHook),&self))
        return false;
    MEMORY_BASIC_INFORMATION m{};
    if (!VirtualQuery(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(target)),
                      &m,sizeof(m)))
        return false;
    const DWORD protect=m.Protect&0xFFu;
    if (m.State!=MEM_COMMIT || m.AllocationBase!=self ||
        (protect!=PAGE_EXECUTE && protect!=PAGE_EXECUTE_READ &&
         protect!=PAGE_EXECUTE_READWRITE && protect!=PAGE_EXECUTE_WRITECOPY))
        return false;
    destination=target;
    return true;
}

bool InstallStage36Hooks()
{
    if (!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE))
    {
        AppendStage36Log("ERROR: FXSAVE/FXRSTOR not supported; Stage36 not installed.");
        return false;
    }
    BYTE* a = reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase + kStage36Input1FCall));
    BYTE* b = reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase + kStage36Input1ECall));
    BYTE* c = reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase + kStage36CandidateCall));
    BYTE* inNative = reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase + kStage36InputTarget));
    BYTE* candidateNative = reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase + kStage36CandidateTarget));
    const BYTE aExpected[5] = {0xE8,0xC3,0xA1,0x00,0x00};
    const BYTE bExpected[5] = {0xE8,0x02,0xA1,0x00,0x00};
    const BYTE cExpected[5] = {0xE8,0x77,0xC9,0xFF,0xFF};
    const BYTE aPrefix[4] = {0x50,0x88,0x5D,0xFF};
    const BYTE bPrefix[3] = {0x6A,0x1E,0x50};
    const BYTE cPrefix[3] = {0x56,0x53,0x57};
    const BYTE aSuffix[5] = {0x83,0xC4,0x08,0x84,0xC0};
    const BYTE bSuffix[7] = {0x83,0xC4,0x08,0x84,0xC0,0x74,0x0B};
    const BYTE cSuffix[8] = {0x83,0xC4,0x14,0x83,0x7D,0xF8,0x00,0x8B};
    const BYTE inPrologue[10] = {0x55,0x8B,0xEC,0x8B,0x45,0x0C,0x56,0x8B,0x75,0x08};
    const BYTE candidatePrologue[10] = {0x55,0x8B,0xEC,0x83,0xEC,0x14,0x53,0x56,0x8B,0x75};
    DWORD stage10Target=0u, stage10Shape=0u;
    const bool originalF4360 =
        std::memcmp(candidateNative,candidatePrologue,sizeof(candidatePrologue))==0;
    const bool stage10Detour = !originalF4360 &&
        Stage36RecognizeStage10Detour(candidateNative,stage10Target,stage10Shape);
    {
        char status[330]{};
        sprintf_s(status,
            "STAGE36_R2_ENTRY F4360=0x%08lX bytes=%02X%02X%02X%02X%02X%02X mode=%s detourShape=%lu target=0x%08lX",
            static_cast<unsigned long>(g_gameBase+kStage36CandidateTarget),
            candidateNative[0],candidateNative[1],candidateNative[2],candidateNative[3],candidateNative[4],candidateNative[5],
            originalF4360?"ORIGINAL":stage10Detour?"STAGE10_OWN_DLL":"UNKNOWN",
            static_cast<unsigned long>(stage10Shape),static_cast<unsigned long>(stage10Target));
        AppendStage36Log(status);
    }
    auto valid = [&]() -> bool {
        if (std::memcmp(a, aExpected, 5) != 0 ||
            std::memcmp(b, bExpected, 5) != 0 ||
            std::memcmp(c, cExpected, 5) != 0 ||
            std::memcmp(a - sizeof(aPrefix), aPrefix, sizeof(aPrefix)) != 0 ||
            std::memcmp(b - sizeof(bPrefix), bPrefix, sizeof(bPrefix)) != 0 ||
            std::memcmp(c - sizeof(cPrefix), cPrefix, sizeof(cPrefix)) != 0 ||
            std::memcmp(a + 5, aSuffix, sizeof(aSuffix)) != 0 ||
            std::memcmp(b + 5, bSuffix, sizeof(bSuffix)) != 0 ||
            std::memcmp(c + 5, cSuffix, sizeof(cSuffix)) != 0 ||
            std::memcmp(inNative, inPrologue, sizeof(inPrologue)) != 0 ||
            (!originalF4360 && !stage10Detour))
            return false;
        std::int32_t ad = 0, bd = 0, cd = 0;
        std::memcpy(&ad, a + 1, 4);
        std::memcpy(&bd, b + 1, 4);
        std::memcpy(&cd, c + 1, 4);
        return a + 5 + ad == inNative &&
               b + 5 + bd == inNative &&
               c + 5 + cd == candidateNative;
    };
    if (!valid())
    {
                char diagnostics[220]{};
        sprintf_s(diagnostics,
            "ERROR: Stage36 r2 site mismatch: callBytes=%d,%d,%d prefix=%d,%d,%d suffix=%d,%d,%d inputPrologue=%d f4360Allowed=%d",
            std::memcmp(a,aExpected,5)==0,std::memcmp(b,bExpected,5)==0,std::memcmp(c,cExpected,5)==0,
            std::memcmp(a-sizeof(aPrefix),aPrefix,sizeof(aPrefix))==0,
            std::memcmp(b-sizeof(bPrefix),bPrefix,sizeof(bPrefix))==0,
            std::memcmp(c-sizeof(cPrefix),cPrefix,sizeof(cPrefix))==0,
            std::memcmp(a+5,aSuffix,sizeof(aSuffix))==0,
            std::memcmp(b+5,bSuffix,sizeof(bSuffix))==0,
            std::memcmp(c+5,cSuffix,sizeof(cSuffix))==0,
            std::memcmp(inNative,inPrologue,sizeof(inPrologue))==0,
            originalF4360||stage10Detour);
        AppendStage36Log(diagnostics);
        return false;
    }
    BYTE patches[3][5]{};
    if (!MakeRelativeCall(a, reinterpret_cast<const void*>(&Stage36InputHook), patches[0]) ||
        !MakeRelativeCall(b, reinterpret_cast<const void*>(&Stage36InputHook), patches[1]) ||
        !MakeRelativeCall(c, reinterpret_cast<const void*>(&Stage36CandidateHook), patches[2]))
    {
        AppendStage36Log("ERROR: Stage36 wrapper outside rel32 CALL range; no patches.");
        return false;
    }
    HANDLE mutex = CreateMutexW(nullptr, FALSE, L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if (!mutex) return false;
    const DWORD wait = WaitForSingleObject(mutex, INFINITE);
    if (wait != WAIT_OBJECT_0 && wait != WAIT_ABANDONED)
    {
        CloseHandle(mutex);
        return false;
    }
    HANDLE suspended[512]{};
    const DWORD suspendedCount = SuspendOtherProcessThreads(suspended, 512);
    BYTE* sites[3] = {a,b,c};
    DWORD old[3]{};
    int protectedCount = 0;
    for (int i=0; i<3; ++i)
    {
        if (!VirtualProtect(sites[i], 5, PAGE_EXECUTE_READWRITE, &old[i]))
            break;
        ++protectedCount;
    }
    bool success = false;
    if (protectedCount == 3 && valid())
    {
        g_stage36InputTarget = g_gameBase + kStage36InputTarget;
        g_stage36CandidateTarget = g_gameBase + kStage36CandidateTarget;
        for (int i=0; i<3; ++i)
        {
            std::memcpy(sites[i], patches[i], 5);
            FlushInstructionCache(GetCurrentProcess(), sites[i], 5);
        }
        InterlockedExchange(&g_stage36Installed, 1);
        success = true;
    }
    for (int i=protectedCount-1; i>=0; --i)
    {
        DWORD ignored = 0;
        VirtualProtect(sites[i], 5, old[i], &ignored);
    }
    ResumeThreads(suspended, suspendedCount);
    ReleaseMutex(mutex);
    CloseHandle(mutex);
    if (success)
        AppendStage36Log("STAGE36_R2_INSTALLED EB718->F58E0 index=1F EB7D9->F58E0 index=1E F79E4->F4360 candidate; originals called once; 3/3 sites.");
    else
        AppendStage36Log("ERROR: Stage36 3-callsite patch installation failed; no patches applied.");
    return success;
}
void FlushStage36()
{
    for (DWORD n=0; n<kStage36RingSize; ++n)
    {
        Stage36Record r{};
        AcquireSRWLockExclusive(&g_stage36Lock);
        const bool exists = g_stage36Count != 0;
        if (exists)
        {
            r = g_stage36Ring[g_stage36Read];
            g_stage36Read = (g_stage36Read + 1u) % kStage36RingSize;
            --g_stage36Count;
        }
        ReleaseSRWLockExclusive(&g_stage36Lock);
        if (!exists) break;
        char message[2500]{};
        int length = sprintf_s(message,
            "STAGE36_RESULT seq=%lu kind=%s side=P%lu player=0x%08lX tid=%lu qpcBefore=%I64u qpcAfter=%I64u tickBefore=%lu tickAfter=%lu callReturnRVA=0x%08lX args=(%08lX,%08lX,%08lX,%08lX,%08lX) argMask=0x%02lX nativeEAX=0x%08lX nativeAL=%lu preMask=0x%03lX postMask=0x%03lX outMask=0x%lX>0x%lX outputs=%08lX>%08lX,%08lX>%08lX",
            static_cast<unsigned long>(r.Seq),
            r.Kind == Stage36Input1F ? "INPUT_1F" : r.Kind == Stage36Input1E ? "INPUT_1E" : "CANDIDATE",
            static_cast<unsigned long>(r.Side), static_cast<unsigned long>(r.Player),
            static_cast<unsigned long>(r.Tid),
            static_cast<unsigned long long>(r.QpcBefore),
            static_cast<unsigned long long>(r.QpcAfter),
            static_cast<unsigned long>(r.TickBefore), static_cast<unsigned long>(r.TickAfter),
            static_cast<unsigned long>(r.CallReturn - g_gameBase),
            static_cast<unsigned long>(r.Args[0]), static_cast<unsigned long>(r.Args[1]),
            static_cast<unsigned long>(r.Args[2]), static_cast<unsigned long>(r.Args[3]),
            static_cast<unsigned long>(r.Args[4]), static_cast<unsigned long>(r.ArgMask),
            static_cast<unsigned long>(r.EaxReturn),
            static_cast<unsigned long>(r.EaxReturn & 0xFFu),
            static_cast<unsigned long>(r.PreMask), static_cast<unsigned long>(r.PostMask),
            static_cast<unsigned long>(r.OutMaskPre), static_cast<unsigned long>(r.OutMaskPost),
            static_cast<unsigned long>(r.OutPre[0]), static_cast<unsigned long>(r.OutPost[0]),
            static_cast<unsigned long>(r.OutPre[1]), static_cast<unsigned long>(r.OutPost[1]));
        if (length <= 0) continue;
        for (DWORD i=0; i<kStage36Fields; ++i)
        {
            if (static_cast<size_t>(length) + 40u >= sizeof(message)) break;
            const int extra = sprintf_s(message + length, sizeof(message)-static_cast<size_t>(length),
                " f%lu=%08lX>%08lX", static_cast<unsigned long>(i),
                static_cast<unsigned long>(r.Pre[i]), static_cast<unsigned long>(r.Post[i]));
            if (extra <= 0) break;
            length += extra;
        }
        AppendStage36Log(message);
    }
}
