/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/RenderError.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render;

const RenderErrorCategory& RenderErrorCategory::GetInstance() noexcept
{
    static const RenderErrorCategory kInstance;
    return kInstance;
}

const char* RenderErrorCategory::name() const noexcept
{
    return "RenderError";
}

std::string RenderErrorCategory::message(int ev) const
{
    switch (static_cast<RenderError>(ev))
    {
        case RenderError::Ok:
            return "ok";
        default:
            return "<unknown>";
    }
}
