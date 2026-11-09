// Stage35 source-only passive per-side controller consumer observer.
// Included inside AIQueueTrace.cpp anonymous namespace immediately before QueueTraceWorker.
// No game-state writes; original native cdecl functions invoked exactly once.
constexpr DWORD kStage35TimerCallRva = 0xACABEu;
constexpr DWORD kStage35TimerTargetRva = 0xF60C0u;
constexpr DWORD kStage35FlagsCallRva = 0xACAC4u;
constexpr DWORD kStage35FlagsTargetRva = 0xF5BD0u;
constexpr DWORD kStage35Capacity = 2048u;
constexpr DWORD kStage35MaxDepth = 8u;
constexpr DWORD kStage35Fields = 12u;
enum { Stage35TimerPhase = 1, Stage35FlagsPhase = 2 };
volatile LONG g_stage35Installed = 0;
volatile LONG g_stage35Enabled = 0;
DWORD g_stage35TimerTarget = 0;
DWORD g_stage35FlagsTarget = 0;
volatile LONG g_stage35Dropped = 0;
volatile LONG g_stage35DepthOverflow = 0;
struct Stage35Record
{
    DWORD Sequence, Phase, Side, Player, Thread, TickBefore, TickAfter;
    ULONGLONG QpcBefore, QpcAfter;
    DWORD PreMask, PostMask;
    DWORD Pre[kStage35Fields], Post[kStage35Fields];
};
__declspec(thread) DWORD g_stage35Depth = 0;
__declspec(thread) Stage35Record g_stage35Scopes[kStage35MaxDepth] = {};
SRWLOCK g_stage35Lock = SRWLOCK_INIT;
Stage35Record g_stage35Ring[kStage35Capacity]{};
DWORD g_stage35Read = 0, g_stage35Write = 0, g_stage35Count = 0, g_stage35Sequence = 0;

void AppendStage35Log(const char* message)
{
    wchar_t path[MAX_PATH + 1]{};
    if (!GetSiblingPath(path, MAX_PATH, L"ArcanaAI_Stage35ControllerConsumerTrace.log"))
        return;
    SYSTEMTIME now{};
    GetLocalTime(&now);
    char line[2900]{};
    const int length = sprintf_s(line,
        "[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond,
        message != nullptr ? message : "(empty)");
    if (length <= 0) return;
    HANDLE file = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                              nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return;
    DWORD written = 0;
    WriteFile(file, line, static_cast<DWORD>(length), &written, nullptr);
    CloseHandle(file);
}

void Stage35Snapshot(DWORD player, DWORD* values, DWORD& mask)
{
    if (player != g_playerP1 && player != g_playerP2) return;
    const DWORD offsets[10] = {
        0x3A08u, 0x3A0Cu, 0x3A10u, 0x3A6Cu,
        0x3B78u, 0x3B70u, 0x3ABCu, 0x3F64u, 0x3A3Cu, 0x3B84u
    };
    for (DWORD i = 0; i < 10u; ++i)
        if (SafeReadDword(player + offsets[i], &values[i]))
            mask |= (1u << i);
    DWORD object = 0;
    if (SafeReadDword(player + 4u, &object) && object != 0)
    {
        if (SafeReadDword(object + 8u, &values[10])) mask |= (1u << 10);
        if (SafeReadDword(object + 0x10u, &values[11])) mask |= (1u << 11);
    }
}

void Stage35Enqueue(Stage35Record& record)
{
    AcquireSRWLockExclusive(&g_stage35Lock);
    record.Sequence = ++g_stage35Sequence; // gaps expose dropped records
    if (g_stage35Count == kStage35Capacity)
        InterlockedIncrement(&g_stage35Dropped);
    else
    {
        g_stage35Ring[g_stage35Write] = record;
        g_stage35Write = (g_stage35Write + 1u) % kStage35Capacity;
        ++g_stage35Count;
    }
    ReleaseSRWLockExclusive(&g_stage35Lock);
}

void __cdecl Stage35Begin(const DWORD* context, DWORD phase)
{
    const DWORD depth = g_stage35Depth++;
    if (depth >= kStage35MaxDepth)
    {
        InterlockedIncrement(&g_stage35DepthOverflow);
        return;
    }
    Stage35Record& record = g_stage35Scopes[depth];
    record = Stage35Record{};
    if (context == nullptr || (phase != Stage35TimerPhase && phase != Stage35FlagsPhase))
        return;
    DWORD player = 0;
    // Original PUSHFD/PUSHAD layout: context+10 is original first cdecl argument.
    if (!SafeReadDword(static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(context + 10)),
                       &player) || (player != g_playerP1 && player != g_playerP2))
        return;
    record.Phase = phase;
    record.Player = player;
    record.Side = player == g_playerP1 ? 1u : 2u;
    record.Thread = GetCurrentThreadId();
    record.TickBefore = GetTickCount();
    record.QpcBefore = Stage31Qpc();
    Stage35Snapshot(player, record.Pre, record.PreMask);
}

