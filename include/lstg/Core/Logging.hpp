/**
 * @file
 * @author 9chu
 * @date 2022/2/13
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <atomic>
#include <chrono>
#include <string_view>
#include <fmt/format.h>
#include "Result.hpp"

namespace lstg
{
    /**
     * 日志等级
     */
    enum class LogLevel
    {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warn = 3,
        Error = 4,
        Critical = 5,
    };

    namespace detail
    {
        class LogBackend;

        using LogTimePoint = std::chrono::system_clock::time_point;

        /**
         * 获取当前日志时间
         */
        LogTimePoint GetLogCurrentTime() noexcept;

        /**
         * 获取精简文件名
         * @param full 完整文件名
         * @return 精简后的文件名
         */
        const char* GetLogShortFileName(const char* full) noexcept;

        /**
         * 日志源文件位置
         */
        struct LogSourceLocation
        {
            const char* FileName = nullptr;
            const char* FunctionName = nullptr;
            int Line = 0;
        };

        /**
         * 日志消息对象
         */
        struct LogMessage
        {
            const char* CategoryName = nullptr;  // 必须是常量字符串，直接使用指针进行查找
            LogLevel Level = LogLevel::Trace;
            LogTimePoint Time = {};
            LogSourceLocation SourceLocation;
            std::string_view Payload;
        };

        /**
         * 日志分类
         */
        struct LogCategory
        {
            const char* Name;
        };
    }

    /**
     * 日志系统
     */
    class Logging
    {
    public:
        /**
         * 获取全局实例
         * @return 日志系统对象
         */
        static Logging& GetInstance() noexcept;

    public:
        /**
         * 自定义 Sink
         */
        class ICustomSink
        {
        public:
            virtual ~ICustomSink() = default;

        public:
            /**
             * 落地日志
             * @note 需要线程安全
             * @param message 日志
             */
            virtual void Sink(const detail::LogMessage& message) noexcept = 0;
        };

        using CustomSinkPtr = std::shared_ptr<ICustomSink>;

    protected:
        Logging()noexcept;
        Logging(const Logging&) = delete;
        Logging(Logging&&)noexcept = delete;
        ~Logging();

    public:
        /**
         * 获取当前最低的全局日志等级
         * @note 多线程安全
         */
        LogLevel GetMinLevel() const noexcept;

        /**
         * 设置全局最低日志等级
         * @note 多线程安全
         * @param level 等级
         */
        void SetMinLevel(LogLevel level) noexcept;

        /**
         * 获取当前最高的全局日志等级
         * @note 多线程安全
         */
        LogLevel GetMaxLevel() const noexcept;

        /**
         * 设置全局最高日志等级
         * @note 多线程安全
         * @param level 等级
         */
        void SetMaxLevel(LogLevel level) noexcept;

        /**
         * 追加自定义的落地器
         * @param sink 落地器
         * @return 是否成功
         */
        Result<void> AddCustomSink(CustomSinkPtr sink) noexcept;

        /**
         * 移除自定义的落地器
         * @param sink 落地器
         * @return 是否成功
         */
        bool RemoveCustomSink(ICustomSink* sink) noexcept;

        /**
         * 记录日志
         * @note 多线程安全
         * @param message 日志消息
         */
        template <typename... TArgs>
        void Log(const char* categoryName, LogLevel level, detail::LogTimePoint time, detail::LogSourceLocation location,
            TArgs&&... args) noexcept
        {
            if (!ShouldLog(level))
                return;

            detail::LogMessage message;
            message.CategoryName = categoryName;
            message.Level = level;
            message.Time = time;
            message.SourceLocation = location;

            // 精简文件名
            message.SourceLocation.FileName = detail::GetLogShortFileName(message.SourceLocation.FileName);

            try
            {
                std::string& buffer = GetTlsBuffer();
                buffer.clear();
                fmt::format_to(std::back_inserter(buffer), std::forward<TArgs>(args)...);
                message.Payload = buffer.c_str();
            }
            catch (const std::bad_alloc&)
            {
                message.Payload = "<bad_alloc>";
            }
            catch (const fmt::format_error&)
            {
                message.Payload = "<format_error>";
            }
            catch (...)
            {
                message.Payload = "<unknown_error>";
            }

            Log(message);
        }

    private:
        static std::string& GetTlsBuffer() noexcept;
        bool ShouldLog(LogLevel level) const noexcept;
        void Log(const detail::LogMessage& message) noexcept;

    private:
        std::atomic<LogLevel> m_iMinLevel;
        std::atomic<LogLevel> m_iMaxLevel;
        detail::LogBackend* m_pImpl = nullptr;
    };
} // namespace lstg

