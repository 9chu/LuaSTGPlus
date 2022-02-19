/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <stdexcept>
#include <fmt/format.h>

namespace lstg
{
    /**
     * 记录异常抛出位置
     */
    struct ExceptionSourceLocation
    {
        const char* FileName = nullptr;
        const char* FunctionName = nullptr;
        int Line = 0;
    };

    /**
     * 异常基类
     * 在 LSTGPlus 中，异常仅在构造函数等场合中使用，其他地方使用 Result 机制进行错误码返回，以方便与 Lua 之间的交互。
     */
    class Exception :
        public std::runtime_error
    {
    public:
        template <typename... TArgs>
        Exception(ExceptionSourceLocation loc, TArgs&&... args)
            : std::runtime_error(fmt::format(std::forward<TArgs>(args)...)), m_stLocation(loc)
        {
        }

    public:
        /**
         * 获取异常抛出位置
         */
        [[nodiscard]] const ExceptionSourceLocation& GetLocation() const noexcept { return m_stLocation; }

    private:
        ExceptionSourceLocation m_stLocation;
    };
} // namespace lstg

#define LSTG_DEFINE_EXCEPTION(TAG) \
    class TAG : \
        public lstg::Exception \
    { \
        using Exception::Exception; \
    }

#define LSTG_THROW(TAG, ...) \
    throw TAG(lstg::ExceptionSourceLocation{__FILE__, __FUNCTION__, __LINE__}, __VA_ARGS__)