void __cdecl Stage35End(DWORD phase)
{
    if (g_stage35Depth == 0)
    {
        InterlockedIncrement(&g_stage35DepthOverflow);
        return;
    }
    const DWORD depth = --g_stage35Depth;
    if (depth >= kStage35MaxDepth) return;
    Stage35Record& record = g_stage35Scopes[depth];
    if (record.Phase == 0 || record.Phase != phase)
    {
        if (record.Phase != 0) InterlockedIncrement(&g_stage35DepthOverflow);
        return;
    }
    record.TickAfter = GetTickCount();
    record.QpcAfter = Stage31Qpc();
    Stage35Snapshot(record.Player, record.Post, record.PostMask);
    Stage35Enqueue(record);
    record.Phase = 0;
}

// Both wrappers save GPR, EFLAGS, x87, MMX, XMM and MXCSR, invoke native
// original exactly once with unchanged first argument and cdecl cleanup.
__declspec(naked) void Stage35TimerHook()
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
        push Stage35TimerPhase
        push ebx
        call Stage35Begin
        add esp, 8
        fxrstor [edi]
        mov esp, ebx
        popad
        popfd
        push dword ptr [esp+4]
        call dword ptr [g_stage35TimerTarget]
        lea esp, [esp+4]
        pushfd
        pushad
        mov ebx, esp
        sub esp, 210h
        lea edi, [esp+0Fh]
        and edi, 0FFFFFFF0h
        fxsave [edi]
        fninit
        cld
        push Stage35TimerPhase
        call Stage35End
        add esp, 4
        fxrstor [edi]
        mov esp, ebx
        popad
        popfd
        ret
    }
}
__declspec(naked) void Stage35FlagsHook()
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
        push Stage35FlagsPhase
        push ebx
        call Stage35Begin
        add esp, 8
        fxrstor [edi]
        mov esp, ebx
        popad
        popfd
        push dword ptr [esp+4]
        call dword ptr [g_stage35FlagsTarget]
        lea esp, [esp+4]
        pushfd
        pushad
        mov ebx, esp
        sub esp, 210h
        lea edi, [esp+0Fh]
        and edi, 0FFFFFFF0h
        fxsave [edi]
        fninit
        cld
        push Stage35FlagsPhase
        call Stage35End
        add esp, 4
        fxrstor [edi]
        mov esp, ebx
        popad
        popfd
        ret
    }
}

