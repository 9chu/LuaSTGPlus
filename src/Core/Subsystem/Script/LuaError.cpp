/**
 * @file
 * @date 2022/3/3
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Script/LuaError.hpp>

#include <fmt/format.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Script;

// <editor-fold desc="LuaErrorCategory">

const LuaErrorCategory& LuaErrorCategory::GetInstance() noexcept
{
    static const LuaErrorCategory kInstance;
    return kInstance;
}

const char* LuaErrorCategory::name() const noexcept
{
    return "LuaError";
}

std::string LuaErrorCategory::message(int ev) const
{
    switch (static_cast<LuaError>(ev))
    {
        case LuaError::Ok:
            return "ok";
        case LuaError::RuntimeError:
            return "runtime error";
        case LuaError::SyntaxError:
            return "syntax error";
        case LuaError::MemoryError:
            return "out of memory";
        case LuaError::ErrorInErrorHandler:
            return "error occur in error handler";
        default:
            return "<unknown>";
    }
}

// </editor-fold>
// <editor-fold desc="LuaBadArgumentError">

const LuaBadArgumentErrorCategory& LuaBadArgumentErrorCategory::GetInstance() noexcept
{
    static const LuaBadArgumentErrorCategory kInstance;
    return kInstance;
}

const char* LuaBadArgumentErrorCategory::name() const noexcept
{
    return "LuaBadArgumentError";
}

std::string LuaBadArgumentErrorCategory::message(int ev) const
{
    if (ev == 0)
        return "ok";
    return fmt::format("bad argument #{0}", ev);
}

// </editor-fold>
