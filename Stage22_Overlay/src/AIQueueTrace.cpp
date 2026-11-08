#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <TlHelp32.h>
#include <intrin.h>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cwchar>

#include "AIQueueTrace.h"

extern "C" void __cdecl ArcanaLogStage21PayloadClassifier(const DWORD* savedContext);
extern "C" void __cdecl ArcanaLogStage21PayloadSlotClear(const DWORD* savedContext);
extern "C" void __cdecl ArcanaLogStage22PayloadPostAnd(const DWORD* savedContext);

namespace
{
    constexpr DWORD kExpectedImageSize = 0x0E0EB000u;
    constexpr DWORD kExpectedTimeDateStamp = 0x5FE5AA91u;

    constexpr DWORD kP1ControlRva = 0x0DCA5EA4u;
    constexpr DWORD kP2ControlRva = 0x0DCD0BCCu;
    constexpr DWORD kPlayerControlOffset = 0x3F2Cu;
    constexpr DWORD kPlayerSideOffset = 0x3F30u;
    constexpr DWORD kPlayerQueueOffset = 0x3A2Cu;

    // AALib.exe VA 0x004F2DE0, with preferred image base 0x00400000.
    constexpr DWORD kQueueInsertRva = 0x000F2DE0u;
    constexpr BYTE kExpectedPrologue[6] = { 0x55, 0x8B, 0xEC, 0x8B, 0x45, 0x08 };

    // AALib.exe VA 0x004EE200 (RVA 0x000EE200): dispatches a resolved
    // per-player event by event type and payload. The entry prologue is 7 bytes.
    constexpr DWORD kEventDispatchRva = 0x000EE200u;
    constexpr BYTE kEventDispatchPrologue[7] = { 0x55, 0x8B, 0xEC, 0x53, 0x8B, 0x5D, 0x0C };

    // Stage21 instruments the two real consumer-side calls inside AALib.exe+0xEE200.
    // EE426 calls 4E8CF0 (EAX=payload, ESI=player, stack arg=1).
    // EE446 calls 4F7EB0 (stack args: controller-array pointer, slot index=3).
    constexpr DWORD kStage21PayloadClassifierCallRva = 0x000EE426u;
    constexpr DWORD kStage21PayloadClassifierRva = 0x000E8CF0u;
    constexpr BYTE kStage21ClassifierPrefix[4] = { 0x6A, 0x01, 0x8B, 0xC3 };
    constexpr BYTE kStage21ClassifierCall[5] = { 0xE8, 0xC5, 0xA8, 0xFF, 0xFF };
    constexpr BYTE kStage21ClassifierSuffix[7] = { 0x83, 0xC4, 0x04, 0x84, 0xC0, 0x74, 0x11 };

    constexpr DWORD kStage21PayloadSlotClearCallRva = 0x000EE446u;
    constexpr DWORD kStage21PayloadSlotClearRva = 0x000F7EB0u;
    constexpr BYTE kStage21SlotClearPrefix[3] = { 0x6A, 0x03, 0x57 };
    constexpr BYTE kStage21SlotClearCall[5] = { 0xE8, 0x65, 0x9A, 0x00, 0x00 };
    constexpr DWORD kStage21Payload1E = 0x0000001Eu;

    // Controller flag clear immediately after the Stage21 fallback helper.
    // Validated 7-byte instruction at AALib.exe+0xEE451.
    constexpr DWORD kStage22ControllerFlagsAndRva = 0x000EE451u;
    constexpr BYTE kStage22ControllerFlagsAndPrefix[6] = { 0x8B, 0x76, 0x04, 0x83, 0xC4, 0x08 };
    constexpr BYTE kStage22ControllerFlagsAndInstruction[7] = { 0x81, 0x66, 0x08, 0xFF, 0xFB, 0xFF, 0xFF };

    constexpr DWORD kSiteCapacity = 512;
    constexpr DWORD kDetailCapacity = 1024;
    constexpr DWORD kDispatchDetailCapacity = 1024;
    constexpr DWORD kDetailStackDepth = 8;
    constexpr DWORD kDefaultFlushIntervalMs = 1000;
    constexpr DWORD kMinFlushIntervalMs = 250;
    constexpr DWORD kMaxFlushIntervalMs = 5000;

    using QueueInsertFn = void(__cdecl*)(void* queue, DWORD eventType, DWORD payload);
    using EventDispatchFn = void(__thiscall*)(void* player, DWORD eventType, DWORD payload);

    struct QueueSnapshot
    {
        BYTE active;
        BYTE field1;
        WORD storedType;
        WORD storedPayload;
        BYTE field6;
        BYTE field7;
        WORD field8;
        WORD field10;
        WORD field12;
        BYTE field14;
        BYTE guard;
    };

    struct QueueSite
    {
        // State: 0 = empty, -1 = being initialized, 1 = ready.
        volatile LONG State;
        volatile LONG Count;
        volatile LONG ReportedCount;
        DWORD Side;
        DWORD FirstThreadId;
        DWORD CallerRva;
        DWORD EventTypeArg;
        DWORD PayloadArg;
        DWORD GuardBefore;
        DWORD ActiveBefore;
        DWORD ActiveAfter;
        DWORD StoredTypeAfter;
        DWORD StoredPayloadAfter;
        DWORD GuardAfter;
    };

    struct EventDispatchSite
    {
        // State: 0 = empty, -1 = being initialized, 1 = ready.
        volatile LONG State;
        volatile LONG Count;
        volatile LONG ReportedCount;
        DWORD Side;
        DWORD FirstThreadId;
        DWORD CallerRva;
        DWORD EventTypeArg;
        DWORD PayloadArg;
        DWORD PlayerMode;
        DWORD QueueActiveBefore;
        DWORD QueueCurrentTypeBefore;
        DWORD QueueCurrentPayloadBefore;
        DWORD QueueHistoryTypeBefore;
        DWORD QueueHistoryPayloadBefore;
        DWORD QueueHistoryMatchesArgs;
        DWORD QueueGuardBefore;
        DWORD QueueActiveAfter;
        DWORD QueueHistoryTypeAfter;
        DWORD QueueHistoryPayloadAfter;
    };

    struct QueueEventDetailRecord
    {
        DWORD Sequence;
        DWORD Side;
        DWORD ThreadId;
        DWORD TickCount;
        ULONGLONG QpcTicks;
        DWORD CallerAddress;
        DWORD CallerRva;
        DWORD PlayerObject;
        DWORD QueueAddress;
        DWORD EventType;
        DWORD Payload;
        DWORD GuardBefore;
        DWORD ActiveBefore;
        DWORD AfterReadable;
        DWORD GuardAfter;
        DWORD ActiveAfter;
        DWORD StoredTypeAfter;
        DWORD StoredPayloadAfter;
        DWORD StackDepth;
        PVOID Stack[kDetailStackDepth];
        DWORD HookEbp;
        DWORD HookEsp;
        DWORD FrameDepth;
        DWORD FrameZeroMatchesCaller;
        DWORD FramePointers[kDetailStackDepth];
        DWORD ReturnAddresses[kDetailStackDepth];
    };

    struct EventDispatchDetailRecord
    {
        DWORD Sequence;
        DWORD Side;
        DWORD ThreadId;
        DWORD TickBefore;
        DWORD TickAfter;
        ULONGLONG QpcBefore;
        ULONGLONG QpcAfter;
        DWORD CallerAddress;
        DWORD CallerRva;
        DWORD PlayerObject;
        DWORD QueueAddress;
        DWORD EventType;
        DWORD Payload;
        DWORD PlayerMode;
        DWORD AfterReadable;
        QueueSnapshot Before;
        QueueSnapshot After;
    };

    INIT_ONCE g_startOnce = INIT_ONCE_STATIC_INIT;
    HMODULE g_proxyModule = nullptr;
    HMODULE g_gameModule = nullptr;
    DWORD g_gameBase = 0;
    DWORD g_gameImageSize = 0;
    DWORD g_playerP1 = 0;
    DWORD g_playerP2 = 0;
    DWORD g_p1Queue = 0;
    DWORD g_p2Queue = 0;
    DWORD g_flushIntervalMs = kDefaultFlushIntervalMs;

    BYTE* g_hookTarget = nullptr;
    BYTE* g_trampoline = nullptr;
    QueueInsertFn g_originalQueueInsert = nullptr;
    volatile LONG g_hookInstalled = 0;
    volatile LONG g_droppedSites = 0;
    volatile LONG g_loggedSites = 0;
    QueueSite g_sites[kSiteCapacity]{};

    BYTE* g_dispatchHookTarget = nullptr;
    BYTE* g_dispatchTrampoline = nullptr;
    EventDispatchFn g_originalEventDispatch = nullptr;
    volatile LONG g_dispatchHookInstalled = 0;
    volatile LONG g_dispatchDroppedSites = 0;
    volatile LONG g_dispatchLoggedSites = 0;
    EventDispatchSite g_dispatchSites[kSiteCapacity]{};
    volatile LONG g_dispatchDetailTraceEnabled = 0;
    volatile LONG g_dispatchDetailDroppedRecords = 0;
    SRWLOCK g_dispatchDetailLock = SRWLOCK_INIT;
    EventDispatchDetailRecord g_dispatchDetailRecords[kDispatchDetailCapacity]{};
    DWORD g_dispatchDetailReadIndex = 0;
    DWORD g_dispatchDetailWriteIndex = 0;
    DWORD g_dispatchDetailCount = 0;
    DWORD g_dispatchDetailSequence = 0;

    volatile DWORD g_stage21PayloadClassifierTarget = 0;
    volatile DWORD g_stage21PayloadSlotClearTarget = 0;
    volatile LONG g_stage21PayloadConsumerInstalled = 0;
    volatile LONG g_stage22PayloadPostAndInstalled = 0;
    volatile LONG g_stage22PayloadPostAndSequence = 0;

    SRWLOCK g_detailLock = SRWLOCK_INIT;
    QueueEventDetailRecord g_detailRecords[kDetailCapacity]{};
    DWORD g_detailReadIndex = 0;
    DWORD g_detailWriteIndex = 0;
    DWORD g_detailCount = 0;
    DWORD g_detailSequence = 0;
    volatile LONG g_detailDroppedRecords = 0;
    volatile LONG g_detailTraceEnabled = 0;

