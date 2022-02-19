/**
 * @file
 * @author 9chu
 * @date 2022/2/14
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Logging.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <SDL_log.h>

#include <lstg/Core/Pal.hpp>

using namespace std;
using namespace lstg;

namespace lstg::detail
{
    detail::LogTimePoint GetLogCurrentTime() noexcept
    {
        return std::chrono::system_clock::now();
    }

    const char* GetLogShortFileName(const char* full) noexcept
    {
        if (!full)
            return "<null>";

        const char* lastSeenSep = full;
        for (const char* p = full; *p; ++p)
        {
            auto ch = *p;
            if (ch == '/' || ch == '\\')
                lastSeenSep = p;
        }
        if (*lastSeenSep == '/' || *lastSeenSep == '\\')
            return lastSeenSep + 1;
        return lastSeenSep;
    }

    class LogBackend
    {
    public:
        LogBackend() noexcept
        {
            // 格式：[MM/DD/YY HH:MM:SS.MS][ThreadID][CategoryName][File:Line#Function][Level] What
            static const char* kLogPattern = "[%D %H:%M:%S.%e][%t][%n][%@#%!][%l] %v";

            try
            {
                auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                consoleSink->set_level(spdlog::level::trace);
                consoleSink->set_pattern(kLogPattern);
                m_pConsoleSink = consoleSink;
            }
            catch (...)
            {
                assert(false);
            }

#ifndef __EMSCRIPTEN__
            try
            {
                auto logFileName = Pal::GetUserStorageDirectory() / "log.txt";
                auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFileName.string(), true);
                fileSink->set_level(spdlog::level::trace);
                fileSink->set_pattern(kLogPattern);
                m_pFileSink = fileSink;
            }
            catch (...)
            {
                assert(false);
            }
#endif
        }

        LogBackend(const LogBackend&) = delete;
        LogBackend(LogBackend&&) noexcept = delete;

    public:
        void Log(const detail::LogMessage& message) noexcept
        {
            spdlog::level::level_enum level;
            switch (message.Level)
            {
                case LogLevel::Trace:
                    level = spdlog::level::trace;
                    break;
                case LogLevel::Debug:
                    level = spdlog::level::debug;
                    break;
                case LogLevel::Info:
                    level = spdlog::level::info;
                    break;
                case LogLevel::Warn:
                    level = spdlog::level::warn;
                    break;
                case LogLevel::Error:
                    level = spdlog::level::err;
                    break;
                case LogLevel::Critical:
                    level = spdlog::level::critical;
                    break;
                default:
                    level = spdlog::level::off;
                    break;
            }

            spdlog::details::log_msg logMsg {
                message.Time,
                {message.SourceLocation.FileName, message.SourceLocation.Line, message.SourceLocation.FunctionName},
                message.CategoryName ? message.CategoryName : "-",
                level,
                message.Payload
            };

            try
            {
                if (m_pConsoleSink)
                    m_pConsoleSink->log(logMsg);

#ifndef __EMSCRIPTEN__
                if (m_pFileSink)
                {
                    m_pFileSink->log(logMsg);
                    m_pFileSink->flush();
                }
#endif
            }
            catch (...)
            {
            }
        }

    private:
        spdlog::sink_ptr m_pConsoleSink;
        spdlog::sink_ptr m_pFileSink;
    };
}

LSTG_DEF_LOG_CATEGORY(SDL);

namespace
{
    void SdlLogOutputRedirect(void* userdata, int category, SDL_LogPriority priority, const char* message)
    {
        LogLevel level;
        switch (priority)
        {
            case SDL_LOG_PRIORITY_VERBOSE:
                level = LogLevel::Trace;
                break;
            case SDL_LOG_PRIORITY_DEBUG:
                level = LogLevel::Debug;
                break;
            case SDL_LOG_PRIORITY_INFO:
                level = LogLevel::Info;
                break;
            case SDL_LOG_PRIORITY_WARN:
                level = LogLevel::Warn;
                break;
            case SDL_LOG_PRIORITY_ERROR:
                level = LogLevel::Error;
                break;
            case SDL_LOG_PRIORITY_CRITICAL:
                level = LogLevel::Critical;
                break;
            default:
                level = LogLevel::Trace;
                break;
        }

        LSTG_LOG(kLogCategorySDL.Name, level, "#{} {}", category, message);
    }
}

Logging& Logging::GetInstance() noexcept
{
    static Logging kInstance;
    return kInstance;
}

Logging::Logging() noexcept
{
#ifdef LSTG_SHIPPING
    m_iMinLevel.store(LogLevel::Info, std::memory_order_relaxed);
    m_iMaxLevel.store(LogLevel::Critical, std::memory_order_relaxed);
#else
    m_iMinLevel.store(LogLevel::Trace, std::memory_order_relaxed);
    m_iMaxLevel.store(LogLevel::Critical, std::memory_order_relaxed);
#endif

    try
    {
        m_pImpl = new detail::LogBackend();
    }
    catch (...)  // bad_alloc
    {
        assert(false);
    }

    // 转发 SDL 日志
    ::SDL_LogSetOutputFunction(SdlLogOutputRedirect, nullptr);
}

Logging::~Logging()
{
    ::SDL_LogSetOutputFunction(nullptr, nullptr);
    delete m_pImpl;
}

LogLevel Logging::GetMinLevel() const noexcept
{
    return m_iMinLevel.load(std::memory_order_relaxed);
}

void Logging::SetMinLevel(LogLevel level) noexcept
{
    m_iMinLevel.store(level, std::memory_order_relaxed);
}

LogLevel Logging::GetMaxLevel() const noexcept
{
    return m_iMaxLevel.load(std::memory_order_relaxed);
}

void Logging::SetMaxLevel(LogLevel level) noexcept
{
    m_iMaxLevel.store(level, std::memory_order_relaxed);
}

std::string& Logging::GetTlsBuffer() noexcept
{
    static thread_local std::string kTlsBuffer;
    return kTlsBuffer;
}

bool Logging::ShouldLog(LogLevel level) const noexcept
{
    LogLevel minLevel = GetMinLevel();
    LogLevel maxLevel = GetMaxLevel();
    return level >= minLevel && level <= maxLevel;
}

void Logging::Log(const detail::LogMessage& message) noexcept
{
    if (m_pImpl)
        m_pImpl->Log(message);
}
