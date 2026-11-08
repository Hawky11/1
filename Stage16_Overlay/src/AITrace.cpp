#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstddef>
#include <wchar.h>

#include "AITrace.h"
#include "ComVsCom.h"

namespace
{
    // RVAs confirmed from the user's correct COMvsCOM entry in the CE table.
    constexpr DWORD kP1ControlRva = 0x0DCA5EA4;
    constexpr DWORD kP2ControlRva = 0x0DCD0BCC;
    constexpr LONG kSiteCount = 128;
    constexpr DWORD kDefaultScanIntervalMs = 250;
    constexpr DWORD kMinimumScanIntervalMs = 50;
    constexpr DWORD kMaximumScanIntervalMs = 2000;
    constexpr DWORD kPlayerControlOffset = 0x3F2Cu;
    constexpr DWORD kPlayerBit17Offset = 0x3B84u;
    constexpr DWORD kPlayerBit17Mask = 0x00020000u;
    constexpr DWORD kPlayerBit17RecordCapacity = 4096u;
    constexpr DWORD kControlTraceDr7 = 0x00FF0005u; // DR0/DR1, RW=11 (read/write), LEN=11 (4 bytes).
    constexpr DWORD kPlayerBit17WriteTraceDr7 = 0x00DD0005u; // DR0/DR1, RW=01 (write only), LEN=11 (4 bytes).

    struct TraceSite
    {
        // State: 0 = empty, -1 = being initialized, 1 = ready.
        volatile LONG State;
        volatile LONG Count;
        volatile LONG Reported;
        DWORD EipAfter;
        DWORD RvaAfter;
        DWORD ThreadId;
        DWORD Eax;
        DWORD Ebx;
        DWORD Ecx;
        DWORD Edx;
        DWORD Esi;
        DWORD Edi;
        DWORD Ebp;
        DWORD Esp;
        DWORD EFlags;
        BYTE CodeWindow[24]; // 16 bytes before EIP, then 8 bytes from EIP.
    };

    struct PlayerBit17WriteRecord
    {
        // State: 0 = empty, -1 = owned by producer/consumer, 1 = ready.
        volatile LONG State;
        DWORD Sequence;
        DWORD Side;
        DWORD ThreadId;
        DWORD TickCount;
        SYSTEMTIME LocalTime;
        ULONGLONG QpcTicks;
        DWORD EipAfter;
        DWORD RvaAfter;
        DWORD PlayerObject;
        DWORD FieldAddress;
        DWORD OldValue;
        DWORD NewValue;
        DWORD ShadowInitialized;
        DWORD Dr6;
        DWORD Eax;
        DWORD Ebx;
        DWORD Ecx;
        DWORD Edx;
        DWORD Esi;
        DWORD Edi;
        DWORD Ebp;
        DWORD Esp;
        DWORD EFlags;
        BYTE CodeWindow[24];
    };

    TraceSite g_sites[2][kSiteCount]{};
    INIT_ONCE g_startOnce = INIT_ONCE_STATIC_INIT;
    HMODULE g_proxyModule = nullptr;
    HMODULE g_gameModule = nullptr;
    PVOID g_vectoredHandler = nullptr;
    DWORD g_gameImageSize = 0;
    DWORD g_gameBase = 0;
    DWORD g_watchP1 = 0;
    DWORD g_watchP2 = 0;
    DWORD g_scanIntervalMs = kDefaultScanIntervalMs;
    DWORD g_watchDr7Configuration = kControlTraceDr7;
    bool g_playerBit17WriteTrace = false;
    volatile LONG g_playerBit17ShadowValue[2]{};
    volatile LONG g_playerBit17ShadowValid[2]{};
    DWORD g_playerBit17InitialValue[2]{};
    volatile LONG g_playerBit17WriteSequence = 0;
    volatile LONG g_playerBit17WriteDropped = 0;
    volatile LONG g_playerBit17WriteReportedDropped = 0;
    PlayerBit17WriteRecord g_playerBit17WriteRecords[kPlayerBit17RecordCapacity]{};
    volatile LONG g_firstInstallLog = 0;
    volatile LONG g_noThreadInstallLog = 0;

    // Keep handles for inspected threads so the scan loop does not suspend
    // every game thread every 250 ms. A signalled handle is a terminated thread;
    // its slot is then reclaimed (also making thread-ID reuse safe).
    struct TrackedThread
    {
        DWORD Id;
        HANDLE Handle;
        bool WatchpointsInstalled;
    };
    constexpr DWORD kTrackedThreadCapacity = 256;
    TrackedThread g_trackedThreads[kTrackedThreadCapacity]{};
    volatile LONG g_capacityWarningLogged = 0;

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

