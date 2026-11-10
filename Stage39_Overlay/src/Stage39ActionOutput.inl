// Stage39: passive, callback-scoped native action-output snapshot.
// Included by Stage38CallbackWindow.inl inside AIQueueTrace.cpp's anonymous namespace.
// No new hooks, no native calls and absolutely no game-state writes.
constexpr DWORD kStage39Fields = 30u;
struct Stage39State {
    DWORD PreMask, PostMask;
    DWORD Pre[kStage39Fields], Post[kStage39Fields];
};
bool Stage39TargetCallback(DWORD callback)
{
    if (callback < g_gameBase) return false;
    const DWORD rva = callback - g_gameBase;
    return rva == 0x0012EB10u || rva == 0x00123AF0u || rva == 0x001DA490u;
}
void Stage39Snapshot(DWORD player, DWORD* out, DWORD& mask)
{
    // 0..3 direct output fields; 4..10 primary action state P+3EE4;
    // 11..17 alternate action state P+40AC; 18 candidate actor P+43B4.
    const DWORD offsets[19] = {
        0x3F4Cu, 0x3B7Cu, 0x3ED0u, 0x3ED4u,
        0x3EE4u, 0x3EE8u, 0x3EECu, 0x3EF0u, 0x3F0Cu, 0x3F14u, 0x3F18u,
        0x40ACu, 0x40B0u, 0x40B4u, 0x40B8u, 0x40D4u, 0x40DCu, 0x40E0u,
        0x43B4u
    };
    DWORD actor = 0, object = 0;
    for (DWORD i = 0; i < 19u; ++i) {
        DWORD value = 0;
        if (SafeReadDword(player + offsets[i], &value)) {
            out[i] = value; mask |= (1u << i);
            if (i == 18u) actor = value;
        }
    }
    // 19..23 belong to actor pointer read separately in each snapshot.
    const DWORD actorOffsets[5] = {0x14u, 0x1ACu, 0x278u, 0x280u, 0x284u};
    if (actor != 0) for (DWORD i = 0; i < 5u; ++i) {
        DWORD value = 0;
        if (SafeReadDword(actor + actorOffsets[i], &value)) {
            out[19u+i] = value; mask |= (1u << (19u+i));
        }
    }
    const DWORD extraOffsets[3] = {0x3F44u, 0x3F3Cu, 0x3F20u};
    for (DWORD i = 0; i < 3u; ++i) {
        DWORD value = 0;
        if (SafeReadDword(player + extraOffsets[i], &value)) {
            out[24u+i] = value; mask |= (1u << (24u+i));
        }
    }
    // 27..29 are object-relative. Never compare derived fields when O changes.
    if (SafeReadDword(player + 0x4u, &object) && object != 0) {
        const DWORD objectOffsets[3] = {0x184u, 0x278u, 0x280u};
        for (DWORD i = 0; i < 3u; ++i) {
            DWORD value = 0;
            if (SafeReadDword(object + objectOffsets[i], &value)) {
                out[27u+i] = value; mask |= (1u << (27u+i));
            }
        }
    }
}
void AppendStage39Log(const char* message)
{
    wchar_t path[MAX_PATH + 1]{};
    if (!GetSiblingPath(path, MAX_PATH, L"ArcanaAI_Stage39ActionOutputTrace.log")) return;
    SYSTEMTIME now{}; GetLocalTime(&now);
    char line[3200]{};
    const int length = sprintf_s(line, "[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, message);
    if (length <= 0) return;
    HANDLE file = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return;
    DWORD written = 0; WriteFile(file, line, static_cast<DWORD>(length), &written, nullptr);
    CloseHandle(file);
}
// Only worker calls this function, AFTER the corresponding Stage38 record is dequeued.
void Stage39LogWindow(DWORD sequence, DWORD side, DWORD player, DWORD thread,
                      DWORD callback, DWORD callbackAfter, DWORD typePayload,
                      DWORD objectBefore, DWORD generationBefore,
                      ULONGLONG qpcBefore, ULONGLONG qpcAfter,
                      const Stage39State& s)
{
    DWORD changed = 0;
    for (DWORD i = 0; i < kStage39Fields; ++i)
        if ((s.PreMask & s.PostMask & (1u << i)) && s.Pre[i] != s.Post[i])
            changed |= 1u << i;
    char line[2900]{};
    int n = sprintf_s(line,
        "STAGE39_OUTPUT_WINDOW stage38Seq=%lu side=P%lu player=0x%08lX tid=%lu "
        "callback=0x%08lX callbackAfter=0x%08lX callbackRVA=0x%08lX "
        "type=%lu payload=0x%04lX objectBefore=0x%08lX generationBefore=0x%08lX "
        "qpcBefore=%I64u qpcAfter=%I64u preMask=0x%08lX postMask=0x%08lX changedMask=0x%08lX",
        static_cast<unsigned long>(sequence), static_cast<unsigned long>(side),
        static_cast<unsigned long>(player), static_cast<unsigned long>(thread),
        static_cast<unsigned long>(callback), static_cast<unsigned long>(callbackAfter),
        static_cast<unsigned long>(callback - g_gameBase),
        static_cast<unsigned long>(typePayload & 0xFFFFu),
        static_cast<unsigned long>((typePayload >> 16) & 0xFFFFu),
        static_cast<unsigned long>(objectBefore), static_cast<unsigned long>(generationBefore),
        static_cast<unsigned long long>(qpcBefore), static_cast<unsigned long long>(qpcAfter),
        static_cast<unsigned long>(s.PreMask), static_cast<unsigned long>(s.PostMask),
        static_cast<unsigned long>(changed));
    if (n <= 0) return;
    for (DWORD i = 0; i < kStage39Fields; ++i) {
        const int part = sprintf_s(line + n, sizeof(line) - static_cast<size_t>(n),
            " a%lu=%08lX>%08lX", static_cast<unsigned long>(i),
            static_cast<unsigned long>(s.Pre[i]), static_cast<unsigned long>(s.Post[i]));
        if (part <= 0) return;
        n += part;
    }
    AppendStage39Log(line);
}