bool InstallStage35Hooks()
{
    if (!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE))
    {
        AppendStage35Log("ERROR: FXSAVE unsupported; Stage35 hooks not installed.");
        return false;
    }
    BYTE* timer = reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase + kStage35TimerCallRva));
    BYTE* flags = reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase + kStage35FlagsCallRva));
    BYTE* timerNative = reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase + kStage35TimerTargetRva));
    BYTE* flagsNative = reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase + kStage35FlagsTargetRva));
    const BYTE timerExpected[5] = {0xE8,0xFD,0x95,0x04,0x00};
    const BYTE flagsExpected[5] = {0xE8,0x07,0x91,0x04,0x00};
    const BYTE timerPrologue[10] = {0x55,0x8B,0xEC,0x83,0xEC,0x10,0x53,0x56,0x8B,0x75};
    const BYTE flagsPrologue[10] = {0x55,0x8B,0xEC,0x83,0xEC,0x1C,0x53,0x56,0x8B,0x75};
    const BYTE flagsSuffix[5] = {0x8B,0x76,0x04,0x57,0x53};
    auto match = [&]() -> bool {
        if (timer[-1] != 0x56 || flags[-1] != 0x56 ||
            std::memcmp(timer, timerExpected, 5) != 0 ||
            std::memcmp(flags, flagsExpected, 5) != 0 ||
            std::memcmp(flags + 5, flagsSuffix, sizeof(flagsSuffix)) != 0 ||
            std::memcmp(timerNative, timerPrologue, sizeof(timerPrologue)) != 0 ||
            std::memcmp(flagsNative, flagsPrologue, sizeof(flagsPrologue)) != 0)
            return false;
        std::int32_t dt = 0, df = 0;
        std::memcpy(&dt, timer + 1, 4);
        std::memcpy(&df, flags + 1, 4);
        return timer + 5 + dt == timerNative && flags + 5 + df == flagsNative;
    };
    if (!match())
    {
        AppendStage35Log("ERROR: Stage35 native callsites/prologues mismatch; no patch applied.");
        return false;
    }
    BYTE timerPatch[5]{}, flagsPatch[5]{};
    if (!MakeRelativeCall(timer, reinterpret_cast<const void*>(&Stage35TimerHook), timerPatch) ||
        !MakeRelativeCall(flags, reinterpret_cast<const void*>(&Stage35FlagsHook), flagsPatch))
    {
        AppendStage35Log("ERROR: Stage35 wrapper outside rel32 CALL range; no patch applied.");
        return false;
    }
    HANDLE mutex = CreateMutexW(nullptr, FALSE, L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if (mutex == nullptr) return false;
    const DWORD wait = WaitForSingleObject(mutex, INFINITE);
    if (wait != WAIT_OBJECT_0 && wait != WAIT_ABANDONED)
    {
        CloseHandle(mutex);
        return false;
    }
    HANDLE suspended[512]{};
    const DWORD stopped = SuspendOtherProcessThreads(suspended, 512);
    DWORD oldTimer = 0, oldFlags = 0;
    const bool pt = VirtualProtect(timer, 5, PAGE_EXECUTE_READWRITE, &oldTimer) != 0;
    const bool pf = pt && VirtualProtect(flags, 5, PAGE_EXECUTE_READWRITE, &oldFlags) != 0;
    bool installed = false;
    if (pt && pf && match())
    {
        g_stage35TimerTarget = g_gameBase + kStage35TimerTargetRva;
        g_stage35FlagsTarget = g_gameBase + kStage35FlagsTargetRva;
        std::memcpy(timer, timerPatch, 5);
        std::memcpy(flags, flagsPatch, 5);
        FlushInstructionCache(GetCurrentProcess(), timer, 5);
        FlushInstructionCache(GetCurrentProcess(), flags, 5);
        InterlockedExchange(&g_stage35Installed, 1);
        installed = true;
    }
    if (pf)
    {
        DWORD ignored = 0;
        VirtualProtect(flags, 5, oldFlags, &ignored);
    }
    if (pt)
    {
        DWORD ignored = 0;
        VirtualProtect(timer, 5, oldTimer, &ignored);
    }
    ResumeThreads(suspended, stopped);
    ReleaseMutex(mutex);
    CloseHandle(mutex);
    if (installed)
        AppendStage35Log("STAGE35_INSTALLED callsite=ACABE->F60C0 phase=TIMER callsite=ACAC4->F5BD0 phase=FLAGS; originals called once; bounded snapshots.");
    else
        AppendStage35Log("ERROR: Stage35 install failed or callsites changed; no Stage35 patch applied.");
    return installed;
}

void FlushStage35()
{
    for (DWORD n = 0; n < kStage35Capacity; ++n)
    {
        Stage35Record r{};
        AcquireSRWLockExclusive(&g_stage35Lock);
        const bool hasRecord = g_stage35Count != 0;
        if (hasRecord)
        {
            r = g_stage35Ring[g_stage35Read];
            g_stage35Read = (g_stage35Read + 1u) % kStage35Capacity;
            --g_stage35Count;
        }
        ReleaseSRWLockExclusive(&g_stage35Lock);
        if (!hasRecord) break;
        char line[2200]{};
        int count = sprintf_s(line,
            "STAGE35_CONSUMER seq=%lu phase=%s side=P%lu player=0x%08lX tid=%lu qpcBefore=%I64u qpcAfter=%I64u tickBefore=%lu tickAfter=%lu preMask=0x%03lX postMask=0x%03lX",
            static_cast<unsigned long>(r.Sequence),
            r.Phase == Stage35TimerPhase ? "TIMER" : "FLAGS",
            static_cast<unsigned long>(r.Side), static_cast<unsigned long>(r.Player),
            static_cast<unsigned long>(r.Thread),
            static_cast<unsigned long long>(r.QpcBefore),
            static_cast<unsigned long long>(r.QpcAfter),
            static_cast<unsigned long>(r.TickBefore),
            static_cast<unsigned long>(r.TickAfter),
            static_cast<unsigned long>(r.PreMask),
            static_cast<unsigned long>(r.PostMask));
        if (count <= 0) continue;
        for (DWORD i = 0; i < kStage35Fields; ++i)
        {
            if (static_cast<size_t>(count) + 40 >= sizeof(line)) break;
            const int added = sprintf_s(line + count, sizeof(line) - static_cast<size_t>(count),
                " f%lu=%08lX>%08lX",
                static_cast<unsigned long>(i),
                static_cast<unsigned long>(r.Pre[i]),
                static_cast<unsigned long>(r.Post[i]));
            if (added <= 0) break;
            count += added;
        }
        AppendStage35Log(line);
    }
}