    void AppendTraceLog(const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_Trace.log"))
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

    void AppendPlayerBit17TraceLog(const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_PlayerBit17WriteTrace.log"))
        {
            return;
        }

        SYSTEMTIME now{};
        GetLocalTime(&now);
        char line[2048]{};
        const int count = sprintf_s(
            line,
            "[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
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

    int ReadIniInt(const wchar_t* section, const wchar_t* key, int defaultValue)
    {
        wchar_t iniPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(iniPath, MAX_PATH, L"ArcanaAI.ini"))
        {
            return defaultValue;
        }
        return GetPrivateProfileIntW(section, key, defaultValue, iniPath);
    }

    DWORD GetImageSize32(HMODULE module)
    {
        if (module == nullptr)
        {
            return 0;
        }

        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0)
        {
            return 0;
        }

        const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS32*>(
            reinterpret_cast<const BYTE*>(module) + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE ||
            nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        {
            return 0;
        }
        return nt->OptionalHeader.SizeOfImage;
    }

    bool IsGameImageAddress(DWORD address)
    {
        return g_gameBase != 0 && g_gameImageSize != 0 &&
            address >= g_gameBase + 0x1000 && address - g_gameBase < g_gameImageSize;
    }

    void CaptureCodeWindow(DWORD eipAfter, BYTE output[24])
    {
        std::memset(output, 0, 24);
        if (g_gameBase == 0 || eipAfter < g_gameBase + 0x1000 ||
            eipAfter >= g_gameBase + g_gameImageSize)
        {
            return;
        }

        DWORD start = eipAfter >= g_gameBase + 0x1010 ? eipAfter - 16 : g_gameBase + 0x1000;
        MEMORY_BASIC_INFORMATION info{};
        if (VirtualQuery(reinterpret_cast<const void*>(start), &info, sizeof(info)) != sizeof(info))
        {
            return;
        }
        if (info.State != MEM_COMMIT || (info.Protect & PAGE_GUARD) != 0 ||
            (info.Protect & 0xFF) == PAGE_NOACCESS)
        {
            return;
        }

        const auto regionStart = reinterpret_cast<std::uintptr_t>(info.BaseAddress);
        const auto regionEnd = regionStart + info.RegionSize;
        const auto copyStart = static_cast<std::uintptr_t>(start);
        const std::size_t available = copyStart < regionEnd
            ? static_cast<std::size_t>(regionEnd - copyStart)
            : 0;
        const std::size_t length = available < 24 ? available : 24;
        if (length != 0)
        {
            std::memcpy(output, reinterpret_cast<const void*>(start), length);
        }
    }

    void AddTraceSite(int fieldIndex, PCONTEXT context)
    {
        if (fieldIndex < 0 || fieldIndex > 1 || context == nullptr)
        {
            return;
        }

        const DWORD eip = context->Eip;
        if (eip == 0)
        {
            return;
        }

        const DWORD startIndex = ((eip >> 4) ^ (static_cast<DWORD>(fieldIndex + 1) * 0x9E3779B9u)) % kSiteCount;
        for (DWORD probe = 0; probe < static_cast<DWORD>(kSiteCount); ++probe)
        {
            TraceSite& site = g_sites[fieldIndex][(startIndex + probe) % kSiteCount];
            const LONG state = InterlockedCompareExchange(&site.State, 0, 0);

            if (state == 1 && site.EipAfter == eip)
            {
                InterlockedIncrement(&site.Count);
                return;
            }

            if (state == 0 && InterlockedCompareExchange(&site.State, -1, 0) == 0)
            {
                site.EipAfter = eip;
                site.RvaAfter = eip - g_gameBase;
                site.ThreadId = GetCurrentThreadId();
                site.Eax = context->Eax;
                site.Ebx = context->Ebx;
                site.Ecx = context->Ecx;
                site.Edx = context->Edx;
                site.Esi = context->Esi;
                site.Edi = context->Edi;
                site.Ebp = context->Ebp;
                site.Esp = context->Esp;
                site.EFlags = context->EFlags;
                CaptureCodeWindow(eip, site.CodeWindow);
                InterlockedExchange(&site.Count, 1);
                InterlockedExchange(&site.State, 1);
                return;
            }
        }
    }

    bool ReadDwordAtAddress(DWORD address, DWORD* value)
    {
        if (address == 0 || value == nullptr) return false;
        SIZE_T bytesRead = 0;
        return ReadProcessMemory(
            GetCurrentProcess(), reinterpret_cast<LPCVOID>(address),
            value, sizeof(*value), &bytesRead) != FALSE && bytesRead == sizeof(*value);
    }

    void QueuePlayerBit17WriteRecord(int sideIndex, PCONTEXT context)
    {
        if (!g_playerBit17WriteTrace || sideIndex < 0 || sideIndex > 1 || context == nullptr)
            return;

        const DWORD fieldAddress = sideIndex == 0 ? g_watchP1 : g_watchP2;
        DWORD newValue = 0;
        if (!ReadDwordAtAddress(fieldAddress, &newValue))
        {
            InterlockedIncrement(&g_playerBit17WriteDropped);
            return;
        }

        const LONG sequence = InterlockedIncrement(&g_playerBit17WriteSequence);
        PlayerBit17WriteRecord& record =
            g_playerBit17WriteRecords[(static_cast<DWORD>(sequence - 1)) % kPlayerBit17RecordCapacity];
        if (InterlockedCompareExchange(&record.State, -1, 0) != 0)
        {
            InterlockedIncrement(&g_playerBit17WriteDropped);
            return;
        }

        const LONG oldKnown = InterlockedCompareExchange(&g_playerBit17ShadowValid[sideIndex], 0, 0);
        const LONG oldValue = InterlockedExchange(
            &g_playerBit17ShadowValue[sideIndex], static_cast<LONG>(newValue));

        record.Sequence = static_cast<DWORD>(sequence);
        record.Side = static_cast<DWORD>(sideIndex + 1);
        record.ThreadId = GetCurrentThreadId();
        record.TickCount = GetTickCount();
        GetLocalTime(&record.LocalTime);
        LARGE_INTEGER qpc{};
        QueryPerformanceCounter(&qpc);
        record.QpcTicks = static_cast<ULONGLONG>(qpc.QuadPart);
        record.EipAfter = context->Eip;
        record.RvaAfter = context->Eip >= g_gameBase ? context->Eip - g_gameBase : 0;
        record.PlayerObject = fieldAddress - kPlayerBit17Offset;
        record.FieldAddress = fieldAddress;
        record.OldValue = oldKnown != 0 ? static_cast<DWORD>(oldValue) : newValue;
        record.NewValue = newValue;
        record.ShadowInitialized = oldKnown != 0 ? 1u : 0u;
        record.Dr6 = context->Dr6;
        record.Eax = context->Eax;
        record.Ebx = context->Ebx;
        record.Ecx = context->Ecx;
        record.Edx = context->Edx;
        record.Esi = context->Esi;
        record.Edi = context->Edi;
        record.Ebp = context->Ebp;
        record.Esp = context->Esp;
        record.EFlags = context->EFlags;
        CaptureCodeWindow(context->Eip, record.CodeWindow);
        InterlockedExchange(&record.State, 1);
    }

    void FlushPlayerBit17WriteRecords()
    {
        for (DWORD i = 0; i < kPlayerBit17RecordCapacity; ++i)
        {
            PlayerBit17WriteRecord& record = g_playerBit17WriteRecords[i];
            if (InterlockedCompareExchange(&record.State, -1, 1) != 1)
                continue;

            char codeHex[24 * 3 + 1]{};
            for (int j = 0; j < 24; ++j)
            {
                sprintf_s(codeHex + j * 3, sizeof(codeHex) - j * 3, "%02X ", record.CodeWindow[j]);
            }

            const DWORD oldBit17 = (record.OldValue & kPlayerBit17Mask) != 0 ? 1u : 0u;
            const DWORD newBit17 = (record.NewValue & kPlayerBit17Mask) != 0 ? 1u : 0u;
            char message[1536]{};
            sprintf_s(
                message, sizeof(message),
                "PLAYER_BIT17_WRITE seq=%lu side=P%lu tid=%lu tick=%lu time=%02u:%02u:%02u.%03u qpc=0x%08lX%08lX "
                "eipAfter=0x%08lX rvaAfter=0x%08lX player=0x%08lX field=0x%08lX old=0x%08lX new=0x%08lX "
                "shadowInitialized=%lu oldBit17=%s newBit17=%lu shadowBit17Changed=%lu dr6=0x%08lX "
                "EAX=%08lX EBX=%08lX ECX=%08lX EDX=%08lX ESI=%08lX EDI=%08lX EBP=%08lX ESP=%08lX EFLAGS=%08lX "
                "bytes_at_EIP_minus_16=%s",
                static_cast<unsigned long>(record.Sequence),
                static_cast<unsigned long>(record.Side),
                static_cast<unsigned long>(record.ThreadId),
                static_cast<unsigned long>(record.TickCount),
                record.LocalTime.wHour, record.LocalTime.wMinute,
                record.LocalTime.wSecond, record.LocalTime.wMilliseconds,
                static_cast<unsigned long>(record.QpcTicks >> 32),
                static_cast<unsigned long>(record.QpcTicks & 0xFFFFFFFFu),
                static_cast<unsigned long>(record.EipAfter),
                static_cast<unsigned long>(record.RvaAfter),
                static_cast<unsigned long>(record.PlayerObject),
                static_cast<unsigned long>(record.FieldAddress),
                static_cast<unsigned long>(record.OldValue),
                static_cast<unsigned long>(record.NewValue),
                static_cast<unsigned long>(record.ShadowInitialized),
                record.ShadowInitialized != 0 ? (oldBit17 != 0 ? "1" : "0") : "NA",
                static_cast<unsigned long>(newBit17),
                static_cast<unsigned long>(record.ShadowInitialized != 0 && oldBit17 != newBit17),
                static_cast<unsigned long>(record.Dr6),
                static_cast<unsigned long>(record.Eax),
                static_cast<unsigned long>(record.Ebx),
                static_cast<unsigned long>(record.Ecx),
                static_cast<unsigned long>(record.Edx),
                static_cast<unsigned long>(record.Esi),
                static_cast<unsigned long>(record.Edi),
                static_cast<unsigned long>(record.Ebp),
                static_cast<unsigned long>(record.Esp),
                static_cast<unsigned long>(record.EFlags),
                codeHex);
            AppendPlayerBit17TraceLog(message);
            InterlockedExchange(&record.State, 0);
        }

        const LONG dropped = InterlockedCompareExchange(&g_playerBit17WriteDropped, 0, 0);
        const LONG reported = InterlockedCompareExchange(&g_playerBit17WriteReportedDropped, 0, 0);
        if (dropped > reported)
        {
            char warning[192]{};
            sprintf_s(warning,
                "WARNING: Stage16 write-record buffer dropped %ld record(s); capacity=%lu.",
                static_cast<long>(dropped), static_cast<unsigned long>(kPlayerBit17RecordCapacity));
            AppendPlayerBit17TraceLog(warning);
            InterlockedExchange(&g_playerBit17WriteReportedDropped, dropped);
        }
    }

    LONG CALLBACK TraceExceptionHandler(EXCEPTION_POINTERS* exceptionInfo)
    {
        if (exceptionInfo == nullptr || exceptionInfo->ExceptionRecord == nullptr ||
            exceptionInfo->ContextRecord == nullptr ||
            exceptionInfo->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP)
        {
            return EXCEPTION_CONTINUE_SEARCH;
        }

        PCONTEXT context = exceptionInfo->ContextRecord;
        const DWORD dr6 = context->Dr6;
        const DWORD ownedHits = dr6 & 0x3;
        const DWORD commandTraceHits = dr6 & 0xCu; // DR2/DR3 belong to AICommandTrace.
        const DWORD otherDebugHits = dr6 & 0x0000E000u; // BD, BS, BT only.
        if (ownedHits == 0 || otherDebugHits != 0)
        {
            return EXCEPTION_CONTINUE_SEARCH;
        }

        // Clear only the status bits belonging to our two data watchpoints.
        context->Dr6 &= ~0x3u;

        // Keep only accesses executed by AALib.exe itself. This excludes our
        // COMvsCOM writer and operating-system routines from the trace.
        if (!IsGameImageAddress(context->Eip))
        {
            return EXCEPTION_CONTINUE_EXECUTION;
        }

        if ((ownedHits & 0x1) != 0)
        {
            if (g_playerBit17WriteTrace) QueuePlayerBit17WriteRecord(0, context);
            else AddTraceSite(0, context);
        }
        if ((ownedHits & 0x2) != 0)
        {
            if (g_playerBit17WriteTrace) QueuePlayerBit17WriteRecord(1, context);
            else AddTraceSite(1, context);
        }

        if (commandTraceHits != 0)
        {
            // Stage16 has already recorded any simultaneous DR0/DR1 data hit.
            // Pass the still-pending DR2/DR3 execution hit to AICommandTrace.
            return EXCEPTION_CONTINUE_SEARCH;
        }

        return EXCEPTION_CONTINUE_EXECUTION;
    }

    bool IsOurWatchpointConfiguration(const CONTEXT& context)
    {
        return context.Dr0 == g_watchP1 &&
            context.Dr1 == g_watchP2 &&
            (context.Dr7 & 0x00FF000Fu) == (g_watchDr7Configuration & 0x00FF000Fu);
    }

    void PruneTrackedThreads()
    {
        for (DWORD i = 0; i < kTrackedThreadCapacity; ++i)
        {
            TrackedThread& slot = g_trackedThreads[i];
            if (slot.Handle != nullptr && WaitForSingleObject(slot.Handle, 0) == WAIT_OBJECT_0)
            {
                CloseHandle(slot.Handle);
                slot = {};
            }
        }
    }

    bool IsThreadTracked(DWORD threadId)
    {
        for (DWORD i = 0; i < kTrackedThreadCapacity; ++i)
        {
            if (g_trackedThreads[i].Handle != nullptr && g_trackedThreads[i].Id == threadId)
            {
                return true;
            }
        }
        return false;
    }

    bool InspectThreadAndKeepHandle(DWORD threadId, HANDLE* retainedHandle, bool* watchpointsInstalled)
    {
        if (retainedHandle == nullptr || watchpointsInstalled == nullptr ||
            threadId == GetCurrentThreadId())
        {
            return false;
        }

        *retainedHandle = nullptr;
        *watchpointsInstalled = false;

        HANDLE thread = OpenThread(
            THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT |
                THREAD_QUERY_INFORMATION | SYNCHRONIZE,
            FALSE,
            threadId);
        if (thread == nullptr)
        {
            return false;
        }

        bool inspected = false;
        bool installed = false;
        const DWORD suspendResult = SuspendThread(thread);
        if (suspendResult != static_cast<DWORD>(-1))
        {
            CONTEXT context{};
            context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
            if (GetThreadContext(thread, &context))
            {
                inspected = true;
                if (IsOurWatchpointConfiguration(context))
                {
                    installed = true;
                }
                else if ((context.Dr7 & 0x00FF000Fu) == 0)
                {
                    context.Dr0 = g_watchP1;
                    context.Dr1 = g_watchP2;
                    context.Dr6 &= ~0x3u;
                    // DR0/DR1 are local, 4-byte data breakpoints. The configured
                    // mode selects read/write (Stage 2) or write-only (Stage 16).
                    // DR2/DR3 and unrelated DR7 fields are preserved.
                    context.Dr7 = (context.Dr7 & ~0x00FF000Fu) | g_watchDr7Configuration;
                    installed = SetThreadContext(thread, &context) != FALSE;
                }
                // If any DR0/DR1 enable or RW/LEN configuration bits are set,
                // keep the thread tracked but leave its debug-register state untouched.
            }
            ResumeThread(thread);
        }

        if (!inspected)
        {
            CloseHandle(thread);
            return false;
        }

        *retainedHandle = thread;
        *watchpointsInstalled = installed;
        return true;
    }

    DWORD InstallWatchpointsOnCurrentThreads()
    {
        const DWORD processId = GetCurrentProcessId();
        const DWORD currentThreadId = GetCurrentThreadId();
        const DWORD comVsComThreadId = GetComVsComThreadId();
        DWORD newlyInstalled = 0;
        DWORD newlyInspected = 0;
        DWORD newlySkipped = 0;
        PruneTrackedThreads();

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
                    entry.th32ThreadID != currentThreadId &&
                    entry.th32ThreadID != comVsComThreadId &&
                    !IsThreadTracked(entry.th32ThreadID))
                {
                    HANDLE retainedHandle = nullptr;
                    bool watchpointsInstalled = false;
                    if (InspectThreadAndKeepHandle(
                        entry.th32ThreadID, &retainedHandle, &watchpointsInstalled))
                    {
                        bool stored = false;
                        for (DWORD slotIndex = 0; slotIndex < kTrackedThreadCapacity; ++slotIndex)
                        {
                            TrackedThread& slot = g_trackedThreads[slotIndex];
                            if (slot.Handle == nullptr)
                            {
                                slot.Id = entry.th32ThreadID;
                                slot.Handle = retainedHandle;
                                slot.WatchpointsInstalled = watchpointsInstalled;
                                stored = true;
                                ++newlyInspected;
                                if (watchpointsInstalled)
                                {
                                    ++newlyInstalled;
                                }
                                else
                                {
                                    ++newlySkipped;
                                }
                                break;
                            }
                        }

                        if (!stored)
                        {
                            CloseHandle(retainedHandle);
                            if (InterlockedCompareExchange(&g_capacityWarningLogged, 1, 0) == 0)
                            {
                                AppendTraceLog("WARNING: tracked-thread capacity reached; additional threads will not be watched.");
                            }
                        }
                    }
                }
                entry.dwSize = sizeof(entry);
            } while (Thread32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
        if (newlySkipped != 0)
        {
            char warning[256]{};
            sprintf_s(warning,
                "WARNING: %lu newly inspected game thread(s) were not given DR0/DR1 because those slots were occupied or unavailable; these threads are tracked but will not be retried.",
                static_cast<unsigned long>(newlySkipped));
            AppendTraceLog(warning);
            if (g_playerBit17WriteTrace) AppendPlayerBit17TraceLog(warning);
        }
        if (newlyInspected != 0 && newlyInstalled == 0 &&
            InterlockedCompareExchange(&g_noThreadInstallLog, 1, 0) == 0)
        {
            AppendTraceLog("Threads were inspected, but DR0/DR1 were unavailable or could not be set. Existing debug-register owners were left untouched.");
            if (g_playerBit17WriteTrace)
                AppendPlayerBit17TraceLog("ERROR: no newly inspected game thread accepted the P1/P2 write watchpoints; external DR0/DR1 owners were left untouched.");
        }
        return newlyInstalled;
    }

    void FlushNewSites()
    {
        for (int field = 0; field < 2; ++field)
        {
            for (LONG i = 0; i < kSiteCount; ++i)
            {
                TraceSite& site = g_sites[field][i];
                if (InterlockedCompareExchange(&site.State, 0, 0) != 1 ||
                    InterlockedCompareExchange(&site.Reported, 1, 0) != 0)
                {
                    continue;
                }

                char codeHex[24 * 3 + 1]{};
                for (int j = 0; j < 24; ++j)
                {
                    sprintf_s(codeHex + j * 3, sizeof(codeHex) - j * 3, "%02X ", site.CodeWindow[j]);
                }

                char message[1536]{};
                sprintf_s(
                    message,
                    "WATCH_SITE field=%s tid=%lu EIP_after=0x%08lX RVA_after=0x%08lX hits=%ld "
                    "EAX=%08lX EBX=%08lX ECX=%08lX EDX=%08lX ESI=%08lX EDI=%08lX EBP=%08lX ESP=%08lX "
                    "EFLAGS=%08lX bytes_at_EIP_minus_16=%s",
                    field == 0 ? "P1_COM" : "P2_COM",
                    static_cast<unsigned long>(site.ThreadId),
                    static_cast<unsigned long>(site.EipAfter),
                    static_cast<unsigned long>(site.RvaAfter),
                    static_cast<long>(InterlockedCompareExchange(&site.Count, 0, 0)),
                    static_cast<unsigned long>(site.Eax),
                    static_cast<unsigned long>(site.Ebx),
                    static_cast<unsigned long>(site.Ecx),
                    static_cast<unsigned long>(site.Edx),
                    static_cast<unsigned long>(site.Esi),
                    static_cast<unsigned long>(site.Edi),
                    static_cast<unsigned long>(site.Ebp),
                    static_cast<unsigned long>(site.Esp),
                    static_cast<unsigned long>(site.EFlags),
                    codeHex);
                AppendTraceLog(message);
            }
        }
    }

    DWORD WINAPI FieldTraceThread(void*)
    {
        const int controlTraceEnabled = ReadIniInt(L"AIResearch", L"EnableControlFieldTrace", 0);
        const int bit17TraceEnabled = ReadIniInt(L"AIResearch", L"EnablePlayerBit17WriteTrace", 0);
        if (controlTraceEnabled != 0 && bit17TraceEnabled != 0)
        {
            AppendTraceLog("ERROR: EnableControlFieldTrace and EnablePlayerBit17WriteTrace both use DR0/DR1; disable one mode. No DR0/DR1 watchpoints were installed.");
            AppendPlayerBit17TraceLog("ERROR: conflicting DR0/DR1 modes. Set EnableControlFieldTrace=0 and keep EnablePlayerBit17WriteTrace=1.");
            return 0;
        }
        g_playerBit17WriteTrace = bit17TraceEnabled != 0;
        g_watchDr7Configuration = g_playerBit17WriteTrace
            ? kPlayerBit17WriteTraceDr7 : kControlTraceDr7;
        const int enabled = controlTraceEnabled != 0 || bit17TraceEnabled != 0;
        int interval = ReadIniInt(L"AIResearch", L"ThreadScanIntervalMs", static_cast<int>(kDefaultScanIntervalMs));
        if (interval < static_cast<int>(kMinimumScanIntervalMs)) interval = static_cast<int>(kMinimumScanIntervalMs);
        if (interval > static_cast<int>(kMaximumScanIntervalMs)) interval = static_cast<int>(kMaximumScanIntervalMs);
        g_scanIntervalMs = static_cast<DWORD>(interval);

        if (enabled == 0)
        {
            AppendTraceLog("AI field trace is disabled in ArcanaAI.ini. No hardware watchpoints were installed.");
            AppendPlayerBit17TraceLog("Stage 16 player bit-17 write trace is disabled in ArcanaAI.ini.");
            return 0;
        }

        for (int attempt = 0; attempt < 300 && g_gameModule == nullptr; ++attempt)
        {
            g_gameModule = GetModuleHandleW(L"AALib.exe");
            if (g_gameModule == nullptr)
            {
                Sleep(100);
            }
        }

        if (g_gameModule == nullptr)
        {
            AppendTraceLog("ERROR: AALib.exe was not found; AI field trace stopped.");
            if (g_playerBit17WriteTrace) AppendPlayerBit17TraceLog("ERROR: AALib.exe was not found; Stage16 trace stopped.");
            return 0;
        }

        g_gameBase = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(g_gameModule));
        g_gameImageSize = GetImageSize32(g_gameModule);
        const DWORD p1PlayerRva = kP1ControlRva - kPlayerControlOffset;
        const DWORD p2PlayerRva = kP2ControlRva - kPlayerControlOffset;
        const DWORD p1WatchRva = g_playerBit17WriteTrace
            ? p1PlayerRva + kPlayerBit17Offset : kP1ControlRva;
        const DWORD p2WatchRva = g_playerBit17WriteTrace
            ? p2PlayerRva + kPlayerBit17Offset : kP2ControlRva;
        if (g_gameImageSize < sizeof(DWORD) ||
            p1WatchRva > g_gameImageSize - sizeof(DWORD) ||
            p2WatchRva > g_gameImageSize - sizeof(DWORD))
        {
            AppendTraceLog("ERROR: the loaded AALib.exe image does not contain the selected P1/P2 watchpoint RVAs.");
            AppendPlayerBit17TraceLog("ERROR: the loaded AALib.exe image does not contain both player+0x3B84 watchpoint RVAs.");
            return 0;
        }

        g_watchP1 = g_gameBase + p1WatchRva;
        g_watchP2 = g_gameBase + p2WatchRva;
        if (g_playerBit17WriteTrace)
        {
            DWORD initialP1 = 0, initialP2 = 0;
            const bool p1Read = ReadDwordAtAddress(g_watchP1, &initialP1);
            const bool p2Read = ReadDwordAtAddress(g_watchP2, &initialP2);
            if (!p1Read || !p2Read)
            {
                AppendPlayerBit17TraceLog("ERROR: could not read initial P1/P2 player+0x3B84 values; refusing to start because the old-value shadow would be invalid.");
                return 0;
            }
            g_playerBit17InitialValue[0] = initialP1;
            g_playerBit17InitialValue[1] = initialP2;
            InterlockedExchange(&g_playerBit17ShadowValue[0], static_cast<LONG>(initialP1));
            InterlockedExchange(&g_playerBit17ShadowValue[1], static_cast<LONG>(initialP2));
            InterlockedExchange(&g_playerBit17ShadowValid[0], 1);
            InterlockedExchange(&g_playerBit17ShadowValid[1], 1);
        }

        // Wait until the COMvsCOM writer publishes its thread ID. We must not
        // install data breakpoints on that writer or its 10 ms polling stores
        // would generate noise in the trace.
        DWORD comThreadId = 0;
        for (int attempt = 0; attempt < 100 && comThreadId == 0; ++attempt)
        {
            comThreadId = GetComVsComThreadId();
            if (comThreadId == 0)
            {
                Sleep(20);
            }
        }
        if (comThreadId == 0)
        {
            AppendTraceLog("ERROR: COMvsCOM worker thread ID was not published; tracer stopped to avoid watching its own writer.");
            if (g_playerBit17WriteTrace) AppendPlayerBit17TraceLog("ERROR: COMvsCOM worker thread ID was not published; Stage16 stopped before installing watchpoints.");
            return 0;
        }

        if (g_vectoredHandler == nullptr)
        {
            g_vectoredHandler = AddVectoredExceptionHandler(1, TraceExceptionHandler);
        }
        if (g_vectoredHandler == nullptr)
        {
            AppendTraceLog("ERROR: AddVectoredExceptionHandler failed; AI field trace stopped.");
            if (g_playerBit17WriteTrace) AppendPlayerBit17TraceLog("ERROR: AddVectoredExceptionHandler failed; Stage16 stopped before installing watchpoints.");
            return 0;
        }

        char startMessage[768]{};
        if (g_playerBit17WriteTrace)
        {
            sprintf_s(
                startMessage,
                "STAGE16 active. Write-only DR0/DR1 watchpoints cover exact P1/P2 DWORD player+0x3B84: "
                "P1=0x%08lX (RVA 0x%08lX, initial=0x%08lX), P2=0x%08lX (RVA 0x%08lX, initial=0x%08lX). "
                "DR7 config=0x%08lX; scan=%lu ms; COMvsCOM writer TID=%lu excluded; external DR0/DR1 owners are not overwritten.",
                static_cast<unsigned long>(g_watchP1), static_cast<unsigned long>(p1WatchRva),
                static_cast<unsigned long>(g_playerBit17InitialValue[0]),
                static_cast<unsigned long>(g_watchP2), static_cast<unsigned long>(p2WatchRva),
                static_cast<unsigned long>(g_playerBit17InitialValue[1]),
                static_cast<unsigned long>(g_watchDr7Configuration),
                static_cast<unsigned long>(g_scanIntervalMs), static_cast<unsigned long>(comThreadId));
            AppendPlayerBit17TraceLog(startMessage);
            AppendPlayerBit17TraceLog("Each record is a post-write hardware-breakpoint snapshot. EIP/RVA is after the writing instruction; bytes_at_EIP_minus_16 allows offline instruction decoding. oldValue is a best-effort shadow of the last observed field value; shadowInitialized=1 means only that the initial baseline read succeeded, not that no write was missed on an uninstrumented thread.");
        }
        else
        {
            sprintf_s(
                startMessage,
                "AI control-field trace active. Watching reads/writes at P1=0x%08lX (RVA 0x%08lX), "
                "P2=0x%08lX (RVA 0x%08lX). Thread scan interval=%lu ms. COMvsCOM writer TID=%lu is excluded; existing DR0/DR1 owners are not overwritten.",
                static_cast<unsigned long>(g_watchP1),
                static_cast<unsigned long>(kP1ControlRva),
                static_cast<unsigned long>(g_watchP2),
                static_cast<unsigned long>(kP2ControlRva),
                static_cast<unsigned long>(g_scanIntervalMs),
                static_cast<unsigned long>(comThreadId));
            AppendTraceLog(startMessage);
        }

        for (;;)
        {
            const DWORD newlyInstalled = InstallWatchpointsOnCurrentThreads();
            if (newlyInstalled != 0)
            {
                char message[256]{};
                sprintf_s(message, "Hardware watchpoint install batch: %lu new game thread(s).",
                    static_cast<unsigned long>(newlyInstalled));
                AppendTraceLog(message);
                if (g_playerBit17WriteTrace) AppendPlayerBit17TraceLog(message);
                InterlockedExchange(&g_firstInstallLog, 1);
            }
            else if (InterlockedCompareExchange(&g_firstInstallLog, 0, 0) == 0 &&
                InterlockedCompareExchange(&g_noThreadInstallLog, 1, 0) == 0)
            {
                AppendTraceLog("No eligible game thread accepted DR0/DR1 yet. Threads using those debug-register slots are left untouched; scanning will continue.");
                if (g_playerBit17WriteTrace)
                    AppendPlayerBit17TraceLog("WARNING: no eligible game thread has accepted the P1/P2 write watchpoints yet.");
            }

            if (g_playerBit17WriteTrace)
                FlushPlayerBit17WriteRecords();
            else
                FlushNewSites();
            Sleep(g_scanIntervalMs);
        }
    }

    BOOL CALLBACK StartTraceWorker(PINIT_ONCE, PVOID, PVOID*)
    {
        HANDLE thread = CreateThread(nullptr, 0, FieldTraceThread, nullptr, 0, nullptr);
        if (thread == nullptr)
        {
            AppendTraceLog("ERROR: failed to create AI field-trace worker thread.");
            return TRUE;
        }
        CloseHandle(thread);
        return TRUE;
    }
}

void StartAIFieldTrace(HMODULE proxyModule)
{
    if (proxyModule != nullptr)
    {
        g_proxyModule = proxyModule;
    }

    InitOnceExecuteOnce(&g_startOnce, StartTraceWorker, nullptr, nullptr);
}
