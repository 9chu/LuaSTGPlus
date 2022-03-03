/**
 * @file
 * @date 2022/3/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <system_error>

#ifdef __EMSCRIPTEN__
// include lua5.1
extern "C" {
#include <lua.h>
};
#else
// include luajit
#include <lua.hpp>
#endif

namespace lstg::Subsystem::Script
{
    /**
     * Lua 错误代码
     */
    enum class LuaError
    {
        Ok = 0,
        RuntimeError = 2,
        SyntaxError = 3,
        MemoryError = 4,
        ErrorInErrorHandler = 5,
    };

    /**
     * Lua 错误代码分类
     */
    class LuaErrorCategory : public std::error_category
    {
    public:
        static const LuaErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    inline std::error_code make_error_code(LuaError ec) noexcept
    {
        return { static_cast<int>(ec), LuaErrorCategory::GetInstance() };
    }

    /**
     * 用于指示参数错误
     */
    enum class LuaBadArgumentError
    {
        Ok = 0,
        BadArgument1 = 1,
        BadArgument2 = 2,
        BadArgument3 = 3,
        BadArgument4 = 4,
        BadArgument5 = 5,
        BadArgument6 = 6,
        BadArgument7 = 7,
        BadArgument8 = 8,
        BadArgument9 = 9,
        BadArgument10 = 10,
        // ...
    };

    class LuaBadArgumentErrorCategory : public std::error_category
    {
    public:
        static const LuaBadArgumentErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    inline std::error_code make_error_code(LuaBadArgumentError ec) noexcept
    {
        return { static_cast<int>(ec), LuaBadArgumentErrorCategory::GetInstance() };
    }
}

namespace std
{
    template <>
    struct is_error_code_enum<lstg::Subsystem::Script::LuaError> : true_type {};

    template <>
    struct is_error_code_enum<lstg::Subsystem::Script::LuaBadArgumentError> : true_type {};
}