#define LSTG_LOG(CATEGORY, LEVEL, ...) \
    do {                               \
        lstg::Logging::GetInstance().Log(CATEGORY, LEVEL, lstg::detail::GetLogCurrentTime(), {__FILE__, __FUNCTION__, __LINE__}, \
            __VA_ARGS__);              \
    } while (false)

#define LSTG_LOG_TRACE(...) LSTG_LOG(nullptr, lstg::LogLevel::Trace, __VA_ARGS__)
#define LSTG_LOG_DEBUG(...) LSTG_LOG(nullptr, lstg::LogLevel::Debug, __VA_ARGS__)
#define LSTG_LOG_INFO(...) LSTG_LOG(nullptr, lstg::LogLevel::Info, __VA_ARGS__)
#define LSTG_LOG_WARN(...) LSTG_LOG(nullptr, lstg::LogLevel::Warn, __VA_ARGS__)
#define LSTG_LOG_ERROR(...) LSTG_LOG(nullptr, lstg::LogLevel::Error, __VA_ARGS__)
#define LSTG_LOG_CRITICAL(...) LSTG_LOG(nullptr, lstg::LogLevel::Critical, __VA_ARGS__)

#define LSTG_LOG_TRACE_CAT(CATEGORY, ...) LSTG_LOG(kLogCategory##CATEGORY.Name, lstg::LogLevel::Trace, __VA_ARGS__)
#define LSTG_LOG_DEBUG_CAT(CATEGORY, ...) LSTG_LOG(kLogCategory##CATEGORY.Name, lstg::LogLevel::Debug, __VA_ARGS__)
#define LSTG_LOG_INFO_CAT(CATEGORY, ...) LSTG_LOG(kLogCategory##CATEGORY.Name, lstg::LogLevel::Info, __VA_ARGS__)
#define LSTG_LOG_WARN_CAT(CATEGORY, ...) LSTG_LOG(kLogCategory##CATEGORY.Name, lstg::LogLevel::Warn, __VA_ARGS__)
#define LSTG_LOG_ERROR_CAT(CATEGORY, ...) LSTG_LOG(kLogCategory##CATEGORY.Name, lstg::LogLevel::Error, __VA_ARGS__)
#define LSTG_LOG_CRITICAL_CAT(CATEGORY, ...) LSTG_LOG(kLogCategory##CATEGORY.Name, lstg::LogLevel::Critical, __VA_ARGS__)

#define LSTG_DEF_LOG_CATEGORY(CATEGORY) \
    const lstg::detail::LogCategory kLogCategory##CATEGORY { #CATEGORY }

#define LSTG_REF_LOG_CATEGORY(CATEGORY) \
    extern const lstg::detail::LogCategory kLogCategory##CATEGORY

// 扩展 libfmt 对 error_code 的支持
namespace fmt
{
    template <>
    struct formatter<std::error_code>
    {
        template <typename ParseContext>
        constexpr auto parse(ParseContext& ctx)
        {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const std::error_code& ec, FormatContext& ctx)
        {
            return fmt::format_to(ctx.out(), "{0}:{1}({2})", ec.category().name(), ec.value(), ec.message());
        }
    };
}
