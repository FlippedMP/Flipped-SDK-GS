#pragma once
enum class ELogLevel { Fatal, Error, Warning, Log, Verbose, VeryVerbose };

inline constexpr ELogLevel Fatal = ELogLevel::Fatal;
inline constexpr ELogLevel Error = ELogLevel::Error;
inline constexpr ELogLevel Warning = ELogLevel::Warning;
inline constexpr ELogLevel Log = ELogLevel::Log;
inline constexpr ELogLevel Verbose = ELogLevel::Verbose;
inline constexpr ELogLevel VeryVerbose = ELogLevel::VeryVerbose;

inline const char* ToString(ELogLevel level) {
    switch (level) {
    case ELogLevel::Verbose:
        return "Verbose";
    case ELogLevel::VeryVerbose:
        return "VeryVerbose";
    case ELogLevel::Log:
        return "Log";
    case ELogLevel::Warning:
        return "Warning";
    case ELogLevel::Error:
        return "Error";
    case ELogLevel::Fatal:
        return "Fatal";
    default:
        return "Unknown";
    }
}

inline const wchar_t* ToWString(ELogLevel level) {
    switch (level) {
    case ELogLevel::Verbose:
        return L"Verbose";
    case ELogLevel::VeryVerbose:
        return L"VeryVerbose";
    case ELogLevel::Log:
        return L"Log";
    case ELogLevel::Warning:
        return L"Warning";
    case ELogLevel::Error:
        return L"Error";
    case ELogLevel::Fatal:
        return L"Fatal";
    default:
        return L"Unknown";
    }
}

inline void SetConsoleColorByLogLevel(ELogLevel Level) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    WORD color = FOREGROUND_RED | FOREGROUND_GREEN |
        FOREGROUND_BLUE; // default white

    switch (Level) {
    case ELogLevel::Warning:
        color = FOREGROUND_RED | FOREGROUND_GREEN; // yellow
        break;
    case ELogLevel::Error:
        color = FOREGROUND_RED; // red
        break;
    case ELogLevel::Fatal:
        color = FOREGROUND_RED | FOREGROUND_INTENSITY; // bright red
        break;
    case ELogLevel::Verbose:
        color = FOREGROUND_GREEN | FOREGROUND_BLUE; // cyan
        break;
    case ELogLevel::VeryVerbose:
        color = FOREGROUND_BLUE | FOREGROUND_INTENSITY; // bright blue
        break;
    case ELogLevel::Log:
    default:
        color = FOREGROUND_RED | FOREGROUND_GREEN |
            FOREGROUND_BLUE; // white
        break;
    }

    SetConsoleTextAttribute(hConsole, color);
}

inline void ResetConsoleColor() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
        FOREGROUND_RED | FOREGROUND_GREEN |
        FOREGROUND_BLUE);
}

struct FLogCategory {
    const char* Name;
    ELogLevel RuntimeVerbosity;
};

#if _DEBUG
#define DEFINE_LOG_CATEGORY( CategoryName )                                    \
        inline FLogCategory CategoryName = { #CategoryName,                    \
                                             ELogLevel::VeryVerbose }
#else
#define DEFINE_LOG_CATEGORY( CategoryName )                                    \
        inline FLogCategory CategoryName = { #CategoryName, ELogLevel::Log }
#endif

inline std::string GetTimestampA() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) %
        1000;
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm timeinfo{};
    localtime_s(&timeinfo, &now_c);
    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%Y.%m.%d-%H.%M.%S") << ':'
        << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

inline std::wstring GetTimestampW() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) %
        1000;
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm timeinfo{};
    localtime_s(&timeinfo, &now_c);
    std::wostringstream oss;
    oss << std::put_time(&timeinfo, L"%Y.%m.%d-%H.%M.%S") << ':'
        << std::setfill(L'0') << std::setw(3) << ms.count();
    return oss.str();
}

inline void UELogImpl(FLogCategory& Category, ELogLevel Level, const char* fmt,
    ...) {
    if ((int)Level > (int)Category.RuntimeVerbosity)
        return;

    char buffer[2048];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    SetConsoleColorByLogLevel(Level);

    std::cout << "[" << GetTimestampA() << "]" << Category.Name << ": "
        << ToString(Level) << ": " << buffer << std::endl;

    ResetConsoleColor();
}

inline std::string WStringToUTF8(const std::wstring& wstr) {
    return std::string(wstr.begin(), wstr.end());
}

inline void UELogWImpl(FLogCategory& Category, ELogLevel Level,
    const wchar_t* fmt, ...) {
    if ((int)Level > (int)Category.RuntimeVerbosity)
        return;

    wchar_t wbuffer[2048];

    va_list args;
    va_start(args, fmt);
    vswprintf(wbuffer, sizeof(wbuffer) / sizeof(wchar_t), fmt, args);
    va_end(args);

    std::string Categorydih = std::string(Category.Name);
    std::wstring WDih =
        std::wstring(Categorydih.begin(), Categorydih.end());

    std::wstring wfinal = std::wstring(wbuffer);
    std::string finalbuf = WStringToUTF8(wfinal);

    SetConsoleColorByLogLevel(Level);

    std::cout << "[" << GetTimestampA() << "] " << Category.Name << ": "
        << ToString(Level) << ": " << finalbuf << std::endl;

    ResetConsoleColor();
}

#define UE_LOG( Category, Verbosity, Format, ... )                             \
        UELogImpl( Category, Verbosity, Format, ##__VA_ARGS__ )

#define UE_LOG_W( Category, VerbosityLevel, Format, ... )                      \
        UELogWImpl( Category, VerbosityLevel, Format, ##__VA_ARGS__ )


DEFINE_LOG_CATEGORY(LogFlipped);