    bool GetSiblingPath(wchar_t* output, DWORD capacity, const wchar_t* filename)
    {
        if (output == nullptr || capacity == 0 || filename == nullptr || g_proxyModule == nullptr)
        {
            return false;
        }

        const DWORD length = GetModuleFileNameW(g_proxyModule, output, capacity);
        if (length == 0 || length >= capacity)
        {
            return false;
        }

        wchar_t* slash = wcsrchr(output, L'\\');
        wchar_t* forwardSlash = wcsrchr(output, L'/');
        if (forwardSlash != nullptr && (slash == nullptr || forwardSlash > slash))
        {
            slash = forwardSlash;
        }
        if (slash == nullptr)
        {
            return false;
        }

        *(slash + 1) = L'\0';
        const std::size_t used = wcslen(output);
        const std::size_t nameLength = wcslen(filename);
        if (used + nameLength + 1 > capacity)
        {
            return false;
        }
        wcscat_s(output, capacity, filename);
        return true;
    }

    void AppendQueueLog(const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_QueueTrace.log"))
        {
            return;
        }

        SYSTEMTIME now{};
        GetLocalTime(&now);
        char line[2048]{};
        const int count = sprintf_s(
            line,
            "[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond,
            message != nullptr ? message : "(empty)");
        if (count <= 0)
        {
            return;
        }

        HANDLE file = CreateFileW(
            logPath,
            FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (file == INVALID_HANDLE_VALUE)
        {
            return;
        }

        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    void AppendStage21ConsumerLog(const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_Stage21PayloadConsumerTrace.log"))
            return;

        SYSTEMTIME now{};
        GetLocalTime(&now);
        char line[1400]{};
        const int count = sprintf_s(
            line,
            "[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond,
            message != nullptr ? message : "(empty)");
        if (count <= 0) return;

        HANDLE file = CreateFileW(
            logPath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;

        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    void AppendStage22PayloadLog(const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_Stage22PayloadPostAndTrace.log"))
            return;

        SYSTEMTIME now{};
        GetLocalTime(&now);
        char line[1600]{};
        const int count = sprintf_s(
            line,
            "[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond,
            message != nullptr ? message : "(empty)");
        if (count <= 0) return;

        HANDLE file = CreateFileW(
            logPath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;

        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    void AppendDispatchLog(const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_EventDispatchTrace.log"))
        {
            return;
        }

        SYSTEMTIME now{};
        GetLocalTime(&now);
        char line[1200]{};
        const int count = sprintf_s(
            line,
            "[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond,
            message != nullptr ? message : "(empty)");
        if (count <= 0)
        {
            return;
        }

        HANDLE file = CreateFileW(
            logPath,
            FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (file == INVALID_HANDLE_VALUE)
        {
            return;
        }

        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    int ReadIniInt(const wchar_t* section, const wchar_t* key, int defaultValue)
    {
        wchar_t iniPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(iniPath, MAX_PATH, L"ArcanaAI.ini"))
        {
            return defaultValue;
        }
        return GetPrivateProfileIntW(section, key, defaultValue, iniPath);
    }

    bool GetImageFingerprint32(HMODULE module, DWORD* imageSize, DWORD* timeDateStamp)
    {
        if (module == nullptr || imageSize == nullptr || timeDateStamp == nullptr)
        {
            return false;
        }

        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0)
        {
            return false;
        }

        const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS32*>(
            reinterpret_cast<const BYTE*>(module) + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE ||
            nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        {
            return false;
        }

        *imageSize = nt->OptionalHeader.SizeOfImage;
        *timeDateStamp = nt->FileHeader.TimeDateStamp;
        return true;
    }

    bool SafeReadDword(DWORD address, DWORD* value)
    {
        if (address == 0 || value == nullptr)
        {
            return false;
        }
        __try
        {
            *value = *reinterpret_cast<const volatile DWORD*>(address);
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            *value = 0;
            return false;
        }
    }

    DWORD CaptureEbpFrameChain(DWORD startEbp, QueueEventDetailRecord* record)
    {
        if (startEbp == 0 || record == nullptr)
            return 0;
        DWORD frame = startEbp;
        for (DWORD i = 0; i < kDetailStackDepth; ++i)
        {
            if (frame < 0x10000u || frame > 0xFFFFF000u || (frame & 3u) != 0)
                break;
            DWORD previousFrame = 0;
            DWORD returnAddress = 0;
            if (!SafeReadDword(frame, &previousFrame) ||
                !SafeReadDword(frame + sizeof(DWORD), &returnAddress) ||
                returnAddress == 0)
                break;
            record->FramePointers[i] = frame;
            record->ReturnAddresses[i] = returnAddress;
            ++record->FrameDepth;
            if (i == 0)
                record->FrameZeroMatchesCaller = (returnAddress == record->CallerAddress) ? 1u : 0u;
            if (previousFrame <= frame || previousFrame - frame > 0x100000u)
                break;
            frame = previousFrame;
        }
        return record->FrameDepth;
    }

    bool SafeReadQueueSnapshot(DWORD address, QueueSnapshot* snapshot)
    {
        if (address == 0 || snapshot == nullptr)
        {
            return false;
        }
        __try
        {
            const volatile BYTE* p = reinterpret_cast<const volatile BYTE*>(address);
            snapshot->active = p[0];
            snapshot->field1 = p[1];
            snapshot->storedType = *reinterpret_cast<const volatile WORD*>(address + 2);
            snapshot->storedPayload = *reinterpret_cast<const volatile WORD*>(address + 4);
            snapshot->field6 = p[6];
            snapshot->field7 = p[7];
            snapshot->field8 = *reinterpret_cast<const volatile WORD*>(address + 8);
            snapshot->field10 = *reinterpret_cast<const volatile WORD*>(address + 10);
            snapshot->field12 = *reinterpret_cast<const volatile WORD*>(address + 12);
            snapshot->field14 = p[14];
            snapshot->guard = p[15];
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            *snapshot = {};
            return false;
        }
    }

    bool IsKnownPlayerQueue(DWORD queueAddress, DWORD* side, DWORD* playerObject)
    {
        if (queueAddress == g_p1Queue)
        {
            if (side != nullptr) *side = 1;
            if (playerObject != nullptr) *playerObject = g_playerP1;
            return true;
        }
        if (queueAddress == g_p2Queue)
        {
            if (side != nullptr) *side = 2;
            if (playerObject != nullptr) *playerObject = g_playerP2;
            return true;
        }
        return false;
    }

    bool SameSiteKey(const QueueSite& site, DWORD side, DWORD callerRva,
        DWORD eventType, DWORD payload, DWORD guardBefore)
    {
        return site.Side == side && site.CallerRva == callerRva &&
            site.EventTypeArg == eventType && site.PayloadArg == payload &&
            site.GuardBefore == guardBefore;
    }

    void AddQueueSite(DWORD side, DWORD threadId, DWORD callerRva,
        DWORD eventType, DWORD payload, const QueueSnapshot& before,
        const QueueSnapshot& after)
    {
        DWORD hash = callerRva ^ (eventType * 0x9E3779B9u) ^
            (payload * 0x85EBCA6Bu) ^ (side << 24) ^ (static_cast<DWORD>(before.guard) << 16);
        const DWORD start = hash % kSiteCapacity;

        for (DWORD probe = 0; probe < kSiteCapacity; ++probe)
        {
            QueueSite& site = g_sites[(start + probe) % kSiteCapacity];
            const LONG state = InterlockedCompareExchange(&site.State, 0, 0);
            if (state == 1 && SameSiteKey(site, side, callerRva, eventType, payload, before.guard))
            {
                InterlockedIncrement(&site.Count);
                return;
            }

            if (state == 0 && InterlockedCompareExchange(&site.State, -1, 0) == 0)
            {
                site.Side = side;
                site.FirstThreadId = threadId;
                site.CallerRva = callerRva;
                site.EventTypeArg = eventType;
                site.PayloadArg = payload;
                site.GuardBefore = before.guard;
                site.ActiveBefore = before.active;
                site.ActiveAfter = after.active;
                site.StoredTypeAfter = after.storedType;
                site.StoredPayloadAfter = after.storedPayload;
                site.GuardAfter = after.guard;
                InterlockedExchange(&site.Count, 1);
                MemoryBarrier();
                InterlockedExchange(&site.State, 1);
                InterlockedIncrement(&g_loggedSites);
                return;
            }
        }
        InterlockedIncrement(&g_droppedSites);
    }

    void AddQueueEventDetail(const QueueEventDetailRecord& source)
    {
        AcquireSRWLockExclusive(&g_detailLock);
        if (g_detailCount >= kDetailCapacity)
        {
            ReleaseSRWLockExclusive(&g_detailLock);
            InterlockedIncrement(&g_detailDroppedRecords);
            return;
        }
        QueueEventDetailRecord& slot = g_detailRecords[g_detailWriteIndex];
        slot = source;
        slot.Sequence = ++g_detailSequence;
        g_detailWriteIndex = (g_detailWriteIndex + 1u) % kDetailCapacity;
        ++g_detailCount;
        ReleaseSRWLockExclusive(&g_detailLock);
    }

    __declspec(noinline) void __cdecl QueueInsertHook(void* queue, DWORD eventType, DWORD payload)
    {
        const DWORD callerAddress = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(_ReturnAddress()));
        const DWORD callerRva = callerAddress >= g_gameBase ? callerAddress - g_gameBase : 0;
        const DWORD queueAddress = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(queue));
        DWORD side = 0;
        DWORD playerObject = 0;
        bool traceThisCall = false;
        QueueSnapshot before{};

        if (InterlockedCompareExchange(&g_hookInstalled, 0, 0) != 0 &&
            IsKnownPlayerQueue(queueAddress, &side, &playerObject))
        {
            DWORD cpuMode = 0;
            if (SafeReadDword(playerObject + kPlayerControlOffset, &cpuMode) && cpuMode == 1 &&
                SafeReadQueueSnapshot(queueAddress, &before))
                traceThisCall = true;
        }

        const bool captureDetail = traceThisCall && eventType == 1 &&
            InterlockedCompareExchange(&g_detailTraceEnabled, 0, 0) != 0;
        QueueEventDetailRecord detail{};
        if (captureDetail)
        {
            detail.Side = side;
            detail.ThreadId = GetCurrentThreadId();
            detail.TickCount = GetTickCount();
            LARGE_INTEGER qpc{};
            QueryPerformanceCounter(&qpc);
            detail.QpcTicks = static_cast<ULONGLONG>(qpc.QuadPart);
            detail.CallerAddress = callerAddress;
            detail.CallerRva = callerRva;
            detail.PlayerObject = playerObject;
            detail.QueueAddress = queueAddress;
            detail.EventType = eventType;
            detail.Payload = payload;
            detail.GuardBefore = before.guard;
            detail.ActiveBefore = before.active;
            DWORD hookEbp = 0;
            DWORD hookEsp = 0;
            __asm
            {
                mov eax, ebp
                mov hookEbp, eax
                mov eax, esp
                mov hookEsp, eax
            }
            detail.HookEbp = hookEbp;
            detail.HookEsp = hookEsp;
            CaptureEbpFrameChain(hookEbp, &detail);
            detail.StackDepth = CaptureStackBackTrace(0, kDetailStackDepth, detail.Stack, nullptr);
        }

        QueueInsertFn original = g_originalQueueInsert;
        if (original != nullptr) original(queue, eventType, payload);

        if (traceThisCall)
        {
            QueueSnapshot after{};
            const bool afterReadable = SafeReadQueueSnapshot(queueAddress, &after);
            if (afterReadable)
                AddQueueSite(side, GetCurrentThreadId(), callerRva, eventType, payload, before, after);
            if (captureDetail)
            {
                detail.AfterReadable = afterReadable ? 1u : 0u;
                if (afterReadable)
                {
                    detail.GuardAfter = after.guard;
                    detail.ActiveAfter = after.active;
                    detail.StoredTypeAfter = after.storedType;
                    detail.StoredPayloadAfter = after.storedPayload;
                }
                AddQueueEventDetail(detail);
            }
        }
    }

    void AddEventDispatchDetail(const EventDispatchDetailRecord& source)
    {
        AcquireSRWLockExclusive(&g_dispatchDetailLock);
        if (g_dispatchDetailCount >= kDispatchDetailCapacity)
        {
            ReleaseSRWLockExclusive(&g_dispatchDetailLock);
            InterlockedIncrement(&g_dispatchDetailDroppedRecords);
            return;
        }
        EventDispatchDetailRecord& slot = g_dispatchDetailRecords[g_dispatchDetailWriteIndex];
        slot = source;
        slot.Sequence = ++g_dispatchDetailSequence;
        g_dispatchDetailWriteIndex = (g_dispatchDetailWriteIndex + 1u) % kDispatchDetailCapacity;
        ++g_dispatchDetailCount;
        ReleaseSRWLockExclusive(&g_dispatchDetailLock);
    }

    bool SameDispatchKey(const EventDispatchSite& site, DWORD side, DWORD callerRva,
        DWORD eventType, DWORD payload)
    {
        return site.Side == side && site.CallerRva == callerRva &&
            site.EventTypeArg == eventType && site.PayloadArg == payload;
    }

    void AddDispatchSite(DWORD side, DWORD threadId, DWORD callerRva,
        DWORD eventType, DWORD payload, DWORD playerMode,
        const QueueSnapshot& before, const QueueSnapshot& after)
    {
        const DWORD start = (callerRva ^ (eventType * 0x9E3779B9u) ^
            (payload * 0x85EBCA6Bu) ^ (side << 24)) % kSiteCapacity;

        for (DWORD probe = 0; probe < kSiteCapacity; ++probe)
        {
            EventDispatchSite& site = g_dispatchSites[(start + probe) % kSiteCapacity];
            const LONG state = InterlockedCompareExchange(&site.State, 0, 0);
            if (state == 1 && SameDispatchKey(site, side, callerRva, eventType, payload))
            {
                InterlockedIncrement(&site.Count);
                return;
            }

            if (state == 0 && InterlockedCompareExchange(&site.State, -1, 0) == 0)
            {
                site.Side = side;
                site.FirstThreadId = threadId;
                site.CallerRva = callerRva;
                site.EventTypeArg = eventType;
                site.PayloadArg = payload;
                site.PlayerMode = playerMode;
                site.QueueActiveBefore = before.active;
                site.QueueCurrentTypeBefore = before.storedType;
                site.QueueCurrentPayloadBefore = before.storedPayload;
                site.QueueHistoryTypeBefore = before.field8;
                site.QueueHistoryPayloadBefore = before.field10;
                site.QueueHistoryMatchesArgs =
                    (before.field8 == static_cast<WORD>(eventType) &&
                     before.field10 == static_cast<WORD>(payload)) ? 1u : 0u;
                site.QueueGuardBefore = before.guard;
                site.QueueActiveAfter = after.active;
                site.QueueHistoryTypeAfter = after.field8;
                site.QueueHistoryPayloadAfter = after.field10;
                InterlockedExchange(&site.Count, 1);
                MemoryBarrier();
                InterlockedExchange(&site.State, 1);
                InterlockedIncrement(&g_dispatchLoggedSites);
                return;
            }
        }
        InterlockedIncrement(&g_dispatchDroppedSites);
    }

    __declspec(naked) void Stage21PayloadClassifierHook()
    {
        __asm
        {
            // Capture the decoded action code while it is still in EDI.
            // The classifier helper repurposes EDI, so reading it after the call is invalid.
            push edi

            // Original argument remains at [esp+8] after the private saved-EDI DWORD.
            mov edx, dword ptr [esp + 8]
            push edx
            call dword ptr [g_stage21PayloadClassifierTarget]
            add esp, 4

            // Preserve the helper's real EAX/AL while the logger inspects the context.
            pushfd
            pushad
            mov edx, esp
            push edx
            call ArcanaLogStage21PayloadClassifier
            add esp, 4
            popad
            popfd

            // Discard the private saved-EDI DWORD without changing the helper's flags.
            lea esp, [esp + 4]
            ret
        }
    }

    __declspec(naked) void Stage21PayloadSlotClearHook()
    {
        __asm
        {
            // Keep the old slot value in a private stack DWORD.
            // Original stack: [return EE44B][array pointer][index=3].
            sub esp, 4
            mov eax, dword ptr [esp + 8]
            mov edx, dword ptr [esp + 12]
            shl edx, 2
            lea edx, [eax + edx + 4]
            mov eax, dword ptr [edx]
            mov dword ptr [esp], eax

            // Recreate the original cdecl argument list and call the real helper once.
            mov edx, dword ptr [esp + 12]
            mov eax, dword ptr [esp + 8]
            push edx
            push eax
            call dword ptr [g_stage21PayloadSlotClearTarget]
            add esp, 8

            pushfd
            pushad
            mov edx, esp
            push edx
            call ArcanaLogStage21PayloadSlotClear
            add esp, 4
            popad
            popfd

            add esp, 4
            ret
        }
    }

    // Reproduce the exact controller-flags AND at EE451 and capture the memory
    // state on both sides. EFLAGS from the original AND are restored before return.
    __declspec(naked) void Stage22ControllerFlagsAndHook()
    {
        __asm
        {
            // Private locals: [esp]=controller pointer, [esp+4]=DWORD before AND.
            sub esp, 8
            mov dword ptr [esp], esi
            mov eax, dword ptr [esi + 8]
            mov dword ptr [esp + 4], eax

            // Execute the original 7-byte instruction exactly once.
            and dword ptr [esi + 8], 0FFFFFBFFh

            // Preserve the flags produced by that AND across the diagnostic logger.
            pushfd
            pushad
            mov edx, esp
            push edx
            call ArcanaLogStage22PayloadPostAnd
            add esp, 4
            popad
            popfd

            add esp, 8
            ret
        }
    }

    // The game calls this as __thiscall (ECX=player, two stack arguments).
    // A __fastcall detour preserves ECX, consumes the unused EDX slot, and
    // receives the same two stack arguments. The original is called exactly once.
    __declspec(noinline) void __fastcall EventDispatchHook(
        void* player, void*, DWORD eventType, DWORD payload)
    {
        const DWORD callerAddress = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(_ReturnAddress()));
        const DWORD playerAddress = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(player));
        DWORD side = 0;
        DWORD playerObject = 0;
        bool traceThisCall = false;
        DWORD playerMode = 0;
        QueueSnapshot before{};

        if (InterlockedCompareExchange(&g_dispatchHookInstalled, 0, 0) != 0 &&
            IsKnownPlayerQueue(playerAddress + kPlayerQueueOffset, &side, &playerObject) &&
            playerObject == playerAddress)
        {
            if (SafeReadDword(playerAddress + kPlayerControlOffset, &playerMode) && playerMode == 1 &&
                SafeReadQueueSnapshot(playerAddress + kPlayerQueueOffset, &before))
            {
                traceThisCall = true;
            }
        }

        const bool captureDetail = traceThisCall && eventType == 1 &&
            InterlockedCompareExchange(&g_dispatchDetailTraceEnabled, 0, 0) != 0;
        EventDispatchDetailRecord detail{};
        if (captureDetail)
        {
            detail.Side = side;
            detail.ThreadId = GetCurrentThreadId();
            detail.TickBefore = GetTickCount();
            LARGE_INTEGER qpc{};
            QueryPerformanceCounter(&qpc);
            detail.QpcBefore = static_cast<ULONGLONG>(qpc.QuadPart);
            detail.CallerAddress = callerAddress;
            detail.CallerRva = callerAddress >= g_gameBase ? callerAddress - g_gameBase : 0;
            detail.PlayerObject = playerAddress;
            detail.QueueAddress = playerAddress + kPlayerQueueOffset;
            detail.EventType = eventType;
            detail.Payload = payload;
            detail.PlayerMode = playerMode;
            detail.Before = before;
        }

        EventDispatchFn original = g_originalEventDispatch;
        if (original != nullptr)
        {
            original(player, eventType, payload);
        }

        if (traceThisCall)
        {
            QueueSnapshot after{};
            const bool afterReadable = SafeReadQueueSnapshot(playerAddress + kPlayerQueueOffset, &after);
            if (afterReadable)
            {
                const DWORD callerRva = callerAddress >= g_gameBase ? callerAddress - g_gameBase : 0;
                AddDispatchSite(side, GetCurrentThreadId(), callerRva,
                    eventType, payload, playerMode, before, after);
            }
            if (captureDetail)
            {
                detail.TickAfter = GetTickCount();
                LARGE_INTEGER qpc{};
                QueryPerformanceCounter(&qpc);
                detail.QpcAfter = static_cast<ULONGLONG>(qpc.QuadPart);
                detail.AfterReadable = afterReadable ? 1u : 0u;
                if (afterReadable)
                    detail.After = after;
                AddEventDispatchDetail(detail);
            }
        }
    }

    bool MakeRelativeJump(BYTE* source, const void* destination, BYTE* output)
    {
        if (source == nullptr || destination == nullptr || output == nullptr)
        {
            return false;
        }
        const std::intptr_t delta =
            reinterpret_cast<std::intptr_t>(destination) -
            (reinterpret_cast<std::intptr_t>(source) + 5);
        if (delta < INT32_MIN || delta > INT32_MAX)
        {
            return false;
        }
        output[0] = 0xE9;
        const std::int32_t relative = static_cast<std::int32_t>(delta);
        std::memcpy(output + 1, &relative, sizeof(relative));
        return true;
    }

    bool MakeRelativeCall(BYTE* source, const void* destination, BYTE* output)
    {
        if (source == nullptr || destination == nullptr || output == nullptr)
            return false;
        const std::intptr_t delta =
            reinterpret_cast<std::intptr_t>(destination) -
            (reinterpret_cast<std::intptr_t>(source) + 5);
        if (delta < INT32_MIN || delta > INT32_MAX)
            return false;
        output[0] = 0xE8;
        const std::int32_t relative = static_cast<std::int32_t>(delta);
        std::memcpy(output + 1, &relative, sizeof(relative));
        return true;
    }

    DWORD SuspendOtherProcessThreads(HANDLE* suspended, DWORD capacity)
    {
        if (suspended == nullptr || capacity == 0)
        {
            return 0;
        }

        const DWORD processId = GetCurrentProcessId();
        const DWORD currentThreadId = GetCurrentThreadId();
        DWORD count = 0;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
        {
            return 0;
        }

        THREADENTRY32 entry{};
        entry.dwSize = sizeof(entry);
        if (Thread32First(snapshot, &entry))
        {
            do
            {
                if (entry.th32OwnerProcessID == processId &&
                    entry.th32ThreadID != currentThreadId && count < capacity)
                {
                    HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, entry.th32ThreadID);
                    if (thread != nullptr)
                    {
                        if (SuspendThread(thread) != static_cast<DWORD>(-1))
                        {
                            suspended[count++] = thread;
                        }
                        else
                        {
                            CloseHandle(thread);
                        }
                    }
                }
                entry.dwSize = sizeof(entry);
            } while (Thread32Next(snapshot, &entry));
        }
        CloseHandle(snapshot);
        return count;
    }

    void ResumeThreads(HANDLE* suspended, DWORD count)
    {
        if (suspended == nullptr) return;
        while (count > 0)
        {
            HANDLE thread = suspended[--count];
            ResumeThread(thread);
            CloseHandle(thread);
        }
    }

    bool InstallQueueHook()
    {
        BYTE* target = reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase + kQueueInsertRva));
        if (target == nullptr)
        {
            AppendQueueLog("ERROR: queue insertion target address is null.");
            return false;
        }

        if (std::memcmp(target, kExpectedPrologue, sizeof(kExpectedPrologue)) != 0)
        {
            AppendQueueLog("ERROR: AALib.exe+0xF2DE0 prologue does not match 55 8B EC 8B 45 08; hook was not installed.");
            return false;
        }

        BYTE* trampoline = reinterpret_cast<BYTE*>(VirtualAlloc(
            nullptr, 16, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
        if (trampoline == nullptr)
        {
            AppendQueueLog("ERROR: VirtualAlloc for queue trampoline failed.");
            return false;
        }

        std::memcpy(trampoline, target, sizeof(kExpectedPrologue));
        if (!MakeRelativeJump(trampoline + sizeof(kExpectedPrologue), target + sizeof(kExpectedPrologue), trampoline + sizeof(kExpectedPrologue)))
        {
            VirtualFree(trampoline, 0, MEM_RELEASE);
            AppendQueueLog("ERROR: cannot encode trampoline return jump.");
            return false;
        }
        FlushInstructionCache(GetCurrentProcess(), trampoline, sizeof(kExpectedPrologue) + 5);

        BYTE patch[sizeof(kExpectedPrologue)]{};
        if (!MakeRelativeJump(target, reinterpret_cast<const void*>(&QueueInsertHook), patch))
        {
            VirtualFree(trampoline, 0, MEM_RELEASE);
            AppendQueueLog("ERROR: cannot encode hook jump; target is outside rel32 range.");
            return false;
        }
        patch[5] = 0x90;

        HANDLE suspended[512]{};
        const DWORD suspendedCapacity = static_cast<DWORD>(sizeof(suspended) / sizeof(suspended[0]));
        const DWORD suspendedCount = SuspendOtherProcessThreads(suspended, suspendedCapacity);

        DWORD oldProtect = 0;
        if (!VirtualProtect(target, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            ResumeThreads(suspended, suspendedCount);
            VirtualFree(trampoline, 0, MEM_RELEASE);
            AppendQueueLog("ERROR: VirtualProtect failed while installing queue hook.");
            return false;
        }

        if (std::memcmp(target, kExpectedPrologue, sizeof(kExpectedPrologue)) != 0)
        {
            DWORD ignored = 0;
            VirtualProtect(target, sizeof(patch), oldProtect, &ignored);
            ResumeThreads(suspended, suspendedCount);
            VirtualFree(trampoline, 0, MEM_RELEASE);
            AppendQueueLog("ERROR: queue function changed before patching; hook was not installed.");
            return false;
        }

        g_hookTarget = target;
        g_trampoline = trampoline;
        g_originalQueueInsert = reinterpret_cast<QueueInsertFn>(trampoline);
        std::memcpy(target, patch, sizeof(patch));
        FlushInstructionCache(GetCurrentProcess(), target, sizeof(patch));
        InterlockedExchange(&g_hookInstalled, 1);

        DWORD ignored = 0;
        VirtualProtect(target, sizeof(patch), oldProtect, &ignored);
        ResumeThreads(suspended, suspendedCount);

        char message[512]{};
        sprintf_s(message,
            "CPU queue trace hook installed at RVA=0x%08lX; P1 queue=0x%08lX, P2 queue=0x%08lX; suspended %lu other process thread(s) during patch. Original function is preserved through a trampoline.",
            static_cast<unsigned long>(kQueueInsertRva),
            static_cast<unsigned long>(g_p1Queue),
            static_cast<unsigned long>(g_p2Queue),
            static_cast<unsigned long>(suspendedCount));
        AppendQueueLog(message);
        return true;
    }

    bool InstallEventDispatchHook()
    {
        BYTE* target = reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase + kEventDispatchRva));
        if (target == nullptr)
        {
            AppendDispatchLog("ERROR: event dispatch target address is null.");
            return false;
        }

        if (std::memcmp(target, kEventDispatchPrologue, sizeof(kEventDispatchPrologue)) != 0)
        {
            AppendDispatchLog("ERROR: AALib.exe+0xEE200 prologue does not match 55 8B EC 53 8B 5D 0C; hook was not installed.");
            return false;
        }

        BYTE* trampoline = reinterpret_cast<BYTE*>(VirtualAlloc(
            nullptr, sizeof(kEventDispatchPrologue) + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
        if (trampoline == nullptr)
        {
            AppendDispatchLog("ERROR: VirtualAlloc for event dispatch trampoline failed.");
            return false;
        }

        std::memcpy(trampoline, target, sizeof(kEventDispatchPrologue));
        if (!MakeRelativeJump(trampoline + sizeof(kEventDispatchPrologue),
            target + sizeof(kEventDispatchPrologue),
            trampoline + sizeof(kEventDispatchPrologue)))
        {
            VirtualFree(trampoline, 0, MEM_RELEASE);
            AppendDispatchLog("ERROR: cannot encode event dispatch trampoline return jump.");
            return false;
        }
        FlushInstructionCache(GetCurrentProcess(), trampoline, sizeof(kEventDispatchPrologue) + 5);

        BYTE patch[sizeof(kEventDispatchPrologue)]{};
        if (!MakeRelativeJump(target, reinterpret_cast<const void*>(&EventDispatchHook), patch))
        {
            VirtualFree(trampoline, 0, MEM_RELEASE);
            AppendDispatchLog("ERROR: cannot encode event dispatch hook jump; target is outside rel32 range.");
            return false;
        }
        for (std::size_t i = 5; i < sizeof(patch); ++i) patch[i] = 0x90;

        HANDLE suspended[512]{};
        const DWORD suspendedCapacity = static_cast<DWORD>(sizeof(suspended) / sizeof(suspended[0]));
        const DWORD suspendedCount = SuspendOtherProcessThreads(suspended, suspendedCapacity);

        DWORD oldProtect = 0;
        if (!VirtualProtect(target, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            ResumeThreads(suspended, suspendedCount);
            VirtualFree(trampoline, 0, MEM_RELEASE);
            AppendDispatchLog("ERROR: VirtualProtect failed while installing event dispatch hook.");
            return false;
        }

        if (std::memcmp(target, kEventDispatchPrologue, sizeof(kEventDispatchPrologue)) != 0)
        {
            DWORD ignored = 0;
            VirtualProtect(target, sizeof(patch), oldProtect, &ignored);
            ResumeThreads(suspended, suspendedCount);
            VirtualFree(trampoline, 0, MEM_RELEASE);
            AppendDispatchLog("ERROR: event dispatch function changed before patching; hook was not installed.");
            return false;
        }

        g_dispatchHookTarget = target;
        g_dispatchTrampoline = trampoline;
        g_originalEventDispatch = reinterpret_cast<EventDispatchFn>(trampoline);
        std::memcpy(target, patch, sizeof(patch));
        FlushInstructionCache(GetCurrentProcess(), target, sizeof(patch));
        InterlockedExchange(&g_dispatchHookInstalled, 1);

        DWORD ignored = 0;
        VirtualProtect(target, sizeof(patch), oldProtect, &ignored);
        ResumeThreads(suspended, suspendedCount);

        char message[512]{};
        sprintf_s(message,
            "CPU event dispatch hook installed at RVA=0x%08lX; P1 object=0x%08lX, P2 object=0x%08lX; suspended %lu other process thread(s) during patch. Original __thiscall function is preserved through a 7-byte trampoline.",
            static_cast<unsigned long>(kEventDispatchRva),
            static_cast<unsigned long>(g_playerP1),
            static_cast<unsigned long>(g_playerP2),
            static_cast<unsigned long>(suspendedCount));
        AppendDispatchLog(message);
        return true;
    }

    bool InstallStage21PayloadConsumerHooks()
    {
        BYTE* classifierCall = reinterpret_cast<BYTE*>(
            static_cast<std::uintptr_t>(g_gameBase + kStage21PayloadClassifierCallRva));
        BYTE* slotClearCall = reinterpret_cast<BYTE*>(
            static_cast<std::uintptr_t>(g_gameBase + kStage21PayloadSlotClearCallRva));

        if (std::memcmp(classifierCall - sizeof(kStage21ClassifierPrefix),
                kStage21ClassifierPrefix, sizeof(kStage21ClassifierPrefix)) != 0 ||
            std::memcmp(classifierCall, kStage21ClassifierCall, sizeof(kStage21ClassifierCall)) != 0 ||
            std::memcmp(classifierCall + sizeof(kStage21ClassifierCall),
                kStage21ClassifierSuffix, sizeof(kStage21ClassifierSuffix)) != 0)
        {
            AppendStage21ConsumerLog("ERROR: classifier callsite RVA 0x000EE426 signature mismatch; no Stage21 patch was applied.");
            return false;
        }

        if (std::memcmp(slotClearCall - sizeof(kStage21SlotClearPrefix),
                kStage21SlotClearPrefix, sizeof(kStage21SlotClearPrefix)) != 0 ||
            std::memcmp(slotClearCall, kStage21SlotClearCall, sizeof(kStage21SlotClearCall)) != 0)
        {
            AppendStage21ConsumerLog("ERROR: fallback slot-clear callsite RVA 0x000EE446 signature mismatch; no Stage21 patch was applied.");
            return false;
        }

        std::int32_t classifierRelative = 0;
        std::memcpy(&classifierRelative, classifierCall + 1, sizeof(classifierRelative));
        const std::intptr_t classifierDestination =
            reinterpret_cast<std::intptr_t>(classifierCall + 5) + classifierRelative;
        if (classifierDestination != static_cast<std::intptr_t>(g_gameBase + kStage21PayloadClassifierRva))
        {
            AppendStage21ConsumerLog("ERROR: RVA 0x000EE426 does not resolve to helper RVA 0x000E8CF0; refusing to patch.");
            return false;
        }

        std::int32_t slotRelative = 0;
        std::memcpy(&slotRelative, slotClearCall + 1, sizeof(slotRelative));
        const std::intptr_t slotDestination =
            reinterpret_cast<std::intptr_t>(slotClearCall + 5) + slotRelative;
        if (slotDestination != static_cast<std::intptr_t>(g_gameBase + kStage21PayloadSlotClearRva))
        {
            AppendStage21ConsumerLog("ERROR: RVA 0x000EE446 does not resolve to helper RVA 0x000F7EB0; refusing to patch.");
            return false;
        }

        BYTE classifierPatch[5]{};
        BYTE slotClearPatch[5]{};
        if (!MakeRelativeCall(classifierCall,
                reinterpret_cast<const void*>(&Stage21PayloadClassifierHook), classifierPatch) ||
            !MakeRelativeCall(slotClearCall,
                reinterpret_cast<const void*>(&Stage21PayloadSlotClearHook), slotClearPatch))
        {
            AppendStage21ConsumerLog("ERROR: Stage21 wrapper is outside rel32 CALL range; no patch was applied.");
            return false;
        }

        HANDLE patchMutex = CreateMutexW(nullptr, FALSE, L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
        if (patchMutex == nullptr)
        {
            AppendStage21ConsumerLog("ERROR: could not create shared patch-install mutex.");
            return false;
        }
        const DWORD wait = WaitForSingleObject(patchMutex, INFINITE);
        if (wait != WAIT_OBJECT_0 && wait != WAIT_ABANDONED)
        {
            CloseHandle(patchMutex);
            AppendStage21ConsumerLog("ERROR: could not acquire shared patch-install mutex.");
            return false;
        }

        HANDLE suspended[512]{};
        const DWORD suspendedCount = SuspendOtherProcessThreads(
            suspended, static_cast<DWORD>(sizeof(suspended) / sizeof(suspended[0])));

        DWORD oldProtectClassifier = 0;
        DWORD oldProtectSlot = 0;
        bool classifierProtected = false;
        bool slotProtected = false;
        bool ok = false;

        if (!VirtualProtect(classifierCall, sizeof(classifierPatch), PAGE_EXECUTE_READWRITE,
                &oldProtectClassifier))
        {
            AppendStage21ConsumerLog("ERROR: VirtualProtect failed for RVA 0x000EE426.");
        }
        else
        {
            classifierProtected = true;
            if (!VirtualProtect(slotClearCall, sizeof(slotClearPatch), PAGE_EXECUTE_READWRITE,
                    &oldProtectSlot))
            {
                AppendStage21ConsumerLog("ERROR: VirtualProtect failed for RVA 0x000EE446; neither callsite was patched.");
            }
            else
            {
                slotProtected = true;
                if (std::memcmp(classifierCall, kStage21ClassifierCall, sizeof(kStage21ClassifierCall)) != 0 ||
                    std::memcmp(slotClearCall, kStage21SlotClearCall, sizeof(kStage21SlotClearCall)) != 0)
                {
                    AppendStage21ConsumerLog("ERROR: Stage21 callsite bytes changed during installation; neither callsite was patched.");
                }
                else
                {
                    g_stage21PayloadClassifierTarget = g_gameBase + kStage21PayloadClassifierRva;
                    g_stage21PayloadSlotClearTarget = g_gameBase + kStage21PayloadSlotClearRva;
                    std::memcpy(classifierCall, classifierPatch, sizeof(classifierPatch));
                    std::memcpy(slotClearCall, slotClearPatch, sizeof(slotClearPatch));
                    FlushInstructionCache(GetCurrentProcess(), classifierCall, sizeof(classifierPatch));
                    FlushInstructionCache(GetCurrentProcess(), slotClearCall, sizeof(slotClearPatch));
                    InterlockedExchange(&g_stage21PayloadConsumerInstalled, 1);
                    ok = true;
                }
            }
        }

        if (slotProtected)
        {
            DWORD ignored = 0;
            VirtualProtect(slotClearCall, sizeof(slotClearPatch), oldProtectSlot, &ignored);
        }
        if (classifierProtected)
        {
            DWORD ignored = 0;
            VirtualProtect(classifierCall, sizeof(classifierPatch), oldProtectClassifier, &ignored);
        }

        ResumeThreads(suspended, suspendedCount);
        ReleaseMutex(patchMutex);
        CloseHandle(patchMutex);

        if (ok)
        {
            char message[512]{};
            sprintf_s(message,
                "Installed both Stage21 observational CALL wrappers: classifier RVA=0x%08lX -> helper=0x%08lX; fallback slot-clear RVA=0x%08lX -> helper=0x%08lX; suspended=%lu thread(s). Original helper calls and caller stack cleanup are preserved.",
                static_cast<unsigned long>(kStage21PayloadClassifierCallRva),
                static_cast<unsigned long>(kStage21PayloadClassifierRva),
                static_cast<unsigned long>(kStage21PayloadSlotClearCallRva),
                static_cast<unsigned long>(kStage21PayloadSlotClearRva),
                static_cast<unsigned long>(suspendedCount));
            AppendStage21ConsumerLog(message);
            return true;
        }
        return false;
    }

    bool InstallStage22PayloadPostAndHook()
    {
        BYTE* target = reinterpret_cast<BYTE*>(
            static_cast<std::uintptr_t>(g_gameBase + kStage22ControllerFlagsAndRva));

        if (std::memcmp(target - sizeof(kStage22ControllerFlagsAndPrefix),
                kStage22ControllerFlagsAndPrefix, sizeof(kStage22ControllerFlagsAndPrefix)) != 0 ||
            std::memcmp(target, kStage22ControllerFlagsAndInstruction,
                sizeof(kStage22ControllerFlagsAndInstruction)) != 0)
        {
            AppendStage22PayloadLog("ERROR: controller bit-0x400 AND signature mismatch at RVA 0x000EE451; no Stage22 patch was applied.");
            return false;
        }

        BYTE patch[7] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        if (!MakeRelativeCall(target,
                reinterpret_cast<const void*>(&Stage22ControllerFlagsAndHook), patch))
        {
            AppendStage22PayloadLog("ERROR: Stage22 wrapper is outside rel32 CALL range; no patch was applied.");
            return false;
        }

        HANDLE patchMutex = CreateMutexW(nullptr, FALSE, L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
        if (patchMutex == nullptr)
        {
            AppendStage22PayloadLog("ERROR: could not create shared patch-install mutex.");
            return false;
        }
        const DWORD wait = WaitForSingleObject(patchMutex, INFINITE);
        if (wait != WAIT_OBJECT_0 && wait != WAIT_ABANDONED)
        {
            CloseHandle(patchMutex);
            AppendStage22PayloadLog("ERROR: could not acquire shared patch-install mutex.");
            return false;
        }

        HANDLE suspended[512]{};
        const DWORD suspendedCount = SuspendOtherProcessThreads(
            suspended, static_cast<DWORD>(sizeof(suspended) / sizeof(suspended[0])));

        DWORD oldProtect = 0;
        bool protectedTarget = false;
        bool ok = false;
        if (!VirtualProtect(target, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            AppendStage22PayloadLog("ERROR: VirtualProtect failed for RVA 0x000EE451.");
        }
        else
        {
            protectedTarget = true;
            if (std::memcmp(target - sizeof(kStage22ControllerFlagsAndPrefix),
                    kStage22ControllerFlagsAndPrefix, sizeof(kStage22ControllerFlagsAndPrefix)) != 0 ||
                std::memcmp(target, kStage22ControllerFlagsAndInstruction,
                    sizeof(kStage22ControllerFlagsAndInstruction)) != 0)
            {
                AppendStage22PayloadLog("ERROR: Stage22 AND bytes changed during installation; no patch was applied.");
            }
            else
            {
                // The CALL is 5 bytes; the two remaining bytes of the original AND are NOPed.
                std::memcpy(target, patch, sizeof(patch));
                FlushInstructionCache(GetCurrentProcess(), target, sizeof(patch));
                InterlockedExchange(&g_stage22PayloadPostAndInstalled, 1);
                ok = true;
            }
        }

        if (protectedTarget)
        {
            DWORD ignored = 0;
            VirtualProtect(target, sizeof(patch), oldProtect, &ignored);
        }
        ResumeThreads(suspended, suspendedCount);
        ReleaseMutex(patchMutex);
        CloseHandle(patchMutex);

        if (ok)
        {
            AppendStage22PayloadLog(
                "Installed Stage22 post-AND observer at RVA 0x000EE451: validated 7-byte instruction replaced by CALL + 2 NOPs; wrapper executes the identical AND once and restores post-AND EFLAGS.");
            return true;
        }
        return false;
    }

    void FlushQueueSites()
    {
        for (DWORD i = 0; i < kSiteCapacity; ++i)
        {
            QueueSite& site = g_sites[i];
            if (InterlockedCompareExchange(&site.State, 0, 0) != 1)
            {
                continue;
            }

            const LONG count = InterlockedCompareExchange(&site.Count, 0, 0);
            const LONG reported = InterlockedCompareExchange(&site.ReportedCount, count, 0);
            if (reported == count)
            {
                continue;
            }
            InterlockedExchange(&site.ReportedCount, count);

            char message[768]{};
            sprintf_s(message,
                "QUEUE_EVENT result=%s side=P%lu callerRVA=0x%08lX tid(first)=%lu eventTypeArg=0x%08lX payloadArg=0x%08lX guardBefore=0x%02lX guardAfter=0x%02lX activeBefore=0x%02lX activeAfter=0x%02lX storedTypeAfter=0x%04lX storedPayloadAfter=0x%04lX totalHits=%ld",
                site.GuardBefore == 0 ? "ACCEPTED_BY_QUEUE" : "BLOCKED_BY_GUARD",
                static_cast<unsigned long>(site.Side),
                static_cast<unsigned long>(site.CallerRva),
                static_cast<unsigned long>(site.FirstThreadId),
                static_cast<unsigned long>(site.EventTypeArg),
                static_cast<unsigned long>(site.PayloadArg),
                static_cast<unsigned long>(site.GuardBefore),
                static_cast<unsigned long>(site.GuardAfter),
                static_cast<unsigned long>(site.ActiveBefore),
                static_cast<unsigned long>(site.ActiveAfter),
                static_cast<unsigned long>(site.StoredTypeAfter),
                static_cast<unsigned long>(site.StoredPayloadAfter),
                static_cast<long>(count));
            AppendQueueLog(message);
        }
    }

    void FlushQueueEventDetails()
    {
        for (;;)
        {
            QueueEventDetailRecord record{};
            AcquireSRWLockExclusive(&g_detailLock);
            if (g_detailCount == 0)
            {
                ReleaseSRWLockExclusive(&g_detailLock);
                break;
            }
            record = g_detailRecords[g_detailReadIndex];
            g_detailReadIndex = (g_detailReadIndex + 1u) % kDetailCapacity;
            --g_detailCount;
            ReleaseSRWLockExclusive(&g_detailLock);

            // The queue slot is released before string formatting and file I/O.
            char message[1800]{};
            int used = sprintf_s(message, sizeof(message),
                "QUEUE_EVENT_DETAIL seq=%lu qpc=%I64u side=P%lu tid=%lu tick=%lu caller=0x%08lX callerRVA=0x%08lX player=0x%08lX queue=0x%08lX eventType=0x%08lX payload=0x%08lX guardBefore=0x%02lX activeBefore=0x%02lX afterReadable=%lu guardAfter=0x%02lX activeAfter=0x%02lX storedAfter=(%04lX,%04lX) stackDepth=%lu rtlStack=[",
                static_cast<unsigned long>(record.Sequence),
                static_cast<unsigned long long>(record.QpcTicks),
                static_cast<unsigned long>(record.Side),
                static_cast<unsigned long>(record.ThreadId),
                static_cast<unsigned long>(record.TickCount),
                static_cast<unsigned long>(record.CallerAddress),
                static_cast<unsigned long>(record.CallerRva),
                static_cast<unsigned long>(record.PlayerObject),
                static_cast<unsigned long>(record.QueueAddress),
                static_cast<unsigned long>(record.EventType),
                static_cast<unsigned long>(record.Payload),
                static_cast<unsigned long>(record.GuardBefore),
                static_cast<unsigned long>(record.ActiveBefore),
                static_cast<unsigned long>(record.AfterReadable),
                static_cast<unsigned long>(record.GuardAfter),
                static_cast<unsigned long>(record.ActiveAfter),
                static_cast<unsigned long>(record.StoredTypeAfter),
                static_cast<unsigned long>(record.StoredPayloadAfter),
                static_cast<unsigned long>(record.StackDepth));
            if (used > 0)
            {
                for (DWORD i = 0; i < record.StackDepth &&
                    static_cast<std::size_t>(used) < sizeof(message) - 20; ++i)
                {
                    const DWORD address = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(record.Stack[i]));
                    const int written = sprintf_s(message + used,
                        sizeof(message) - static_cast<std::size_t>(used),
                        "%s0x%08lX", i == 0 ? "" : ",", static_cast<unsigned long>(address));
                    if (written <= 0) break;
                    used += written;
                }
                if (static_cast<std::size_t>(used) < sizeof(message) - 2)
                    used += sprintf_s(message + used, sizeof(message) - static_cast<std::size_t>(used),
                        "] hookEBP=0x%08lX hookESP=0x%08lX frameDepth=%lu frame0MatchesCaller=%lu frameChain=[",
                        static_cast<unsigned long>(record.HookEbp),
                        static_cast<unsigned long>(record.HookEsp),
                        static_cast<unsigned long>(record.FrameDepth),
                        static_cast<unsigned long>(record.FrameZeroMatchesCaller));
                for (DWORD i = 0; i < record.FrameDepth &&
                    static_cast<std::size_t>(used) < sizeof(message) - 40; ++i)
                {
                    const int written = sprintf_s(message + used,
                        sizeof(message) - static_cast<std::size_t>(used),
                        "%s0x%08lX:0x%08lX", i == 0 ? "" : ",",
                        static_cast<unsigned long>(record.FramePointers[i]),
                        static_cast<unsigned long>(record.ReturnAddresses[i]));
                    if (written <= 0) break;
                    used += written;
                }
                if (used > 0 && static_cast<std::size_t>(used) < sizeof(message) - 2)
                    sprintf_s(message + used, sizeof(message) - static_cast<std::size_t>(used), "]");
                AppendQueueLog(message);
            }
        }
    }

    void FlushDispatchSites()
    {
        for (DWORD i = 0; i < kSiteCapacity; ++i)
        {
            EventDispatchSite& site = g_dispatchSites[i];
            if (InterlockedCompareExchange(&site.State, 0, 0) != 1) continue;

            const LONG count = InterlockedCompareExchange(&site.Count, 0, 0);
            const LONG reported = InterlockedCompareExchange(&site.ReportedCount, count, 0);
            if (reported == count) continue;
            InterlockedExchange(&site.ReportedCount, count);

            char message[1100]{};
            sprintf_s(message,
                "EVENT_DISPATCH side=P%lu callerRVA=0x%08lX tid(first)=%lu eventTypeArg=0x%08lX payloadArg=0x%08lX playerMode=0x%08lX queueActiveBefore=0x%02lX queueCurrentBefore=(%04lX,%04lX) queueHistoryBefore=(%04lX,%04lX) queueHistoryMatchesArgs=%lu guardBefore=0x%02lX queueActiveAfter=0x%02lX queueHistoryAfter=(%04lX,%04lX) totalHits=%ld",
                static_cast<unsigned long>(site.Side),
                static_cast<unsigned long>(site.CallerRva),
                static_cast<unsigned long>(site.FirstThreadId),
                static_cast<unsigned long>(site.EventTypeArg),
                static_cast<unsigned long>(site.PayloadArg),
                static_cast<unsigned long>(site.PlayerMode),
                static_cast<unsigned long>(site.QueueActiveBefore),
                static_cast<unsigned long>(site.QueueCurrentTypeBefore),
                static_cast<unsigned long>(site.QueueCurrentPayloadBefore),
                static_cast<unsigned long>(site.QueueHistoryTypeBefore),
                static_cast<unsigned long>(site.QueueHistoryPayloadBefore),
                static_cast<unsigned long>(site.QueueHistoryMatchesArgs),
                static_cast<unsigned long>(site.QueueGuardBefore),
                static_cast<unsigned long>(site.QueueActiveAfter),
                static_cast<unsigned long>(site.QueueHistoryTypeAfter),
                static_cast<unsigned long>(site.QueueHistoryPayloadAfter),
                static_cast<long>(count));
            AppendDispatchLog(message);
        }
    }

    void FlushEventDispatchDetails()
    {
        for (;;)
        {
            EventDispatchDetailRecord record{};
            AcquireSRWLockExclusive(&g_dispatchDetailLock);
            if (g_dispatchDetailCount == 0)
            {
                ReleaseSRWLockExclusive(&g_dispatchDetailLock);
                break;
            }
            record = g_dispatchDetailRecords[g_dispatchDetailReadIndex];
            g_dispatchDetailReadIndex = (g_dispatchDetailReadIndex + 1u) % kDispatchDetailCapacity;
            --g_dispatchDetailCount;
            ReleaseSRWLockExclusive(&g_dispatchDetailLock);

            const QueueSnapshot& before = record.Before;
            const QueueSnapshot& after = record.After;
            const DWORD beforeMatches =
                (before.field8 == static_cast<WORD>(record.EventType) &&
                 before.field10 == static_cast<WORD>(record.Payload)) ? 1u : 0u;
            const DWORD afterMatches =
                (record.AfterReadable != 0 &&
                 after.field8 == static_cast<WORD>(record.EventType) &&
                 after.field10 == static_cast<WORD>(record.Payload)) ? 1u : 0u;

            char message[1400]{};
            sprintf_s(message,
                "EVENT_DISPATCH_DETAIL seq=%lu qpcBefore=%I64u qpcAfter=%I64u side=P%lu tid=%lu tickBefore=%lu tickAfter=%lu caller=0x%08lX callerRVA=0x%08lX player=0x%08lX queue=0x%08lX eventType=0x%08lX payload=0x%08lX playerMode=0x%08lX before=(active:%02lX current:%04lX,%04lX history:%04lX,%04lX guard:%02lX) beforeMatches=%lu afterReadable=%lu after=(active:%02lX current:%04lX,%04lX history:%04lX,%04lX guard:%02lX) afterMatches=%lu",
                static_cast<unsigned long>(record.Sequence),
                static_cast<unsigned long long>(record.QpcBefore),
                static_cast<unsigned long long>(record.QpcAfter),
                static_cast<unsigned long>(record.Side),
                static_cast<unsigned long>(record.ThreadId),
                static_cast<unsigned long>(record.TickBefore),
                static_cast<unsigned long>(record.TickAfter),
                static_cast<unsigned long>(record.CallerAddress),
                static_cast<unsigned long>(record.CallerRva),
                static_cast<unsigned long>(record.PlayerObject),
                static_cast<unsigned long>(record.QueueAddress),
                static_cast<unsigned long>(record.EventType),
                static_cast<unsigned long>(record.Payload),
                static_cast<unsigned long>(record.PlayerMode),
                static_cast<unsigned long>(before.active),
                static_cast<unsigned long>(before.storedType),
                static_cast<unsigned long>(before.storedPayload),
                static_cast<unsigned long>(before.field8),
                static_cast<unsigned long>(before.field10),
                static_cast<unsigned long>(before.guard),
                static_cast<unsigned long>(beforeMatches),
                static_cast<unsigned long>(record.AfterReadable),
                static_cast<unsigned long>(after.active),
                static_cast<unsigned long>(after.storedType),
                static_cast<unsigned long>(after.storedPayload),
                static_cast<unsigned long>(after.field8),
                static_cast<unsigned long>(after.field10),
                static_cast<unsigned long>(after.guard),
                static_cast<unsigned long>(afterMatches));
            AppendDispatchLog(message);
        }
    }

    DWORD WINAPI QueueTraceWorker(void*)
    {
        const int queueTraceEnabled = ReadIniInt(L"AIResearch", L"EnableCpuQueueTrace", 0);
        const int queueDetailTraceEnabled = ReadIniInt(L"AIResearch", L"EnableCpuQueueDetailTrace", 1);
        const int dispatchTraceEnabled = ReadIniInt(L"AIResearch", L"EnableCpuEventDispatchTrace", 0);
        const int dispatchDetailTraceEnabled = ReadIniInt(L"AIResearch", L"EnableCpuEventDispatchDetailTrace", 1);
        const int stage21ConsumerTraceEnabled = ReadIniInt(L"AIResearch", L"EnableStage21PayloadConsumerTrace", 0);
        const int stage22PostAndTraceEnabled = ReadIniInt(L"AIResearch", L"EnableStage22PayloadPostAndTrace", 0);
        InterlockedExchange(&g_detailTraceEnabled, queueDetailTraceEnabled != 0 ? 1 : 0);
        InterlockedExchange(&g_dispatchDetailTraceEnabled, dispatchDetailTraceEnabled != 0 ? 1 : 0);
        int flushMs = ReadIniInt(L"AIResearch", L"QueueTraceFlushIntervalMs", kDefaultFlushIntervalMs);
        if (flushMs < static_cast<int>(kMinFlushIntervalMs)) flushMs = kMinFlushIntervalMs;
        if (flushMs > static_cast<int>(kMaxFlushIntervalMs)) flushMs = kMaxFlushIntervalMs;
        g_flushIntervalMs = static_cast<DWORD>(flushMs);

        if (stage21ConsumerTraceEnabled != 0)
            AppendStage21ConsumerLog("STAGE21_PAYLOAD_1E_CONSUMER_BUILD=2026-11-08-r2");
        if (stage22PostAndTraceEnabled != 0)
            AppendStage22PayloadLog("STAGE22_PAYLOAD_POST_AND_BUILD=2026-11-08-r1");
        if (queueTraceEnabled == 0 && dispatchTraceEnabled == 0 &&
            stage21ConsumerTraceEnabled == 0 && stage22PostAndTraceEnabled == 0)
        {
            AppendQueueLog("CPU queue and event-dispatch traces are disabled in ArcanaAI.ini. No code hook was installed.");
            return 0;
        }

        for (int attempt = 0; attempt < 300; ++attempt)
        {
            g_gameModule = GetModuleHandleW(L"AALib.exe");
            if (g_gameModule != nullptr) break;
            Sleep(100);
        }
        if (g_gameModule == nullptr)
        {
            AppendQueueLog("ERROR: AALib.exe was not found; CPU queue trace stopped.");
            return 0;
        }

        g_gameBase = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(g_gameModule));
        DWORD timestamp = 0;
        if (!GetImageFingerprint32(g_gameModule, &g_gameImageSize, &timestamp) ||
            g_gameImageSize != kExpectedImageSize || timestamp != kExpectedTimeDateStamp ||
            kQueueInsertRva + sizeof(kExpectedPrologue) >= g_gameImageSize ||
            kEventDispatchRva + sizeof(kEventDispatchPrologue) >= g_gameImageSize ||
            kP1ControlRva < kPlayerControlOffset || kP2ControlRva < kPlayerControlOffset)
        {
            AppendQueueLog("ERROR: AALib.exe PE fingerprint does not match the analyzed binary; no code hook was installed.");
            return 0;
        }

        g_playerP1 = g_gameBase + kP1ControlRva - kPlayerControlOffset;
        g_playerP2 = g_gameBase + kP2ControlRva - kPlayerControlOffset;
        g_p1Queue = g_playerP1 + kPlayerQueueOffset;
        g_p2Queue = g_playerP2 + kPlayerQueueOffset;

        char start[720]{};
        sprintf_s(start,
            "Stage 12/21 tracing requested. gameBase=0x%08lX; P1 object=0x%08lX queue=0x%08lX; P2 object=0x%08lX queue=0x%08lX; queueInsert=%d; queueDetail=%d; eventDispatch=%d; dispatchDetail=%d; stage21Consumer=%d; stage22PostAnd=%d; flush=%lu ms. Exact P1/P2 CPU queues only.",
            static_cast<unsigned long>(g_gameBase),
            static_cast<unsigned long>(g_playerP1),
            static_cast<unsigned long>(g_p1Queue),
            static_cast<unsigned long>(g_playerP2),
            static_cast<unsigned long>(g_p2Queue),
            queueTraceEnabled != 0 ? 1 : 0,
            queueDetailTraceEnabled != 0 ? 1 : 0,
            dispatchTraceEnabled != 0 ? 1 : 0,
            dispatchDetailTraceEnabled != 0 ? 1 : 0,
            stage21ConsumerTraceEnabled != 0 ? 1 : 0,
            stage22PostAndTraceEnabled != 0 ? 1 : 0,
            static_cast<unsigned long>(g_flushIntervalMs));
        AppendQueueLog(start);
        AppendDispatchLog(start);

        if (queueTraceEnabled != 0 && !InstallQueueHook())
        {
            AppendDispatchLog("Queue insertion hook failed; event dispatch installation will still be attempted if enabled.");
        }
        if (dispatchTraceEnabled != 0 && !InstallEventDispatchHook())
        {
            AppendQueueLog("Event dispatch hook installation failed; queue insertion trace may still be active.");
        }
        if (stage21ConsumerTraceEnabled != 0 && !InstallStage21PayloadConsumerHooks())
        {
            AppendStage21ConsumerLog("ERROR: Stage21 consumer hooks were not installed; inspect the preceding validation/patch message.");
        }
        if (stage22PostAndTraceEnabled != 0 && !InstallStage22PayloadPostAndHook())
        {
            AppendStage22PayloadLog("ERROR: Stage22 post-AND observer was not installed; inspect the preceding validation/patch message.");
        }
        if (InterlockedCompareExchange(&g_hookInstalled, 0, 0) == 0 &&
            InterlockedCompareExchange(&g_dispatchHookInstalled, 0, 0) == 0 &&
            InterlockedCompareExchange(&g_stage21PayloadConsumerInstalled, 0, 0) == 0 &&
            InterlockedCompareExchange(&g_stage22PayloadPostAndInstalled, 0, 0) == 0)
        {
            AppendQueueLog("ERROR: no Stage 5 trace hook was installed.");
            return 0;
        }

        for (;;)
        {
            Sleep(g_flushIntervalMs);
            if (queueTraceEnabled != 0 && InterlockedCompareExchange(&g_hookInstalled, 0, 0) != 0)
            {
                FlushQueueSites();
                if (queueDetailTraceEnabled != 0)
                    FlushQueueEventDetails();
                if (InterlockedCompareExchange(&g_detailDroppedRecords, 0, 0) != 0)
                {
                    char detailWarning[160]{};
                    sprintf_s(detailWarning, "WARNING: queue detail buffer full; dropped records=%ld (capacity=%lu).",
                        static_cast<long>(InterlockedCompareExchange(&g_detailDroppedRecords, 0, 0)),
                        static_cast<unsigned long>(kDetailCapacity));
                    AppendQueueLog(detailWarning);
                    InterlockedExchange(&g_detailDroppedRecords, 0);
                }
                if (InterlockedCompareExchange(&g_droppedSites, 0, 0) != 0)
                {
                    char warning[160]{};
                    sprintf_s(warning, "WARNING: queue trace site table full; dropped groups=%ld.",
                        static_cast<long>(InterlockedCompareExchange(&g_droppedSites, 0, 0)));
                    AppendQueueLog(warning);
                    InterlockedExchange(&g_droppedSites, 0);
                }
            }
            if (dispatchTraceEnabled != 0 && InterlockedCompareExchange(&g_dispatchHookInstalled, 0, 0) != 0)
            {
                FlushDispatchSites();
                if (dispatchDetailTraceEnabled != 0)
                    FlushEventDispatchDetails();
                if (InterlockedCompareExchange(&g_dispatchDetailDroppedRecords, 0, 0) != 0)
                {
                    char detailWarning[180]{};
                    sprintf_s(detailWarning, "WARNING: event dispatch detail buffer full; dropped records=%ld (capacity=%lu).",
                        static_cast<long>(InterlockedCompareExchange(&g_dispatchDetailDroppedRecords, 0, 0)),
                        static_cast<unsigned long>(kDispatchDetailCapacity));
                    AppendDispatchLog(detailWarning);
                    InterlockedExchange(&g_dispatchDetailDroppedRecords, 0);
                }
                if (InterlockedCompareExchange(&g_dispatchDroppedSites, 0, 0) != 0)
                {
                    char warning[160]{};
                    sprintf_s(warning, "WARNING: event dispatch trace site table full; dropped groups=%ld.",
                        static_cast<long>(InterlockedCompareExchange(&g_dispatchDroppedSites, 0, 0)));
                    AppendDispatchLog(warning);
                    InterlockedExchange(&g_dispatchDroppedSites, 0);
                }
            }
        }
    }

    BOOL CALLBACK StartQueueTraceWorker(PINIT_ONCE, PVOID, PVOID*)
    {
        HANDLE thread = CreateThread(nullptr, 0, QueueTraceWorker, nullptr, 0, nullptr);
        if (thread == nullptr)
        {
            AppendQueueLog("ERROR: failed to create CPU queue trace worker thread.");
            return TRUE;
        }
        CloseHandle(thread);
        return TRUE;
    }
}

extern "C" void __cdecl ArcanaLogStage21PayloadClassifier(const DWORD* savedContext)
{
    if (savedContext == nullptr) return;

    const DWORD player = savedContext[1];        // ESI
    const DWORD payload = savedContext[4];       // EBX
    const DWORD resultEax = savedContext[7];     // EAX after the real helper
    if (payload != kStage21Payload1E) return;

    DWORD side = 0;
    if (player == g_playerP1) side = 1;
    else if (player == g_playerP2) side = 2;
    else return;

    const DWORD dispatcherEbp = savedContext[2];     // EBP of dispatcher 0xEE200
    const DWORD savedEspAfterPushfd = savedContext[3];
    DWORD decodedActionCode = 0, returnAddress = 0, helperArg = 0;
    DWORD dispatchType = 0, dispatchPayload = 0;
    // The wrapper's private saved-EDI DWORD is now at savedESP+4.
    SafeReadDword(savedEspAfterPushfd + 4u, &decodedActionCode);
    SafeReadDword(savedEspAfterPushfd + 8u, &returnAddress);
    SafeReadDword(savedEspAfterPushfd + 12u, &helperArg);
    SafeReadDword(dispatcherEbp + 8u, &dispatchType);
    SafeReadDword(dispatcherEbp + 0x0Cu, &dispatchPayload);
    if (dispatchType != 1u || dispatchPayload != kStage21Payload1E ||
        payload != dispatchPayload || helperArg != 1u)
        return;
    // decodedActionCode was captured from EDI before the real classifier call.

    DWORD playerMode = 0, controlState = 0, controller = 0, controllerFlags = 0;
    SafeReadDword(player + kPlayerControlOffset, &playerMode);
    SafeReadDword(player + 0x3978u, &controlState);
    SafeReadDword(player + 4u, &controller);
    if (controller != 0) SafeReadDword(controller + 8u, &controllerFlags);

    LARGE_INTEGER qpc{};
    QueryPerformanceCounter(&qpc);
    char message[1100]{};
    sprintf_s(message,
        "STAGE21_CLASSIFIER side=P%lu tid=%lu qpc=%llu callerRVA=0x%08lX player=0x%08lX dispatchType=%lu dispatchPayload=0x%08lX decodedActionCodePreCall=%lu helperArg=%lu helperRVA=0x000E8CF0 resultEAX=0x%08lX resultAL=%lu eflags=0x%08lX playerMode=0x%08lX state3978=0x%08lX controller=0x%08lX controllerFlags=0x%08lX",
        static_cast<unsigned long>(side),
        static_cast<unsigned long>(GetCurrentThreadId()),
        static_cast<unsigned long long>(qpc.QuadPart),
        static_cast<unsigned long>(returnAddress >= g_gameBase ? returnAddress - g_gameBase : 0),
        static_cast<unsigned long>(player),
        static_cast<unsigned long>(dispatchType),
        static_cast<unsigned long>(dispatchPayload),
        static_cast<unsigned long>(decodedActionCode),
        static_cast<unsigned long>(helperArg),
        static_cast<unsigned long>(resultEax),
        static_cast<unsigned long>(resultEax & 0xFFu),
        static_cast<unsigned long>(savedContext[8]),
        static_cast<unsigned long>(playerMode),
        static_cast<unsigned long>(controlState),
        static_cast<unsigned long>(controller),
        static_cast<unsigned long>(controllerFlags));
    AppendStage21ConsumerLog(message);
}

extern "C" void __cdecl ArcanaLogStage21PayloadSlotClear(const DWORD* savedContext)
{
    if (savedContext == nullptr) return;

    const DWORD player = savedContext[1];        // ESI
    const DWORD payload = savedContext[4];       // EBX
    if (payload != kStage21Payload1E) return;

    DWORD side = 0;
    if (player == g_playerP1) side = 1;
    else if (player == g_playerP2) side = 2;
    else return;

    const DWORD dispatcherEbp = savedContext[2];     // EBP of dispatcher 0xEE200
    const DWORD savedEspAfterPushfd = savedContext[3];
    DWORD slotBefore = 0, returnAddress = 0, arrayPointer = 0, slotIndex = 0;
    DWORD dispatchType = 0, dispatchPayload = 0;
    SafeReadDword(savedEspAfterPushfd + 4u, &slotBefore);      // private pre-call local
    SafeReadDword(savedEspAfterPushfd + 8u, &returnAddress);
    SafeReadDword(savedEspAfterPushfd + 12u, &arrayPointer);
    SafeReadDword(savedEspAfterPushfd + 16u, &slotIndex);
    SafeReadDword(dispatcherEbp + 8u, &dispatchType);
    SafeReadDword(dispatcherEbp + 0x0Cu, &dispatchPayload);
    if (slotIndex != 3u || dispatchType != 1u ||
        dispatchPayload != kStage21Payload1E || payload != dispatchPayload)
        return;

    DWORD slotAfter = 0;
    const DWORD slotAddress = arrayPointer + slotIndex * 4u + 4u;
    const bool afterReadable = SafeReadDword(slotAddress, &slotAfter);
    DWORD controller = 0, controllerFlags = 0;
    SafeReadDword(player + 4u, &controller);
    if (controller != 0) SafeReadDword(controller + 8u, &controllerFlags);

    LARGE_INTEGER qpc{};
    QueryPerformanceCounter(&qpc);
    char message[1100]{};
    sprintf_s(message,
        "STAGE21_FALLBACK_SLOT_CLEAR side=P%lu tid=%lu qpc=%llu callerRVA=0x%08lX player=0x%08lX dispatchType=%lu dispatchPayload=0x%08lX payloadRegister=0x%08lX helperRVA=0x000F7EB0 array=0x%08lX index=%lu slotAddress=0x%08lX slotValueBefore=0x%08lX slotValueAfter=0x%08lX afterReadable=%lu controller=0x%08lX controllerFlagsBeforeFollowingAND=0x%08lX nextInstruction=AND [controller+0x08],0xFFFFFBFF",
        static_cast<unsigned long>(side),
        static_cast<unsigned long>(GetCurrentThreadId()),
        static_cast<unsigned long long>(qpc.QuadPart),
        static_cast<unsigned long>(returnAddress >= g_gameBase ? returnAddress - g_gameBase : 0),
        static_cast<unsigned long>(player),
        static_cast<unsigned long>(dispatchType),
        static_cast<unsigned long>(dispatchPayload),
        static_cast<unsigned long>(payload),
        static_cast<unsigned long>(arrayPointer),
        static_cast<unsigned long>(slotIndex),
        static_cast<unsigned long>(slotAddress),
        static_cast<unsigned long>(slotBefore),
        static_cast<unsigned long>(slotAfter),
        static_cast<unsigned long>(afterReadable ? 1u : 0u),
        static_cast<unsigned long>(controller),
        static_cast<unsigned long>(controllerFlags));
    AppendStage21ConsumerLog(message);
}

extern "C" void __cdecl ArcanaLogStage22PayloadPostAnd(const DWORD* savedContext)
{
    if (savedContext == nullptr) return;

    const DWORD dispatcherEbp = savedContext[2];   // EBP of dispatcher 0xEE200.
    const DWORD savedEsp = savedContext[3];        // ESP immediately after PUSHFD, before PUSHAD.
    DWORD controller = 0, flagsBefore = 0, returnAddress = 0;
    DWORD dispatchType = 0, dispatchPayload = 0;

    // Private locals created by the naked wrapper:
    // savedEsp+4 = controller pointer; savedEsp+8 = flags before the AND.
    SafeReadDword(savedEsp + 4u, &controller);
    SafeReadDword(savedEsp + 8u, &flagsBefore);
    SafeReadDword(savedEsp + 12u, &returnAddress);
    SafeReadDword(dispatcherEbp + 8u, &dispatchType);
    SafeReadDword(dispatcherEbp + 0x0Cu, &dispatchPayload);
    if (dispatchType != 1u || dispatchPayload != kStage21Payload1E)
        return;

    DWORD flagsAfter = 0;
    const bool afterReadable = controller != 0 &&
        SafeReadDword(controller + 8u, &flagsAfter);

    DWORD p1Controller = 0, p2Controller = 0;
    if (g_playerP1 != 0) SafeReadDword(g_playerP1 + 4u, &p1Controller);
    if (g_playerP2 != 0) SafeReadDword(g_playerP2 + 4u, &p2Controller);

    DWORD side = 0, player = 0;
    if (controller != 0 && controller == p1Controller)
    {
        side = 1;
        player = g_playerP1;
    }
    else if (controller != 0 && controller == p2Controller)
    {
        side = 2;
        player = g_playerP2;
    }

    LARGE_INTEGER qpc{};
    QueryPerformanceCounter(&qpc);
    const LONG sequence = InterlockedIncrement(&g_stage22PayloadPostAndSequence);

    char message[1400]{};
    sprintf_s(message,
        "STAGE22_POST_AND seq=%ld side=P%lu tid=%lu qpc=%llu callReturnRVA=0x%08lX player=0x%08lX dispatchType=%lu dispatchPayload=0x%08lX controller=0x%08lX controllerFlagsBeforeAND=0x%08lX controllerFlagsAfterAND=0x%08lX bit400Before=%lu bit400After=%lu afterReadable=%lu eflagsAfterAND=0x%08lX",
        static_cast<long>(sequence),
        static_cast<unsigned long>(side),
        static_cast<unsigned long>(GetCurrentThreadId()),
        static_cast<unsigned long long>(qpc.QuadPart),
        static_cast<unsigned long>(returnAddress >= g_gameBase ? returnAddress - g_gameBase : 0),
        static_cast<unsigned long>(player),
        static_cast<unsigned long>(dispatchType),
        static_cast<unsigned long>(dispatchPayload),
        static_cast<unsigned long>(controller),
        static_cast<unsigned long>(flagsBefore),
        static_cast<unsigned long>(flagsAfter),
        static_cast<unsigned long>((flagsBefore & 0x00000400u) != 0 ? 1u : 0u),
        static_cast<unsigned long>((afterReadable && (flagsAfter & 0x00000400u) != 0) ? 1u : 0u),
        static_cast<unsigned long>(afterReadable ? 1u : 0u),
        static_cast<unsigned long>(savedContext[8]));
    AppendStage22PayloadLog(message);
}

void StartAIQueueTrace(HMODULE proxyModule)
{
    g_proxyModule = proxyModule;
    InitOnceExecuteOnce(&g_startOnce, StartQueueTraceWorker, nullptr, nullptr);
}
